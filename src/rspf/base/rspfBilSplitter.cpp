//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  David Burken (dburken@imagelinks.com)
//
// Description:
//
// Contains class definition for rspfBilSplitter.
//
// Utility class for splitting an image that is band interleaved by line into
// separate files.
//*******************************************************************
//  $Id: rspfBilSplitter.cpp 9966 2006-11-29 02:01:07Z gpotts $

#include <iomanip>
#include <iostream>
using namespace std;

#include <rspf/base/rspfBilSplitter.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfNotifyContext.h>

rspfBilSplitter::rspfBilSplitter(const char* file_to_split,
                                   rspf_uint32      header_size_in_bytes,
                                   rspf_uint32      bytes_per_pixel,
                                   rspf_uint32      samples_per_line,
                                   rspf_uint32      number_of_channels)
   :
      theSourceFileStr(),
      theHeaderSizeInBytes(header_size_in_bytes),
      theBytesPerLine(bytes_per_pixel * samples_per_line),
      theLinesToWrite(0),
      theNumberOfChannels(number_of_channels),
      theErrorStatus(RSPF_OK)
{
   static const char MODULE[] = "rspfBilSplitter::rspfBilSplitter";
                                
   //***
   // Open up the source bil file and check for a good descriptor.
   //***
   theSourceFileStr.open(file_to_split, ios::in | ios::binary);
   if (!theSourceFileStr)
   {
      theErrorStatus = RSPF_ERROR;
      rspfNotify(rspfNotifyLevel_FATAL) << MODULE << ":\nCould not open file:  " << file_to_split
           << "\n";
      
      return;
   }

   //***
   // Get the file size to make sure that the ( size - header ) / byte per line
   // come out even; if not, consider this an error.
   //***
   rspfFilename f(file_to_split);
   rspf_uint32 size = f.fileSize();
   
   if ( (size-theHeaderSizeInBytes) % (theBytesPerLine * theNumberOfChannels) )
   {
      theErrorStatus = RSPF_ERROR;
      rspfNotify(rspfNotifyLevel_FATAL) << MODULE << ":"
                                          << "\nFile size error..."
                                          << "\nbytes_per_pixel * samples_per_line * "
                                          << "number_of_channels should"
                                          << "\ndivide evenly into total file size in bytes."
                                          << "\nHeader size in bytes:  " << header_size_in_bytes
                                          << "\nBytes per pixel:       " << bytes_per_pixel
                                          << "\nSamples per line:      " << samples_per_line
                                          << "\nBytes per line:        " << theBytesPerLine
                                          << "\nNumber of channels:    " << theNumberOfChannels
                                          << "\nFile size in bytes:    " << size
                                          << "\nthe file size in bytes.  Returning with error."
                                          << "\n";
      return;
   }

   theLinesToWrite = (size-theHeaderSizeInBytes) /
                     (theBytesPerLine * theNumberOfChannels);

   rspfNotify(rspfNotifyLevel_INFO) << "\nFile to split:               " << file_to_split
                                      << "\nHeader size in bytes:        " << header_size_in_bytes
                                      << "\nBytes per pixel:             " << bytes_per_pixel
                                      << "\nSamples per line:            " << samples_per_line
                                      << "\nBytes per line:              " << theBytesPerLine
                                      << "\nNumber of channels:          " << theNumberOfChannels
                                      << "\nLines to write per channel:  " << theLinesToWrite
                                      << "\n";
}

rspfBilSplitter::~rspfBilSplitter()
{
   theSourceFileStr.close();
}

bool rspfBilSplitter::output_multi_files(const char* output_dir)
{
   static const char MODULE[] = "rspfBilSplitter::output_multi_files";
   
   if (theErrorStatus)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << MODULE << " :"
           << "\nError status has been set.  Cannot split file." << "\n";
      return false;
   }
   
   // Open up all the FILE pointers needed.
   rspfFilename outDir = output_dir;
   rspfString base("channel_");
   rspfString ext(".ras");
   ofstream* os = new ofstream[theNumberOfChannels];

   rspf_uint32 i = 0;
   for(i = 0; i < theNumberOfChannels; i++)
   {
      rspfString file_basename = base + rspfString::toString(int(i+1)) + ext;
      rspfFilename tmpFile = outDir.dirCat(file_basename);
      
      os[i].open(tmpFile.c_str(), ios::out | ios::binary);

      if (!os)
      {
         rspfNotify(rspfNotifyLevel_FATAL) << MODULE << ":\n"
              << "Cannot open:  " << tmpFile.c_str()
              << "\nReturning from application." << "\n";
         return false;
      }
   }

   char* lineBuffer = new char[theBytesPerLine];

   // Move the source pointer past the header.
   theSourceFileStr.seekg(theHeaderSizeInBytes, ios::beg);
   
   // Write the lines out to the individual files.
   rspfNotify(rspfNotifyLevel_INFO) << "\nOutputting to separate files to:  " << output_dir << "\n";
   
   for (i=0; i<theLinesToWrite; ++i)
   {
      for(rspf_uint32 j=0; j < theNumberOfChannels; ++j)
      {
         // Read a line.
         theSourceFileStr.read(lineBuffer, theBytesPerLine);

         // Write the line to the channel file.
         os[j].write(lineBuffer, theBytesPerLine);

      } // End of loop through the channels.

      //***
      // Output some status info.
      //***
      rspf_float64 totalLines     = theLinesToWrite;
      rspf_float64 linesWritten   = i + 1;
      rspfNotify(rspfNotifyLevel_INFO) << setiosflags(ios::fixed)
                                         << setprecision(0) << setw(3)
                                         << linesWritten / totalLines * 100.0 << " % \r"
                                         << flush;

   } // End of loop through lines.

   rspfNotify(rspfNotifyLevel_INFO) << "100 %\nFinished...\n";

   // Cleanup.
   for(i = 0; i < theNumberOfChannels; i++) os[i].close();
   delete [] os;
   delete [] lineBuffer;   

   return true;
}
