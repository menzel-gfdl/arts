/* Copyright (C) 2003-2008 Patrick Eriksson <Patrick.Eriksson@chalmers.se>

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



/*===========================================================================
  ===  File description 
  ===========================================================================*/

/*!
  \file   m_refraction.cc
  \author Patrick Eriksson <Patrick.Eriksson@chalmers.se>
  \date   2003-01-09

  \brief  Workspace methods releated to refraction.

  These functions are listed in the doxygen documentation as entries of the
  file auto_md.h.
*/



/*===========================================================================
  === External declarations
  ===========================================================================*/

#include <cmath>
#include "absorption.h"
#include "arts.h"
#include "check_input.h"
#include "math_funcs.h"
#include "matpackI.h"
#include "messages.h"
#include "refraction.h"
#include "special_interp.h"
#include "abs_species_tags.h"

extern const Numeric ELECTRON_CHARGE;
extern const Numeric ELECTRON_MASS;
extern const Numeric PI;
extern const Numeric VACUUM_PERMITTIVITY;

/*===========================================================================
  === WSMs
  ===========================================================================*/


/* Workspace method: Doxygen documentation will be auto-generated */
void refr_indexFreeElectrons(
          Numeric&   refr_index,
          Numeric&   refr_index_group,
    const Vector&    f_grid,
    const Index&     f_index,
    const Numeric&   rte_edensity,
    const Verbosity&)
{
  // The expression used in found in many textbooks, e.g. Rybicki and Lightman
  // (1979). Note that the refractive index corresponds to the phase velocity.

  static const Numeric k = ELECTRON_CHARGE * ELECTRON_CHARGE / 
                         ( VACUUM_PERMITTIVITY * ELECTRON_MASS * 4 * PI * PI );

  if( rte_edensity > 0 )
    {
      Numeric f;
      if( f_index < 0 )
        { f = ( f_grid[0] + last(f_grid) ) / 2.0; }
      else
        { f = f_grid[f_index]; }

      const Numeric a = rte_edensity*k/(f*f);

      if( a > 0.25 ) 
        {
          ostringstream os;
          os << "The frequency must at least be twice the plasma frequency.\n"
             << "For this particular point, the plasma frequency is: " 
             << sqrt(rte_edensity*k)/1e6 << " MHz.";
          throw runtime_error( os.str() );
        }

      const Numeric n = sqrt( 1 - a );

      refr_index       += n - 1;
      refr_index_group += 1/n - 1;
    }
}



/* Workspace method: Doxygen documentation will be auto-generated */
void refr_indexIR(
          Numeric&   refr_index,
          Numeric&   refr_index_group,
    const Numeric&   rte_pressure,
    const Numeric&   rte_temperature,
    const Verbosity&)
{
  static const Numeric bn0  = 1.000272620045304;
  static const Numeric bn02 = bn0*bn0;
  static const Numeric bk   = 288.16 * (bn02-1.0) / (1013.25*(bn02+2.0));

  // Pa -> hPa
  const Numeric n = sqrt( (2.0*bk*rte_pressure/100.0+rte_temperature) / 
                          ( rte_temperature-bk*rte_pressure/100.0) ) - 1; 

  refr_index       += n;
  refr_index_group += n;
}



/* Workspace method: Doxygen documentation will be auto-generated */
void refr_indexThayer(
          Numeric&   refr_index,
          Numeric&   refr_index_group,
    const Numeric&   rte_pressure,
    const Numeric&   rte_temperature,
    const Vector&    rte_vmr_list,
    const ArrayOfArrayOfSpeciesTag& abs_species,
    const Verbosity& )
{
  if( abs_species.nelem() != rte_vmr_list.nelem() )
    throw runtime_error( "The number of tag groups differ between "
                                         "*rte_vmr_list* and *abs_species*." );

  Index   firstH2O = find_first_species_tg( abs_species,
                                      species_index_from_species_name("H2O") );

  if( firstH2O < 0 )
    throw runtime_error(
       "Water vapour is a required (must be a tag group in *abs_species*)." );

  const Numeric   e = rte_pressure * rte_vmr_list[firstH2O];

  const Numeric n = ( 77.6e-8 * ( rte_pressure - e ) + 
             ( 64.8e-8 + 3.776e-3 / rte_temperature ) * e ) / rte_temperature;

  refr_index       += n;
  refr_index_group += n;
}
