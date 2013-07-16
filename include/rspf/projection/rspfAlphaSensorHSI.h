//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Dave Hicks
//
// Description:  Alpha HSI Sensor Model
//
//*******************************************************************
//  $Id$
#ifndef rspfAlphaSensorHSI_HEADER
#define rspfAlphaSensorHSI_HEADER 1
#include <rspf/projection/rspfAlphaSensor.h>

class rspfAlphaSensorSupportData;

class RSPF_DLL rspfAlphaSensorHSI : public rspfAlphaSensor
{
public:
   rspfAlphaSensorHSI();

   rspfAlphaSensorHSI(const rspfAlphaSensorHSI& src);

   virtual rspfObject* dup()const;
   
   virtual void imagingRay(const rspfDpt& image_point,
                           rspfEcefRay&   image_ray) const;
   
   virtual void worldToLineSample(const rspfGpt& world_point,
                                  rspfDpt&       image_point) const;

   virtual void updateModel();
   
   virtual void initAdjustableParameters();
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=0)const;

   rspf_float64 getScanAngle(const rspf_float64& line)const;
   
   /**
    * @brief Initializes model from support data and calls update Model on 
    * success.
    * @param supData
    * @return true on success, false on error.
    */
   virtual bool initialize( const rspfAlphaSensorSupportData& supData );

protected:
   TYPE_DATA
};

#endif // #ifndef rspfAlphaSensorHSI_HEADER
