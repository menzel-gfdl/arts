/* Copyright (C) 2002-2008 Stefan Buehler <sbuehler@ltu.se>

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

/*!
  \file   gas_abs_lookup.cc
  \author Stefan Buehler <sbuehler@ltu.se>
  \date   Thu Sep 19 17:25:07 2002
  
  \brief  Implementation of scalar gas absorption lookup table functions.
*/

#include <cmath>
#include "gas_abs_lookup.h"
#include "interpolation.h"
#include "interpolation_poly.h"
#include "make_vector.h"
#include "logic.h"
#include "check_input.h"
#include "messages.h"
#include "physics_funcs.h"

//! Find positions of new grid points in old grid.
/*! 
  Throw a runtime error if the frequencies of the new grid are not
  found in the old grid. 

  Comparison of Numerics is a bit tricky, we use a tolerance of 1Hz here.

  \retval pos      Positions of new grid points in old grid.
  \param  old_grid The old grid.
  \param  new_grid The new grid.
*/
void find_new_grid_in_old_grid( ArrayOfIndex& pos,
                                ConstVectorView old_grid,
                                ConstVectorView new_grid,
                                const Verbosity& verbosity )
{
  CREATE_OUT3
  
  const Index n_new_grid = new_grid.nelem();
  const Index n_old_grid = old_grid.nelem();

  // Make sure that pos has the right size:
  assert( n_new_grid == pos.nelem() );

  // The frequency difference in Hz that we are willing to tolerate. 1
  // Hz seems to be on the safe side.
  const Numeric tolerance = 1;

  // Old grid position:
  Index j = 0;

  // Loop the new frequencies:
  for ( Index i=0; i<n_new_grid; ++i )
    {
      // We have done runtime checks that both the new and the old
      // frequency gris are sorted in GasAbsLookup::Adapt, so we can
      // use the fact here.

      while ( abs(new_grid[i]-old_grid[j]) > tolerance )
        {
          ++j;
          if ( j>=n_old_grid )
            {
              ostringstream os;
              os << "Cannot find new frequency " << i
                 << " (" << new_grid[i] << "Hz) in the lookup table frequency grid.";
              throw runtime_error( os.str() );
            }
        }

      pos[i] = j;
      out3 << "    " << new_grid[i] << " found, index = " << pos[i] << ".\n";
    }
}

//! Adapt lookup table to current calculation.
/*!
  This method has the following tasks:

  1. Find and remember the indices of the current species in the
  lookup table. At the same time verify that each species is included
  in the table exactly once.

  2. Find and remember the frequencies of the current calculation in
  the lookup table. At the same time verify that all frequencies are
  included and that no frequency occurs twice.

  3. Use the species and frequency index lists to build the new lookup
  table.

  4. Replace original table by the new one.

  5. Initialize log_p_grid.

  The method is intended to be called only once per ARTS job, more or
  less directly from a corresponding workspace method. Therefore,
  runtime errors are thrown, rather than assertions, if something is
  wrong. 

  \param current_species The list of species for the current calculation.
  \param current_f_grid  The list of frequencies for the current calculation.

  \date 2002-12-12
*/
void GasAbsLookup::Adapt( const ArrayOfArrayOfSpeciesTag& current_species,
                          ConstVectorView current_f_grid,
                          const Verbosity& verbosity )
{
  CREATE_OUT2
  CREATE_OUT3
  
  // Some constants we will need:
  const Index n_current_species = current_species.nelem();
  const Index n_current_f_grid  = current_f_grid.nelem();

  const Index n_species         = species.nelem();
  const Index n_nls             = nonlinear_species.nelem();
  const Index n_nls_pert        = nls_pert.nelem();
  const Index n_f_grid          = f_grid.nelem();
  const Index n_p_grid          = p_grid.nelem();
  
  out2 << "  Original table: " << n_species << " species, "
       << n_f_grid << " frequencies.\n"
       << "  Adapt to:       " << n_current_species << " species, "
       << n_current_f_grid << " frequencies.\n";

  if ( 0 == n_nls )
    {
      out2 << "  Table contains no nonlinear species.\n";
    }

  // Set up a logical array for the nonlinear species
  ArrayOfIndex non_linear(n_species,0);
  for ( Index s=0; s<n_nls; ++s )
    {
      non_linear[nonlinear_species[s]] = 1;
    }

  if ( 0 == t_pert.nelem() )
    {
      out2 << "  Table contains no temperature perturbations.\n";
    }

  // We are constructing a new lookup table, containing just the
  // species and frequencies that are necessary for the current
  // calculation. We will build it in this local variable, then copy
  // it back to *this in the end.
  GasAbsLookup new_table;

  // First some checks on the lookup table itself:

  // Species:
  if ( 0 == n_species )
    {
      ostringstream os;
      os << "The lookup table should have at least one species.";
      throw runtime_error( os.str() );
    }
  
  // Nonlinear species:
  // They should be unique ...
  if ( !is_unique(nonlinear_species) )
    {
      ostringstream os;
      os << "The table must not have duplicate nonlinear species.\n"
         << "Value of *nonlinear_species*: " << nonlinear_species;
      throw runtime_error( os.str() );
    }

  // ... and pointing at valid species. 
  for ( Index i=0; i<n_nls; ++i )
    {
      ostringstream os;
      os << "nonlinear_species[" << i << "]";
      chk_if_in_range( os.str(),
                       nonlinear_species[i],
                       0,
                       n_species-1 );
    }

  // Frequency grid:
  chk_if_increasing( "f_grid", f_grid );

  // Pressure grid:
  chk_if_decreasing( "p_grid", p_grid );

  // Reference VMRs:
  chk_matrix_nrows( "vmrs_ref", vmrs_ref, n_species );
  chk_matrix_ncols( "vmrs_ref", vmrs_ref, n_p_grid );

  // Reference temperatur:
  chk_vector_length( "t_ref", t_ref, n_p_grid );

  // Temperature perturbations:
  // Nothing to check for t_pert, it seems.

  // Perturbations for nonlinear species:
  // Check that nls_pert is empty if and only if nonlinear_species is
  // empty:
  if ( 0 == n_nls )
    {
      chk_vector_length( "nls_pert", nls_pert, 0 );
    }
  else
    {
      if ( 0 == n_nls_pert )
        {
          ostringstream os;
          os << "The vector nls_pert should contain the perturbations\n"
             << "for the nonlinear species, but it is empty.";
          throw runtime_error( os.str() );
        }
    }

  // The table itself, xsec:
  //
  // We have to separtely consider the 3 cases described in the
  // documentation of GasAbsLookup.
  //
  //     Dimension: [ a, b, c, d ]
  //
  if ( 0==n_nls )
    {
      if ( 0==t_pert.nelem() )
        {
          //     Simplest case (no temperature perturbations,
          //     no vmr perturbations):
          //     a = 1
          //     b = n_species
          //     c = n_f_grid 
          //     d = n_p_grid
          chk_size( "xsec", xsec,
                    1,
                    n_species,
                    n_f_grid,
                    n_p_grid );
        }
      else
        {
          //     Standard case (temperature perturbations,
          //     but no vmr perturbations):
          //     a = n_t_pert
          //     b = n_species
          //     c = n_f_grid
          //     d = n_p_grid
          chk_size( "xsec", xsec,
                    t_pert.nelem(),
                    n_species,
                    n_f_grid,
                    n_p_grid );
        }
    }
  else
    {
      //     Full case (with temperature perturbations and
      //     vmr perturbations):
      //     a = n_t_pert
      //     b = n_species + n_nonlinear_species * ( n_nls_pert - 1 )
      //     c = n_f_grid
      //     d = n_p_grid
      Index a = t_pert.nelem();
      Index b = n_species
        + n_nls * ( n_nls_pert - 1 );
      Index c = n_f_grid;
      Index d = n_p_grid;

      chk_size( "xsec", xsec, a, b, c, d );
    }

  // We also need indices to the positions of the original species
  // data in xsec. Nonlinear species take more space, therefor the
  // position in xsec is not the same as the position in species.
  ArrayOfIndex original_spec_pos_in_xsec(n_species);
  for (Index i=0,sp=0; i<n_species; ++i)
    {
      original_spec_pos_in_xsec[i] = sp;
      if (non_linear[i]) sp += n_nls_pert;
      else               sp += 1;
    }



  // Now some checks on the input data:

  // The list of current species should not be empty:
  if ( 0==n_current_species )
    {
      ostringstream os;
      os << "The list of current species should not be empty.";
      throw runtime_error( os.str() );
    }

  // The grid of current frequencies should be monotonically sorted:
  chk_if_increasing( "current_f_grid", current_f_grid );


  // 1. Find and remember the indices of the current species in the
  //    lookup table. At the same time verify that each species is
  //    included in the table exactly once.
  ArrayOfIndex i_current_species(n_current_species);
  out3 << "  Looking for species in lookup table:\n";
  for ( Index i=0; i<n_current_species; ++i )
    {
      out3 << "  " << get_tag_group_name( current_species[i] ) << ": ";
      // We need no error checking for the next statement, since the
      // chk_contains function throws a runtime error if the species
      // is not found exactly once.
      i_current_species[i] =
        chk_contains( "species", species, current_species[i] );
      out3 << "found.\n";
    }

  // 1a. Find out which of the current species are nonlinear species:
  Index n_current_nonlinear_species = 0;        // Number of current nonlinear species
  ArrayOfIndex current_non_linear(n_current_species,0); // A logical array to
                                                        // flag which of the
                                                        // current species are
                                                        // nonlinear.

  out3 << "  Finding out which of the current species are nonlinear:\n";
  for ( Index i=0; i<n_current_species; ++i )
    {
      // Check if this is a nonlinear species:
      if (non_linear[i_current_species[i]])
        {
          out3 << "  " << get_tag_group_name( current_species[i] ) << "\n";

          current_non_linear[i] = 1;
          ++n_current_nonlinear_species;
        }
    }

  // 2. Find and remember the frequencies of the current calculation in
  //    the lookup table. At the same time verify that all frequencies are
  //    included and that no frequency occurs twice.

  // FIXME: This is a bit tricky, because we are comparing
  // Numerics. Let's see how well this works in practice.

  ArrayOfIndex i_current_f_grid(n_current_f_grid);
  out3 << "  Looking for Frequencies in lookup table:\n";

  // We need no error checking for the next statement, since the
  // function called throws a runtime error if a frequency
  // is not found, or if the grids are not ok.
  find_new_grid_in_old_grid( i_current_f_grid,
                             f_grid,
                             current_f_grid,
                             verbosity );


  // 3. Use the species and frequency index lists to build the new lookup
  // table.

  // Species:
  new_table.species.resize( n_current_species );
  for ( Index i=0; i<n_current_species; ++i )
    {
      new_table.species[i] = species[i_current_species[i]];

      // Is this a nonlinear species?
      if (current_non_linear[i])
        new_table.nonlinear_species.push_back( i );
    }

  // Frequency grid:
  new_table.f_grid.resize( n_current_f_grid );
  for ( Index i=0; i<n_current_f_grid; ++i )
    {
      new_table.f_grid[i] = f_grid[i_current_f_grid[i]];
    }

  // Pressure grid:
  //  new_table.p_grid.resize( n_p_grid ); 
  new_table.p_grid = p_grid;

  // Reference VMR profiles:
  new_table.vmrs_ref.resize( n_current_species,
                             n_p_grid     );
  for ( Index i=0; i<n_current_species; ++i )
    {
      new_table.vmrs_ref( i,
                          Range(joker) )
        = vmrs_ref( i_current_species[i],
                    Range(joker)          );
    }

  // Reference temperature profile:
  //  new_table.t_ref.resize( t_ref.nelem() );
  new_table.t_ref = t_ref;

  // Vector of temperature perturbations:
  //  new_table.t_pert.resize( t_pert.nelem() );
  new_table.t_pert = t_pert;

  // Vector of perturbations for the VMRs of the nonlinear species: 
  // (Should stay empty if we have no nonlinear species)
  if ( 0 != new_table.nonlinear_species.nelem() )
    {
      //      new_table.nls_pert.resize( n_nls_pert );
      new_table.nls_pert = nls_pert;
    }

  // Absorption coefficients:
  new_table.xsec.resize( xsec.nbooks(),
                         n_current_species+n_current_nonlinear_species*(n_nls_pert-1),
                         n_current_f_grid,
                         xsec.ncols()        );

  // We have to copy the right species and frequencies from the old to
  // the new table. Temperature perturbations and pressure grid remain
  // the same.

  // Do species:
  for ( Index i_s=0,sp=0; i_s<n_current_species; ++i_s )
    {
      // n_v is the number of VMR perturbations
      Index n_v;
      if (current_non_linear[i_s])
        n_v = n_nls_pert;
      else
        n_v = 1;

//      cout << "i_s / sp / n_v = " << i_s << " / " << sp << " / " << n_v << endl;
//      cout << "orig_pos = " << original_spec_pos_in_xsec[i_current_species[i_s]] << endl;

      // Do frequencies:
      for ( Index i_f=0; i_f<n_current_f_grid; ++i_f )
        {
          new_table.xsec( Range(joker),
                          Range(sp,n_v),
                          i_f,
                          Range(joker) )
            =
            xsec( Range(joker),
                  Range(original_spec_pos_in_xsec[i_current_species[i_s]],n_v),
                  i_current_f_grid[i_f],
                  Range(joker) );

//           cout << "result: " << xsec( Range(joker),
//                                       Range(original_spec_pos_in_xsec[i_current_species[i_s]],n_v),
//                                       i_current_f_grid[i_f],
//                                       Range(joker) ) << endl;

        }

      sp += n_v;
    }


  // 4. Replace original table by the new one.
  *this = new_table;

  // 5. Initialize log_p_grid.
  log_p_grid.resize( n_p_grid );
  transform( log_p_grid,
             log,
             p_grid );
}

//! Extract scalar gas absorption coefficients from the lookup table. 
/*!  
  This carries out a simple interpolation in temperature and
  pressure. The interpolated value is then scaled by the ratio between
  actual VMR and reference VMR. In the case of nonlinear species the
  interpolation goes also over H2O VMR.

  All input parameters (f_index, p, T, VMRs for non-linear species)
  must be in the range coverd by the table. Violation will result in a
  runtime error. Those checks are here, because they are a bit
  difficult to make outside, due to the irregularity of the
  grids. Otherwise there are no runtime checks in this function, only
  assertions. This is, because the function is called many times
  inside the RT calculation.

  In this case pressure is not an altitude coordinate, so we are free
  to choose the type of interpolation that gives lowest interpolation
  errors or is easiest. I tested both linear and log p interpolation
  with the result that log p interpolation is slightly better, so that
  is used.

  \retval sga A Matrix with scalar gas absorption coefficients
  [1/m]. Dimension is adjusted automatically to either
  [1,n_species] or [n_f_grid,n_species]!

  \param f_index The frequency index. If this is >=0, it means that
  absorption for this frequency will be extracted. (The leading
  dimension of sga will be 1.) If this is <0, it means that absorption
  for ALL frequencies is extracted. (The leading dimension of sga will
  be n_f_grid.)

  \param p The pressures [Pa].

  \param T The temperature [K].

  \param abs_vmrs The VMRs [absolute number]. Dimension: [species].  

  \date 2002-09-20, 2003-02-22, 2007-05-22

  \author Stefan Buehler
*/
void GasAbsLookup::Extract( Matrix&         sga,
                            const Index&    p_interp_order,
                            const Index&    t_interp_order,
                            const Index&    h2o_interp_order,
                            const Index&    f_index,
                            const Numeric&  p,
                            const Numeric&  T,
                            ConstVectorView abs_vmrs ) const
{
  // 1. Obtain some properties of the lookup table:
  
  // Number of gas species in the table:
  const Index n_species = species.nelem();

  // Number of nonlinear species:
  const Index n_nls = nonlinear_species.nelem();

  // Number of frequencies in the table:
  const Index n_f_grid = f_grid.nelem();

  // Number of pressure grid points in the table:
  const Index n_p_grid = p_grid.nelem();

  // Number of temperature perturbations:
  const Index n_t_pert = t_pert.nelem();

  // Number of nonlinear species perturbations:
  const Index n_nls_pert = nls_pert.nelem();


  // 2. First some checks on the lookup table itself:

  // Most checks here are asserts, because they check the internal
  // consistency of the lookup table. They should never fail if the
  // table has been created with ARTS.

  // If there are nonlinear species, then at least one species must be
  // H2O. We will use that to perturb in the case of nonlinear
  // species.
  Index h2o_index = -1;
  if (n_nls>0)
    {
      h2o_index = find_first_species_tg( species,
                                         species_index_from_species_name("H2O") );

      // This is a runtime error, even though it would be more logical
      // for it to be an assertion, since it is an internal check on
      // the table. The reason is that it is somewhat awkward to check
      // for this in other places.
      if ( h2o_index == -1 )
        {
          ostringstream os;
          os << "With nonlinear species, at least one species must be a H2O species.";
          throw runtime_error( os.str() );
        }
    }

  // Check that the dimension of vmrs_ref is consistent with species and p_grid:
  assert( is_size( vmrs_ref, n_species, n_p_grid) );

  // Check dimension of t_ref:
  assert( is_size( t_ref, n_p_grid) );

  // Check dimension of xsec:
  DEBUG_ONLY({
      Index a,b,c,d;
      if ( 0 == n_t_pert ) a = 1;
      else a = n_t_pert;
      b = n_species + n_nls * ( n_nls_pert - 1 );
      c = n_f_grid;
      d = n_p_grid;
//       cout << "xsec: "
//            << xsec.nbooks() << ", "
//            << xsec.npages() << ", "
//            << xsec.nrows() << ", "
//            << xsec.ncols() << "\n";
//       cout << "a b c d: "
//            << a << ", "
//            << b << ", "
//            << c << ", "
//            << d << "\n";
      assert( is_size( xsec, a, b, c, d ) );
    })

  // Make sure that log_p_grid is initialized:
  if (log_p_grid.nelem() != n_p_grid)
    {
      ostringstream os;
      os << "The lookup table internal variable log_p_grid is not initialized.\n"
         << "Use the abs_lookupAdapt method!";
      throw runtime_error( os.str() );
    }

  // Verify that we have enough pressure, temperature and humdity grid points
  // for the desired interpolation orders. This check is not only
  // table internal, since abs_nls_interp_order and abs_t_interp_order
  // are separate WSVs that could have been modified. Hence, these are
  // runtime errors.

  if ( (n_p_grid < p_interp_order+1) )
    {
      ostringstream os;
      os << "The number of pressure grid points in the table ("
         << n_p_grid << ") is not enough for the desired order of interpolation ("
         << p_interp_order << ").";
      throw runtime_error( os.str() );
    }

  if ( (n_nls_pert != 0) && (n_nls_pert < h2o_interp_order+1) )
    {
      ostringstream os;
      os << "The number of humidity perturbation grid points in the table ("
         << n_nls_pert << ") is not enough for the desired order of interpolation ("
         << h2o_interp_order << ").";
      throw runtime_error( os.str() );
    }

  if ( (n_t_pert != 0) && (n_t_pert < t_interp_order+1) )
    {
      ostringstream os;
      os << "The number of temperature perturbation grid points in the table ("
         << n_t_pert << ") is not enough for the desired order of interpolation ("
         << t_interp_order << ").";
      throw runtime_error( os.str() );
    }


  // 3. Checks on the input variables:

  // Assert that abs_vmrs has the right dimension:
  if ( !is_size( abs_vmrs, n_species ) )
    {
      ostringstream os;
      os << "Number of species in lookup table does not match number\n"
         << "of species for which you want to extract absorption.\n"
         << "Have you used abs_lookupAdapt?";
      throw runtime_error( os.str() );
    }


  // 4. Set up some things we will need later on: 

  // Set the start and extent for the frequency loop:
  Index f_start, f_extent;
  if ( f_index < 0 )
    {
      // This means we should extract for all frequencies.

      f_start  = 0;
      f_extent = n_f_grid;
    }
  else
    {
      // This means we should extract only for one frequency.

      // Check that f_index is inside f_grid:
      if ( f_index >= n_f_grid )
      {
        ostringstream os;
        os << "Problem with gas absorption lookup table.\n"
           << "Frequency index f_index is too high, you have " << f_index
           << ", the largest allowed value is " << n_f_grid-1 << ".";
        throw runtime_error( os.str() );
      }
      
      f_start  = f_index;
      f_extent = 1;
    }
  const Range f_range(f_start,f_extent);


  // Set up a logical array for the nonlinear species
  ArrayOfIndex non_linear(n_species,0);
  for ( Index s=0; s<n_nls; ++s )
    {
      non_linear[nonlinear_species[s]] = 1;
    }


  // Calculate the number density for the given pressure and
  // temperature: 
  // n = n0*T0/p0 * p/T or n = p/kB/t, ideal gas law
  const Numeric n = number_density( p, T );

  
  // 5. Determine pressure grid position and interpolation weights:

  // Check that p is inside the grid. (p_grid is sorted in decreasing order.)
  {
    const Numeric p_max = p_grid[0] + 0.5*(p_grid[0]-p_grid[1]); 
    const Numeric p_min = p_grid[n_p_grid-1] - 0.5*(p_grid[n_p_grid-2]-p_grid[n_p_grid-1]);
    if ( ( p > p_max ) ||
         ( p < p_min ) )
    {
      ostringstream os;
      os << "Problem with gas absorption lookup table.\n"
         << "Pressure p is outside the range covered by the lookup table.\n" 
         << "Your p value is " << p << " Pa.\n"
         << "The allowed range is " << p_min << " to " << p_max << ".\n"
         << "The pressure grid range in the table is " << p_grid[n_p_grid-1]
         << " to " << p_grid[0] << ".\n"
         << "We allow a bit of extrapolation, but NOT SO MUCH!";
      throw runtime_error( os.str() );
    }
  }
  

  // For sure, we need to store the pressure grid position. 
  // We do the interpolation in log(p). Test have shown that this
  // gives slightly better accuracy than interpolating in p directly. 
  ArrayOfGridPosPoly pgp(1);
  gridpos_poly( pgp,
                log_p_grid,
                log(p),
                p_interp_order );

  // Pressure interpolation weights:
  Vector pitw;
  pitw.resize(p_interp_order+1);
  interpweights(pitw,pgp[0]);


  // 6. We do the T and VMR interpolation for the pressure levels
  // that are used in the pressure interpolation. (How many depends on
  // p_interp_order.)  
  
  // To store the interpolated result for the p_interp_order+1
  // pressure levels:
  Tensor3 xsec_pre_interpolated;
  xsec_pre_interpolated.resize( p_interp_order+1, f_extent, n_species );

  for ( Index pi=0; pi<p_interp_order+1; ++pi )
    {
      // Index into p_grid:
      const Index this_p_grid_index = pgp[0].idx[pi];

      // Flag for temperature interpolation, if this is not 0 we want
      // to do T interpolation: 
      const Index do_T = n_t_pert;

      // Determine temperature grid position. This is only done if we
      // want temperature interpolation, but the variable tgp has to
      // be visible also outside for later use:
      ArrayOfGridPosPoly tgp(1);       // only a scalar
      if (do_T)
        {
            
          // Temperature in the atmosphere is altitude
          // dependent. When we do the interpolation for the pressure level
          // below and above our point, we should correct the target value of
          // the interpolation to the altitude (pressure) difference. This
          // ensures that there is for example no T interpolation if the
          // desired T is right on the reference profile curve.
          //
          // I explicitly compared this with the old option to calculate
          // the temperature offset relative to the temperature at
          // this level. The performance in both cases is very
          // similar. The reason, why I decided to keep this new
          // version, is that it avoids the problem of needing
          // oversized temperature perturbations if the pressure
          // grid is coarse.
          //
          // No! The above approach leads to problems when combined with
          // higher order pressure interpolation. The problem is that
          // the reference T and VMR profiles may be very
          // irregular. (For example the H2O profile often has a big
          // jump near the bottom.) That sometimes leads to negative
          // effective reference values when the reference profile is
          // interpolated. I therefore reverted back to the original
          // version of using the real temperature and humidity, not
          // the interpolated one.

          //          const Numeric effective_T_ref = interp(pitw,t_ref,pgp);
          const Numeric effective_T_ref = t_ref[this_p_grid_index];

          // Convert temperature to offset from t_ref:
          const Numeric T_offset = T - effective_T_ref;

          //          cout << "T_offset = " << T_offset << endl;

          // Check that temperature offset is inside the allowed range.
          {
            const Numeric t_min = t_pert[0] - 0.5*(t_pert[1]-t_pert[0]); 
            const Numeric t_max = t_pert[n_t_pert-1] + 0.5*(t_pert[n_t_pert-1]-t_pert[n_t_pert-2]);
            if ( ( T_offset > t_max ) ||
                 ( T_offset < t_min ) )
            {
              ostringstream os;
              os << "Problem with gas absorption lookup table.\n"
                 << "Temperature T is outside the range covered by the lookup table.\n" 
                 << "Your temperature was " << T
                 << " K at a pressure of " << p << " Pa.\n"
                 << "The temperature offset value is " << T_offset << ".\n"
                 << "The allowed range is " << t_min << " to " << t_max << ".\n"
                 << "The temperature perturbation grid range in the table is "
                 << t_pert[0] << " to " << t_pert[n_t_pert-1] << ".\n"
                 << "We allow a bit of extrapolation, but NOT SO MUCH!";
                throw runtime_error( os.str() );
            }
          }

          gridpos_poly( tgp, t_pert, T_offset, t_interp_order );
        }

      // Determine the H2O VMR grid position. We need to do this only
      // once, since the only species who's VMR is interpolated is
      // H2O. We do this only if there are nonlinear species, but the
      // variable has to be visible later.
      ArrayOfGridPosPoly vgp(1);       // only a scalar
      if (n_nls>0)
        {
          // Similar to the T case, we first interpolate the reference
          // VMR to the pressure of extraction, then compare with
          // the extraction VMR to determine the offset/fractional
          // difference for the VMR interpolation.
          //
          // No! The above approach leads to problems when combined with
          // higher order pressure interpolation. The problem is that
          // the reference T and VMR profiles may be very
          // irregular. (For example the H2O profile often has a big
          // jump near the bottom.) That sometimes leads to negative
          // effective reference values when the reference profile is
          // interpolated. I therefore reverted back to the original
          // version of using the real temperature and humidity, not
          // the interpolated one.
          
//           const Numeric effective_vmr_ref = interp(pitw,
//                                                    vmrs_ref(h2o_index, Range(joker)),
//                                                    pgp);
          const Numeric effective_vmr_ref = vmrs_ref(h2o_index, this_p_grid_index);

              
          // Fractional VMR:
          const Numeric VMR_frac = abs_vmrs[h2o_index] / effective_vmr_ref;

          // Check that VMR_frac is inside the allowed range.
          {
            // FIXME: This check depends on how I interpolate VMR.
            const Numeric x_min = nls_pert[0] - 0.5*(nls_pert[1]-nls_pert[0]); 
            const Numeric x_max = nls_pert[n_nls_pert-1]
              + 0.5*(nls_pert[n_nls_pert-1]-nls_pert[n_nls_pert-2]);

            if ( ( VMR_frac > x_max ) ||
                 ( VMR_frac < x_min ) )
              {
                ostringstream os;
                os << "Problem with gas absorption lookup table.\n"
                   << "VMR for H2O (species " << h2o_index
                   << ") is outside the range covered by the lookup table.\n" 
                   << "Your VMR was " << abs_vmrs[h2o_index]
                   << " at a pressure of " << p << " Pa.\n"
                   << "The reference VMR value there is " << effective_vmr_ref << "\n"
                   << "The fractional VMR relative to the reference value is "
                   << VMR_frac << ".\n"
                   << "The allowed range is " << x_min << " to " << x_max << ".\n"
                   << "The fractional VMR perturbation grid range in the table is "
                   << nls_pert[0] << " to " << nls_pert[n_nls_pert-1] << ".\n"
                   << "We allow a bit of extrapolation, but NOT SO MUCH!";
                throw runtime_error( os.str() );
              }
          }

          // For now, do linear interpolation in the fractional VMR.
          gridpos_poly( vgp, nls_pert, VMR_frac, h2o_interp_order );
        }

      // 7. Loop species:
      Index fpi=0;
      for ( Index si=0; si<n_species; ++si )
        {
          // Flag for VMR interpolation, if this is not 0 we want to
          // do VMR interpolation:
          const Index do_VMR = non_linear[si];

          // We now have everything that we need to handle all
          // different interpolation cases.

          // 8. Do the interpolation in T and/or VMR. Here are
          // handlers for 4 different cases (all possible
          // combinations). 

          // For interpolation result.
          // Fixed pressure level and species.
          // Free dimension is frequency.
          VectorView res(xsec_pre_interpolated(pi,Range(joker),si));

          if (do_T)
            if (do_VMR)
              {
                // With T and VMR

                // This is a "red" 2D interpolation case.

                Vector itw;
                itw.resize( (t_interp_order+1)*
                            (h2o_interp_order+1) );
                
                interpweights(itw,tgp[0],vgp[0]);

                // Get the right view on xsec:
                ConstTensor3View this_xsec
                  = xsec( Range(joker),          // Temperature range
                          Range(fpi,n_nls_pert), // VMR profile range
                          f_range,               // Frequency range
                          this_p_grid_index );   // Pressure index 

                // We must do the same interpolation for all
                // frequencies. Instead of playing with interp and
                // Views and looping the whole thing over frequency, I
                // choose to do this explicitly here, but directly for
                // all frequencies. This should be much more efficient.

                // Initialize result to zero:
                res = 0;
                Index iti = 0;
                for ( Index r=0; r<t_interp_order+1; ++r )
                  for ( Index c=0; c<h2o_interp_order+1; ++c )
                    {
                      for ( Index f=0; f<f_extent; ++f )
                        res[f] += itw[iti] * this_xsec( tgp[0].idx[r], vgp[0].idx[c], f );
                      
                      ++iti;
                    }
              }
            else
              {
                // With T no VMR

                // This is a "red" 1D interpolation case.

                Vector itw;
                itw.resize(t_interp_order+1);
                
                interpweights(itw,tgp[0]);

                // Get the right view on xsec:
                ConstMatrixView this_xsec
                  = xsec( Range(joker),          // Temperature range
                          fpi,                   // the matching species
                          f_range,               // Frequency range
                          this_p_grid_index );   // Pressure index 

                // We must do the same interpolation for all
                // frequencies. Instead of playing with interp and
                // Views and looping the whole thing over frequency, I
                // choose to do this explicitly here, but directly for
                // all frequencies. This should be much more efficient.

                // Initialize result to zero:
                res = 0;
                Index iti = 0;
                for ( Index r=0; r<t_interp_order+1; ++r )
                  {
                    for ( Index f=0; f<f_extent; ++f )
                      res[f] += itw[iti] * this_xsec( tgp[0].idx[r], f );

                    ++iti;
                  }
              }
          else
            if (do_VMR)
              {
                // No T with VMR

                // This is a "red" 1D interpolation case.

                Vector itw;
                itw.resize(h2o_interp_order+1);
                
                interpweights(itw,vgp[0]);

                // Get the right view on xsec:
                ConstMatrixView this_xsec
                  = xsec( 0,                     // no T variations
                          Range(fpi,n_nls_pert), // VMR profile range
                          f_range,               // Frequency range
                          this_p_grid_index );   // Pressure index 

                // We must do the same interpolation for all
                // frequencies. Instead of playing with interp and
                // Views and looping the whole thing over frequency, I
                // choose to do this explicitly here, but directly for
                // all frequencies. This should be much more efficient.

                // Initialize result to zero:
                res = 0;
                Index iti = 0;
                for ( Index c=0; c<h2o_interp_order+1; ++c )
                  {
                    for ( Index f=0; f<f_extent; ++f )                      
                      res[f] += itw[iti] * this_xsec( vgp[0].idx[c], f );             
                      
                    ++iti;
                  }
              }
            else
              {
                // No T no VMR

                // No need to interpolate anything here, actually.
                // We can copy all frequencies simultaneously, using
                // the range variable f_range.
                res = xsec(0,fpi,f_range,this_p_grid_index);
              }

          // Increase fpi. fpi marks the position of the first profile
          // of the current species in xsec. This is needed to find
          // the right subsection of xsec in the presence of nonlinear species.
          if (do_VMR)
            fpi += n_nls_pert;
          else
            fpi++;

        } // End of species loop 

      // fpi should have reached the end of that dimension of xsec. Check
      // this with an assertion:
      assert( fpi==xsec.npages() );

    } // End of pressure index loop (below and above gp)

  // Now we have to interpolate between the p_interp_order+1 pressure levels

  // It is a "red" 1D interpolation case we are talking about here.
  // (But for a matrix in frequency and species.) Doing a loop over
  // frequency and species with an interp call inside would be
  // unefficient, so we do this by hand here.
  sga.resize(f_extent, n_species);
  sga = 0;
  for ( Index pi=0; pi<p_interp_order+1; ++pi )
    {
      // Multiply pre interpolated quantities with pressure interpolation weights:
      xsec_pre_interpolated(pi,Range(joker),Range(joker)) *= pitw[pi];

      // Add up in sga:
      sga += xsec_pre_interpolated(pi,Range(joker),Range(joker));
    }

  // Watch out, this is not yet the final result, we
  // need to multiply with the number density of the species, i.e.,
  // with the total number density n, times the VMR of the
  // species: 
  for ( Index si=0; si<n_species; ++si )
    sga(Range(joker),si) *= ( n * abs_vmrs[si] );

  // That's it, we're done!
}


const Vector&  GasAbsLookup::GetFgrid() const
{
  return f_grid;
}


const Vector&  GasAbsLookup::GetPgrid() const
{
  return p_grid;
}


/** Output operatior for GasAbsLookup. */
ostream& operator<< (ostream& os, const GasAbsLookup& /* gal */)
{
  os << "GasAbsLookup: Output operator not implemented";
  return os;
}

