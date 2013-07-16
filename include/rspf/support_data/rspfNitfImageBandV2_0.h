//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfImageBandV2_0.h 15416 2009-09-11 20:58:51Z dburken $
#ifndef rspfNitfImageBandV2_0_HEADER
#define rspfNitfImageBandV2_0_HEADER

#include <string>
#include <rspf/base/rspfConstants.h>
#include <rspf/support_data/rspfNitfImageBand.h>
#include <rspf/support_data/rspfNitfImageLutV2_0.h>

class RSPF_DLL rspfNitfImageBandV2_0 : public rspfNitfImageBand
{
public:
   
   /** default constructor */
   rspfNitfImageBandV2_0();

   /** virtual destructory */
   virtual ~rspfNitfImageBandV2_0();
   
   virtual void parseStream(std::istream& in);
   virtual void writeStream(std::ostream& out);

   /**
    * @brief print method that outputs a key/value type format adding prefix
    * to keys.
    * @param out Stream to output to.
    * @param prefix Like "image0."
    * @param band zero based band.
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix=std::string(),
                               rspf_uint32 band=0) const;   
   
   virtual rspf_uint32 getNumberOfLuts()const;
   virtual const rspfRefPtr<rspfNitfImageLut> getLut(rspf_uint32 idx)const;
   virtual rspfRefPtr<rspfNitfImageLut> getLut(rspf_uint32 idx);

   /** @return The band representation as an rspfString. */
   virtual rspfString getBandRepresentation()const;

   /**
    * Sets the band representation.
    *
    * @param rep The band representation.
    */
   virtual void setBandRepresentation(const rspfString& rep);

   /** @return The band significance as an rspfString. */
   virtual rspfString getBandSignificance()const;

   /**
    * Sets the band significance.
    *
    * @param rep The band significance.
    */
   virtual void setBandSignificance(const rspfString& rep);

protected:
   void clearFields();
   void printLookupTables(std::ostream& out)const;
   /*!
    * FIELD: IREPBAND,,
    * Is a required 2 byte field.  When theNumberOfBands is
    * 1 this field will contain all spaces.
    */
   char theBandRepresentation[3];

   /*!
    * FIELD:  ISUBCATnn
    * Is a required 6 byte field.
    */
   char theBandSignificance[7];

   /*!
    * FIELD:  IFCnn
    * Is an required 1 byte field.  Will be N
    */
   char theBandImageFilterCondition[2];

   /*!
    * FIELD:  IMFLTnn
    * is a required 3 byte field.  This is a reserved field
    */
   char theBandStandardImageFilterCode[4];

   /*!
    * FIELD:  NLUTSnn
    * This is a required 1 byte field.  Has value 0-4
    */
   char theBandNumberOfLuts[2];

   /*!
    * NELUTnn
    * This is a conditional field.
    */
   char theBandNumberOfLutEntries[6];

   std::vector<rspfRefPtr<rspfNitfImageLut> > theLookupTables;
};

#endif
