/* Copyright (C) 2000, 2001 Stefan Buehler <sbuehler@uni-bremen.de>
                            Patrick Eriksson <Patrick.Eriksson@rss.chalmers.se>

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

/**
  \file  arts.h

  The global header file for ARTS. This file is included directly or
  indirectly by each and every ARTS source file. It must therefor not
  contain stuff that should not always be present.

  Note that you do not have to include this file explicitly in many
  cases, since it is included directly or indirectly by most ARTS
  header files.

  \author Stefan Buehler
  \date 16.05.1999 */

/** \mainpage
    
    This automatic documentation is still \e very experimental. We
    have just switched from doc++ to doxygen. You can use the HTML
    version to browse the source text. Just point and click, and
    eventually you will see the real implementation of functions and
    classes. 

    If you are looking for a more comprehensive text, check out the
    Arts User Guide that is also distributed along with the
    program. Section `Documentation' in Chapter `The art of developing
    ARTS' there also tells you how you should add documentation
    headers to your code if you are an ARTS developer.
 */

#ifndef arts_h
#define arts_h

#include <cstddef>

//----------< First of all, include the configuration header >----------
// This header is generated by the configure script.
#if HAVE_CONFIG_H
#include <config.h>
#else
#error "Please run ./configure in the top arts directory before compiling."
#endif		

// C Assert macro:
#include <cassert>


// We need those to support ansi-compliant compilers (gcc-3x)
using namespace std;

//--------------------< Set floating point type >--------------------
/** The type to use for all floating point numbers. You should never
    use float or double explicitly, unless you have a very good
    reason. Always use this type instead.  */
typedef NUMERIC Numeric;

//--------------------< Set integer type >--------------------
/** The type to use for all integer numbers and indices. You should never
    use int, long, or size_t explicitly, unless you have a very good
    reason. Always use this type instead.  */
typedef INDEX Index;

//--------------------< Set string type >--------------------
/** The type to use for all strings. This is just to have consistent
    notation for all the atomic ARTS types. */ 
//typedef string String;

// Declare the existance of class Array:
template<class base>
class Array;

/** An array of Index. */
typedef Array<Index> ArrayOfIndex;

/** An array of Numeric. */
typedef Array<Numeric> ArrayOfNumeric;


//-----------< define a quick output for std::vector<> >----------
/* A quick hack output for std::vector<>. This is only for
    debugging purposes.
    \return As for all output operator functions, the output stream is 
            returned.
    \param  os Output stream
    \param  v  Vector to print                      
    \author SAB  */  
// template<class T>
// ostream& operator<<(ostream& os, const std::vector<T>& v)
// {
//   for (std::vector<T>::const_iterator i=v.begin(); i<v.end()-1; ++i)
//     os << *i << ", ";
//   os << *(v.end()-1);
//   return os;
// }



//---------------< Global variable declarations >---------------
// (Definitions of these are in FIXME: where?)


//---------------< Global function declarations: >---------------
// Documentations are with function definitions.
void define_wsv_group_names();	
void define_species_data();
void define_lineshape_data();
void define_lineshape_norm_data();

//
// Physical constants are now in constants.cc
//



#endif // arts_h



