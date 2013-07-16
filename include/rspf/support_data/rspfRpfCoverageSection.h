//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
// 
// Author: Garrett Potts
// 
// Description: Rpf support class
// 
//********************************************************************
// $Id: rspfRpfCoverageSection.h 16997 2010-04-12 18:53:48Z dburken $
#ifndef rspfRpfCoverageSection_HEADER
#define rspfRpfCoverageSection_HEADER

#include <iosfwd>

#include <rspf/base/rspfConstants.h>
#include <rspf/support_data/rspfRpfConstants.h>
#include <rspf/base/rspfErrorCodes.h>

class rspfRpfCoverageSection
{
public:
   friend std::ostream& operator <<(std::ostream &out,
                                    const rspfRpfCoverageSection &data);
   
   rspfRpfCoverageSection();
   rspfRpfCoverageSection(const rspfRpfCoverageSection& obj);
   const rspfRpfCoverageSection& operator=(const rspfRpfCoverageSection& rhs);
   
   ~rspfRpfCoverageSection(){}

   rspfErrorCode parseStream(std::istream &in, rspfByteOrder byteOrder);

   /**
    * @brief Write method.
    *
    * @param out Stream to write to.
    */
   void writeStream(std::ostream& out);


   /**
    * @brief print method that outputs a key/value type format adding prefix
    * to keys.
    * @param out String to output to.
    * @param prefix This will be prepended to key.
    * e.g. Where prefix = "nitf." and key is "file_name" key becomes:
    * "nitf.file_name:"
    * @return output stream.
    */
   std::ostream& print(std::ostream& out,
                       const std::string& prefix=std::string()) const;
   
   void clearFields();

   bool isGeographicRectNull()const
   {
      return ((theUpperLeftLat  == RSPF_RPF_ULONG_NULL) &&
              (theUpperLeftLon  == RSPF_RPF_ULONG_NULL) &&
              (theLowerLeftLat  == RSPF_RPF_ULONG_NULL) &&
              (theLowerLeftLon  == RSPF_RPF_ULONG_NULL) &&
              (theLowerRightLat == RSPF_RPF_ULONG_NULL) &&
              (theLowerRightLon == RSPF_RPF_ULONG_NULL) &&
              (theUpperRightLat == RSPF_RPF_ULONG_NULL) &&
              (theUpperRightLon == RSPF_RPF_ULONG_NULL));
   }
   bool isIntervalNull()const
   {
      return ((theVerticalInterval   == RSPF_RPF_ULONG_NULL)&&
              (theHorizontalInterval == RSPF_RPF_ULONG_NULL));
   }

   rspf_float64 getUlLat()const{return theUpperLeftLat;}
   rspf_float64 getUlLon()const{return theUpperLeftLon;}
   rspf_float64 getLlLat()const{return theLowerLeftLat;}
   rspf_float64 getLlLon()const{return theLowerLeftLon;}
   rspf_float64 getLrLat()const{return theLowerRightLat;}
   rspf_float64 getLrLon()const{return theLowerRightLon;}
   rspf_float64 getUrLat()const{return theUpperRightLat;}
   rspf_float64 getUrLon()const{return theUpperRightLon;}

   rspf_float64 getVerticalResolution()const{return theVerticalResolution;}
   rspf_float64 getHorizontalResolution()const{return theHorizontalResolution;}

   /*!
    * This indicates the lat increment per pixel in degrees.
    */
   rspf_float64 getVerticalInterval()const{return theVerticalInterval;}

   /*!
    * This indicates the lon increment per pixel in degrees.
    */
   rspf_float64 getHorizontalInterval()const{return theHorizontalInterval;}

   /** @brief Sets theUpperLeftLat to value. */
   void setUlLat(rspf_float64 value);

   /** @brief Sets theUpperLeftLon to value. */
   void setUlLon(rspf_float64 value);

   /** @brief Sets theLowerLeftLat to value. */
   void setLlLat(rspf_float64 value);

   /** @brief Sets theLowerLeftLon to value. */
   void setLlLon(rspf_float64 value);
   
   /** @brief Sets theLowerRightLat to value. */
   void setLrLat(rspf_float64 value);
   
   /** @brief Sets theLowerRightLon to value. */
   void setLrLon(rspf_float64 value);

   /** @brief Sets theUpperRightLat to value. */
   void setUrLat(rspf_float64 value);

   /** @brief Sets theUpperRightLon to value. */
   void setUrLon(rspf_float64 value);

   /** @brief Sets theVerticalResolution to value. */
   void setVerticalResolution(rspf_float64 value);

   /** @brief Sets theHorizontalResolution to value. */
   void setHorizontalResolution(rspf_float64 value);
   
   /** @brief Sets theVerticalInterval to value. */
   void setVerticalInterval(rspf_float64 value);
   
   /** @brief Sets theHorizontalInterval to value. */
   void setHorizontalInterval(rspf_float64 value);
   
private:   
   /*!
    * 8 byte rspf_float64 and is the upper left lat or
    * North West portion of the image.
    */
   rspf_float64 theUpperLeftLat;

   /*!
    * 8 byte rspf_float64 and is the upper left lon
    * (North west)
    */
   rspf_float64 theUpperLeftLon;

   rspf_float64 theLowerLeftLat;
   rspf_float64 theLowerLeftLon;

   rspf_float64 theUpperRightLat;
   rspf_float64 theUpperRightLon;

   rspf_float64 theLowerRightLat;
   rspf_float64 theLowerRightLon;

   rspf_float64 theVerticalResolution;
   rspf_float64 theHorizontalResolution;

   rspf_float64 theVerticalInterval;
   rspf_float64 theHorizontalInterval;
};

#endif
