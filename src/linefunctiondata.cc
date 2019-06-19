/* Copyright (C) 2018
 Richard Larsson <larsson@mps.mpg.de>

 
 This program is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation; either version 2, or (at your option) any
 later version.

 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 USA. */

/** Contains the line function data class
 * \file   linefunctiondata.cc
 * 
 * \author Richard Larsson
 * \date   2018-09-19
 **/


#include "linefunctiondata.h"
#include "constants.h"
#include "file.h"


/*
 Main functions of this file is to compute a 
 variable based on some temperature fit.  The
 implemented temperature fits are:

  
  t0: a constant
  t1: standard HITRAN, x0 (T0 / T) ^ x1
  t2: line shifts, x0 (T0 / T) ^ x1 / (1 + x2 ln(T / T0))
  t3: speed-dependent parameters, x0 + x1 (T - T0)
  t4: second order line mixing, (x0 + x1 (T0 / T - 1)) (T0 / T) ^ x2
  t5: ARTS pressure shift, x0 * (T0 / T) ^ (0.25 + 1.5*x1)

  
 Note that each of these temperature fits need
 to have not only a main function implemented,
 but a derivative based on how many parameters
 are required for ALL OTHER temperature fits.

 
 At the moment, we have three coefficients, the 
 atmospheric temperature, and the temperature
 at which the line parameters were derived.
 This means that each main function must implement
 derivative functionality with regards to:

 
  x0
  x1,
  x2,
  T, and
  T0,

  
 where x0-x2 are coefficients often derived from lab
 work, T is the atmospheric temperature, and T0 is the
 temperature at which x0-x2 have been derived.  Note
 that the latter has to be the same not just for line
 broadening and mixing parameters, but also for other
 line parameters, notably, the line strength.

 
 Update all derivatives when/if there are more coefficients
 in the future.
*/


/**********************************
 Main functionality, TH = T0/T 
**********************************/

//! Returns x0
constexpr Numeric main_t0(const Numeric& x0) { return x0; }

//! Returns x0 * pow(TH, x1)
inline Numeric main_t1(const Numeric& T, const Numeric& T0, const Numeric& x0, const Numeric& x1) noexcept  { return x0 * pow(T0/T, x1); }

//! Returns x0 * pow(TH, x1) * (1 + x2 * log(1/TH))
inline Numeric main_t2(const Numeric& T, const Numeric& T0, const Numeric& x0, const Numeric& x1, const Numeric& x2) noexcept { return x0 * pow(T0/T, x1) * (1 + x2 * log(T/T0)); }

//! Returns x0 + x1 * (T - T0)
constexpr Numeric main_t3(const Numeric& T, const Numeric& T0, const Numeric& x0, const Numeric& x1) { return x0 + x1 * (T - T0); }

//! Returns (x0 + x1 * (TH - 1)) * pow(TH, x2)
inline Numeric main_t4(const Numeric& T, const Numeric& T0, const Numeric& x0, const Numeric& x1, const Numeric& x2) noexcept { return (x0 + x1 * (T0/T - 1.)) * pow(T0/T, x2); }

//! Returns x0 * pow(TH, 0.25 + 1.5*x1)
inline Numeric main_t5(const Numeric& T, const Numeric& T0, const Numeric& x0, const Numeric& x1) noexcept { return x0 * pow(T0/T, 0.25 + 1.5*x1); }


/**********************************
 Derivatives with regards to x0  
**********************************/

//! Returns 1
constexpr Numeric dmain_dx0_t0() { return 1; }

//! Returns pow(T0/T, x1)
inline Numeric dmain_dx0_t1(const Numeric& T, const Numeric& T0, const Numeric& x1) noexcept { return pow(T0/T, x1); }

//! Returns pow(T0/T, x1)*(x2*log(T/T0) + 1)
inline Numeric dmain_dx0_t2(const Numeric& T, const Numeric& T0, const Numeric& x1, const Numeric& x2) noexcept { return pow(T0/T, x1)*(x2*log(T/T0) + 1); }

//! Returns 1
constexpr Numeric dmain_dx0_t3() { return 1; }

//! Returns pow(T0/T, x2)
inline Numeric dmain_dx0_t4(const Numeric& T, const Numeric& T0, const Numeric& x2) noexcept { return pow(T0/T, x2); }

//! Returns pow(T0/T, 1.5*x1 + 0.25)
inline Numeric dmain_dx0_t5(const Numeric& T, const Numeric& T0, const Numeric& x1) noexcept { return pow(T0/T, 1.5*x1 + 0.25); }


/**********************************
 Derivatives with regards to x1  
**********************************/

//! Returns 0
constexpr Numeric dmain_dx1_t0() { return 0.; }

//! Returns x0*pow(T0/T, x1)*log(T0/T)
inline Numeric dmain_dx1_t1(const Numeric& T, const Numeric& T0, const Numeric& x0, const Numeric& x1) noexcept { return x0*pow(T0/T, x1)*log(T0/T); }

//! Returns x0*pow(T0/T, x1)*(x2*log(T/T0) + 1)*log(T0/T)
inline Numeric dmain_dx1_t2(const Numeric& T, const Numeric& T0, const Numeric& x0, const Numeric& x1, const Numeric& x2) noexcept { return x0*pow(T0/T,x1)*(x2*log(T/T0)+1.)*log(T0/T); }

//! Returns (T - T0)
constexpr Numeric dmain_dx1_t3(const Numeric& T, const Numeric& T0) { return (T - T0); }

//! Returns pow(T0/T, x2)*(T0/T - 1)
inline Numeric dmain_dx1_t4(const Numeric& T, const Numeric& T0, const Numeric& x2) noexcept { return pow(T0/T, x2)*(T0/T - 1.); }

//! Returns 1.5*x0*pow(T0/T, 1.5*x1 + 0.25)*log(T0/T)
inline Numeric dmain_dx1_t5(const Numeric& T, const Numeric& T0, const Numeric& x0, const Numeric& x1) noexcept { return 1.5*x0*pow(T0/T, 1.5*x1 + 0.25)*log(T0/T); }


/**********************************
 Derivatives with regards to x2  
**********************************/

//! Returns 0
constexpr Numeric dmain_dx2_t0() { return 0.; }

//! Returns 0
constexpr Numeric dmain_dx2_t1() { return 0.; }

//! Returns x0*pow(T0/T, x1)*log(T/T0)
inline Numeric dmain_dx2_t2(const Numeric& T, const Numeric& T0, const Numeric& x0, const Numeric& x1) noexcept { return x0*pow(T0/T, x1)*log(T/T0); }

//! Returns 0
constexpr Numeric dmain_dx2_t3() { return 0.; }

//! Returns pow(T0/T, x2)*(x0 + x1*(T0/T-1))*log(T0/T)
inline Numeric dmain_dx2_t4(const Numeric& T, const Numeric& T0, const Numeric& x0, const Numeric& x1, const Numeric& x2) noexcept { return pow(T0/T,x2)*(x0+x1*(T0/T-1))*log(T0/T); }

//! Returns 0
constexpr Numeric dmain_dx2_t5() { return 0.; }


/**********************************
 Derivatives with regards to T 
**********************************/

//! Returns 0
constexpr Numeric dmain_dT_t0() { return 0.; }

//! Returns -x0*x1*pow(T0/T, x1)/T
inline Numeric dmain_dT_t1(const Numeric& T, const Numeric& T0, const Numeric& x0, const Numeric& x1) noexcept { return -x0*x1*pow(T0/T, x1)/T; }

//! Returns -x0*x1*pow(T0/T, x1)*(x2*log(T/T0) + 1)/T + x0*x2*pow(T0/T, x1)/T
inline Numeric dmain_dT_t2(const Numeric& T, const Numeric& T0, const Numeric& x0, const Numeric& x1, const Numeric& x2) noexcept { return -x0*x1*pow(T0/T,x1)*(x2*log(T/T0)+1.)/T+x0*x2*pow(T0/T,x1)/T; }

//! Returns x1
constexpr Numeric dmain_dT_t3(const Numeric& x1) { return x1; }

//! Returns -x2*pow(T0/T, x2)*(x0 + x1*(T0/T-1))/T - T0*x1*pow(T0/T, x2)/pow(T, 2)
inline Numeric dmain_dT_t4(const Numeric& T, const Numeric& T0, const Numeric& x0, const Numeric& x1, const Numeric& x2) noexcept { return -x2*pow(T0/T,x2)*(x0+x1*(T0/T-1.))/T-T0*x1*pow(T0/T,x2)/pow(T,2); }

//! Returns -x0*pow(T0/T, 1.5*x1 + 0.25)*(1.5*x1 + 0.25)/T
inline Numeric dmain_dT_t5(const Numeric& T, const Numeric& T0, const Numeric& x0, const Numeric& x1) noexcept { return -x0*pow(T0/T,1.5*x1 + 0.25)*(1.5*x1 + 0.25)/T; }


/**********************************
 Derivatives with regards to T0  
**********************************/

//! Returns 0
constexpr Numeric dmain_dT0_t0() { return 0.; }

//! Returns x0*x1*pow(T0/T, x1)/T0
inline Numeric dmain_dT0_t1(const Numeric& T, const Numeric& T0, const Numeric& x0, const Numeric& x1) noexcept { return x0*x1*pow(T0/T, x1)/T0; }

//! Returns x0*x1*pow(T0/T, x1)*(x2*log(T/T0) + 1)/T0 - x0*x2*pow(T0/T, x1)/T0
inline Numeric dmain_dT0_t2(const Numeric& T, const Numeric& T0, const Numeric& x0, const Numeric& x1, const Numeric& x2) noexcept { return x0*x1*pow(T0/T,x1)*(x2*log(T/T0)+1.)/T0-x0*x2*pow(T0/T,x1)/T0; }

//! Returns -x1
constexpr Numeric dmain_dT0_t3(const Numeric& x1) { return -x1; }

//! Returns x2*pow(T0/T, x2)*(x0 + x1*(T0/T - 1))/T0 + x1*pow(T0/T, x2)/T
inline Numeric dmain_dT0_t4(const Numeric& T, const Numeric& T0, const Numeric& x0, const Numeric& x1, const Numeric& x2) noexcept { return x2*pow(T0/T,x2)*(x0+x1*(T0/T-1.))/T0+x1*pow(T0/T,x2)/T; }

//! Returns x0*pow(T0/T, 1.5*x1 + 0.25)*(1.5*x1 + 0.25)/T0
inline Numeric dmain_dT0_t5(const Numeric& T, const Numeric& T0, const Numeric& x0, const Numeric& x1) noexcept { return x0*pow(T0/T,1.5*x1 + 0.25)*(1.5*x1 + 0.25)/T0; }


//! Select line parameter based on parameter order
/*! 
 * This helper code simply selects which of the input the
 * param-index is pointing at as a function of the type of
 * line shape under consideration
 * 
 * This function is assumed to never reach its last statement
 * 
 * \param G0    The speed-independent pressure broadening term
 * \param D0    The speed-independent pressure shift term
 * \param G2    The speed-dependent pressure broadening term
 * \param D2    The speed-dependent pressure shift term
 * \param FVC   The velocity-changing collisional parameter
 * \param ETA   The correlation between velocity and rotational changes due to collisions
 * \param param Index enumerating the parameter
 * \param type  Enum class for knowing which parameter we are interested in
 * 
 * \return A reference pointing at G0, D0, G2, D2, FVC, or ETA.
 * 
 * \author Richard Larsson
 * \date   2018-07-16
 */
inline Numeric& select_line_shape_param(Numeric& G0,
                                        Numeric& D0,
                                        Numeric& G2,
                                        Numeric& D2,
                                        Numeric& FVC,
                                        Numeric& ETA,
                                        const Index param,
                                        const LineFunctionData::LineShapeType type) noexcept
{
  switch(type) {
    case LineFunctionData::LineShapeType::DP:
      // Do not let this happen!
    case LineFunctionData::LineShapeType::LP:
      switch(param) {
        case Index(LineFunctionData::LorentzParam::G0): return G0;
        case Index(LineFunctionData::LorentzParam::D0): return D0;
      } break;
    case LineFunctionData::LineShapeType::VP:
      switch(param) {
        case Index(LineFunctionData::VoigtParam::G0): return G0;
        case Index(LineFunctionData::VoigtParam::D0): return D0;
      } break;
    case LineFunctionData::LineShapeType::SDVP:
      switch(param) {
        case Index(LineFunctionData::SpeedVoigtParam::G0): return G0;
        case Index(LineFunctionData::SpeedVoigtParam::D0): return D0;
        case Index(LineFunctionData::SpeedVoigtParam::G2): return G2;
        case Index(LineFunctionData::SpeedVoigtParam::D2): return D2;
      } break;
    case LineFunctionData::LineShapeType::HTP:
      switch(param) {
        case Index(LineFunctionData::HTPParam::G0):  return G0;
        case Index(LineFunctionData::HTPParam::D0):  return D0;
        case Index(LineFunctionData::HTPParam::G2):  return G2;
        case Index(LineFunctionData::HTPParam::D2):  return D2;
        case Index(LineFunctionData::HTPParam::FVC): return FVC;
        case Index(LineFunctionData::HTPParam::ETA): return ETA;
      } break;
  }
  return G0;  // Only to suppress warnings, this is not allowed to happen!
}


//! Select line parameter based on parameter order
/*! 
 * This helper code simply selects which of the input the
 * param-index is pointing at as a function of the type of
 * line mixing under consideration
 * 
 * This function is assumed to never reach its last statement
 * 
 * \param Y     The imaginary part of the line shape due to line mixing
 * \param G     The strength-altering due to line mixing
 * \param DV    The frequency shift due to line mixing
 * \param param Index enumerating the parameter
 * \param type  Enum class for knowing which parameter we are interested in
 * 
 * \return A reference pointing at Y, G, or DV.
 * 
 * \author Richard Larsson
 * \date   2018-07-16
 */
inline Numeric& select_line_mixing_param(Numeric& Y,
                                         Numeric& G,
                                         Numeric& DV,
                                         const Index param,
                                         const LineFunctionData::LineMixingOrderType type) noexcept
{
  switch(type) {
    case LineFunctionData::LineMixingOrderType::None:
      // Do not let this happen
    case LineFunctionData::LineMixingOrderType::Interp:
      // This does not matter;
    case LineFunctionData::LineMixingOrderType::LM1:
      switch(param) case Index(LineFunctionData::FirstOrderParam::Y): return Y; break;
    case LineFunctionData::LineMixingOrderType::LM2:
      switch(param) {
        case Index(LineFunctionData::SecondOrderParam::Y):  return Y;
        case Index(LineFunctionData::SecondOrderParam::G):  return G;
        case Index(LineFunctionData::SecondOrderParam::DV): return DV;
      } break;
    case LineFunctionData::LineMixingOrderType::ConstG:
      switch(param) case Index(LineFunctionData::ConstGParam::G):  return G; break;
  }
  return Y;  // Only to suppress warnings, this is not allowed to happen!
}


struct LBLRTM_data {Numeric y, g;} ;

//! Special function for line mixing of LBLRTM type
/*! 
 * LBLRTM interpolates linearly a set of variables.  Data-structure must be as data variable
 * describes it.  This should be tested earlier than here.
 * 
 * \param Y           The imaginary part of the line shape due to line mixing
 * \param G           The strength-altering due to line mixing
 * \param partial_vmr The VMR of the species in question
 * \param T           The atmospheric temperature
 * \param data        The associated data struction [T1, T2, T3, T4, Y1, Y2, Y3, Y4, G1, G2, G3, G4]
 * 
 * \author Richard Larsson
 * \date   2018-07-16
 */
inline LBLRTM_data special_line_mixing_aer(const Numeric& T,
                                           const ConstVectorView data) noexcept {
  if(T < data[1])
    return {data[4] + (T - data[0]) * (data[5] - data[4]) / (data[1] - data[0]),
            data[8] + (T - data[0]) * (data[9] - data[8]) / (data[1] - data[0])};
  else if(T > data[2])
    return {data[6 ] + (T - data[2]) * (data[7 ] - data[6 ]) / (data[3] - data[2]),
            data[10] + (T - data[2]) * (data[11] - data[10]) / (data[3] - data[2])};
  else
    return {data[5] + (T - data[1]) * (data[6 ] - data[5]) / (data[2] - data[1]),
            data[9] + (T - data[1]) * (data[10] - data[9]) / (data[2] - data[1])};
}


//! Are calculations available
/*! 
 * Is the variable "type" in available line mixing or pressure broadening scheme?
 * 
 * If so, return true.  Otherwise, return false.
 * 
 * NOTE to devs:  Never use this function with non constant inputs
 * 
 * \param type
 * 
 * \return Whether to ignore said variable in these calculations
 * 
 * \author Richard Larsson
 * \date   2018-09-20
 */
bool LineFunctionData::ComputesParam(const String& type) const noexcept
{
  if(type == "G0")
    return mp == LineShapeType::LP or mp == LineShapeType::VP or
           mp == LineShapeType::SDVP or mp == LineShapeType::HTP;
  else if(type == "D0")
    return mp == LineShapeType::LP or mp == LineShapeType::VP or
           mp == LineShapeType::SDVP or mp == LineShapeType::HTP;
  else if(type == "G2")
    return mp == LineShapeType::SDVP or mp == LineShapeType::HTP;
  else if(type == "D2")
    return mp == LineShapeType::SDVP or mp == LineShapeType::HTP;
  else if(type == "FVC")
    return mp == LineShapeType::HTP;
  else if(type == "ETA")
    return mp == LineShapeType::HTP;
  else if(type == "Y")
    return mlm == LineMixingOrderType::LM1 or mlm == LineMixingOrderType::LM2 or
           mlm == LineMixingOrderType::Interp or mlm == LineMixingOrderType::ConstG;
  else if(type == "G")
    return mlm == LineMixingOrderType::LM2 or mlm == LineMixingOrderType::Interp;
  else if(type == "DV")
    return mlm == LineMixingOrderType::LM2;
  return false;
}


//! Are calculations available
/*! 
 * Is the variable "type" in available line mixing or pressure broadening scheme?
 * 
 * If so, return true.  Otherwise, return false.
 * 
 * NOTE to devs:  Never use this function with non constant inputs
 * 
 * \param type
 * 
 * \return Whether to ignore said variable in these calculations
 * 
 * \author Richard Larsson
 * \date   2018-09-20
 */
Index LineFunctionData::IndexOfParam(const String& type) const noexcept
{
  if(type == "G0") {
    if(mp == LineShapeType::LP or mp == LineShapeType::VP or
       mp == LineShapeType::SDVP or mp == LineShapeType::HTP)
      return 0;
  }
  else if(type == "D0") {
    if(mp == LineShapeType::LP or mp == LineShapeType::VP or
       mp == LineShapeType::SDVP or mp == LineShapeType::HTP)
      return 1;
  }
  else if(type == "G2") {
    if(mp == LineShapeType::SDVP or mp == LineShapeType::HTP)
      return 2;
  }
  else if(type == "D2") {
    if(mp == LineShapeType::SDVP or mp == LineShapeType::HTP)
      return 3;
  }
  else if(type == "FVC") {
    if(mp == LineShapeType::HTP)
      return 4;
  }
  else if(type == "ETA") {
    if(mp == LineShapeType::HTP)
      return 5;
  }
  else if(type == "Y") {
    if(mlm == LineMixingOrderType::LM1 or mlm == LineMixingOrderType::LM2 or
       mlm == LineMixingOrderType::Interp)
      return 0 + LineShapeTypeNelem();
  }
  else if(type == "G") {
    if(mlm == LineMixingOrderType::LM2 or mlm == LineMixingOrderType::Interp)
      return 1 + LineShapeTypeNelem();
    else if(mlm == LineMixingOrderType::ConstG)
      return 0 + LineShapeTypeNelem();
  }
  else if(type == "DV") {
    if(mlm == LineMixingOrderType::LM2)
      return 2 + LineShapeTypeNelem();
  }
  return -1;
}


//! Compute the pressure broadening and line mixing parameters
/*! 
 * Computes pressure broadening and line mixing for the considered
 * line function data given some atmospheric conditions
 * 
 * \param T0          The line reference temperature
 * \param T           The atmospheric temperature
 * \param P           The atmospheric pressure
 * \param self_vmr    The VMR of the species pressure
 * \param rtp_vmr     The VMR of all species in the atmosphere at a specific radiative transfer point
 * \param abs_species The list of all species in the atmosphere
 * 
 * \return A tuple of all required line broadening and line mixing parameters we consider in ARTS
 * 
 * \author Richard Larsson
 * \date   2018-07-16
 */
LineFunctionDataOutput LineFunctionData::GetParams(const Numeric& T0,
                                                   const Numeric& T,
                                                   const Numeric& P,
                                                   const Numeric& self_vmr,
                                                   const ConstVectorView& rtp_vmr, 
                                                   const ArrayOfArrayOfSpeciesTag& abs_species,
                                                   const bool do_linemixing,
                                                   const bool normalization) const
{
  LineFunctionDataOutput m;
  m.G0=0; m.D0=0; m.G2=0; m.D2=0; m.FVC=0; m.ETA=0; m.Y=0; m.G=0; m.DV=0;
  
  // Set up holders for partial and total VMR
  Numeric total_vmr=0, partial_vmr;
  
  // Add all species up
  for(Index i=0; i<mspecies.nelem(); i++) {
    if(i == 0 and mself)  // The first value might be self-broadening (use self_vmr)
      partial_vmr = self_vmr;
    else if(i == mspecies.nelem()-1 and mbath)  // The last value might be air-broadening (set total_vmr to 1)
      partial_vmr = 1 - total_vmr;
    else {  // Otherwise we have to find the species in the list of species tags
      Index this_species=-1;
      for(Index j=0; j<abs_species.nelem(); j++)
        if(abs_species[j][0].Species() == mspecies[i].Species())
          this_species = j;
        
      // If we cannot find the species, it does not exist and we are in a different atmosphere than covered by the line data
      if(this_species == -1) continue;  // Species does not exist
      partial_vmr = rtp_vmr[this_species];
    }

    // Sum up VMR
    total_vmr += partial_vmr;
    
    // Set up a current counter to keep track of the data
    Index current = 0;
    
    // Make reading easier
    #define x0 mdata[i][current]
    #define x1 mdata[i][current+1]
    #define x2 mdata[i][current+2]
    
    // Do the line shape parameters
    for(Index j=0; j<LineShapeTypeNelem(); j++) {
      // Selects one of G0, D0, G2, D2, FVC, ETA based on the order of the data for this line shape type
      Numeric& param = select_line_shape_param(m.G0, m.D0, m.G2, m.D2, m.FVC, m.ETA, j, mp);
      
      switch(mtypes[i][j]) {
        case TemperatureType::None: break;
        case TemperatureType::T0: param += partial_vmr * main_t0(x0); break;
        case TemperatureType::T1: param += partial_vmr * main_t1(T, T0, x0, x1); break;
        case TemperatureType::T2: param += partial_vmr * main_t2(T, T0, x0, x1, x2); break;
        case TemperatureType::T3: param += partial_vmr * main_t3(T, T0, x0, x1); break;
        case TemperatureType::T4: param += partial_vmr * main_t4(T, T0, x0, x1, x2); break;
        case TemperatureType::T5: param += partial_vmr * main_t5(T, T0, x0, x1); break;
        case TemperatureType::LM_AER: throw std::runtime_error("Not allowed for line shape parameters"); break;
      }
      current += TemperatureTypeNelem(mtypes[i][j]);
    }
    
    // Do the line mixing parameters
    for(Index j=0; j<LineMixingTypeNelem(); j++) {
      Numeric& param = select_line_mixing_param(m.Y, m.G, m.DV, j, mlm);
      switch(mtypes[i][j+LineShapeTypeNelem()]) {
        case TemperatureType::None: break;
        case TemperatureType::T0: param += partial_vmr * main_t0(x0); break;
        case TemperatureType::T1: param += partial_vmr * main_t1(T, T0, x0, x1); break;
        case TemperatureType::T2: param += partial_vmr * main_t2(T, T0, x0, x1, x2); break;
        case TemperatureType::T3: param += partial_vmr * main_t3(T, T0, x0, x1); break;
        case TemperatureType::T4: param += partial_vmr * main_t4(T, T0, x0, x1, x2); break;
        case TemperatureType::T5: param += partial_vmr * main_t5(T, T0, x0, x1); break;
        case TemperatureType::LM_AER:
          {
            // Special case adding to both G and Y and nothing else!
            auto X = special_line_mixing_aer(T, mdata[i][Range(current, joker)]);
            m.Y += partial_vmr * X.y;
            m.G += partial_vmr * X.g;
          }
          break;
      }
      current += TemperatureTypeNelem(mtypes[i][j+LineShapeTypeNelem()]);
    }

    
    // Stop destroying names
    #undef x0
    #undef x1
    #undef x2
  }
  
  // If there was no species at all, then we cannot compute the pressure broadening
  if(total_vmr == 0) return m;
  
  // Rescale to pressure with or without normalization
  const Numeric scale = normalization ? P / total_vmr : P;
  m.G0  *= scale;
  m.D0  *= scale;
  m.G2  *= scale;
  m.D2  *= scale;
  m.FVC *= scale;
  if(normalization) m.ETA /= total_vmr;
  if(do_linemixing) {
    m.Y   *= scale;
    m.G   *= scale * P;
    m.DV  *= scale * P;
  }
  else {
    m.Y = 0.0;
    m.G = 0.0;
    m.DV = 0.0;
  }
  
  return m;
}


//! Compute the pressure broadening and line mixing parameters
/*! 
 * Computes pressure broadening and line mixing for the considered
 * line function data given some atmospheric conditions
 * 
 * \param T0          The line reference temperature
 * \param T           The atmospheric temperature
 * \param P           The atmospheric pressure
 * \param self_vmr    The VMR of the species pressure
 * \param rtp_vmr     The VMR of all species in the atmosphere at a specific radiative transfer point
 * \param abs_species The list of all species in the atmosphere
 * \param rt          The quantity to be retrieved
 * \param line_qi     The line information that tests if self is same as the rest
 * 
 * \return A tuple of all required line broadening and line mixing parameters we consider in ARTS
 * 
 * \author Richard Larsson
 * \date   2018-09-24
 */
LineFunctionDataOutput LineFunctionData::GetVMRDerivs(const Numeric& T0,
                                                      const Numeric& T,
                                                      const Numeric& P,
                                                      const Numeric& self_vmr,
                                                      const ConstVectorView& rtp_vmr,
                                                      const ArrayOfArrayOfSpeciesTag& abs_species,
                                                      const QuantumIdentifier& vmr_qi, 
                                                      const QuantumIdentifier& line_qi,
                                                      const bool do_linemixing,
                                                      const bool normalization) const
{
  LineFunctionDataOutput d;
  d.G0=0; d.D0=0; d.G2=0; d.D2=0; d.FVC=0; d.ETA=0; d.Y=0; d.G=0; d.DV=0;
  
  // Set up holders for partial and total VMR
  Numeric total_vmr=0, partial_vmr;
  bool do_this, done_once=false, air=false;
  
  // Add all species up
  for(Index i=0; i<mspecies.nelem(); i++) {
    do_this = false;
    if(i == 0 and mself) { // The first value might be self-broadening (use self_vmr)
      partial_vmr = self_vmr;
      if(vmr_qi.In(line_qi))  // Limitation: vmr_qi must be distinct species and isotopologue!
        do_this=true;
    }

    else if(i == mspecies.nelem()-1 and mbath) { // The last value might be air-broadening (set total_vmr to 1)
      partial_vmr = 1 - total_vmr;  // Ignore air-broadening for derivatives here because spectroscopy is unclear...
      air=true;
    }
    else {  // Otherwise we have to find the species in the list of species tags
      Index this_species=-1;
      for(Index j=0; j<abs_species.nelem(); j++)
        if(abs_species[j][0].Species() == mspecies[i].Species())
          this_species = j;
        
      // If we cannot find the species, it does not exist and we are in a different atmosphere than covered by the line data
      if(this_species == -1) continue;  // Species does not exist
      
      if(vmr_qi.Species() == mspecies[this_species].Species())
        do_this = true;
      
      partial_vmr = rtp_vmr[this_species];
    }

    // Sum up VMR
    total_vmr += partial_vmr;
    
    if((not do_this or done_once) and not air) continue;
    done_once = true;
    
    // Set up a current counter to keep track of the data
    Index current = 0;
    
    // Make reading easier
    #define x0 mdata[i][current]
    #define x1 mdata[i][current+1]
    #define x2 mdata[i][current+2]
    
    // Do the line shape parameters
    for(Index j=0; j<LineShapeTypeNelem(); j++) {
      Numeric val=0;
      switch(mtypes[i][j]) {
        case TemperatureType::None: break;
        case TemperatureType::T0: val = main_t0(x0); break;
        case TemperatureType::T1: val = main_t1(T, T0, x0, x1); break;
        case TemperatureType::T2: val = main_t2(T, T0, x0, x1, x2); break;
        case TemperatureType::T3: val = main_t3(T, T0, x0, x1); break;
        case TemperatureType::T4: val = main_t4(T, T0, x0, x1, x2); break;
        case TemperatureType::T5: val = main_t5(T, T0, x0, x1); break;
        case TemperatureType::LM_AER: throw std::runtime_error("Not allowed for line shape parameters"); break;
      }
      current += TemperatureTypeNelem(mtypes[i][j]);
      if(air) select_line_shape_param(d.G0, d.D0, d.G2, d.D2, d.FVC, d.ETA, j, mp) -= val;
      else    select_line_shape_param(d.G0, d.D0, d.G2, d.D2, d.FVC, d.ETA, j, mp) += val;
    }
    
    // Do the line mixing parameters
    for(Index j=0; j<LineMixingTypeNelem(); j++) {
      Numeric val=0;
      switch(mtypes[i][j+LineShapeTypeNelem()]) {
        case TemperatureType::None: break;
        case TemperatureType::T0: val = main_t0(x0); break;
        case TemperatureType::T1: val = main_t1(T, T0, x0, x1); break;
        case TemperatureType::T2: val = main_t2(T, T0, x0, x1, x2); break;
        case TemperatureType::T3: val = main_t3(T, T0, x0, x1); break;
        case TemperatureType::T4: val = main_t4(T, T0, x0, x1, x2); break;
        case TemperatureType::T5: val = main_t5(T, T0, x0, x1); break;
        case TemperatureType::LM_AER:
        {
          // Special case adding to both G and Y and nothing else!
          auto X = special_line_mixing_aer(T, mdata[i][Range(current, joker)]);
          if(air) { d.Y -= X.y; d.G -= X.g; }
          else    { d.Y += X.y; d.G += X.g; }
        }
        current += TemperatureTypeNelem(mtypes[i][j+LineShapeTypeNelem()]);
        continue;
      }
      current += TemperatureTypeNelem(mtypes[i][j+LineShapeTypeNelem()]);
      if(air)
        select_line_mixing_param(d.Y, d.G, d.DV, j, mlm) -= val;
      else
        select_line_mixing_param(d.Y, d.G, d.DV, j, mlm) += val;
    }
    
    // Stop destroying names
    #undef x0
    #undef x1
    #undef x2
  }
  
  // If there was no species at all, then we cannot compute the pressure broadening
  if(total_vmr == 0) return d;
  
  // Rescale to pressure with or without normalization
  const Numeric scale = normalization ? P / total_vmr : P;
  d.G0  *= scale;
  d.D0  *= scale;
  d.G2  *= scale;
  d.D2  *= scale;
  d.FVC *= scale;
  if(normalization) d.ETA /= total_vmr;
  if(do_linemixing) {
    d.Y   *= scale;
    d.G   *= scale * P;
    d.DV  *= scale * P;
  }
  else {
    d.Y = 0.0;
    d.G = 0.0;
    d.DV = 0.0;
  }
  
  return d;

}

//! Compute the pressure broadening and line mixing parameters temperature derivatives
/*! 
 * Computes pressure broadening and line mixing temperature 
 * derivatives for the considered line function data given 
 * some atmospheric conditions
 * \param T0          The line reference temperature
 * \param T           The atmospheric temperature
 * \param P           The atmospheric pressure
 * \param self_vmr    The VMR of the species pressure
 * \param rtp_vmr     The VMR of all species in the atmosphere at a specific radiative transfer point
 * \param abs_species The list of all species in the atmosphere
 * 
 * \return A reference pointing at Y, G, or DV.
 * 
 * \author Richard Larsson
 * \date   2018-07-16
 */
LineFunctionDataOutput LineFunctionData::GetTemperatureDerivs(const Numeric& T0,
                                                              const Numeric& T,
                                                              const Numeric& dT,
                                                              const Numeric& P,
                                                              const Numeric& self_vmr,
                                                              const ConstVectorView& rtp_vmr, 
                                                              const ArrayOfArrayOfSpeciesTag& abs_species,
                                                              const bool do_linemixing,
                                                              const bool normalization) const
{
  LineFunctionDataOutput d;
  d.G0=0; d.D0=0; d.G2=0; d.D2=0; d.FVC=0; d.ETA=0; d.Y=0; d.G=0; d.DV=0;
  
  // Set up holders for partial and total VMR
  Numeric total_vmr=0, partial_vmr;
  
  // Add all species up
  for(Index i=0; i<mspecies.nelem(); i++) {
    if(i == 0 and mself)  // The first value might be self-broadening (use self_vmr)
      partial_vmr = self_vmr;
    else if(i == mspecies.nelem()-1 and mbath)  // The last value might be air-broadening (set total_vmr to 1)
      partial_vmr = 1 - total_vmr;
    else {  // Otherwise we have to find the species in the list of species tags
      Index this_species=-1;
      for(Index j=0; j<abs_species.nelem(); j++)
        if(abs_species[j][0].Species() == mspecies[i].Species())
          this_species = j;
        
      // If we cannot find the species, it does not exist and we are in a different atmosphere than covered by the line data
      if(this_species == -1) continue;  // Species does not exist
      partial_vmr = rtp_vmr[this_species];
    }

    // Sum up VMR
    total_vmr += partial_vmr;
    
    // Set up a current counter to keep track of the data
    Index current = 0;
    
    // Make reading easier
    #define x0 mdata[i][current]
    #define x1 mdata[i][current+1]
    #define x2 mdata[i][current+2]
    
    // Do the line shape parameters
    for(Index j=0; j<LineShapeTypeNelem(); j++) {
      // Selects one of G0, D0, G2, D2, FVC, ETA based on the order of the data for this line shape type
      Numeric& param = select_line_shape_param(d.G0, d.D0, d.G2, d.D2, d.FVC, d.ETA, j, mp);
      
      switch(mtypes[i][j]) {
        case TemperatureType::None: break;
        case TemperatureType::T0: param += partial_vmr * dmain_dT_t0(); break;
        case TemperatureType::T1: param += partial_vmr * dmain_dT_t1(T, T0, x0, x1); break;
        case TemperatureType::T2: param += partial_vmr * dmain_dT_t2(T, T0, x0, x1, x2); break;
        case TemperatureType::T3: param += partial_vmr * dmain_dT_t3(x1); break;
        case TemperatureType::T4: param += partial_vmr * dmain_dT_t4(T, T0, x0, x1, x2); break;
        case TemperatureType::T5: param += partial_vmr * dmain_dT_t5(T, T0, x0, x1); break;
        case TemperatureType::LM_AER: throw std::runtime_error("Not allowed for line shape parameters"); break;
      }
      current += TemperatureTypeNelem(mtypes[i][j]);
    }
    
    // Do the line mixing parameters
    for(Index j=0; j<LineMixingTypeNelem(); j++) {
      Numeric& param = select_line_mixing_param(d.Y, d.G, d.DV, j, mlm);
      switch(mtypes[i][j+LineShapeTypeNelem()]) {
        case TemperatureType::None: break;
        case TemperatureType::T0: param += partial_vmr * dmain_dT_t0(); break;
        case TemperatureType::T1: param += partial_vmr * dmain_dT_t1(T, T0, x0, x1); break;
        case TemperatureType::T2: param += partial_vmr * dmain_dT_t2(T, T0, x0, x1, x2); break;
        case TemperatureType::T3: param += partial_vmr * dmain_dT_t3(x1); break;
        case TemperatureType::T4: param += partial_vmr * dmain_dT_t4(T, T0, x0, x1, x2); break;
        case TemperatureType::T5: param += partial_vmr * dmain_dT_t5(T, T0, x0, x1); break;
        case TemperatureType::LM_AER:
          {
              // Special case adding to both G and Y and nothing else!
              auto X1 = special_line_mixing_aer(T+dT, mdata[i][Range(current, joker)]);
              auto X0 = special_line_mixing_aer(T,    mdata[i][Range(current, joker)]);
              d.Y += partial_vmr * (X1.y - X0.y) / dT;
              d.G += partial_vmr * (X1.g - X0.g) / dT;
          }
          break;
      }
      current += TemperatureTypeNelem(mtypes[i][j+LineShapeTypeNelem()]);
    }

    
    // Stop destroying names
    #undef x0
    #undef x1
    #undef x2
  }
  
  if(total_vmr == 0) return d;
  
  // Rescale to pressure with or without normalization
  const Numeric scale = normalization ? P / total_vmr : P;
  d.G0  *= scale;
  d.D0  *= scale;
  d.G2  *= scale;
  d.D2  *= scale;
  d.FVC *= scale;
  if(normalization) d.ETA /= total_vmr;
  if(do_linemixing) {
    d.Y   *= scale;
    d.G   *= scale * P;
    d.DV  *= scale * P;
  }
  else {
    d.Y = 0.0;
    d.G = 0.0;
    d.DV = 0.0;
  }
  
  return d;
}


//! Compute the pressure broadening and line mixing parameters reference temperature derivatives for one broadening species
/*! 
 * Computes pressure broadening and line mixing reference temperature 
 * derivatives for the considered line function data given 
 * some atmospheric conditions
 * \param T0          The line reference temperature
 * \param T           The atmospheric temperature
 * \param P           The atmospheric pressure
 * \param self_vmr    The VMR of the species pressure
 * \param rtp_vmr     The VMR of all species in the atmosphere at a specific radiative transfer point
 * \param abs_species The list of all species in the atmosphere
 * \param rt          The quantity to be retrieved
 * \param line_qi     The line information that must be more than rt.QuantumIdentity()
 * 
 * \return A reference pointing at Y, G, or DV.
 * 
 * \author Richard Larsson
 * \date   2018-07-16
 */
LineFunctionDataOutput LineFunctionData::GetReferenceT0Derivs(const Numeric& T0,
                                                              const Numeric& T,
                                                              const Numeric& P,
                                                              const Numeric& self_vmr,
                                                              const ConstVectorView& rtp_vmr,
                                                              const ArrayOfArrayOfSpeciesTag& abs_species,
                                                              const RetrievalQuantity& rt,
                                                              const QuantumIdentifier& line_qi,
                                                              const bool do_linemixing,
                                                              const bool normalization) const
{
  LineFunctionDataOutput d;
  d.G0=0; d.D0=0; d.G2=0; d.D2=0; d.FVC=0; d.ETA=0; d.Y=0; d.G=0; d.DV=0;
  
  // Doppler broadening has no types...
  if(not rt.QuantumIdentity().In(line_qi)) return d;
  
  // Set up holders for partial and total VMR
  Numeric total_vmr=0, partial_vmr=0;
  // Find calculation details
  Numeric this_vmr=0.0;
  Index this_derivative=-1;
  for(Index i=0; i<mspecies.nelem(); i++) {
    if(i == 0 and mself) { // The first value might be self-broadening (use self_vmr)
      if(rt.Mode() == LineFunctionData_SelfBroadening or line_qi.SpeciesName() == rt.Mode()) {
        this_vmr = self_vmr;
        this_derivative = i;
      }
      partial_vmr = self_vmr;
    }

    else if(i == mspecies.nelem()-1 and mbath) { // The last value might be air-broadening (set total_vmr to 1)
      if(rt.Mode() == LineFunctionData_BathBroadening) { 
        this_vmr = 1 - total_vmr;
        this_derivative = i;
      }
      partial_vmr = 1 - total_vmr;
    }

    else {  // Otherwise we have to find the species in the list of species tags
      Index this_species=-1;
      for(Index j=0; j<abs_species.nelem(); j++) {
        if(abs_species[j][0].Species() == mspecies[i].Species())
          this_species = j;
      }
      
      if(rt.Mode() not_eq LineFunctionData_BathBroadening and rt.Mode() not_eq LineFunctionData_SelfBroadening) {
        if(SpeciesTag(rt.Mode()) == mspecies[i]) {
          this_vmr = rtp_vmr[this_species];
          this_derivative = i;
        }
      }
      
      // If we cannot find the species, it does not exist and we are in a different atmosphere than covered by the line data
      if(this_species == -1) continue;  // Species does not exist
      partial_vmr = rtp_vmr[this_species];
    }

    
    // Sum up VMR
    total_vmr += partial_vmr;
  }
    
  if(this_derivative == -1 or total_vmr == 0) return d;
  
  // Set up a current counter to keep track of the data
  Index current = 0;
  
  // Make reading easier
  #define x0 mdata[this_derivative][current]
  #define x1 mdata[this_derivative][current+1]
  #define x2 mdata[this_derivative][current+2]
  
  // Do the line shape parameters
  for(Index j=0; j<LineShapeTypeNelem(); j++) {
    // Selects one of G0, D0, G2, D2, FVC, ETA based on the order of the data for this line shape type
    Numeric& param = select_line_shape_param(d.G0, d.D0, d.G2, d.D2, d.FVC, d.ETA, j, mp);
    
    switch(mtypes[this_derivative][j]) {
      case TemperatureType::None: break;
      case TemperatureType::T0: param += partial_vmr * dmain_dT0_t0(); break;
      case TemperatureType::T1: param += partial_vmr * dmain_dT0_t1(T, T0, x0, x1); break;
      case TemperatureType::T2: param += partial_vmr * dmain_dT0_t2(T, T0, x0, x1, x2); break;
      case TemperatureType::T3: param += partial_vmr * dmain_dT0_t3(x1); break;
      case TemperatureType::T4: param += partial_vmr * dmain_dT0_t4(T, T0, x0, x1, x2); break;
      case TemperatureType::T5: param += partial_vmr * dmain_dT0_t5(T, T0,x0, x1); break;
      case TemperatureType::LM_AER: throw std::runtime_error("Not allowed for line shape parameters"); break;
    }
    current += TemperatureTypeNelem(mtypes[this_derivative][j]);
  }
  
  // Do the line mixing parameters
  for(Index j=0; j<LineMixingTypeNelem(); j++) {
    Numeric& param = select_line_mixing_param(d.Y, d.G, d.DV, j, mlm);
    switch(mtypes[this_derivative][j+LineShapeTypeNelem()]) {
      case TemperatureType::None: break;
      case TemperatureType::T0: param += partial_vmr * dmain_dT0_t0(); break;
      case TemperatureType::T1: param += partial_vmr * dmain_dT0_t1(T, T0, x0, x1); break;
      case TemperatureType::T2: param += partial_vmr * dmain_dT0_t2(T, T0, x0, x1, x2); break;
      case TemperatureType::T3: param += partial_vmr * dmain_dT0_t3(x1); break;
      case TemperatureType::T4: param += partial_vmr * dmain_dT0_t4(T, T0, x0, x1, x2); break;
      case TemperatureType::T5: param += partial_vmr * dmain_dT0_t5(T, T0, x0, x1); break;
      case TemperatureType::LM_AER: /* No derivatives for it depends on T not T0 */ break;
    }

    current += TemperatureTypeNelem(mtypes[this_derivative][j+LineShapeTypeNelem()]);
  }
  
  // Stop destroying names
  #undef x0
  #undef x1
  #undef x2
  
  // Rescale to pressure with or without normalization
  const Numeric scale = normalization ? this_vmr * P / total_vmr : this_vmr * P;
  d.G0  *= scale;
  d.D0  *= scale;
  d.G2  *= scale;
  d.D2  *= scale;
  d.FVC *= scale;
  if(normalization) d.ETA /= total_vmr;
  if(do_linemixing) {
    d.Y   *= scale;
    d.G   *= scale * P;
    d.DV  *= scale * P;
  }
  else {
    d.Y = 0.0;
    d.G = 0.0;
    d.DV = 0.0;
  }
  
  return d;
}


//! Compute the pressure broadening and line mixing parameters temperature derivatives
/*! 
 * Computes pressure broadening and line mixing line parameter
 * derivatives for the considered line function data given 
 * some atmospheric conditions
 * 
 * \param T0          The line reference temperature
 * \param T           The atmospheric temperature
 * \param P           The atmospheric pressure
 * \param self_vmr    The VMR of the species pressure
 * \param rtp_vmr     The VMR of all species in the atmosphere at a specific radiative transfer point
 * \param abs_species The list of all species in the atmosphere
 * \param rt          The derivative
 * \param line_qi     The identifier of the current line
 * 
 * \return The derivative of the line parameter with regards to rt
 * 
 * \author Richard Larsson
 * \date   2018-09-14
 */
Numeric LineFunctionData::GetLineParamDeriv(const Numeric& T0,
                                            const Numeric& T,
                                            const Numeric& P,
                                            const Numeric& self_vmr,
                                            const ConstVectorView& rtp_vmr, 
                                            const ArrayOfArrayOfSpeciesTag& abs_species,
                                            const RetrievalQuantity& rt, 
                                            const QuantumIdentifier& line_qi,
                                            const bool do_linemixing,
                                            const bool normalization) const
{
  Numeric val=0.0;
  
  // Doppler broadening has no types...
  if(not is_linefunctiondata_parameter(rt) or not rt.QuantumIdentity().In(line_qi)) return val;
  
  // Find calculation details
  Numeric this_vmr=0.0, total_vmr=0, partial_vmr;
  Index this_derivative=-1;
  for(Index i=0; i<mspecies.nelem(); i++) {
    if(i == 0 and mself) { // The first value might be self-broadening (use self_vmr)
      if(rt.Mode() == LineFunctionData_SelfBroadening or line_qi.SpeciesName() == rt.Mode()) {
        this_vmr = self_vmr;
        this_derivative = i;
      }
      partial_vmr = self_vmr;
    }

    else if(i == mspecies.nelem()-1 and mbath) { // The last value might be air-broadening (set total_vmr to 1)
      if(rt.Mode() == LineFunctionData_BathBroadening) { // if it still is not set by this point, it is going to
        this_vmr = 1 - total_vmr;
        this_derivative = i;
      }
      partial_vmr = 1 - total_vmr;
    }
    else {  // Otherwise we have to find the species in the list of species tags
      Index this_species=-1;
      for(Index j=0; j<abs_species.nelem(); j++) {
        if(abs_species[j][0].Species() == mspecies[i].Species())
          this_species = j;
      }
      
      if(rt.Mode() not_eq LineFunctionData_BathBroadening and rt.Mode() not_eq LineFunctionData_SelfBroadening) {
        if(SpeciesTag(rt.Mode()) == mspecies[i]) {
          this_vmr = rtp_vmr[this_species];
          this_derivative = i;
        }
      }
      
      // If we cannot find the species, it does not exist and we are in a different atmosphere than covered by the line data
      if(this_species == -1) continue;  // Species does not exist
      partial_vmr = rtp_vmr[this_species];
    }
    
    // Sum up VMR
    total_vmr += partial_vmr;
  }
  
  if(this_derivative == -1 or total_vmr == 0) return val;  // Species not in list
  Index current=0;
  
  // Make reading easier
  #define x0 mdata[this_derivative][current]
  #define x1 mdata[this_derivative][current+1]
  #define x2 mdata[this_derivative][current+2]
  
  Index param;
  if(rt.PropMatType() == JacPropMatType::LineFunctionDataG0X0 or
     rt.PropMatType() == JacPropMatType::LineFunctionDataG0X1 or
     rt.PropMatType() == JacPropMatType::LineFunctionDataG0X2)
    param = IndexOfParam("G0");
  else if(rt.PropMatType() == JacPropMatType::LineFunctionDataD0X0 or
          rt.PropMatType() == JacPropMatType::LineFunctionDataD0X1 or
          rt.PropMatType() == JacPropMatType::LineFunctionDataD0X2)
    param = IndexOfParam("D0");
  else if(rt.PropMatType() == JacPropMatType::LineFunctionDataG2X0 or
          rt.PropMatType() == JacPropMatType::LineFunctionDataG2X1 or
          rt.PropMatType() == JacPropMatType::LineFunctionDataG2X2)
    param = IndexOfParam("G2");
  else if(rt.PropMatType() == JacPropMatType::LineFunctionDataD2X0 or
          rt.PropMatType() == JacPropMatType::LineFunctionDataD2X1 or
          rt.PropMatType() == JacPropMatType::LineFunctionDataD2X2)
    param = IndexOfParam("D2");
  else if(rt.PropMatType() == JacPropMatType::LineFunctionDataFVCX0 or
          rt.PropMatType() == JacPropMatType::LineFunctionDataFVCX1 or
          rt.PropMatType() == JacPropMatType::LineFunctionDataFVCX2)
    param = IndexOfParam("FVC");
  else if(rt.PropMatType() == JacPropMatType::LineFunctionDataETAX0 or
          rt.PropMatType() == JacPropMatType::LineFunctionDataETAX1 or
          rt.PropMatType() == JacPropMatType::LineFunctionDataETAX2)
    param = IndexOfParam("ETA");
  else if(rt.PropMatType() == JacPropMatType::LineFunctionDataYX0 or
          rt.PropMatType() == JacPropMatType::LineFunctionDataYX1 or
          rt.PropMatType() == JacPropMatType::LineFunctionDataYX2) {
    if(not do_linemixing) return val;
    param = IndexOfParam("Y");
  }
  else if(rt.PropMatType() == JacPropMatType::LineFunctionDataGX0 or
          rt.PropMatType() == JacPropMatType::LineFunctionDataGX1 or
          rt.PropMatType() == JacPropMatType::LineFunctionDataGX2) {
    if(not do_linemixing) return val;
    param = IndexOfParam("G");
  }
  else if(rt.PropMatType() == JacPropMatType::LineFunctionDataDVX0 or
          rt.PropMatType() == JacPropMatType::LineFunctionDataDVX1 or
          rt.PropMatType() == JacPropMatType::LineFunctionDataDVX2) {
    if(not do_linemixing) return val;
    param = IndexOfParam("DV");
  }
  else
    throw std::runtime_error("Developer error!  This should not happen");
  
  // Skip to the right current position if it exists
  if(param == -1) return val;
  else for(Index j=0; j<param; j++) current += TemperatureTypeNelem(mtypes[this_derivative][j]);
  
  // Now perform the calculations, we know the type is available and in the below top level switch-statement 
  switch(rt.PropMatType()) {
    case JacPropMatType::LineFunctionDataG0X0:  
    case JacPropMatType::LineFunctionDataD0X0:
    case JacPropMatType::LineFunctionDataG2X0:
    case JacPropMatType::LineFunctionDataD2X0:
    case JacPropMatType::LineFunctionDataETAX0:
    case JacPropMatType::LineFunctionDataFVCX0:
    case JacPropMatType::LineFunctionDataYX0:
    case JacPropMatType::LineFunctionDataGX0:
    case JacPropMatType::LineFunctionDataDVX0:
      switch(mtypes[this_derivative][param]) {
        case TemperatureType::None: case TemperatureType::LM_AER: val = 0; break;
        case TemperatureType::T0: val = dmain_dx0_t0(); break;
        case TemperatureType::T1: val = dmain_dx0_t1(T, T0, x1); break;
        case TemperatureType::T2: val = dmain_dx0_t2(T, T0, x1, x2); break;
        case TemperatureType::T3: val = dmain_dx0_t3(); break;
        case TemperatureType::T4: val = dmain_dx0_t4(T, T0, x2); break;
        case TemperatureType::T5: val = dmain_dx0_t5(T, T0, x1); break;
      } break;
    case JacPropMatType::LineFunctionDataG0X1:
    case JacPropMatType::LineFunctionDataD0X1:
    case JacPropMatType::LineFunctionDataG2X1:
    case JacPropMatType::LineFunctionDataD2X1:
    case JacPropMatType::LineFunctionDataETAX1:
    case JacPropMatType::LineFunctionDataFVCX1:
    case JacPropMatType::LineFunctionDataYX1:
    case JacPropMatType::LineFunctionDataGX1:
    case JacPropMatType::LineFunctionDataDVX1:
      switch(mtypes[this_derivative][param]) {
        case TemperatureType::None: case TemperatureType::LM_AER: val = 0; break;
        case TemperatureType::T0: val = dmain_dx1_t0(); break;
        case TemperatureType::T1: val = dmain_dx1_t1(T, T0, x0, x1); break;
        case TemperatureType::T2: val = dmain_dx1_t2(T, T0, x0, x1, x2); break;
        case TemperatureType::T3: val = dmain_dx1_t3(T, T0); break;
        case TemperatureType::T4: val = dmain_dx1_t4(T, T0, x2); break;
        case TemperatureType::T5: val = dmain_dx1_t5(T, T0, x0, x1); break;
      } break;
    case JacPropMatType::LineFunctionDataG0X2:
    case JacPropMatType::LineFunctionDataD0X2:
    case JacPropMatType::LineFunctionDataG2X2:
    case JacPropMatType::LineFunctionDataD2X2:
    case JacPropMatType::LineFunctionDataETAX2:
    case JacPropMatType::LineFunctionDataFVCX2:
    case JacPropMatType::LineFunctionDataYX2:
    case JacPropMatType::LineFunctionDataGX2:
    case JacPropMatType::LineFunctionDataDVX2:
      switch(mtypes[this_derivative][param]) {
        case TemperatureType::None: case TemperatureType::LM_AER: val = 0; break;
        case TemperatureType::T0: val = dmain_dx2_t0(); break;
        case TemperatureType::T1: val = dmain_dx2_t1(); break;
        case TemperatureType::T2: val = dmain_dx2_t2(T, T0, x0, x1); break;
        case TemperatureType::T3: val = dmain_dx2_t3(); break;
        case TemperatureType::T4: val = dmain_dx2_t4(T, T0, x0, x1, x2); break;
        case TemperatureType::T5: val = dmain_dx2_t5(); break;
      } break;
    default: 
      throw std::runtime_error("Developer error:  This should not happen");
  }
  
  // Stop destroying names
  #undef x0
  #undef x1
  #undef x2
  
  switch(rt.PropMatType()){ // Squared parameters
    case JacPropMatType::LineFunctionDataGX0:
    case JacPropMatType::LineFunctionDataGX1:
    case JacPropMatType::LineFunctionDataGX2:
    case JacPropMatType::LineFunctionDataDVX0:
    case JacPropMatType::LineFunctionDataDVX1:
    case JacPropMatType::LineFunctionDataDVX2:
      val *= P;
      break;
    default: {/*pass*/}
  }
  
  const bool is_eta = is_pressure_broadening_correlation(rt);
  if(normalization and is_eta)
    val *= this_vmr / total_vmr;
  else if(normalization)
    val *= this_vmr / total_vmr * P;
  else if(is_eta)
    val *= this_vmr;
  else
    val *= this_vmr * P;
  return val;
}


//! Prints data so as operator>> can read it
std::ostream& operator<<(std::ostream& os, const LineFunctionData& lfd) {
  // Keep track of the size of the problem
  const Index nshapes=lfd.LineShapeTypeNelem(), nmixing=lfd.LineMixingTypeNelem();
  
  // Init the problem
  os << lfd.LineShapeType2String() << " " << lfd.LineMixingType2String() << " " << lfd.mspecies.nelem() << " ";
  
  // For all species we should now do mostly the same
  for(Index i=0; i<lfd.mspecies.nelem(); i++) {
    if(i==0 and lfd.mself)
      os << LineFunctionData_SelfBroadening << " ";  // SELF can be the first species
    else if(i==lfd.mspecies.nelem()-1 and lfd.mbath)
      os << LineFunctionData_BathBroadening << " ";  // BATH can be the last species
    else
      os << lfd.mspecies[i].SpeciesNameMain() << " ";  // Otherwise we have a species defined in the assoc. SpeciesTag
    
    // Now we must count the data
    Index counter=0;
    
    // For everything that relates to shapes
    for(Index j=0; j<nshapes; j++) {
      os << lfd.TemperatureType2String(lfd.mtypes[i][j]) << " ";  // Name the temperature type
      for(Index k=0; k<lfd.TemperatureTypeNelem(lfd.mtypes[i][j]); k++)
        os << lfd.mdata[i][counter+k] << " ";  // Print the assoc. data
      counter += lfd.TemperatureTypeNelem(lfd.mtypes[i][j]);  // Count how much data has been printed
    }

    
    // For everything that relates to mixing
    for(Index j=nshapes; j<nmixing+nshapes; j++) {
      os << lfd.TemperatureType2String(lfd.mtypes[i][j]) << " ";
      for(Index k=0; k<lfd.TemperatureTypeNelem(lfd.mtypes[i][j]); k++)
        os << lfd.mdata[i][counter+k] << " ";
      counter += lfd.TemperatureTypeNelem(lfd.mtypes[i][j]);
    }
  }
  
  return os;
}


//! Reads data as created by operator<< can read it
std::istream& operator>>(std::istream& data, LineFunctionData& lfd) {
  lfd.mself = lfd.mbath = false;
  Index specs, c;
  Numeric n;
  String s;
  
  // The first tag should give the line shape scheme
  data >> s;
  lfd.StringSetLineShapeType(s);
  
  // The second tag should give the line mixing scheme
  data >> s;
  lfd.StringSetLineMixingType(s);
  
  // From the line shape and line mixing types, we know how many parameters are needed
  const Index count = lfd.LineShapeTypeNelem() + lfd.LineMixingTypeNelem();
  
  // The third tag should contain the number of species
  data >> specs;
  lfd.mspecies.resize(specs);
  lfd.mtypes = Array<Array<LineFunctionData::TemperatureType>>(specs, Array<LineFunctionData::TemperatureType>(count));
  lfd.mdata.resize(specs);
  
  if(lfd.mp not_eq LineFunctionData::LineShapeType::DP and not specs)
    throw std::runtime_error("Need at least one species for non-Doppler line shapes");
  
  // For all species, we need to set the methods to compute them
  for(Index i=0; i<specs; i++) {
    
    // This should be a species tag or one of the specials, SELF or BATH
    data >> s;
    if(s == LineFunctionData_SelfBroadening) {
      // If the species is self, then  we need to flag this
      lfd.mself = true;
      if(i not_eq 0)  // but self has to be first for consistent behavior
        throw std::runtime_error("Self broadening must be first, it is not\n");
    }

    else if(s == LineFunctionData_BathBroadening) {
      // If the species is air, then we need to flag this
      lfd.mbath = true;
      if(i not_eq specs - 1)  // but air has to be last because it needs the rest's VMR
        throw std::runtime_error("Air/bath broadening must be last, it is not\n");
    }
    else {
      // Otherwise, we hope we find a species
      try {
        lfd.mspecies[i] = SpeciesTag(s);
      }
      catch(const std::runtime_error& e) {
        ostringstream os;
        os << "Encountered " << s << " in a position where a species should have been ";
        os << "defined.\nPlease check your pressure broadening data structure and ensure ";
        os << "that it follows the correct conventions.\n";
        os << "SpeciesTag error reads:  " << e.what();
        throw std::runtime_error(os.str());
      }
    }
    
    ArrayOfNumeric nums; nums.reserve(20); // buffers
    
    // For all parameters
    for(Index j=0; j<count; j++) {
      data >> s;  // Should contain a temperature tag
      lfd.StringSetTemperatureType(i, j, s);
      
      c = lfd.TemperatureTypeNelem(lfd.mtypes[i][j]);
      // Add all new numbers for this line to the numbers
      for(Index k=0; k<c; k++) {
        data >> double_imanip() >> n;
        nums.push_back(n);
      }
    }
    
    // Set the data now that we know how many counts are required
    lfd.mdata[i].resize(nums.nelem());
    for(Index j=0; j<nums.nelem(); j++)
      lfd.mdata[i][j] = nums[j];
  }
  
  // Set the variable that determines whether or not 
  // the line has to be used under non-standard calculations,
  // such as full line mixing.  In case this line needs to be
  // computed separately, a special catalog that references
  // this line needs to be created, and this catalog
  // should ignore the variable below
  lfd.do_line_in_standard_calculations = true;
  
  return data;
}


inline Vector LM2DatafromLineMixingDataVector(const Vector& lmData) {
  return Vector({lmData[0], lmData[1], lmData[7], 
                 lmData[2], lmData[3], lmData[8],
                 lmData[4], lmData[5], lmData[9]});
}


inline Vector LM1DatafromLineMixingDataVector(const Vector& lmData) { return Vector({lmData[1], lmData[2]}); }


//! Splits the old LineMixingData
std::tuple<LineFunctionData::LineMixingOrderType, Array<LineFunctionData::TemperatureType>, Vector, bool> splitLineMixingData(const LineMixingData& lm, const Numeric& T0)
{
  Vector lmData, lmTranslated;
  const LineMixingData::LM_Type lmType = lm.Type();
  lm.GetVectorFromData(lmData);
  
  // Fox LM type and set the line mixing data
  bool do_line_in_standard_calculations;
  LineFunctionData::LineMixingOrderType mlm;
  switch(lmType) {
    case LineMixingData::LM_LBLRTM:
      do_line_in_standard_calculations = true;
      mlm = LineFunctionData::LineMixingOrderType::Interp;
      lmTranslated = std::move(lmData);
      break;
    case LineMixingData::LM_LBLRTM_O2NonResonant:
      do_line_in_standard_calculations = true;
      mlm = LineFunctionData::LineMixingOrderType::ConstG;
      lmTranslated = std::move(lmData);
      break;
    case LineMixingData::LM_1STORDER:
      do_line_in_standard_calculations = true;
      mlm = LineFunctionData::LineMixingOrderType::LM1;
      lmTranslated = LM1DatafromLineMixingDataVector(lmData);
      if(T0 not_eq lmData[0])
        throw std::runtime_error("Cannot translate data of line since it has bad temperature\n"
        "information.  The line would produce poor absorption profiles anyways, so please\n"
        "reform it in the new format of line catalogs.");
      break;
    case LineMixingData::LM_2NDORDER:
      do_line_in_standard_calculations = true;
      mlm = LineFunctionData::LineMixingOrderType::LM2;
      lmTranslated = LM2DatafromLineMixingDataVector(lmData);
        if(T0 not_eq lmData[6])
          throw std::runtime_error("Cannot translate data of line since it has bad temperature\n"
          "information.  The line would produce poor absorption profiles anyways, so please\n"
          "reform it in the new format of line catalogs.");
        break;
    case LineMixingData::LM_NONE:
      do_line_in_standard_calculations = true;
      mlm = LineFunctionData::LineMixingOrderType::None;
      lmTranslated = Vector(0);
      break;
    case LineMixingData::LM_BYBAND:
      do_line_in_standard_calculations = false;
      mlm = LineFunctionData::LineMixingOrderType::None;
      lmTranslated = Vector(0);
      break;
    default:
      throw std::runtime_error("Error, unsupported conversion.  Please update to\n"
      "new line catalog format manually.  Cannot support non-resonant LM as\n"
      "it was a hack to begin with.");
  }
  
  // Set the temperature types
  Array<LineFunctionData::TemperatureType> mtypes;
  switch(mlm) {
    case LineFunctionData::LineMixingOrderType::None:
      mtypes.resize(0);
      break;
    case LineFunctionData::LineMixingOrderType::LM1:
      mtypes.resize(1);
      mtypes[0] = LineFunctionData::TemperatureType::T1;
      break;
    case LineFunctionData::LineMixingOrderType::LM2:
      mtypes = {LineFunctionData::TemperatureType::T4, LineFunctionData::TemperatureType::T4, LineFunctionData::TemperatureType::T4};
      break;
    case LineFunctionData::LineMixingOrderType::Interp:
      mtypes.resize(1);
      mtypes[0] = LineFunctionData::TemperatureType::LM_AER;
      break;
    case LineFunctionData::LineMixingOrderType::ConstG:
      mtypes.resize(1);
      mtypes[0] = LineFunctionData::TemperatureType::T0;
      break;
  }
  
  return std::make_tuple(mlm, mtypes, lmTranslated, do_line_in_standard_calculations);
}


//! Splits the old PressureBroadeningData
std::tuple<LineFunctionData::LineShapeType,  Array<Array<LineFunctionData::TemperatureType>>, 
ArrayOfVector, ArrayOfVector, ArrayOfSpeciesTag, bool, bool> splitPressureBroadeningData(const PressureBroadeningData& pb, const String& species)
{
  const PressureBroadeningData::PB_Type pbType = pb.Type();
  
  Vector pbData;
  pb.GetVectorFromData(pbData);
  
  bool mself = true;
  bool mbath;
  ArrayOfVector mdata;
  ArrayOfVector merrors;
  ArrayOfSpeciesTag mspecies;
  LineFunctionData::LineShapeType mp = LineFunctionData::LineShapeType::VP;
  Array<Array<LineFunctionData::TemperatureType>> mtypes;
  
  // Give an Index number to self if in the list of species for ARTSCAT-4
  const Index speciesID = SpeciesTag("N2" ).IsSpecies(species) ? 0 :
                          SpeciesTag("O2" ).IsSpecies(species) ? 1 :
                          SpeciesTag("H2O").IsSpecies(species) ? 2 :
                          SpeciesTag("CO2").IsSpecies(species) ? 3 :
                          SpeciesTag("H2" ).IsSpecies(species) ? 4 :
                          SpeciesTag("He" ).IsSpecies(species) ? 5 : - 1;
  
  // Fix mspecies and mdata for pressure broadening
  Index inx=0;
  switch(pbType) {
    case PressureBroadeningData::PB_AIR_BROADENING:
      mbath = true;
      mdata = ArrayOfVector(2, Vector(4));
      merrors = ArrayOfVector(2, Vector(4, 0));
      mspecies = {SpeciesTag(species), SpeciesTag()};
      
      // For SELF broadening
      mdata[0] = {pbData[0], pbData[1], pbData[4], pbData[3]}; 
      merrors[0] = {pbData[5], pbData[6], 0, pbData[6]};
      
      // For AIR broadening
      mdata[1] = {pbData[2], pbData[3], pbData[4], pbData[3]};
      merrors[1] = {pbData[7], pbData[8], pbData[9], pbData[8]};
      inx++;
      break;
    case PressureBroadeningData::PB_AIR_AND_WATER_BROADENING:
      mbath = true;
      if(speciesID == 2) { // Water
        mdata = ArrayOfVector(2, Vector(4));
        merrors = ArrayOfVector(2, Vector(4, 0));
        mspecies = {SpeciesTag("H2O"), SpeciesTag()};
        
        mdata[0] = {pbData[6], pbData[7], pbData[8], pbData[7]};  // Self/water
        mdata[1] = {pbData[3], pbData[4], pbData[5], pbData[4]};  // Air
      }
      else { // not Water
        mdata = ArrayOfVector(3, Vector(4));
        merrors = ArrayOfVector(3, Vector(4, 0));
        mspecies = {SpeciesTag(species), SpeciesTag("H2O"), SpeciesTag()};
        
        mdata[0] = {pbData[0], pbData[1], pbData[2], pbData[1]};  // self
        mdata[1] = {pbData[6], pbData[7], pbData[8], pbData[7]};  // water
        mdata[2] = {pbData[3], pbData[4], pbData[5], pbData[4]};  // air
      }
      break;
    case PressureBroadeningData::PB_PLANETARY_BROADENING:
      mbath = false;
      if(speciesID not_eq -1) {
        mdata = ArrayOfVector(6, Vector(4));
        merrors = ArrayOfVector(6, Vector(4, 0));
        mspecies.resize(6);
        
        if(speciesID == 0) { mdata[inx] = {pbData[1], pbData[8], pbData[14], pbData[8]};   mspecies[inx] = SpeciesTag("N2");  inx++; }// N2
        if(speciesID == 1) { mdata[inx] = {pbData[2], pbData[9], pbData[15], pbData[9]};   mspecies[inx] = SpeciesTag("O2");  inx++; }// O2
        if(speciesID == 2) { mdata[inx] = {pbData[3], pbData[10], pbData[16], pbData[10]}; mspecies[inx] = SpeciesTag("H2O"); inx++; }// H2O
        if(speciesID == 3) { mdata[inx] = {pbData[4], pbData[11], pbData[17], pbData[11]}; mspecies[inx] = SpeciesTag("CO2"); inx++; }// CO2
        if(speciesID == 4) { mdata[inx] = {pbData[5], pbData[12], pbData[18], pbData[12]}; mspecies[inx] = SpeciesTag("H2");  inx++; }// H2
        if(speciesID == 5) { mdata[inx] = {pbData[6], pbData[13], pbData[19], pbData[13]}; mspecies[inx] = SpeciesTag("He");  inx++; }// He
      }
      else {
        mdata = ArrayOfVector(7, Vector(4));
        merrors = ArrayOfVector(7, Vector(4, 0));
        mspecies = {SpeciesTag(species), SpeciesTag("N2"), SpeciesTag("O2"), SpeciesTag("H2O"), SpeciesTag("CO2"), SpeciesTag("H2"), SpeciesTag("He")};
        
        mdata[inx] = {pbData[0], pbData[7], 0, pbData[7]}; // self
        inx++;
      }
      
      if(speciesID not_eq 0) { mdata[inx] = {pbData[1], pbData[8], pbData[14], pbData[8]};   mspecies[inx] = SpeciesTag("N2");  inx++; }// N2
      if(speciesID not_eq 1) { mdata[inx] = {pbData[2], pbData[9], pbData[15], pbData[9]};   mspecies[inx] = SpeciesTag("O2");  inx++; }// O2
      if(speciesID not_eq 2) { mdata[inx] = {pbData[3], pbData[10], pbData[16], pbData[10]}; mspecies[inx] = SpeciesTag("H2O"); inx++; }// H2O
      if(speciesID not_eq 3) { mdata[inx] = {pbData[4], pbData[11], pbData[17], pbData[11]}; mspecies[inx] = SpeciesTag("CO2"); inx++; }// CO2
      if(speciesID not_eq 4) { mdata[inx] = {pbData[5], pbData[12], pbData[18], pbData[12]}; mspecies[inx] = SpeciesTag("H2");  inx++; }// H2
      if(speciesID not_eq 5) { mdata[inx] = {pbData[6], pbData[13], pbData[19], pbData[13]}; mspecies[inx] = SpeciesTag("He");  inx++; }// He
      break;
    default:
      throw std::runtime_error("Error, unsupported conversion.  Please update to\n"
      "new line catalog format manually.  Only air, water+air, and planetary\n"
      "conversions are accepted as the rest where experimental at time of\n"
      "implementing the new code.");
  }
  
  // Fix mtypes
  mtypes.resize(mdata.nelem(), {LineFunctionData::TemperatureType::T1, LineFunctionData::TemperatureType::T5});
  
  return std::make_tuple(mp, mtypes, mdata, merrors, mspecies, mbath, mself);
}


LineFunctionData::LineFunctionData(const PressureBroadeningData& pb,
                                   const LineMixingData& lm,
                                   const String& species,
                                   const Numeric& T0)
{
  const std::tuple<LineMixingOrderType, Array<TemperatureType>, Vector, bool> LM = splitLineMixingData(lm, T0);
  std::tuple<LineShapeType,  Array<Array<TemperatureType>>, ArrayOfVector, ArrayOfVector, ArrayOfSpeciesTag, bool, bool> PB = splitPressureBroadeningData(pb, species);
  
  do_line_in_standard_calculations = std::get<3>(LM);
  mlm = std::get<0>(LM);
  mp = std::get<0>(PB);
  mspecies = std::get<4>(PB);
  mbath = std::get<5>(PB);
  mself = std::get<6>(PB);
  mtypes = std::get<1>(PB); for(auto&d: mtypes) for(auto& t: std::get<1>(LM)) d.push_back(t); if(mtypes.nelem() == 0) return;
  
  mdata.resize(mtypes.nelem());
  merrors = ArrayOfVector(mtypes.nelem());
  for(Index i=0; i<mtypes.nelem(); i++) {
    mdata[i].resize(std::get<2>(PB)[i].nelem() + std::get<2>(LM).nelem());
    merrors[i].resize(mdata[i].nelem());
    
    mdata[i][Range(0, std::get<2>(PB)[i].nelem())] = std::get<2>(PB)[i][Range(0, std::get<2>(PB)[i].nelem())];
    mdata[i][Range(std::get<2>(PB)[i].nelem(), joker)] = std::get<2>(LM);
    
    merrors[i][Range(0, std::get<3>(PB)[i].nelem())] = std::get<3>(PB)[i];
  }
}

//! {"X0", "X1", "X2"}
ArrayOfString all_coefficientsLineFunctionData() {return {"X0", "X1", "X2"};}

//! {"G0", "D0", "G2", "D2", "ETA", "FVC", "Y", "G", "DV"}
ArrayOfString all_variablesLineFunctionData() {return {"G0", "D0", "G2", "D2", "FVC", "ETA", "Y", "G", "DV"};}

JacPropMatType select_derivativeLineFunctionData(const String& var, const String& coeff)
{
  // Test viability of model variables
  static const ArrayOfString vars = all_variablesLineFunctionData();
  bool var_OK = false;
  for(auto& v: vars) if(var == v) var_OK = true;
  
  // Test viability of model coefficients
  static const ArrayOfString coeffs = all_coefficientsLineFunctionData();
  bool coeff_OK = false;
  for(auto& c: coeffs) if(coeff == c) coeff_OK = true;
  
  // Fails either when the user has bad input or when the developer fails to update all_variablesLineFunctionData or all_coefficientsLineFunctionData
  if(not var_OK or not coeff_OK) {
    std::ostringstream os;
    os << "At least one of your variable and/or your coefficient is not OK\n";
    os << "Your variable: \"" << var <<  "\".  OK variables include: " << vars << "\n";
    os << "Your coefficient: \"" << coeff <<  "\".  OK coefficients include: " << coeffs << "\n";
    throw std::runtime_error(os.str());
  }
  
  // Define a repetitive pattern.  Update if/when there are more coefficients in the future
  #define ReturnJacPropMatType(ID) \
  (var == #ID) {      if(coeff == "X0") return JacPropMatType::LineFunctionData ## ID ## X0; \
                 else if(coeff == "X1") return JacPropMatType::LineFunctionData ## ID ## X1; \
                 else if(coeff == "X2") return JacPropMatType::LineFunctionData ## ID ## X2; }

  
  if      ReturnJacPropMatType(G0 )
  else if ReturnJacPropMatType(D0 )
  else if ReturnJacPropMatType(G2 )
  else if ReturnJacPropMatType(D2 )
  else if ReturnJacPropMatType(FVC)
  else if ReturnJacPropMatType(ETA)
  else if ReturnJacPropMatType(Y  )
  else if ReturnJacPropMatType(G  )
  else if ReturnJacPropMatType(DV )
  
  #undef ReturnJacPropMatType
  
  // This piece of code cannot be reached during normal execution and will only be seen when
  // the developer has made a mistake in not updating parts of the code
  throw std::runtime_error("DEVELOPMENT ERROR: All three of\n"
                           "\tall_coefficientsLineFunctionData(),\n"
                           "\tall_variablesLineFunctionData(), and\n"
                           "\tselect_derivativeLineFunctionData(...)\n"
                           "needs to be updated to add a new derivative.\n"
                           "So do the JacPropMatType enum, but this function should fail if it is not updated.\n"
                           "Please check that these have all been updated accordingly.\n");
}


Numeric LineFunctionData::AirG0() const
{
  if(mp == LineShapeType::VP)
    if(mbath)
      if(mtypes[mtypes.nelem()-1][0] == TemperatureType::T1 and mtypes[mtypes.nelem()-1][1] == TemperatureType::T5)
        return mdata[mtypes.nelem()-1][0];
  throw std::runtime_error("Unavailable AIR computations for select line.  Please use legacy line catalogs for intended calculations.");
}


Numeric LineFunctionData::AirN() const
{
  if(mp == LineShapeType::VP)
    if(mbath)
      if(mtypes[mtypes.nelem()-1][0] == TemperatureType::T1 and mtypes[mtypes.nelem()-1][1] == TemperatureType::T5)
        if(mdata[mtypes.nelem()-1][1] == mdata[mtypes.nelem()-1][3])
          return mdata[mtypes.nelem()-1][1];
  throw std::runtime_error("Unavailable AIR computations for select line.  Please use legacy line catalogs for intended calculations.");
}


Numeric LineFunctionData::AirD0() const
{
  if(mp == LineShapeType::VP)
    if(mbath)
      if(mtypes[mtypes.nelem()-1][0] == TemperatureType::T1 and mtypes[mtypes.nelem()-1][1] == TemperatureType::T5)
        return mdata[mtypes.nelem()-1][2];
  throw std::runtime_error("Unavailable computations for select line.  Please use legacy line catalogs for intended calculations.");
}

Numeric LineFunctionData::SelfG0() const
{
  if(mp == LineShapeType::VP)
    if(mself)
      if(mtypes[0][0] == TemperatureType::T1 and mtypes[0][1] == TemperatureType::T5)
        return mdata[0][0];
  throw std::runtime_error("Unavailable SELF computations for select line.  Please use legacy line catalogs for intended calculations.");
}


Numeric LineFunctionData::SelfN() const
{
  if(mp == LineShapeType::VP)
    if(mself)
      if(mtypes[0][0] == TemperatureType::T1 and mtypes[0][1] == TemperatureType::T5)
        return mdata[0][1];
  throw std::runtime_error("Unavailable SELF computations for select line.  Please use legacy line catalogs for intended calculations.");
}


LineFunctionDataOutput LineFunctionData::AirBroadening(const Numeric& theta, const Numeric& P, const Numeric& self_vmr) const
{
  LineFunctionDataOutput t;
  
  const Numeric an = AirN();
  const Numeric aD0 = AirD0();
  const Numeric aG0 = AirG0();
  const Numeric sG0 = SelfG0();
  const Numeric sn = SelfN();
  
  t.G0 = P * (pow(theta, an) * (1 - self_vmr) * aG0 + pow(theta, sn) * self_vmr * sG0);
  t.D0 = P * pow(theta, 1.5 * an + 0.25) * aD0;
  return t;
}


Numeric LineFunctionData::dAirG0() const
{
  if(mp == LineShapeType::VP)
    if(mbath)
      if(mtypes[mtypes.nelem()-1][0] == TemperatureType::T1 and mtypes[mtypes.nelem()-1][1] == TemperatureType::T5)
        return merrors[mtypes.nelem()-1][0];
  throw std::runtime_error("Unavailable error computations for select line.  Please use legacy line catalogs for intended calculations.");
}


Numeric LineFunctionData::dAirN() const
{
  if(mp == LineShapeType::VP)
    if(mbath)
      if(mtypes[mtypes.nelem()-1][0] == TemperatureType::T1 and mtypes[mtypes.nelem()-1][1] == TemperatureType::T5)
        if(merrors[mtypes.nelem()-1][1] == merrors[mtypes.nelem()-1][3])
          return merrors[mtypes.nelem()-1][1];
  throw std::runtime_error("Unavailable error computations for select line.  Please use legacy line catalogs for intended calculations.");
}


Numeric LineFunctionData::dAirD0() const
{
  if(mp == LineShapeType::VP)
    if(mbath)
      if(mtypes[mtypes.nelem()-1][0] == TemperatureType::T1 and mtypes[mtypes.nelem()-1][1] == TemperatureType::T5)
        return merrors[mtypes.nelem()-1][2];
  throw std::runtime_error("Unavailable error computations for select line.  Please use legacy line catalogs for intended calculations.");
}

Numeric LineFunctionData::dSelfG0() const
{
  if(mp == LineShapeType::VP)
    if(mself)
      if(mtypes[0][0] == TemperatureType::T1 and mtypes[0][1] == TemperatureType::T5)
        return merrors[0][0];
  throw std::runtime_error("Unavailable error computations for select line.  Please use legacy line catalogs for intended calculations.");
}


Numeric LineFunctionData::dSelfN() const
{
  if(mp == LineShapeType::VP)
    if(mbath)
      if(mtypes[0][0] == TemperatureType::T1 and mtypes[0][1] == TemperatureType::T5)
        if(merrors[0][1] == merrors[0][3])
          return merrors[0][1];
  throw std::runtime_error("Unavailable error computations for select line.  Please use legacy line catalogs for intended calculations.");
}


Vector LineFunctionData::PlanetaryForeignG0() const
{
  Vector v(6);
  if(mspecies.nelem() == 7) {
    for(Index i=1; i < 7; i++)
      if(mtypes[i][0] == TemperatureType::T1)
        v[i-1] = mdata[i][0];
      else
        throw std::runtime_error("Wrong type for wanted conversion, use a modern line catalog format.");
  }
  else {
    ArrayOfSpeciesTag s({SpeciesTag("N2"), SpeciesTag("O2"), SpeciesTag("H2O"), SpeciesTag("CO2"), SpeciesTag("H2"), SpeciesTag("He")});
    for(Index i=0; i<s.nelem(); i++) {
      bool found = false;
      for(Index j=0; j<mspecies.nelem(); j++) if(mspecies[j] == s[i]) if(mtypes[j][0] == TemperatureType::T1) {v[i]=mdata[j][0]; found=true; break;}
      if(not found)
        throw std::runtime_error("Wrong type for wanted conversion, use a modern line catalog format.");
    }
  }
  return v;
}


Vector LineFunctionData::PlanetaryForeignD0() const
{
  Vector v(6);
  if(mspecies.nelem() == 7) {
    for(Index i=1; i < 7; i++)
      if(mtypes[i][0] == TemperatureType::T1)
        v[i-1] = mdata[i][2];
      else
        throw std::runtime_error("Wrong type for wanted conversion, use a modern line catalog format.");
  }
  else {
    ArrayOfSpeciesTag s({SpeciesTag("N2"), SpeciesTag("O2"), SpeciesTag("H2O"), SpeciesTag("CO2"), SpeciesTag("H2"), SpeciesTag("He")});
    for(Index i=0; i<s.nelem(); i++) {
      bool found = false;
      for(Index j=0; j<mspecies.nelem(); j++) if(mspecies[j] == s[i]) if(mtypes[j][0] == TemperatureType::T1) {v[i]=mdata[j][2]; found=true; break;}
      if(not found)
        throw std::runtime_error("Wrong type for wanted conversion, use a modern line catalog format.");
    }
  }
  return v;
}


Vector LineFunctionData::PlanetaryForeignN() const
{
  Vector v(6);
  if(mspecies.nelem() == 7) {
    for(Index i=1; i < 7; i++)
      if(mtypes[i][0] == TemperatureType::T1)
        v[i-1] = mdata[i][1];
      else
        throw std::runtime_error("Wrong type for wanted conversion, use a modern line catalog format.");
  }
  else {
    ArrayOfSpeciesTag s({SpeciesTag("N2"), SpeciesTag("O2"), SpeciesTag("H2O"), SpeciesTag("CO2"), SpeciesTag("H2"), SpeciesTag("He")});
    for(Index i=0; i<s.nelem(); i++) {
      bool found = false;
      for(Index j=0; j<mspecies.nelem(); j++) if(mspecies[j] == s[i]) if(mtypes[j][0] == TemperatureType::T1) {v[i]=mdata[j][1]; found=true; break;}
      if(not found)
        throw std::runtime_error("Wrong type for wanted conversion, use a modern line catalog format.");
    }
  }
  return v;
}


void LineFunctionData::ChangeLineMixing(const LineMixingOrderType lm, const Array<TemperatureType>& ts, const Vector& data)
{
  for(Index species=0; species<mspecies.nelem(); species++) {
    const Index current=LineShapeDataNelemForSpecies(species);
    
    // Count for new items
    Index temperature_count=0;
    for(auto i=0; i<ts.nelem(); i++)
      temperature_count += TemperatureTypeNelem(ts[i]);
    
    // Check for sanity
    if(data.nelem() not_eq temperature_count)
      throw std::runtime_error("Mismatch between data and temperature lengths.  Data must match request by temperature array");
    
    Vector newdata(current + temperature_count, 0);
    Array<TemperatureType> newtypes(LineShapeTypeNelem() + ts.nelem());
    
    // Old data and error exists so copy them
    for(auto i=0; i<current; i++)
      newdata[i] = mdata[species][i]; // Copy old
    
    // New data and error does not have to be same length
    for(auto i=0; i<data.nelem(); i++)
      newdata[current + i] = data[i]; // Copy new
    
    for(auto i=0; i<LineShapeTypeNelem(); i++)
      newtypes[i] = mtypes[species][i];  // Copy old
    for(auto i=0; i<ts.nelem(); i++)
      newtypes[LineShapeTypeNelem() + i] = ts[i];  // Copy new
    
    // Overwrite the old data
    mdata[species] = newdata;
    mtypes[species] = newtypes;
  }
  
  mlm = lm;
}

void LineFunctionData::Set(const Numeric& X, const String& species, const String& coefficient, const String& variable)
{
  const ArrayOfString coeffs = all_coefficientsLineFunctionData();
  const ArrayOfString vars = all_variablesLineFunctionData();
  
  Index ic=-1;
  for(Index i=0; i<coeffs.nelem(); i++)
    if(coefficient==coeffs[i])
      ic=i;
  const Index iv = IndexOfParam(variable);
  
  if(iv==-1 or ic==-1) {
    std::ostringstream os;
    os << "You have line shape type: " << LineShapeType2String() << "-" << LineMixingType2String() << '\n';
    os << "You want to set: " << species << " " << variable << " " << coefficient << '\n';
    os << "This combination is not possible because your line shape type does not support\n";
    os << "the variable, or because the coefficient is not available\n";
    
    throw std::runtime_error(os.str());
  }
  
  Index ispecies=-1;
  if(species == LineFunctionData_SelfBroadening and mself)
    ispecies = 0;
  else if(species == LineFunctionData_BathBroadening) {
    if(mbath)
      ispecies = mtypes.nelem()-1;
  }
  else
    for(Index i=0; i<mspecies.nelem(); i++) if(mspecies[i].IsSpecies(species)) {ispecies=i; break;}
  
  if(ispecies < 0 or iv<0 or TemperatureTypeNelem(mtypes[ispecies][iv]) <= ic) {
    std::ostringstream os;
    os << "The combination of " << species << " " << variable << " and " << coefficient << '\n';
    os << "is not available in " << LineShapeType2String() << "-" << LineMixingType2String() << '\n';
    os << "The reason is either that the species does not exist, that the variable is not\n";
    os << "defined for the type of line shape, or that the temperature type is bad\n";
    
    throw std::runtime_error(os.str());
  }
  
  Index current=0; for(Index i=0; i<iv; i++) current += TemperatureTypeNelem(mtypes[ispecies][i]);
  mdata[ispecies][current+ic] = X;
}


Numeric LineFunctionData::Get(const String& species, const String& coefficient, const String& variable) const
{
  const ArrayOfString coeffs = all_coefficientsLineFunctionData();
  const ArrayOfString vars = all_variablesLineFunctionData();
  
  Index ic=-1; for(Index i=0; i<coeffs.nelem(); i++) if(coefficient==coeffs[i]) ic=i;
  const Index iv = IndexOfParam(variable);
  
  if(iv==-1 or ic==-1) {
    std::ostringstream os;
    os << "You have line shape type: " << LineShapeType2String() << "-" << LineMixingType2String() << '\n';
    os << "You want to set: " << species << " " << variable << " " << coefficient << '\n';
    os << "This combination is not possible because your line shape type does not support\n";
    os << "the variable, or because the coefficient is not available\n";
    
    throw std::runtime_error(os.str());
  }
  
  Index ispecies=-1;
  if(species == LineFunctionData_SelfBroadening and mself)
    ispecies = 0;
  else if(species == LineFunctionData_BathBroadening) {
    if(mbath)
      ispecies = mtypes.nelem()-1;
  }
  else
    for(Index i=0; i<mspecies.nelem(); i++) if(mspecies[i].IsSpecies(species)) {ispecies=i; break;}
    
  if(ispecies < 0 or TemperatureTypeNelem(mtypes[ispecies][iv]) <= ic) {
    std::ostringstream os;
    os << "You have line shape type: " << LineShapeType2String() << "-" << LineMixingType2String() << '\n';
    os << "You want to set: " << species << " " << variable << " " << coefficient << '\n';
    os << "This combination is not possible because your line shape type does not support\n";
    os << "the variable, or because the coefficient is not available\n";
    
    throw std::runtime_error(os.str());
  }
  
  Index current=0;
  for(Index i=0; i<iv; i++)
    current += TemperatureTypeNelem(mtypes[ispecies][i]);
  return mdata[ispecies][current+ic];
}

LineFunctionDataOutput mirroredOutput(LineFunctionDataOutput v) noexcept
{
  v.D0 *= -1;
  v.D2 *= -1;
  v.DV *= -1;
  return v;
}

LineFunctionDataOutput si2cgs(LineFunctionDataOutput v)
{
  using Conversion::freq2kaycm;
  
  v.G0 = freq2kaycm(v.G0);
  v.D0 = freq2kaycm(v.D0);
  v.G2 = freq2kaycm(v.G2);
  v.D2 = freq2kaycm(v.D2);
  v.FVC = freq2kaycm(v.FVC);
  v.DV = freq2kaycm(v.DV);
  return v;
}

LineFunctionDataOutput cgs2si(LineFunctionDataOutput v)
{
  using Conversion::kaycm2freq;
  
  v.G0 = kaycm2freq(v.G0);
  v.D0 = kaycm2freq(v.D0);
  v.G2 = kaycm2freq(v.G2);
  v.D2 = kaycm2freq(v.D2);
  v.FVC = kaycm2freq(v.FVC);
  v.DV = kaycm2freq(v.DV);
  return v;
}

void LineFunctionData::Remove(Index i)
{
  if(i >= mdata.nelem())
    throw std::runtime_error("Trying to remove values that do not exist");

  if(mself and i==0)
    mself=false;

  if(mbath and i==(mdata.nelem()-1))
    mbath=false;

  if(merrors.nelem() == mdata.nelem())  // Is this available?  Then remove, otherwise ignore.
    merrors.erase(merrors.begin()+i);
  mdata.erase(mdata.begin()+i);
  mtypes.erase(mtypes.begin()+i);
  mspecies.erase(mspecies.begin()+i);
}
