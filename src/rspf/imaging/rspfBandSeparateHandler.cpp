//*************************************************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Oscar Kramer
//
// Description:
// 
// Image handler used when the multi-band image has each band represented by a different disk file.
// This is the case for some Ikonos imagery. 
//
//*************************************************************************************************
//  $Id: rspfBandSeparateHandler.cpp 2644 2011-05-26 15:20:11Z oscar.kramer $

#include <rspf/imaging/rspfBandSeparateHandler.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfStdOutProgress.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>
#include <algorithm>

RTTI_DEF1(rspfBandSeparateHandler, "rspfBandSeparateHandler", rspfImageHandler)

//*************************************************************************************************
//!  Constructor (default):
//*************************************************************************************************
rspfBandSeparateHandler::rspfBandSeparateHandler()
{ }

//*************************************************************************************************
//! Destructor:
//*************************************************************************************************
rspfBandSeparateHandler::~rspfBandSeparateHandler()
{
   close();
}

//*************************************************************************************************
// Opens multiple band-files based on info in metadata file.
//*************************************************************************************************
bool rspfBandSeparateHandler::open()
{
   if (isOpen())
      close();

   // Fetch the list of band filenames:
   vector<rspfFilename> file_names;
   getBandFileNames(file_names);

   // There should be a list of filenames populated:
   if (file_names.size() == 0)
      return false;

   m_mergeSource = new rspfBandMergeSource;
   rspfRefPtr<rspfImageHandler> handler = 0;
   for (rspf_uint32 band=0; band<file_names.size(); ++band)
   {
      // Open input band file:
      handler = rspfImageHandlerRegistry::instance()->open(file_names[band], true, false);
      if (!handler.valid())
      {
         rspfNotify(rspfNotifyLevel_WARN)<<"rspfBandSeparateHandler::open() -- "
            "Expected to find band file at <"<<file_names[band]<<"> but could not open file."
            " Image not opened."<<endl;
         close();
         return false;
      }

      // Add this band file to the merge source:
      m_bandFiles.push_back(handler);
      m_mergeSource->connectMyInputTo(band, handler.get());
   }

   completeOpen();
   return true;
}

//*************************************************************************************************
//! Deletes the overview and clears the valid image vertices.  Derived
//! classes should implement.
//*************************************************************************************************
void rspfBandSeparateHandler::close()
{
   vector<rspfRefPtr<rspfImageHandler> >::iterator iter = m_bandFiles.begin();
   while (iter != m_bandFiles.end())
   {
      (*iter)->close();
      *iter = 0;
      ++iter;
   }
   m_bandFiles.clear();
   m_mergeSource = 0;
}

//*************************************************************************************************
//! Derived classes must implement this method to be concrete.
//! @return true if open, false if not.
//*************************************************************************************************
bool rspfBandSeparateHandler::isOpen()const
{
   if (m_bandFiles.size() > 0)
      return true;

   return false;
}

//*************************************************************************************************
//! Fills the requested tile by pulling pixels from multiple file tiles as needed.
//*************************************************************************************************
rspfRefPtr<rspfImageData> rspfBandSeparateHandler::getTile(const rspfIrect& tile_rect, 
                                                              rspf_uint32 resLevel)
{
   // First verify that there are band-files available:
   if ((m_bandFiles.size() == 0) || !m_mergeSource.valid())
      return rspfRefPtr<rspfImageData>();

   // Check if res level represents data in the overview:
   if (theOverview.valid() && (resLevel > 0))
      return theOverview->getTile(tile_rect, resLevel);

   // Just pass getTile call on to contained bandMergeSource:
   return m_mergeSource->getTile(tile_rect, resLevel);
}

//*************************************************************************************************
//! @param resLevel Reduced resolution level to return lines of.
//! Default = 0
//! @return The number of lines for specified reduced resolution level.
//*************************************************************************************************
rspf_uint32 rspfBandSeparateHandler::getNumberOfLines(rspf_uint32 resLevel) const
{
   if ((m_bandFiles.size() == 0))
      return 0;

   // Using simple decimation by powers of 2:
   rspf_uint32 numlines = m_bandFiles[0]->getNumberOfLines() >> resLevel;
   return numlines;
}

//*************************************************************************************************
//! @param resLevel Reduced resolution level to return samples of.
//! Default = 0
//! @return The number of samples for specified reduced resolution level.
//*************************************************************************************************
rspf_uint32 rspfBandSeparateHandler::getNumberOfSamples(rspf_uint32 resLevel) const
{
   if ((m_bandFiles.size() == 0))
      return 0;

   // Using simple decimation by powers of 2:
   rspf_uint32 numsamps = m_bandFiles[0]->getNumberOfSamples() >> resLevel;
   return numsamps;
}

//*************************************************************************************************
//! Method to save the state of an object to a keyword list.
//! Return true if ok or false on error.
//*************************************************************************************************
bool rspfBandSeparateHandler::saveState(rspfKeywordlist& kwl, const char* prefix) const
{
   return rspfImageHandler::saveState(kwl, prefix);
}

//*************************************************************************************************
//! Method to the load (recreate) the state of an object from a keyword
//! list.  Return true if ok or false on error.
//*************************************************************************************************
bool rspfBandSeparateHandler::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   if (!rspfImageHandler::loadState(kwl, prefix))
      return false;

   return open();
}

//*************************************************************************************************
// Returns the number of bands of the first tile since all tiles need to have the same pixel type.
//*************************************************************************************************
rspf_uint32 rspfBandSeparateHandler::getNumberOfInputBands() const
{
   return (rspf_uint32) m_bandFiles.size();
}

//*************************************************************************************************
//! Returns scalar type of first tile (should be the same for all tiles)
//*************************************************************************************************
rspfScalarType rspfBandSeparateHandler::getOutputScalarType() const
{
   if ((m_bandFiles.size() == 0) || (!m_bandFiles[0].valid()))
      return RSPF_SCALAR_UNKNOWN;

   return m_bandFiles[0]->getOutputScalarType();
}

//*************************************************************************************************
//! Overrides base connection method to connect output of merge source.
// Returns TRUE if connection possible
//*************************************************************************************************
bool rspfBandSeparateHandler::canConnectMyOutputTo(rspf_int32 index, 
                                                    const rspfConnectableObject* obj)
{
   if (!m_mergeSource.valid())
      return false;

   return m_mergeSource->canConnectMyOutputTo(index, obj);
}

//*************************************************************************************************
//! Fetches the list of band file names from the header file. The header file name is contained
//! in rspfImageHandler::theImageFile.
//! NOTE TO PROGRAMMER: This class is presently tailored to the Ikonos metadata file specifying
//! the individual band files. If other "metadata" formats are to be supported, then this class
//! should be derived from. This method would therefore be pure virtual in this base-class and only 
//! this method would need to be implemented in the derived classes.
//*************************************************************************************************
void rspfBandSeparateHandler::getBandFileNames(vector<rspfFilename>& file_names)
{
   file_names.clear();

   // Ikonos expects metadata file with text extension:
   if (theImageFile.ext() != "txt")
      return;

   // Attempt to open the metadata file:
   ifstream is (theImageFile.chars());
   if (is.fail())
   {
      rspfNotify(rspfNotifyLevel_WARN)<<"rspfBandSeparateHandler::open() -- "
         "Cannot open text file at <"<<theImageFile<<">. Image not opened."<<endl;
      return;
   }

   vector<rspfString> fname_list;
   const rspfString separator (" ");
   char line_buf[4096];
   while (!is.eof())
   {
      is.getline(line_buf, 4096);
      rspfString line_str (line_buf);

      // Look for mention of multiple band files. It must indicate "separate files". This comes 
      // before the list of band files:
      if (line_str.contains("Multispectral Files:"))
      {
         if (!line_str.contains("Separate Files"))
            return;
      }

      // Look for list of band files:
      if (line_str.contains("Tile File Name:"))
      {
         rspfString file_list_str = line_str.after(":");
         file_list_str.split(fname_list, separator, true);
         break;
      }
   }
   is.close();

   // Set the path of individual band files to match metadata file path:
   rspfFilename pathName (theImageFile.path());
   for (size_t i=0; i<fname_list.size(); ++i)
   {
      rspfFilename fname (fname_list[i]);
      fname.setPath(pathName);
      file_names.push_back(fname);
   }
}

//*************************************************************************************************
//! Returns the geometry of the first band.
//*************************************************************************************************
rspfRefPtr<rspfImageGeometry> rspfBandSeparateHandler::getImageGeometry()
{
   if ((m_bandFiles.size() == 0) || (!m_bandFiles[0].valid()))
      return 0;

   return m_bandFiles[0]->getImageGeometry();
}
