dnl Autoconf support for C++
dnl Copyright (C) 1988 Eleftherios Gkioulekas <lf@amath.washington.edu>
dnl  
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl 
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl 
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software 
dnl Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
dnl 
dnl As a special exception to the GNU General Public License, if you 
dnl distribute this file as part of a program that contains a configuration 
dnl script generated by Autoconf, you may include it under the same 
dnl distribution terms that you use for the rest of that program.

# -----------------------------------------------------------------
# This macro should be called to configure your C++ compiler.
# When called, the macro does the following things:
# 1. It finds an appropriate C++ compiler
#    If you passed the flag --with-cxx=foo, then it uses that
#    particular compiler
# 2. Checks whether the compiler accepts the -g 
# ------------------------------------------------------------------

AC_DEFUN(LF_CONFIGURE_CXX,[
 AC_PROG_CXX 
 AC_PROG_CXXCPP
 LF_CPP_PORTABILITY
])

# -----------------------------------------------------------------------
# This macro tests the C++ compiler for various portability problem.
# 1. Defines CXX_HAS_NO_BOOL if the compiler does not support the bool
#    data type
# 2. Defines CXX_HAS_BUGGY_FOR_LOOPS if the compiler has buggy
#    scoping for the for-loop
# 3. Defines USE_ASSERT if the user wants to use assertions
# Seperately we provide some config.h.bot code to be added to acconfig.h
# that implements work-arounds for these problems.
# -----------------------------------------------------------------------

dnl ACCONFIG TEMPLATE
dnl #undef CXX_HAS_BUGGY_FOR_LOOPS
dnl #undef CXX_HAS_NO_BOOL
dnl #undef NDEBUG
dnl END ACCONFIG

AC_DEFUN(LF_CPP_PORTABILITY,[

  dnl
  dnl Check for common C++ portability problems
  dnl

  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS

  dnl Check whether we have bool
  AC_MSG_CHECKING(whether C++ has bool)
  AC_TRY_RUN([main() { bool b1=true; bool b2=false; }],
             [ AC_MSG_RESULT(yes) ],
             [ AC_MSG_RESULT(no)
               AC_DEFINE(CXX_HAS_NO_BOOL) ],
             [ AC_MSG_WARN(Don't cross-compile)]
            )

  dnl Test whether C++ has buggy for-loops
  AC_MSG_CHECKING(whether C++ has buggy scoping in for-loops)
  AC_TRY_COMPILE([#include <iostream.h>], [
   for (int i=0;i<10;i++) { }
   for (int i=0;i<10;i++) { }
], [ AC_MSG_RESULT(no) ],
   [ AC_MSG_RESULT(yes)
     AC_DEFINE(CXX_HAS_BUGGY_FOR_LOOPS) ])

  dnl Test whether the user wants to enable assertions
  AC_MSG_CHECKING(whether user wants assertions)
  AC_ARG_ENABLE(assert,
                [  --disable-assert        don't use cpp.h assert],
                [ AC_DEFINE(NDEBUG)
                  AC_MSG_RESULT(no)  ],
                [ AC_MSG_RESULT(yes) ],
               )

  dnl Done with the portability checks
  AC_LANG_RESTORE
])

dnl ACCONFIG BOTTOM
dnl 
dnl // Include cassert in order to allow assertions. 
dnl // SAB 29.01.2000: Changed assert.h to cassert
dnl #include <cassert>
dnl 
dnl END ACCONFIG
