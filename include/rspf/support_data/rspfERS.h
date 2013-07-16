//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Chong-Ket Chuah
// Contributor: Anrew Huang
//
// Description: 
// This class parses a ER Mapper raster file format header.
//
//********************************************************************
// $Id: rspfERS.h 13216 2008-07-23 18:56:53Z dburken $

#ifndef rspfERS_HEADER
#define rspfERS_HEADER

#include <iosfwd>
#include <vector>

#include <rspf/base/rspfErrorStatusInterface.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfKeywordlist.h>

class rspfERS : public rspfErrorStatusInterface
{
public:

   rspfERS();
   rspfERS(const char* file);
   virtual ~rspfERS() {};

   void clearFields();
   // DatasetHeader 
   double      theVersion;
   rspfString theFilename;
   rspfString theDescription;
   
//   tm lastupdated;
   rspfString theSensorname;
//   tm sensedate;
   rspf_int32       theHeaderOffset; // for non-ermapper file
   rspfString theDatasetType;	      // ERStorage
   rspfString theDatatype;           // Raster
   rspfString theByteorder;          //MSBFirst or LSBFirst
   rspfString theComments;           // arbitrary comment field
   
   //  CoordSpace 
   rspfString theDatum; /* geodetic datum */
   rspfString theProjection;
   rspfString theCoordSysType;
   rspfString theUnits;
   double      theRotation;

   rspfUnitType theTieUnitType;
   // RasterInfo 
   rspfScalarType theCelltype;
   rspf_int32 theCellsizeof;
   bool theHasNullCells;
   double theNullCell;
   double theCellSizeX;
   double theCellSizeY;
   rspf_int32 theLine;
   rspf_int32 theSample;
   double theOriginX;
   double theOriginY;
   rspf_int32 theBands;
   std::vector<rspfString> theBandID;
   
   virtual std::ostream& print(std::ostream& out) const;
   
   bool writeFile(const rspfFilename& file);
   
   bool toOssimProjectionGeom(rspfKeywordlist& kwl,
                              const char* prefix=NULL)const;
   rspfString toOssimProjection()const;
   rspfString toOssimDatum()const;
   rspf_int32 errorStatus() const { return theErrorStatus; }
   
private:

   void parseError(const char *msg);
   std::vector<rspfString> parseLine(std::istream& in);
   void parseHeader(std::istream& fptr);
   void parseRasterInfo(std::istream& fptr);
   void parseCoordinateSpace(std::istream& fptr);
};

#endif
