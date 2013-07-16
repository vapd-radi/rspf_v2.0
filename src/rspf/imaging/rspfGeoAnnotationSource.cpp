//************************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See LICENSE.txt file in the top level directory.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfGeoAnnotationSource.cpp 19900 2011-08-04 14:19:57Z dburken $

#include <rspf/imaging/rspfGeoAnnotationSource.h>
#include <rspf/imaging/rspfGeoAnnotationObject.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>

RTTI_DEF2(rspfGeoAnnotationSource,
          "rspfGeoAnnotationSource",
          rspfAnnotationSource,
          rspfViewInterface);
   

ostream& operator <<(ostream& out, const rspfGeoAnnotationSource& rhs)
{
   const rspfAnnotationSource::AnnotationObjectListType &tempList = rhs.getObjectList();
   for(rspf_uint32 index = 0; index < tempList.size(); ++index)
   {
      out << "output " << index << endl;
      tempList[index]->print(out);
   }

   return out;
}

rspfGeoAnnotationSource::rspfGeoAnnotationSource(rspfImageGeometry* geom,
                                                   bool /* ownsProjectionFlag */ )
   :rspfAnnotationSource(),
    rspfViewInterface(),
    m_geometry(geom)
{
   rspfViewInterface::theObject = this;
}

rspfGeoAnnotationSource::rspfGeoAnnotationSource(rspfImageSource* inputSource,
                                                   rspfImageGeometry* geom,
                                                   bool /* ownsProjectionFlag */)
   :rspfAnnotationSource(inputSource),
    rspfViewInterface(),
    m_geometry(geom)
{
   rspfViewInterface::theObject = this;
}

rspfGeoAnnotationSource::~rspfGeoAnnotationSource()
{
}

bool rspfGeoAnnotationSource::addObject(rspfAnnotationObject* anObject)
{
   rspfGeoAnnotationObject *objectToAdd = PTR_CAST(rspfGeoAnnotationObject, anObject);
   
   if(objectToAdd)
   {
      rspfAnnotationSource::addObject(objectToAdd);
      if(m_geometry.valid())
      {
         objectToAdd->transform(m_geometry.get());
         computeBoundingRect();
      }
      return true;
   }

   return false;
}

void rspfGeoAnnotationSource::setGeometry(rspfImageGeometry* geom)
{
   m_geometry = geom;
   transformObjects(m_geometry.get());
}

bool rspfGeoAnnotationSource::setView(rspfObject* baseObject)
{
   bool result = false;
   rspfProjection* proj = PTR_CAST(rspfProjection, baseObject);
   if(proj)
   {
      if(m_geometry.valid())
      {
         m_geometry->setProjection(proj);
      }
      else
      {
         m_geometry = new rspfImageGeometry(0, proj);
      }
      setGeometry(m_geometry.get());
      result = true;
   }
   else
   {
      m_geometry = dynamic_cast<rspfImageGeometry*>(baseObject);
      if ( m_geometry.valid() )
      {
         result = true;
      }
   }
   return result;
}

rspfObject* rspfGeoAnnotationSource::getView()
{
   return m_geometry.get();
}

const rspfObject* rspfGeoAnnotationSource::getView()const
{
   return m_geometry.get();
}

void rspfGeoAnnotationSource::computeBoundingRect()
{
//   static const char *MODULE = "rspfAnnotationSource::computeBoundingRect";
   
   theRectangle.makeNan();

   if(theAnnotationObjectList.size()>0)
   {
      rspfDrect rect;
      theAnnotationObjectList[0]->getBoundingRect(theRectangle);
      
      AnnotationObjectListType::iterator object = (theAnnotationObjectList.begin()+1);
      while(object != theAnnotationObjectList.end())
      {
         (*object)->getBoundingRect(rect);
         theRectangle = theRectangle.combine(rect);
         ++object;
      }
   }
}

void rspfGeoAnnotationSource::transformObjects(rspfImageGeometry* geom)
{
   rspfRefPtr<rspfImageGeometry> tempGeom = geom;
   if(!tempGeom)
   {
      tempGeom = m_geometry.get();
   }

   if(!tempGeom) return;
   AnnotationObjectListType::iterator currentObject;

   currentObject = theAnnotationObjectList.begin();

   while(currentObject != theAnnotationObjectList.end())
   {
      // this is safe since we trapped all adds to make
      // sure that each object added to the list is
      // geographic.
      rspfGeoAnnotationObject* object = static_cast<rspfGeoAnnotationObject*>((*currentObject).get());

      // transform the object to image space.
      object->transform(m_geometry.get());

      ++currentObject;
   }
   computeBoundingRect();
}

rspfRefPtr<rspfImageGeometry> rspfGeoAnnotationSource::getImageGeometry()
{
   if(!m_geometry.valid())
   {
      return rspfImageSource::getImageGeometry();
   }
   return m_geometry;
}

bool rspfGeoAnnotationSource::saveState(rspfKeywordlist& kwl,
                                         const char* prefix)const
{
   return rspfAnnotationSource::saveState(kwl, prefix);
}


bool rspfGeoAnnotationSource::loadState(const rspfKeywordlist& kwl,
                                         const char* prefix)
{
   m_geometry = new rspfImageGeometry;
   
   rspfString newPrefix = rspfString(prefix)+"view_proj.";
   if(!m_geometry->loadState(kwl, newPrefix.c_str()))
   {
      m_geometry = 0;
   }
   else
   {
      if(!m_geometry->hasProjection())
      {
         m_geometry = 0;
      }
   }
   
   return rspfAnnotationSource::loadState(kwl, prefix);
}
