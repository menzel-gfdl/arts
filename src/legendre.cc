/* Copyright (C) 2003-2012 Oliver Lemke <olemke@core-dump.info>

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
   USA.
*/



////////////////////////////////////////////////////////////////////////////
//   File description
////////////////////////////////////////////////////////////////////////////
/**
  \file   legendre.cc

  Contains the code to calculate Legendre polynomials.

  \author Oliver Lemke
  \date 2003-08-14
  */

#include <cmath>
#include <sstream>
#include <limits>

#include "legendre.h"
#include "exceptions.h"
#include "math_funcs.h"


//! legendre_poly
/*!
    Returns the associated Legendre polynomial Plm(x).

    The input parameters must fulfill the following conditions:
    0 <= m <= l and |x| <= 1

    The code is based on the Numerical recipes. Results were compared
    to the Legendre calculations from the GNU Scientific library and found
    to be identical.

    \return      Plm
    \param   l   Index
    \param   m   Index
    \param   x   Value

    \author Oliver Lemke
    \date   2003-08-14
*/
Numeric
legendre_poly (Index l, Index m, Numeric x)
{
  Numeric pmm;
  Numeric result = 0.;

  if (m < 0 || m > l || abs (x) > 1.0)
    {
      ostringstream os;
      os << "legendre_poly: Condition 0 <= m <= l && -1 < x < 1 failed"
        << endl << "  l = " << l << "  m = " << m << "  x = " << x << endl;
      throw runtime_error (os.str ());
    }

  pmm = 1.0;
  if (m > 0)
    {
      Numeric fact, somx2;

      somx2 = sqrt ((1.0 - x) * (1.0 + x));
      fact = 1.0;
      for (Index i = 1; i <= m; i++)
        {
          pmm *= -fact * somx2;
          fact += 2.0;
        }
    }

  if (l == m)
    result = pmm;
  else
    {
      Numeric pmmp1;

      pmmp1 = x * (Numeric)(2*m + 1) * pmm;
      if (l == (m+1))
        result = pmmp1;
      else
        {
          for (Index ll = m+2; ll <= l; ll++)
            {
              result = (x * (Numeric)(2*ll - 1) * pmmp1 - (Numeric)(ll + m - 1) * pmm) / (Numeric)(ll - m);
              pmm = pmmp1;
              pmmp1 = result;
            }
        }
    }

  return (result);
}


//! legendre_poly_norm_schmidt
/*!
    Returns the Schmidt quasi-normalized associated Legendre polynomial Plm(x).

    The input parameters must fulfill the following conditions:
    0 <= m <= l and |x| <= 1

    The code is based on the Numerical recipes. Results were compared
    to the Legendre calculations from the GNU Scientific library and found
    to be identical.

    \return      Plm
    \param   l   Index
    \param   m   Index
    \param   x   Value

    \author Oliver Lemke
    \date   2003-08-15
*/
Numeric
legendre_poly_norm_schmidt (Index l, Index m, Numeric x)
{
  Numeric result;

  if (m != 0)

    {
      result = ((sqrt (2.0 * fac (l - m) / fac (l + m))
                 * legendre_poly (l, m, x)));
    }
  else
    {
      result = (legendre_poly (l, m, x));
    }

  return(result);

}


//! legendre_poly_deriv
/*!
    Returns the derivative of the associated Legendre polynomial Plm(x).

    The input parameters must fulfill the following conditions:
    0 <= m <= l and |x| < 1

    \return      dPlm
    \param   l   Index
    \param   m   Index
    \param   x   Value

    \author Oliver Lemke
    \date   2003-08-18
*/
Numeric
legendre_poly_deriv (Index l, Index m, Numeric x)
{
  assert (x != 1.);
  if (x == 1.)
    {
      ostringstream os;
      os << "legendre_poly_deriv: Condition x != 1 failed"
        << endl << "  x = " << x << endl;
      throw runtime_error (os.str ());
    }
  Numeric result;

  if (l == 1)
    {
      if (m == 0)
        {
          result = 1;
        }
      else if (m == 1)
        {
          result = x / sqrt(1 - x*x);
        }
      else
        {
          ostringstream os;
          os << "legendre_poly_deriv: "
            << "Condition l == 1 && (m == 0 || m == 1) failed" << endl
            << "l = " << l << "  m = " << m << endl;
          throw runtime_error (os.str ());
        }
    }
  else if ( m < l )
    {
      try
        {
          result = ((Numeric)(l + m) * legendre_poly (l-1, m, x) - 
                    (Numeric)l * x * legendre_poly (l, m, x)) /
            (1 - x * x);
        }
      catch (const std::runtime_error &e)
        {
          ostringstream os;
          os << e.what () << "legendre_poly_deriv: "
            << "Condition m < l failed" << endl
            << "l = " << l << "  m = " << m << endl;
          throw runtime_error (os.str ());
        }
    }
  else
    {
      try
        {
          result = (Numeric)m * x * legendre_poly (l, m, x) / (1 - x * x)
            + (Numeric)((l + m) * (l - m + 1)) * legendre_poly (l, m - 1, x)
            / sqrt (1 - x * x);
        }
      catch (const std::runtime_error &e)
        {
          ostringstream os;
          os << e.what () << "legendre_poly_norm_schmidt_deriv: "
            << "Condition m = l failed" << endl
            << "l = " << l << "  m = " << m << endl;
          throw runtime_error (os.str ());
        }
    }
 return (result);
}

//! legendre_poly_norm_schmidt_deriv
/*!
  Returns the derivative of the Schmidt quasi-normalized associated Legendre polynomial Plm(x).

  The input parameters must fulfill the following conditions:
  0 <= m <= l and |x| < 1

  \return      dPlm
  \param   l   Index
  \param   m   Index
  \param   x   Value

  \author Nikolay Koulev
  \date   2003-08-18
*/
Numeric
legendre_poly_norm_schmidt_deriv (Index l, Index m, Numeric x)
{
  assert (x != 1.);
  if (x == 1.)
    {
      ostringstream os;
      os << "legendre_poly_norm_schmidt_deriv: Condition x != 1 failed"
        << endl << "  x = " << x << endl;
      throw runtime_error (os.str ());
    }

  Numeric result;

  if (l == 1)
    {
      if (m == 0)
        {
          result = sqrt (2.0 * fac (1 - m) / fac (1 + m));
        }
      else if (m == 1)
        {
          result = sqrt (2.0 * fac (1 - m) / fac (1 + m)) * x / sqrt(1 - x*x);
        }
      else
        {
          ostringstream os;
          os << "legendre_poly_norm_schmidt_deriv: "
            << "Condition l == 1 && (m == 0 || m == 1) failed" << endl
            << "l = " << l << "  m = " << m << endl;
          throw runtime_error (os.str ());
        }

    }
  else if ( m < l )
    {
      try
        {
          result = ((Numeric)(l + m) * legendre_poly_norm_schmidt (l-1, m, x) - 
                    (Numeric)l * x * legendre_poly_norm_schmidt (l, m, x)) /
            (1 - x * x);
        }
      catch (const std::runtime_error &e)
        {
          ostringstream os;
          os << e.what () << "legendre_poly_norm_schmidt_deriv: "
            << "Condition m < l failed" << endl
            << "l = " << l << "  m = " << m << endl;
          throw runtime_error (os.str ());
        }
    }
  else
    {
      try
        {
          result = (Numeric)m * x * legendre_poly_norm_schmidt (l, m, x) / (1 - x * x)
            + (Numeric)((l + m) * (l - m + 1)) * legendre_poly_norm_schmidt (l, m - 1, x)
            / sqrt (1 - x * x);
        }
      catch (const std::runtime_error &e)
        {
          ostringstream os;
          os << e.what () << "legendre_poly_norm_schmidt_deriv: "
            << "Condition m = l failed" << endl
            << "l = " << l << "  m = " << m << endl;
          throw runtime_error (os.str ());
        }
    }

  return (result);
}

//! g_legendre_poly
/*!
  Returns the associated Legendre polynomial Plm(x) without 
  the factor (-1)^m.

  The input parameters must fulfill the following conditions:
  0 <= m <= l and |x| <= 1

  The code is based on the Numerical recipes. Results were compared
  to the Legendre calculations from the GNU Scientific library and found
  to be identical.

  \return      Plm
  \param   l   Index
  \param   m   Index
  \param   x   Value

  \author Nikolay Koulev
  \date   2003-09-02
  */
Numeric
g_legendre_poly (Index l, Index m, Numeric x)
{
  Numeric pmm;
  Numeric result = 0.;

  if (m < 0 || m > l || abs (x) > 1.0)
    {
      ostringstream os;
      os << "g_legendre_poly: Condition 0 <= m <= l && -1 < x < 1 failed"
        << endl << "  l = " << l << "  m = " << m << "  x = " << x << endl;
      throw runtime_error (os.str ());
    }

  pmm = 1.0;
  if (m > 0)
    {
      Numeric fact, somx2;

      somx2 = sqrt ((1.0 - x) * (1.0 + x));
      fact = 1.0;
      for (Index i = 1; i <= m; i++)
        {
          pmm *= fact * somx2;
          fact += 2.0;
        }
    }

  if (l == m)
    result = pmm;
  else
    {
      Numeric pmmp1;

      pmmp1 = x * (Numeric)(2*m + 1) * pmm;
      if (l == (m+1))
        result = pmmp1;
      else
        {
          for (Index ll = m+2; ll <= l; ll++)
            {
              result = (x * (Numeric)(2*ll - 1) * pmmp1 - (Numeric)(ll + m - 1) * pmm) / (Numeric)(ll - m);
              pmm = pmmp1;
              pmmp1 = result;
            }
        }
    }

  return (result);
}


//! g_legendre_poly_norm_schmidt
/*!
  Returns the Schmidt quasi-normalized associated Legendre polynomial Plm(x)) without 
  the factor (-1)^m..

  The input parameters must fulfill the following conditions:
  0 <= m <= l and |x| <= 1

  The code is based on the Numerical recipes. Results were compared
  to the Legendre calculations from the GNU Scientific library and found
  to be identical.

  \return      Plm
  \param   l   Index
  \param   m   Index
  \param   x   Value

  \author Nikolay koulev
  \date   2003-09-02
  */
Numeric
g_legendre_poly_norm_schmidt (Index l, Index m, Numeric x)
{
  Numeric result;

  if (m != 0)

    {
      result = ((sqrt (2.0 * fac (l - m) / fac (l + m))
                 * g_legendre_poly (l, m, x)));
    }
  else
    {
      result = (g_legendre_poly (l, m, x));
    }

  return(result);

}


//! g_legendre_poly_deriv
/*!
  Returns the derivative of the associated Legendre polynomial Plm(x)) without 
  the factor (-1)^m..

  The input parameters must fulfill the following conditions:
  0 <= m <= l and |x| < 1

  \return      dPlm
  \param   l   Index
  \param   m   Index
  \param   x   Value

  \author Nikolay Koulev
  \date   2003-09-02
  */
Numeric
g_legendre_poly_deriv (Index l, Index m, Numeric x)
{
  assert (x != 1.);
  if (x == 1.)
    {
      ostringstream os;
      os << "g_legendre_poly_deriv: Condition x != 1 failed"
        << endl << "  x = " << x << endl;
      throw runtime_error (os.str ());
    }
  Numeric result;

  if (l == 1)
    {
      if (m == 0)
        {
          result = 1;
        }
      else if (m == 1)
        {
          result = x / sqrt(1 - x*x);
        }
      else
        {
          ostringstream os;
          os << "g_legendre_poly_deriv: "
            << "Condition l == 1 && (m == 0 || m == 1) failed" << endl
            << "l = " << l << "  m = " << m << endl;
          throw runtime_error (os.str ());
        }
    }
  else if ( m < l )
    {
      try
        {
          result = ((Numeric)(l + m) * g_legendre_poly (l-1, m, x) - 
                    (Numeric)l * x * g_legendre_poly (l, m, x)) /
            (1 - x * x);
        }
      catch (const std::runtime_error &e)
        {
          ostringstream os;
          os << e.what () << "g_legendre_poly_deriv: "
            << "Condition m < l failed" << endl
            << "l = " << l << "  m = " << m << endl;
          throw runtime_error (os.str ());
        }
    }
  else
    {
      try
        {
          result = - (Numeric)m * x * g_legendre_poly (l, m, x) / (1 - x * x) + 
            (Numeric)((l + m) * (l - m + 1)) * g_legendre_poly (l, m - 1, x) / 
            sqrt (1 - x * x);
        }
      catch (const std::runtime_error &e)
        {
          ostringstream os;
          os << e.what () << "g_legendre_poly_norm_schmidt_deriv: "
            << "Condition m = l failed" << endl
            << "l = " << l << "  m = " << m << endl;
          throw runtime_error (os.str ());
        }
    }
 return (result);
}

//! g_legendre_poly_norm_schmidt_deriv
/*!
  Returns the derivative of the Schmidt quasi-normalized associated Legendre polynomial Plm(x)) without 
  the factor (-1)^m.

  The input parameters must fulfill the following conditions:
  0 <= m <= l and |x| < 1

  \return      dPlm
  \param   l   Index
  \param   m   Index
  \param   x   Value

  \author Nikolay Koulev
  \date   2003-09-02
*/
Numeric
g_legendre_poly_norm_schmidt_deriv (Index l, Index m, Numeric x)
{
  assert (x != 1.);
  if (x == 1.)
    {
      ostringstream os;
      os << "g_legendre_poly_norm_schmidt_deriv: Condition x != 1 failed"
        << endl << "  x = " << x << endl;
      throw runtime_error (os.str ());
    }

  Numeric result;

  if (l == 1)
    {
      if (m == 0)
        {
          result = 1;
        }
      else if (m == 1)
        {
          result = x / sqrt(1 - x * x);
        }
      else
        {
          ostringstream os;
          os << "g_legendre_poly_norm_schmidt_deriv: "
            << "Condition l == 1 && (m == 0 || m == 1) failed" << endl
            << "l = " << l << "  m = " << m << endl;
          throw runtime_error (os.str ());
        }

    }
  else if ( m < l )
    {
      try
        {
          result = sqrt (2.0 * fac (l - m) / fac (l + m)) * 
            ((Numeric)(l + m) * g_legendre_poly(l-1, m, x) - 
             (Numeric)l * x * g_legendre_poly (l, m, x)) / (1 - x * x);
        }
      catch (const std::runtime_error &e)
        {
          ostringstream os;
          os << e.what () << "g_legendre_poly_norm_schmidt_deriv: "
            << "Condition m < l failed" << endl
            << "l = " << l << "  m = " << m << endl;
          throw runtime_error (os.str ());
        }
    }
  else
    {
      try
        {
          result = sqrt (2.0 * fac (l - m) / fac (l + m)) * 
            ( - (Numeric)m * x * g_legendre_poly (l, m, x) / (1 - x * x) + 
              (Numeric)((l + m) * (l - m + 1)) * g_legendre_poly (l, m - 1, x) / 
              sqrt (1 - x * x));
        }
      catch (const std::runtime_error &e)
        {
          ostringstream os;
          os << e.what () << "g_legendre_poly_norm_schmidt_deriv: "
            << "Condition m = l failed" << endl
            << "l = " << l << "  m = " << m << endl;
          throw runtime_error (os.str ());
        }
    }

  return (result);
}

//! g_legendre_poly_norm_schmidt_deriv1
/*!
  Returns the derivative of the Schmidt quasi-normalized associated Legendre polynomial Plm(x)) without 
  the factor (-1)^m. Utilizes the simplest recurrence scheme.

  The input parameters must fulfill the following conditions:
  0 <= m <= l and |x| < 1

  \return      dPlm
  \param   l   Index
  \param   m   Index
  \param   x   Value

  \author Nikolay Koulev
  \date   2003-09-02
*/
Numeric
g_legendre_poly_norm_schmidt_deriv1 (Index l, Index m, Numeric x)
{
  assert (x != 1.);
  if (x == 1.)
    {
      ostringstream os;
      os << "g_legendre_poly_norm_schmidt_deriv: Condition x != 1 failed"
        << endl << "  x = " << x << endl;
      throw runtime_error (os.str ());
    }

  Numeric result;

  if (l == 1)
    {
      if (m == 0)
        {
          result = 1;
        }
      else if (m == 1)
        {
          result = x / sqrt(1 - x * x);
        }
      else
        {
          ostringstream os;
          os << "g_legendre_poly_norm_schmidt_deriv: "
            << "Condition l == 1 && (m == 0 || m == 1) failed" << endl
            << "l = " << l << "  m = " << m << endl;
          throw runtime_error (os.str ());
        }

    }
  else if ( m <= l - 1 )
    {
      try
        {
          // result = - m * x * g_legendre_poly_norm_schmidt (l, m, x) / (1 - x * x)
          //  +  sqrt((double)(l + m + 1 / l - m - 1)) * g_legendre_poly_norm_schmidt (l, m + 1, x)
          //  / sqrt (1 - x * x);

          result = sqrt (2.0 * fac (l - m) / fac (l + m)) * 
            ( - (Numeric)m * x * g_legendre_poly (l, m, x) / (1 - x * x)  + 
              g_legendre_poly (l, m + 1, x)
              / sqrt (1 - x * x));
        }
      catch (const std::runtime_error &e)
        {
          ostringstream os;
          os << e.what () << "g_legendre_poly_norm_schmidt_deriv: "
            << "Condition m <= l - 1 failed" << endl
            << "l = " << l << "  m = " << m << endl;
          throw runtime_error (os.str ());
        }
    }
  else
    {
      try
        {
          result = - sqrt (2.0 * fac (l - m) / fac (l + m)) * 
            (Numeric)m * x * g_legendre_poly (l, m, x) / (1 - x * x);
        }
      catch (const std::runtime_error &e)
        {
          ostringstream os;
          os << e.what () << "g_legendre_poly_norm_schmidt_deriv: "
            << "Condition m = l failed" << endl
            << "l = " << l << "  m = " << m << endl;
          throw runtime_error (os.str ());
        }
    }

  return (result);
}

//! g_legendre_poly_norm_schmidt_deriv2
/*!
  Returns the derivative of the Schmidt quasi-normalized associated Legendre polynomial Plm(x)) without 
  the factor (-1)^m.

  The input parameters must fulfill the following conditions:
  0 <= m <= l and |x| < 1

  \return      dPlm
  \param   l   Index
  \param   m   Index
  \param   x   Value

  \author Nikolay Koulev
  \date   2003-09-02
*/
Numeric
g_legendre_poly_norm_schmidt_deriv2 (Index l, Index m, Numeric x)
{
  assert (x != 1.);
  if (x == 1.)
    {
      ostringstream os;
      os << "g_legendre_poly_norm_schmidt_deriv: Condition x != 1 failed"
        << endl << "  x = " << x << endl;
      throw runtime_error (os.str ());
    }

  Numeric result;

  if (l == 1)
    {
      if (m == 0)
        {
          result = 1;
        }
      else if (m == 1)
        {
          result = x / sqrt(1 - x * x);
        }
      else
        {
          ostringstream os;
          os << "g_legendre_poly_norm_schmidt_deriv: "
            << "Condition l == 1 && (m == 0 || m == 1) failed" << endl
            << "l = " << l << "  m = " << m << endl;
          throw runtime_error (os.str ());
        }

    }
  else if ( m < l )
    {
      try
        {
          result = - sqrt (2.0 * fac (l - m) / fac (l + m)) * 
            ((Numeric)(l + m) * g_legendre_poly (l-1, m, x) - 
             (Numeric)l * x * g_legendre_poly (l, m, x)) /
            (1 - x * x);
        }
      catch (const std::runtime_error &e)
        {
          ostringstream os;
          os << e.what () << "g_legendre_poly_norm_schmidt_deriv: "
            << "Condition m < l failed" << endl
            << "l = " << l << "  m = " << m << endl;
          throw runtime_error (os.str ());
        }
    }
  else
    {
      try
        {
          result = - (Numeric)m * x * g_legendre_poly_norm_schmidt (l, m, x) / (1 - x * x)
            ;
        }
      catch (const std::runtime_error &e)
        {
          ostringstream os;
          os << e.what () << "g_legendre_poly_norm_schmidt_deriv: "
            << "Condition m = l failed" << endl
            << "l = " << l << "  m = " << m << endl;
          throw runtime_error (os.str ());
        }
    }

  return (result);
}

//! g_legendre_poly_norm_schmidt_deriv3
/*!
  Returns the derivative of the Schmidt quasi-normalized associated Legendre polynomial Plm(x)) without 
  the factor (-1)^m.

  The input parameters must fulfill the following conditions:
  0 <= m <= l and |x| < 1

  \return      dPlm
  \param   l   Index
  \param   m   Index
  \param   x   Value

  \author Nikolay Koulev
  \date   2003-09-02
  */
Numeric
g_legendre_poly_norm_schmidt_deriv3 (Index l, Index m, Numeric x)
{
  assert (x != 1.);
  if (x == 1.)
    {
      ostringstream os;
      os << "g_legendre_poly_norm_schmidt_deriv: Condition x != 1 failed"
        << endl << "  x = " << x << endl;
      throw runtime_error (os.str ());
    }

  Numeric result;

  if (l == 1)
    {
      if (m == 0)
        {
          result = 1;
        }
      else if (m == 1)
        {
          result = x / sqrt(1 - x * x);
        }
      else
        {
          ostringstream os;
          os << "g_legendre_poly_norm_schmidt_deriv: "
            << "Condition l == 1 && (m == 0 || m == 1) failed" << endl
            << "l = " << l << "  m = " << m << endl;
          throw runtime_error (os.str ());
        }

    }
  else if ( m < l )
    {
      try
        {
          result = sqrt(2.0 * fac (l - m) / fac (l + m)) * 
            ((Numeric)l * g_legendre_poly (l - 1, m, x) + 
             (Numeric)(m - l) * x * g_legendre_poly (l, m, x)) /
            (1 - x * x);
        }
      catch (const std::runtime_error &e)
        {
          ostringstream os;
          os << e.what () << "g_legendre_poly_norm_schmidt_deriv: "
            << "Condition m < l failed" << endl
            << "l = " << l << "  m = " << m << endl;
          throw runtime_error (os.str ());
        }
    }
  else
    {
      try
        {
          result = - sqrt(2.0 * fac (l - m) / fac (l + m)) * 
            (Numeric)m * x * g_legendre_poly (l, m, x) / (1 - x * x);
        }
      catch (const std::runtime_error &e)
        {
          ostringstream os;
          os << e.what () << "g_legendre_poly_norm_schmidt_deriv: "
            << "Condition m = l failed" << endl
            << "l = " << l << "  m = " << m << endl;
          throw runtime_error (os.str ());
        }
    }

  return (result);
}

//! g_legendre_poly_norm_schmidt_deriv4
/*!
  Returns the derivative of the Schmidt quasi-normalized associated Legendre polynomial Plm(x)) without 
  the factor (-1)^m.

  The input parameters must fulfill the following conditions:
  0 <= m <= l and |x| < 1

  \return      dPlm
  \param   l   Index
  \param   m   Index
  \param   x   Value

  \author Nikolay Koulev
  \date   2003-09-02
*/
Numeric
g_legendre_poly_norm_schmidt_deriv4 (Index l, Index m, Numeric x)
{
  assert (x != 1.);
  if (x == 1.)
    {
      ostringstream os;
      os << "g_legendre_poly_norm_schmidt_deriv: Condition x != 1 failed"
        << endl << "  x = " << x << endl;
      throw runtime_error (os.str ());
    }

  Numeric result;

  if (l == 1)
    {
      if (m == 0)
        {
          result = 1;
        }
      else if (m == 1)
        {
          result = x / sqrt(1 - x * x);
        }
      else
        {
          ostringstream os;
          os << "g_legendre_poly_norm_schmidt_deriv: "
            << "Condition l == 1 && (m == 0 || m == 1) failed" << endl
            << "l = " << l << "  m = " << m << endl;
          throw runtime_error (os.str ());
        }

    }
  else if ( m < l )
    {
      try
        {
          result = sqrt (2.0 * fac (l - m) / fac (l + m)) * 
            ((Numeric)((l + m) * (l + 1)) * g_legendre_poly (l - 1, m, x)  - 
             (Numeric)((l + 2 * m) * (l - m + 1)) * g_legendre_poly (l + 1, m, x)
             / ((Numeric)(2 * l + 1 ) * (1 - x * x)));
        }
      catch (const std::runtime_error &e)
        {
          ostringstream os;
          os << e.what () << "g_legendre_poly_norm_schmidt_deriv: "
            << "Condition m < l failed" << endl
            << "l = " << l << "  m = " << m << endl;
          throw runtime_error (os.str ());
        }
    }
  else
    {
      try
        {
          result = - sqrt (2.0 * fac (l - m) / fac (l + m)) * 
            (Numeric)m * x * g_legendre_poly (l, m, x) / (1 - x * x);
        }
      catch (const std::runtime_error &e)
        {
          ostringstream os;
          os << e.what () << "g_legendre_poly_norm_schmidt_deriv: "
            << "Condition m = l failed" << endl
            << "l = " << l << "  m = " << m << endl;
          throw runtime_error (os.str ());
        }
    }

  return (result);
}

/* n = 2 */
static Numeric x2[1] = {0.5773502691896257645091488};
static Numeric w2[1] = {1.0000000000000000000000000};

/* n = 4 */
static Numeric x4[2] = {0.3399810435848562648026658,
                        0.8611363115940525752239465};
static Numeric w4[2] = {0.6521451548625461426269361,
                        0.3478548451374538573730639};

/* n = 6 */
static Numeric x6[3] = {0.2386191860831969086305017,
                        0.6612093864662645136613996,
                        0.9324695142031520278123016};
static Numeric w6[3] = {0.4679139345726910473898703,
                        0.3607615730481386075698335,
                        0.1713244923791703450402961};

/* n = 8 */
static Numeric x8[4] = {0.1834346424956498049394761,
                        0.5255324099163289858177390,
                        0.7966664774136267395915539,
                        0.9602898564975362316835609};
static Numeric w8[4] = {0.3626837833783619829651504,
                        0.3137066458778872873379622,
                        0.2223810344533744705443560,
                        0.1012285362903762591525314};

/* n = 10 */
static Numeric x10[5] = {0.1488743389816312108848260,
                         0.4333953941292471907992659,
                         0.6794095682990244062343274,
                         0.8650633666889845107320967,
                         0.9739065285171717200779640};
static Numeric w10[5] = {0.2955242247147528701738930,
                         0.2692667193099963550912269,
                         0.2190863625159820439955349,
                         0.1494513491505805931457763,
                         0.0666713443086881375935688};

/* n = 12 */
static Numeric x12[6] = {0.1252334085114689154724414,
                         0.3678314989981801937526915,
                         0.5873179542866174472967024,
                         0.7699026741943046870368938,
                         0.9041172563704748566784659,
                         0.9815606342467192506905491};
static Numeric w12[6] = {0.2491470458134027850005624,
                         0.2334925365383548087608499,
                         0.2031674267230659217490645,
                         0.1600783285433462263346525,
                         0.1069393259953184309602547,
                         0.0471753363865118271946160};

/* n = 14 */
static Numeric x14[7] = {0.1080549487073436620662447,
                         0.3191123689278897604356718,
                         0.5152486363581540919652907,
                         0.6872929048116854701480198,
                         0.8272013150697649931897947,
                         0.9284348836635735173363911,
                         0.9862838086968123388415973};
static Numeric w14[7] = {0.2152638534631577901958764,
                         0.2051984637212956039659241,
                         0.1855383974779378137417166,
                         0.1572031671581935345696019,
                         0.1215185706879031846894148,
                         0.0801580871597602098056333,
                         0.0351194603317518630318329};

/* n = 16 */
static Numeric x16[8] = {0.0950125098376374401853193,
                         0.2816035507792589132304605,
                         0.4580167776572273863424194,
                         0.6178762444026437484466718,
                         0.7554044083550030338951012,
                         0.8656312023878317438804679,
                         0.9445750230732325760779884,
                         0.9894009349916499325961542};
static Numeric w16[8] = {0.1894506104550684962853967,
                         0.1826034150449235888667637,
                         0.1691565193950025381893121,
                         0.1495959888165767320815017,
                         0.1246289712555338720524763,
                         0.0951585116824927848099251,
                         0.0622535239386478928628438,
                         0.0271524594117540948517806};

/* n = 18 */
static Numeric x18[9] = {0.0847750130417353012422619,
                         0.2518862256915055095889729,
                         0.4117511614628426460359318,
                         0.5597708310739475346078715,
                         0.6916870430603532078748911,
                         0.8037049589725231156824175,
                         0.8926024664975557392060606,
                         0.9558239495713977551811959,
                         0.9915651684209309467300160};
static Numeric w18[9] = {0.1691423829631435918406565,
                         0.1642764837458327229860538,
                         0.1546846751262652449254180,
                         0.1406429146706506512047313,
                         0.1225552067114784601845191,
                         0.1009420441062871655628140,
                         0.0764257302548890565291297,
                         0.0497145488949697964533349,
                         0.0216160135264833103133427};

/* n = 20 */
static Numeric x20[10] = {0.0765265211334973337546404,
                          0.2277858511416450780804962,
                          0.3737060887154195606725482,
                          0.5108670019508270980043641,
                          0.6360536807265150254528367,
                          0.7463319064601507926143051,
                          0.8391169718222188233945291,
                          0.9122344282513259058677524,
                          0.9639719272779137912676661,
                          0.9931285991850949247861224};
static Numeric w20[10] = {0.1527533871307258506980843,
                          0.1491729864726037467878287,
                          0.1420961093183820513292983,
                          0.1316886384491766268984945,
                          0.1181945319615184173123774,
                          0.1019301198172404350367501,
                          0.0832767415767047487247581,
                          0.0626720483341090635695065,
                          0.0406014298003869413310400,
                          0.0176140071391521183118620};

/* n = 32 */
static Numeric x32[16] = {0.0483076656877383162348126,
                          0.1444719615827964934851864,
                          0.2392873622521370745446032,
                          0.3318686022821276497799168,
                          0.4213512761306353453641194,
                          0.5068999089322293900237475,
                          0.5877157572407623290407455,
                          0.6630442669302152009751152,
                          0.7321821187402896803874267,
                          0.7944837959679424069630973,
                          0.8493676137325699701336930,
                          0.8963211557660521239653072,
                          0.9349060759377396891709191,
                          0.9647622555875064307738119,
                          0.9856115115452683354001750,
                          0.9972638618494815635449811};
static Numeric w32[16] = {0.0965400885147278005667648,
                          0.0956387200792748594190820,
                          0.0938443990808045656391802,
                          0.0911738786957638847128686,
                          0.0876520930044038111427715,
                          0.0833119242269467552221991,
                          0.0781938957870703064717409,
                          0.0723457941088485062253994,
                          0.0658222227763618468376501,
                          0.0586840934785355471452836,
                          0.0509980592623761761961632,
                          0.0428358980222266806568786,
                          0.0342738629130214331026877,
                          0.0253920653092620594557526,
                          0.0162743947309056706051706,
                          0.0070186100094700966004071};

/* n = 64 */
static Numeric x64[32] = {0.0243502926634244325089558,
                          0.0729931217877990394495429,
                          0.1214628192961205544703765,
                          0.1696444204239928180373136,
                          0.2174236437400070841496487,
                          0.2646871622087674163739642,
                          0.3113228719902109561575127,
                          0.3572201583376681159504426,
                          0.4022701579639916036957668,
                          0.4463660172534640879849477,
                          0.4894031457070529574785263,
                          0.5312794640198945456580139,
                          0.5718956462026340342838781,
                          0.6111553551723932502488530,
                          0.6489654712546573398577612,
                          0.6852363130542332425635584,
                          0.7198818501716108268489402,
                          0.7528199072605318966118638,
                          0.7839723589433414076102205,
                          0.8132653151227975597419233,
                          0.8406292962525803627516915,
                          0.8659993981540928197607834,
                          0.8893154459951141058534040,
                          0.9105221370785028057563807,
                          0.9295691721319395758214902,
                          0.9464113748584028160624815,
                          0.9610087996520537189186141,
                          0.9733268277899109637418535,
                          0.9833362538846259569312993,
                          0.9910133714767443207393824,
                          0.9963401167719552793469245,
                          0.9993050417357721394569056};
static Numeric w64[32] = {0.0486909570091397203833654,
                          0.0485754674415034269347991,
                          0.0483447622348029571697695,
                          0.0479993885964583077281262,
                          0.0475401657148303086622822,
                          0.0469681828162100173253263,
                          0.0462847965813144172959532,
                          0.0454916279274181444797710,
                          0.0445905581637565630601347,
                          0.0435837245293234533768279,
                          0.0424735151236535890073398,
                          0.0412625632426235286101563,
                          0.0399537411327203413866569,
                          0.0385501531786156291289625,
                          0.0370551285402400460404151,
                          0.0354722132568823838106931,
                          0.0338051618371416093915655,
                          0.0320579283548515535854675,
                          0.0302346570724024788679741,
                          0.0283396726142594832275113,
                          0.0263774697150546586716918,
                          0.0243527025687108733381776,
                          0.0222701738083832541592983,
                          0.0201348231535302093723403,
                          0.0179517157756973430850453,
                          0.0157260304760247193219660,
                          0.0134630478967186425980608,
                          0.0111681394601311288185905,
                          0.0088467598263639477230309,
                          0.0065044579689783628561174,
                          0.0041470332605624676352875,
                          0.0017832807216964329472961};

/* n = 96 */
static Numeric x96[48] = {0.0162767448496029695791346,
                          0.0488129851360497311119582,
                          0.0812974954644255589944713,
                          0.1136958501106659209112081,
                          0.1459737146548969419891073,
                          0.1780968823676186027594026,
                          0.2100313104605672036028472,
                          0.2417431561638400123279319,
                          0.2731988125910491414872722,
                          0.3043649443544963530239298,
                          0.3352085228926254226163256,
                          0.3656968614723136350308956,
                          0.3957976498289086032850002,
                          0.4254789884073005453648192,
                          0.4547094221677430086356761,
                          0.4834579739205963597684056,
                          0.5116941771546676735855097,
                          0.5393881083243574362268026,
                          0.5665104185613971684042502,
                          0.5930323647775720806835558,
                          0.6189258401254685703863693,
                          0.6441634037849671067984124,
                          0.6687183100439161539525572,
                          0.6925645366421715613442458,
                          0.7156768123489676262251441,
                          0.7380306437444001328511657,
                          0.7596023411766474987029704,
                          0.7803690438674332176036045,
                          0.8003087441391408172287961,
                          0.8194003107379316755389996,
                          0.8376235112281871214943028,
                          0.8549590334346014554627870,
                          0.8713885059092965028737748,
                          0.8868945174024204160568774,
                          0.9014606353158523413192327,
                          0.9150714231208980742058845,
                          0.9277124567223086909646905,
                          0.9393703397527552169318574,
                          0.9500327177844376357560989,
                          0.9596882914487425393000680,
                          0.9683268284632642121736594,
                          0.9759391745851364664526010,
                          0.9825172635630146774470458,
                          0.9880541263296237994807628,
                          0.9925439003237626245718923,
                          0.9959818429872092906503991,
                          0.9983643758631816777241494,
                          0.9996895038832307668276901};
static Numeric w96[48] = {0.0325506144923631662419614,
                          0.0325161187138688359872055,
                          0.0324471637140642693640128,
                          0.0323438225685759284287748,
                          0.0322062047940302506686671,
                          0.0320344562319926632181390,
                          0.0318287588944110065347537,
                          0.0315893307707271685580207,
                          0.0313164255968613558127843,
                          0.0310103325863138374232498,
                          0.0306713761236691490142288,
                          0.0302999154208275937940888,
                          0.0298963441363283859843881,
                          0.0294610899581679059704363,
                          0.0289946141505552365426788,
                          0.0284974110650853856455995,
                          0.0279700076168483344398186,
                          0.0274129627260292428234211,
                          0.0268268667255917621980567,
                          0.0262123407356724139134580,
                          0.0255700360053493614987972,
                          0.0249006332224836102883822,
                          0.0242048417923646912822673,
                          0.0234833990859262198422359,
                          0.0227370696583293740013478,
                          0.0219666444387443491947564,
                          0.0211729398921912989876739,
                          0.0203567971543333245952452,
                          0.0195190811401450224100852,
                          0.0186606796274114673851568,
                          0.0177825023160452608376142,
                          0.0168854798642451724504775,
                          0.0159705629025622913806165,
                          0.0150387210269949380058763,
                          0.0140909417723148609158616,
                          0.0131282295669615726370637,
                          0.0121516046710883196351814,
                          0.0111621020998384985912133,
                          0.0101607705350084157575876,
                          0.0091486712307833866325846,
                          0.0081268769256987592173824,
                          0.0070964707911538652691442,
                          0.0060585455042359616833167,
                          0.0050142027429275176924702,
                          0.0039645543384446866737334,
                          0.0029107318179349464084106,
                          0.0018539607889469217323359,
                          0.0007967920655520124294381};

/* n = 100 */
static Numeric x100[50] = {0.0156289844215430828722167,
                           0.0468716824215916316149239,
                           0.0780685828134366366948174,
                           0.1091892035800611150034260,
                           0.1402031372361139732075146,
                           0.1710800805386032748875324,
                           0.2017898640957359972360489,
                           0.2323024818449739696495100,
                           0.2625881203715034791689293,
                           0.2926171880384719647375559,
                           0.3223603439005291517224766,
                           0.3517885263724217209723438,
                           0.3808729816246299567633625,
                           0.4095852916783015425288684,
                           0.4378974021720315131089780,
                           0.4657816497733580422492166,
                           0.4932107892081909335693088,
                           0.5201580198817630566468157,
                           0.5465970120650941674679943,
                           0.5725019326213811913168704,
                           0.5978474702471787212648065,
                           0.6226088602037077716041908,
                           0.6467619085141292798326303,
                           0.6702830156031410158025870,
                           0.6931491993558019659486479,
                           0.7153381175730564464599671,
                           0.7368280898020207055124277,
                           0.7575981185197071760356680,
                           0.7776279096494954756275514,
                           0.7968978923903144763895729,
                           0.8153892383391762543939888,
                           0.8330838798884008235429158,
                           0.8499645278795912842933626,
                           0.8660146884971646234107400,
                           0.8812186793850184155733168,
                           0.8955616449707269866985210,
                           0.9090295709825296904671263,
                           0.9216092981453339526669513,
                           0.9332885350430795459243337,
                           0.9440558701362559779627747,
                           0.9539007829254917428493369,
                           0.9628136542558155272936593,
                           0.9707857757637063319308979,
                           0.9778093584869182885537811,
                           0.9838775407060570154961002,
                           0.9889843952429917480044187,
                           0.9931249370374434596520099,
                           0.9962951347331251491861317,
                           0.9984919506395958184001634,
                           0.9997137267734412336782285};
static Numeric w100[50] = {0.0312554234538633569476425,
                           0.0312248842548493577323765,
                           0.0311638356962099067838183,
                           0.0310723374275665165878102,
                           0.0309504788504909882340635,
                           0.0307983790311525904277139,
                           0.0306161865839804484964594,
                           0.0304040795264548200165079,
                           0.0301622651051691449190687,
                           0.0298909795933328309168368,
                           0.0295904880599126425117545,
                           0.0292610841106382766201190,
                           0.0289030896011252031348762,
                           0.0285168543223950979909368,
                           0.0281027556591011733176483,
                           0.0276611982207923882942042,
                           0.0271926134465768801364916,
                           0.0266974591835709626603847,
                           0.0261762192395456763423087,
                           0.0256294029102081160756420,
                           0.0250575444815795897037642,
                           0.0244612027079570527199750,
                           0.0238409602659682059625604,
                           0.0231974231852541216224889,
                           0.0225312202563362727017970,
                           0.0218430024162473863139537,
                           0.0211334421125276415426723,
                           0.0204032326462094327668389,
                           0.0196530874944353058653815,
                           0.0188837396133749045529412,
                           0.0180959407221281166643908,
                           0.0172904605683235824393442,
                           0.0164680861761452126431050,
                           0.0156296210775460027239369,
                           0.0147758845274413017688800,
                           0.0139077107037187726879541,
                           0.0130259478929715422855586,
                           0.0121314576629794974077448,
                           0.0112251140231859771172216,
                           0.0103078025748689695857821,
                           0.0093804196536944579514182,
                           0.0084438714696689714026208,
                           0.0074990732554647115788287,
                           0.0065469484508453227641521,
                           0.0055884280038655151572119,
                           0.0046244500634221193510958,
                           0.0036559612013263751823425,
                           0.0026839253715534824194396,
                           0.0017093926535181052395294,
                           0.0007346344905056717304063};

/* n = 128 */
static Numeric x128[64] = {0.0122236989606157641980521,
                           0.0366637909687334933302153,
                           0.0610819696041395681037870,
                           0.0854636405045154986364980,
                           0.1097942311276437466729747,
                           0.1340591994611877851175753,
                           0.1582440427142249339974755,
                           0.1823343059853371824103826,
                           0.2063155909020792171540580,
                           0.2301735642266599864109866,
                           0.2538939664226943208556180,
                           0.2774626201779044028062316,
                           0.3008654388776772026671541,
                           0.3240884350244133751832523,
                           0.3471177285976355084261628,
                           0.3699395553498590266165917,
                           0.3925402750332674427356482,
                           0.4149063795522750154922739,
                           0.4370245010371041629370429,
                           0.4588814198335521954490891,
                           0.4804640724041720258582757,
                           0.5017595591361444642896063,
                           0.5227551520511754784539479,
                           0.5434383024128103634441936,
                           0.5637966482266180839144308,
                           0.5838180216287630895500389,
                           0.6034904561585486242035732,
                           0.6228021939105849107615396,
                           0.6417416925623075571535249,
                           0.6602976322726460521059468,
                           0.6784589224477192593677557,
                           0.6962147083695143323850866,
                           0.7135543776835874133438599,
                           0.7304675667419088064717369,
                           0.7469441667970619811698824,
                           0.7629743300440947227797691,
                           0.7785484755064119668504941,
                           0.7936572947621932902433329,
                           0.8082917575079136601196422,
                           0.8224431169556438424645942,
                           0.8361029150609068471168753,
                           0.8492629875779689691636001,
                           0.8619154689395484605906323,
                           0.8740527969580317986954180,
                           0.8856677173453972174082924,
                           0.8967532880491581843864474,
                           0.9073028834017568139214859,
                           0.9173101980809605370364836,
                           0.9267692508789478433346245,
                           0.9356743882779163757831268,
                           0.9440202878302201821211114,
                           0.9518019613412643862177963,
                           0.9590147578536999280989185,
                           0.9656543664319652686458290,
                           0.9717168187471365809043384,
                           0.9771984914639073871653744,
                           0.9820961084357185360247656,
                           0.9864067427245862088712355,
                           0.9901278184917343833379303,
                           0.9932571129002129353034372,
                           0.9957927585349811868641612,
                           0.9977332486255140198821574,
                           0.9990774599773758950119878,
                           0.9998248879471319144736081};
static Numeric w128[64] = {0.0244461801962625182113259,
                           0.0244315690978500450548486,
                           0.0244023556338495820932980,
                           0.0243585572646906258532685,
                           0.0243002001679718653234426,
                           0.0242273192228152481200933,
                           0.0241399579890192849977167,
                           0.0240381686810240526375873,
                           0.0239220121367034556724504,
                           0.0237915577810034006387807,
                           0.0236468835844476151436514,
                           0.0234880760165359131530253,
                           0.0233152299940627601224157,
                           0.0231284488243870278792979,
                           0.0229278441436868469204110,
                           0.0227135358502364613097126,
                           0.0224856520327449668718246,
                           0.0222443288937997651046291,
                           0.0219897106684604914341221,
                           0.0217219495380520753752610,
                           0.0214412055392084601371119,
                           0.0211476464682213485370195,
                           0.0208414477807511491135839,
                           0.0205227924869600694322850,
                           0.0201918710421300411806732,
                           0.0198488812328308622199444,
                           0.0194940280587066028230219,
                           0.0191275236099509454865185,
                           0.0187495869405447086509195,
                           0.0183604439373313432212893,
                           0.0179603271850086859401969,
                           0.0175494758271177046487069,
                           0.0171281354231113768306810,
                           0.0166965578015892045890915,
                           0.0162550009097851870516575,
                           0.0158037286593993468589656,
                           0.0153430107688651440859909,
                           0.0148731226021473142523855,
                           0.0143943450041668461768239,
                           0.0139069641329519852442880,
                           0.0134112712886163323144890,
                           0.0129075627392673472204428,
                           0.0123961395439509229688217,
                           0.0118773073727402795758911,
                           0.0113513763240804166932817,
                           0.0108186607395030762476596,
                           0.0102794790158321571332153,
                           0.0097341534150068058635483,
                           0.0091830098716608743344787,
                           0.0086263777986167497049788,
                           0.0080645898904860579729286,
                           0.0074979819256347286876720,
                           0.0069268925668988135634267,
                           0.0063516631617071887872143,
                           0.0057726375428656985893346,
                           0.0051901618326763302050708,
                           0.0046045842567029551182905,
                           0.0040162549837386423131943,
                           0.0034255260409102157743378,
                           0.0028327514714579910952857,
                           0.0022382884309626187436221,
                           0.0016425030186690295387909,
                           0.0010458126793403487793129,
                           0.0004493809602920903763943};

/* n = 256 */
static Numeric x256[128] = {0.0061239123751895295011702,
                            0.0183708184788136651179263,
                            0.0306149687799790293662786,
                            0.0428545265363790983812423,
                            0.0550876556946339841045614,
                            0.0673125211657164002422903,
                            0.0795272891002329659032271,
                            0.0917301271635195520311456,
                            0.1039192048105094036391969,
                            0.1160926935603328049407349,
                            0.1282487672706070947420496,
                            0.1403856024113758859130249,
                            0.1525013783386563953746068,
                            0.1645942775675538498292845,
                            0.1766624860449019974037218,
                            0.1887041934213888264615036,
                            0.2007175933231266700680007,
                            0.2127008836226259579370402,
                            0.2246522667091319671478783,
                            0.2365699497582840184775084,
                            0.2484521450010566668332427,
                            0.2602970699919425419785609,
                            0.2721029478763366095052447,
                            0.2838680076570817417997658,
                            0.2955904844601356145637868,
                            0.3072686197993190762586103,
                            0.3189006618401062756316834,
                            0.3304848656624169762291870,
                            0.3420194935223716364807297,
                            0.3535028151129699895377902,
                            0.3649331078236540185334649,
                            0.3763086569987163902830557,
                            0.3876277561945155836379846,
                            0.3988887074354591277134632,
                            0.4100898214687165500064336,
                            0.4212294180176238249768124,
                            0.4323058260337413099534411,
                            0.4433173839475273572169258,
                            0.4542624399175899987744552,
                            0.4651393520784793136455705,
                            0.4759464887869833063907375,
                            0.4866822288668903501036214,
                            0.4973449618521814771195124,
                            0.5079330882286160362319249,
                            0.5184450196736744762216617,
                            0.5288791792948222619514764,
                            0.5392340018660591811279362,
                            0.5495079340627185570424269,
                            0.5596994346944811451369074,
                            0.5698069749365687590576675,
                            0.5798290385590829449218317,
                            0.5897641221544543007857861,
                            0.5996107353629683217303882,
                            0.6093674010963339395223108,
                            0.6190326557592612194309676,
                            0.6286050494690149754322099,
                            0.6380831462729113686686886,
                            0.6474655243637248626170162,
                            0.6567507762929732218875002,
                            0.6659375091820485599064084,
                            0.6750243449311627638559187,
                            0.6840099204260759531248771,
                            0.6928928877425769601053416,
                            0.7016719143486851594060835,
                            0.7103456833045433133945663,
                            0.7189128934599714483726399,
                            0.7273722596496521265868944,
                            0.7357225128859178346203729,
                            0.7439624005491115684556831,
                            0.7520906865754920595875297,
                            0.7601061516426554549419068,
                            0.7680075933524456359758906,
                            0.7757938264113257391320526,
                            0.7834636828081838207506702,
                            0.7910160119895459945467075,
                            0.7984496810321707587825429,
                            0.8057635748129986232573891,
                            0.8129565961764315431364104,
                            0.8200276660989170674034781,
                            0.8269757238508125142890929,
                            0.8337997271555048943484439,
                            0.8404986523457627138950680,
                            0.8470714945172962071870724,
                            0.8535172676795029650730355,
                            0.8598350049033763506961731,
                            0.8660237584665545192975154,
                            0.8720825999954882891300459,
                            0.8780106206047065439864349,
                            0.8838069310331582848598262,
                            0.8894706617776108888286766,
                            0.8950009632230845774412228,
                            0.9003970057703035447716200,
                            0.9056579799601446470826819,
                            0.9107830965950650118909072,
                            0.9157715868574903845266696,
                            0.9206227024251464955050471,
                            0.9253357155833162028727303,
                            0.9299099193340056411802456,
                            0.9343446275020030942924765,
                            0.9386391748378148049819261,
                            0.9427929171174624431830761,
                            0.9468052312391274813720517,
                            0.9506755153166282763638521,
                            0.9544031887697162417644479,
                            0.9579876924111781293657904,
                            0.9614284885307321440064075,
                            0.9647250609757064309326123,
                            0.9678769152284894549090038,
                            0.9708835784807430293209233,
                            0.9737445997043704052660786,
                            0.9764595497192341556210107,
                            0.9790280212576220388242380,
                            0.9814496290254644057693031,
                            0.9837240097603154961666861,
                            0.9858508222861259564792451,
                            0.9878297475648606089164877,
                            0.9896604887450652183192437,
                            0.9913427712075830869221885,
                            0.9928763426088221171435338,
                            0.9942609729224096649628775,
                            0.9954964544810963565926471,
                            0.9965826020233815404305044,
                            0.9975192527567208275634088,
                            0.9983062664730064440555005,
                            0.9989435258434088565550263,
                            0.9994309374662614082408542,
                            0.9997684374092631861048786,
                            0.9999560500189922307348012};
static Numeric w256[128] = {0.0122476716402897559040703,
                            0.0122458343697479201424639,
                            0.0122421601042728007697281,
                            0.0122366493950401581092426,
                            0.0122293030687102789041463,
                            0.0122201222273039691917087,
                            0.0122091082480372404075141,
                            0.0121962627831147135181810,
                            0.0121815877594817721740476,
                            0.0121650853785355020613073,
                            0.0121467581157944598155598,
                            0.0121266087205273210347185,
                            0.0121046402153404630977578,
                            0.0120808558957245446559752,
                            0.0120552593295601498143471,
                            0.0120278543565825711612675,
                            0.0119986450878058119345367,
                            0.0119676359049058937290073,
                            0.0119348314595635622558732,
                            0.0119002366727664897542872,
                            0.0118638567340710787319046,
                            0.0118256971008239777711607,
                            0.0117857634973434261816901,
                            0.0117440619140605503053767,
                            0.0117005986066207402881898,
                            0.0116553800949452421212989,
                            0.0116084131622531057220847,
                            0.0115597048540436357726687,
                            0.0115092624770394979585864,
                            0.0114570935980906391523344,
                            0.0114032060430391859648471,
                            0.0113476078955454919416257,
                            0.0112903074958755095083676,
                            0.0112313134396496685726568,
                            0.0111706345765534494627109,
                            0.0111082800090098436304608,
                            0.0110442590908139012635176,
                            0.0109785814257295706379882,
                            0.0109112568660490397007968,
                            0.0108422955111147959952935,
                            0.0107717077058046266366536,
                            0.0106995040389797856030482,
                            0.0106256953418965611339617,
                            0.0105502926865814815175336,
                            0.0104733073841704030035696,
                            0.0103947509832117289971017,
                            0.0103146352679340150682607,
                            0.0102329722564782196569549,
                            0.0101497741990948656546341,
                            0.0100650535763063833094610,
                            0.0099788230970349101247339,
                            0.0098910956966958286026307,
                            0.0098018845352573278254988,
                            0.0097112029952662799642497,
                            0.0096190646798407278571622,
                            0.0095254834106292848118297,
                            0.0094304732257377527473528,
                            0.0093340483776232697124660,
                            0.0092362233309563026873787,
                            0.0091370127604508064020005,
                            0.0090364315486628736802278,
                            0.0089344947837582075484084,
                            0.0088312177572487500253183,
                            0.0087266159616988071403366,
                            0.0086207050884010143053688,
                            0.0085135010250224906938384,
                            0.0084050198532215357561803,
                            0.0082952778462352254251714,
                            0.0081842914664382699356198,
                            0.0080720773628734995009470,
                            0.0079586523687543483536132,
                            0.0078440334989397118668103,
                            0.0077282379473815556311102,
                            0.0076112830845456594616187,
                            0.0074931864548058833585998,
                            0.0073739657738123464375724,
                            0.0072536389258339137838291,
                            0.0071322239610753900716724,
                            0.0070097390929698226212344,
                            0.0068862026954463203467133,
                            0.0067616333001737987809279,
                            0.0066360495937810650445900,
                            0.0065094704150536602678099,
                            0.0063819147521078805703752,
                            0.0062534017395424012720636,
                            0.0061239506555679325423891,
                            0.0059935809191153382211277,
                            0.0058623120869226530606616,
                            0.0057301638506014371773844,
                            0.0055971560336829100775514,
                            0.0054633085886443102775705,
                            0.0053286415939159303170811,
                            0.0051931752508692809303288,
                            0.0050569298807868423875578,
                            0.0049199259218138656695588,
                            0.0047821839258926913729317,
                            0.0046437245556800603139791,
                            0.0045045685814478970686418,
                            0.0043647368779680566815684,
                            0.0042242504213815362723565,
                            0.0040831302860526684085998,
                            0.0039413976414088336277290,
                            0.0037990737487662579981170,
                            0.0036561799581425021693892,
                            0.0035127377050563073309711,
                            0.0033687685073155510120191,
                            0.0032242939617941981570107,
                            0.0030793357411993375832054,
                            0.0029339155908297166460123,
                            0.0027880553253277068805748,
                            0.0026417768254274905641208,
                            0.0024951020347037068508395,
                            0.0023480529563273120170065,
                            0.0022006516498399104996849,
                            0.0020529202279661431745488,
                            0.0019048808534997184044191,
                            0.0017565557363307299936069,
                            0.0016079671307493272424499,
                            0.0014591373333107332010884,
                            0.0013100886819025044578317,
                            0.0011608435575677247239706,
                            0.0010114243932084404526058,
                            0.0008618537014200890378141,
                            0.0007121541634733206669090,
                            0.0005623489540314098028152,
                            0.0004124632544261763284322,
                            0.0002625349442964459062875,
                            0.0001127890178222721755125};

/* n = 512 */
static Numeric x512[256] = {0.0030649621851593961529232,
                            0.0091947713864329108047442,
                            0.0153242350848981855249677,
                            0.0214531229597748745137841,
                            0.0275812047119197840615246,
                            0.0337082500724805951232271,
                            0.0398340288115484476830396,
                            0.0459583107468090617788760,
                            0.0520808657521920701127271,
                            0.0582014637665182372392330,
                            0.0643198748021442404045319,
                            0.0704358689536046871990309,
                            0.0765492164062510452915674,
                            0.0826596874448871596284651,
                            0.0887670524624010326092165,
                            0.0948710819683925428909483,
                            0.1009715465977967786264323,
                            0.1070682171195026611052004,
                            0.1131608644449665349442888,
                            0.1192492596368204011642726,
                            0.1253331739174744696875513,
                            0.1314123786777137080093018,
                            0.1374866454852880630171099,
                            0.1435557460934960331730353,
                            0.1496194524497612685217272,
                            0.1556775367042018762501969,
                            0.1617297712181921097989489,
                            0.1677759285729161198103670,
                            0.1738157815779134454985394,
                            0.1798491032796159253350647,
                            0.1858756669698757062678115,
                            0.1918952461944840310240859,
                            0.1979076147616804833961808,
                            0.2039125467506523717658375,
                            0.2099098165200239314947094,
                            0.2158991987163350271904893,
                            0.2218804682825090362529109,
                            0.2278534004663095955103621,
                            0.2338177708287858931763260,
                            0.2397733552527061887852891,
                            0.2457199299509792442100997,
                            0.2516572714750633493170137,
                            0.2575851567233626262808095,
                            0.2635033629496102970603704,
                            0.2694116677712385990250046,
                            0.2753098491777350342234845,
                            0.2811976855389846383013106,
                            0.2870749556135979555970354,
                            0.2929414385572244074855835,
                            0.2987969139308507415853707,
                            0.3046411617090842500066247,
                            0.3104739622884204453906292,
                            0.3162950964954948840736281,
                            0.3221043455953188263048133,
                            0.3279014912994984240551598,
                            0.3336863157744371275728377,
                            0.3394586016495210024715049,
                            0.3452181320252866497799379,
                            0.3509646904815714220351686,
                            0.3566980610856456291665404,
                            0.3624180284003264285948478,
                            0.3681243774920730946589582,
                            0.3738168939390633631820054,
                            0.3794953638392505477003659,
                            0.3851595738184011246011504,
                            0.3908093110381124851478484,
                            0.3964443632038105531190080,
                            0.4020645185727269675414064,
                            0.4076695659618555307670286,
                            0.4132592947558876229222955,
                            0.4188334949151262845483445,
                            0.4243919569833786700527309,
                            0.4299344720958265754056529,
                            0.4354608319868747443376920,
                            0.4409708289979766581310498,
                            0.4464642560854375149423431,
                            0.4519409068281941054521446,
                            0.4574005754355712925046003,
                            0.4628430567550148032795831,
                            0.4682681462798000434299255,
                            0.4736756401567166435172692,
                            0.4790653351937284489919577,
                            0.4844370288676086658851277,
                            0.4897905193315498753147078,
                            0.4951256054227486308513615,
                            0.5004420866699643537454866,
                            0.5057397633010522419821678,
                            0.5110184362504699101074361,
                            0.5162779071667574777562819,
                            0.5215179784199908258105606,
                            0.5267384531092077401231844,
                            0.5319391350698066637637706,
                            0.5371198288809177797701793,
                            0.5422803398727461474300859,
                            0.5474204741338866161668468,
                            0.5525400385186102421644070,
                            0.5576388406541219339368088,
                            0.5627166889477890541289656,
                            0.5677733925943407059267120,
                            0.5728087615830374335557009,
                            0.5778226067048110674604360,
                            0.5828147395593744458765762,
                            0.5877849725623007456415722,
                            0.5927331189520721562306608,
                            0.5976589927970976321572046,
                            0.6025624090026994600382737,
                            0.6074431833180683777981926,
                            0.6123011323431869846644595,
                            0.6171360735357211818019505,
                            0.6219478252178793846326095,
                            0.6267362065832392490988318,
                            0.6315010377035416553494506,
                            0.6362421395354516935575740,
                            0.6409593339272863978194482,
                            0.6456524436257089753330001,
                            0.6503212922823892793136899,
                            0.6549657044606302753737317,
                            0.6595855056419602523685720,
                            0.6641805222326905300017078,
                            0.6687505815704384167754210,
                            0.6732955119306151731807642,
                            0.6778151425328787363350998,
                            0.6823093035475509635996236,
                            0.6867778261019991540425409,
                            0.6912205422869816079558685,
                            0.6956372851629569859851427,
                            0.7000278887663572307915895,
                            0.7043921881158238155354902,
                            0.7087300192184070848475163,
                            0.7130412190757284553416507,
                            0.7173256256901052441189100,
                            0.7215830780706378951153816,
                            0.7258134162392593745610389,
                            0.7300164812367465082373380,
                            0.7341921151286930346516885,
                            0.7383401610114441496854630,
                            0.7424604630179923197192207,
                            0.7465528663238341416942072,
                            0.7506172171527880300329109,
                            0.7546533627827725118134392,
                            0.7586611515515449130726824,
                            0.7626404328624002206015913,
                            0.7665910571898299050923647,
                            0.7705128760851404930018538,
                            0.7744057421820316760079998,
                            0.7782695092021337484565606,
                            0.7821040319605041647237048,
                            0.7859091663710830099561901,
                            0.7896847694521071791947507,
                            0.7934306993314830614379285,
                            0.7971468152521175267628422,
                            0.8008329775772070161862372,
                            0.8044890477954845355235412,
                            0.8081148885264243560855026,
                            0.8117103635254042266412553,
                            0.8152753376888249026732770,
                            0.8188096770591868005536242,
                            0.8223132488301235858819787,
                            0.8257859213513925068443721,
                            0.8292275641338212850768968,
                            0.8326380478542113781512150,
                            0.8360172443601974294381733,
                            0.8393650266750627227522641,
                            0.8426812690025104608329811,
                            0.8459658467313906883792422,
                            0.8492186364403826820199251,
                            0.8524395159026326312771384,
                            0.8556283640903464362590494,
                            0.8587850611793374495058711,
                            0.8619094885535289911058997,
                            0.8650015288094114678982387,
                            0.8680610657604539292849800,
                            0.8710879844414698938880857,
                            0.8740821711129372830049576,
                            0.8770435132652722985416439,
                            0.8799718996230570848337538,
                            0.8828672201492210155023745,
                            0.8857293660491754482355527,
                            0.8885582297749017921351663,
                            0.8913537050289927340242104,
                            0.8941156867686464718706125,
                            0.8968440712096138052506156,
                            0.8995387558300979345474886,
                            0.9021996393746068223597927,
                            0.9048266218577579723776075,
                            0.9074196045680354827749729,
                            0.9099784900714992329623006,
                            0.9125031822154460643436214,
                            0.9149935861320228175302595,
                            0.9174496082417910902748409,
                            0.9198711562572435822074657,
                            0.9222581391862718942794141,
                            0.9246104673355856526489486,
                            0.9269280523140828285786768,
                            0.9292108070361711277546193,
                            0.9314586457250403242837002,
                            0.9336714839158854164789745,
                            0.9358492384590804834007204,
                            0.9379918275233031229867813,
                            0.9400991705986093544775539,
                            0.9421711884994588697201555,
                            0.9442078033676905198230562,
                            0.9462089386754479255274304,
                            0.9481745192280551015654245,
                            0.9501044711668419871894147,
                            0.9519987219719197769813274,
                            0.9538572004649059479887372,
                            0.9556798368115988811866200,
                            0.9574665625246019772327448,
                            0.9592173104658971684737507,
                            0.9609320148493677311718534,
                            0.9626106112432703039637754,
                            0.9642530365726560206402068,
                            0.9658592291217406674540047,
                            0.9674291285362237773389233,
                            0.9689626758255565756615864,
                            0.9704598133651586944555050,
                            0.9719204848985835745206522,
                            0.9733446355396324773464471,
                            0.9747322117744170315712560,
                            0.9760831614633702416830300,
                            0.9773974338432058899681861,
                            0.9786749795288262664309572,
                            0.9799157505151781656726285,
                            0.9811197001790570947322311,
                            0.9822867832808596419166429,
                            0.9834169559662839640681455,
                            0.9845101757679783590716126,
                            0.9855664016071379024692622,
                            0.9865855937950491429603684,
                            0.9875677140345828729848910,
                            0.9885127254216350200148487,
                            0.9894205924465157453777048,
                            0.9902912809952868962106899,
                            0.9911247583510480415528399,
                            0.9919209931951714500244370,
                            0.9926799556084865573546763,
                            0.9934016170724147657859271,
                            0.9940859504700558793702825,
                            0.9947329300872282225027936,
                            0.9953425316134657151476031,
                            0.9959147321429772566997088,
                            0.9964495101755774022837600,
                            0.9969468456176038804367370,
                            0.9974067197828498321611183,
                            0.9978291153935628466036470,
                            0.9982140165816127953876923,
                            0.9985614088900397275573677,
                            0.9988712792754494246541769,
                            0.9991436161123782382453400,
                            0.9993784092025992514480161,
                            0.9995756497983108555936109,
                            0.9997353306710426625827368,
                            0.9998574463699794385446275,
                            0.9999419946068456536361287,
                            0.9999889909843818679872841};
static Numeric w512[256] = {0.0061299051754057857591564,
                            0.0061296748380364986664278,
                            0.0061292141719530834395471,
                            0.0061285231944655327693402,
                            0.0061276019315380226384508,
                            0.0061264504177879366912426,
                            0.0061250686964845654506976,
                            0.0061234568195474804311878,
                            0.0061216148475445832082156,
                            0.0061195428496898295184288,
                            0.0061172409038406284754329,
                            0.0061147090964949169991245,
                            0.0061119475227879095684795,
                            0.0061089562864885234199252,
                            0.0061057354999954793256260,
                            0.0061022852843330780981965,
                            0.0060986057691466529805468,
                            0.0060946970926976980917399,
                            0.0060905594018586731119147,
                            0.0060861928521074844014940,
                            0.0060815976075216427620556,
                            0.0060767738407720980583934,
                            0.0060717217331167509334394,
                            0.0060664414743936418598512,
                            0.0060609332630138177841916,
                            0.0060551973059538766317450,
                            0.0060492338187481899521175,
                            0.0060430430254808039978627,
                            0.0060366251587770195404584,
                            0.0060299804597946507400317,
                            0.0060231091782149633972884,
                            0.0060160115722332929281516,
                            0.0060086879085493424136484,
                            0.0060011384623571610896056,
                            0.0059933635173348036527221,
                            0.0059853633656336707715812,
                            0.0059771383078675312031423,
                            0.0059686886531012259272183,
                            0.0059600147188390547233923,
                            0.0059511168310128456267588,
                            0.0059419953239697077107922,
                            0.0059326505404594676575446,
                            0.0059230828316217905872556,
                            0.0059132925569729856313229,
                            0.0059032800843924967444267,
                            0.0058930457901090792634301,
                            0.0058825900586866627324847,
                            0.0058719132830099005255609,
                            0.0058610158642694068093892,
                            0.0058498982119466814015496,
                            0.0058385607437987230901727,
                            0.0058270038858423319934219,
                            0.0058152280723381015486124,
                            0.0058032337457741007324836,
                            0.0057910213568492471257818,
                            0.0057785913644563714469284,
                            0.0057659442356649741911390,
                            0.0057530804457036750229319,
                            0.0057400004779423555815070,
                            0.0057267048238739963699973,
                            0.0057131939830962084110906,
                            0.0056994684632924603629882,
                            0.0056855287802130018011102,
                            0.0056713754576554833823756,
                            0.0056570090274452746202723,
                            0.0056424300294154800102991,
                            0.0056276390113866542566918,
                            0.0056126365291462173626557,
                            0.0055974231464275703576030,
                            0.0055819994348889124461425,
                            0.0055663659740917603747899,
                            0.0055505233514791708235538,
                            0.0055344721623536666407146,
                            0.0055182130098548677502395,
                            0.0055017465049368275723757,
                            0.0054850732663450758090285,
                            0.0054681939205933684565648,
                            0.0054511091019401459196852,
                            0.0054338194523647001109732,
                            0.0054163256215430514316688,
                            0.0053986282668235365401123,
                            0.0053807280532021078251738,
                            0.0053626256532973455128155,
                            0.0053443217473251833447318,
                            0.0053258170230733487787774,
                            0.0053071121758755186716175,
                            0.0052882079085851914147269,
                            0.0052691049315492765055207,
                            0.0052498039625814025460136,
                            0.0052303057269349446719890,
                            0.0052106109572757724261988,
                            0.0051907203936547190996206,
                            0.0051706347834797735752665,
                            0.0051503548814879957194620,
                            0.0051298814497171563759039,
                            0.0051092152574771030281542,
                            0.0050883570813208522065339,
                            0.0050673077050154097256505,
                            0.0050460679195123198490183,
                            0.0050246385229179444874178,
                            0.0050030203204634735477834,
                            0.0049812141244746675595135,
                            0.0049592207543413337151533,
                            0.0049370410364865364724225,
                            0.0049146758043355438745290,
                            0.0048921258982845107556462,
                            0.0048693921656689000083132,
                            0.0048464754607316430993636,
                            0.0048233766445910410307843,
                            0.0048000965852084069516609,
                            0.0047766361573554516370718,
                            0.0047529962425814130594576,
                            0.0047291777291799312876071,
                            0.0047051815121556699579709,
                            0.0046810084931906855725376,
                            0.0046566595806105458869828,
                            0.0046321356893501986622283,
                            0.0046074377409195920619320,
                            0.0045825666633690479877601,
                            0.0045575233912543896535753,
                            0.0045323088656018247089130,
                            0.0045069240338725852313010,
                            0.0044813698499273259161146,
                            0.0044556472739902818017469,
                            0.0044297572726131868769073,
                            0.0044037008186389549258496,
                            0.0043774788911651239762643,
                            0.0043510924755070657234522,
                            0.0043245425631609613132305,
                            0.0042978301517665448748000,
                            0.0042709562450696162035304,
                            0.0042439218528843240022977,
                            0.0042167279910552210986262,
                            0.0041893756814190930634598,
                            0.0041618659517665616659011,
                            0.0041341998358034646067195,
                            0.0041063783731120129818357,
                            0.0040784026091117279353449,
                            0.0040502735950201579699371,
                            0.0040219923878133783908191,
                            0.0039935600501862743674273,
                            0.0039649776505126091053562,
                            0.0039362462628048786290012,
                            0.0039073669666739546834366,
                            0.0038783408472885172720108,
                            0.0038491689953342783540510,
                            0.0038198525069729982349166,
                            0.0037903924838012961884344,
                            0.0037607900328092568594835,
                            0.0037310462663388340021755,
                            0.0037011623020420531166926,
                            0.0036711392628390145554094,
                            0.0036409782768756986764252,
                            0.0036106804774815746300758,
                            0.0035802470031270143713799,
                            0.0035496789973805134987000,
                            0.0035189776088657205261605,
                            0.0034881439912182762045767,
                            0.0034571793030424645127888,
                            0.0034260847078676769483860,
                            0.0033948613741046917538288,
                            0.0033635104750017697209450,
                            0.0033320331886005682236783,
                            0.0033004306976918751358177,
                            0.0032687041897711642972145,
                            0.0032368548569939741987234,
                            0.0032048838961311115627642,
                            0.0031727925085236815030060,
                            0.0031405819000379459532169,
                            0.0031082532810200120618074,
                            0.0030758078662503522550163,
                            0.0030432468748981576780527,
                            0.0030105715304755267298129,
                            0.0029777830607914904130339,
                            0.0029448826979058762279357,
                            0.0029118716780830123435331,
                            0.0028787512417452737868732,
                            0.0028455226334264723964728,
                            0.0028121871017250922921949,
                            0.0027787458992573726197173,
                            0.0027452002826102393336092,
                            0.0027115515122940877888456,
                            0.0026778008526954179163600,
                            0.0026439495720293237639656,
                            0.0026099989422918391896635,
                            0.0025759502392121415000167,
                            0.0025418047422046148318992,
                            0.0025075637343207750815413,
                            0.0024732285022010581903898,
                            0.0024388003360264736029032,
                            0.0024042805294701247170072,
                            0.0023696703796485981535706,
                            0.0023349711870732236769383,
                            0.0023001842556012066042973,
                            0.0022653108923866345474810,
                            0.0022303524078313603367724,
                            0.0021953101155357629823745,
                            0.0021601853322493885355395,
                            0.0021249793778214727179358,
                            0.0020896935751513471947536,
                            0.0020543292501387313744068,
                            0.0020188877316339116255770,
                            0.0019833703513878098109153,
                            0.0019477784440019430461334,
                            0.0019121133468782766036998,
                            0.0018763764001689718921795,
                            0.0018405689467260314557679,
                            0.0018046923320508429542037,
                            0.0017687479042436241015783,
                            0.0017327370139527705642995,
                            0.0016966610143241088445575,
                            0.0016605212609500562072903,
                            0.0016243191118186897474239,
                            0.0015880559272627267421479,
                            0.0015517330699084184928942,
                            0.0015153519046243599371387,
                            0.0014789137984702174059640,
                            0.0014424201206453770259886,
                            0.0014058722424375164225552,
                            0.0013692715371711025869345,
                            0.0013326193801558190401403,
                            0.0012959171486349257824991,
                            0.0012591662217335559930561,
                            0.0012223679804069540808915,
                            0.0011855238073886605549070,
                            0.0011486350871386503607080,
                            0.0011117032057914329649653,
                            0.0010747295511041247428251,
                            0.0010377155124045074300544,
                            0.0010006624805390909706032,
                            0.0009635718478212056798501,
                            0.0009264450079791582697455,
                            0.0008892833561045005372012,
                            0.0008520882886004809402792,
                            0.0008148612031307819965602,
                            0.0007776034985686972438014,
                            0.0007403165749469818962867,
                            0.0007030018334087411433900,
                            0.0006656606761599343409382,
                            0.0006282945064244358390880,
                            0.0005909047284032230162400,
                            0.0005534927472403894647847,
                            0.0005160599690007674370993,
                            0.0004786078006679509066920,
                            0.0004411376501795405636493,
                            0.0004036509265333198797447,
                            0.0003661490400356268530141,
                            0.0003286334028523334162522,
                            0.0002911054302514885125319,
                            0.0002535665435705865135866,
                            0.0002160181779769908583388,
                            0.0001784618055459532946077,
                            0.0001408990173881984930124,
                            0.0001033319034969132362968,
                            0.0000657657316592401958310,
                            0.0000282526373739346920387};

/* n = 1024 */
static Numeric x1024[512] = {0.0015332313560626384065387,
                             0.0045996796509132604743248,
                             0.0076660846940754867627839,
                             0.0107324176515422803327458,
                             0.0137986496899844401539048,
                             0.0168647519770217265449962,
                             0.0199306956814939776907024,
                             0.0229964519737322146859283,
                             0.0260619920258297325581921,
                             0.0291272870119131747190088,
                             0.0321923081084135882953009,
                             0.0352570264943374577920498,
                             0.0383214133515377145376052,
                             0.0413854398649847193632977,
                             0.0444490772230372159692514,
                             0.0475122966177132524285687,
                             0.0505750692449610682823599,
                             0.0536373663049299446784129,
                             0.0566991590022410150066456,
                             0.0597604185462580334848567,
                             0.0628211161513580991486838,
                             0.0658812230372023327000985,
                             0.0689407104290065036692117,
                             0.0719995495578116053446277,
                             0.0750577116607543749280791,
                             0.0781151679813377563695878,
                             0.0811718897697013033399379,
                             0.0842278482828915197978074,
                             0.0872830147851321356094940,
                             0.0903373605480943146797811,
                             0.0933908568511667930531222,
                             0.0964434749817259444449839,
                             0.0994951862354057706638682,
                             0.1025459619163678143852404,
                             0.1055957733375709917393206,
                             0.1086445918210413421754502,
                             0.1116923886981416930665228,
                             0.1147391353098412365177689,
                             0.1177848030069850158450139,
                             0.1208293631505633191883714,
                             0.1238727871119809777282145,
                             0.1269150462733265659711591,
                             0.1299561120276415015747167,
                             0.1329959557791890421802183,
                             0.1360345489437231767245806,
                             0.1390718629487574087024745,
                             0.1421078692338334288514767,
                             0.1451425392507896747338214,
                             0.1481758444640297746894331,
                             0.1512077563507908736360111,
                             0.1542382464014118381930443,
                             0.1572672861196013386077717,
                             0.1602948470227058049622614,
                             0.1633209006419772551419632,
                             0.1663454185228409920472972,
                             0.1693683722251631675310675,
                             0.1723897333235182105457458,
                             0.1754094734074561169859457,
                             0.1784275640817695987127083,
                             0.1814439769667610892475458,
                             0.1844586836985096036255346,
                             0.1874716559291374498981239,
                             0.1904828653270767897777182,
                             0.1934922835773360459175133,
                             0.1964998823817661533215037,
                             0.1995056334593266523810493,
                             0.2025095085463516210358758,
                             0.2055114793968154435588961,
                             0.2085115177825984134657778,
                             0.2115095954937521680517391,
                             0.2145056843387649520596422,
                             0.2174997561448267079850562,
                             0.2204917827580939905255947,
                             0.2234817360439547026834844,
                             0.2264695878872926510320010,
                             0.2294553101927519176581055,
                             0.2324388748850010462953415,
                             0.2354202539089970401627982,
                             0.2383994192302491690277166,
                             0.2413763428350825830111093,
                             0.2443509967309017306575811,
                             0.2473233529464535787923793,
                             0.2502933835320906316905658,
                             0.2532610605600337470850902,
                             0.2562263561246347465424530,
                             0.2591892423426388177365829,
                             0.2621496913534467061535080,
                             0.2651076753193766937613805,
                             0.2680631664259263621824189,
                             0.2710161368820341379053566,
                             0.2739665589203406170790369,
                             0.2769144047974496674298651,
                             0.2798596467941893048479266,
                             0.2828022572158723421886958,
                             0.2857422083925568078394062,
                             0.2886794726793061316013119,
                             0.2916140224564490954412652,
                             0.2945458301298395466682397,
                             0.2974748681311158710926665,
                             0.3004011089179602237287060,
                             0.3033245249743575146018584,
                             0.3062450888108541472266190,
                             0.3091627729648165073212094,
                             0.3120775500006891993287636,
                             0.3149893925102530283167230,
                             0.3178982731128827248285835,
                             0.3208041644558044102645582,
                             0.3237070392143528003701590,
                             0.3266068700922281444141618,
                             0.3295036298217528976399056,
                             0.3323972911641281245763845,
                             0.3352878269096896307981228,
                             0.3381752098781638207253743,
                             0.3410594129189232790587667,
                             0.3439404089112420734451077,
                             0.3468181707645507759736923,
                             0.3496926714186912011050938,
                             0.3525638838441708576370887,
                             0.3554317810424171123150528,
                             0.3582963360460310626968790,
                             0.3611575219190411168852009,
                             0.3640153117571562777424605,
                             0.3668696786880191292071420,
                             0.3697205958714585223322883,
                             0.3725680364997419586702471,
                             0.3754119737978276686304337,
                             0.3782523810236163824397703,
                             0.3810892314682027913383487,
                             0.3839224984561266966457784,
                             0.3867521553456238443366159,
                             0.3895781755288764427662286,
                             0.3924005324322633611914264,
                             0.3952191995166100067331951,
                             0.3980341502774378774318886,
                             0.4008453582452137890482864,
                             0.4036527969855987732669841,
                             0.4064564400996966449616823,
                             0.4092562612243022361850445,
                             0.4120522340321492945489319,
                             0.4148443322321580436639788,
                             0.4176325295696824033106488,
                             0.4204167998267568670171117,
                             0.4231971168223430347225035,
                             0.4259734544125757982073747,
                             0.4287457864910091769763965,
                             0.4315140869888618022816824,
                             0.4342783298752620469783905,
                             0.4370384891574927989076034,
                             0.4397945388812358755048319,
                             0.4425464531308160773358662,
                             0.4452942060294448782650898,
                             0.4480377717394637499647905,
                             0.4507771244625871184774399,
                             0.4535122384401449505463744,
                             0.4562430879533249674337895,
                             0.4589696473234144839484647,
                             0.4616918909120418704091584,
                             0.4644097931214176352731591,
                             0.4671233283945751261630457,
                             0.4698324712156108470282980,
                             0.4725371961099243891820077,
                             0.4752374776444579739565725,
                             0.4779332904279356047259052,
                             0.4806246091111018260453658,
                             0.4833114083869600876643171,
                             0.4859936629910107111699206,
                             0.4886713477014884570245255,
                             0.4913444373395996897627612,
                             0.4940129067697591391182235,
                             0.4966767308998262548534419,
                             0.4993358846813411530706387,
                             0.5019903431097601517846292,
                             0.5046400812246908935430768,
                             0.5072850741101270528831987,
                             0.5099252968946826264179220,
                             0.5125607247518258033484145,
                             0.5151913329001124142038603,
                             0.5178170966034189556133159,
                             0.5204379911711751889184691,
                             0.5230539919585963104401304,
                             0.5256650743669146912153147,
                             0.5282712138436111840258187,
                             0.5308723858826459955432696,
                             0.5334685660246891214197081,
                             0.5360597298573503421568799,
                             0.5386458530154087775915395,
                             0.5412269111810419978382210,
                             0.5438028800840546885350993,
                             0.5463737355021068682427603,
                             0.5489394532609416558499039,
                             0.5515000092346125858442412,
                             0.5540553793457104693110943,
                             0.5566055395655897985264809,
                             0.5591504659145946930157566,
                             0.5616901344622843849532002,
                             0.5642245213276582417822586,
                             0.5667536026793803239405196,
                             0.5692773547360034755778519,
                             0.5717957537661929461605442,
                             0.5743087760889495408586850,
                             0.5768163980738322976184566,
                             0.5793185961411806888254667,
                             0.5818153467623363454697137,
                             0.5843066264598643017272666,
                             0.5867924118077737578782574,
                             0.5892726794317383594853053,
                             0.5917474060093159907610475,
                             0.5942165682701680800580147,
                             0.5966801429962784154186793,
                             0.5991381070221714681281111,
                             0.6015904372351302222163013,
                             0.6040371105754135078618616,
                             0.6064781040364728366534687,
                             0.6089133946651687366701116,
                             0.6113429595619865853458987,
                             0.6137667758812519380899084,
                             0.6161848208313453506363029,
                             0.6185970716749166931046915,
                             0.6210035057290989537555048,
                             0.6234041003657215304299416,
                             0.6257988330115230076688675,
                             0.6281876811483634175098794,
                             0.6305706223134359819666081,
                             0.6329476340994783351992008,
                             0.6353186941549832233898213,
                             0.6376837801844086803419153,
                             0.6400428699483876768269192,
                             0.6423959412639372417070377,
                             0.6447429720046670528676835,
                             0.6470839401009874959981582,
                             0.6494188235403171892641570,
                             0.6517476003672899719207013,
                             0.6540702486839613549191454,
                             0.6563867466500144315669620,
                             0.6586970724829652463040876,
                             0.6610012044583676196647058,
                             0.6632991209100174274984589,
                             0.6655908002301563325302097,
                             0.6678762208696749663426270,
                             0.6701553613383155598710345,
                             0.6724282002048740205051479,
                             0.6746947160974014538975312,
                             0.6769548877034051285838219,
                             0.6792086937700488815250166,
                             0.6814561131043529626873631,
                             0.6836971245733933167806834,
                             0.6859317071045003002812397,
                             0.6881598396854568318705713,
                             0.6903815013646959744270519,
                             0.6925966712514979467122689,
                             0.6948053285161865628996815,
                             0.6970074523903250980984011,
                             0.6992030221669115780303307,
                             0.7013920172005734910243170,
                             0.7035744169077619204963997,
                             0.7057502007669450960906928,
                             0.7079193483188013616608982,
                             0.7100818391664115582779368,
                             0.7122376529754508204546805,
                             0.7143867694743797837842896,
                             0.7165291684546352021941915,
                             0.7186648297708199730232898,
                             0.7207937333408925681355609,
                             0.7229158591463558692887801,
                             0.7250311872324454059827217,
                             0.7271396977083169940167956,
                             0.7292413707472337729927181,
                             0.7313361865867526410034676,
                             0.7334241255289100847554419,
                             0.7355051679404074033764222,
                             0.7375792942527953241676460,
                             0.7396464849626580085640129,
                             0.7417067206317964465721772,
                             0.7437599818874112379620360,
                             0.7458062494222847584928838,
                             0.7478455039949627094612890,
                             0.7498777264299350488635483,
                             0.7519028976178163024713854,
                             0.7539209985155252531253957,
                             0.7559320101464640065565832,
                             0.7579359136006964320521972,
                             0.7599326900351259762879594,
                             0.7619223206736728486546595,
                             0.7639047868074505764130149,
                             0.7658800697949419280166093,
                             0.7678481510621742029486694,
                             0.7698090121028938864243967,
                             0.7717626344787406673165402,
                             0.7737089998194208176678866,
                             0.7756480898228799321603470,
                             0.7775798862554750259163361,
                             0.7795043709521459890141759,
                             0.7814215258165863961053031,
                             0.7833313328214136695271245,
                             0.7852337740083385943114429,
                             0.7871288314883341834944720,
                             0.7890164874418038921405657,
                             0.7908967241187491784979139,
                             0.7927695238389364107105941,
                             0.7946348689920631175175217,
                             0.7964927420379235813750136,
                             0.7983431255065737724458586,
                             0.8001860019984956219039900,
                             0.8020213541847606330100649,
                             0.8038491648071928284194859,
                             0.8056694166785310321906380,
                             0.8074820926825904849673728,
                             0.8092871757744237908160400,
                             0.8110846489804811942036542,
                             0.8128744953987701856100790,
                             0.8146566981990144342734272,
                             0.8164312406228120465742028,
                             0.8181981059837931485700490,
                             0.8199572776677767911993239,
                             0.8217087391329271766780945,
                             0.8234524739099092046215225,
                             0.8251884656020433364270094,
                             0.8269166978854597764628854,
                             0.8286371545092519686128428,
                             0.8303498192956294067327593,
                             0.8320546761400697575830038,
                             0.8337517090114702948057846,
                             0.8354409019522986425235764,
                             0.8371222390787428271411563,
                             0.8387957045808606359402829,
                             0.8404612827227282810625704,
                             0.8421189578425883674826439,
                             0.8437687143529971635802028,
                             0.8454105367409711729261812,
                             0.8470444095681330059047621,
                             0.8486703174708565497995875,
                             0.8502882451604114359791023,
                             0.8518981774231068028225812,
                             0.8535000991204343530350070,
                             0.8550939951892107040056078,
                             0.8566798506417190298715048,
                             0.8582576505658499939545848,
                             0.8598273801252419702463831,
                             0.8613890245594205526224495,
                             0.8629425691839373504743648,
                             0.8644879993905080694542896,
                             0.8660253006471498760336444,
                             0.8675544584983180445842596,
                             0.8690754585650418856970762,
                             0.8705882865450599544602407,
                             0.8720929282129545374252050,
                             0.8735893694202854169962281,
                             0.8750775960957229119854680,
                             0.8765575942451801930826613,
                             0.8780293499519448719952049,
                             0.8794928493768098630212838,
                             0.8809480787582035158255322,
                             0.8823950244123190181935674,
                             0.8838336727332430675485994,
                             0.8852640101930838100201983,
                             0.8866860233420980458621863,
                             0.8880996988088177000235219,
                             0.8895050233001755566829532,
                             0.8909019836016302565651375,
                             0.8922905665772905558628607,
                             0.8936707591700388455969280,
                             0.8950425484016539302522575,
                             0.8964059213729330645356690,
                             0.8977608652638132471078410,
                             0.8991073673334917701488930,
                             0.9004454149205460236240486,
                             0.9017749954430525531228459,
                             0.9030960963987053701523781,
                             0.9044087053649335137720782,
                             0.9057128099990178624646022,
                             0.9070083980382071951444166,
                             0.9082954572998335002127549,
                             0.9095739756814265315746820,
                             0.9108439411608276105410847,
                             0.9121053417963026725455006,
                             0.9133581657266545576127977,
                             0.9146024011713345435238301,
                             0.9158380364305531206273175,
                             0.9170650598853900072573273,
                             0.9182834599979034047218800,
                             0.9194932253112384908353520,
                             0.9206943444497351509745089,
                             0.9218868061190349456451742,
                             0.9230705991061873135537215,
                             0.9242457122797550091847637,
                             0.9254121345899187738936182,
                             0.9265698550685812395293315,
                             0.9277188628294700636112689,
                             0.9288591470682402950895005,
                             0.9299906970625759697264543,
                             0.9311135021722909341445515,
                             0.9322275518394288975917975,
                             0.9333328355883627104845635,
                             0.9344293430258928687940732,
                             0.9355170638413452433503852,
                             0.9365959878066680331449597,
                             0.9376661047765279417201973,
                             0.9387274046884055757416456,
                             0.9397798775626900648558921,
                             0.9408235135027729019444869,
                             0.9418583026951410028915762,
                             0.9428842354094689849902736,
                             0.9439013019987106631201510,
                             0.9449094928991897628355911,
                             0.9459087986306898495121205,
                             0.9468992097965434727052183,
                             0.9478807170837205248834878,
                             0.9488533112629158137054760,
                             0.9498169831886358470168335,
                             0.9507717237992848297519245,
                             0.9517175241172498719314184,
                             0.9526543752489854069548347,
                             0.9535822683850968193944507,
                             0.9545011948004232815044368,
                             0.9554111458541197976665483,
                             0.9563121129897384560011695,
                             0.9572040877353088863799924,
                             0.9580870617034179240840996,
                             0.9589610265912884783587268,
                             0.9598259741808576051234879,
                             0.9606818963388537831043733,
                             0.9615287850168733926613630,
                             0.9623666322514563965930439,
                             0.9631954301641612222071790,
                             0.9640151709616388439537466,
                             0.9648258469357060659245549,
                             0.9656274504634180035311332,
                             0.9664199740071397636802195,
                             0.9672034101146173227737943,
                             0.9679777514190476018682591,
                             0.9687429906391477383350273,
                             0.9694991205792235533724866,
                             0.9702461341292372147270016,
                             0.9709840242648740939883669,
                             0.9717127840476088178328839,
                             0.9724324066247705125950353,
                             0.9731428852296072415565604,
                             0.9738442131813496343496072,
                             0.9745363838852737078785517,
                             0.9752193908327628781730396,
                             0.9758932276013691625928266,
                             0.9765578878548735718130775,
                             0.9772133653433456910269459,
                             0.9778596539032024498104955,
                             0.9784967474572660801033674,
                             0.9791246400148212617670490,
                             0.9797433256716714551911835,
                             0.9803527986101944204270933,
                             0.9809530530993969223366037,
                             0.9815440834949686212533729,
                             0.9821258842393351486632952,
                             0.9826984498617103674201996,
                             0.9832617749781478160230522,
                             0.9838158542915913364912672,
                             0.9843606825919248853856025,
                             0.9848962547560215275335618,
                             0.9854225657477916120303537,
                             0.9859396106182301300994116,
                             0.9864473845054632544104222,
                             0.9869458826347940594679517,
                             0.9874351003187474227003598,
                             0.9879150329571141058970610,
                             0.9883856760369940166627304,
                             0.9888470251328386495802522,
                             0.9892990759064927068006818,
                             0.9897418241072348978090276,
                             0.9901752655718179181502248,
                             0.9905993962245076069415402,
                             0.9910142120771212830473891,
                             0.9914197092290652598522332,
                             0.9918158838673715386394944,
                             0.9922027322667336806727008,
                             0.9925802507895418581838653,
                             0.9929484358859170846092543,
                             0.9933072840937446245820355,
                             0.9936567920387065844051246,
                             0.9939969564343136839997662,
                             0.9943277740819362116746914,
                             0.9946492418708341635125525,
                             0.9949613567781865697596566,
                             0.9952641158691200113800912,
                             0.9955575162967363309635588,
                             0.9958415553021395435525955,
                             0.9961162302144619548145649,
                             0.9963815384508894965215124,
                             0.9966374775166862927999356,
                             0.9968840450052184754903082,
                             0.9971212385979772738362093,
                             0.9973490560646014135491635,
                             0.9975674952628988745188845,
                             0.9977765541388680773265018,
                             0.9979762307267185998745420,
                             0.9981665231488915727109186,
                             0.9983474296160799746514418,
                             0.9985189484272491654281575,
                             0.9986810779696581776171579,
                             0.9988338167188825964389443,
                             0.9989771632388403756649803,
                             0.9991111161818228462260355,
                             0.9992356742885348165163858,
                             0.9993508363881507486653971,
                             0.9994566013984000492749057,
                             0.9995529683257070064969677,
                             0.9996399362654382464576482,
                             0.9997175044023747284307007,
                             0.9997856720116889628341744,
                             0.9998444384611711916084367,
                             0.9998938032169419878731474,
                             0.9999337658606177711221103,
                             0.9999643261538894550943330,
                             0.9999854843850284447675914,
                             0.9999972450545584403516182};
static Numeric w1024[512] = {0.0030664603092439082115513,
                             0.0030664314747171934849726,
                             0.0030663738059349007324470,
                             0.0030662873034393008056861,
                             0.0030661719680437936084028,
                             0.0030660278008329004477528,
                             0.0030658548031622538363679,
                             0.0030656529766585847450783,
                             0.0030654223232197073064431,
                             0.0030651628450145009692318,
                             0.0030648745444828901040266,
                             0.0030645574243358210601357,
                             0.0030642114875552366740338,
                             0.0030638367373940482295700,
                             0.0030634331773761048702058,
                             0.0030630008112961604635720,
                             0.0030625396432198379186545,
                             0.0030620496774835909559465,
                             0.0030615309186946633309249,
                             0.0030609833717310455112352,
                             0.0030604070417414288079918,
                             0.0030598019341451569616257,
                             0.0030591680546321751827342,
                             0.0030585054091629766484119,
                             0.0030578140039685464545661,
                             0.0030570938455503030247440,
                             0.0030563449406800369760227,
                             0.0030555672963998474425352,
                             0.0030547609200220758572342,
                             0.0030539258191292371925135,
                             0.0030530620015739486603347,
                             0.0030521694754788558725307,
                             0.0030512482492365564619779,
                             0.0030502983315095211653578,
                             0.0030493197312300123682482,
                             0.0030483124576000001133114,
                             0.0030472765200910755723677,
                             0.0030462119284443619831693,
                             0.0030451186926704230517109,
                             0.0030439968230491688209395,
                             0.0030428463301297590067471,
                             0.0030416672247305038021562,
                             0.0030404595179387621506312,
                             0.0030392232211108374894710,
                             0.0030379583458718709642643,
                             0.0030366649041157321154111,
                             0.0030353429080049070377385,
                             0.0030339923699703840142628,
                             0.0030326133027115366251721,
                             0.0030312057191960043331307,
                             0.0030297696326595705460252,
                             0.0030283050566060381583022,
                             0.0030268120048071025720655,
                             0.0030252904913022221991274,
                             0.0030237405303984864452325,
                             0.0030221621366704811776946,
                             0.0030205553249601516777118,
                             0.0030189201103766630786495,
                             0.0030172565082962582916016,
                             0.0030155645343621134195681,
                             0.0030138442044841906616068,
                             0.0030120955348390887083441,
                             0.0030103185418698906302495,
                             0.0030085132422860092601062,
                             0.0030066796530630300711306,
                             0.0030048177914425515522176,
                             0.0030029276749320230818149,
                             0.0030010093213045803019478,
                             0.0029990627485988779939449,
                             0.0029970879751189204574353,
                             0.0029950850194338893942123,
                             0.0029930539003779692985814,
                             0.0029909946370501703558363,
                             0.0029889072488141488505262,
                             0.0029867917552980250862041,
                             0.0029846481763941988183689,
                             0.0029824765322591622023349,
                             0.0029802768433133102577897,
                             0.0029780491302407488518214,
                             0.0029757934139891002022209,
                             0.0029735097157693059028890,
                             0.0029711980570554274731990,
                             0.0029688584595844444331918,
                             0.0029664909453560499065010,
                             0.0029640955366324437529314,
                             0.0029616722559381232326340,
                             0.0029592211260596712038487,
                             0.0029567421700455418562030,
                             0.0029542354112058439815854,
                             0.0029517008731121217846274,
                             0.0029491385795971332348581,
                             0.0029465485547546259626151,
                             0.0029439308229391107008170,
                             0.0029412854087656322747309,
                             0.0029386123371095381418860,
                             0.0029359116331062444843108,
                             0.0029331833221509998552933,
                             0.0029304274298986463828860,
                             0.0029276439822633785324025,
                             0.0029248330054184994301727,
                             0.0029219945257961747508486,
                             0.0029191285700871841705750,
                             0.0029162351652406703883623,
                             0.0029133143384638857180205,
                             0.0029103661172219362530391,
                             0.0029073905292375236068160,
                             0.0029043876024906842306667,
                             0.0029013573652185263120627,
                             0.0028982998459149642555740,
                             0.0028952150733304507490135,
                             0.0028921030764717064173001,
                             0.0028889638846014470665859,
                             0.0028857975272381085212091,
                             0.0028826040341555690560623,
                             0.0028793834353828694269858,
                             0.0028761357612039305018167,
                             0.0028728610421572684947521,
                             0.0028695593090357078067012,
                             0.0028662305928860914743281,
                             0.0028628749250089892305081,
                             0.0028594923369584031789413,
                             0.0028560828605414710856927,
                             0.0028526465278181672904478,
                             0.0028491833711010012402964,
                             0.0028456934229547136488796,
                             0.0028421767161959702837564,
                             0.0028386332838930533848701,
                             0.0028350631593655507170153,
                             0.0028314663761840422592303,
                             0.0028278429681697845340603,
                             0.0028241929693943925796601,
                             0.0028205164141795195677262,
                             0.0028168133370965340702726,
                             0.0028130837729661949782821,
                             0.0028093277568583240752928,
                             0.0028055453240914762689974,
                             0.0028017365102326074839556,
                             0.0027979013510967402185435,
                             0.0027940398827466267692845,
                             0.0027901521414924101257281,
                             0.0027862381638912825390663,
                             0.0027822979867471417676962,
                             0.0027783316471102450029635,
                             0.0027743391822768604783394,
                             0.0027703206297889167653083,
                             0.0027662760274336497592617,
                             0.0027622054132432473587211,
                             0.0027581088254944918412282,
                             0.0027539863027083999392661,
                             0.0027498378836498606195970,
                             0.0027456636073272705694208,
                             0.0027414635129921673927833,
                             0.0027372376401388605206822,
                             0.0027329860285040598383428,
                             0.0027287087180665020331547,
                             0.0027244057490465746667821,
                             0.0027200771619059379749851,
                             0.0027157229973471443987056,
                             0.0027113432963132558499974,
                             0.0027069380999874587163979,
                             0.0027025074497926766073634,
                             0.0026980513873911808464073,
                             0.0026935699546841987126055,
                             0.0026890631938115194351518,
                             0.0026845311471510979446691,
                             0.0026799738573186563850015,
                             0.0026753913671672833892344,
                             0.0026707837197870311237119,
                             0.0026661509585045101038391,
                             0.0026614931268824817854798,
                             0.0026568102687194489357814,
                             0.0026521024280492437872770,
                             0.0026473696491406139791397,
                             0.0026426119764968062894804,
                             0.0026378294548551481626046,
                             0.0026330221291866270351630,
                             0.0026281900446954674651512,
                             0.0026233332468187060677353,
                             0.0026184517812257642618999,
                             0.0026135456938180188319369,
                             0.0026086150307283703078113,
                             0.0026036598383208091684657,
                             0.0025986801631899798721388,
                             0.0025936760521607427178014,
                             0.0025886475522877335418257,
                             0.0025835947108549212540321,
                             0.0025785175753751632172710,
                             0.0025734161935897584747222,
                             0.0025682906134679988291122,
                             0.0025631408832067177780710,
                             0.0025579670512298373098703,
                             0.0025527691661879125638030,
                             0.0025475472769576743594882,
                             0.0025423014326415695994010,
                             0.0025370316825672995489502,
                             0.0025317380762873559984451,
                             0.0025264206635785553113127,
                             0.0025210794944415703629476,
                             0.0025157146191004603745948,
                             0.0025103260880021986466869,
                             0.0025049139518161981960773,
                             0.0024994782614338353016280,
                             0.0024940190679679709626349,
                             0.0024885364227524702745874,
                             0.0024830303773417197267843,
                             0.0024775009835101424263432,
                             0.0024719482932517112531633,
                             0.0024663723587794599504176,
                             0.0024607732325249921551741,
                             0.0024551509671379883737605,
                             0.0024495056154857109065099,
                             0.0024438372306525067265426,
                             0.0024381458659393083172574,
                             0.0024324315748631324732279,
                             0.0024266944111565770692147,
                             0.0024209344287673158020275,
                             0.0024151516818575909099866,
                             0.0024093462248037038747545,
                             0.0024035181121955041103265,
                             0.0023976673988358756439882,
                             0.0023917941397402217940673,
                             0.0023858983901359478493246,
                             0.0023799802054619417548485,
                             0.0023740396413680528093376,
                             0.0023680767537145683786720,
                             0.0023620915985716886306938,
                             0.0023560842322189992961374,
                             0.0023500547111449424606655,
                             0.0023440030920462853929883,
                             0.0023379294318275874140606,
                             0.0023318337876006648123684,
                             0.0023257162166840538103394,
                             0.0023195767766024715869239,
                             0.0023134155250862753614165,
                             0.0023072325200709195436049,
                             0.0023010278196964109553481,
                             0.0022948014823067621287099,
                             0.0022885535664494426857857,
                             0.0022822841308748288053830,
                             0.0022759932345356507817318,
                             0.0022696809365864386804193,
                             0.0022633472963829660967620,
                             0.0022569923734816920218464,
                             0.0022506162276392008214839,
                             0.0022442189188116403333494,
                             0.0022378005071541580875846,
                             0.0022313610530203356561684,
                             0.0022249006169616211363732,
                             0.0022184192597267597736437,
                             0.0022119170422612227292520,
                             0.0022053940257066339981005,
                             0.0021988502714001954820607,
                             0.0021922858408741102242558,
                             0.0021857007958550038097087,
                             0.0021790951982633439377969,
                             0.0021724691102128581719720,
                             0.0021658225940099498722195,
                             0.0021591557121531123157498,
                             0.0021524685273323410114303,
                             0.0021457611024285442134846,
                             0.0021390335005129516400021,
                             0.0021322857848465214018174,
                             0.0021255180188793451473363,
                             0.0021187302662500514289029,
                             0.0021119225907852072963166,
                             0.0021050950564987181231273,
                             0.0020982477275912256713511,
                             0.0020913806684495044002679,
                             0.0020844939436458560249764,
                             0.0020775876179375023304007,
                             0.0020706617562659762464561,
                             0.0020637164237565111901030,
                             0.0020567516857174286800274,
                             0.0020497676076395242297101,
                             0.0020427642551954515246552,
                             0.0020357416942391048895728,
                             0.0020286999908050000513193,
                             0.0020216392111076532034194,
                             0.0020145594215409583780096,
                             0.0020074606886775631310555,
                             0.0020003430792682425467160,
                             0.0019932066602412715667394,
                             0.0019860514987017956507927,
                             0.0019788776619311997736447,
                             0.0019716852173864757651327,
                             0.0019644742326995879988655,
                             0.0019572447756768374356240,
                             0.0019499969142982240274419,
                             0.0019427307167168074883601,
                             0.0019354462512580664378677,
                             0.0019281435864192559230531,
                             0.0019208227908687633255086,
                             0.0019134839334454626590447,
                             0.0019061270831580672642844,
                             0.0018987523091844809062265,
                             0.0018913596808711472808775,
                             0.0018839492677323979370705,
                             0.0018765211394497986196010,
                             0.0018690753658714940398285,
                             0.0018616120170115510799024,
                             0.0018541311630493004367905,
                             0.0018466328743286767122991,
                             0.0018391172213575569552912,
                             0.0018315842748070976623218,
                             0.0018240341055110702429247,
                             0.0018164667844651949558009,
                             0.0018088823828264733221690,
                             0.0018012809719125190225581,
                             0.0017936626232008872833327,
                             0.0017860274083284027592567,
                             0.0017783753990904859184165,
                             0.0017707066674404779358362,
                             0.0017630212854889641021349,
                             0.0017553193255030957535871,
                             0.0017476008599059107299616,
                             0.0017398659612756523665312,
                             0.0017321147023450870266539,
                             0.0017243471560008201813452,
                             0.0017165633952826110422716,
                             0.0017087634933826857546100,
                             0.0017009475236450491562317,
                             0.0016931155595647951096823,
                             0.0016852676747874154134422,
                             0.0016774039431081072989678,
                             0.0016695244384710795200224,
                             0.0016616292349688570408253,
                             0.0016537184068415843295541,
                             0.0016457920284763272637533,
                             0.0016378501744063736542136,
                             0.0016298929193105323938983,
                             0.0016219203380124312385075,
                             0.0016139325054798132252838,
                             0.0016059294968238317366751,
                             0.0015979113872983442154825,
                             0.0015898782522992045381361,
                             0.0015818301673635540527516,
                             0.0015737672081691112886347,
                             0.0015656894505334603439125,
                             0.0015575969704133379579831,
                             0.0015494898439039192754876,
                             0.0015413681472381023085203,
                             0.0015332319567857911038062,
                             0.0015250813490531776215856,
                             0.0015169164006820223329593,
                             0.0015087371884489335424584,
                             0.0015005437892646454426166,
                             0.0014923362801732949073323,
                             0.0014841147383516970308228,
                             0.0014758792411086194189814,
                             0.0014676298658840552399621,
                             0.0014593666902484950408286,
                             0.0014510897919021973371136,
                             0.0014427992486744579821480,
                             0.0014344951385228783230315,
                             0.0014261775395326321501237,
                             0.0014178465299157314469528,
                             0.0014095021880102909474427,
                             0.0014011445922797915073771,
                             0.0013927738213123422970256,
                             0.0013843899538199418218713,
                             0.0013759930686377377783877,
                             0.0013675832447232857518263,
                             0.0013591605611558067629844,
                             0.0013507250971354436709363,
                             0.0013422769319825164387192,
                             0.0013338161451367762689788,
                             0.0013253428161566586165863,
                             0.0013168570247185350852537,
                             0.0013083588506159642151809,
                             0.0012998483737589411687807,
                             0.0012913256741731463215379,
                             0.0012827908319991927650686,
                             0.0012742439274918727294554,
                             0.0012656850410194029319476,
                             0.0012571142530626688591208,
                             0.0012485316442144679896043,
                             0.0012399372951787519644928,
                             0.0012313312867698677125706,
                             0.0012227136999117975374834,
                             0.0012140846156373981740056,
                             0.0012054441150876388205601,
                             0.0011967922795108381551550,
                             0.0011881291902619003419159,
                             0.0011794549288015500353964,
                             0.0011707695766955663898644,
                             0.0011620732156140160807669,
                             0.0011533659273304853455891,
                             0.0011446477937213110513287,
                             0.0011359188967648107958214,
                             0.0011271793185405120501566,
                             0.0011184291412283803494364,
                             0.0011096684471080465391373,
                             0.0011008973185580330843445,
                             0.0010921158380549794491381,
                             0.0010833240881728665534171,
                             0.0010745221515822403144596,
                             0.0010657101110494342805238,
                             0.0010568880494357913638046,
                             0.0010480560496968846800697,
                             0.0010392141948817375023057,
                             0.0010303625681320423357186,
                             0.0010215012526813791214350,
                             0.0010126303318544325762649,
                             0.0010037498890662086758941,
                             0.0009948600078212502888805,
                             0.0009859607717128519688418,
                             0.0009770522644222739122264,
                             0.0009681345697179550890732,
                             0.0009592077714547255541688,
                             0.0009502719535730179460261,
                             0.0009413272000980781811114,
                             0.0009323735951391753507612,
                             0.0009234112228888108282347,
                             0.0009144401676219265933610,
                             0.0009054605136951127822476,
                             0.0008964723455458144695262,
                             0.0008874757476915376906225,
                             0.0008784708047290547115472,
                             0.0008694576013336085537138,
                             0.0008604362222581167813022,
                             0.0008514067523323745586954,
                             0.0008423692764622569855308,
                             0.0008333238796289207169173,
                             0.0008242706468880048763834,
                             0.0008152096633688312691343,
                             0.0008061410142736039032099,
                             0.0007970647848766078261514,
                             0.0007879810605234072847989,
                             0.0007788899266300432158601,
                             0.0007697914686822300749096,
                             0.0007606857722345520114971,
                             0.0007515729229096583980656,
                             0.0007424530063974587204051,
                             0.0007333261084543168373926,
                             0.0007241923149022446178008,
                             0.0007150517116280949619884,
                             0.0007059043845827542163241,
                             0.0006967504197803339882351,
                             0.0006875899032973623698204,
                             0.0006784229212719745780188,
                             0.0006692495599031030193850,
                             0.0006600699054496667875923,
                             0.0006508840442297606018626,
                             0.0006416920626198431946113,
                             0.0006324940470539251567018,
                             0.0006232900840227562488244,
                             0.0006140802600730121876541,
                             0.0006048646618064809156059,
                             0.0005956433758792483631993,
                             0.0005864164890008837132649,
                             0.0005771840879336241764943,
                             0.0005679462594915592881427,
                             0.0005587030905398147360662,
                             0.0005494546679937357307118,
                             0.0005402010788180699282026,
                             0.0005309424100261499182844,
                             0.0005216787486790752896494,
                             0.0005124101818848942860548,
                             0.0005031367967977850677401,
                             0.0004938586806172365939677,
                             0.0004845759205872291441124,
                             0.0004752886039954144966810,
                             0.0004659968181722957880391,
                             0.0004567006504904070755681,
                             0.0004474001883634926336095,
                             0.0004380955192456860150653,
                             0.0004287867306306889171352,
                             0.0004194739100509498966958,
                             0.0004101571450768429896514,
                             0.0004008365233158462997325,
                             0.0003915121324117206363681,
                             0.0003821840600436882993131,
                             0.0003728523939256121308821,
                             0.0003635172218051749865499,
                             0.0003541786314630598135175,
                             0.0003448367107121305776064,
                             0.0003354915473966143456333,
                             0.0003261432293912849189248,
                             0.0003167918446006485317858,
                             0.0003074374809581322877037,
                             0.0002980802264252762217455,
                             0.0002887201689909301727620,
                             0.0002793573966704570567274,
                             0.0002699919975049447012834,
                             0.0002606240595604292032823,
                             0.0002512536709271339139118,
                             0.0002418809197187298044384,
                             0.0002325058940716253739001,
                             0.0002231286821442978268308,
                             0.0002137493721166826096154,
                             0.0002043680521896465790359,
                             0.0001949848105845827899210,
                             0.0001855997355431850062940,
                             0.0001762129153274925249194,
                             0.0001668244382203495280013,
                             0.0001574343925265138930609,
                             0.0001480428665748079976500,
                             0.0001386499487219861751244,
                             0.0001292557273595155266326,
                             0.0001198602909254695827354,
                             0.0001104637279257437565603,
                             0.0001010661269730276014588,
                             0.0000916675768613669107254,
                             0.0000822681667164572752810,
                             0.0000728679863190274661367,
                             0.0000634671268598044229933,
                             0.0000540656828939400071988,
                             0.0000446637581285753393838,
                             0.0000352614859871986975067,
                             0.0000258591246764618586716,
                             0.0000164577275798968681068,
                             0.0000070700764101825898713};

/* n = 3 */
static Numeric x3[2] = {0.0000000000000000000000000,
                        0.7745966692414833770358531};
static Numeric w3[2] = {0.8888888888888888888888889,
                        0.5555555555555555555555556};

/* n = 5 */
static Numeric x5[3] = {0.0000000000000000000000000,
                        0.5384693101056830910363144,
                        0.9061798459386639927976269};
static Numeric w5[3] = {0.5688888888888888888888889,
                        0.4786286704993664680412915,
                        0.2369268850561890875142640};

/* n = 7 */
static Numeric x7[4] = {0.0000000000000000000000000,
                        0.4058451513773971669066064,
                        0.7415311855993944398638648,
                        0.9491079123427585245261897};
static Numeric w7[4] = {0.4179591836734693877551020,
                        0.3818300505051189449503698,
                        0.2797053914892766679014678,
                        0.1294849661688696932706114};

/* n = 9 */
static Numeric x9[5] = {0.0000000000000000000000000,
                        0.3242534234038089290385380,
                        0.6133714327005903973087020,
                        0.8360311073266357942994298,
                        0.9681602395076260898355762};
static Numeric w9[5] = {0.3302393550012597631645251,
                        0.3123470770400028400686304,
                        0.2606106964029354623187429,
                        0.1806481606948574040584720,
                        0.0812743883615744119718922};

/* n = 11 */
static Numeric x11[6] = {0.0000000000000000000000000,
                         0.2695431559523449723315320,
                         0.5190961292068118159257257,
                         0.7301520055740493240934163,
                         0.8870625997680952990751578,
                         0.9782286581460569928039380};
static Numeric w11[6] = {0.2729250867779006307144835,
                         0.2628045445102466621806889,
                         0.2331937645919904799185237,
                         0.1862902109277342514260976,
                         0.1255803694649046246346943,
                         0.0556685671161736664827537};

/* n = 13 */
static Numeric x13[7] = {0.0000000000000000000000000,
                         0.2304583159551347940655281,
                         0.4484927510364468528779129,
                         0.6423493394403402206439846,
                         0.8015780907333099127942065,
                         0.9175983992229779652065478,
                         0.9841830547185881494728294};
static Numeric w13[7] = {0.2325515532308739101945895,
                         0.2262831802628972384120902,
                         0.2078160475368885023125232,
                         0.1781459807619457382800467,
                         0.1388735102197872384636018,
                         0.0921214998377284479144218,
                         0.0404840047653158795200216};

/* n = 15 */
static Numeric x15[8] = {0.0000000000000000000000000,
                         0.2011940939974345223006283,
                         0.3941513470775633698972074,
                         0.5709721726085388475372267,
                         0.7244177313601700474161861,
                         0.8482065834104272162006483,
                         0.9372733924007059043077589,
                         0.9879925180204854284895657};
static Numeric w15[8] = {0.2025782419255612728806202,
                         0.1984314853271115764561183,
                         0.1861610000155622110268006,
                         0.1662692058169939335532009,
                         0.1395706779261543144478048,
                         0.1071592204671719350118695,
                         0.0703660474881081247092674,
                         0.0307532419961172683546284};

/* n = 17 */
static Numeric x17[9] = {0.0000000000000000000000000,
                         0.1784841814958478558506775,
                         0.3512317634538763152971855,
                         0.5126905370864769678862466,
                         0.6576711592166907658503022,
                         0.7815140038968014069252301,
                         0.8802391537269859021229557,
                         0.9506755217687677612227170,
                         0.9905754753144173356754340};
static Numeric w17[9] = {0.1794464703562065254582656,
                         0.1765627053669926463252710,
                         0.1680041021564500445099707,
                         0.1540457610768102880814316,
                         0.1351363684685254732863200,
                         0.1118838471934039710947884,
                         0.0850361483171791808835354,
                         0.0554595293739872011294402,
                         0.0241483028685479319601100};

/* n = 19 */
static Numeric x19[10] = {0.0000000000000000000000000,
                          0.1603586456402253758680961,
                          0.3165640999636298319901173,
                          0.4645707413759609457172671,
                          0.6005453046616810234696382,
                          0.7209661773352293786170959,
                          0.8227146565371428249789225,
                          0.9031559036148179016426609,
                          0.9602081521348300308527788,
                          0.9924068438435844031890177};
static Numeric w19[10] = {0.1610544498487836959791636,
                          0.1589688433939543476499564,
                          0.1527660420658596667788554,
                          0.1426067021736066117757461,
                          0.1287539625393362276755158,
                          0.1115666455473339947160239,
                          0.0914900216224499994644621,
                          0.0690445427376412265807083,
                          0.0448142267656996003328382,
                          0.0194617882297264770363120};

/* Merge all together */
static gsl_integration_glfixed_table glaw[] =
        {
                {2,    (Numeric *)x2,    (Numeric *)w2,    1 /* precomputed */ },
                {3,    (Numeric *)x3,    (Numeric *)w3,    1},
                {4,    (Numeric *)x4,    (Numeric *)w4,    1},
                {5,    (Numeric *)x5,    (Numeric *)w5,    1},
                {6,    (Numeric *)x6,    (Numeric *)w6,    1},
                {7,    (Numeric *)x7,    (Numeric *)w7,    1},
                {8,    (Numeric *)x8,    (Numeric *)w8,    1},
                {9,    (Numeric *)x9,    (Numeric *)w9,    1},
                {10,   (Numeric *)x10,   (Numeric *)w10,   1},
                {11,   (Numeric *)x11,   (Numeric *)w11,   1},
                {12,   (Numeric *)x12,   (Numeric *)w12,   1},
                {13,   (Numeric *)x13,   (Numeric *)w13,   1},
                {14,   (Numeric *)x14,   (Numeric *)w14,   1},
                {15,   (Numeric *)x15,   (Numeric *)w15,   1},
                {16,   (Numeric *)x16,   (Numeric *)w16,   1},
                {17,   (Numeric *)x17,   (Numeric *)w17,   1},
                {18,   (Numeric *)x18,   (Numeric *)w18,   1},
                {19,   (Numeric *)x19,   (Numeric *)w19,   1},
                {20,   (Numeric *)x20,   (Numeric *)w20,   1},
                {32,   (Numeric *)x32,   (Numeric *)w32,   1},
                {64,   (Numeric *)x64,   (Numeric *)w64,   1},
                {96,   (Numeric *)x96,   (Numeric *)w96,   1},
                {100,  (Numeric *)x100,  (Numeric *)w100,  1},
                {128,  (Numeric *)x128,  (Numeric *)w128,  1},
                {256,  (Numeric *)x256,  (Numeric *)w256,  1},
                {512,  (Numeric *)x512,  (Numeric *)w512,  1},
                {1024, (Numeric *)x1024, (Numeric *)w1024, 1}
        };


/* Look up table for fast calculation of Legendre polynomial for n<1024 */
/* ltbl[k] = 1.0 - 1.0/(Numeric)k; */
static Numeric ltbl[1024] = {0.00000000000000000000, 0.00000000000000000000,
                             0.50000000000000000000, 0.66666666666666674000,
                             0.75000000000000000000, 0.80000000000000004000,
                             0.83333333333333337000, 0.85714285714285721000,
                             0.87500000000000000000, 0.88888888888888884000,
                             0.90000000000000002000, 0.90909090909090906000,
                             0.91666666666666663000, 0.92307692307692313000,
                             0.92857142857142860000, 0.93333333333333335000,
                             0.93750000000000000000, 0.94117647058823528000,
                             0.94444444444444442000, 0.94736842105263164000,
                             0.94999999999999996000, 0.95238095238095233000,
                             0.95454545454545459000, 0.95652173913043481000,
                             0.95833333333333337000, 0.95999999999999996000,
                             0.96153846153846156000, 0.96296296296296302000,
                             0.96428571428571430000, 0.96551724137931039000,
                             0.96666666666666667000, 0.96774193548387100000,
                             0.96875000000000000000, 0.96969696969696972000,
                             0.97058823529411764000, 0.97142857142857142000,
                             0.97222222222222221000, 0.97297297297297303000,
                             0.97368421052631582000, 0.97435897435897434000,
                             0.97499999999999998000, 0.97560975609756095000,
                             0.97619047619047616000, 0.97674418604651159000,
                             0.97727272727272729000, 0.97777777777777775000,
                             0.97826086956521741000, 0.97872340425531912000,
                             0.97916666666666663000, 0.97959183673469385000,
                             0.97999999999999998000, 0.98039215686274506000,
                             0.98076923076923073000, 0.98113207547169812000,
                             0.98148148148148151000, 0.98181818181818181000,
                             0.98214285714285710000, 0.98245614035087714000,
                             0.98275862068965514000, 0.98305084745762716000,
                             0.98333333333333328000, 0.98360655737704916000,
                             0.98387096774193550000, 0.98412698412698418000,
                             0.98437500000000000000, 0.98461538461538467000,
                             0.98484848484848486000, 0.98507462686567160000,
                             0.98529411764705888000, 0.98550724637681164000,
                             0.98571428571428577000, 0.98591549295774650000,
                             0.98611111111111116000, 0.98630136986301364000,
                             0.98648648648648651000, 0.98666666666666669000,
                             0.98684210526315785000, 0.98701298701298701000,
                             0.98717948717948723000, 0.98734177215189878000,
                             0.98750000000000004000, 0.98765432098765427000,
                             0.98780487804878048000, 0.98795180722891562000,
                             0.98809523809523814000, 0.98823529411764710000,
                             0.98837209302325579000, 0.98850574712643680000,
                             0.98863636363636365000, 0.98876404494382020000,
                             0.98888888888888893000, 0.98901098901098905000,
                             0.98913043478260865000, 0.98924731182795700000,
                             0.98936170212765961000, 0.98947368421052628000,
                             0.98958333333333337000, 0.98969072164948457000,
                             0.98979591836734693000, 0.98989898989898994000,
                             0.98999999999999999000, 0.99009900990099009000,
                             0.99019607843137258000, 0.99029126213592233000,
                             0.99038461538461542000, 0.99047619047619051000,
                             0.99056603773584906000, 0.99065420560747663000,
                             0.99074074074074070000, 0.99082568807339455000,
                             0.99090909090909096000, 0.99099099099099097000,
                             0.99107142857142860000, 0.99115044247787609000,
                             0.99122807017543857000, 0.99130434782608701000,
                             0.99137931034482762000, 0.99145299145299148000,
                             0.99152542372881358000, 0.99159663865546221000,
                             0.99166666666666670000, 0.99173553719008267000,
                             0.99180327868852458000, 0.99186991869918695000,
                             0.99193548387096775000, 0.99199999999999999000,
                             0.99206349206349209000, 0.99212598425196852000,
                             0.99218750000000000000, 0.99224806201550386000,
                             0.99230769230769234000, 0.99236641221374045000,
                             0.99242424242424243000, 0.99248120300751874000,
                             0.99253731343283580000, 0.99259259259259258000,
                             0.99264705882352944000, 0.99270072992700731000,
                             0.99275362318840576000, 0.99280575539568350000,
                             0.99285714285714288000, 0.99290780141843971000,
                             0.99295774647887325000, 0.99300699300699302000,
                             0.99305555555555558000, 0.99310344827586206000,
                             0.99315068493150682000, 0.99319727891156462000,
                             0.99324324324324320000, 0.99328859060402686000,
                             0.99333333333333329000, 0.99337748344370858000,
                             0.99342105263157898000, 0.99346405228758172000,
                             0.99350649350649356000, 0.99354838709677418000,
                             0.99358974358974361000, 0.99363057324840764000,
                             0.99367088607594933000, 0.99371069182389937000,
                             0.99375000000000002000, 0.99378881987577639000,
                             0.99382716049382713000, 0.99386503067484666000,
                             0.99390243902439024000, 0.99393939393939390000,
                             0.99397590361445787000, 0.99401197604790414000,
                             0.99404761904761907000, 0.99408284023668636000,
                             0.99411764705882355000, 0.99415204678362579000,
                             0.99418604651162790000, 0.99421965317919070000,
                             0.99425287356321834000, 0.99428571428571433000,
                             0.99431818181818177000, 0.99435028248587576000,
                             0.99438202247191010000, 0.99441340782122900000,
                             0.99444444444444446000, 0.99447513812154698000,
                             0.99450549450549453000, 0.99453551912568305000,
                             0.99456521739130432000, 0.99459459459459465000,
                             0.99462365591397850000, 0.99465240641711228000,
                             0.99468085106382975000, 0.99470899470899465000,
                             0.99473684210526314000, 0.99476439790575921000,
                             0.99479166666666663000, 0.99481865284974091000,
                             0.99484536082474229000, 0.99487179487179489000,
                             0.99489795918367352000, 0.99492385786802029000,
                             0.99494949494949492000, 0.99497487437185927000,
                             0.99500000000000000000, 0.99502487562189057000,
                             0.99504950495049505000, 0.99507389162561577000,
                             0.99509803921568629000, 0.99512195121951219000,
                             0.99514563106796117000, 0.99516908212560384000,
                             0.99519230769230771000, 0.99521531100478466000,
                             0.99523809523809526000, 0.99526066350710896000,
                             0.99528301886792447000, 0.99530516431924887000,
                             0.99532710280373837000, 0.99534883720930234000,
                             0.99537037037037035000, 0.99539170506912444000,
                             0.99541284403669728000, 0.99543378995433796000,
                             0.99545454545454548000, 0.99547511312217196000,
                             0.99549549549549554000, 0.99551569506726456000,
                             0.99553571428571430000, 0.99555555555555553000,
                             0.99557522123893805000, 0.99559471365638763000,
                             0.99561403508771928000, 0.99563318777292575000,
                             0.99565217391304350000, 0.99567099567099571000,
                             0.99568965517241381000, 0.99570815450643779000,
                             0.99572649572649574000, 0.99574468085106382000,
                             0.99576271186440679000, 0.99578059071729963000,
                             0.99579831932773111000, 0.99581589958159000000,
                             0.99583333333333335000, 0.99585062240663902000,
                             0.99586776859504134000, 0.99588477366255146000,
                             0.99590163934426235000, 0.99591836734693873000,
                             0.99593495934959353000, 0.99595141700404854000,
                             0.99596774193548387000, 0.99598393574297184000,
                             0.99600000000000000000, 0.99601593625498008000,
                             0.99603174603174605000, 0.99604743083003955000,
                             0.99606299212598426000, 0.99607843137254903000,
                             0.99609375000000000000, 0.99610894941634243000,
                             0.99612403100775193000, 0.99613899613899615000,
                             0.99615384615384617000, 0.99616858237547889000,
                             0.99618320610687028000, 0.99619771863117867000,
                             0.99621212121212122000, 0.99622641509433962000,
                             0.99624060150375937000, 0.99625468164794007000,
                             0.99626865671641796000, 0.99628252788104088000,
                             0.99629629629629635000, 0.99630996309963105000,
                             0.99632352941176472000, 0.99633699633699635000,
                             0.99635036496350360000, 0.99636363636363634000,
                             0.99637681159420288000, 0.99638989169675085000,
                             0.99640287769784175000, 0.99641577060931896000,
                             0.99642857142857144000, 0.99644128113879005000,
                             0.99645390070921991000, 0.99646643109540634000,
                             0.99647887323943662000, 0.99649122807017543000,
                             0.99650349650349646000, 0.99651567944250874000,
                             0.99652777777777779000, 0.99653979238754320000,
                             0.99655172413793103000, 0.99656357388316152000,
                             0.99657534246575341000, 0.99658703071672350000,
                             0.99659863945578231000, 0.99661016949152548000,
                             0.99662162162162160000, 0.99663299663299665000,
                             0.99664429530201337000, 0.99665551839464883000,
                             0.99666666666666670000, 0.99667774086378735000,
                             0.99668874172185429000, 0.99669966996699666000,
                             0.99671052631578949000, 0.99672131147540988000,
                             0.99673202614379086000, 0.99674267100977199000,
                             0.99675324675324672000, 0.99676375404530748000,
                             0.99677419354838714000, 0.99678456591639875000,
                             0.99679487179487181000, 0.99680511182108622000,
                             0.99681528662420382000, 0.99682539682539684000,
                             0.99683544303797467000, 0.99684542586750791000,
                             0.99685534591194969000, 0.99686520376175547000,
                             0.99687499999999996000, 0.99688473520249221000,
                             0.99689440993788825000, 0.99690402476780182000,
                             0.99691358024691357000, 0.99692307692307691000,
                             0.99693251533742333000, 0.99694189602446481000,
                             0.99695121951219512000, 0.99696048632218848000,
                             0.99696969696969695000, 0.99697885196374625000,
                             0.99698795180722888000, 0.99699699699699695000,
                             0.99700598802395213000, 0.99701492537313430000,
                             0.99702380952380953000, 0.99703264094955490000,
                             0.99704142011834318000, 0.99705014749262533000,
                             0.99705882352941178000, 0.99706744868035191000,
                             0.99707602339181289000, 0.99708454810495628000,
                             0.99709302325581395000, 0.99710144927536237000,
                             0.99710982658959535000, 0.99711815561959649000,
                             0.99712643678160917000, 0.99713467048710602000,
                             0.99714285714285711000, 0.99715099715099720000,
                             0.99715909090909094000, 0.99716713881019825000,
                             0.99717514124293782000, 0.99718309859154930000,
                             0.99719101123595510000, 0.99719887955182074000,
                             0.99720670391061450000, 0.99721448467966578000,
                             0.99722222222222223000, 0.99722991689750695000,
                             0.99723756906077343000, 0.99724517906336085000,
                             0.99725274725274726000, 0.99726027397260275000,
                             0.99726775956284153000, 0.99727520435967298000,
                             0.99728260869565222000, 0.99728997289972898000,
                             0.99729729729729732000, 0.99730458221024254000,
                             0.99731182795698925000, 0.99731903485254692000,
                             0.99732620320855614000, 0.99733333333333329000,
                             0.99734042553191493000, 0.99734748010610075000,
                             0.99735449735449733000, 0.99736147757255933000,
                             0.99736842105263157000, 0.99737532808398954000,
                             0.99738219895287961000, 0.99738903394255873000,
                             0.99739583333333337000, 0.99740259740259740000,
                             0.99740932642487046000, 0.99741602067183466000,
                             0.99742268041237114000, 0.99742930591259638000,
                             0.99743589743589745000, 0.99744245524296671000,
                             0.99744897959183676000, 0.99745547073791352000,
                             0.99746192893401020000, 0.99746835443037973000,
                             0.99747474747474751000, 0.99748110831234260000,
                             0.99748743718592969000, 0.99749373433583965000,
                             0.99750000000000005000, 0.99750623441396513000,
                             0.99751243781094523000, 0.99751861042183620000,
                             0.99752475247524752000, 0.99753086419753090000,
                             0.99753694581280783000, 0.99754299754299758000,
                             0.99754901960784315000, 0.99755501222493892000,
                             0.99756097560975610000, 0.99756690997566910000,
                             0.99757281553398058000, 0.99757869249394671000,
                             0.99758454106280192000, 0.99759036144578317000,
                             0.99759615384615385000, 0.99760191846522783000,
                             0.99760765550239239000, 0.99761336515513122000,
                             0.99761904761904763000, 0.99762470308788598000,
                             0.99763033175355453000, 0.99763593380614657000,
                             0.99764150943396224000, 0.99764705882352944000,
                             0.99765258215962438000, 0.99765807962529274000,
                             0.99766355140186913000, 0.99766899766899764000,
                             0.99767441860465111000, 0.99767981438515085000,
                             0.99768518518518523000, 0.99769053117782913000,
                             0.99769585253456217000, 0.99770114942528731000,
                             0.99770642201834858000, 0.99771167048054921000,
                             0.99771689497716898000, 0.99772209567198178000,
                             0.99772727272727268000, 0.99773242630385484000,
                             0.99773755656108598000, 0.99774266365688491000,
                             0.99774774774774777000, 0.99775280898876406000,
                             0.99775784753363228000, 0.99776286353467558000,
                             0.99776785714285710000, 0.99777282850779514000,
                             0.99777777777777776000, 0.99778270509977829000,
                             0.99778761061946908000, 0.99779249448123619000,
                             0.99779735682819382000, 0.99780219780219781000,
                             0.99780701754385970000, 0.99781181619256021000,
                             0.99781659388646293000, 0.99782135076252720000,
                             0.99782608695652175000, 0.99783080260303691000,
                             0.99783549783549785000, 0.99784017278617709000,
                             0.99784482758620685000, 0.99784946236559136000,
                             0.99785407725321884000, 0.99785867237687365000,
                             0.99786324786324787000, 0.99786780383795304000,
                             0.99787234042553197000, 0.99787685774946921000,
                             0.99788135593220340000, 0.99788583509513740000,
                             0.99789029535864981000, 0.99789473684210528000,
                             0.99789915966386555000, 0.99790356394129975000,
                             0.99790794979079500000, 0.99791231732776620000,
                             0.99791666666666667000, 0.99792099792099798000,
                             0.99792531120331951000, 0.99792960662525876000,
                             0.99793388429752061000, 0.99793814432989691000,
                             0.99794238683127567000, 0.99794661190965095000,
                             0.99795081967213117000, 0.99795501022494892000,
                             0.99795918367346936000, 0.99796334012219956000,
                             0.99796747967479671000, 0.99797160243407712000,
                             0.99797570850202433000, 0.99797979797979797000,
                             0.99798387096774188000, 0.99798792756539234000,
                             0.99799196787148592000, 0.99799599198396793000,
                             0.99800000000000000000, 0.99800399201596801000,
                             0.99800796812749004000, 0.99801192842942343000,
                             0.99801587301587302000, 0.99801980198019802000,
                             0.99802371541501977000, 0.99802761341222879000,
                             0.99803149606299213000, 0.99803536345776034000,
                             0.99803921568627452000, 0.99804305283757344000,
                             0.99804687500000000000, 0.99805068226120852000,
                             0.99805447470817121000, 0.99805825242718449000,
                             0.99806201550387597000, 0.99806576402321079000,
                             0.99806949806949807000, 0.99807321772639690000,
                             0.99807692307692308000, 0.99808061420345484000,
                             0.99808429118773945000, 0.99808795411089868000,
                             0.99809160305343514000, 0.99809523809523815000,
                             0.99809885931558939000, 0.99810246679316883000,
                             0.99810606060606055000, 0.99810964083175802000,
                             0.99811320754716981000, 0.99811676082862522000,
                             0.99812030075187974000, 0.99812382739212002000,
                             0.99812734082397003000, 0.99813084112149530000,
                             0.99813432835820892000, 0.99813780260707630000,
                             0.99814126394052050000, 0.99814471243042668000,
                             0.99814814814814812000, 0.99815157116451014000,
                             0.99815498154981552000, 0.99815837937384899000,
                             0.99816176470588236000, 0.99816513761467895000,
                             0.99816849816849818000, 0.99817184643510060000,
                             0.99817518248175185000, 0.99817850637522765000,
                             0.99818181818181817000, 0.99818511796733211000,
                             0.99818840579710144000, 0.99819168173598549000,
                             0.99819494584837543000, 0.99819819819819822000,
                             0.99820143884892087000, 0.99820466786355477000,
                             0.99820788530465954000, 0.99821109123434704000,
                             0.99821428571428572000, 0.99821746880570406000,
                             0.99822064056939497000, 0.99822380106571940000,
                             0.99822695035460995000, 0.99823008849557526000,
                             0.99823321554770317000, 0.99823633156966496000,
                             0.99823943661971826000, 0.99824253075571179000,
                             0.99824561403508771000, 0.99824868651488619000,
                             0.99825174825174823000, 0.99825479930191974000,
                             0.99825783972125437000, 0.99826086956521742000,
                             0.99826388888888884000, 0.99826689774696709000,
                             0.99826989619377160000, 0.99827288428324701000,
                             0.99827586206896557000, 0.99827882960413084000,
                             0.99828178694158076000, 0.99828473413379071000,
                             0.99828767123287676000, 0.99829059829059830000,
                             0.99829351535836175000, 0.99829642248722317000,
                             0.99829931972789121000, 0.99830220713073003000,
                             0.99830508474576274000, 0.99830795262267347000,
                             0.99831081081081086000, 0.99831365935919059000,
                             0.99831649831649827000, 0.99831932773109244000,
                             0.99832214765100669000, 0.99832495812395305000,
                             0.99832775919732442000, 0.99833055091819700000,
                             0.99833333333333329000, 0.99833610648918469000,
                             0.99833887043189373000, 0.99834162520729686000,
                             0.99834437086092720000, 0.99834710743801658000,
                             0.99834983498349839000, 0.99835255354200991000,
                             0.99835526315789469000, 0.99835796387520526000,
                             0.99836065573770494000, 0.99836333878887074000,
                             0.99836601307189543000, 0.99836867862969003000,
                             0.99837133550488599000, 0.99837398373983743000,
                             0.99837662337662336000, 0.99837925445705022000,
                             0.99838187702265369000, 0.99838449111470118000,
                             0.99838709677419357000, 0.99838969404186795000,
                             0.99839228295819937000, 0.99839486356340290000,
                             0.99839743589743590000, 0.99839999999999995000,
                             0.99840255591054317000, 0.99840510366826152000,
                             0.99840764331210186000, 0.99841017488076311000,
                             0.99841269841269842000, 0.99841521394611732000,
                             0.99841772151898733000, 0.99842022116903628000,
                             0.99842271293375395000, 0.99842519685039366000,
                             0.99842767295597479000, 0.99843014128728413000,
                             0.99843260188087779000, 0.99843505477308292000,
                             0.99843749999999998000, 0.99843993759750393000,
                             0.99844236760124616000, 0.99844479004665632000,
                             0.99844720496894412000, 0.99844961240310082000,
                             0.99845201238390091000, 0.99845440494590421000,
                             0.99845679012345678000, 0.99845916795069334000,
                             0.99846153846153851000, 0.99846390168970811000,
                             0.99846625766871167000, 0.99846860643185298000,
                             0.99847094801223246000, 0.99847328244274813000,
                             0.99847560975609762000, 0.99847792998477924000,
                             0.99848024316109418000, 0.99848254931714719000,
                             0.99848484848484853000, 0.99848714069591527000,
                             0.99848942598187307000, 0.99849170437405732000,
                             0.99849397590361444000, 0.99849624060150377000,
                             0.99849849849849848000, 0.99850074962518742000,
                             0.99850299401197606000, 0.99850523168908822000,
                             0.99850746268656720000, 0.99850968703427723000,
                             0.99851190476190477000, 0.99851411589895989000,
                             0.99851632047477745000, 0.99851851851851847000,
                             0.99852071005917165000, 0.99852289512555392000,
                             0.99852507374631272000, 0.99852724594992637000,
                             0.99852941176470589000, 0.99853157121879588000,
                             0.99853372434017595000, 0.99853587115666176000,
                             0.99853801169590639000, 0.99854014598540142000,
                             0.99854227405247808000, 0.99854439592430855000,
                             0.99854651162790697000, 0.99854862119013066000,
                             0.99855072463768113000, 0.99855282199710560000,
                             0.99855491329479773000, 0.99855699855699853000,
                             0.99855907780979825000, 0.99856115107913668000,
                             0.99856321839080464000, 0.99856527977044474000,
                             0.99856733524355301000, 0.99856938483547930000,
                             0.99857142857142855000, 0.99857346647646217000,
                             0.99857549857549854000, 0.99857752489331442000,
                             0.99857954545454541000, 0.99858156028368794000,
                             0.99858356940509918000, 0.99858557284299854000,
                             0.99858757062146897000, 0.99858956276445698000,
                             0.99859154929577465000, 0.99859353023909991000,
                             0.99859550561797750000, 0.99859747545582045000,
                             0.99859943977591037000, 0.99860139860139863000,
                             0.99860335195530725000, 0.99860529986053004000,
                             0.99860724233983289000, 0.99860917941585536000,
                             0.99861111111111112000, 0.99861303744798890000,
                             0.99861495844875348000, 0.99861687413554634000,
                             0.99861878453038677000, 0.99862068965517237000,
                             0.99862258953168048000, 0.99862448418156813000,
                             0.99862637362637363000, 0.99862825788751719000,
                             0.99863013698630132000, 0.99863201094391241000,
                             0.99863387978142082000, 0.99863574351978168000,
                             0.99863760217983655000, 0.99863945578231295000,
                             0.99864130434782605000, 0.99864314789687925000,
                             0.99864498644986455000, 0.99864682002706362000,
                             0.99864864864864866000, 0.99865047233468285000,
                             0.99865229110512133000, 0.99865410497981155000,
                             0.99865591397849462000, 0.99865771812080539000,
                             0.99865951742627346000, 0.99866131191432395000,
                             0.99866310160427807000, 0.99866488651535379000,
                             0.99866666666666670000, 0.99866844207723038000,
                             0.99867021276595747000, 0.99867197875166003000,
                             0.99867374005305043000, 0.99867549668874167000,
                             0.99867724867724872000, 0.99867899603698806000,
                             0.99868073878627972000, 0.99868247694334655000,
                             0.99868421052631584000, 0.99868593955321949000,
                             0.99868766404199472000, 0.99868938401048490000,
                             0.99869109947643975000, 0.99869281045751634000,
                             0.99869451697127942000, 0.99869621903520212000,
                             0.99869791666666663000, 0.99869960988296491000,
                             0.99870129870129876000, 0.99870298313878081000,
                             0.99870466321243523000, 0.99870633893919791000,
                             0.99870801033591727000, 0.99870967741935479000,
                             0.99871134020618557000, 0.99871299871299868000,
                             0.99871465295629824000, 0.99871630295250324000,
                             0.99871794871794872000, 0.99871959026888601000,
                             0.99872122762148341000, 0.99872286079182626000,
                             0.99872448979591832000, 0.99872611464968153000,
                             0.99872773536895676000, 0.99872935196950441000,
                             0.99873096446700504000, 0.99873257287705952000,
                             0.99873417721518987000, 0.99873577749683939000,
                             0.99873737373737370000, 0.99873896595208067000,
                             0.99874055415617125000, 0.99874213836477987000,
                             0.99874371859296485000, 0.99874529485570895000,
                             0.99874686716791983000, 0.99874843554443049000,
                             0.99875000000000003000, 0.99875156054931336000,
                             0.99875311720698257000, 0.99875466998754669000,
                             0.99875621890547261000, 0.99875776397515525000,
                             0.99875930521091816000, 0.99876084262701359000,
                             0.99876237623762376000, 0.99876390605686027000,
                             0.99876543209876545000, 0.99876695437731200000,
                             0.99876847290640391000, 0.99876998769987702000,
                             0.99877149877149873000, 0.99877300613496933000,
                             0.99877450980392157000, 0.99877600979192172000,
                             0.99877750611246940000, 0.99877899877899878000,
                             0.99878048780487805000, 0.99878197320341044000,
                             0.99878345498783450000, 0.99878493317132444000,
                             0.99878640776699024000, 0.99878787878787878000,
                             0.99878934624697335000, 0.99879081015719473000,
                             0.99879227053140096000, 0.99879372738238847000,
                             0.99879518072289153000, 0.99879663056558365000,
                             0.99879807692307687000, 0.99879951980792314000,
                             0.99880095923261392000, 0.99880239520958081000,
                             0.99880382775119614000, 0.99880525686977295000,
                             0.99880668257756566000, 0.99880810488676997000,
                             0.99880952380952381000, 0.99881093935790721000,
                             0.99881235154394299000, 0.99881376037959668000,
                             0.99881516587677721000, 0.99881656804733732000,
                             0.99881796690307334000, 0.99881936245572611000,
                             0.99882075471698117000, 0.99882214369846878000,
                             0.99882352941176467000, 0.99882491186839018000,
                             0.99882629107981225000, 0.99882766705744430000,
                             0.99882903981264637000, 0.99883040935672518000,
                             0.99883177570093462000, 0.99883313885647607000,
                             0.99883449883449882000, 0.99883585564610011000,
                             0.99883720930232556000, 0.99883855981416958000,
                             0.99883990719257543000, 0.99884125144843572000,
                             0.99884259259259256000, 0.99884393063583810000,
                             0.99884526558891451000, 0.99884659746251436000,
                             0.99884792626728114000, 0.99884925201380903000,
                             0.99885057471264371000, 0.99885189437428246000,
                             0.99885321100917435000, 0.99885452462772051000,
                             0.99885583524027455000, 0.99885714285714289000,
                             0.99885844748858443000, 0.99885974914481190000,
                             0.99886104783599083000, 0.99886234357224113000,
                             0.99886363636363640000, 0.99886492622020429000,
                             0.99886621315192747000, 0.99886749716874290000,
                             0.99886877828054299000, 0.99887005649717520000,
                             0.99887133182844245000, 0.99887260428410374000,
                             0.99887387387387383000, 0.99887514060742411000,
                             0.99887640449438198000, 0.99887766554433222000,
                             0.99887892376681620000, 0.99888017917133254000,
                             0.99888143176733779000, 0.99888268156424576000,
                             0.99888392857142860000, 0.99888517279821631000,
                             0.99888641425389757000, 0.99888765294771964000,
                             0.99888888888888894000, 0.99889012208657046000,
                             0.99889135254988914000, 0.99889258028792915000,
                             0.99889380530973448000, 0.99889502762430937000,
                             0.99889624724061810000, 0.99889746416758540000,
                             0.99889867841409696000, 0.99889988998899892000,
                             0.99890109890109891000, 0.99890230515916578000,
                             0.99890350877192979000, 0.99890470974808321000,
                             0.99890590809628010000, 0.99890710382513659000,
                             0.99890829694323147000, 0.99890948745910579000,
                             0.99891067538126366000, 0.99891186071817195000,
                             0.99891304347826082000, 0.99891422366992400000,
                             0.99891540130151846000, 0.99891657638136511000,
                             0.99891774891774887000, 0.99891891891891893000,
                             0.99892008639308860000, 0.99892125134843579000,
                             0.99892241379310343000, 0.99892357373519913000,
                             0.99892473118279568000, 0.99892588614393130000,
                             0.99892703862660948000, 0.99892818863879962000,
                             0.99892933618843682000, 0.99893048128342243000,
                             0.99893162393162394000, 0.99893276414087517000,
                             0.99893390191897657000, 0.99893503727369537000,
                             0.99893617021276593000, 0.99893730074388953000,
                             0.99893842887473461000, 0.99893955461293749000,
                             0.99894067796610164000, 0.99894179894179891000,
                             0.99894291754756870000, 0.99894403379091867000,
                             0.99894514767932485000, 0.99894625922023184000,
                             0.99894736842105258000, 0.99894847528916930000,
                             0.99894957983193278000, 0.99895068205666315000,
                             0.99895178197064993000, 0.99895287958115186000,
                             0.99895397489539750000, 0.99895506792058519000,
                             0.99895615866388310000, 0.99895724713242962000,
                             0.99895833333333328000, 0.99895941727367321000,
                             0.99896049896049899000, 0.99896157840083077000,
                             0.99896265560165975000, 0.99896373056994814000,
                             0.99896480331262938000, 0.99896587383660806000,
                             0.99896694214876036000, 0.99896800825593390000,
                             0.99896907216494846000, 0.99897013388259526000,
                             0.99897119341563789000, 0.99897225077081198000,
                             0.99897330595482547000, 0.99897435897435893000,
                             0.99897540983606559000, 0.99897645854657113000,
                             0.99897750511247441000, 0.99897854954034726000,
                             0.99897959183673468000, 0.99898063200815490000,
                             0.99898167006109984000, 0.99898270600203454000,
                             0.99898373983739841000, 0.99898477157360410000,
                             0.99898580121703850000, 0.99898682877406286000,
                             0.99898785425101211000, 0.99898887765419619000,
                             0.99898989898989898000, 0.99899091826437947000,
                             0.99899193548387100000, 0.99899295065458205000,
                             0.99899396378269623000, 0.99899497487437183000,
                             0.99899598393574296000, 0.99899699097291872000,
                             0.99899799599198402000, 0.99899899899899902000,
                             0.99900000000000000000, 0.99900099900099903000,
                             0.99900199600798401000, 0.99900299102691925000,
                             0.99900398406374502000, 0.99900497512437814000,
                             0.99900596421471177000, 0.99900695134061568000,
                             0.99900793650793651000, 0.99900891972249750000,
                             0.99900990099009901000, 0.99901088031651830000,
                             0.99901185770750989000, 0.99901283316880551000,
                             0.99901380670611439000, 0.99901477832512320000,
                             0.99901574803149606000, 0.99901671583087515000,
                             0.99901768172888017000, 0.99901864573110888000,
                             0.99901960784313726000, 0.99902056807051909000,
                             0.99902152641878672000, 0.99902248289345064000};


//! gauss_legendre_tbl
/*!
 Code is taken from the GNU Scientific Library under GPLv3 license.
*/
static void
gauss_legendre_tbl(Index n, Numeric *x, Numeric *w, Numeric eps)
{
    Numeric x0, x1, dx; /* Abscissas */
    Numeric w0, w1, dw; /* Weights */
    Numeric P0, P_1, P_2; /* Legendre polynomial values */
    Numeric dpdx;   /* Legendre polynomial derivative */
    Index i, j, k, m;   /* Iterators */
    Numeric t0, t1, t2, t3;

    m = (n + 1) >> 1;

    t0 = (1.0 - (1.0 - 1.0 / (Numeric)n) / (8.0 * (Numeric)n * (Numeric)n));
    t1 = 1.0 / (4.0 * (Numeric)n + 2.0);

    for (i = 1; i <= m; i++)
    {
        /* Find i-th root of Legendre polynomial */

        /* Initial guess */
        const Numeric pi_to_many_places
                = 3.1415926535897932384626433832795028841971693993751;
        x0 = cos(pi_to_many_places
                 * (Numeric)((i << 2) - 1) * t1) * t0;

        /* Newton iterations, at least one */
        j = 0;
        dx = dw = std::numeric_limits<Numeric>::max();

        do
        {
            /* Compute Legendre polynomial value at x0 */
            P_1 = 1.0;
            P0 = x0;

            /* Optimized version using lookup tables */

            if (n < 1024)
            {
                /* Use fast algorithm for small n*/
                for (k = 2; k <= n; k++)
                {
                    P_2 = P_1;
                    P_1 = P0;
                    t2 = x0 * P_1;

                    P0 = t2 + ltbl[k] * (t2 - P_2);

                }

            }
            else
            {

                /* Use general algorithm for other n */
                for (k = 2; k < 1024; k++)
                {
                    P_2 = P_1;
                    P_1 = P0;
                    t2 = x0 * P_1;

                    P0 = t2 + ltbl[k] * (t2 - P_2);
                }

                for (k = 1024; k <= n; k++)
                {
                    P_2 = P_1;
                    P_1 = P0;
                    t2 = x0 * P_1;
                    t3 = (Numeric)(k - 1) / (Numeric)k;

                    P0 = t2 + t3 * (t2 - P_2);
                }
            }

            /* Compute Legendre polynomial derivative at x0 */
            dpdx = ((x0 * P0 - P_1) * (Numeric)n) / (x0 * x0 - 1.0);

            /* Newton step */
            x1 = x0 - P0 / dpdx;

            /* Weight computing */
            w1 = 2.0 / ((1.0 - x1 * x1) * dpdx * dpdx);

            /* Compute weight w0 on first iteration, needed for dw */
            if (j == 0) { w0 = 2.0 / ((1.0 - x0 * x0) * dpdx * dpdx); }

            dx = x0 - x1;

            dw = w0 - w1;

            x0 = x1;

            w0 = w1;

            j++;
        } while ((fabs(dx) > eps || fabs(dw) > eps) && j < 100);

        x[(m - 1) - (i - 1)] = x1;

        w[(m - 1) - (i - 1)] = w1;
    }
}


//! gsl_integration_glfixed_table_alloc
/*!
 This function determines the Gauss-Legendre abscissae and weights necessary
 for an n-point fixed order integration scheme. If possible, high precision
 precomputed coefficients are used. If precomputed weights are not available,
 lower precision coefficients are computed on the fly.

 Code is taken from the GNU Scientific Library under GPLv3 license.
 Adapted to return ARTS datatypes.

 \param[out]  x   Vector with Gauss-Legendre abscissae.
 \param[out]  w   Vector with Gauss-Legendre weights.
 \param[in]   n   Integration scheme order.

 \return true if precomputed values have been used.

 \author Oliver Lemke
 \date   2018-05-29
*/
bool
gsl_integration_glfixed_table_alloc(Vector& x, Vector& w, Index n)
{
    /* Kludgy error check: indices in the original algorithm use int but GSL
       conventions are to take a size_t parameter for n */
    if (n > INT_MAX)
    {
        throw std::runtime_error("gsl_integration_glfixed_table_alloc: "
                                 "Requested n is too large");
    }
    if (n < 1)
    {
        throw std::runtime_error("gsl_integration_glfixed_table_alloc: "
                                 "n must not be smaller than 1");
    }

    const Index m = (n + 1) >> 1;
    bool precomputed = false;

    x.resize(m);
    w.resize(m);

    /* Use a predefined table of size n if possible */
    for (auto&& g : glaw)
    {
        if (n == (Index)g.n)
        {
            memcpy(x.get_c_array(), g.x, sizeof(Numeric) * m);
            memcpy(w.get_c_array(), g.w, sizeof(Numeric) * m);
            precomputed = true;
            break;
        }
    }

    /* No predefined table is available.  Generate one on the fly. */
    if (!precomputed)
    {
        gauss_legendre_tbl(n, x.get_c_array(), w.get_c_array(),
                           1e-10 /* precision magic number */);
    }

    return precomputed;
}
