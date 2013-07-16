//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
// 
// Author: Garrett Potts
// 
// Description: This class give the capability to access tiles from an
//              rpf file.
//
//********************************************************************
// $Id: rspfRpfFrame.h 20324 2011-12-06 22:25:23Z dburken $

#ifndef rspfRpfFrame_HEADER
#define rspfRpfFrame_HEADER 1

#include <rspf/base/rspfReferenced.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfErrorCodes.h>
#include <rspf/support_data/rspfNitfFile.h>
#include <rspf/support_data/rspfRpfColorGrayscaleTable.h>
#include <rspf/support_data/rspfRpfReplaceUpdateTable.h>
#include <iosfwd>
#include <vector>

class rspfRpfHeader;
class rspfRpfAttributes;
class rspfRpfCoverageSection;
class rspfRpfImageDescriptionSubheader;
class rspfRpfImageDisplayParameterSubheader;
class rspfRpfMaskSubheader;
class rspfRpfCompressionSection;
class rspfRpfColorGrayscaleSubheader;
class rspfRpfColorConverterSubsection;

class RSPF_DLL rspfRpfFrame : public rspfReferenced
{
public:
   friend std::ostream& operator <<(std::ostream& out,
                                    const rspfRpfFrame& data);
   rspfRpfFrame();
   ~rspfRpfFrame();
   
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

   rspfErrorCode parseFile(const rspfFilename& filename,
                            bool minimalParse = false);
   
   const rspfRpfHeader* getRpfHeader()const{return theHeader;}

   bool hasSubframeMaskTable()const;

   const vector< vector<rspf_uint32> >& getSubFrameMask(
      rspf_uint32 spectralGroup)
   {
      return theSubframeMaskTable[spectralGroup];
   }
   
   bool fillSubFrameBuffer(rspf_uint8* buffer,
                           rspf_uint32 spectralGroup,
                           rspf_uint32 row,
                           rspf_uint32 col)const;
   
   const rspfRpfCompressionSection* getCompressionSection()const
   {
      return theCompressionSection;
   }
   const vector<rspfRpfColorGrayscaleTable>& getColorGrayscaleTable()const
   {
      return theColorGrayscaleTable;
   }
   const rspfRpfColorConverterSubsection* getColorConverterSubsection()const
   {
      return theColorConverterSubsection;
   }
   const rspfRpfAttributes* getAttributes()const
   {
      return theAttributes;
   }
   const rspfNitfFile* getNitfFile()const
   {
      return theNitfFile.get();
   }

   /**
    * @return The RPF replace / update table.  The rspfRefPtr can have a null
    * internal pointer if record was not found. Callers should check
    * rspfRefPtr<rspfRpfReplaceUpdateTable>::valid() before using pointer.
    */
   rspfRefPtr<rspfRpfReplaceUpdateTable> getRpfReplaceUpdateTable() const;

private:
   void clearFields();
   void deleteAll();
   rspfErrorCode populateCoverageSection(std::istream& in);
   rspfErrorCode populateCompressionSection(std::istream& in);
   rspfErrorCode populateImageSection(std::istream& in);
   rspfErrorCode populateAttributeSection(std::istream& in);
   rspfErrorCode populateColorGrayscaleSection(std::istream& in);
   rspfErrorCode populateMasks(std::istream& in);
   rspfErrorCode populateReplaceUpdateTable(std::istream& in);

   /*!
    * The header will be instantiated during the opening of the
    * frame file.
    */
   rspfRpfHeader *theHeader;

   /*!
    * The filename is set if its a successful open.
    */
   rspfFilename   theFilename;

   /*!
    * This will hold the coverage.
    */
   rspfRpfCoverageSection* theCoverage;

   /*!
    * This will hold the attributes of this frame.
    */
   rspfRpfAttributes *theAttributes;

   /*!
    * This is the subheader for the image.  It will have
    * some general information about the image.
    */
   rspfRpfImageDescriptionSubheader* theImageDescriptionSubheader;

   /*!
    * This is the mask subheader.
    */
   rspfRpfMaskSubheader* theMaskSubheader;
   
   /*!
    * This is the subheader for the display parameters
    * for this frame.
    */
   rspfRpfImageDisplayParameterSubheader* theImageDisplayParameterSubheader;

   /*!
    * Will hold a pointer to the compression information section.
    */
   rspfRpfCompressionSection*             theCompressionSection;
   
   /*!
    * If present, it will hold the color grayscale subheader.
    */
   rspfRpfColorGrayscaleSubheader*        theColorGrayscaleSubheader;

   /*!
    * 
    */
   rspfRpfColorConverterSubsection*       theColorConverterSubsection;

   /*!
    * 
    */
   vector<rspfRpfColorGrayscaleTable>     theColorGrayscaleTable;

   /*!
    * 
    */
   rspfRefPtr<rspfNitfFile>              theNitfFile;

   /*!
    * We have a 3-D array.  For the most part the numberof spectral groups
    * should be one.  The indexes are as follows:
    *
    * theSubframeMasKTable[spectralGroup][row][col]
    *
    *  It will hold RPF_NULL(0xffffffff) if the subframe does not exist
    *  and it will hold an offset value from the start of the
    * [spatial data section] to the first byte of the subframe table.
    */
   vector< vector< vector< rspf_uint32> > > theSubframeMaskTable;

   /*!
    * We have a 3-D array.  For the most part the numberof spectral groups
    * should be one.  The indexes are as follows:
    *
    * theSubframeTransparencyMaskTable[spectralGroup][row][col]
    *
    *  It will hold RPF_NULL(0xffffffff) if the subframe does not exist
    *  and it will hold an offset value from the start of the
    * [spatial data section] to the first byte of the subframe table.
    */
   vector< vector< vector< rspf_uint32> > > theSubframeTransparencyMaskTable;

   /** Holds table of "replace / update" records if present. */
   rspfRefPtr<rspfRpfReplaceUpdateTable> theReplaceUpdateTable;

};

#endif
