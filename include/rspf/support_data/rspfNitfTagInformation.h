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
// $Id: rspfNitfTagInformation.h 16997 2010-04-12 18:53:48Z dburken $
#ifndef rspfNitfTagInformation_HEADER
#define rspfNitfTagInformation_HEADER

#include <rspf/base/rspfObject.h>
#include <rspf/support_data/rspfNitfRegisteredTag.h>

class rspfString;

class RSPFDLLEXPORT rspfNitfTagInformation : public rspfObject
{
public:
   rspfNitfTagInformation(rspfRefPtr<rspfNitfRegisteredTag> tagData = 0);
   virtual ~rspfNitfTagInformation();
   
   virtual void parseStream(std::istream& in);
   virtual void writeStream(std::ostream& out);

   void setTagName(const rspfString& tagName);
   void setTagLength(rspf_uint32 tagLength);

   /**
    * Length of the 5 byte tag length the 6 byte tag name and
    * the data length.
    * So we have Data length + 11 bytes.
    */
   rspf_uint32 getTotalTagLength()const;

   /**
    * Should return the value of theTagLength which is the length of
    * the data in bytes.
    */
   rspf_uint32 getTagLength()const;
   rspf_uint64 getTagOffset()const;
   rspf_uint64 getTagDataOffset()const;
   
   rspfString   getTagName()const;
   virtual std::ostream& print(std::ostream& out)const;
   void clearFields();
   
   rspfRefPtr<rspfNitfRegisteredTag> getTagData();
   const rspfRefPtr<rspfNitfRegisteredTag> getTagData()const;
   void setTagData(rspfRefPtr<rspfNitfRegisteredTag> tagData);
   
private:

   /**
    * This is a 6 byte field
    */
   char          theTagName[7];

   /**
    * This is a 5 byte field
    */
   char           theTagLength[6];

   /**
    * This will hold the offset to the start of the above information
    * This is just a work variable.
    */
   rspf_uint64 theTagOffset;

   /**
    * This will hold the start to the data.  This is just the
    * position in the file just past the tag length field.
    * This is just a work variable
    */
   rspf_uint64 theTagDataOffset;

   /**
    * Used to hold the tag data.
    */
   rspfRefPtr<rspfNitfRegisteredTag> theTagData;
};

#endif
