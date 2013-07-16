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
// $Id: rspfNitfDataExtensionSegment.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfNitfDataExtensionSegment_HEADER
#define rspfNitfDataExtensionSegment_HEADER

#include <iosfwd>
#include <rspf/base/rspfObject.h>
class rspfNitfTagInformation;

class RSPFDLLEXPORT rspfNitfDataExtensionSegment : public rspfObject
{
public:
   friend std::ostream& operator <<(std::ostream &out,
                                    const rspfNitfDataExtensionSegment &data);
   rspfNitfDataExtensionSegment(){}
   virtual void parseStream(std::istream &in)=0;
   virtual const rspfNitfTagInformation&  getTagInformation()const=0;

protected:
   virtual ~rspfNitfDataExtensionSegment(){}

TYPE_DATA
};

#endif
