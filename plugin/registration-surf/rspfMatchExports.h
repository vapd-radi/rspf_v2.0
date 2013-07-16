//----------------------------------------------------------------------------
// Copyright (c) 2005, Garrett Potts, all rights reserved.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
// Description: Constants file for rspf plugins.
//
// $Id: rspfMatchExports.h 8366 2005-11-17 13:34:03Z fclaudel $
//----------------------------------------------------------------------------
#ifndef rspfMatchExports_HEADER
#define rspfMatchExports_HEADER

#if defined(__MINGW32__) || defined(__CYGWIN__) || defined(_MSC_VER) || defined(__VISUALC__) || defined(__BORLANDC__) || defined(__WATCOMC__)
#  define RSPF_REGISTRATION_EXPORT __declspec(dllexport)
#  define RSPF_REGISTRATION_IMPORT __declspec(dllexport)

#  ifdef RSPF_REGISTRATION_MAKINGDLL
#     define RSPF_REGISTRATION_DLL RSPF_REGISTRATION_EXPORT
#  else
#     define  RSPF_REGISTRATION_DLL RSPF_REGISTRATION_IMPORT
#  endif
#else
# define RSPF_REGISTRATION_DLL
#endif

#endif /* #ifndef rspfMatchExports_HEADER */
