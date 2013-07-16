// $Id$

#include <rspf/base/rspfWms.h>
#include <rspf/base/rspfXmlString.h>
#include <deque>
#include <iostream>
#include <sstream>
#include <rspf/base/rspfCommon.h>

bool rspfWmsGetMap::read(rspfRefPtr<rspfXmlNode> node)
{
   clearFields();
   const vector<rspfRefPtr<rspfXmlNode> >& childNodes = node->getChildNodes();
   rspf_uint32 idx = 0;
   
   for(idx = 0; idx < childNodes.size();++idx)
   {
      if(childNodes[idx]->getTag() == "Format")
      {
         rspfString text = childNodes[idx]->getText();
         text = text.trim();
         if(!text.empty())
         {
            theFormatTypes.push_back(text);
         }
         else
         {
            const vector<rspfRefPtr<rspfXmlNode> >& childFormatNodes =  childNodes[idx]->getChildNodes();
            if(childFormatNodes.size())
            {
               rspf_uint32 childIdx = 0;
               for(childIdx = 0; childIdx < childFormatNodes.size();++childIdx)
               {
                  text = childFormatNodes[childIdx]->getTag();
                  text = text.trim();
                  if(!text.empty())
                  {
                     theFormatTypes.push_back(text);
                  }
               }
            }
         }
      }
      else if(childNodes[idx]->getTag() == "DCPType")
      {
         rspfRefPtr<rspfXmlNode> node = childNodes[idx]->findFirstNode("HTTP/Get/OnlineResource");
         if(node.valid())
         {
            node->getAttributeValue(theUrl, "xlink:href");
         }
         else
         {
            node = childNodes[idx]->findFirstNode("HTTP/Get");
            if(node.valid())
            {
               node->getAttributeValue(theUrl, "onlineResource");
               theUrl = rspf::convertHtmlSpecialCharactersToNormalCharacter(theUrl);
            }
         }
      }
   }
   
   return true;
}

bool rspfWmsGetMap::hasFormat(const rspfString& format, bool caseSensitive)const
{
   if(theFormatTypes.empty()) return false;
   rspfString tempFormat = format;
   if(!caseSensitive) tempFormat = tempFormat.downcase();

   rspf_uint32 idx = 0;
   for(idx = 0; idx < theFormatTypes.size();++idx)
   {
      rspfString temp = theFormatTypes[idx];
      if(!caseSensitive)
      {
         temp = temp.downcase();
      }
      if(temp==tempFormat)
      {
         return true;
      }
   }

   return false;
}

bool rspfWmsGetMap::containsFormat(const rspfString& format, bool caseSensitive)const
{
   if(theFormatTypes.empty()) return false;
   rspfString tempFormat = format;
   if(!caseSensitive) tempFormat = tempFormat.downcase();

   rspf_uint32 idx = 0;
   for(idx = 0; idx < theFormatTypes.size();++idx)
   {
      rspfString temp = theFormatTypes[idx];
      if(!caseSensitive)
      {
         temp = temp.downcase();
      }
      if(temp.contains(tempFormat))
      {
         return true;
      }
   }

   return false;
}

bool rspfWmsGetCapabilities::read(rspfRefPtr<rspfXmlNode> node)
{
   rspfRefPtr<rspfXmlNode> childNode = node->findFirstNode("DCPType/HTTP/Get/OnlineResource");

   if(childNode.valid())
   {
      childNode->getAttributeValue(theUrl, "xlink:href");
   }

   return true;
}

bool rspfWmsRequest::read(rspfRefPtr<rspfXmlNode> node)
{
   const vector<rspfRefPtr<rspfXmlNode> >& childNodes = node->getChildNodes();
   rspf_uint32 idx = 0;
   
   for(idx = 0; idx < childNodes.size();++idx)
   {
      if((childNodes[idx]->getTag() == "GetCapabilities")||
         (childNodes[idx]->getTag() == "Capabilities"))
      {
         theGetCapabilities = new rspfWmsGetCapabilities;
         theGetCapabilities->read(childNodes[idx]);
      }
      else if((childNodes[idx]->getTag() == "GetMap")||
              (childNodes[idx]->getTag() == "Map"))
      {
         theGetMap = new rspfWmsGetMap;
         theGetMap->read(childNodes[idx]);
      }
   }
   
   return true;
}

bool rspfWmsTimeExtent::read(rspfRefPtr<rspfXmlNode> node)
{
   clearFields();
   node->getAttributeValue(theDefaultValue, "default");
   rspfString timeValues = node->getText();

   timeValues.split(theTimes, "/");

   return true;
}

bool rspfWmsStyle::read(rspfRefPtr<rspfXmlNode> node)
{
   const vector<rspfRefPtr<rspfXmlNode> >& childNodes = node->getChildNodes();
   rspf_uint32 idx = 0;
   for(idx = 0; idx < childNodes.size();++idx)
   {
      if(childNodes[idx]->getTag() == "Name")
      {
         theName = childNodes[idx]->getText();
      }
      else if(childNodes[idx]->getTag() == "Title")
      {
         theTitle = childNodes[idx]->getText();
      }
      else if(childNodes[idx]->getTag() == "Abstract")
      {
         theAbstract = childNodes[idx]->getText();
      }
   }

   return true;
}

bool rspfWmsScaleHint::read(rspfRefPtr<rspfXmlNode> node)
{
   rspfString minScale, maxScale;

   if(node->getAttributeValue(minScale, "min")&&
      node->getAttributeValue(maxScale, "max"))
   {

      theMin = minScale.toDouble();
      theMax = maxScale.toDouble();
      
      return true;
   }
   
   return false;
}

bool rspfWmsBoundingBox::read(rspfRefPtr<rspfXmlNode> node)
{
   rspfString minx, miny, maxx, maxy;

   bool result = (node->getAttributeValue(minx, "minx")&&
                  node->getAttributeValue(miny, "miny")&&
                  node->getAttributeValue(maxx, "maxx")&&
                  node->getAttributeValue(maxy, "maxy"));
   
   node->getAttributeValue(theSrs, "SRS");
   
   if(node->getTag() == "LatLonBoundingBox")
   {
      theSrs = "EPSG:4326";
   }

   theMinX = minx.toDouble();
   theMinY = miny.toDouble();
   theMaxX = maxx.toDouble();
   theMaxY = maxy.toDouble();

   return result;
}

bool rspfWmsLayer::read(rspfRefPtr<rspfXmlNode> node)
{
   clearFields();
   const vector<rspfRefPtr<rspfXmlNode> >& childNodes = node->getChildNodes();
   rspf_uint32 idx = 0;
   for(idx = 0; idx < childNodes.size();++idx)
   {
      if(childNodes[idx]->getTag() == "Name")
      {
         theName = childNodes[idx]->getText();
      }
      else if(childNodes[idx]->getTag() == "Title")
      {
         theTitle = childNodes[idx]->getText();
      }
      else if(childNodes[idx]->getTag() == "Abstract")
      {
         theAbstract = childNodes[idx]->getText();
      }
      else if(childNodes[idx]->getTag() == "Extent")
      {
         rspfString name;
         if(childNodes[idx]->getAttributeValue(name, "name"))
         {
            if(name == "time")
            {
               theTimeExtent = new rspfWmsTimeExtent;
               if(!theTimeExtent->read(childNodes[idx]))
               {
                  theTimeExtent = 0;
               }
            }
         }
      }
      else if(childNodes[idx]->getTag() == "SRS")
      {
         theSrs = childNodes[idx]->getText();
      }
      else if(childNodes[idx]->getTag() == "Dimension")
      {
         childNodes[idx]->getAttributeValue(theDimensionUnits, "units");
         childNodes[idx]->getAttributeValue(theDimensionName,  "name");
      }
      else if(childNodes[idx]->getTag() == "Style")
      {
         rspfRefPtr<rspfWmsStyle> style = new rspfWmsStyle;
         style->read(childNodes[idx]);
         theStyles.push_back(style);
      }
      else if(childNodes[idx]->getTag() == "ScaleHint")
      {
         theScaleHint = new rspfWmsScaleHint;
         if(!theScaleHint->read(childNodes[idx]))
         {
            theScaleHint = 0;
         }
      }
      else if(childNodes[idx]->getTag() == "LatLonBoundingBox"||
              childNodes[idx]->getTag() == "BoundingBox")
      {
         theBoundingBox = new rspfWmsBoundingBox;
         if(!theBoundingBox->read(childNodes[idx]))
         {
            theBoundingBox = 0;
         }
      }
      else if(childNodes[idx]->getTag() == "Layer")
      {
         rspfRefPtr<rspfWmsLayer> layer = new rspfWmsLayer;
         if(!layer->read(childNodes[idx]))
         {
            return false;
         }
         layer->setParent(this);
         theLayers.push_back(layer.get());
      }
   }

   return true;
}

void rspfWmsLayer::getNamedLayers(rspfWmsLayerListType& namedLayers)
{
   if(theLayers.empty()) return;
   
   std::deque<rspfRefPtr<rspfWmsLayer> > layers(theLayers.begin(), theLayers.end());
   
   while(!layers.empty())
   {
      rspfRefPtr<rspfWmsLayer> layer = layers.front();
      layers.pop_front();
      if(!layer->getName().empty())
      {
         namedLayers.push_back(layer);
      }
      if(layer->getNumberOfChildren()>0)
      {
         rspfWmsLayerListType& layerList = layer->getLayers();
         layers.insert(layers.end(), layerList.begin(), layerList.end());
      }
   }
}

bool rspfWmsCapability::read(const rspfRefPtr<rspfXmlNode> node)
{
   const vector<rspfRefPtr<rspfXmlNode> >& childNodes = node->getChildNodes();
   
   rspf_uint32 idx = 0;
   for(idx = 0; idx < childNodes.size();++idx)
   {
      if(childNodes[idx]->getTag() == "Layer")
      {
         rspfRefPtr<rspfWmsLayer> layer = new rspfWmsLayer;
         if(!layer->read(childNodes[idx]))
         {
            return false;
         }
         theLayers.push_back(layer.get());
      }
      else if(childNodes[idx]->getTag() == "Request")
      {
         theRequest = new rspfWmsRequest;
         theRequest->read(childNodes[idx]);
      }
   }
   
   return true;
}

void rspfWmsCapability::getNamedLayers(rspfWmsLayerListType&  layers)
{
   rspf_uint32 idx = 0;

   for(idx = 0; idx < theLayers.size(); ++idx)
   {
      if(!theLayers[idx]->getName().empty())
      {
         layers.push_back(theLayers[idx]);
      }
      theLayers[idx]->getNamedLayers(layers);
   }
   
}

bool rspfWmsCapabilitiesDocument:: read(const std::string& inString)
{
   std::istringstream in(inString);
   rspfRefPtr<rspfXmlDocument> document = new rspfXmlDocument;
   if(!document->read(in))
   {
//       std::cout << "Couldn't parse XML!!!!!" << std::endl;
      return false;
   }
   if(!read(document->getRoot()))
   {
      return false;
   }
   
//   std::cout << *document << std::endl;
   return true;
}

bool rspfWmsCapabilitiesDocument::read(rspfRefPtr<rspfXmlNode> node)
{
   const vector<rspfRefPtr<rspfXmlNode> >& childNodes = node->getChildNodes();
   clearFields();
   if(!node.valid()) return false;
   node->getAttributeValue(theVersion, "version");
   rspf_uint32 idx = 0;
   for(idx = 0; idx < childNodes.size();++idx)
   {
      if(childNodes[idx]->getTag() == "Capability")
      {
         theCapability = new rspfWmsCapability;
         if(!theCapability->read(childNodes[idx]))
         {
            return false;
         }
      }
   }

   return theCapability.valid();
}

rspfRefPtr<rspfWmsGetMap> rspfWmsCapabilitiesDocument::getRequestGetMap()
{
   rspfRefPtr<rspfWmsGetMap> result;

   if(theCapability.valid())
   {
      rspfRefPtr<rspfWmsRequest> request = theCapability->getRequest();
      if(request.valid())
      {
         result = request->getMap();
      }
   }
   
   return result;
}

const rspfRefPtr<rspfWmsGetMap> rspfWmsCapabilitiesDocument::getRequestGetMap()const
{

   if(theCapability.valid())
   {
      const rspfRefPtr<rspfWmsRequest> request = theCapability->getRequest();
      if(request.valid())
      {
         return request->getMap();
      }
   }
   
   return 0;
}

rspfRefPtr<rspfWmsGetCapabilities> rspfWmsCapabilitiesDocument::getRequestGetCapabilities()
{
   rspfRefPtr<rspfWmsGetCapabilities> result;

   if(theCapability.valid())
   {
      rspfRefPtr<rspfWmsRequest> request = theCapability->getRequest();
      if(request.valid())
      {
         result = request->getCapabilities();
      }
   }
   
   return result;
}

const rspfRefPtr<rspfWmsGetCapabilities> rspfWmsCapabilitiesDocument::getRequestGetCapabilities()const
{
   if(theCapability.valid())
   {
      const rspfRefPtr<rspfWmsRequest> request = theCapability->getRequest();
      if(request.valid())
      {
         return request->getCapabilities();
      }
   }
   
   return 0;
}
