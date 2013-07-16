//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
// 
// Class declaration for rspfIkonosMetaData.
// 
// This class parses a Space Imaging Ikonos meta data file.
//
//********************************************************************
// $Id: rspfIkonosMetaData.h 15828 2009-10-28 13:11:31Z dburken $

#ifndef rspfIkonosMetaData_HEADER
#define rspfIkonosMetaData_HEADER

#include <iosfwd>

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfString.h>

class rspfFilename;
class rspfKeywordlist;


class RSPFDLLEXPORT rspfIkonosMetaData : public rspfObject
{
public:

   /** @brief default constructor */
   rspfIkonosMetaData();


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
   
   rspfString   getSensorID() const;

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
    * @brief Method to parse Ikonos metadata file.
    *
    * @param metadata File name usually in the form of
    * "po_149875_metadata.txt."
    *
    * @return true on success, false on error.
    */
   bool parseMetaData(const rspfFilename& metadata);

   /**
    * @brief Method to parse Ikonos header file.
    *
    * @param metadata File name usually in the form of
    * "po_149875_pan_0000000.hdr"
    *
    * @return true on success, false on error.
    */
   bool parseHdrData(const rspfFilename& data_file);

   /**
    * @brief Method to parse Ikonos rpc file.
    *
    * @param metadata File name usually in the form of
    * "po_149875_pan_0000000_rpc.txt"
    *
    * @return true on success, false on error.
    *
    * @note Currently NOT implemented.
    */
   bool parseRpcData(const rspfFilename& data_file);

protected:
   /** virtual destructor */
   virtual ~rspfIkonosMetaData();

private:
   rspf_float64 theNominalCollectionAzimuth;
   rspf_float64 theNominalCollectionElevation;
   rspf_float64 theSunAzimuth;
   rspf_float64 theSunElevation;
   rspf_uint32  theNumBands;
   rspfString   theBandName;
   rspfString   theProductionDate;
   rspfString   theAcquisitionDate;
   rspfString   theAcquisitionTime;
   rspfString   theSensorID;
   
TYPE_DATA   
};

#endif /* #ifndef rspfIkonosMetaData_HEADER */
