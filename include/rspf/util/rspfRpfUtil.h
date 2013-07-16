//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:  rspfRpfUtil.h
// 
// Utility class to stuff with rpf files.
// 
//----------------------------------------------------------------------------
// $Id$

#ifndef rspfRpfUtil_HEADER
#define rspfRpfUtil_HEADER 1

#include <rspf/base/rspfReferenced.h>
#include <rspf/imaging/rspfImageGeometry.h> 

class rspfFilename;
class rspfGpt;
class rspfRpfToc;
class rspfRpfTocEntry;

class RSPF_DLL rspfRpfUtil : public rspfReferenced
{
public:

   /** @brief default constructor */
   rspfRpfUtil();


   /**
    * @brief Write dot rpf file(s) to output directory from a.toc file.
    *
    * This creates a dot rpf file for each entry.
    *
    * @param aDotFile The a.toc file.
    *
    * @param outputDir Directory to write dot rpf file(s) to.
    *
    * throws rspfException on error.
    */
   void writeDotRpfFiles( const rspfFilename& aDotTocFile,
                          const rspfFilename& outputDir);

   void writeDotRpfFile( const rspfRpfToc* toc,
                         const rspfRpfTocEntry* tocEntry,
                         const rspfFilename& outputDir,
                         rspf_uint32 entry);

protected:

   /**
    * @brief protected virtual destructor
    *
    * Do not call directly.  Use as a ref ptr.
    */
   virtual ~rspfRpfUtil();

private:

   /**
    * @brief Method to test for 360 spread, 180.0 <--> 180.00 and set
    * leftLon to -180 if both left and right are 180.0 degrees.
    *
    * This adds a small tolerance to the test.
    * 
    * @param left Left hand side of scene.  This is the value
    * that will be flipped to -180 if test is true.
    *
    * @param right Right hand side of scene.
    */
   void checkLongitude(rspfGpt& left, rspfGpt& right) const;

   /** @brief Method to get the file for entry. */
   void getDotRfpFilenameForEntry(const rspfFilename& outputDir,
                                  rspf_uint32 entry,
                                  rspfFilename& outFile) const;
   
}; // Matches: class rspfRpfUtil

#endif /* #ifndef rspfRpfUtil_HEADER */
