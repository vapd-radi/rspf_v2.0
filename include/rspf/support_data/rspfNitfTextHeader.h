//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfTextHeader.h 9094 2006-06-13 19:12:40Z dburken $
#ifndef rspfNitfTextHeader_HEADER
#define rspfNitfTextHeader_HEADER

#include <rspf/base/rspfObject.h>

class RSPFDLLEXPORT rspfNitfTextHeader : public rspfObject
{
public:
   rspfNitfTextHeader(){}
   virtual ~rspfNitfTextHeader(){}
   
   virtual void parseStream(std::istream &in)=0;
   
TYPE_DATA
};

#endif
