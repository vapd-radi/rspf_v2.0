//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:  rspfRpfUtil.cpp
// 
// Utility class to stuff with rpf files.
// 
//----------------------------------------------------------------------------
// $Id$

#include <rspf/util/rspfRpfUtil.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/support_data/rspfRpfToc.h>
#include <rspf/support_data/rspfRpfTocEntry.h>
#include <ctime>
#include <iomanip>

static rspfTrace traceDebug = rspfTrace("rspfRpfUtil:debug");

rspfRpfUtil::rspfRpfUtil()
{
}

rspfRpfUtil::~rspfRpfUtil()
{
}


// Note: throws rspfException on error.
void rspfRpfUtil::writeDotRpfFiles( const rspfFilename& aDotTocFile,
                                     const rspfFilename& outputDir )
{
   static const char MODULE[] = "rspfRpfUtil::writeDotRpfFiles";

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " entered..."
         << "\na.toc file:        " << aDotTocFile
         << "\noutput directory:  " << outputDir
         << "\n";
   }
   
   // Parse the a.toc file:
   rspfRefPtr<rspfRpfToc> toc = new rspfRpfToc();
   
   if ( toc->parseFile(aDotTocFile) != rspfErrorCodes::RSPF_OK )
   {
      std::string e = MODULE;
      e += " ERROR:\nCould not open: ";
      e+= aDotTocFile.string();
      throw rspfException(e);
   }

   if ( outputDir.expand().exists() == false )
   {
      if ( !outputDir.createDirectory(true, 0775) )
      {
         std::string e = MODULE;
         e += " ERROR:\nCould not create directory: ";
         e+= outputDir.c_str();
         throw rspfException(e);
      }
   }

   //---
   // Go through the entries...
   //---
   rspf_uint32 entries = toc->getNumberOfEntries();
   for (rspf_uint32 entry = 0; entry < entries; ++entry)
   {
      const rspfRpfTocEntry* tocEntry = toc->getTocEntry(entry);
      if (tocEntry)
      {
         if ( tocEntry->isEmpty() == false )
         {
            writeDotRpfFile(toc.get(), tocEntry, outputDir, entry);
         }
      }
      else
      {
         std::string e = MODULE;
         e += " ERROR:  Null entry: ";
         e += rspfString::toString(entry).string();
         throw rspfException(e);
      }
   }
   
} // End: rspfRpfUtil::writeDotRpfFiles

//---
// Writer a dot rpf file for entry to output directory.
// 
// NOTES:
//
// 1) All coordinate written out in AREA or edge to edge format.
// 2) Throws rspfException on error.
//---
void rspfRpfUtil::writeDotRpfFile( const rspfRpfToc* toc,
                                    const rspfRpfTocEntry* tocEntry,
                                    const rspfFilename& outputDir,
                                    rspf_uint32 entry)
{
   static const char MODULE[] = "rspfRpfUtil::writeDotRpfFile";

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " entered..."
         << "\noutput directory:  " << outputDir
         << "\nentry: " << entry << "\n";
   }

   if ( !toc )
   {
      std::string errMsg = MODULE;
      errMsg += " ERROR toc pointer null!";
      throw rspfException(errMsg);
   }
   if ( !tocEntry )
   {
      std::string errMsg = MODULE;
      errMsg += " ERROR toc entry pointer null!";
      throw rspfException(errMsg);
   }

   // Get the file name.
   rspfFilename outFile;
   if ( outputDir.expand().isDir() )
   {
      getDotRfpFilenameForEntry(outputDir, entry, outFile);
   }
   else
   {
      outFile = outputDir;
   }
   
   // Open the file to write.
   std::ofstream os;
   os.open(outFile.c_str(), ios::out);
   if ( os.good() == false )
   {
      std::string errMsg = MODULE;
      errMsg += "ERROR could not open: ";
      errMsg += outFile.string();
      throw rspfException(errMsg);
   }
   
   // Set up the output stream fix with full precision for ground points.
   os << setiosflags(std::ios_base::fixed) << setprecision(15);
   
   //---
   // Overall TOC entry bounds:
   // 
   // Write the first line which is the bounding box of the entry in the form of:
   // "89.9850464205332, 23.9892538162654|90.5085823882692, 24.5002602501599|1"
   //      lr-lon            lr-lat           ul-lon            ul-lat
   //---
   rspfRefPtr<rspfImageGeometry> geom = tocEntry->getImageGeometry();
   if( geom.valid() == false)
   {
      std::string errMsg = "ERROR could not get geometry.";
      errMsg += outFile.string();
      throw rspfException(errMsg);  
   }

   // Rectangle in image space.
   rspfIrect outputRect;
   tocEntry->getBoundingRect(outputRect);

   // bands:
   rspf_uint32 bands = tocEntry->getNumberOfBands();

   // scale:
   rspfDpt scale;
   tocEntry->getDecimalDegreesPerPixel(scale);
   rspfDpt halfPix = scale / 2.0;

   rspfGpt llg;
   rspfGpt urg;
   geom->localToWorld(outputRect.ur(), urg);
   geom->localToWorld(outputRect.ll(), llg);

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "outputRect: " << outputRect
         << "\nbands: " << bands
         << "\nscale: " << scale
         << "\nllg:   " << llg
         << "\nurg:   " << urg
         << std::endl;
   }

   // Expand coordinates to edge:
   llg.lon -= halfPix.x;
   llg.lat -= halfPix.y;
   urg.lon += halfPix.x;
   urg.lat += halfPix.y;
   
   // Test for 360 degrees apart.
   checkLongitude(llg, urg);
   
   os << llg.lon << "," // lower left longitude
      << llg.lat << "|" // lower left latitude
      << urg.lon << "," // upper right longitude
      << urg.lat << "|" // upper right latitude
      << bands << "\n";

   // Frame loop:
   const rspf_int32 FRAMESIZE = 1536;
   const rspf_int32 ROWS = static_cast<rspf_int32>(tocEntry->getNumberOfFramesVertical());
   if( ROWS == 0 )
   {
      std::string errMsg = MODULE;
      errMsg += " ERROR no rows!";
      throw rspfException(errMsg);  
   }
   const rspf_int32 COLS = static_cast<rspf_int32>(tocEntry->getNumberOfFramesHorizontal());
   if( COLS == 0 )
   {
      std::string errMsg = MODULE;
      errMsg += " ERROR no columns!";
      throw rspfException(errMsg);  
   }

   // Set the initial lower left and upper right image points for localToWorld call.
   //rspfDpt urd( ( (ROWS-1)*FRAMESIZE) -1, 0.0);
   //rspfDpt lld(0.0, (ROWS*FRAMESIZE)-1);
   rspfDpt urd( FRAMESIZE-1, 0.0);
   rspfDpt lld(0.0, FRAMESIZE-1);
   
   for (rspf_int32 row = ROWS-1; row > -1; --row)
   {
      for (rspf_int32 col = 0; col < COLS; ++col)
      {
         //---
         // Example format (only with 15 digit precision):
         // /data/spadac/rpf/world/cb01/ng467a1/0xslpk1a.i41|90.0448,24.3621|90.0598,24.3750
         //---
         
         // Get the path to the frame.
         rspfFilename path;
         toc->getRootDirectory(path);
         
         path = path.dirCat( toc->getRelativeFramePath(entry, row, col) );

         // Not sure if this is backwards:
         geom->localToWorld(urd, urg);
         geom->localToWorld(lld, llg);

         // Expand coordinates to edge:
         llg.lon -= halfPix.x;
         llg.lat -= halfPix.y;
         urg.lon += halfPix.x;
         urg.lat += halfPix.y;
         
         // Test for 360 degrees apart.
         checkLongitude(llg, urg);

         os << path.c_str() << "|"
            << llg.lon << "," // lower left longitude
            << llg.lat << "|" // lower left latitude
            << urg.lon << "," // upper right longitude
            << urg.lat        // upper right latitude
            << "\n";

         if ( traceDebug() )
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "row[" << row << "]col[" << col << "]path: " << path
               << "\nlld: " << lld
               << "\nllg: " << llg
               << "\nurd: " << urd
               << "\nurg: " << urg
               << std::endl;
         }

         // Go to next col.
         urd.x += FRAMESIZE;
         lld.x += FRAMESIZE;
            
      } // End column loop.

      // Go to nex row.
      urd.y += FRAMESIZE;
      urd.x = FRAMESIZE-1;
      lld.y += FRAMESIZE;
      lld.x = 0;
      
   } // End row loop.

   // Close the file.
   os.close();

   rspfNotify(rspfNotifyLevel_DEBUG) << "wrote file: " << outFile << std::endl;
   
} // End: rspfRpfUtil::writeDotRpfFile

void rspfRpfUtil::checkLongitude(rspfGpt& left, rspfGpt& right) const
{
   //---
   // Test for scene coordinates being 180 to 180 (360 degree spread) and
   // adjust leftLon to -180 if so.
   //
   // NOTE:
   // Setting tolerance to 1/7200 about 15 meters.
   // Not sure if this is too loose or not. (drb)
   //---
   const rspf_float64 TOLERANCE = 0.000138889; // 1/7200 about 15 meters.

   if ( rspf::almostEqual(left.lon, 180.0, TOLERANCE) )
   {
      if ( rspf::almostEqual(right.lon, 180.0, TOLERANCE) )
      {
         left.lon = -180.0;
         right.lon = 180.0;
      }
   }
}

void rspfRpfUtil::getDotRfpFilenameForEntry(const rspfFilename& outputDir,
                                             rspf_uint32 entry,
                                             rspfFilename& outFile) const
{
   // Get the build date in the format of (yyyymmddhhmmss).
   char s[15];
   s[14] = '\0';
   time_t t;
   time(&t);
   tm* lt = localtime(&t);
   strftime(s, 15, "%Y%m%d%H%M%S", lt);
   std::string date = s;
   
   outFile = outputDir.dirCat(s);
   outFile += "_e";
   outFile += rspfString::toString(entry);
   outFile += ".rpf";
}
