//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfLandsatTopoCorrectionFilter.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfLandsatTopoCorrectionFilter_HEADER
#define rspfLandsatTopoCorrectionFilter_HEADER
#include <rspf/base/rspfFilename.h>
#include <rspf/imaging/rspfTopographicCorrectionFilter.h>

class RSPF_DLL rspfLandsatTopoCorrectionFilter : public rspfTopographicCorrectionFilter
{
public:
   rspfLandsatTopoCorrectionFilter();
   rspfLandsatTopoCorrectionFilter(rspfImageSource* colorSource,
                                    rspfImageSource* elevSource,
                                    const rspfFilename& landsatFileName=rspfFilename(""));
   virtual void initialize();
   virtual void setLandsatHeader(const rspfFilename& header);
   virtual rspfFilename getLandsatHeader()const
      {
         return theLandsatHeader;
      }
   
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
protected:
   virtual ~rspfLandsatTopoCorrectionFilter();
   rspfFilename theLandsatHeader;
   rspfFilename findLandsatHeader();
//   virtual void computeC();
TYPE_DATA
};
#endif
