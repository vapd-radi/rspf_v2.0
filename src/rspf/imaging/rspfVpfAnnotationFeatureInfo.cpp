#include <sstream>

#include <rspf/imaging/rspfVpfAnnotationFeatureInfo.h>
#include <rspf/vec/rspfVpfCoverage.h>
#include <rspf/vec/rspfVpfLibrary.h>
#include <rspf/vec/rspfVpfFeatureClassSchema.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/base/rspfPolyLine.h>
#include <rspf/base/rspfGeoPolygon.h>
#include <rspf/imaging/rspfGeoAnnotationSource.h>
#include <rspf/imaging/rspfAnnotationObject.h>
#include <rspf/imaging/rspfGeoAnnotationMultiPolyLineObject.h>
#include <rspf/imaging/rspfGeoAnnotationMultiEllipseObject.h>
#include <rspf/imaging/rspfGeoAnnotationFontObject.h>
#include <rspf/imaging/rspfGeoAnnotationMultiPolyObject.h>
#include <rspf/imaging/rspfRgbImage.h>
#include <rspf/font/rspfFont.h>
#include <rspf/font/rspfFontFactoryRegistry.h>
#include <rspf/base/rspfTrace.h>

static rspfTrace traceDebug("rspfVpfAnnotationFeatureInfo:debug");

RTTI_DEF1(rspfVpfAnnotationFeatureInfo, "rspfVpfAnnotationFeatureInfo", rspfObject);

rspfVpfAnnotationFeatureInfo::rspfVpfAnnotationFeatureInfo(const rspfString& featureName,
                                                             const rspfRgbVector& penColor,
                                                             const rspfRgbVector& brushColor,
                                                             const rspfDpt& pointRadius,
                                                             int   thickness,
                                                             bool enabledFlag)
   :theName(featureName),
    thePenColor(penColor),
    theBrushColor(brushColor),
    thePointRadius(pointRadius),
    theThickness(thickness),
    theFillEnabledFlag(false),
    theEnabledFlag(enabledFlag),
    theFeatureType(rspfVpfAnnotationFeatureType_UNKNOWN),
    theFontInformation(),
    theAnnotationArray(0)
{
   rspfFont* font = rspfFontFactoryRegistry::instance()->getDefaultFont();

   if(font)
   {
      std::vector<rspfFontInformation> fontInfoArray;

      font->getFontInformation(fontInfoArray);

      if(fontInfoArray.size())
      {
         theFontInformation = fontInfoArray[0];
      }
   }
}


rspfVpfAnnotationFeatureInfo::~rspfVpfAnnotationFeatureInfo()
{
   deleteAllObjects();
}

void rspfVpfAnnotationFeatureInfo::transform(rspfImageGeometry* proj)
{
   if(theEnabledFlag)
   {
      for(int idx = 0; idx < (int)theAnnotationArray.size();++idx)
      {
         if(theAnnotationArray[idx].valid())
         {
            theAnnotationArray[idx]->transform(proj);
            theAnnotationArray[idx]->computeBoundingRect();
         }
      }
   }
}
rspfIrect rspfVpfAnnotationFeatureInfo::getBoundingProjectedRect()const
{
  rspfIrect result;
  result.makeNan();

  if(theEnabledFlag)
  {
     for(int idx = 0; idx < (int)theAnnotationArray.size();++idx)
     {
        if(theAnnotationArray[idx].valid())
	{
           rspfIrect tempRect = theAnnotationArray[idx]->getBoundingRect();
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
     }
  }
  
  return result;
}

void rspfVpfAnnotationFeatureInfo::setEnabledFlag(bool flag)
{
   if(!flag && theEnabledFlag)
   {
      deleteAllObjects();
   }
   else if(flag&&(!theEnabledFlag))
   {
      theEnabledFlag = flag;
      buildFeature();
   }
   theEnabledFlag = flag;
}

void rspfVpfAnnotationFeatureInfo::buildFeature()
{
  deleteAllObjects();
  
  rspfVpfFeatureClassSchemaNode node;

  rspfVpfFeatureClassSchema schema;
  theCoverage.openFeatureClassSchema(schema);
  if(schema.getFeatureClassNode(theName,
				node))
    {
      if(node.thePrimitiveTable.contains("edg"))
      {
         theFeatureType = rspfVpfAnnotationFeatureType_LINE;
	 if(theEnabledFlag)
	   {
	     buildEdgFeature(node.theTable.trim(),
			     node.theTableKey.trim(),
			     node.thePrimitiveTable.trim(),
			     node.thePrimitiveTableKey.trim());
	   }
	}
      else if(node.thePrimitiveTable.contains("txt"))
	{
            theFeatureType = rspfVpfAnnotationFeatureType_TEXT;
	    if(theEnabledFlag)
	      {
		buildTxtFeature(node.theTable.trim(),
				node.theTableKey.trim(),
				node.thePrimitiveTable.trim(),
				node.thePrimitiveTableKey.trim());
	      }
	}
      else if(node.thePrimitiveTable.contains("fac"))
	{
            theFeatureType = rspfVpfAnnotationFeatureType_POLYGON;
	    if(theEnabledFlag)
	      {
		buildFaceFeature(node.theTable.trim(),
				 node.theTableKey.trim(),
				 node.thePrimitiveTable.trim(),
				 node.thePrimitiveTableKey.trim());
	      }
	}
      else if(node.thePrimitiveTable.contains("cnd"))
	{
            theFeatureType = rspfVpfAnnotationFeatureType_POINT;
	    if(theEnabledFlag)
	      {
		buildPointFeature("cnd",
				  node.theTable.trim(),
				  node.theTableKey.trim(),
				  node.thePrimitiveTable.trim(),
				  node.thePrimitiveTableKey.trim());
	      }
	}
      else if(node.thePrimitiveTable.contains("end"))
	{
            theFeatureType = rspfVpfAnnotationFeatureType_POINT;
	    if(theEnabledFlag)
	      {
		buildPointFeature("end",
				  node.theTable.trim(),
				  node.theTableKey.trim(),
				  node.thePrimitiveTable.trim(),
				  node.thePrimitiveTableKey.trim());
	      }
	}
      else
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG) << "Primitive \""<<node.thePrimitiveTable<<"\" not handled yet" << endl;
         }
      }
    }
}

void rspfVpfAnnotationFeatureInfo::drawAnnotations(rspfRgbImage* tile)
{
   if(theEnabledFlag)
   {
      int idx = 0;
      for(idx = 0; idx < (int)theAnnotationArray.size(); ++idx)
      {
         theAnnotationArray[idx]->draw(*tile);
      }
   }
}

bool rspfVpfAnnotationFeatureInfo::saveState(rspfKeywordlist& kwl,
					      const char* prefix)const
{
  kwl.add(prefix,
	  "name",
	  theName.c_str(),
	  true);
  kwl.add(prefix,
	  "description",
	  theDescription.c_str(),
	  true);
  rspfString penColor = (rspfString::toString((int)thePenColor.getR())+ " " +
			  rspfString::toString((int)thePenColor.getG())+ " " +
			  rspfString::toString((int)thePenColor.getB()));
  rspfString brushColor = (rspfString::toString((int)theBrushColor.getR())+ " " +
			    rspfString::toString((int)theBrushColor.getG())+ " " +
			    rspfString::toString((int)theBrushColor.getB()));
  kwl.add(prefix,
	  "pen_color",
	  penColor,
	  true);
  kwl.add(prefix,
	  "brush_color",
	  brushColor,
	  true);

  if(theFeatureType == rspfVpfAnnotationFeatureType_POINT)
    {
      kwl.add(prefix,
	      "point_radius",
	      rspfString::toString(thePointRadius.x) + " " +
	      rspfString::toString(thePointRadius.y),
	      true);
      kwl.add(prefix,
	      "fill_enabled",
	      theFillEnabledFlag,
	      true);
    }
  else if(theFeatureType == rspfVpfAnnotationFeatureType_LINE)
    {
      kwl.add(prefix,
	      "thickness",
	      theThickness,
	      true);
    }
  else if(theFeatureType == rspfVpfAnnotationFeatureType_POLYGON)
    {
      kwl.add(prefix,
	      "fill_enabled",
	      theFillEnabledFlag,
	      true);
      kwl.add(prefix,
	      "thickness",
	      theThickness,
	      true);
    }
  else if(theFeatureType == rspfVpfAnnotationFeatureType_TEXT)
    {
      theFontInformation.saveState(kwl,
				   (rspfString(prefix)+"font.").c_str());
    }
  kwl.add(prefix,
	  "enabled",
	  theEnabledFlag,
	  true);
  kwl.add(prefix,
	  "feature_type",
	  getFeatureTypeAsString(),
	  true);

  return true;
}

bool rspfVpfAnnotationFeatureInfo::loadState(const rspfKeywordlist& kwl,
					      const char* prefix)
{
  const char* name        = kwl.find(prefix, "name");
  const char* description = kwl.find(prefix, "description");
  const char* enabled     = kwl.find(prefix, "enabled");
  const char* fillEnabled = kwl.find(prefix, "fill_enabled");
  const char* thickness   = kwl.find(prefix, "thickness");
  const char* pointRadius = kwl.find(prefix, "point_radius");
  const char* brushColor  = kwl.find(prefix, "brush_color");
  const char* penColor    = kwl.find(prefix, "pen_color");
  
  theFontInformation.loadState(kwl,
			       (rspfString(prefix)+"font.").c_str());

  if(name)
    {
      if(theName != name)
	{
	  deleteAllObjects();
	}
      theName = name;
    }
  if(description)
    {
      theDescription = description;
    }
  if(enabled)
    {
      theEnabledFlag = rspfString(enabled).toBool();

      if(!theEnabledFlag)
	{
	  deleteAllObjects();
	}
    }
  
  if(fillEnabled)
    {
      theFillEnabledFlag = rspfString(fillEnabled).toBool();
    }
  if(thickness)
    {
      theThickness = rspfString(thickness).toInt();
    }
  if(pointRadius)
    {
      stringstream ptRadiusStream(pointRadius);
      ptRadiusStream >> thePointRadius.x >> thePointRadius.y;
    }
  if(brushColor)
    {
      int r, g, b;
      stringstream colorStream(brushColor);
      colorStream >> r >> g >> b;

      theBrushColor.setR((rspf_uint8)r);
      theBrushColor.setG((rspf_uint8)g);
      theBrushColor.setB((rspf_uint8)b);
    }
  if(penColor)
    {
      int r, g, b;
      stringstream colorStream(brushColor);
      colorStream >> r >> g >> b;

      thePenColor.setR((rspf_uint8)r);
      thePenColor.setG((rspf_uint8)g);
      thePenColor.setB((rspf_uint8)b);
    }
  
  if(theEnabledFlag&&(theAnnotationArray.size() < 1))
    {
      buildFeature();
    }
  else
    {
      setDrawingFeaturesToAnnotation();
    }

  return true;
}

rspfString rspfVpfAnnotationFeatureInfo::getFeatureTypeAsString()const
{
   switch(theFeatureType)
   {
      case rspfVpfAnnotationFeatureType_POINT:
      {
         return "point";
      }
      case rspfVpfAnnotationFeatureType_LINE:
      {
         return "line";
      }
      case rspfVpfAnnotationFeatureType_POLYGON:
      {
         return "polygon";
      }
      case rspfVpfAnnotationFeatureType_TEXT:
      {
         return "text";
      }
      default:
         break;
   }
   
  return "unknown";
}

rspfVpfAnnotationFeatureInfo::rspfVpfAnnotationFeatureType rspfVpfAnnotationFeatureInfo::getFeatureTypeFromString(const rspfString& featureType)const
{
  rspfString downCased = featureType;
  downCased = downCased.downcase();

  if(downCased.contains("point"))
    {
      return rspfVpfAnnotationFeatureType_POINT;
    }
  else if(downCased.contains("line"))
    {
      return rspfVpfAnnotationFeatureType_LINE;
    }
  else if(downCased.contains("polygon"))
    {
      return rspfVpfAnnotationFeatureType_POLYGON;
    }
  else if(downCased.contains("text"))
    {
      return rspfVpfAnnotationFeatureType_TEXT;
    }

  return rspfVpfAnnotationFeatureType_UNKNOWN;

}

void rspfVpfAnnotationFeatureInfo::deleteAllObjects()
{
   theAnnotationArray.clear();
}

void rspfVpfAnnotationFeatureInfo::setDrawingFeaturesToAnnotation()
{
   switch(theFeatureType)
   {
   case rspfVpfAnnotationFeatureType_POINT:
   {
      rspfGeoAnnotationMultiEllipseObject* annotation = 0;
      for(int idx = 0; idx < (int)theAnnotationArray.size();++idx)
      {
         annotation = (rspfGeoAnnotationMultiEllipseObject*)theAnnotationArray[idx].get();

         annotation->setColor(thePenColor.getR(),
                              thePenColor.getG(),
                              thePenColor.getB());
         annotation->setThickness(theThickness);
         annotation->setFillFlag(theFillEnabledFlag);
         annotation->setWidthHeight(thePointRadius);
      }
      
      break;
   }
   case rspfVpfAnnotationFeatureType_TEXT:
   {
      rspfGeoAnnotationFontObject* annotation = 0;
      rspfRefPtr<rspfFont> font = rspfFontFactoryRegistry::instance()->createFont(theFontInformation);

      for(int idx = 0; idx < (int)theAnnotationArray.size();++idx)
      {
         annotation = (rspfGeoAnnotationFontObject*)theAnnotationArray[idx].get();
         annotation->setColor(thePenColor.getR(),
                              thePenColor.getG(),
                              thePenColor.getB());
         annotation->setThickness(theThickness);
         
         if(font.valid())
         {
            annotation->setFont((rspfFont*)font->dup());
         }
         annotation->setPointSize(theFontInformation.thePointSize);
         annotation->setScale(theFontInformation.theScale);
         annotation->setShear(theFontInformation.theShear);
         annotation->setRotation(theFontInformation.theRotation);
      }
      break;
   }
   case rspfVpfAnnotationFeatureType_LINE:
   {
      rspfGeoAnnotationMultiPolyLineObject* annotation = 0;
      for(int idx = 0; idx < (int)theAnnotationArray.size();++idx)
      {
         annotation = (rspfGeoAnnotationMultiPolyLineObject*)theAnnotationArray[idx].get();
         annotation->setColor(thePenColor.getR(),
                              thePenColor.getG(),
                              thePenColor.getB());
         annotation->setThickness(theThickness);
      }
      
      break;
   }
   case rspfVpfAnnotationFeatureType_POLYGON:
   {
      rspfGeoAnnotationMultiPolyObject* annotation = 0;
      for(int idx = 0; idx < (int)theAnnotationArray.size();++idx)
      {
         annotation = (rspfGeoAnnotationMultiPolyObject*)theAnnotationArray[idx].get();
         annotation->setColor(thePenColor.getR(),
                              thePenColor.getG(),
                              thePenColor.getB());
         annotation->setThickness(theThickness);
         annotation->setFillFlag(theFillEnabledFlag);
      }
      break;
   }
   default:
   {
      break;
   }
   }
}


void rspfVpfAnnotationFeatureInfo::buildTxtFeature(const rspfFilename& tableName,
						    const rspfString&   tableKey,
						    const rspfFilename& /* primitiveName */,
						    const rspfString&   /* primitiveKey */)
{
  rspfFilename tableFileName      = theCoverage.getPath().dirCat(tableName);
  rspfFilename primitiveTableName;
  rspfVpfTable table;
  rspfVpfTable primitiveTable;

  if(table.openTable(tableFileName))
    {
      table.reset();
      vector<rspfString> columnValues = table.getColumnValues(tableKey.trim());
      
      primitiveTableName = theCoverage.getPath().dirCat("txt");
      
      vector<rspfString> tileIds;
      rspf_int32 stringValuePosition = 0;
      rspf_int32 shapeLinePosition = 0;
      bool isTiled = false;
      if(table.getColumnPosition("tile_id") >= 0)
	{
	  tileIds = table.getColumnValues("tile_id");
	  isTiled = true;
	}
      if(!isTiled)
	{
	  primitiveTableName = theCoverage.getPath().dirCat("txt");
	  if(!primitiveTable.openTable(primitiveTableName))
	    {
	      return;
	    }
	  stringValuePosition = primitiveTable.getColumnPosition("string");
	  shapeLinePosition   = primitiveTable.getColumnPosition("shape_line");
	}
      rspf_int32 tileId = -1;
      for(rspf_uint32 idx = 0; idx < columnValues.size();++idx)
	{
	  if(isTiled)
	    {
	      if(tileId != tileIds[idx].toInt())
		{
		  tileId = tileIds[idx].toInt();
		  rspfFilename filename = theCoverage.getLibrary()->getTileName(tileIds[idx].toInt());
		  if(theCoverage.getPath().dirCat(filename).dirCat("txt").exists())
		    {
		      primitiveTableName = theCoverage.getPath().dirCat(filename).dirCat("txt");
		    }
		  else if(theCoverage.getPath().dirCat(filename.downcase()).dirCat("txt").exists())
		    {
		      primitiveTableName = theCoverage.getPath().dirCat(filename.downcase()).dirCat("txt");
		    }
		  if(!primitiveTable.openTable(primitiveTableName))
		    {
		      return;
		    }
		  stringValuePosition = primitiveTable.getColumnPosition("string");
		  shapeLinePosition   = primitiveTable.getColumnPosition("shape_line");
		}
	    }
	  row_type row = read_row( columnValues[idx].toInt(), 
				   *primitiveTable.getVpfTableData());
	  rspf_int32 count = 0;
	  rspfDpt* ptArray = getXy(*primitiveTable.getVpfTableData(),
				    row,
				    shapeLinePosition,
				    &count);
	  rspfDpt midPoint;
	  if(ptArray)
	    {
	      midPoint = *ptArray;
	      
	      delete [] ptArray;
	    }
	  rspfString stringValue = primitiveTable.getColumnValueAsString(row,
									  stringValuePosition);
	  free_row(row,  *primitiveTable.getVpfTableData());
	  
	  if(!midPoint.hasNans())
	    {
	      rspfGpt centerPoint(midPoint.lat,
				   midPoint.lon,
				   rspf::nan());
              rspfFont* font = rspfFontFactoryRegistry::instance()->createFont(theFontInformation);
	      rspfGeoAnnotationFontObject* annotation = new rspfGeoAnnotationFontObject(centerPoint,
											  stringValue);
              annotation->setColor(thePenColor.getR(),
                                   thePenColor.getG(),
                                   thePenColor.getB());
              annotation->setThickness(theThickness);
                 
              annotation->setFont(font);
	      annotation->setPointSize(theFontInformation.thePointSize);
	      annotation->setScale(theFontInformation.theScale);
	      annotation->setShear(theFontInformation.theShear);
	      annotation->setRotation(theFontInformation.theRotation);
	      theAnnotationArray.push_back(annotation);
	    }
	}  
    }
}

void rspfVpfAnnotationFeatureInfo::buildEdgFeature(const rspfFilename& tableName,
						    const rspfString&   tableKey,
						    const rspfFilename& /* primitiveName */,
						    const rspfString&   /* primitiveKey */)
{
   rspfFilename tableFileName      = theCoverage.getPath().dirCat(tableName);
   rspfFilename primitiveTableName;
   rspfVpfTable table;
   rspfVpfTable primitiveTable;
  
   std::vector<rspfPolyLine> polyLineArray;

   if(table.openTable(tableFileName))
   {
      
      vector<rspfString> columnValues = table.getColumnValues(tableKey.trim());
      vector<rspfString> tileIds;
      bool isTiled = false;
      if(table.getColumnPosition("tile_id") >= 0)
      {
         tileIds = table.getColumnValues("tile_id");
         isTiled = true;
      }
      rspf_int32 coordinateValuePosition = 0;
      if(!isTiled)
      {
         primitiveTableName = theCoverage.getPath().dirCat("edg");
         if(!primitiveTable.openTable(primitiveTableName))
         {
            return;
         }
         coordinateValuePosition = primitiveTable.getColumnPosition("coordinates");
      }
      rspf_int32 tileId = -1;
      for(rspf_uint32 idx = 0; idx < columnValues.size();++idx)
      {
         if(isTiled)
         {
            if(tileId != tileIds[idx].toInt())
            {
               tileId = tileIds[idx].toInt();
               rspfFilename filename = theCoverage.getLibrary()->getTileName(tileIds[idx].toInt());
               if(theCoverage.getPath().dirCat(filename).dirCat("edg").exists())
               {
                  primitiveTableName = theCoverage.getPath().dirCat(filename).dirCat("edg");
               }
               else if(theCoverage.getPath().dirCat(filename.downcase()).dirCat("edg").exists())
               {
                  primitiveTableName = theCoverage.getPath().dirCat(filename.downcase()).dirCat("edg");
               }
               if(!primitiveTable.openTable(primitiveTableName))
               {
                  return;
               }
               coordinateValuePosition = primitiveTable.getColumnPosition("coordinates");
            }
         }
         rspfPolyLine polyLine;
	  
         readEdge(polyLine,
                  columnValues[idx].toInt(),
                  coordinateValuePosition,
                  primitiveTable);

	 readAttributes(polyLine, table, idx + 1); // third parm is one-based row
	  
         polyLineArray.push_back(polyLine);
      }
   }
   rspfGeoAnnotationMultiPolyLineObject* annotation = new rspfGeoAnnotationMultiPolyLineObject(polyLineArray);
   annotation->setColor(thePenColor.getR(),
                        thePenColor.getG(),
                        thePenColor.getB());
   annotation->setThickness(theThickness);
   theAnnotationArray.push_back(annotation);
}

void rspfVpfAnnotationFeatureInfo::readAttributes(rspfPolyLine& polyLine, rspfVpfTable& table, int row) {
  int numCols = table.getNumberOfColumns();

  for (int col = 0; col < numCols; col ++) {
    polyLine.addAttribute( table.getColumnValueAsString( row, col ));
  }
}

void rspfVpfAnnotationFeatureInfo::buildPointFeature(const rspfString& primitiveName,
						      const rspfFilename& tableName,
						      const rspfString&   tableKey,
						      const rspfFilename& /* primitive */,
						      const rspfString&   /* primitiveKey */)
{
  rspfFilename tableFileName      = theCoverage.getPath().dirCat(tableName);
  rspfFilename primitiveTableName;
  rspfVpfTable table;
  rspfVpfTable primitiveTable;
  
  std::vector<rspfGpt> centerPointArray;

  if(table.openTable(tableFileName))
    {
      
      vector<rspfString> columnValues = table.getColumnValues(tableKey.trim());
      vector<rspfString> tileIds;
      bool isTiled = false;
      if(table.getColumnPosition("tile_id") >= 0)
	{
	  tileIds = table.getColumnValues("tile_id");
	  isTiled = true;
	}
      rspf_int32 coordinateValuePosition = 0;
      if(!isTiled)
	{
	  primitiveTableName = theCoverage.getPath().dirCat(primitiveName);
	  if(!primitiveTable.openTable(primitiveTableName))
	    {
	      return;
	    }
	  coordinateValuePosition = primitiveTable.getColumnPosition("coordinate");
	}
      rspf_int32 tileId = -1;
      for(rspf_uint32 idx = 0; idx < columnValues.size();++idx)
	{
	  if(isTiled)
	    {
	      if(tileId != tileIds[idx].toInt())
		{
		  tileId = tileIds[idx].toInt();
		  rspfFilename filename = theCoverage.getLibrary()->getTileName(tileIds[idx].toInt());
		  if(theCoverage.getPath().dirCat(filename).dirCat(primitiveName).exists())
		    {
		      primitiveTableName = theCoverage.getPath().dirCat(filename).dirCat(primitiveName);
		    }
		  else if(theCoverage.getPath().dirCat(filename.downcase()).dirCat(primitiveName).exists())
		    {
		      primitiveTableName = theCoverage.getPath().dirCat(filename.downcase()).dirCat(primitiveName);
		    }
		  if(!primitiveTable.openTable(primitiveTableName))
		    {
		      return;
		    }
		  coordinateValuePosition = primitiveTable.getColumnPosition("coordinate");
		}
	    }
	  row_type row = read_row( columnValues[idx].toInt(), 
				   *primitiveTable.getVpfTableData());

	  rspf_int32 count = 0;
	  rspfDpt* ptArray = getXy(*primitiveTable.getVpfTableData(),
				    row,
				    coordinateValuePosition,
				    &count);
	  
	  if(ptArray)
	    {
	      for(int i = 0; i < count; ++i)
		{
		  if((fabs(ptArray[i].x) <= 180.0)&&
		     (fabs(ptArray[i].y) <= 90.0))
		    {
		      centerPointArray.push_back(rspfGpt(ptArray[i].lat,
							  ptArray[i].lon,
							  rspf::nan()));
		    }
		}
	      delete [] ptArray;
	    }
	  free_row(row,  *primitiveTable.getVpfTableData());
	}
    }
  rspfGeoAnnotationMultiEllipseObject* annotation = new rspfGeoAnnotationMultiEllipseObject(centerPointArray,
											      rspfDpt(2,2));
  annotation->setColor(thePenColor.getR(),
                       thePenColor.getG(),
                       thePenColor.getB());
  annotation->setThickness(theThickness);
  annotation->setFillFlag(theFillEnabledFlag);
  annotation->setWidthHeight(thePointRadius);
  theAnnotationArray.push_back(annotation);
}

int myFaceCount = 0;

void rspfVpfAnnotationFeatureInfo::buildFaceFeature(const rspfFilename& tableName,
						     const rspfString&   tableKey, // face_id
						     const rspfFilename& /* primitive */, //face
						     const rspfString&   /* primitiveKey */) // id
{
   rspfFilename tableFileName      = theCoverage.getPath().dirCat(tableName);
   rspfFilename primitiveTableName;
   rspfFilename rngTableName;
   rspfFilename edgTableName;
   rspfVpfTable table;
   rspfVpfTable primitiveTable;
   rspfVpfTable rngTable;
   rspfVpfTable edgTable;
   vector<rspfGeoPolygon> thePolyList;
  
   if(table.openTable(tableFileName))
   {
      vector<rspfString> columnValues = table.getColumnValues(tableKey.trim()); // fac_id
      vector<rspfString> tileIds;
      bool isTiled = false;
      if(table.getColumnPosition("tile_id") >= 0)
      {
         tileIds = table.getColumnValues("tile_id");
         isTiled = true;
      }
      // rspf_int32 coordinateValuePosition = 0;
      rspf_int32 startEdgePosition = 0;
      rspf_int32 rngPtrPosition = 0;
      if(!isTiled)
      {
         primitiveTableName = theCoverage.getPath().dirCat("fac");
         rngTableName = theCoverage.getPath().dirCat("rng");
         edgTableName = theCoverage.getPath().dirCat("edg");
         if(!primitiveTable.openTable(primitiveTableName)||
            !rngTable.openTable(rngTableName)||
            !edgTable.openTable(edgTableName))
         {
            return;
         }
         // coordinateValuePosition = edgTable.getColumnPosition("coordinates");
         startEdgePosition = rngTable.getColumnPosition("start_edge");
         rngPtrPosition = primitiveTable.getColumnPosition("ring_ptr");
      } // else will get for each face in loop below

      rspf_int32 tileId = -1;

      for(rspf_uint32 idx = 0; idx < columnValues.size();++idx) // for each face feature
      {
         if(isTiled)		// then get table names because we didn't get them above
         {
            if((tileId != tileIds[idx].toInt())||
               (tileId < 0))
            {
               tileId = tileIds[idx].toInt();
               rspfFilename filename = theCoverage.getLibrary()->getTileName(tileIds[idx].toInt());
               if(theCoverage.getPath().dirCat(filename).dirCat("fac").exists())
               {
                  primitiveTableName = theCoverage.getPath().dirCat(filename).dirCat("fac");
                  rngTableName = theCoverage.getPath().dirCat(filename).dirCat("rng");
                  edgTableName = theCoverage.getPath().dirCat(filename).dirCat("edg");
               }
               else if(theCoverage.getPath().dirCat(filename.downcase()).dirCat("fac").exists())
               {
                  primitiveTableName = theCoverage.getPath().dirCat(filename.downcase()).dirCat("fac");
                  rngTableName = theCoverage.getPath().dirCat(filename.downcase()).dirCat("rng");
                  edgTableName = theCoverage.getPath().dirCat(filename.downcase()).dirCat("edg");
               }
               if(!primitiveTable.openTable(primitiveTableName)||
                  !rngTable.openTable(rngTableName)||
                  !edgTable.openTable(edgTableName))
               {
                  return;
               }
               // coordinateValuePosition = edgTable.getColumnPosition("coordinates");
               startEdgePosition = rngTable.getColumnPosition("start_edge");
               rngPtrPosition = primitiveTable.getColumnPosition("ring_ptr");
            }
         } // if(isTiled)

         // get the outer ring id for this face
         int thisFaceId = columnValues[idx].toInt();
         int rngId = -1;

         if (thisFaceId <= primitiveTable.getNumberOfRows())
	    rngId = readRngId(thisFaceId,
			      rngPtrPosition,
			      primitiveTable);           // face table
         else
            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG) << "not getting face " << thisFaceId << " from tile " << tileId << " " << primitiveTableName <<
                  " bacause it has only " << primitiveTable.getNumberOfRows() << " rows" << endl;
            }

         if(rngId > 0)
         {
            int startEdge = readStartEdgeId(rngId,
                                            startEdgePosition,
                                            rngTable);
            int outerStartEdge = startEdge;
            if(startEdge > 0 )
            {
               rspfGeoPolygon polygon;
                 
               myFaceCount ++;
                 
               int ringTableFaceIdColumn = rngTable.getColumnPosition("face_id");
               int ringFaceId = readTableCellAsInt(rngId, ringTableFaceIdColumn, rngTable);
                 
               if (thisFaceId != ringFaceId) {
                  // as of Mon Dec 20 2004, this has not been observed in vmap0 or vmap1
                  if(traceDebug())
                  {
                     rspfNotify(rspfNotifyLevel_DEBUG) << "rejecting face " << thisFaceId << " from " << tableFileName <<
                        " because it's ring (" << rngId << ") has face " << ringFaceId << " and startEdgeId " << startEdge << endl;
                  }
               } else {
                  readGeoPolygon( polygon,
                                  thisFaceId,
                                  startEdge,
                                  edgTable );
                    
                  // get the inner rings (holes)
                  int faceIdPosition = rngTable.getColumnPosition("face_id");
                  int innerRingOffset = 1;
                  int rowFaceId = -1;
                  if (rngId + innerRingOffset <= rngTable.getNumberOfRows())
                     rowFaceId = rngTable.getColumnValueAsString( rngId + innerRingOffset, faceIdPosition ).toInt();
                  while (rowFaceId == thisFaceId && rngId + innerRingOffset <= rngTable.getNumberOfRows()) {	// while there are more inner rings
                     startEdge = readStartEdgeId(rngId + innerRingOffset, startEdgePosition, rngTable);

                     if (startEdge == outerStartEdge) {
			// as of Mon Dec 20 2004, this has not been observed in vmap0 or vmap1
                        if(traceDebug())
                        {
                           rspfNotify(rspfNotifyLevel_DEBUG) << "rejecting hole in face "
                                                               << thisFaceId << " because inner ring "
                                                               << rngId + innerRingOffset
                                                               << " touches outer ring " << rngId << endl;
                        }

			innerRingOffset ++;
			rowFaceId = rngTable.getColumnValueAsString( rngId + innerRingOffset, faceIdPosition ).toInt();
			continue;
                     }

                     if(startEdge > 0 )
                     {
                        rspfGeoPolygon holePolygon;
		  
                        readGeoPolygon( holePolygon,
                                        thisFaceId,
                                        startEdge,
                                        edgTable );
                        if(holePolygon.size())
                           polygon.addHole( holePolygon );
                     }

                     innerRingOffset ++;
                     if (rngId + innerRingOffset <= rngTable.getNumberOfRows())
                        rowFaceId = rngTable.getColumnValueAsString( rngId + innerRingOffset, faceIdPosition ).toInt();
                  }

                  readAttributes(polygon, table, idx + 1); // third parm is one-based row

                  thePolyList.push_back(polygon);
               }
            }
         }
      } // each fac_id
   }
   rspfGeoAnnotationMultiPolyObject* annotation = new rspfGeoAnnotationMultiPolyObject(thePolyList);
   annotation->setColor(thePenColor.getR(),
                        thePenColor.getG(),
                        thePenColor.getB());
   annotation->setThickness(theThickness);
   annotation->setFillFlag(theFillEnabledFlag);
  
   theAnnotationArray.push_back(annotation);
}

void rspfVpfAnnotationFeatureInfo::readAttributes(rspfGeoPolygon& polygon, rspfVpfTable& table, int row) {
  int numCols = table.getNumberOfColumns();

  for (int col = 0; col < numCols; col ++) {
    rspfString s = table.getColumnValueAsString( row, col );
    polygon.addAttribute( s );
  }
}

/* GET_XY                                                                    */
/*****************************************************************************/
rspfDpt *rspfVpfAnnotationFeatureInfo::getXy(vpf_table_type table, 
					       row_type row, 
					       rspf_int32 pos, 
					       rspf_int32 *count)
{
   rspf_int32 i;
   rspfDpt *coord = 0;
  
   switch (table.header[pos].type)
   {
      case 'C':
      {
         coordinate_type temp, *ptr;
         ptr = (coordinate_type*)get_table_element(pos, row, table, &temp, count);
         coord = new rspfDpt[*count];
         if ((*count == 1) && (ptr == (coordinate_type*)0))
         {
            coord->x = (double)temp.x;
            coord->y = (double)temp.y;
         }
         else 
         {
	    for (i=0; i<*count; i++)
            {
               coord[i].x = (double)ptr[i].x;
               coord[i].y = (double)ptr[i].y;
            }
         }
         if (ptr)
         {
            free((char *)ptr);
         }
         break;
      }
      case 'Z':
      {
         tri_coordinate_type temp, *ptr;
         ptr = (tri_coordinate_type*)get_table_element (pos, row, table, &temp, count);
	 coord = new rspfDpt[*count];
         if ((*count == 1) && (ptr == (tri_coordinate_type*)0))
         {
            coord->x = (double)temp.x;
            coord->y = (double)temp.y;
         }
         else
         {
            for (i=0; i<*count; i++)
            {
               coord[i].x = (double)ptr[i].x;
               coord[i].y = (double)ptr[i].y;
            }
         }
         if (ptr)
         {
            free ((char*)ptr);
         }
         break;
      }
      case 'B':
      {
         double_coordinate_type temp, *ptr;
         ptr = (double_coordinate_type*)get_table_element (pos, row, table, &temp, count);
         coord = new rspfDpt[*count];
         if ((*count == 1) && (ptr == (double_coordinate_type*)0))
         {
            coord->x = temp.x;
            coord->y = temp.y;
         }
         else
         {
	    for (i=0; i<*count; i++)
            {
               coord[i].x = ptr[i].x;
               coord[i].y = ptr[i].y;
            }
         }
         if (ptr)
         {
	    free ((char*)ptr);
         }
         break;
      }
      case 'Y':
      {
         double_tri_coordinate_type temp, *ptr;
         ptr = (double_tri_coordinate_type*)get_table_element (pos, row, table, &temp, count);
         coord = new rspfDpt[*count];
         if ((*count == 1) && (ptr == (double_tri_coordinate_type*)0))
         {
            coord->x = temp.x;
            coord->y = temp.y;
         }
         else
         {
            for (i=0; i<*count; i++)
            {
               coord[i].x = ptr[i].x;
               coord[i].y = ptr[i].y;
            }
         }
         if (ptr)
         {
	    free((char*)ptr);
         }
         break;
      }
      
      default:
         break;
   } /* switch type */
   return (coord);
}

int rspfVpfAnnotationFeatureInfo::readTableCellAsInt (int rowNumber,
						       int colNumber,
						       rspfVpfTable& table)
{
  int result = -1;
  row_type row = read_row( rowNumber, *table.getVpfTableData());
  
  result = table.getColumnValueAsString(row, colNumber).toInt();
  
  free_row(row, *table.getVpfTableData());
  
  return result;
}

int rspfVpfAnnotationFeatureInfo::readRngId(int rowNumber,
					     int colNumber,
					     rspfVpfTable& faceTable)
{
  int idResult = -1;
  row_type row = read_row( rowNumber, 
			   *faceTable.getVpfTableData());
  
  idResult = faceTable.getColumnValueAsString(row,
					      colNumber).toInt();
  
  free_row(row, *faceTable.getVpfTableData());
  
  return idResult;
}

int rspfVpfAnnotationFeatureInfo::readStartEdgeId(int rowNumber,
						   int colNumber,
						   rspfVpfTable& rngTable)
{
  int idResult = -1;

  row_type row = read_row( rowNumber, 
			   *rngTable.getVpfTableData());
  
  idResult = rngTable.getColumnValueAsString(row,
					     colNumber).toInt();
  
  free_row(row, *rngTable.getVpfTableData());
  
  return idResult;
  
}

int rspfVpfAnnotationFeatureInfo::getEdgeKeyId (vpf_table_type& table, row_type& row, int col) {
    id_triplet_type key;
    rspf_int32 keyCount;
    get_table_element( col,
		       row,
		       table,
		       & key,
		       & keyCount );
    return key.id;
}

void rspfVpfAnnotationFeatureInfo::readGeoPolygon(rspfGeoPolygon& polygon,
						   int faceId,
						   int startEdgeId,
						   rspfVpfTable& edgTable) {

  rspf_int32 coordinatesCol = edgTable.getColumnPosition( "coordinates" );
  rspf_int32 startNodeCol = edgTable.getColumnPosition( "start_node" );
  rspf_int32 endNodeCol = edgTable.getColumnPosition( "end_node" );
  rspf_int32 rightEdgeCol = edgTable.getColumnPosition( "right_edge" );
  rspf_int32 leftEdgeCol = edgTable.getColumnPosition( "left_edge" );
  rspf_int32 rightFaceCol = edgTable.getColumnPosition( "right_face" );
  rspf_int32 leftFaceCol = edgTable.getColumnPosition( "left_face" );

  // collect edges
  vector < int > edges;
  int lastEdge = startEdgeId;
  edges.push_back( lastEdge );

  row_type row = read_row( startEdgeId, *edgTable.getVpfTableData() );
  int startNode = edgTable.getColumnValueAsString( row, startNodeCol ).toInt();
  int endNode = edgTable.getColumnValueAsString( row, endNodeCol ).toInt();

  if (startNode != endNode) { // there's more than one edge to this ring
    int rightFace = getEdgeKeyId( *edgTable.getVpfTableData(), row, rightFaceCol );
    int leftFace = getEdgeKeyId( *edgTable.getVpfTableData(), row, leftFaceCol );
    int rightEdge = getEdgeKeyId( *edgTable.getVpfTableData(), row, rightEdgeCol );
    int leftEdge = getEdgeKeyId( *edgTable.getVpfTableData(), row, leftEdgeCol );
    int nextEdge;
    int firstNode, lastNode;

    if (rightFace == leftFace) {
      if (rightEdge == leftEdge) {
	free_row(row,  *edgTable.getVpfTableData());
	// as of Mon Dec 20 2004, this has not been observed in vmap0 or vmap1
	// cout << "rejecting floating line face " << faceId << " line " << __LINE__ << endl;
	return;
      }

      // this is a dangling start edge; find a non dangling edge to use as the start edge
      int dirRight = 1;
      int nextEdge = startEdgeId;
      int nextLeftFace = leftFace;
      int nextRightFace = rightFace;
      int mobiusLimit = 1000;

      while (nextLeftFace == nextRightFace) {
	if (-- mobiusLimit < 0) {
	    free_row(row,  *edgTable.getVpfTableData());
            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG) << "rejecting mobius face " << faceId << " line " << __LINE__ << endl;
            }
	    return;
	}

	int thisEdge = nextEdge;
	if (dirRight) {
	  nextEdge = getEdgeKeyId( *edgTable.getVpfTableData(), row, rightEdgeCol );
	  if (nextEdge == thisEdge)
	    dirRight = 0;
	  else if (nextEdge == startEdgeId) {
	    free_row(row,  *edgTable.getVpfTableData());
            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG) << "rejecting mobius face " << faceId << " line " << __LINE__ << endl;
            }
	    return;
	  }
	} else {
	  nextEdge = getEdgeKeyId( *edgTable.getVpfTableData(), row, leftEdgeCol );
	  if (nextEdge == thisEdge) {
	    free_row(row,  *edgTable.getVpfTableData());
	    // as of Mon Dec 20 2004, this has not been observed in vmap0 or vmap1
	    // cout << "rejecting multi floating line face " << faceId << " line " << __LINE__ << endl;
	    return;
	  } else if (nextEdge == startEdgeId) {
	    free_row(row,  *edgTable.getVpfTableData());
            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG) << "rejecting mobius face " << faceId << " line " << __LINE__ << endl;
            }
	    return;
	  }
	}
	free_row(row,  *edgTable.getVpfTableData());
	row = read_row( nextEdge, *edgTable.getVpfTableData() );
	nextRightFace = getEdgeKeyId( *edgTable.getVpfTableData(), row, rightFaceCol );
	nextLeftFace = getEdgeKeyId( *edgTable.getVpfTableData(), row, leftFaceCol );
      }

      startEdgeId = nextEdge;
      rightFace = nextRightFace;
      leftFace = nextLeftFace;
      free_row(row,  *edgTable.getVpfTableData());
      row = read_row( startEdgeId, *edgTable.getVpfTableData() );
      startNode = edgTable.getColumnValueAsString( row, startNodeCol ).toInt();
      endNode = edgTable.getColumnValueAsString( row, endNodeCol ).toInt();
    }

    if (rightFace == faceId) {
      nextEdge = getEdgeKeyId( *edgTable.getVpfTableData(), row, rightEdgeCol );
      firstNode = startNode;
      lastNode = endNode;
    } else if (leftFace == faceId) {
      nextEdge = getEdgeKeyId( *edgTable.getVpfTableData(), row, leftEdgeCol );
      firstNode = endNode;
      lastNode = startNode;
    }
    else
    {
       if(traceDebug())
       {
          rspfNotify(rspfNotifyLevel_DEBUG) << "rejecting face " << faceId << " line " << __LINE__ << endl;
       }
       return;
    }

    row_type nextRow;
    while (true) {
      nextRow = read_row( nextEdge, *edgTable.getVpfTableData() );
      int nextLeftEdge = getEdgeKeyId( *edgTable.getVpfTableData(), nextRow, leftEdgeCol );
      int nextRightEdge = getEdgeKeyId( *edgTable.getVpfTableData(), nextRow, rightEdgeCol );
      int nextLeftFace = getEdgeKeyId( *edgTable.getVpfTableData(), nextRow, leftFaceCol );
      int nextRightFace = getEdgeKeyId( *edgTable.getVpfTableData(), nextRow, rightFaceCol );
      int nextStartNode = edgTable.getColumnValueAsString( nextRow, startNodeCol ).toInt();
      int nextEndNode = edgTable.getColumnValueAsString( nextRow, endNodeCol ).toInt();

      while (nextLeftFace == nextRightFace) {
	int lastEnd;
	if (rightFace == faceId)
	  lastEnd = edgTable.getColumnValueAsString( row, endNodeCol ).toInt();
	else
	  lastEnd = edgTable.getColumnValueAsString( row, startNodeCol ).toInt();

	if (lastEnd == nextStartNode)
	  nextEdge = nextLeftEdge;
	else if (lastEnd == nextEndNode)
	  nextEdge = nextRightEdge;
	else {			// bad face
           if(traceDebug())
           {
              rspfNotify(rspfNotifyLevel_DEBUG) << "rejecting face " << faceId << " line " << __LINE__ << endl;
           }
	  return;
	}
	if (lastEdge == nextEdge)
        { // bad face
           if(traceDebug())
           {
              rspfNotify(rspfNotifyLevel_DEBUG) << "rejecting face " << faceId << " line " << __LINE__ << endl;
           }
           return;
	}

	free_row(nextRow,  *edgTable.getVpfTableData());
	nextRow = read_row( nextEdge, *edgTable.getVpfTableData() );
	nextLeftEdge = getEdgeKeyId( *edgTable.getVpfTableData(), nextRow, leftEdgeCol );
	nextRightEdge = getEdgeKeyId( *edgTable.getVpfTableData(), nextRow, rightEdgeCol );
	nextLeftFace = getEdgeKeyId( *edgTable.getVpfTableData(), nextRow, leftFaceCol );
	nextRightFace = getEdgeKeyId( *edgTable.getVpfTableData(), nextRow, rightFaceCol );
	nextStartNode = edgTable.getColumnValueAsString( nextRow, startNodeCol ).toInt();
	nextEndNode = edgTable.getColumnValueAsString( nextRow, endNodeCol ).toInt();
      }
	
      lastEdge = nextEdge;
      edges.push_back( lastEdge );

      {
	int lastEnd;
	if (nextRightFace == faceId) {
	  if (lastNode != nextStartNode) {
             if(traceDebug())
             {
                rspfNotify(rspfNotifyLevel_DEBUG) << "rejecting face " << faceId << " line " << __LINE__ << endl;
             }
	    return;
	  }
	  lastEnd = nextEndNode;
	} else if (nextLeftFace == faceId) {
	  if (lastNode != nextEndNode) {
             if(traceDebug())
             {
                rspfNotify(rspfNotifyLevel_DEBUG) << "rejecting face " << faceId << " line " << __LINE__ << endl;
             }
	    return;
	  }
	  lastEnd = nextStartNode;
	} else {
           if(traceDebug())
           {
              rspfNotify(rspfNotifyLevel_DEBUG) << "rejecting face " << faceId << " line " << __LINE__ << endl;
           }
	  return;
	}

	if (lastEnd == firstNode)
	  break;
      }

      free_row(row,  *edgTable.getVpfTableData());
      row = nextRow;
      startNode = nextStartNode;
      endNode = nextEndNode;
      rightFace = nextRightFace;
      leftFace = nextLeftFace;

      if (rightFace == faceId) {
	nextEdge = nextRightEdge;
	lastNode = endNode;
      } else {
	nextEdge = nextLeftEdge;
	lastNode = startNode;
      }
    }

    free_row(nextRow,  *edgTable.getVpfTableData());
    nextRow = 0;
  }

  for (vector< int >::iterator i = edges.begin(); i != edges.end(); i++) {
    rspf_int32 count = 0;
    free_row(row,  *edgTable.getVpfTableData());
    row = read_row( *i, *edgTable.getVpfTableData() );
    rspfDpt* ptArray = getXy(*edgTable.getVpfTableData(),
			      row,
			      coordinatesCol,
			      &count);
  
    if(ptArray)
    {
       int rightFace = getEdgeKeyId( *edgTable.getVpfTableData(), row, rightFaceCol );
       
       if (rightFace == faceId)
       {
	  for(int p = 0; p < count; ++p)
          {
             if((fabs(ptArray[p].x) <= 180.0)&&
                (fabs(ptArray[p].y) <= 90.0))
             {
                polygon.addPoint(ptArray[p].y, ptArray[p].x);
             }
          }
       }
       else
       {
	  for(int p = count - 1; p >= 0; --p)
          {
             if((fabs(ptArray[p].x) <= 180.0)&&
                (fabs(ptArray[p].y) <= 90.0))
             {
                polygon.addPoint(ptArray[p].y, ptArray[p].x);
             }
          }
       }
       delete [] ptArray;
    }
  }
  free_row(row,  *edgTable.getVpfTableData());
}

void rspfVpfAnnotationFeatureInfo::readEdge(rspfPolyLine& polyLine,
					     int rowNumber,
					     int colPosition,
					     rspfVpfTable& edgeTable)
{
  polyLine.clear();
  row_type row = read_row( rowNumber, 
			   *edgeTable.getVpfTableData());

  rspf_int32 count = 0;
  rspfDpt* ptArray = getXy(*edgeTable.getVpfTableData(),
			    row,
			    colPosition,
			    &count);
  
  if(ptArray)
    {
      for(int i = 0; i < count; ++i)
	{
	  if((fabs(ptArray[i].x) <= 180.0)&&
	     (fabs(ptArray[i].y) <= 90.0))
	    {
	      polyLine.addPoint(rspfDpt(ptArray[i].x,
					 ptArray[i].y));
	    }
	}
      delete [] ptArray;
    }
  free_row(row,  *edgeTable.getVpfTableData());
}


