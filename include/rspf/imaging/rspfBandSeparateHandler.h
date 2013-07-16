//**************************************************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Oscar Kramer
// 
// Description:
//
// Image handler used when the multi-band image has each band represented by a different disk file.
// This is the case for some Ikonos imagery. 
//
//**************************************************************************************************
// $Id: rspfBandSeparateHandler.h 2644 2011-05-26 15:20:11Z oscar.kramer $
#ifndef rspfBandSeparateHandler_HEADER
#define rspfBandSeparateHandler_HEADER

#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/imaging/rspfBandMergeSource.h>

/**
 *  This class defines an abstract Handler which all image handlers(loaders)
 *  should derive from.
 */
class RSPFDLLEXPORT rspfBandSeparateHandler : public rspfImageHandler
{
public:
   //!  Constructor (default):
   rspfBandSeparateHandler();
   
   //! Destructor:
   virtual ~rspfBandSeparateHandler();
   
   //! Opens multiple band-files based on info in metadata file.
   //! @return Returns true on success, false on error.
   virtual bool open();

   //! Closes all band-files and clears input handler list.
   virtual void close();

   //! @return true if open, false if not.
   virtual bool isOpen()const;

   virtual rspf_uint32 getNumberOfInputBands() const;
   
   //! @param resLevel Reduced resolution level to return lines of.
   //! Default = 0
   //! @return The number of lines for specified reduced resolution level.
   virtual rspf_uint32 getNumberOfLines(rspf_uint32 resLevel = 0) const;

   //! @param resLevel Reduced resolution level to return samples of.
   //! Default = 0
   //! @return The number of samples for specified reduced resolution level.
   virtual rspf_uint32 getNumberOfSamples(rspf_uint32 resLevel = 0) const;
   
   //! Method to save the state of an object to a keyword list.
   //! Return true if ok or false on error.
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=0)const;

   //! Method to the load (recreate) the state of an object from a keyword
   //! list.  Return true if ok or false on error.
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);

   //! Fills the requested tile by pulling pixels from multiple file tiles as needed.
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& rect, rspf_uint32 resLevel);

   //! Returns scalar type of first band (should be the same for all bands)
   virtual rspfScalarType getOutputScalarType() const;

   //! Overrides base connection method to connect output of merge source.
   virtual bool canConnectMyOutputTo(rspf_int32 index, const rspfConnectableObject* obj);

   //! Returns the geometry of the first band:
   virtual rspfRefPtr<rspfImageGeometry> getImageGeometry();

   //! These images are not tiled
   virtual rspf_uint32 getImageTileWidth()  const { return 0; }
   virtual rspf_uint32 getImageTileHeight() const { return 0; }

protected:
   //! Fetches the list of band file names from the header file. The header file name is contained
   //! in rspfImageHandler::theImageFile.
   //! NOTE TO PROGRAMMER: This class is presently tailored to the Ikonos metadata file specifying
   //! the individual band files. If other "metadata" formats are to be supported, then this class
   //! should be derived from. This method would therefore be pure virtual in this base-class and 
   //! only this method would need to be implemented in the derived classes.
   virtual void getBandFileNames(vector<rspfFilename>& file_names);

  vector<rspfRefPtr<rspfImageHandler> > m_bandFiles;
  rspfRefPtr<rspfBandMergeSource>       m_mergeSource;

TYPE_DATA
};

#endif /* #ifndef rspfBandSeparateHandler_HEADER */
