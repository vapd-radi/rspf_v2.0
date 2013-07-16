//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfArcInfoGridWriter.h 16081 2009-12-10 20:56:36Z eshirschorn $
#ifndef rspfArcInfoGridWriter_HEADER
#define rspfArcInfoGridWriter_HEADER

#include <rspf/imaging/rspfImageFileWriter.h>
#include <rspf/base/rspfKeywordlist.h>

//*******************************************************************
// CLASS:  rspfArdcInfoGridWriterFactory
//*******************************************************************

class RSPF_DLL rspfArcInfoGridWriter : public rspfImageFileWriter
{
public:
   rspfArcInfoGridWriter();
   virtual              ~rspfArcInfoGridWriter();
   
   rspfArcInfoGridWriter(rspfImageSource *inputSource,
                          const rspfFilename& filename);
   virtual bool isOpen()const;
   
   virtual bool open();
   virtual void close();

   /*!
    * saves the state of the object.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   
   /*!
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

   /**
    * Returns a 3-letter extension from the image type descriptor 
    * (theOutputImageType) that can be used for image file extensions.
    *
    * @param imageType string representing image type.
    *
    * @return the 3-letter string extension.
    */
   virtual rspfString getExtension() const;

private:
   virtual bool writeFile();

   virtual bool writeBinaryAigHeader();
   virtual bool writeBinaryAigBounds();
   virtual bool writeBinaryAigStats();
   
   rspfIpt theOutputTileSize;
   
   
TYPE_DATA
};

#endif
