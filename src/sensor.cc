/* Copyright (C)  Mattias Ekstr�m <ekstrom@rss.chalmers.se>

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
  \file   sensor.cc
  \author Mattias Ekstr�m <ekstrom@rss.chalmers.se>
  \date   2003-02-27

  \brief  Functions related to sensor modelling.

  Functions to model sensor behaviour and integration calculated as vector
  multiplication.
*/

/*===========================================================================
  === External declarations
  ===========================================================================*/

#include <cmath>
#include <list>
#include "matpackI.h"
#include "matpackII.h"
//#include <stdexcept>
//#include "array.h"
//#include "auto_md.h"
//#include "check_input.h"
//#include "math_funcs.h"
//#include "messages.h"
//#include "mystring.h"
//#include "poly_roots.h"
//#include "special_interp.h"

  extern const Numeric DEG2RAD;

/*===========================================================================
  === The functions (in alphabetical order)
  ===========================================================================*/

//! sensor_integration_vector
/*!
   Calculates the (row) vector that multiplied with an unknown
   (column) vector approximates the integral of the product
   between the functions represented by the two vectors.

   E.g. h*g = integral( f(x)*g(x) dx )

   \param   h      The multiplication (row) vector.
   \param   f      The values of function f(x).
   \param   x_f    The grid points of function f(x).
   \param   x_g    The grid points of function g(x).

   \author Mattias Ekstr�m
   \date   2003-02-13
*/
void sensor_integration_vector(
           VectorView   h,
      ConstVectorView   f,
      ConstVectorView   x_ftot,
      ConstVectorView   x_g )
{
  //Check that vectors are sorted, ascending (no descending?)

  //Assert that h has the right size
  assert( h.nelem() == x_g.nelem() );

  //Find x_f points that lies outside the scope of x_g and remove them
  Index i1_f = 0, i2_f = x_ftot.nelem()-1;
  while( x_ftot[i1_f] < x_g[0] ) {
    i1_f++;
  }
  while( x_ftot[i2_f] > x_g[x_g.nelem()-1] ) {
    i2_f--;
  }
  Vector x_f = x_ftot[Range(i1_f, i2_f-i1_f+1)];

  //Create a reference grid vector, x_ref that containing the values of
  //x_f and x_g strictly sorted.
  list<Index> l_x;
  for (Index i=0; i<x_f.nelem(); i++)
  	l_x.push_back(x_f[i]);
  for (Index i=0; i<x_g.nelem(); i++) {
    if( x_g[i] > l_x.front() && x_g[i]<l_x.back() )
      l_x.push_back(x_g[i]);
  }

  l_x.sort();
  l_x.unique();

  Vector x_ref(l_x.size());
  Index i=0;
  for (list<Index>::iterator li=l_x.begin(); li != l_x.end(); li++) {
	x_ref[i] = *li;
	i++;
  }

  //Initiate output vector, with equal size as x_g, with zeros.
  //Start calculations
  h = 0.0;
  Index i_f = 0;
  Index i_g = 0;
  //i = 0;
  Numeric dx,a0,b0,c0,a1,b1,c1,x3,x2,x1;
  //while( i_g < x_g.nelem() && i_f < x_f.nelem() ) {
  for( Index i=0; i<x_ref.nelem()-1; i++ ) {
    //Find for what index in x_g (which is the same as for h) and f
    //calculation corresponds to
    while( x_g[i_g+1] <= x_ref[i] ) {
      i_g++;
    }
    while( x_f[i_f+1] <= x_ref[i] ) {
     i_f++;
	 //cout << "i_f: " << i_f << "\n";
    }

    //If x_ref[i] is out of x_f's range then that part of the integral
    //is set to 0, so no calculations will be done
    if( x_ref[i] >= x_f[0] && x_ref[i] < x_f[x_f.nelem()-1] ) {
      //Product of steps in x_f and x_g
      dx = (x_f[i_f+1] - x_f[i_f]) * (x_g[i_g+1] - x_g[i_g]);

      //Calculate a, b and c coefficients; h[i]=ax^3+bx^2+cx
      a0 = (f[i_f] - f[i_f+1]) / 3;
      b0 = (-f[i_f]*(x_g[i_g+1]+x_f[i_f+1])+f[i_f+1]*(x_g[i_g+1]+x_f[i_f]))/2;
      c0 = f[i_f]*x_f[i_f+1]*x_g[i_g+1]-f[i_f+1]*x_f[i_f]*x_g[i_g+1];

      a1 = -a0;
      b1 = (f[i_f]*(x_g[i_g]+x_f[i_f+1])-f[i_f+1]*(x_g[i_g]+x_f[i_f]))/2;
      c1 = -f[i_f]*x_f[i_f+1]*x_g[i_g]+f[i_f+1]*x_f[i_f]*x_g[i_g];

      x3 = pow(x_ref[i+1],3) - pow(x_ref[i],3);
      x2 = pow(x_ref[i+1],2) - pow(x_ref[i],2);
      x1 = x_ref[i+1]-x_ref[i];

      //Calculate h[i] and h[i+1] increment
      h[i_g] += (a0*x3+b0*x2+c0*x1) / dx;
      h[i_g+1] += (a1*x3+b1*x2+c1*x1) / dx;

    }
	//i++;
  }

  //Normalize h.
  h /= h.sum();
}

//! antenna_transfer_matrix
/*!
   Constructs the sparse matrix that multiplied with the spectral values
   for one or several line-of-sights models the antenna transfer matrix.
   The matrix it built up of spaced row vectors, to match the format
   of the spectral values.

   The size of the antenna transfer matrix has to be set by the calling
   function, and it must be set as:
    nrows = x_f.nelem()
    ncols = x_f.nelem() * m_za.nelem()

   The number of line-of-sights is determined by the length of the
   measurement block grid. The number of sensor response matrix rows
   don't need to match the number of frequency grid points.

   FIXME: The antenna diagram values could be set up and scaled using the
   antenna_diagram_gaussian and scale_antenna_diagram functions.

   \param   H      The antenna transfer matrix.
   \param   m_za   The measurement block grid of zenith angles.
   \param   srm	   The sensor response matrix, i.e. the antenna diagram
   \param   x_f    The frequency grid points.

   \author Mattias Ekstr�m
   \date   2003-04-09
*/
void antenna_transfer_matrix(
           SparseView   H,
      ConstVectorView   m_za,
      ConstMatrixView   srm,
      ConstVectorView   x_f )
{
  //Assert that the transfer matrix has the right size
  assert( H.nrows()==x_f.nelem() && H.ncols()==m_za.nelem()*x_f.nelem() );

  //FIXME: Allocate a temporary vector to keep values before sorting them into the
  //final sparse matrix, could (should) be fixed with a SparseView(range, range) operator
  Vector temp(m_za.nelem()*x_f.nelem(), 0.0);

  //Calculate the sensor integration vector and put values in the temp vector
  //FIXME: Scale the antenna diagram?? If so, do it here.
  for (Index i=0; i<x_f.nelem(); i++) {
    sensor_integration_vector( temp[Range(i, m_za.nelem(), x_f.nelem())],
      srm(Range(joker),1), srm(Range(joker),0), m_za);
  }

  //Copy values to the antenna matrix
  for (Index j=0; j<m_za.nelem(); j++) {
    for (Index i=0; i<x_f.nelem(); i++) {
      if (temp[i+j*x_f.nelem()]!=0)
        H.rw(i, i+j*x_f.nelem()) = temp[i+j*x_f.nelem()];
    }
  }
}

//! antenna_diagram_gaussian
/*!
   Sets up a matrix containing a standardised Gaussian antenna diagram,
   described for a certain frequency. The function is called with the
   half-power beam width that determines the shape of the curve for the
   reference frequency, and a matrix that must contain the grid that sets
   up the antenna diagram as the first column. The antenna diagram values
   will the be put in the second column.

   \param   srm     The antenna diagram matrix.
   \param   theta   The antenna average width

   \author Mattias Ekstr�m
   \date   2003-03-11
*/
void antenna_diagram_gaussian(
           MatrixView   srm,
       const Numeric&   theta )
{
  //Assert that a has the right size
  assert( srm.ncols()==2 );

  //Initialise variables
  Numeric ln2 = log(2.0);

  //Loop over grid points to calculate antenna diagram
  for (Index i=0; i<srm.nrows(); i++) {
    srm(i,1)=exp(-4*ln2*pow(srm(i,0)*DEG2RAD/theta,2));
  }
}

//! scale_antenna_diagram
/*!
   Scales a Gaussian antenna diagram for a reference frequency to match
   the new frequency.

   \return          The scaled antenna diagram
   \param   srm     The antenna diagram matrix
   \param   f_ref   The reference frequency
   \param   f_new   The new frequency

   \author Mattias Ekstr�m
   \date   2003-03-11
*/
Matrix scale_antenna_diagram(
        ConstMatrixView   srm,
         const Numeric&   f_ref,
         const Numeric&   f_new )
{
  //Initialise new vector
  Matrix srm_new = srm;

  //Get scale factor
  Numeric s = f_new / f_ref;

  //Scale
  for (Index i=0; i<srm.nrows(); i++) {
    srm_new(i,1)=pow(srm(i,1), s);
  }

  return srm_new;
}

//! spectrometer_transfer_matrix
/*!
   Constructs the sparse matrix that multiplied with the spectral values.
   There are no dependency on line-of-sights, therefor the antenna transfer
   matrix has to be applied before the spectrometer.

   The size of the spectrometer transfer matrix has to be set by the calling
   function, and the number of rows must match the number of spectrometer
   channels while the number of columns must match the number of frequency
   grid points.

   The spectrometer response matrix decribes how the spectrometer behaves on a
   relative frequency grid. The first column contains the relative grid points
   and the second holds the response values.
   NB:(FIXME?) The same spectrometer response is used for all channels, it is only
   shifted to the right centre frequency.

   \param   H      The transfer matrix.
   \param   srm    The spectrometer response matrix.
   \param   x_s    The spectrometer channel centre frequencies.
   \param   x_f    The frequency grid points.

   \author Mattias Ekstr�m
   \date   2003-05-12
*/
void spectrometer_transfer_matrix(
           SparseView   H,
      ConstMatrixView   srm,
      ConstVectorView   x_s,
      ConstVectorView   x_f )
{
  //Assert that the transfer matrix has the right size
  assert( H.nrows()==x_s.nelem() && H.ncols()==x_f.nelem() );

  //Allocate memory for temporary vectors
  Vector temp(x_f.nelem(), 0.0);
  Vector x_srm(srm.nrows());

  //Calculate the sensor integration vector and put values in the temporary
  //vector, then copy vector to the transfer matrix
  for (Index i=0; i<x_s.nelem(); i++) {
    //The spectrometer response is shifted for each centre frequency step
	x_srm = srm(Range(joker),0);
	x_srm += x_s[i];
    sensor_integration_vector( temp, srm(Range(joker),1), x_srm, x_f);

	//FIXME: Should be fixed by a SparseView(Range, Range) -> VectorView
	//This might take more time than needed
	for (Index j=0; j<temp.nelem(); j++) {
	  H.rw(i,j) = temp[j];
    }
  }
}
