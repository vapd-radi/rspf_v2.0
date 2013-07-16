//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
//
// Contains class declaration for rspfBilSplitter.
// 
// Utility class for splitting an image that is band interleaved by line into
// separate files.
//*******************************************************************
//  $Id: rspfBilSplitter.h 9968 2006-11-29 14:01:53Z gpotts $

#ifndef rspfBilSplitter_HEADER
#define rspfBilSplitter_HEADER

#include <fstream>
using namespace std;

#include <rspf/base/rspfConstants.h>

class RSPFDLLEXPORT rspfBilSplitter
{
public:
   rspfBilSplitter(const char* file_to_split,
                    rspf_uint32      header_size_in_bytes,
                    rspf_uint32      bytes_per_pixel,
                    rspf_uint32      samples_per_line,
                    rspf_uint32      number_of_channels);

   ~rspfBilSplitter();

   enum rspfStatus
   {
      RSPF_OK    = 0,
      RSPF_ERROR = 1
   };

   /*!
    *  Splits source image into separate bands and writes to output
    *  directory as channel_1.ras, channel_2.ras, ...
    *  Returns true on success, false on error.
    */
   bool output_multi_files(const char* output_dir);

   int errorStatus() const { return theErrorStatus; }

private:

   ifstream      theSourceFileStr;
   rspf_uint32  theHeaderSizeInBytes;
   rspf_uint32  theBytesPerLine;
   rspf_uint32  theLinesToWrite;
   rspf_uint32  theNumberOfChannels;
   rspfStatus   theErrorStatus;
};

#endif
