//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file
// 
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfImageLut.h 9094 2006-06-13 19:12:40Z dburken $
#ifndef rspfNitfImageLut_HEADER
#define rspfNitfImageLut_HEADER

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfObject.h>

//using namespace std;

class rspfNitfImageLut : public rspfObject
{
public:
   friend std::ostream& operator <<(std::ostream& out,
                                    const rspfNitfImageLut &data);
   rspfNitfImageLut();
   virtual ~rspfNitfImageLut();
   virtual void parseStream(std::istream &in)=0;
   virtual std::ostream& print(std::ostream& out)const=0;
   virtual rspf_uint32 getNumberOfEntries()const=0;
   virtual void setNumberOfEntries(rspf_uint32 numberOfEntries)=0;
   virtual rspf_uint8 getValue(rspf_uint32 idx)const=0;

TYPE_DATA;
};
#endif
