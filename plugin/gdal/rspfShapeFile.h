#ifndef rspfShapeFile_HEADER
#define rspfShapeFile_HEADER 1
#include <iostream>
#include <shapefil.h>
#include <rspfPluginConstants.h>
#include <rspf/base/rspfRtti.h>
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfDrect.h>
class rspfShapeFile;
class rspfAnnotationObject;
class rspfShapeObject
{
public:
   friend std::ostream& operator<<(std::ostream& out,
                                   const rspfShapeObject& rhs);
   
   rspfShapeObject();
      
   ~rspfShapeObject();
   void setShape(SHPObject* obj);
   bool isLoaded()const;
   long getIndex()const;
   long getId()const;
   bool loadShape(const rspfShapeFile& shapeFile, long shapeRecord);
   
   rspf_uint32 getNumberOfParts()const;
   rspf_uint32 getNumberOfVertices()const;
   
   void getBounds(double& minX, double& minY, double& minZ, double& minM,
                  double& maxX, double& maxY, double& maxZ, double& maxM)const;
   
   void getBounds(double& minX, double& minY,
                  double& maxX, double& maxY)const;
   
   void getBoundingRect(rspfDrect& result,
                        rspfCoordSysOrientMode orient = RSPF_RIGHT_HANDED)const;
   
   rspfDrect getBoundingRect(rspfCoordSysOrientMode orient = RSPF_RIGHT_HANDED)const;
   
   int getType()const;
   rspfString getTypeByName()const;
   int getPartType(rspf_uint32 partIndex)const;
   rspfString getPartByName(rspf_uint32 partIndex)const;
   SHPObject* getShapeObject();
   const SHPObject* getShapeObject()const;
   
protected:
   SHPObject* theShape;
   long       theIndex;
};
class RSPF_PLUGINS_DLL rspfShapeFile : public rspfObject
{
public:
   friend std::ostream& operator <<(std::ostream& out,
                                    const rspfShapeFile& rhs);
   
   rspfShapeFile();
   virtual ~rspfShapeFile();
   
   virtual bool open(const rspfFilename& file,
                     const rspfString& flags=rspfString("rb"));
   
   virtual void close();
   bool isOpen()const;
   virtual SHPHandle getHandle();
   virtual const SHPHandle& getHandle()const;
   virtual std::ostream& print(std::ostream& out) const;
   virtual rspfString getShapeTypeString()const;
   
   
   virtual long getNumberOfShapes()const;
   void getBounds(double& minX, double& minY, double& minZ, double& minM,
                  double& maxX, double& maxY, double& maxZ, double& maxM)const;
   
   void getBounds(double& minX, double& minY,
                  double& maxX, double& maxY)const;
   
   void getBoundingRect(rspfDrect& result,
                        rspfCoordSysOrientMode orient = RSPF_RIGHT_HANDED)const;
   
   rspfDrect getBoundingRect(rspfCoordSysOrientMode orient = RSPF_RIGHT_HANDED)const;
   const rspfFilename& getFilename()const;
protected:
   SHPHandle	 theHandle;
   rspfFilename theFilename;
TYPE_DATA
};
#endif
