//*****************************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description: Class implementation of rspfImageGeometryFactory.
// See .h file for class documentation.
//
//*****************************************************************************
// $Id$
#include <rspf/imaging/rspfImageGeometryFactory.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspf2dTo2dShiftTransform.h>
#include <rspf/imaging/rspfNitfTileSource.h>
#include <rspf/imaging/rspfTiffTileSource.h>
#include <rspf/support_data/rspfNitfRegisteredTag.h>
#include <rspf/support_data/rspfNitfIchipbTag.h>
#include <rspf/support_data/rspfNitfStdidcTag.h>
#include <rspf/projection/rspfSensorModel.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/base/rspf2dTo2dTransformRegistry.h>

rspfImageGeometryFactory* rspfImageGeometryFactory::m_instance = 0;

rspfImageGeometryFactory::rspfImageGeometryFactory()
{
   m_instance = this;
}

rspfImageGeometryFactory* rspfImageGeometryFactory::instance()
{
   if(!m_instance)
   {
      m_instance = new rspfImageGeometryFactory();
   }
   
   return m_instance;
}

rspfImageGeometry* rspfImageGeometryFactory::createGeometry(
   const rspfString& typeName)const
{
   if(typeName == "rspfImageGeometry")
   {
      return new rspfImageGeometry();
   }
   
   return 0;
}

rspfImageGeometry* rspfImageGeometryFactory::createGeometry(
   const rspfKeywordlist& kwl, const char* prefix)const
{
   rspfRefPtr<rspfImageGeometry> result = 0;
   rspfString type = kwl.find(prefix, rspfKeywordNames::TYPE_KW);
   if(!type.empty())
   {
      result = createGeometry(type);
      if(result.valid()&&!result->loadState(kwl, prefix))
      {
         result = 0;
      }
   }
   
   return result.release();
}

rspfImageGeometry* rspfImageGeometryFactory::createGeometry(
   const rspfFilename& /* filename */, rspf_uint32 /* entryIdx */)const
{
   // currently don't support this option just yet by this factory
   return 0;
}

bool rspfImageGeometryFactory::extendGeometry(rspfImageHandler* handler)const
{
   bool result = false;
   if (handler)
   {
      bool add2D = true;
      rspfRefPtr<rspfImageGeometry> geom = handler->getImageGeometry();
      if(geom.valid())
      {
         if(!geom->getProjection())
         {
            geom->setProjection(createProjection(handler));
            result = geom->hasProjection();
         }
         if(geom->getProjection())
         {
            if( !(dynamic_cast<rspfSensorModel*>(geom->getProjection())))
            {
               add2D = false;
            }
         }
         if(!geom->getTransform()&&add2D)
         {
            geom->setTransform(createTransform(handler));
            result |= geom->hasTransform();
        }
      }
   }
   return result;
}

void rspfImageGeometryFactory::getTypeNameList(
   std::vector<rspfString>& typeList)const
{
   typeList.push_back("rspfImageGeometry");
}

rspf2dTo2dTransform* rspfImageGeometryFactory::createTransform(
   rspfImageHandler* handler)const
{
   // Currently nothing to do...
   
   rspfRefPtr<rspf2dTo2dTransform> result = 0;
   
   rspfNitfTileSource* nitf = dynamic_cast<rspfNitfTileSource*> (handler);
   
   if(nitf)
   {
      result = createTransformFromNitf(nitf);
   }
   
   return result.release();
}

rspfProjection* rspfImageGeometryFactory::createProjection(
   rspfImageHandler* handler) const
{
   rspfRefPtr<rspfProjection> result =
      rspfProjectionFactoryRegistry::instance()->createProjection(handler);

   return result.release();
}

rspf2dTo2dTransform* rspfImageGeometryFactory::createTransformFromNitf(rspfNitfTileSource* nitf)const
{
   rspf2dTo2dTransform* result = 0;
   
   rspfRefPtr<rspfNitfImageHeader> hdr = nitf->getCurrentImageHeader();
   if (!hdr)
   {
      return result;
   }
   
   //---
   // Test for the ichipb tag and set the sub image if needed.
   // 
   // NOTE # 1:
   // 
   // There are nitf writers that set the ichipb offsets and only have
   // IGEOLO field present.  For these it has been determined
   // (but still in question) that we should not apply the sub image offset.
   //
   // See trac # 1578
   // http://trac.osgeo.org/rspf/ticket/1578
   //
   // NOTE # 2:
   //
   // Let the ICHIPB have precedence over the STDIDC tag as we could have a
   // chip of a segment.
   //---
   rspfRefPtr<rspfNitfRegisteredTag> tag =
   hdr->getTagData(rspfString("ICHIPB"));
   if (tag.valid())
   {
      rspfNitfIchipbTag* ichipb = PTR_CAST(rspfNitfIchipbTag, tag.get());
      if (ichipb)
      {
//         const rspfRefPtr<rspfNitfRegisteredTag> blocka =
//         hdr->getTagData(rspfString("BLOCKA"));
//         const rspfRefPtr<rspfNitfRegisteredTag> rpc00a =
//         hdr->getTagData(rspfString("RPC00A"));              
//         const rspfRefPtr<rspfNitfRegisteredTag> rpc00b =
//         hdr->getTagData(rspfString("RPC00B"));
         
         //---
         // If any of these tags are present we will use the sub image from
         // the ichipb tag.
         //---
//         if ( blocka.get() || rpc00a.get() || rpc00b.get() )
         
         // ************************* THERE ARE PROBLEMS NOT SETTING THIS AT SITE.  GO AHEAD AND ALWAYS INIT THE SHIFT
         {
            result = ichipb->newTransform();
         }
      }
   }
   
   if ( !result)
   {
      //---
      // Look for the STDIDC tag for a sub image (segment) offset.
      //
      // See: STDI-002 Table 7.3 for documentation.
      //---
      tag = hdr->getTagData(rspfString("STDIDC"));
      if (tag.valid() && (hdr->getIMode() == "B") )
      {
         rspfDpt shift;
         rspfNitfStdidcTag* stdidc = PTR_CAST(rspfNitfStdidcTag, tag.get());
         if (stdidc)
         {
            rspf_int32 startCol = stdidc->getStartColumn().toInt32();
            rspf_int32 startRow = stdidc->getStartRow().toInt32();
            if ( (startCol > 0) && (startRow > 0) )
            {
               
               // field are one based; hence, the - 1.
               shift.x = (startCol-1) * hdr->getNumberOfPixelsPerBlockHoriz();
               shift.y = (startRow-1) * hdr->getNumberOfPixelsPerBlockVert();
            }
            if(shift.x > 0 ||
               shift.y > 0)
            {
               result = new rspf2dTo2dShiftTransform(shift);
            }
         }
      }
   }
   
   return result;
}

