//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
// Description:
//
// Contains class declaration for rspfImageMetaData.
// 
//*******************************************************************
//  $Id: rspfImageMetaData.h 21527 2012-08-26 16:50:49Z dburken $
#ifndef rspfImageMetaData_HEADER
#define rspfImageMetaData_HEADER 1

#include <rspf/base/rspfConstants.h>
#include <iosfwd>
#include <string>

class rspfKeywordlist;

class RSPF_DLL rspfImageMetaData
{
public:
   rspfImageMetaData();
   rspfImageMetaData(rspfScalarType aType,
                      rspf_uint32 numberOfBands);
   rspfImageMetaData(const rspfImageMetaData& rhs);

   const rspfImageMetaData& operator=(const rspfImageMetaData& rhs);
   
   ~rspfImageMetaData();
   
   void clear();

   void setDefaultsForArrays();
   
   void setNumberOfBands(rspf_uint32 numberOfBands);
   
   rspf_uint32 getNumberOfBands()const;
   
   void setScalarType(rspfScalarType aType);
   
   rspfScalarType getScalarType()const;

   /** @return The bytes per pixel. This is for a single band. */
   rspf_uint32 getBytesPerPixel() const;
   
   double getMinPix(rspf_uint32 band)const;
   
   void setMinPix(rspf_uint32 band, double pix);
   
   void setMaxPix(rspf_uint32 band, double pix);
   
   void setNullPix(rspf_uint32 band, double pix);
   
   double getMaxPix(rspf_uint32 band)const;
   
   double getNullPix(rspf_uint32 band)const;
   
   const double* getMinPixelArray()const;
   
   const double* getMaxPixelArray()const;
   
   const double* getNullPixelArray()const;

   void setMinValuesValid(bool flag);
   
   void setMaxValuesValid(bool flag);
   
   void setNullValuesValid(bool flag);

   bool getMinValuesValidFlag()const;

   bool getMaxValuesValidFlag()const;

   bool getNullValuesValidFlag()const;

   bool isValid()const;
   
   bool loadState(const rspfKeywordlist& kwl,
                  const char* prefix=0);
   bool saveState(rspfKeywordlist& kwl,
                  const char* prefix=0)const;

   /**
    * @brief Method to update band values.
    *
    * Assumes a previous initialization and does not error out if band data is
    * not found.  This does NOT clear the object prior to loading like the
    * loadState(...) method.  Can be used to update min/max values from a
    * "compute min max".  
    *
    * @param kwl Keyword list to initialize from.
    *
    * @param prefix Prefix, e.g. "image0.".
    */
   void updateMetaData( const rspfKeywordlist& kwl,
                        const std::string& prefix );

   /**
    * @brief Print method.
    * @return std::ostream&
    */
   std::ostream& print(std::ostream& out) const;

   /**
    * @note  Since the print method is virtual, derived classes only need
    *        to implement that, not an addition operator<<.
    */
   friend RSPF_DLL std::ostream& operator<<(std::ostream& out,
                                             const rspfImageMetaData& obj);
   
private:

   /**
    * Looks for rspfKeywordNames::NUMBER_BANDS_KW, if not found looks for.
    */
   rspf_uint32 getBandCount(const rspfKeywordlist& kwl,
                             const std::string& prefix) const;
   
   double*         theNullPixelArray;
   double*         theMinPixelArray;
   double*         theMaxPixelArray;
  
   bool            theMinValuesValidFlag;
   bool            theMaxValuesValidFlag;
   bool            theNullValuesValidFlag;
   
   rspfScalarType theScalarType;
   rspf_uint32    theBytesPerPixel;
   rspf_uint32    theNumberOfBands;
};

#endif /* #ifndef rspfImageMetaData_HEADER */
