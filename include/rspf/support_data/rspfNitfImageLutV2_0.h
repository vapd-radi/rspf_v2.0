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
// $Id: rspfNitfImageLutV2_0.h 9094 2006-06-13 19:12:40Z dburken $
#ifndef rspfNitfImageLutV2_0_HEADER
#define rspfNitfImageLutV2_0_HEADER
#include <rspf/support_data/rspfNitfImageLut.h>

#include <vector>

class rspfNitfImageLutV2_0 : public rspfNitfImageLut
{
public:
   rspfNitfImageLutV2_0();
   virtual ~rspfNitfImageLutV2_0();
   virtual void parseStream(std::istream &in);
   virtual std::ostream& print(std::ostream& out)const;
   virtual rspf_uint32 getNumberOfEntries()const;
   virtual void setNumberOfEntries(rspf_uint32 numberOfEntries);
   virtual rspf_uint8 getValue(rspf_uint32 idx)const;
   
protected:
   std::vector<rspf_uint8> theData;
   rspf_uint32 theNumberOfEntries;
};
#endif
