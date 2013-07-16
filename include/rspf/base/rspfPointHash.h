//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
// Description: Point Hash is a base for hashing functions
//              
//*******************************************************************
//  $Id: rspfPointHash.h 9094 2006-06-13 19:12:40Z dburken $

#ifndef rspfPointHash_HEADER
#define rspfPointHash_HEADER
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfFpt.h>

class RSPFDLLEXPORT rspfPointHash
{
public:
   virtual ~rspfPointHash(){}
   virtual long operator ()(const rspfDpt &aPoint)=0;
   virtual long operator ()(const rspfFpt &aPoint)=0;
};

#endif
