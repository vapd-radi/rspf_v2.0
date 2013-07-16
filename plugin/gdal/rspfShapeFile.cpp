#include <iomanip>
#include <rspfShapeFile.h>
RTTI_DEF1(rspfShapeFile, "rspfShapeFile", rspfObject);
std::ostream& operator<<(std::ostream& out, const rspfShapeObject& rhs)
{
   if(rhs.isLoaded())
   {
      double minx, miny, minz, minm;
      double maxx, maxy, maxz, maxm;
      rspf_uint32 i = 0;
      
      rhs.getBounds(minx, miny, minz, minm,
                    maxx, maxy, maxz, maxm);
      
      out << std::setw(15) << setiosflags(std::ios::left)<<"type:"<<rhs.getTypeByName() << std::endl
          << std::setw(15) << setiosflags(std::ios::left)<<"id:"<<rhs.getId()<<std::endl
          << std::setw(15) << setiosflags(std::ios::left)<<"minx:"<<minx <<std::endl
          << std::setw(15) << setiosflags(std::ios::left)<<"miny:"<<miny <<std::endl
          << std::setw(15) << setiosflags(std::ios::left)<<"minz:"<<minz <<std::endl
          << std::setw(15) << setiosflags(std::ios::left)<<"minm:"<<minm <<std::endl
          << std::setw(15) << setiosflags(std::ios::left)<<"maxx:"<<maxx <<std::endl
          << std::setw(15) << setiosflags(std::ios::left)<<"maxy:"<<maxy <<std::endl
          << std::setw(15) << setiosflags(std::ios::left)<<"maxz:"<<maxz <<std::endl
          << std::setw(15) << setiosflags(std::ios::left)<<"maxm:"<<maxm <<std::endl
          << std::setw(15) << setiosflags(std::ios::left)<<"parts:"<<rhs.getNumberOfParts()<<std::endl
          << std::setw(15) << setiosflags(std::ios::left)<<"vertices:"<<rhs.getNumberOfVertices();
      if(rhs.getNumberOfParts())
      {
         out << std::endl;
         for(i = 0; i < rhs.getNumberOfParts()-1; ++i)
         {
            rspfString s1 = "part start ";
            rspfString s2 = "part type ";
            s1 += (rspfString::toString(i+1)+":");
            s2 += (rspfString::toString(i+1)+":");
            
            out << std::setw(15) << setiosflags(std::ios::left) << s1.c_str() << rhs.theShape->panPartStart[i]<<std::endl;
            out << std::setw(15) << setiosflags(std::ios::left) << s2.c_str() <<  SHPPartTypeName(rhs.theShape->panPartType[i])<<std::endl;
         }
         rspfString s1 = "part start ";
         rspfString s2 = "part type ";
         
         s1 += (rspfString::toString(rhs.getNumberOfParts())+":");
         out << std::setw(15) << setiosflags(std::ios::left) << s1.c_str() <<  rhs.theShape->panPartStart[i]<<std::endl;
         out << std::setw(15) << setiosflags(std::ios::left) << s2.c_str() <<  SHPPartTypeName(rhs.theShape->panPartType[i]);
      }
      
      out << std::setw(0);
      
   }
   return out;   
}
rspfShapeObject::rspfShapeObject()
   :theShape((SHPObject*)NULL),
    theIndex(-1)
{
}
rspfShapeObject::~rspfShapeObject()
{
   if(theShape)
   {
      SHPDestroyObject(theShape);
      theShape = NULL;
   }
}
void rspfShapeObject::getBoundingRect(rspfDrect& result,
                                       rspfCoordSysOrientMode orient)const
{
   double minx, miny, maxx, maxy;
   
   if(theShape)
   {
      getBounds(minx, miny, maxx, maxy);
      
      if(orient == RSPF_RIGHT_HANDED)
      {
         result = rspfDrect(minx, maxy, maxx, miny, orient);
      }
      else
      {
         result = rspfDrect(minx, miny, maxx, maxy, orient);         
      }
   }
   else
   {
      result = rspfDrect(0,0,0,0,orient);
      result.makeNan();
   }
}
void rspfShapeObject::setShape(SHPObject* obj)
{
   if(theShape)
   {
      SHPDestroyObject(theShape);
      theShape = NULL;            
   }
   
   theShape = obj;
}
bool rspfShapeObject::isLoaded()const
{
   return (theShape!=NULL);
}
long rspfShapeObject::getIndex()const
{
   return theIndex;
}
long rspfShapeObject::getId()const
{
   if(theShape)
   {
      return theShape->nShapeId;
   }
   
   return -1;
}
bool rspfShapeObject::loadShape(const rspfShapeFile& shapeFile,
                                 long shapeRecord)
{
   if(theShape)
   {
      SHPDestroyObject(theShape);
      theShape = NULL;            
   }
   if(shapeFile.isOpen())
   {
      theShape = SHPReadObject(shapeFile.getHandle(),
                               shapeRecord);
      theIndex = shapeRecord;
   }
   else
   {
      theIndex = -1;
   }
   return (theShape != (SHPObject*)NULL);
}
rspf_uint32 rspfShapeObject::getNumberOfParts()const
{
   if(theShape)
   {
      return theShape->nParts;
   }
   
   return 0;
}
rspf_uint32 rspfShapeObject::getNumberOfVertices()const
{
   if(theShape)
   {
      return theShape->nVertices;
   }
   
   return 0;
}
void rspfShapeObject::getBounds(
   double& minX, double& minY, double& minZ, double& minM,
   double& maxX, double& maxY, double& maxZ, double& maxM)const
{
   if(theShape)
   {
      minX = theShape->dfXMin;
      minY = theShape->dfYMin;
      minZ = theShape->dfZMin;
      minM = theShape->dfMMin;
      maxX = theShape->dfXMax;
      maxY = theShape->dfYMax;
      maxZ = theShape->dfZMax;
      maxM = theShape->dfMMax;
   }
   else
   {
      minX = minY = minZ = minM =
         maxX = maxY = maxZ = maxM = rspf::nan();
   }
}
void rspfShapeObject::getBounds(double& minX, double& minY,
                                 double& maxX, double& maxY)const
{
   if(theShape)
   {
      minX = theShape->dfXMin;
      minY = theShape->dfYMin;
      maxX = theShape->dfXMax;
      maxY = theShape->dfYMax;
   }
   else
   {
      minX = minY = 
         maxX = maxY = rspf::nan();
   }
}
rspfDrect rspfShapeObject::getBoundingRect(
   rspfCoordSysOrientMode orient)const
{
   rspfDrect result;
   
   getBoundingRect(result, orient);
   
   return result;
}
   
int rspfShapeObject::getType()const
{
   if(theShape)
   {
      return theShape->nSHPType;
   }
   return SHPT_NULL;
}
rspfString rspfShapeObject::getTypeByName()const
{
   if(theShape)
   {
      return rspfString(SHPTypeName(theShape->nSHPType));
   }
   return "unknown";
}
int rspfShapeObject::getPartType(rspf_uint32 partIndex)const
{
   if((partIndex > getNumberOfParts())||
      (!theShape))
   {
      return -1;
   }
   
   return theShape->panPartType[partIndex];
}
rspfString rspfShapeObject::getPartByName(rspf_uint32 partIndex)const
{
   if((partIndex > getNumberOfParts())||
      (!theShape))
   {
   }
   switch(theShape->panPartType[partIndex])
   {
      case SHPP_TRISTRIP:
      {
         rspfString("tristrip");
         break;
      }
      case SHPP_TRIFAN:
      {
         rspfString("trifan");
         break;
      }
      case SHPP_OUTERRING:
      {
         rspfString("outerring");
         break;
      }
      case SHPP_INNERRING:
      {
         rspfString("innerring");
         break;
      }
      case SHPP_RING:
      {
         rspfString("ring");
         break;
      }
   }
   return rspfString("unknown");
}
SHPObject* rspfShapeObject::getShapeObject()
{
   return theShape;
}
const SHPObject* rspfShapeObject::getShapeObject()const
{
   return theShape;
}
   
std::ostream& operator <<(std::ostream& out, const rspfShapeFile& rhs)
{
   rhs.print(out);
   
   return out;
}
rspfShapeFile::rspfShapeFile()
   :theHandle(NULL)
{
}
rspfShapeFile::~rspfShapeFile()
{
   close();
}
void rspfShapeFile::close()
{
   if(theHandle)
   {
      SHPClose(theHandle);
      theHandle = NULL;
   }
}
bool rspfShapeFile::open(const rspfFilename& file,
                          const rspfString& flags)
{
   if(isOpen()) close();
   
   theHandle = SHPOpen( file.c_str(),
                        flags.c_str());
   if(isOpen())
   {
      theFilename = file;
   }
   
   return (theHandle != NULL);
}
bool rspfShapeFile::isOpen()const
{
   return (theHandle!=NULL);
}
SHPHandle rspfShapeFile::getHandle()
{
   return theHandle;
}
   
const SHPHandle& rspfShapeFile::getHandle()const
{
   return theHandle;
}
long rspfShapeFile::getNumberOfShapes()const
{
   if(theHandle)
   {
      return theHandle->nRecords;
   }
   return 0;
}
void rspfShapeFile::getBounds(
   double& minX, double& minY, double& minZ, double& minM,
   double& maxX, double& maxY, double& maxZ, double& maxM)const
{
   if(theHandle)
   {
      minX = theHandle->adBoundsMin[0];
      minY = theHandle->adBoundsMin[1];
      minZ = theHandle->adBoundsMin[2];
      minM = theHandle->adBoundsMin[3];
      maxX = theHandle->adBoundsMax[0];
      maxY = theHandle->adBoundsMax[1];
      maxZ = theHandle->adBoundsMax[2];
      maxM = theHandle->adBoundsMax[3];
   }
   else
   {
      minX = minY = minZ = minM =
         maxX = maxY = maxZ = maxM = rspf::nan();
   }
}
   
void rspfShapeFile::getBounds(double& minX, double& minY,
                               double& maxX, double& maxY)const
{
   if(theHandle)
   {
      minX = theHandle->adBoundsMin[0];
      minY = theHandle->adBoundsMin[1];
      maxX = theHandle->adBoundsMax[0];
      maxY = theHandle->adBoundsMax[1];
   }
   else
   {
      minX = minY = maxX = maxY = rspf::nan();
   }
}
std::ostream& rspfShapeFile::print(std::ostream& out) const
{
   if(isOpen())
   {
      out << std::setw(15) << setiosflags(std::ios::left)<<"Shp filename:" << theFilename << std::endl;
      out << std::setw(15) << setiosflags(std::ios::left)<<"Record count:" << theHandle->nRecords << std::endl;
      out << std::setw(15) << setiosflags(std::ios::left)<<"File type:" << getShapeTypeString().c_str() << std::endl;
      out << std::setw(15) << setiosflags(std::ios::left)<<"minx:" << theHandle->adBoundsMin[0] << std::endl;
      out << std::setw(15) << setiosflags(std::ios::left)<<"miny:" << theHandle->adBoundsMin[1] << std::endl;
      out << std::setw(15) << setiosflags(std::ios::left)<<"minz:" << theHandle->adBoundsMin[2] << std::endl;
      out << std::setw(15) << setiosflags(std::ios::left)<<"minm:" << theHandle->adBoundsMin[3] << std::endl;
      out << std::setw(15) << setiosflags(std::ios::left)<<"maxx:" << theHandle->adBoundsMax[0] << std::endl;
      out << std::setw(15) << setiosflags(std::ios::left)<<"maxy:" << theHandle->adBoundsMax[1] << std::endl;
      out << std::setw(15) << setiosflags(std::ios::left)<<"maxz:" << theHandle->adBoundsMax[2] << std::endl;
      out << std::setw(15) << setiosflags(std::ios::left)<<"maxm:" << theHandle->adBoundsMax[3] << std::endl;
      rspfShapeObject shape;
      
      if(theHandle->nRecords)
      {
         out << std::setw(30) << std::setfill('_') << "" << std::setfill(' ')<<std::endl;
      }
      for(int i=0; i < theHandle->nRecords; ++i)
      {
         if(shape.loadShape(*this, i))
         {
            out << shape << std::endl;
            out << std::setw(30) << std::setfill('_') << ""<<std::setfill(' ')<<std::endl;
         }
      }
   }
   return out;
}
rspfString rspfShapeFile::getShapeTypeString()const
{
   if(theHandle)
   {
      return SHPTypeName(theHandle->nShapeType);
   }
   return "";
}
void rspfShapeFile::getBoundingRect(rspfDrect& result,
                                     rspfCoordSysOrientMode orient)const
{
   double minx, miny, maxx, maxy;
   
   getBounds(minx, miny, maxx, maxy);
   if(orient == RSPF_RIGHT_HANDED)
   {
      result = rspfDrect(minx, maxy, maxx, miny, orient);
   }
   else
   {
      result = rspfDrect(minx, miny, maxx, maxy, orient);         
   }
}
rspfDrect rspfShapeFile::getBoundingRect(
   rspfCoordSysOrientMode orient)const
{
   rspfDrect result;
   
   getBoundingRect(result, orient);
   
   return result;
}
const rspfFilename& rspfShapeFile::getFilename()const
{
   return theFilename;
}
