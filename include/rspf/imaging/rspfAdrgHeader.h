//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Ken Melero
// Description: This class encapsulates the image parameters and
//              support data for a ADRG image file(s).
//
//********************************************************************
// $Id: rspfAdrgHeader.h 10266 2007-01-14 19:27:14Z dburken $

#ifndef rspfAdrgHeader_H
#define rspfAdrgHeader_H

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfIrect.h>

class RSPF_DLL rspfAdrgHeader
{
public:
   // Instantiated with the image file (.img)
   rspfAdrgHeader(const rspfFilename& img_file);
   ~rspfAdrgHeader();
   
   enum ErrorStatus
   {
      RSPF_OK    = 0,
      RSPF_ERROR = 1
   };

   enum
   {
      ADRG_TILE_SIZE = 128
   };
   
   int errorStatus() const { return theErrorStatus; }

   rspfFilename file()
      const { return theHeaderFile; }
   rspfFilename imageFile()
      const { return theImageFile; }
   rspfIrect validImageRect()
      const { return theValidImageRect; }

   rspf_uint32    pixelType()      const { return thePixelType; }
   rspf_uint32    interleaveType() const { return theInterleaveType; }
   rspf_uint32    numberOfBands()  const { return theNumberOfBands; }
   rspf_uint32    lines()          const { return theLines; }
   rspf_uint32    tlines()         const { return theLinesInTiles; }
   rspf_uint32    samples()        const { return theSamples; }
   rspf_uint32    tsamples()       const { return theSamplesInTiles; }
   rspf_uint32    headerSize()     const { return theHeaderSize; }
   rspf_uint32    startRow()       const { return theStartRow; }
   rspf_uint32    startCol()       const { return theStartCol; }
   rspf_uint32    stopRow()        const { return theStopRow; }
   rspf_uint32    stopCol()        const { return theStopCol; }

   rspf_uint32    tim(rspf_uint32 row, rspf_uint32 col) const;
   
   rspfString minLon() const { return theMinLon; }
   rspfString minLat() const { return theMinLat; }
   rspfString maxLon() const { return theMaxLon; }
   rspfString maxLat() const { return theMaxLat; }

   double minLongitude()   const;
   double minLatitude()    const;
   double maxLongitude()   const;
   double maxLatitude()    const;
   
   friend RSPF_DLL std::ostream& operator<<(std::ostream& os,
                                             const rspfAdrgHeader& adrg);

private:

   // Returns the longitude in decimal degrees given the formatted string.
   double parseLongitudeString(const rspfString& lon) const;

   // Returns the latitude in decimal degrees given the formatted string.
   double parseLatitudeString(const rspfString& lat) const;
   
   // prevent use.
   rspfAdrgHeader();
   rspfAdrgHeader(const rspfAdrgHeader& source);

   ErrorStatus theErrorStatus;

   rspfFilename theHeaderFile;
   rspfFilename theImageFile;
   rspfIrect    theValidImageRect;
   
   rspf_uint32  thePixelType;
   rspf_uint32  theInterleaveType;
   rspf_uint32  theNumberOfBands;
   rspf_uint32  theLines;
   rspf_uint32  theLinesInTiles;
   rspf_uint32  theSamples;
   rspf_uint32  theSamplesInTiles;
   rspf_uint32  theHeaderSize;
   rspf_uint32  theStartRow;
   rspf_uint32  theStartCol;
   rspf_uint32  theStopRow;
   rspf_uint32  theStopCol;
   rspf_uint32* theTim;   // Tile index map value -- only valid if TIF is 'Y'.
   
   rspfString theMinLon;
   rspfString theMinLat;
   rspfString theMaxLon;
   rspfString theMaxLat;
   rspfString theTif;  // Tile index map flag -- Y indicates there are tiles
                        // with no data; N indicates that all tiles contain
                        // RGB graphic data.

   void parse();
};

#endif
