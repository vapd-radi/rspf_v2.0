//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  David Burken
//
// Description:
//
// Contains class definition for ImageTypeLUT.  Currently has mapping of
// ouput image writes from enumeration to string and string to
// enumeration.
//
//*******************************************************************
//  $Id: rspfImageTypeLut.cpp 22221 2013-04-11 15:30:08Z dburken $

#include <rspf/base/rspfImageTypeLut.h>

//***
// Pixel type keyword to use for getState/saveState methods.
//***
const rspfKeyword rspfImageTypeLut::IMAGE_TYPE_KW("image_type",
                                                    "Image type.\n\
Valid types are:  ccf, tiff, tiled_tiff, jpeg, and general_raster.");

static const int TABLE_SIZE = 9;

//*******************************************************************
// Public Constructor:
//*******************************************************************
rspfImageTypeLut::rspfImageTypeLut()
   :
      rspfLookUpTable(TABLE_SIZE)
{
   //***
   // Complete initialization of data member "theImageTypeTable".
   // Note:  Output type enumerations defined in constants.h file.
   //***
   theTable[0].theKey   = RSPF_TIFF_STRIP;
   theTable[0].theValue = "tiff_strip";
   theTable[1].theKey   = RSPF_TIFF_STRIP_BAND_SEPARATE;
   theTable[1].theValue = "tiff_strip_band_separate";
   theTable[2].theKey   = RSPF_TIFF_TILED;
   theTable[2].theValue = "tiff_tiled";
   theTable[3].theKey   = RSPF_TIFF_TILED_BAND_SEPARATE;
   theTable[3].theValue = "tiff_tiled_band_separate";
   theTable[4].theKey   = RSPF_GENERAL_RASTER_BIP;
   theTable[4].theValue = "general_raster_bip";
   theTable[5].theKey   = RSPF_GENERAL_RASTER_BIL;
   theTable[5].theValue = "general_raster_bil";
   theTable[6].theKey   = RSPF_GENERAL_RASTER_BSQ;
   theTable[6].theValue = "general_raster_bsq";
   theTable[7].theKey   = RSPF_JPEG;
   theTable[7].theValue = "jpeg";
   theTable[8].theKey   = RSPF_PDF;
   theTable[8].theValue = "rspf_pdf";
}
