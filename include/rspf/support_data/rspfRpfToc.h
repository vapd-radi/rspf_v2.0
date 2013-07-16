//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
// 
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfRpfToc.h 18044 2010-09-06 14:20:52Z dburken $
#ifndef osimRpfToc_HEADER
#define osimRpfToc_HEADER

#include <vector>
#include <iosfwd>

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfErrorCodes.h>
#include <rspf/base/rspfException.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfReferenced.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/support_data/rspfNitfFileHeader.h>
#include <rspf/support_data/rspfRpfHeader.h>

class rspfRpfFrameFileIndexSubsection;
class rspfRpfTocEntry;
class rspfRpfFrameEntry;

class RSPF_DLL rspfRpfToc : public rspfReferenced
{
public:
   friend RSPF_DLL std::ostream& operator <<(std::ostream& out,
                                              const rspfRpfToc& data);
   rspfRpfToc();

   virtual ~rspfRpfToc();

   /**
    * @brief Parses a.toc file.
    * @param fileName File to parse.
    * @param keepFileHeader If true the rspfNitfFileHeader will be kept.
    * @return rspfErrorCodes::RSPF_OK on success, rspfErrorCodes::RSPF_ERROR on error.
    */
   rspfErrorCode parseFile(const rspfFilename &fileName,
                            bool keepFileHeader = false);

   /**
    * @brief Creates a new a.toc file from list of rpf frames in output directory.
    *
    * @param dotRpfFile File containing list of rpf frames wanted.
    *
    * @param outputDir Directory to write a.toc file and rpf frames to.
    *
    * @see rspf-rpf application usage for dotRpfFile format.
    *
    * @note Throws rspfException on error.
    */
   void createTocAndCopyFrames( const rspfFilename& dotRpfFile,
                                const rspfFilename& outputDir);

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
                       const std::string& prefix=std::string(),
                       bool printOverviews=false) const;
   
   rspf_uint32 getNumberOfEntries()const;
   
   const rspfRpfTocEntry* getTocEntry(rspf_uint32 index)const;

   /**
    * Returns -1 if not found.
    */
   rspf_int32 getTocEntryIndex(const rspfRpfTocEntry* entry);

   /**
    * @brief Get the nitf file header.
    *
    * @return The nitf file header or 0 if not set.
    */
   const rspfNitfFileHeader* getNitfFileHeader()const;
   
   const rspfRpfHeader* getRpfHeader() const;

   /**
    * For the given entry index, this routine returns the number of 
    * frames that exist in the horizontal direction. If the entry index
    * is invalid, 0 frames are returned.
    *
    * @param idx the entry index.
    * @return number of frames in the horizontal direction
    */
   rspf_uint32 getNumberOfFramesHorizontal(rspf_uint32 idx)const;

   /**
    * For the given entry index, this routine returns the number of 
    * frames that exist in the vertical direction. If the entry index
    * is invalid, 0 frames are returned.
    *
    * @param idx the entry index.
    * @return number of frames in the vertical direction
    */
   rspf_uint32 getNumberOfFramesVertical(rspf_uint32 idx)const;

   /**
    * For the given entry index, frame row, and frame column, this 
    * routine returns the corresponding rspfRpfFrameEntry instance. 
    *
    * @param entryIdx the entry index.
    * @param row the frame row.
    * @param col the frame col.
    * @return true if successful
    */
   bool getRpfFrameEntry(rspf_uint32 entryIdx, 
                         rspf_uint32 row,
                         rspf_uint32 col,
                         rspfRpfFrameEntry& result)const;

   /**
    * For the given entry index, frame row, and frame column, this 
    * routine returns the corresponding name of the frame image
    * with respect to the location of the toc file. 
    *
    * @param entryIdx the entry index.
    * @param row the frame row.
    * @param col the frame col.
    * @return the name of the frame image
    */
   const rspfString getRelativeFramePath( rspf_uint32 entryIdx,
                                           rspf_uint32 row,
                                           rspf_uint32 col)const;

   /**
    * @brief Method to get the root directory from the a.toc file name.
    * @param dir This initializes dir.
    */
   void getRootDirectory(rspfFilename& dir) const;

private:
   void deleteAll();
   void clearAll();
   void deleteTocEntryList();
   void buildTocEntryList(rspfRpfHeader* rpfHeader);
   void allocateTocEntryList(rspf_uint32 numberOfEntries);

   /** @brief Walks through frames to find the first entry that exists... */
   void getFirstEntry(const rspfRpfTocEntry* rpfTocEntry,
                      rspfRpfFrameEntry& frameEntry) const;

   /**
    * @brief Writes the frame file sub section of an a.toc.
    * 
    * This is a utility method for createTocAndCopyFrames method. 
    */
   void writeFrameFileIndexSection(rspfRpfFrameFileIndexSubsection* frameFileSubSection,
                                   std::ifstream& dotRpfStr,
                                   std::ofstream& dotTocStr);

   /**
    * @brief Copies frame to output directory.
    * 
    * This is a utility method for createTocAndCopyFrames method. 
    */
   void copyFrames(std::ifstream& dotRpfStr, const rspfFilename& outputDir);

   

   /**
    * @brief Get the number of frames from the dot rpf file.
    * This is a utility method for createTocAndCopyFrames method.
    */
   rspf_uint32 getNumberOfFrames(std::ifstream& dotRpfStr) const;

   /**
    * @brief Get a.toc file from the dot rpf file.
    * This is a utility method for createTocAndCopyFrames method.
    */
   rspfFilename getSourceTocFile(std::ifstream& dotRpfStr) const;

   /**
    * @brief Gets the zero based entry number for the first file in the dot rpf file.
    * This is a utility method for createTocAndCopyFrames method.
    */
   bool getCorespondingEntry(rspfRpfFrameFileIndexSubsection* frameFileSubSection,
                             std::ifstream& dotRpfStr,
                             rspf_uint32& entry) const;
   
   /**
    * @brief Gets the filename from line in.
    *
    * Line format:
    *
    * /data/spadac/rpf-frame-test/t1/N03E030/005FNU1B.I21|30.9141787715578,3.03831480449669|30.9889225265623,3.10741371349693
    *
    * @param line Line to extract filename from.
    *
    * @param out Filename to initialize.
    *
    * This is a utility method for createTocAndCopyFrames method.
    */
   bool getFile(const std::string& line, rspfFilename& file) const;

   /*!
    * This will hold a list of table of content entries.  There is one entry
    * per directory.  Each entry will have its geographic coverage.
    * Each directory is then divided into frames.  There could be 30 or
    * more frame images that make up an entire image.
    */
   std::vector<rspfRpfTocEntry*> m_tocEntryList;
   
   /*!
    * We will remember the file that we opened
    */
   rspfFilename m_filename;

   rspfRefPtr<const rspfNitfFileHeader> m_nitfFileHeader;
   
   rspfRefPtr<rspfRpfHeader> m_rpfHeader;
};

#endif
