//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  David Burken
//
// Description:
//
// Contains class declaration for CcfHead.
//
// Notes:
// - This is not a full blown CCF header class as it only captures the
//   data needed for loading.
// - Currently only version 6 is supported.
//
//*******************************************************************
//  $Id: rspfCcfHead.h 10265 2007-01-14 19:18:43Z dburken $
#ifndef rspfCcfHead_HEADER
#define rspfCcfHead_HEADER

#include <iosfwd>
#include <vector>

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfErrorStatusInterface.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfIpt.h>

class rspfIrect;

//***************************************************************************
// CLASS:  rspfCcfHead
//***************************************************************************
class RSPF_DLL rspfCcfHead : public rspfErrorStatusInterface
{
public:

   rspfCcfHead();
   
   rspfCcfHead(const char* ccf_file);

   ~rspfCcfHead ();

   /*!
    *  Reads the ccf header and initializes data members.  Returns true on
    *  successful read, false on error.
    */
   bool parseCcfHeader(const char* ccf_file);

   virtual std::ostream& print(std::ostream& out) const;

   rspf_uint32 numberOfReducedResSets() const { return theNumberOfRLevels; }

   rspf_uint32 numberOfLines(rspf_uint32 reduced_res_level) const;

   rspf_uint32 numberOfSamples(rspf_uint32 reduced_res_level) const;

   rspf_uint32 chunksInLineDir(rspf_uint32 reduced_res_level) const;
   
   rspf_uint32 chunksInSampleDir(rspf_uint32 reduced_res_level) const;

   std::streampos startOfData(rspf_uint32 reduced_res_level) const;

   /*!
    *  Returns the highest available reduced resolution level.
    *  Zero based.
    */
   rspf_uint32 highestReducedResSet() const
      { return !theNumberOfRLevels ? 0 : (theNumberOfRLevels - 1); }

   /*!
    *  Returns the zero based image rectangle for the reduced resolution
    *  level passed in.  Returns a rectangle of (0,0,0,0) if reduced_res_level
    *  is out of range.
    */
   rspfIrect imageRect(rspf_uint32 reduced_res_level) const;

   /*!
    *  Returns the ccf file name as a String.
    */
   rspfString imageFile() const { return theCcfFile; }

   /*!
    *  Returns the radiometry string.
    */
   rspfString radiometry() const { return theRadiometryString; }

   /*!
    *  Returns the number of bands.
    */
   rspf_uint32 numberOfBands() const { return theNumberOfBands; }

   /*!
    *  Returns the enumerated pixel type.
    */
   rspfScalarType pixelType() const { return thePixelType; }

   /*!
    *  Returns the bytes per chunk.
    */
   rspf_uint32 bytesPerChunk() const { return theBytesPerChunk; }

   /*!
    *  Returns the bytes per chip.
    */
   rspf_uint32 bytesPerChip() const { return theBytesPerChip; }

   /*!
    *  Returns the number of chips in sample direction of a chunk.
    */
   rspf_uint32 sampleChipsPerChunk() const { return theSampleChipsPerChunk; }

   /*!
    *  Returns the number of chips in line direction of a chunk.
    */
   rspf_uint32 lineChipsPerChunk() const { return theLineChipsPerChunk; }

   /*!
    *  Returns the samples per chip.
    */
   rspf_uint32 samplesPerChip() const { return theSamplesPerChip; }

   /*!
    *  Returns the lines per chip.
    */
   rspf_uint32 linesPerChip() const { return theLinesPerChip; }

   /*!
    *  Returns the number of pixels in a single band of a chip.
    *  Note:  This is not in bytes but in pixels.
    */
   rspf_uint32 chipSizePerBand() const
      { return theLinesPerChip * theSamplesPerChip; }

   const std::vector<rspfIpt>& getValidImageVertices()const
      {
         return theValidImageVertices;
      }
private:

   /*!
    *  Parses a version five header.
    */
   bool parseV5CcfHeader(std::istream& is);

   /*!
    *  Parses a version six header.
    */
   bool parseV6CcfHeader(std::istream& is);

   /*!
    *  Parses the radiometry string and initializes "theNumberOfBands" and
    *  "thePixelType".
    */
   void parseRadString();

   rspfString            theCcfFile;
   rspf_uint32           theNumberOfBands;
   rspfScalarType        thePixelType;
   rspfString            theFileType;
   rspf_uint32           theVersionNumber;
   rspf_uint32           theLinesPerChip;
   rspf_uint32           theSamplesPerChip;
   rspf_uint32           theLineChipsPerChunk;
   rspf_uint32           theSampleChipsPerChunk;
   rspf_uint32           theRectInFullImageStartLine;
   rspf_uint32           theRectInFuleImageStartSample;
   rspf_uint32           theRectInFullImageStopLine;
   rspf_uint32           theRectInFuleImageStopSample;
   rspf_uint32           theNumberOfValidImageVertices;
   std::vector<rspfIpt>  theValidImageVertices;
   rspf_uint32           theFirstBandHeaderPointer;
   rspfString            theRadiometryString;
   rspf_uint32           theBytesPerPixel;
   rspf_uint32           theBytesPerChip;
   rspf_uint32           theBytesPerChunk;
   rspfString            theCompressionType;
   rspf_uint32           theNumberOfRLevels;
   std::vector<bool>           theOccupiedFlag;
   std::vector<std::streampos> theStartOfData;
   std::vector<rspf_uint32>   theNumberOfLines;
   std::vector<rspf_uint32>   theNumberOfSamples;
   std::vector<rspf_uint32>   theChunksInLineDir;
   std::vector<rspf_uint32>   theChunksInSampleDir;
};


#endif // #ifndef CcfHead_HEADER
