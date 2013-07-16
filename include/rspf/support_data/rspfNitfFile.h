//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfFile.h 19583 2011-05-13 10:58:10Z gpotts $
#ifndef rspfNitfFile_HEADER
#define rspfNitfFile_HEADER

#include <iosfwd>
#include <vector>

#include <rspf/base/rspfReferenced.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/support_data/rspfNitfFileHeader.h>

class rspfNitfImageHeader;
class rspfNitfSymbolHeader;
class rspfNitfLabelHeader;
class rspfNitfTextHeader;
class rspfNitfDataExtensionSegment;
class rspfFilename;
class rspfNitfRegisteredTag;
class rspfNitfTagInformation;

class RSPFDLLEXPORT rspfNitfFile : public rspfReferenced
{
public:
   friend RSPFDLLEXPORT std::ostream& operator <<(std::ostream& out,
                                                   const rspfNitfFile &data);

   /**
    * @brief print method that outputs a key/value type format adding prefix
    * to keys.
    * @param out Stream to output to.
    * @param prefix This will be prepended to key.
    * e.g. Where prefix = "nitf." and key is "file_name" key becomes:
    * "nitf.file_name:"
    * @param printOverviews If true overview, if present(e.g. rpf's) will be
    * printed.
    * @return output stream.
    */
   std::ostream& print(std::ostream& out,
                       const std::string& prefix=std::string(),
                       bool printOverviews=false) const;
   
   rspfNitfFile();
   virtual ~rspfNitfFile();

   /*!
    *  Opens the nitf file and attempts to parse.
    *  Returns true on success, false on error.
    */
   bool parseFile(const rspfFilename &file);

   /*!
    * Will return the header.
    */
   const rspfNitfFileHeader* getHeader()const;
   rspfNitfFileHeader* getHeader();
   rspfIrect getImageRect()const;
   
   rspfNitfImageHeader*  getNewImageHeader(rspf_uint32 imageNumber)const;
   rspfNitfSymbolHeader* getNewSymbolHeader(rspf_uint32 symbolNumber)const;
   rspfNitfLabelHeader*  getNewLabelHeader(rspf_uint32 labelNumber)const;
   rspfNitfTextHeader*   getNewTextHeader(rspf_uint32 textNumber)const;
   rspfNitfDataExtensionSegment* getNewDataExtensionSegment(rspf_uint32 dataExtNumber)const;
   
   rspfString getVersion()const;

   /** @return The filename parsed by this object. */
   rspfFilename getFilename() const;
   
   virtual bool saveState(rspfKeywordlist& kwl, const rspfString& prefix)const;

protected:
   rspfNitfImageHeader* allocateImageHeader()const;
   
   rspfFilename                    theFilename;
   rspfRefPtr<rspfNitfFileHeader> theNitfFileHeader;
};

#endif
