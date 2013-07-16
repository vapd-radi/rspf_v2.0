//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Mingjie Su
//
// Description:
//
// Contains class declaration for NitfTileSource_12.
//
//*******************************************************************
//  $Id: rspfNitfTileSource_12.h 958 2010-06-03 23:00:32Z ming.su $
#ifndef rspfNitfTileSource_12_HEADER
#define rspfNitfTileSource_12_HEADER

#include <rspf/base/rspfConstants.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/support_data/rspfNitfImageHeader.h>

class RSPF_DLL rspfNitfTileSource_12 
{
public:
  static bool uncompressJpeg12Block(rspf_uint32 x, 
                                    rspf_uint32 y,
                                    rspfRefPtr<rspfImageData> cacheTile,
                                    rspfNitfImageHeader* hdr,
                                    rspfIpt cacheSize,
                                    std::vector<rspf_uint8> compressedBuf,
                                    rspf_uint32 readBlockSizeInBytes,
                                    rspf_uint32 bands);


  static bool loadJpegQuantizationTables(rspfNitfImageHeader* hdr,
                                         jpeg_decompress_struct& cinfo);

  static bool loadJpegHuffmanTables(jpeg_decompress_struct& cinfo);
};
   
#endif /* #ifndef rspfNitfTileSource_12_HEADER */
