; ==========================================================================
; ####################### ARTS IDL INTERFACE PROCEDURE #####################
; ==========================================================================
;
PRO plot_vmr_per_tg, jobname=jobname, $
                     add_to_title=add_to_title,$
                     avoid_tg=avoid_tg, $
                     color=color, $
                     cm=cm,$
                     xrange=xrange, $
                     yrange=yrange,$
                     nostamp=nostamp, $
                     jobdir=jobdir, $
                     punit=punit, $
                     tunit=tunit, $
                     vmrunit=vmrunit, $
                     altunit=altunit, $
                     plotfilename=plotfilename,$
                     plotfileformat=plotfileformat
;;;;;;;;;;;;;;;;;
;+
;NAME:
;      plot_vmr_per_tg.pro
;
;PURPOSE:
; reads an arts abs_per_tg file, frequency file, and altitude file ,
; and plots the different tag group absorption, the tag groups are
; identified by reading the arts controlfile. procedure does not plot
; zero absorption of tag groups. Procedure can handle compressed and
; gzipped files without uncompressing them.
;
; INPUT KEYWORDS:
;
;     jobname         : jobname of calculation
;     jobdir          : sting containing the directory where the
;                       jobfiles are located 
;     add_to_title    : string-is added to default title
;     avoid_tg        : array of strings-which tag groups to plot
;     color           : int-make color plot (actually always color)
;     nostamp         : int-produce no stamp info on output
;     pressure        : selects pressure information
;                       Possible values are: hPa, mbar, bar, Pa
;     altunit        :  selects altitude information
;                       Possible values are: km, m
;     temperature     : selects temperature information
;                       Possible values are: K, C
;     vmrunit         : selection of the absorption units
;                       Possible units are:
;                       VMR, kg/m3
;     plotfilename    : string containing output file name
;     plotfileformat  : integer variable for the output file format.
;                       Possible output files formats are
;                          1: Postscript portrait mode, 
;                          2: Postscript landscape mode, 
;                          3: encapsulated Postscript portrait mode,
;                          4: encapsulated Postscript landscape mode,
;                          5: window
; HISTORY:
;     2001-06-04 : 1. version TKS
;-
;;;;;;;;;;;;;;;;;;;;;;;;;;
;; read the tag groups from controlfile
read_tag_groups,jobdir+jobname+'.arts','tgsDefine',tg
;; total number of tag groups
tot_tg= (size(tg,/DIMENSIONS))[0]

;; set directory name apropriate
if keyword_set(jobdir) then begin 
    jobdirname = jobdir+jobname
endif else begin
    jobdirname = jobname
endelse

;; now reads the absorption per tag, and the altitudes and
;; frequencies, only ascii arrays are allowed currently
no_vmr_alt_grid = 0
no_vmr_species  = 0
if not keyword_set(read) then begin
    print,'Reading temperature grid data...'
    aii_readfile,jobdirname+'.t_abs.aa',temperature
    print,'Reading altitude grid data...'
    aii_readfile,jobdirname+'.z_abs.aa',alt
    print,'Reading pressure grid data...'
    aii_readfile,jobdirname+'.p_abs.aa',pre
    print,'Reading VMR data...'
    aii_readfile,jobdirname+'.vmrs.aa',vmr
    no_vmr_alt_grid = N_ELEMENTS(vmr[*,0])
    no_vmr_species  = N_ELEMENTS(vmr[0,*])
endif

print, 'number of tags: ', tot_tg
if (tot_tg NE no_vmr_species) then begin
    print, 'ERROR: the number of tags is not equal to the number of'
    print, 'species derived from the vmr input file'
    print, 'total tags       :', tot_tg
    print, 'number of species:', no_vmr_specie
    return
endif

if ( (N_ELEMENTS(alt) NE N_ELEMENTS(pre)) OR $
     (N_ELEMENTS(alt) NE N_ELEMENTS(temperature)) OR $
     (N_ELEMENTS(alt) NE no_vmr_alt_grid) ) then begin
    print, 'ERROR:  the vector length must be the same for the following vectors:'
    print, 'altitude grid points        : ', N_ELEMENTS(alt)
    print, 'pressure grid points        : ', N_ELEMENTS(pre)
    print, 'temperature grid points     : ', N_ELEMENTS(temperature)
    print, 'species VMR alt. grid points: ', no_vmr_alt_grid
    return
endif



;; are certain tag groups selected or should all be plotted?
;; we use min_arr values of zero for avoiding
if keyword_set(avoid_tg) then begin
    avoid_tg_in = aii_where_str(tg1,avoid_tg,dim)
    if dim eq 0 then begin
        print,'Error: No matching avoid_tg found.'
    endif else begin
        min_arr[avoid_tg_in] = 0
    endelse
endif


;; pressure unit setting
string_title_p = ' '
if keyword_set(punit) then begin
    case punit of
        'Pa'   : begin
                 ppscale = 1.000
                 ppunit = '[Pa]'
                 end
        'hPa'  : begin
                 ppscale = 0.010
                 ppunit = '[hPa]'
                 end
        'kPa'  : begin
                 ppscale = 0.001
                 ppunit = '[kPa]'
                 end
        else:    begin
                 ppscale = 1.000
                 ppunit = '[Pa]'
                 end
   endcase
endif else begin
    ppscale = 1.000
    ppunit = '[Pa]'
endelse
prePa = dblarr(N_ELEMENTS(pre))
for i = 0, N_ELEMENTS(pre)-1 do begin
    prePa[i] =  pre[i]           ;; pressure in [Pa]
    pre[i]   = pre[i] * ppscale  ;; pressure in user defined units
endfor


;; altitude unit setting
if keyword_set(altunit) then begin
    case altunit of
        'km'  : begin
                 zscale = 0.001
                 zunit = '[km]'
                 end
        'm'   : begin
                 zscale = 1.000
                 zunit = '[m]'
                 end
        else:    begin
                 zscale = 1.000
                 zunit = '[m]'
                 end
    endcase
endif else begin
    zscale = 1.000
    zunit = '[m]'
endelse
for i = 0, N_ELEMENTS(alt)-1 do begin
    alt[i] = alt[i] * zscale
endfor


;; vmr unit setting
if keyword_set(vmrunit) then begin
    case vmrunit of
        'VMR'  : begin
                 vmrscale = 1.000
                 vmrunit = '[1]'
                 end
        'kg/m3': begin
                 vmrscale = 1.000
                 vmrunit = '[kg/m!U3!N]'
                 end
        else:    begin
                 vmrscale = 1.000
                 vmrunit = '[1]'
                 end
    endcase
endif else begin
    vmrscale = 1.000
    vmrunit = 'VMR'
endelse
for i = 0,no_vmr_alt_grid-1 do begin
    for j = 0,no_vmr_species-1 do begin
        vmr[i,j] = vmr[i,j] * vmrscale
    endfor
endfor


;; temperature unit setting
string_title_T = ' '
if keyword_set(tunit) then begin
    case tunit of
        'K' : begin
              ttsub = 0.000
              ttunit = '[K]'
              end
        'C' : begin
              ttsub = -273.15
              ttunit = '[C]'
              end
        else: begin
              ttsub = 0.000
              ttunit = '[K]'
              end
    endcase
endif else begin
    ttsub = 0.000
    ttunit = '[K]'
endelse
for i = 0, N_ELEMENTS(alt)-1 do begin
    temperature[i] = temperature[i] + ttsub
endfor

print, '--- end of unit selection ---'

;; check for color output? -> always use colors
if not keyword_set(color) then color=1

;; save settings
P_ini = !P

;; use aii_plot_file for writing into plot output file
if not keyword_set(plotfilename)   then plotfilename=jobname+'_plot'
if not keyword_set(plotfileformat) then plotfileformat=2
aii_plot_file, action='begin', fname=plotfilename, fformat=plotfileformat

;; create color and lineshape arrays
colors=intarr(tot_tg)
ls=intarr(tot_tg)
for i=0,tot_tg-1 do begin
    colors[i] = i mod 29    ;; we have 29 different colors in mycolor
    ls[i]     = i mod  5    ;; we have  5 different line styles
endfor

;; set line thickness
thick      = 4.5                 ; standard value of thick
slidethick = 5.0                 ; value of thick to use for slides

;; 6 plots on a single page
!P.multi  = 0
posvec = [[0.2, 0.7, 0.45, 0.9], $
          [0.6, 0.7, 0.85, 0.9], $
          [0.2, 0.4, 0.45, 0.6], $
          [0.6, 0.4, 0.85, 0.6], $
          [0.2, 0.1, 0.45, 0.3], $
          [0.6, 0.1, 0.85, 0.3]]
!P.multi  = [0,2,3]

;; set the rright y-axis side to altitude
y2axval_length = 6
y2axval = fltarr(y2axval_length)
y2axval[0] = alt[0]
y2axval[1] = alt[FIX(N_ELEMENTS(alt) * 0.20)]
y2axval[2] = alt[FIX(N_ELEMENTS(alt) * 0.40)]
y2axval[3] = alt[FIX(N_ELEMENTS(alt) * 0.60)]
y2axval[3] = alt[FIX(N_ELEMENTS(alt) * 0.80)]
y2axval[4] = alt[N_ELEMENTS(alt)-1]



;; special water tags tags
h2ovmr                = dblarr(N_ELEMENTS(alt)) ; H2O VMR profile 
liquidcloudprofile    = dblarr(N_ELEMENTS(alt)) ; liquid cloud profile 
icecloudprofile       = dblarr(N_ELEMENTS(alt)) ; ice cloud profile 
h2ovmr[*]             = 0.000
liquidcloudprofile[*] = 0.000
icecloudprofile[*]    = 0.000
watercloudtag         = -1
icecloudtag           = -1
h2otag                = -1

;; y-axis range
alt_range  = [max(pre[*]), min(pre[*])] ; in pressure
alti_range = [min(alt[*]), max(alt[*])] ; in altitude



;; loop over tags
index = 0 & posi = 0
for k = 0,N_ELEMENTS(tg)-1 do begin ; loop over all tags

;;  construct title of the plot:
    total_title= 'species="'+tg[k]+'"'

;;  find min and max of the tag concentration
    xmin = MIN(vmr[*,k])
    xmax = MAX(vmr[*,k])
    if ((xmin/xmax) GT 0.10) then begin
        xmin = xmin *  0.100
        xmax = xmax * 10.000
    endif

;;  water vapor tag found?
    if ( STRPOS(tg[k],'H2O') GE 0) then begin
        h2otag    = k
    endif

;;  water cloud in the tag group?
    if ( STRPOS(tg[k],'liquidcloud') GE 0) then begin
        watercloudtag = k
    endif

;;  ice cloud in the tag group?
    if ( STRPOS(tg[k],'icecloud') GE 0) then begin
        icecloudtag = k
    endif

;;  counts plots per single page
    if (posi GE 6) then begin
        posi = 0
        !P.multi  = [0,2,3]
    endif

;;  plot VMR vs. altitude/pressure (exclude clouds here)
    if ( ( STRPOS(tg[k],'icecloud')    LT 0) AND $
         ( STRPOS(tg[k],'liquidcloud') LT 0) ) then begin
        plot, /XLOG,                  $
          vmr[0:N_ELEMENTS(alt)-1,k], $
          pre[0:N_ELEMENTS(alt)-1],   $
          title=total_title,                     $
          yrange=alt_range,                      $
          xrange=[xmin, xmax],                   $
          xtitle=vmrunit,                        $
          XCHARSIZE=1.2,                         $
          YCHARSIZE=1.2,                         $
          color=colors[index],                   $
          linestyle = ls[index],                 $
          thick=thick,                           $
          xstyle=1,                              $
          ystyle=4,                              $
          POSITION=[posvec[0:3,posi]]

        print, 'POSITION=', [posvec[0:3,posi]]

        axis, yaxis=0, $
          yrange=[max(pre[0:N_ELEMENTS(alt)-1]), min(pre[0:N_ELEMENTS(alt)-1])], $
          /save, $
          YCHARSIZE=1.1, $
          ytitle='pressure '+ppunit

        axis, yaxis=1, $
          yrange=[min(alt[0:N_ELEMENTS(alt)-1]), max(alt[0:N_ELEMENTS(alt)-1])], $
          /save, $
          YCHARSIZE=1.1, $
          ytitle='altitude '+zunit

        posi = posi + 1
    endif
endfor





;; --- plot temperature profile -------------------------------------
if (posi GE 6) then begin
    posi = 0
    !P.multi  = [0,2,3]
endif
xmin = min(temperature[0,0:N_ELEMENTS(alt)-1])-10.0
xmax = max(temperature[0,0:N_ELEMENTS(alt)-1])+10.0
plot, $
  temperature[0,0:N_ELEMENTS(alt)-1],$
  alt[0:N_ELEMENTS(alt)-1],$
  title='Temperature profile',$
  yrange=alti_range,$
  xrange=[xmin, xmax],$
  xtitle=ttunit,$
  ytitle='altitude '+zunit,$
  color=colors[0],$
  linestyle=ls[0],$
  thick=thick,$
  xstyle=1,$
  ystyle=4, $
  POSITION=[posvec[0:3,posi]]
    
print, 'POSITION=', [posvec[0:3,posi]]

axis, yaxis=0, $
  yrange=alt_range, $
  /save, $
  YCHARSIZE=1.1, $
  ytitle='pressure '+ppunit

axis, yaxis=1, $
  yrange=alti_range, $
  /save, $
  YCHARSIZE=1.1, $
  ytitle='altitude '+zunit

posi = posi+ 1





;; --- relative humidity calculation -----------------------------------------------------
rhfound = 1
RHliquid = dblarr(N_ELEMENTS(alt))
RHice    = dblarr(N_ELEMENTS(alt))
if ( h2otag GE 0) then begin
    for z = 0, N_ELEMENTS(alt)-1 do begin
        TT = temperature[0,z]
        if (ttunit EQ '[C]') then TT = temperature[0,z]+273.15
        esw = 0.000
        esi = 0.000
        esw = ppscale * WaterVaporSatPressure(TT, phase='water', punit='Pa', corr='yes')
        esi = ppscale * WaterVaporSatPressure(TT, phase='ice',   punit='Pa', corr='yes')
        RHliquid[z] = 100.00 * ( pre[z] * vmr[z,h2otag] ) / esw ; [%] 
        RHice[z]    = 100.00 * ( pre[z] * vmr[z,h2otag] ) / esi ; [%]
    endfor

    if (posi GE 6) then begin
        posi = 0
        !P.multi  = [0,2,3]
    endif
    
; --- plot RH (over liquid water)
    plot, $
      RHliquid[0:N_ELEMENTS(alt)-1],$
      pre[0:N_ELEMENTS(alt)-1],$
      title='relative humidity profile (over liquid water)',$
      xrange=[0.0, 110.0],$
      yrange=alt_range, $
      xtitle='[%]',$  
      XCHARSIZE=1.2, $
      YCHARSIZE=1.2, $
      color=colors[0],$
      linestyle=ls[0],$
      thick=thick,$
      xstyle=1,$
      ystyle=4, $
      POSITION=[posvec[0:3,posi]]

    print, 'POSITION=', [posvec[0:3,posi]]

    axis, yaxis=0, $
      yrange=alt_range, $
      /save, $
      YCHARSIZE=1.1, $
      ytitle='pressure '+ppunit
    
    axis, yaxis=1, $
      yrange=alti_range, $
      /save, $
      YCHARSIZE=1.1, $
      ytitle='altitude '+zunit

    posi = posi+ 1
endif


!P.multi  = [0,2,3]



;; --- relative humidity and liquid water clouds superimposed --------------------
if ( (h2otag GE 0) and (watercloudtag GE 0) ) then begin

    if (posi GE 6) then begin
        posi = 0
        !P.multi  = [0,2,3]
    endif

; plot RH:
    plot, $
      RHliquid[0:N_ELEMENTS(alt)-1],  $
      pre[0:N_ELEMENTS(alt)-1], $
      xrange=[0.0, 110.0],$
      yrange=alt_range, $
      xtitle='relative humidity [%]',$  
      XCHARSIZE=1.2, $
      YCHARSIZE=1.2, $
      color=colors[0],$
      linestyle=ls[0],$
      thick=thick,$
      xstyle=1,$
      ystyle=4, $
      POSITION=[posvec[0:3,posi]]

    print, 'POSITION=', [posvec[0:3,posi]]

    axis, yaxis=0, $
      yrange=alt_range, $
      /save, $
      YCHARSIZE=1.1, $
      ytitle='pressure '+ppunit
    
    axis, yaxis=1, $
      yrange=alti_range, $
      /save, $
      YCHARSIZE=1.1, $
      ytitle='altitude '+zunit

;  plot liquid water clouds:
    water_range = [0.0, 1.0e6*1.01*max(vmr[0:N_ELEMENTS(alt)-1,watercloudtag])]
    plot, $
      1.0e6*vmr[0:N_ELEMENTS(alt)-1,watercloudtag],$
      pre[0:N_ELEMENTS(alt)-1],$
      xrange=water_range,$
      yrange=alt_range, $
      XCHARSIZE=1.2, $
      YCHARSIZE=1.2, $
      color=colors[1],$
      linestyle=ls[1],$
      thick=thick,$
      xstyle=4,$
      ystyle=4, $
      POSITION=[posvec[0:3,posi]]
    axis, xaxis=1, $
      xrange=water_range,$
      /save, $
      YCHARSIZE=1.1, $
      xtitle='liquid water density [10!U-6!N kg/m!U3!N]'

    posi = posi+ 1
endif


;; --- relative humidity and ice cloud profiles ----------------------------------
if ( (h2otag GE 0) and (icecloudtag GE 0) ) then begin

    if (posi GE 6) then begin
        posi = 0
        !P.multi  = [0,2,3]
    endif

; plot RH:
    plot, $
      RHice[0:N_ELEMENTS(alt)-1],  $
      pre[0:N_ELEMENTS(alt)-1], $
      xrange=[0.0, 200.0],$
      yrange=alt_range, $
      xtitle='relative humidity [%]',$  
      XCHARSIZE=1.2, $
      YCHARSIZE=1.2, $
      color=colors[0],$
      linestyle=ls[0],$
      thick=thick,$
      xstyle=1,$
      ystyle=4, $
      POSITION=[posvec[0:3,posi]]

    print, 'POSITION=', [posvec[0:3,posi]]

    axis, yaxis=0, $
      yrange=alt_range, $
      /save, $
      YCHARSIZE=1.1, $
      ytitle='pressure '+ppunit
    
    axis, yaxis=1, $
      yrange=alti_range, $
      /save, $
      YCHARSIZE=1.1, $
      ytitle='altitude '+zunit

;   liquid water clouds -----------------------------------
    ice_range = [0.0, 1.0000e6*1.01*max(vmr[0:N_ELEMENTS(alt)-1,icecloudtag])]
    plot, $
      1.0000e6*vmr[0:N_ELEMENTS(alt)-1,icecloudtag],$
      pre[0:N_ELEMENTS(alt)-1],$
      xrange=ice_range,$
      yrange=alt_range, $
      XCHARSIZE=1.5, $
      YCHARSIZE=1.5, $
      color=colors[2],$
      linestyle=ls[2],$
      thick=thick,$
      xstyle=4,$
      ystyle=4, $
      POSITION=[posvec[0:3,posi]]

    axis, xaxis=1, $
      xrange=ice_range,$
      /save, $
      YCHARSIZE=1.5, $
      xtitle='ice water density [10!U-6!N kg/m!U3!N]'

    posi = posi+ 1
endif



;; close plot output file
aii_plot_file, action='end', show='yes', print='no'



;; cp plot output file to job directory
FINDFILENAMEVEC = FINDFILE(plotfilename+'.*', count=numfilefound)
if (numfilefound EQ 1) then begin
    spawn, 'cp '+FINDFILENAMEVEC[0]+' '+jobdir+'/'+FINDFILENAMEVEC[0]
endif


;; restore settings
!P = P_ini

ende:

end