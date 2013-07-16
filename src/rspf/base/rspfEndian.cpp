//******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// 
// Description: This file contains the implementation of the
//              endian byte swap routines.
//
//***********************************
// $Id: rspfEndian.cpp 22197 2013-03-12 02:00:55Z dburken $

#include <rspf/base/rspfEndian.h>

rspfEndian::rspfEndian()
{
   rspf_uint16  test;
   rspf_uint8  *testPtr=0;
   test = 0x0001;
   
   testPtr       = reinterpret_cast<unsigned char*>(&test);
   theSystemEndianType = testPtr[0] ? RSPF_LITTLE_ENDIAN : RSPF_BIG_ENDIAN;
}
