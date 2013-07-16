//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
// 
// Class declaration for rspfQuickbirdMetaData.
// 
// This class parses a Space Imaging Quickbird meta data file.
//
//********************************************************************
// $Id: rspfQuickbirdMetaData.h 14412 2009-04-27 16:58:46Z dburken $

#ifndef rspfQuickbirdMetaData_HEADER
#define rspfQuickbirdMetaData_HEADER 1

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfString.h>

#include <iosfwd>

class rspfFilename;
class rspfKeywordlist;

class RSPFDLLEXPORT rspfQuickbirdMetaData : public rspfObject
{
public:

   /** @brief default constructor */
   rspfQuickbirdMetaData();

   /** virtual destructor */
   virtual ~rspfQuickbirdMetaData();

   /**
    * @brief Open method that takes the image file, derives the metadata,
    * header and rpc files, then calls parse methods parseMetaData,
    * parseHdrData, and parseRpcData.
    *
    * @param imageFile Usually in the form of "po_2619900_pan_0000000.tif".
    *
    * @return true on success, false on error.
    */
   bool open(const rspfFilename& imageFile);

   void clearFields();

   //---
   // Convenient method to print important image info:
   //---
   virtual std::ostream& print(std::ostream& out) const;

   /**
    * Method to save the state of the object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix = 0) const;
   
   /**
    * Method to the load (recreate) the state of the object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix = 0);

   /**
    * @brief Method to parse Quickbird metadata file.
    *
    * @param metadata File name usually in the form of
    * ".IMD."
    *
    * @return true on success, false on error.
    */
   bool parseMetaData(const rspfFilename& metadata);

   /**
    * @brief Method to parse Quickbird rpc file.
    *
    * @param metadata File name usually in the form of
    * ".GEO.txt"
    *
    * @return true on success, false on error.
    *
    * @note Currently NOT implemented.
    */
   bool parseGEOData(const rspfFilename& data_file);

   bool parseATTData(const rspfFilename& data_file);

   bool parseEPHData(const rspfFilename& data_file);

   /** @return theSatID */
   rspfString getSatID() const;

   bool getEndOfLine( char * fileBuf,
                      rspfString lineBeginning,
                      const char * format,
                      rspfString & name );

   const rspfIpt& getImageSize() const;

/*****************************************
*parseATTData EPH GEO IMD RPB TIL
*
****************************************/
   
private:

   rspfString         theGenerationDate;
   rspfString         theBandId;
   int                 theBitsPerPixel;	
   rspfString         theSatID;
   rspfString         theTLCDate;
   rspf_float64       theSunAzimuth;
   rspf_float64       theSunElevation;
   rspf_float64       theSatAzimuth;
   rspf_float64       theSatElevation;
   int                 theTDILevel;
   std::vector<double> theAbsCalFactors;
   rspfString         theBandNameList;
   rspfIpt            theImageSize;

TYPE_DATA
};

#endif /* #ifndef rspfQuickbirdMetaData_HEADER */
