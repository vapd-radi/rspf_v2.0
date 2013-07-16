//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
// 
// Description:
//
// Contains class declaration for  rspfNormalizedU11RemapTable.  Table for
// normalizing unsigned 11 bit data.
//
//*******************************************************************
//  $Id: rspfNormalizedU11RemapTable.h 16034 2009-12-03 14:49:25Z dburken $

#ifndef rspfNormalizedU11RemapTable_HEADER
#define rspfNormalizedU11RemapTable_HEADER

#include <rspf/imaging/rspfNormalizedRemapTable.h>
#include <rspf/base/rspfCommon.h> /* for round */

/**
 * @class rspfNormalizedU11RemapTable
 *
 * @brief Eleven bit normalized remap table to go to/from normalized value
 * to pixel value.
 */
class RSPF_DLL rspfNormalizedU11RemapTable : public rspfNormalizedRemapTable
{
public:

   /** @brief default constructor */
   rspfNormalizedU11RemapTable();
   /** @brief virtual destructor */
   virtual ~rspfNormalizedU11RemapTable();


   enum
   {
      TABLE_ENTRIES = 2048 //  2^11
   };

   /**
    * @brief Gets the number of table entries.
    * @return The number of entries in a table.
    */
   virtual rspf_int32 getEntries() const;

   /**
    *  @brief Gets a normalized value (between '0.0' and '1.0') from
    *  a pixel value.
    *
    *  @return Value between 0.0 and 1.0.
    */
   virtual rspf_float64 operator[](rspf_int32 pix) const;

   /**
    *  @brief Gets a normalized value (between '0.0' and '1.0') from
    *  a pixel value.
    *
    *  @return Value between 0.0 and 1.0.
    */
   virtual rspf_float64 normFromPix(rspf_int32 pix) const;

   /**
    * @brief Returns an pixel value as an int from a normalized value.
    *
    * @return Value between scalar range of remap table.
    */
   virtual rspf_int32 pixFromNorm(rspf_float64 normPix) const;   

private:

   static rspf_float64  theTable[TABLE_ENTRIES];
   static bool theTableIsInitialized;
   
};

inline rspf_int32 rspfNormalizedU11RemapTable::getEntries() const
{
   return TABLE_ENTRIES;
}

inline rspf_float64 rspfNormalizedU11RemapTable::operator[](
   rspf_int32 pix) const
{
   return ( (pix < TABLE_ENTRIES) ? (pix >= 0 ? theTable[pix] : 0.0) : 1.0);  
}

inline rspf_float64 rspfNormalizedU11RemapTable::normFromPix(
   rspf_int32 pix) const
{
   return ( (pix < TABLE_ENTRIES) ? (pix >= 0 ? theTable[pix] : 0.0) : 1.0);
}

inline rspf_int32 rspfNormalizedU11RemapTable::pixFromNorm(
   rspf_float64 normPix) const
{
   if(normPix <= 0.0) return 0;
   
   // un-normalize...
   rspf_float64 p = normPix * getNormalizer();

   // Ensure pixel is in range.
   p = ( (p < TABLE_ENTRIES) ? (p >= 0.0 ? p : 0.0) : getNormalizer());

   // Since going from double to int round...
   p = rspf::round<rspf_int32>(p);

   if(p == 0.0)
   {
      p = 1.0;
   }
   
   return static_cast<rspf_int32>(p);
}

#endif