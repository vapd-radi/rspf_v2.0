//*****************************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description: Class declaration of rspfImageGeometryFactory.
//
//*****************************************************************************
// $Id$
#ifndef rspfImageGeometryFactory_HEADER
#define rspfImageGeometryFactory_HEADER 1

#include <rspf/imaging/rspfImageGeometryFactoryBase.h>

class rspf2dTo2dTransform;
class rspfNitfTileSource;
class rspfProjection;

class RSPF_DLL rspfImageGeometryFactory : public rspfImageGeometryFactoryBase
{
public:
   static rspfImageGeometryFactory* instance();
   virtual rspfImageGeometry* createGeometry(const rspfString& typeName)const;
   virtual rspfImageGeometry* createGeometry(const rspfKeywordlist& kwl,
                                              const char* prefix=0)const;
   virtual rspfImageGeometry* createGeometry(const rspfFilename& filename,
                                              rspf_uint32 entryIdx)const;
   virtual bool extendGeometry(rspfImageHandler* handler)const;
   
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;
   
   /**
    * This is a utility method used by crateGeoemtry that takes an image handler
    */
   virtual rspf2dTo2dTransform* createTransform(rspfImageHandler* handler)const;

   /**
    * @brief Utility method to create a projection from an image handler.
    * @param handler The image handler to create projection from.
    * @return Pointer to an rspfProjection on success, null on error.
    */
   virtual rspfProjection* createProjection(rspfImageHandler* handler) const;
   
protected:
   rspfImageGeometryFactory();

   virtual rspf2dTo2dTransform* createTransformFromNitf(rspfNitfTileSource* handler)const;
   
   static rspfImageGeometryFactory* m_instance;
};

#endif
