//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
// 
// Description:
//
// Contains class declaration for base class rspfNormalizedRemapTable.
// 
//*******************************************************************
//  $Id: rspfNormalizedRemapTable.h 15742 2009-10-17 12:59:43Z dburken $


#ifndef rspfNormalizedRemapTable_HEADER
#define rspfNormalizedRemapTable_HEADER

#include <rspf/base/rspfConstants.h>

/**
 * @class rspfNormalizedRemapTable
 *
 * @brief Base class implemetation of normalized remap tables to go to/from
 * normalized value to pixel value.
 */
class RSPF_DLL rspfNormalizedRemapTable
{
public:

   /** default constructor */
   rspfNormalizedRemapTable();

   /** virtual destructor */
   virtual ~rspfNormalizedRemapTable();

   /**
    *  @brief Gets a normalized value (between '0.0' and '1.0') from
    *  a pixel value.
    *
    *  @return Value between 0.0 and 1.0.
    */
   virtual rspf_float64 operator[](rspf_int32 pix) const = 0;

   /**
    *  @brief Gets a normalized value (between '0.0' and '1.0') from
    *  a pixel value.
    *
    *  @return Value between 0.0 and 1.0.
    */
   virtual rspf_float64 normFromPix(rspf_int32 pix) const = 0;

   /**
    * @brief Returns an pixel value as an int from a normalized value.
    *
    * @return Value between scalar range of remap table.
    */
   virtual rspf_int32 pixFromNorm(rspf_float64 normPix) const = 0;

   /**
    * @brief Pure virtual method to get the number of table entries.
    * @return The number of entries in a table.
    */
   virtual rspf_int32 getEntries() const = 0;

   /**
    * @brief Get the value used to normalize and un-normalize table entries.
    * @return Value used to normalize and un-normalize table entries.
    */
   virtual rspf_float64 getNormalizer() const;

private:

   // Forbid copy constructor and asignment operator.
   rspfNormalizedRemapTable(const rspfNormalizedRemapTable& source);
   rspfNormalizedRemapTable& operator=(const rspfNormalizedRemapTable&);  

 };

inline rspf_float64 rspfNormalizedRemapTable::getNormalizer() const
{
   return static_cast<rspf_float64>(getEntries()-1);
}

#endif
