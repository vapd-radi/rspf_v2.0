#include <algorithm>

#include <rspf/imaging/rspfVpfAnnotationSource.h>
#include <rspf/imaging/rspfVpfAnnotationLibraryInfo.h>
#include <rspf/vec/rspfVpfExtent.h>
#include <rspf/projection/rspfEquDistCylProjection.h>
#include <rspf/imaging/rspfRgbImage.h>
#include <rspf/imaging/rspfU8ImageData.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfKeywordNames.h>

static rspfTrace traceDebug("rspfVpfAnnotationSource:debug");

rspfVpfAnnotationSource::rspfVpfAnnotationSource()
   :rspfGeoAnnotationSource()
{
  if(!m_geometry)
  {
     m_geometry = new rspfImageGeometry(0, new rspfEquDistCylProjection);
  }
   rspfMapProjection* mapProj = dynamic_cast<rspfMapProjection*>(m_geometry->getProjection());

   if(mapProj)
   {
      mapProj->setMetersPerPixel(rspfDpt(30, 30));
   }
}

rspfVpfAnnotationSource::~rspfVpfAnnotationSource()
{
   close();
}

bool rspfVpfAnnotationSource::open(const rspfFilename& file)
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfVpfAnnotationSource::open DEBUG: entering..." << std::endl;
   }
   bool result = false;
   
   if(file.file().downcase() == "dht")
   {
      if(openDatabase(file))
      {
         vector<rspfString> libraryNames = theDatabase.getLibraryNames();
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG) << "Library names list size = " << libraryNames.size() << std::endl;
         }
         
         for(int idx = 0; idx < (int)libraryNames.size();++idx)
         {
            rspfVpfAnnotationLibraryInfo* info = new rspfVpfAnnotationLibraryInfo;
            info->setName(libraryNames[idx]);
            theLibraryInfo.push_back(info);
            info->setDatabase(&theDatabase);
            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG) << "loading library " << libraryNames[idx] << std::endl;
            }
            info->buildLibrary();
            info->getAllFeatures(theFeatureList);
         }
         transformObjects();
         result = true;
      }
   }
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfVpfAnnotationSource::open DEBUG: leaving..." << std::endl;
   }
   return result;
}

void rspfVpfAnnotationSource::close()
{
   theDatabase.closeDatabase();
   for(int idx = 0; idx < (int)theLibraryInfo.size(); ++idx)
   {
      delete theLibraryInfo[idx];
   }
   
   theLibraryInfo.clear();
}

rspfFilename rspfVpfAnnotationSource::getFilename()const
{
   return theFilename;
}

void rspfVpfAnnotationSource::transformObjects(rspfImageGeometry* geom)
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfVpfAnnotationSource::transformObjects DEBUG: entered..." << std::endl;
   }
   rspfImageGeometry* tempGeom = geom;
   
   if(!tempGeom)
   {
      tempGeom = m_geometry.get();
   }
   
   if(!tempGeom) return;
   for(int idx = 0; idx < (int)theLibraryInfo.size(); ++idx)
   {
      theLibraryInfo[idx]->transform(tempGeom);
   }
   computeBoundingRect();
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfVpfAnnotationSource::transformObjects DEBUG: leaving..." << std::endl;
   }
}

void rspfVpfAnnotationSource::computeBoundingRect()

{
  rspfIrect result;
  result.makeNan();
  for(int i = 0; i < (int)theLibraryInfo.size();++i)
    {
      rspfIrect tempRect = theLibraryInfo[i]->getBoundingProjectedRect();
      if(!tempRect.hasNans())
	{
	  if(result.hasNans())
	    {
	      result = tempRect;
	    }
	  else
	    {
	      result = result.combine(tempRect);
	    }
	}
    }

  theRectangle = result;
}


void rspfVpfAnnotationSource::deleteAllLibraries()
{
  for(int idx = 0; idx < (int)theLibraryInfo.size();++idx)
    {
      delete theLibraryInfo[idx];
    }
  
  theLibraryInfo.clear();
}

void rspfVpfAnnotationSource::getAllFeatures(std::vector<rspfVpfAnnotationFeatureInfo*>& features)
{
   features = theFeatureList;
}

void rspfVpfAnnotationSource::setAllFeatures(const std::vector<rspfVpfAnnotationFeatureInfo*>& features)
{
  theFeatureList = features;
  
  computeBoundingRect();
  
}


void rspfVpfAnnotationSource::drawAnnotations(rspfRefPtr<rspfImageData> tile)
{
   theImage->setCurrentImageData(tile);
   if(theImage->getImageData().valid())
   {
      for(rspf_uint32 idx = 0; idx < theFeatureList.size();++idx)
      {
         if(theFeatureList[idx]->getEnabledFlag())
         {
            theFeatureList[idx]->drawAnnotations(theImage.get());
         }
      }
   }  
}

bool rspfVpfAnnotationSource::openDatabase(const rspfFilename& file)
{
  rspfAnnotationSource::deleteAll();
  deleteAllLibraries();
  theFeatureList.clear();
  if(theDatabase.isOpened())
    {
      theDatabase.closeDatabase();
    }

  theFilename = file;
  return theDatabase.openDatabase(file);
}


bool rspfVpfAnnotationSource::saveState(rspfKeywordlist& kwl,
                                         const char* prefix)const
{
  kwl.add(prefix,
	  rspfKeywordNames::FILENAME_KW,
	  theFilename,
	  true);
  for(rspf_uint32 idx = 0; idx < theLibraryInfo.size(); ++idx)
    {
       theLibraryInfo[idx]->saveState(kwl,
 				     (rspfString(prefix) + "library" + rspfString::toString(idx) + ".").c_str());
    }
   return true;
}

bool rspfVpfAnnotationSource::loadState(const rspfKeywordlist& kwl,
                                         const char* prefix)
{
  bool result = false;
  deleteAllLibraries();
  const char* filename    = kwl.find(prefix, rspfKeywordNames::FILENAME_KW);
  if(filename)
    {
      if(openDatabase(filename))
	{
	  int idx = 0;
	  rspfString regExpression =  rspfString("^(") + rspfString(prefix) + "library[0-9]+.)";
	  vector<rspfString> keys =
	    kwl.getSubstringKeyList( regExpression );
	  std::vector<int> theNumberList(keys.size());
	  int offset = (int)(rspfString(prefix)+"library").size();

	  for(idx = 0; idx < (int)theNumberList.size();++idx)
	    {
	      rspfString numberStr(keys[idx].begin() + offset,
				    keys[idx].end());
	      theNumberList[idx] = numberStr.toInt();
	    }
	  std::sort(theNumberList.begin(), theNumberList.end());

	  for(idx=0;idx < (int)keys.size();++idx)
	    {
	      rspfString newPrefix = rspfString(prefix);
	      newPrefix += rspfString("library");
	      newPrefix += rspfString::toString(theNumberList[idx]);
	      newPrefix += rspfString(".");

	      rspfVpfAnnotationLibraryInfo* info = new rspfVpfAnnotationLibraryInfo;
	      theLibraryInfo.push_back(info);
	      info->setDatabase(&theDatabase);
	      info->loadState(kwl,
			      newPrefix);
	      info->getAllFeatures(theFeatureList);
	    }
	  transformObjects();
	  result = true;
	}
    }
  
  return result;
}

bool rspfVpfAnnotationSource::open()
{
   return open(theFilename);
}

bool rspfVpfAnnotationSource::isOpen()const
{
   return (theDatabase.isOpened());
}
