//*******************************************************************
// Copyright (C) 2005 Garrett Potts
//
// LGPL
// 
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfApplanixEOFile.h 20483 2012-01-21 15:42:22Z dburken $
#ifndef rspfApplanixEOFile_HEADER
#define rspfApplanixEOFile_HEADER 1

#include <rspf/base/rspfReferenced.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfString.h>
#include <iostream>
#include <map>
#include <vector>

class RSPF_DLL rspfApplanixEORecord : public rspfReferenced
{
public:

   RSPF_DLL friend std::ostream& operator <<(std::ostream& out, const rspfApplanixEORecord& src);
   rspfApplanixEORecord(const rspfApplanixEORecord& src)
      :theField(src.theField)
   {
   }
   rspfApplanixEORecord(rspf_uint32 size=0)
   {
      setSize(size);
   }

   void setSize(rspf_uint32 size)
   {
      theField.resize(size);
   }
   const rspfString& operator [](rspf_uint32 idx)const
   {
      return theField[idx];
   }
   rspfString& operator [](rspf_uint32 idx)
   {
      return theField[idx];
   }

   std::vector<rspfString> theField;
};

class RSPF_DLL rspfApplanixEOFile : public rspfReferenced
{
public:
   RSPF_DLL friend std::ostream& operator <<(std::ostream& out, const rspfApplanixEOFile& src);
   rspfApplanixEOFile();

   /**
    * Will parse the entire Exterior orientation file and then index any record
    * ID's found.  This calls the parseStream method.
    *
    * @param file  The exterior orientation file
    */ 
   bool parseFile(const rspfFilename& file);

   /**
    * Will parse the stream and set internal queriable parameters.
    * @param inThe input stream containing the Applanix format exterior orientation file
    * 
    */
   bool parseStream(std::istream& in);
   

   /**
    * Utility method that just checks for an APPlanix exterior orientation header.
    * Will call the isEOFile(istream) method.
    *
    * @param file theExterior orientation file.
    */ 
   bool isEOFile(const rspfFilename& file)const;

   /**
    * Utility method that just checks for an APPlanix exterior orientation header.
    */ 
   bool isEOFile(std::istream& in)const;

   /**
    * Parses the header of the exterior orientation file.
    * Calls the parseHeader(istream, header) method.
    *
    * @param file The filename that is an Applanix Exterior orientation file.
    * @param header The resulting header that was parsed out.
    */
   bool parseHeader(const rspfFilename& file,
                    rspfString& header)const;

   /**
    * Parses the header of the exterior orientation file.
    * Calls the parseHeader(istream, header) method.
    *
    * @param in The input stream That contains an exterior orientation 
    * @param header The resulting header that was parsed out.
    */
   bool parseHeader(std::istream& in,
                    rspfString& header)const;

   /**
    * Is a case insensitive parse of the column names for the orientation
    * record.  Note this will check to see if the search key is contained
    * within any column heading.  So if a Heading has something like
    * "ID MORE TEXT"  then you can call this with just "ID" and get a match
    *
    * @param searchKey  the substring to search for
    * @return The id of the column or -1 if can't find the column
    */
   rspf_int32 getFieldIdxLike(const rspfString& searchKey)const;
   
   
   /**
    * Is a case insensitive parse of the column names for the orientation
    * record.  Note this will check to see if the search key exactly matches
    * a column heading.  So if a Heading has something like
    * "ID MORE TEXT"  then you can call this with just "ID MORE TEXT" to have a match
    *
    * @param searchKey  the substring to search for
    *
    * @return The id of the column or -1 if can't find the column
    */
   rspf_int32 getFieldIdx(const rspfString& searchKey)const;
   
   /** 
    * Will index all records based on the "ID #" field for fast searching.
    */ 
   void indexRecordIds();

   /**
    *  Given an ID it will search and find an EO record that matches the ID.
    *
    *  If the records are not indexed it will do a linear search on the record array.
    *
    *  @param id  The id of the record to find.
    *  
    *  @return The exterior orientation for that record.
    */
   const rspfRefPtr<rspfApplanixEORecord> getRecordGivenId(const rspfString& id)const;

   rspf_uint32 getNumberOfRecords()const;
   const rspfRefPtr<rspfApplanixEORecord> getRecord(rspf_uint32 idx)const;
      
   bool isHeightAboveMSL()const;
   bool isEcefModel()const;
   bool isUtmFrame()const;
   rspf_int32 getUtmZone()const;
   rspfString getUtmHemisphere()const;

   rspfString getSbetField()const;
   double getBoreSightTx()const;
   double getBoreSightTy()const;
   double getBoreSightTz()const;
   double getLeverArmLx()const;
   double getLeverArmLy()const;
   double getLeverArmLz()const;
   double getKardinalKappa()const;
   double getShiftValuesX()const;
   double getShiftValuesY()const;
   double getShiftValuesZ()const;

   rspfString getMappingFrameDatum()const;
   rspfString getMappingFrameProjection()const;
   double getMappingFrameCentralMeridian()const;
   double getMappingFrameOriginLatitude()const;
   double getMappingFrameGridScaleFactor()const;
   double getMappingFrameFalseEasting()const;
   double getMappingFrameFalseNorthing()const;

   rspfString convertToOssimDatumCode(const rspfString& datum)const;
   
protected:
   rspfString theHeader;
   rspfString theVersion;
   rspfString theBoreSightTx;
   rspfString theBoreSightTy;
   rspfString theBoreSightTz;
   rspfString theKappaCardinal;
   rspfString theLeverArmLx;
   rspfString theLeverArmLy;
   rspfString theLeverArmLz;
   rspfString theShiftValuesX;
   rspfString theShiftValuesY;
   rspfString theShiftValuesZ;
   rspfString theSbetField;

   rspfString theMappingFrameDatum;
   rspfString theMappingFrameProjection;
   rspfString theCentralMeridian;
   rspfString theOriginLatitude;
   rspfString theGridScaleFactor;
   rspfString theFalseEasting;
   rspfString theFalseNorthing;
   rspfString theUtmZone;
   rspfString theUtmHemisphere;

   rspf_float64 theMinLat;
   rspf_float64 theMinLon;
   rspf_float64 theMaxLat;
   rspf_float64 theMaxLon;
   std::vector<rspfString> theRecordFormat;
   std::vector< rspfRefPtr<rspfApplanixEORecord> > theApplanixRecordList;
   std::map<rspfString, rspfRefPtr<rspfApplanixEORecord>, rspfStringLtstr> theRecordIdMap;
};

#endif
