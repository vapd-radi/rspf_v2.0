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
// $Id: rspfNitfLabelHeader.h 9094 2006-06-13 19:12:40Z dburken $
#ifndef rspfNitfLabelHeader_HEADER
#define rspfNitfLabelHeader_HEADER

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfObject.h>

class rspfDrect;

class RSPFDLLEXPORT rspfNitfLabelHeader : public rspfObject
{
public:
   rspfNitfLabelHeader(){}
   virtual ~rspfNitfLabelHeader(){}
   
   virtual void        parseStream(std::istream &in)=0;
   virtual rspf_int32 getDisplayLevel()const=0;
   virtual rspfDrect  getImageRect()const=0;
   
TYPE_DATA
};

#endif
