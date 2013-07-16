//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts (gpotts@imagelinks.com)
//
// Description:  Converts SpaceImaging geometry file to
//               RSPF Geometry file.
//
//*******************************************************************
//  $Id: rspfSpaceImagingGeom.h 10251 2007-01-14 17:30:14Z dburken $
#ifndef rspfSpaceImagingGeom_HEADER
#define rspfSpaceImagingGeom_HEADER

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfKeywordlist.h>

class RSPF_DLL rspfSpaceImagingGeom
{
public:
   rspfSpaceImagingGeom(const rspfFilename& file,
                         const char* prefix = 0);
   rspfSpaceImagingGeom();

   void setGeometry(const rspfFilename& file);
   void setGeometry(const rspfKeywordlist& kwl);

   void exportToOssim(rspfKeywordlist& kwl,
                      const char* prefix=0)const;

   rspfString spaceImagingToOssimKeyword(const rspfString& spaceImagingKeyword)const;
   rspfString spaceImagingToOssimValue(const rspfString& spaceImagingKeyword,
                                        const rspfString& spaceImagingValue)const;

   rspfString getBandName()const;
   rspfFilename getFilename()const;
   rspfString getProducer()const;
   
   static const char* SIG_PRODUCER_KW;
   static const char* SIG_PROJECT_NAME_KW;
   static const char* SIG_FILENAME_KW;
   static const char* SIG_BAND_KW;
   static const char* SIG_BITS_PER_PIXEL_KW;
   static const char* SIG_NUMBER_OF_BANDS_KW;
   static const char* SIG_DATUM_KW;
   static const char* SIG_PROJECTION_KW;
   static const char* SIG_SELECTED_PROJECTION_KW;
   static const char* SIG_ZONE_KW;
   static const char* SIG_UL_EASTING_KW;
   static const char* SIG_UL_NORTHING_KW;
   static const char* SIG_PIXEL_SIZE_X_KW;
   static const char* SIG_PIXEL_SIZE_Y_KW;
   static const char* SIG_COLUMNS_KW;
   static const char* SIG_ROWS_KW;
   
protected:
   rspfKeywordlist theSpaceImagingGeometry;
   rspfString      thePrefix;
};

#endif
