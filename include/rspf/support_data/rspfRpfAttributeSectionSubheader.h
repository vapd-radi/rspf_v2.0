//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
// Description: Rpf support class
// 
//********************************************************************
// $Id: rspfRpfAttributeSectionSubheader.h 9967 2006-11-29 02:01:23Z gpotts $
#ifndef rspfRpfAttributeSectionSubheader_HEADER
#define rspfRpfAttributeSectionSubheader_HEADER
#include <iostream>
using namespace std;

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfErrorContext.h>

class rspfRpfAttributeSectionSubheader
{
public:
   friend ostream& operator <<(ostream& out,
                               const rspfRpfAttributeSectionSubheader& data);
   rspfRpfAttributeSectionSubheader();

   virtual ~rspfRpfAttributeSectionSubheader(){}
   rspfErrorCode parseStream(istream& in, rspfByteOrder byteOrder);
   virtual void print(ostream& out)const;
   rspf_uint16 getNumberOfAttributeOffsetRecords()const
      {
         return theNumberOfAttributeOffsetRecords;
      }
   rspf_uint16 getAttributeOffsetTableOffset()const
      {
         return theAttributeOffsetTableOffset;
      }

   rspf_uint64 getSubheaderStart()const;
   rspf_uint64 getSubheaderEnd()const;
   
private:
   void clearFields();
   
   rspf_uint16 theNumberOfAttributeOffsetRecords;
   rspf_uint16 theNumberOfExplicitArealCoverageRecords;
   rspf_uint32 theAttributeOffsetTableOffset;
   rspf_uint16 theAttribteOffsetRecordLength;


   mutable rspf_uint64 theAttributeSectionSubheaderStart;
   mutable rspf_uint64 theAttributeSectionSubheaderEnd;
};

#endif
