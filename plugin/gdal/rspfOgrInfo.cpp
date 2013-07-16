#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <rspfOgrInfo.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfKeywordlist.h>
static rspfTrace traceDebug("rspfOgrInfo:debug");
static rspfTrace traceDump("rspfOgrInfo:dump"); // This will dump offsets.
rspfString getKeyValue(rspfString metaPrefix,
                        rspfString prefix,
                        rspfString metaNameValue,
                        rspfKeywordlist& kwl)
{
  if (!metaNameValue.contains("{") && !metaNameValue.contains("}"))
  {
    std::vector<int> indexVector;
    rspfString name = metaNameValue.split(":")[0].downcase().trim().substitute(" ", "_", true);
    rspfString keyValue = rspfString(metaPrefix + prefix + name);
    std::vector<rspfString> allMatchKeys = kwl.findAllKeysThatMatch(keyValue);
    if (allMatchKeys.size() == 0)
    {
      return name;
    }
    else
    {
      for (rspf_uint32 i = 0; i < allMatchKeys.size(); i++)
      {
        rspfString keyMatchValue = allMatchKeys[i];
        rspfString intValue = keyMatchValue.after(keyValue);
        if (!intValue.empty())
        {
          indexVector.push_back(intValue.toInt());
        }
      }
      if (indexVector.size() == 0) //only found one entry, e.g vpf.cat.Coverage_name
      {
        const char* tmpValue = kwl.find(keyValue);
        rspfString metaValue = tmpValue;
        kwl.remove(keyValue);
        rspfString newPrefix = rspfString(prefix + name + rspfString::toString(0));
        kwl.add(metaPrefix, 
                newPrefix,
                metaValue,
                true);
        return rspfString(name + rspfString::toString(1));
      }
      else // e.g vpf.cat.Coverage_name0 found
      {
        double max = 0; 
        for (rspf_uint32 i = 0; i < indexVector.size(); i++)
        {
          if (max < indexVector.at(i)) 
          {
            max = indexVector.at(i);
          }
        }
        return rspfString(name + rspfString::toString(max+1));
      }
    }
  }
  return rspfString("");
}
rspfOgrInfo::rspfOgrInfo()
   : rspfInfoBase(),
     theFile(),
     ogrDatasource(0),
     ogrDriver(0)
{
}
rspfOgrInfo::~rspfOgrInfo()
{
  if (ogrDatasource != NULL)
  {
    OGRDataSource::DestroyDataSource(ogrDatasource);
    ogrDatasource = 0;
  }
}
bool rspfOgrInfo::open(const rspfFilename& file)
{
   theFile = file;
   ogrDatasource = OGRSFDriverRegistrar::Open(file.c_str(), false, &ogrDriver);
 
   if (ogrDatasource == NULL)
   {
     return false;
   }
   return true;
}
void rspfOgrInfo::parseMetadata(rspfString metaData, 
                                 rspfKeywordlist& kwl, 
                                 rspfString metaPrefix) const
{
   std::vector<rspfString> metaList = metaData.split("\n");
   bool isCat = false;	
   bool isLat = false;
   bool isDht = false;
   bool isLht = false;
   bool isGrt = false;
   bool isDqt = false;
   bool isFcs = false;
   bool isFca = false;
   rspfString catKeyPrefix = "cat.";
   rspfString latKeyPrefix = "lat.";
   rspfString dhtKeyPrefix = "dht.";
   rspfString lhtKeyPrefix = "lht.";
   rspfString grtKeyPrefix = "grt.";
   rspfString dqtKeyPrefix = "dqt.";
   rspfString fcsKeyPrefix = "fcs.";
   rspfString fcaKeyPrefix = "fca.";
   
   for (rspf_uint32 i = 0; i < metaList.size(); i++)
   {
     rspfString metaNameValue = metaList[i].trim();
     rspfString metaStr = metaList[i].downcase().trim();
     if (metaStr.contains("(cat)"))
     {
       isCat = true;
       isLat = false; //set others to false
       isDht = false;
       isLht = false;
       isGrt = false;
       isDqt = false;
       isFcs = false;
       isFca = false;
       continue;
     }
     if (isCat)
     {
       if (metaStr.contains(":") && 
           !metaStr.contains("(lat)") && 
           !metaStr.contains("(dht)") && 
           !metaStr.contains("(lht)") && 
           !metaStr.contains("(grt)") && 
           !metaStr.contains("(dqt)") && 
           !metaStr.contains("(fcs)") && 
           !metaStr.contains("} } }  { family"))
       {
         rspfString keyValue = getKeyValue(metaPrefix, catKeyPrefix, 
                                            metaNameValue, kwl);
         rspfString metaValue = metaNameValue.split(":")[1].trim();
         rspfString prefix = rspfString(catKeyPrefix + keyValue);
         kwl.add(metaPrefix,
           prefix,
           metaValue,
           false);
       }
     }
     if (metaStr.contains("(lat)"))
     {
       isLat = true;
       isCat = false; //set others to false
       isDht = false;
       isLht = false;
       isGrt = false;
       isDqt = false;
       isFcs = false;
       isFca = false;
       continue;
     }
     if (isLat)
     {
       if (metaStr.contains(":") && 
         !metaStr.contains("(cat)") && 
         !metaStr.contains("(dht)") && 
         !metaStr.contains("(lht)") && 
         !metaStr.contains("(grt)") && 
         !metaStr.contains("(dqt)") && 
         !metaStr.contains("(fcs)") && 
         !metaStr.contains("} } }  { family"))
       {
       }
     }
     if (metaStr.contains("(dht)"))
     {
       isDht = true;
       isCat = false;
       isLat = false;
       isLht = false;
       isGrt = false;
       isDqt = false;
       isFcs = false;
       isFca = false;
       continue;
     }
     if (isDht)
     {
       if (metaStr.contains(":") && 
         !metaStr.contains("(lat)") && 
         !metaStr.contains("(cat)") && 
         !metaStr.contains("(lht)") && 
         !metaStr.contains("(grt)") && 
         !metaStr.contains("(dqt)") && 
         !metaStr.contains("(fcs)") && 
         !metaStr.contains("} } }  { family"))
       {
         rspfString keyValue = getKeyValue(metaPrefix, dhtKeyPrefix, 
                                            metaNameValue, kwl);
         rspfString metaValue = metaNameValue.split(":")[1].trim();
         rspfString prefix = rspfString(dhtKeyPrefix + keyValue);
         kwl.add(metaPrefix,
           prefix,
           metaValue,
           false);
       }
     }
     if (metaStr.contains("(lht)"))
     {
       isLht = true;
       isDht = false;
       isCat = false;
       isLat = false;
       isGrt = false;
       isDqt = false;
       isFcs = false;
       isFca = false;
       continue;
     }
     if (isLht)
     {
       if (metaStr.contains(":") && 
         !metaStr.contains("(lat)") && 
         !metaStr.contains("(dht)") && 
         !metaStr.contains("(cat)") && 
         !metaStr.contains("(grt)") && 
         !metaStr.contains("(dqt)") && 
         !metaStr.contains("(fcs)") && 
         !metaStr.contains("} } }  { family"))
       {
         rspfString keyValue = getKeyValue(metaPrefix, lhtKeyPrefix, 
                                            metaNameValue, kwl);
         rspfString metaValue = metaNameValue.split(":")[1].trim();
         rspfString prefix = rspfString(lhtKeyPrefix + keyValue);
         kwl.add(metaPrefix,
           prefix,
           metaValue,
           false);
       }
     }
     if (metaStr.contains("(grt)"))
     {
       isGrt = true;
       isLht = false;
       isDht = false;
       isCat = false;
       isLat = false;
       isDqt = false;
       isFcs = false;
       isFca = false;
       continue;
     }
     if (isGrt)
     {
       if (metaStr.contains(":") && 
         !metaStr.contains("(lat)") && 
         !metaStr.contains("(dht)") && 
         !metaStr.contains("(lht)") && 
         !metaStr.contains("(cat)") && 
         !metaStr.contains("(dqt)") && 
         !metaStr.contains("(fcs)") && 
         !metaStr.contains("} } }  { family"))
       {
         rspfString keyValue = getKeyValue(metaPrefix, grtKeyPrefix, 
                                            metaNameValue, kwl);
         rspfString metaValue = metaNameValue.split(":")[1].trim();
         rspfString prefix = rspfString(grtKeyPrefix + keyValue);
         kwl.add(metaPrefix,
           prefix,
           metaValue,
           false);
       }
     }
     if (metaStr.contains("(dqt)"))
     {
       isDqt = true;
       isGrt = false;
       isLht = false;
       isDht = false;
       isCat = false;
       isLat = false;
       isFcs = false;
       isFca = false;
       continue;
     }
     if (isDqt)
     {
       if (metaStr.contains(":") && 
         !metaStr.contains("(lat)") && 
         !metaStr.contains("(dht)") && 
         !metaStr.contains("(lht)") && 
         !metaStr.contains("(grt)") && 
         !metaStr.contains("(cat)") && 
         !metaStr.contains("(fcs)") && 
         !metaStr.contains("} } }  { family"))
       {
         rspfString keyValue = getKeyValue(metaPrefix, dqtKeyPrefix, 
                                            metaNameValue, kwl);
         rspfString metaValue = metaNameValue.split(":")[1].trim();
         rspfString prefix = rspfString(dqtKeyPrefix + keyValue);
         kwl.add(metaPrefix,
           prefix,
           metaValue,
           false);
       }
     }
     if (metaStr.contains("(fcs)"))
     {
       isFcs = true;
       isDqt = false;
       isGrt = false;
       isLht = false;
       isDht = false;
       isCat = false;
       isLat = false;
       isFca = false;
       continue;
     }
     if (isFcs)
     {
       if (metaStr.contains(":") && 
         !metaStr.contains("(lat)") && 
         !metaStr.contains("(dht)") && 
         !metaStr.contains("(lht)") && 
         !metaStr.contains("(grt)") && 
         !metaStr.contains("(dqt)") && 
         !metaStr.contains("(cat)") && 
         !metaStr.contains("} } }  { family"))
       {
         rspfString keyValue = getKeyValue(metaPrefix, fcsKeyPrefix, 
                                            metaNameValue, kwl);
         rspfString metaValue = metaNameValue.split(":")[1].trim();
         rspfString prefix = rspfString(fcsKeyPrefix + keyValue);
         kwl.add(metaPrefix,
           prefix,
           metaValue,
           false);
       }
     }
     if (metaStr.contains("} } }  { family"))
     {
       isFca = true;
       isDqt = false;
       isGrt = false;
       isLht = false;
       isDht = false;
       isCat = false;
       isLat = false;
       isFcs = false;
     }
     if (isFca)
     {
       if (!metaStr.contains("(lat)") && 
         !metaStr.contains("(dht)") && 
         !metaStr.contains("(lht)") && 
         !metaStr.contains("(grt)") && 
         !metaStr.contains("(dqt)") && 
         !metaStr.contains("(fcs)"))
       {
         std::vector<rspfString> fcaTmpVector = metaNameValue.split("}}");
         rspfString fcaClassName;
         for (rspf_uint32 i = 0; i < fcaTmpVector.size(); i++)
         {
           rspfString fcaTemp = fcaTmpVector[i].trim();
           if (!fcaTemp.empty())
           {
             if (fcaTemp.contains("family"))
             {
               if (fcaTemp.split(" ").size() > 1)
               {
                 fcaClassName = fcaTemp.split(" ")[2].trim();
               }
             }
             else
             {
               std::vector<rspfString> fcaValues;
               if (fcaTemp.contains("<Grassland>displaymetadata {"))
               {
                 rspfString displaymetadataVector = fcaTemp.after("<Grassland>displaymetadata {").trim();
                 if (!displaymetadataVector.empty())
                 {
                   fcaValues = displaymetadataVector.split("{");
                 }
               }
               else
               {
                 fcaValues = fcaTemp.split("{");
               }
               if (fcaValues.size() > 2)
               {
                 rspfString fcaKey = fcaValues[1].trim();
                 rspfString fcaValue = fcaValues[2].trim();
                 rspfString prefix = rspfString(fcaKeyPrefix + fcaClassName + "." + fcaKey);
                 kwl.add(metaPrefix,
                   prefix,
                   fcaValue,
                   false);
               }
             }
           }          
         }
       }
     }
   }
}
bool rspfOgrInfo::getKeywordlist(rspfKeywordlist& kwl) const
{  
  if (ogrDatasource != NULL)  
  {
    rspfString driverName = getDriverName(rspfString(GDALGetDriverShortName(ogrDriver)).downcase());
    rspfString metaPrefix = rspfString(driverName + ".");
    rspfString strValue;
    char** metaData = 0; // tmp drb ogrDatasource->GetMetadata("metadata");
    rspfString keyName = "tableinfo";
    size_t nLen = strlen(keyName.c_str());
    if (metaData != NULL && driverName == "vpf")
    {
      while(*metaData != NULL)
      {
        if (EQUALN(*metaData, keyName.c_str(), nLen)
          && ( (*metaData)[nLen] == '=' || 
          (*metaData)[nLen] == ':' ) )
        {
          strValue = (*metaData)+nLen+1;
        }
        metaData++;
      }
      if (!strValue.empty())
     {
        parseMetadata(strValue, kwl, metaPrefix);
      }
   }
    rspfString geomType;
    int layerCount = ogrDatasource->GetLayerCount();
    rspfString prefixInt = rspfString(metaPrefix + "layer");
    for(int i = 0; i < layerCount; ++i)
    {
      rspfString prefix = prefixInt + rspfString::toString(i) + ".";
      rspfString specialPrefix = "layer" + rspfString::toString(i) + ".";
      OGRLayer* layer = ogrDatasource->GetLayer(i);
      if(layer)
      {
        int featureCount = layer->GetFeatureCount();
        const char* layerName = layer->GetLayerDefn()->GetName();
        OGRFeature* feature = layer->GetFeature(0);
        if(feature)
        {
          OGRGeometry* geom = feature->GetGeometryRef(); 
          if(geom)
          {
            switch(geom->getGeometryType())
            {
            case wkbMultiPoint:
            case wkbMultiPoint25D:
              {
                geomType = "Multi Points";
                break;
              }
            case wkbPolygon25D:
            case wkbPolygon:
              {
                geomType = "Polygon";
                break;
              }
            case wkbLineString25D:
            case wkbLineString:
              {
                geomType = "LineString";
                break;
              }
            case wkbPoint:
            case wkbPoint25D:
              {
                geomType = "Points";
                break;
              }
            case wkbMultiPolygon25D:
            case wkbMultiPolygon:
              {
                geomType = "Multi Polygon";
                break;
              }
            default:
              {
                geomType = "Unknown Type";
                break;
              }
            }
          }
        }//end if feature
        OGRFeature::DestroyFeature(feature);
        kwl.add(specialPrefix,
          "features",
          featureCount,
          true);
        OGRFeatureDefn* featureDefn = layer->GetLayerDefn();
        if (featureDefn)
        {
          for(int iField = 0; iField < featureDefn->GetFieldCount(); iField++ )
          {
            OGRFieldDefn* fieldDefn = featureDefn->GetFieldDefn(iField);
            rspfString fieldType;
            if (fieldDefn)
            {
              rspfString fieldName = rspfString(fieldDefn->GetNameRef());
              if( fieldDefn->GetType() == OFTInteger )
              {
                fieldType = "Integer";
              }
              else if( fieldDefn->GetType() == OFTReal )
              {
                fieldType = "Real";
              }
              else if( fieldDefn->GetType() == OFTString )
              {
                fieldType = "String";
              }
              else if ( fieldDefn->GetType() == OFTWideString)
              {
                fieldType = "WideString";
              }
              else if ( fieldDefn->GetType() == OFTBinary )
              {
                fieldType = "Binary";
              }
              else if ( fieldDefn->GetType() == OFTDate )
              {
                fieldType = "Date";
              }
              else if ( fieldDefn->GetType() == OFTTime )
              {
                fieldType = "Time";
              }
              else if ( fieldDefn->GetType() == OFTDateTime )
              {
                fieldType = "DateTime";
              }
              else
              {
                fieldType = "String";
              }
              rspfString fieldInfo = rspfString(fieldName + " (" + fieldType + ")");
              rspfString colPrefix = prefix + "column" + rspfString::toString(iField);
              kwl.add(colPrefix,
                "",
                fieldInfo,
                true);
            }
          }
        }
        kwl.add(specialPrefix,
          "name",
          layerName,
          true);
        kwl.add(specialPrefix,
          "geometry",
          geomType,
          true);
      } //end if layer
    }// end i
    return true;
  }// end if datasource
  return false;
}
std::ostream& rspfOgrInfo::print(std::ostream& out) const
{
  static const char MODULE[] = "rspfOgrInfo::print";
  if (traceDebug())
  {    
     rspfNotify(rspfNotifyLevel_DEBUG)
      << MODULE << " DEBUG Entered...\n";
  }
  return out;
}
rspfString rspfOgrInfo::getDriverName(rspfString driverName) const
{
  if (driverName == "esri shapefile")
  {
    return "shp";
  }
  else if (driverName == "ogdi")
  {
    return "vpf";
  }
  else
  {
    return driverName;
  }
}
