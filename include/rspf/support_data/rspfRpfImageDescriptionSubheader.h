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
// $Id: rspfRpfImageDescriptionSubheader.h 14241 2009-04-07 19:59:23Z dburken $
#ifndef rspfRpfImageDescriptionSubheader_HEADER
#define rspfRpfImageDescriptionSubheader_HEADER

#include <iosfwd>

#include <rspf/base/rspfConstants.h>
#include <rspf/support_data/rspfRpfConstants.h>
#include <rspf/base/rspfErrorContext.h>

class rspfRpfImageDescriptionSubheader
{
public:
   friend std::ostream& operator <<(
      std::ostream& out, const rspfRpfImageDescriptionSubheader& data);

   rspfRpfImageDescriptionSubheader();
   ~rspfRpfImageDescriptionSubheader(){}

   rspfErrorCode parseStream(std::istream& in, rspfByteOrder byteOrder);
   
   rspf_uint32 getStartOffset()const{return theStartOffset;}
   rspf_uint32 getEndOffset()const{return theEndOffset;}

   bool isSubframeMaskTableOffsetNull()const
   {
      return (theSubframeMaskTableOffset == RSPF_RPF_ULONG_NULL);
   }
   bool isTransparencyMaskTableOffsetNull()const
   {
      return (theTransparencyMaskTableOffset == RSPF_RPF_ULONG_NULL);
   }
   rspf_uint16 getNumberOfSpectralGroups()const
   {
      return theNumberOfSpectralGroups;
   }
   rspf_uint16 getNumberOfSubframeTables()const
   {
      return theNumberOfSubframeTables;
   }
   rspf_uint16 getNumberOfSpectralBandTables()const
   {
      return theNumberOfSpectralBandTables;
   }
   rspf_uint16 getNumberOfSpectralBandLinesPerImageRow()const
   {
      return theNumberOfSpectralBandLinesPerImageRow;
   }
   rspf_uint16 getNumberOfSubframesHorizontal()const
   {
      return theNumberOfSubframesHorizontal;
   }
   rspf_uint16 getNumberOfSubframesVertical()const
   {
      return theNumberOfSubframesVertical;
   }
   rspf_uint32  getNumberOfOutputColumnsPerSubframe()const
   {
      return theNumberOfOutputColumnsPerSubframe;
   }
   rspf_uint32  getNumberOfOutputRowsPerSubframe()const
   {
      return theNumberOfOutputRowsPerSubframe;
   }
   rspf_uint32  getSubframeMaskTableOffset()const
   {
      return theSubframeMaskTableOffset;
   }
   rspf_uint32  getTransparencyMaskTableOffset()const
   {
      return theTransparencyMaskTableOffset;
   }

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
   
private:
   void clearFields();
   // work variables to store useful information.
   rspf_uint32 theStartOffset;
   rspf_uint32 theEndOffset;

   rspf_uint16 theNumberOfSpectralGroups;
   rspf_uint16 theNumberOfSubframeTables;
   rspf_uint16 theNumberOfSpectralBandTables;
   rspf_uint16 theNumberOfSpectralBandLinesPerImageRow;

   /*!
    * Each frame file could be divided into sub frames.
    * This identifies the number of subframes in the
    * horizontal direction.
    */
   rspf_uint16 theNumberOfSubframesHorizontal;

   /*!
    * Each frame file could be divided into sub frames.
    * This identifies the number of subframes in the
    * vertical direction.
    */
   rspf_uint16 theNumberOfSubframesVertical;
   rspf_uint32 theNumberOfOutputColumnsPerSubframe;
   rspf_uint32 theNumberOfOutputRowsPerSubframe;
   rspf_uint32 theSubframeMaskTableOffset;
   rspf_uint32 theTransparencyMaskTableOffset;
};

#endif
