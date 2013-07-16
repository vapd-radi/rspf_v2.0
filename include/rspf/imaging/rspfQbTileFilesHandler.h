//**************************************************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Oscar Kramer
// 
//**************************************************************************************************
// $Id: rspfQbTileFilesHandler.h 2669 2011-06-06 12:28:20Z oscar.kramer $
#ifndef rspfQbTileFilesHandler_HEADER
#define rspfQbTileFilesHandler_HEADER

#include <rspf/imaging/rspfTiledImageHandler.h>

// Enable this flag to utilize the system of separate overviews for each tile-file
#define USING_SUB_OVRS 0

//! Image handler used for tiled Quickbird imagery. 
class RSPFDLLEXPORT rspfQbTileFilesHandler : public rspfTiledImageHandler
{
public:
   //!  Constructor (default):
   rspfQbTileFilesHandler();
   
   //! Destructor:
   virtual ~rspfQbTileFilesHandler();
   
   //! @return Returns true on success, false on error.
   //! @note This method relies on the data member rspfImageData::theImageFile being set.  
   virtual bool open();

   virtual rspfRefPtr<rspfImageGeometry> getImageGeometry();
   
protected:

   //! Initializes tile image rects by considering adjacent row/col offsets. Called when TIL
   //! doesn't contain the info. Returns true if successful.
   bool computeImageRects();

TYPE_DATA
};

#endif /* #ifndef rspfQbTileFilesHandler_HEADER */
