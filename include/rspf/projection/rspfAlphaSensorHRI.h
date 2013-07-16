//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Dave Hicks
//
// Description:  Alpha HRI Sensor Model
//
//*******************************************************************
//  $Id$
#ifndef rspfAlphaSensorHRI_HEADER
#define rspfAlphaSensorHRI_HEADER 1
#include <rspf/projection/rspfAlphaSensor.h>

class rspfAlphaSensorSupportData;

class RSPF_DLL rspfAlphaSensorHRI : public rspfAlphaSensor
{
public:
   rspfAlphaSensorHRI();

   rspfAlphaSensorHRI(const rspfAlphaSensorHRI& src);

   virtual rspfObject* dup()const;
   
   virtual void imagingRay(const rspfDpt& image_point,
                           rspfEcefRay&   image_ray) const;

   virtual void worldToLineSample(const rspfGpt& world_point,
                                  rspfDpt&       image_point) const;

   virtual void updateModel();
   
   virtual void initAdjustableParameters();
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=0)const;
   
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

#endif // #ifndef rspfAlphaSensorHRI_HEADER
