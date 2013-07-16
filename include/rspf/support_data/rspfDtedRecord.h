//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Ken Melero
// 
// Description:  This class gives access to the Data Record Description
//               of a DTED Level 1 file.
//
// Notes:  Each elevation is a true value referenced to meas sea level
//         (MSL) datum recorded to the nearest meter.  The horizontal
//         position is referenced to precise longitude-latitiude
//         locations in terms of the current World Geodetic System
//         (WGS84) determined for each file by reference to the origin
//         at the southwest corner.  The elevations are evenly spaced
//         in latitude and longitude at the interval designated in the
//         User Header Label (UHL) in South to North profile sequence.
//
//********************************************************************
// $Id: rspfDtedRecord.h 14248 2009-04-08 19:38:11Z dburken $
#ifndef rspfDtedRecord_H
#define rspfDtedRecord_H
#include <iosfwd>

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfErrorStatusInterface.h>

class RSPF_DLL rspfDtedRecord : public rspfErrorStatusInterface
{
public:
   rspfDtedRecord(std::istream& in,
                   rspf_int32 offest,
                   rspf_int32 num_points);
   // NOTE:  When using this class be sure that as you cycle through
   //        all the points within a record, that you verify they are
   //        sequential.  According to the DTED Specification
   //        (MIL-PRF-89020A) issued in 19 April 1996, page 22 all
   //        records must be sequential.  If not, file may be corrupt.
   //
   //        To verify this, you can do the following:
   //
   //        int count = 0;
   //        for(int i = 0; i < num_lon_lines; i++)
   //          rec[i] = new rspfDtedRecord(theFileDesc, offset,
   //                       num_lat_points);
   //          if(count != (rec[i]->dataBlockCount() + 1))
   //             ERROR -- Records are not sequential
   //          count = rec[i]->dataBlockCount();

   ~rspfDtedRecord();
   
   enum
   {
      DATA_LENGTH = 12,
      DATA_BLOCK_COUNT = 2,
      DATA_LON_COUNT = 4,
      DATA_LAT_COUNT = 6,
      DATA_ELEV_START = 8,
      DATA_BYTES_PER_POINT = 2,
   };

   // The Recognition Sentinel signifies if the Data Record exists.
   rspfString recognitionSentinel() const;

   rspf_int32  dataBlockCount()   const;
   rspf_int32  lonCount()         const;
   rspf_int32  latCount()         const;
   rspf_uint32 checkSum()         const;
   rspf_uint32 computedCheckSum() const;
   rspf_int32  numPoints()        const;

   // Access methods for the elevation data
   rspf_int32   getPoint(rspf_int32 i)     const;
   rspf_uint16  getPointData(rspf_int32 i) const;
   rspf_int32*  points()                    const;
   rspf_uint16* pointsData()                const;
   
   rspf_int32 startOffset() const;
   rspf_int32 stopOffset()  const;

   friend RSPF_DLL std::ostream& operator<<( std::ostream& os,
                                              const rspfDtedRecord& rec);
   void parse(std::istream& in);

private:
   // prevent use
   rspfDtedRecord(const rspfDtedRecord& source);

   rspf_int32  theFile;

   /*!
    * 252 (8 bit)
    */
   rspfString theRecSen;

   /*!
    *  Sequential count of the block within the file.
    */
   rspf_int32  theDataBlockCount;

   /*!
    *  Count of the meridian.
    *  True longitude = longitude count x data interval + origin
    *  (Offset from the SW corner longitude)
    */
   rspf_int32  theLonCount;

   /*!
    *  Count of the parallel.
    *  True latitude = latitude count x data interval + origin
    *  (Offset from the SW corner latitude)
    */
   rspf_int32  theLatCount;

   /*!
    *  Algebraic addition of contents of block.
    *  The checksum is computed algebraically using integer arithmetic by
    *  summing all header and elevation bytes contained int the record as
    *  8-bit values.  Each byte is considered an unsigned, 8-bit value for
    *  checksum calculation.
    */
   rspf_uint32  theCheckSum;

   /*!
    *  All the elevation points in a Data Record as ints.
    *  (ie. all the latitudal points in a longitudinal line)
    */
   rspf_int32* thePoints;

   /*!
    *  All the elevation points in a Data Record as unsigned shorts.
    *  (ie. all the latitudal points in a longitudinal line)
    */
   rspf_uint16* thePointsData;

   /*!
    *  Our computed check sum.  This should match the checksum
    *  at the end of the Data Record.
    */
   rspf_uint32  theComputedCheckSum;

   /*!
    *  The number of points in a longitudinal line.
    */
   rspf_int32  theNumPoints;

   rspf_int32  theStartOffset;
   rspf_int32  theStopOffset;

   /*!
    *  Compute the check sum for the Data Record and compare against
    *  the parsed check sum from the data record. This must be correct
    *  to be a valid data record. If not, there is a chance of a
    *  corrupted elevation cell.
    *
    *  @return true if check sum validates, false if not.
    */
   bool validateCheckSum(std::istream& in);
};

#endif
