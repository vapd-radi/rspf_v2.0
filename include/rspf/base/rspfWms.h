// $Id$

#ifndef rspfWms_HEADER
#define rspfWms_HEADER

#include <rspf/base/rspfXmlDocument.h>
#include <rspf/base/rspfXmlNode.h>

#include <queue>
class rspfWmsStyle;
class rspfWmsLayer;

typedef std::vector<rspfString> rspfWmsStringListType;

typedef std::vector<rspfRefPtr<rspfWmsStyle> > rspfWmsStyleListType;
typedef const std::vector<rspfRefPtr<rspfWmsStyle> > rspfConstWmsStyleListType;
typedef std::vector<rspfRefPtr<rspfWmsLayer> > rspfWmsLayerListType;
typedef const std::vector<rspfRefPtr<rspfWmsLayer> > rspfConstWmsLayerListType;

class RSPF_DLL rspfWmsGetMap : public rspfReferenced
{
public:
   rspfWmsGetMap()
   {
      clearFields();
   }
   virtual bool read(rspfRefPtr<rspfXmlNode> node);

   rspfWmsStringListType& getFormatTypes()
   {
      return theFormatTypes;
   }
   const rspfWmsStringListType& getFormatTypes()const
   {
      return theFormatTypes;
   }
   bool hasFormats()const
   {
      return theFormatTypes.size() > 0;
   }
   /**
    * Will do an exact compare on the input format.  You can also specify if you want case sensitive
    * compares
    */ 
   bool hasFormat(const rspfString& format, bool caseSensitive=true)const;

   /**
    * Will not do an exact compare but instead will test if the passed in string is contained within
    * the formats.  You can also specify case sensitive.
    */ 
   bool containsFormat(const rspfString& format, bool caseSensitive=true)const;
   void clearFields()
   {
      theUrl = "";
      thePostUrl = "";
      theFormatTypes.clear();
   }
   const rspfString& getUrl()const
   {
      return theUrl;
   }
   const rspfString& getPostUrl()const
   {
      return thePostUrl;
   }
protected:
   rspfString            theUrl; 
   rspfString            thePostUrl; 
   rspfWmsStringListType theFormatTypes;
};

class RSPF_DLL rspfWmsGetCapabilities : public rspfReferenced
{
public:
   rspfWmsGetCapabilities()
   {
   }
   virtual bool read(rspfRefPtr<rspfXmlNode> node);

   const rspfString& getUrl()const
   {
      return theUrl;
   }
protected:
   rspfString        theUrl; 
  
};

class RSPF_DLL rspfWmsRequest : public rspfReferenced
{
public:
   rspfWmsRequest()
   {
   }
   virtual bool read(rspfRefPtr<rspfXmlNode> node);

   rspfRefPtr<rspfWmsGetCapabilities> getCapabilities()
   {
      return theGetCapabilities;
   }
   const rspfRefPtr<rspfWmsGetCapabilities> getCapabilities()const
   {
      return theGetCapabilities;
   }
   rspfRefPtr<rspfWmsGetMap> getMap()
   {
      return theGetMap;
   }
   const rspfRefPtr<rspfWmsGetMap> getMap()const
   {
      return theGetMap;
   }
   
protected:
   rspfRefPtr<rspfWmsGetCapabilities> theGetCapabilities;
   rspfRefPtr<rspfWmsGetMap> theGetMap;
};
class RSPF_DLL rspfWmsContactAdress : public rspfReferenced
{
public:
   rspfWmsContactAdress()
      {}
protected:
   rspfString theAddressType;
   rspfString theAddress;
   rspfString theCity;
   rspfString theStateOrProvince;
   rspfString thePostCode;
   rspfString theCountry;
};

class RSPF_DLL rspfWmsContactInformation : public rspfReferenced
{
public:
   virtual bool read(rspfRefPtr<rspfXmlNode> /* node */ )
      {
         return true;
      }
   
protected:
   rspfString thePrimaryContactPerson;
   rspfString thePrimaryContactOrganization;
   rspfString theContactPosition;
   rspfRefPtr<rspfWmsContactAdress> theContactAddress;
   rspfString theContactVoiceTelephone;
   rspfString theContactEmailAddress;
};

class RSPF_DLL rspfWmsService : public rspfReferenced
{
public:
   rspfWmsService()
      {
      }

   virtual bool read(rspfRefPtr<rspfXmlNode> /* xml */ )
      {
         return true;
      }

protected:
   rspfString theName;
   rspfString theTitle;
   rspfString theAbstract;
   std::vector<rspfString> theKeywordList;
   rspfString theOnlineResource;
   rspfRefPtr<rspfWmsContactInformation> theContactInformation;
   rspfString theFees;
   rspfString theAccessConstraints;
};


class RSPF_DLL rspfWmsTimeExtent : public rspfReferenced
{
public:
   rspfWmsTimeExtent()
      {
      }
   virtual bool read(rspfRefPtr<rspfXmlNode> node);

   void clearFields()
      {
         theDefaultValue = "";
         theTimes.clear();
      }
protected:
   rspfString theDefaultValue;
   std::vector<rspfString> theTimes;
};


class RSPF_DLL rspfWmsMetadataUrl : public rspfReferenced
{
public:
   rspfWmsMetadataUrl()
      {
      }

protected:
   rspfString theFormat;
   rspfString theOnlineResourceHref;
};

class RSPF_DLL rspfWmsDataUrl : public rspfReferenced
{
public:
protected:
   rspfString theFormat;
   rspfString theOnlineResourceHref;
};

class RSPF_DLL rspfWmsStyle : public rspfReferenced
{
public:
   rspfWmsStyle()
      {}
   virtual bool read(rspfRefPtr<rspfXmlNode> node);
   void clearFields()
      {
         theName  = "";
         theTitle = "";
         theAbstract = "";
      }

   const rspfString& getName()const
      {
         return theName;
      }
   
   const rspfString& getTitle()const
      {
         return theTitle;
      }

   const rspfString& getAbstract()const
   {
      return theAbstract;
   }
protected:
   rspfString theName;
   rspfString theTitle;
   rspfString theAbstract;
};

class RSPF_DLL rspfWmsScaleHint : public rspfReferenced
{
public:
   virtual bool read(rspfRefPtr<rspfXmlNode> node);

   void clearFields()
      {
         theMin = 0.0;
         theMax = 0.0;
      }
   void getMinMax(double& min, double& max)
   {
      min = theMin;
      max = theMax;
   }
   double getMin()const
   {
      return theMin;
   }
   double getMax()const
   {
      return theMax;
   }
protected:
   double theMin;
   double theMax;
};

class RSPF_DLL rspfWmsBoundingBox : public rspfReferenced
{
public:
   rspfWmsBoundingBox()
      {
      }
   virtual bool read(rspfRefPtr<rspfXmlNode> node);
   void clearFields()
      {
         theSrs = "";
         theMinX = 0.0;
         theMinY = 0.0;
         theMaxX = 0.0;
         theMaxY = 0.0;
      }
   bool isGeographic()const
      {
         return theSrs.contains("4326");
      }
   const rspfString& getSrs()const
      {
         return theSrs;
      }
   double getMinX()const
      {
         return theMinX;
      }
   double getMinY()const
      {
         return theMinY;
      }
   double getMaxX()const
      {
         return theMaxX;
      }
   double getMaxY()const
      {
         return theMaxY;
      }
protected:
   rspfString theSrs;
   double theMinX;
   double theMinY;
   double theMaxX;
   double theMaxY;
};

class RSPF_DLL rspfWmsLayer : public rspfReferenced
{
public:
   
   rspfWmsLayer()
      :theParent(0)
      {
         clearFields();
      }
   virtual bool read(rspfRefPtr<rspfXmlNode> node);

   void clearFields()
      {
         theName           = "";
         theTitle           = "";
         theAbstract       = "";
         theSrs            = "";
         theTimeExtent     = 0;
         theDimensionUnits = "";
         theDimensionName  = "";
         theBoundingBox    = 0;
         theScaleHint      = 0;
         theStyles.clear();
         theLayers.clear();
      }
   const rspfWmsLayer* getParent()const
      {
         return theParent;
      }

   rspfWmsLayer* getParent()
      {
         return theParent;
      }
   
    void setParent(rspfWmsLayer* parent)
      {
         theParent = parent;
      }
   const rspfRefPtr<rspfWmsBoundingBox> findBoundingBox()const
      {
         const rspfWmsLayer* currentLayer = this;

         while(currentLayer)
         {
            if(currentLayer->theBoundingBox.valid())
            {
               return currentLayer->theBoundingBox;
            }
            currentLayer = currentLayer->theParent;
         }
         
         return 0;
      }

   void getNamedLayers(rspfWmsLayerListType& namedLayers);

   const rspfString& getName()const
      {
         return theName;
      }
   const rspfString& getTitle()const
      {
         return theTitle;
      }
   const rspfString& getAbstract()const
      {
         return theAbstract;
      }
   const rspfString& getSrs()const
      {
         return theSrs;
      }
   const rspfRefPtr<rspfWmsTimeExtent> getTimeExtent()
      {
         return theTimeExtent;
      }
   const rspfString& getDimensionUnits()const
      {
         return theDimensionUnits;
      }
   const rspfString& getDimensionName()const
      {
         return theDimensionName;
      }
   const rspfRefPtr<rspfWmsBoundingBox> getBoundingBox()const
      {
         return theBoundingBox;
      }
   const rspfRefPtr<rspfWmsScaleHint> getScaleHint()const
      {
         return theScaleHint;
      }
   const rspfWmsStyleListType& getStyles()const
      {
         return theStyles;
      }
   rspfWmsStyleListType& getStyles()
      {
         return theStyles;
      }
   const rspfWmsLayerListType& getLayers()const
      {
         return theLayers;
      }
   rspfWmsLayerListType& getLayers()
   {
      return theLayers;
   }

   rspf_uint32 getNumberOfChildren()const
   {
      return (rspf_uint32)theLayers.size();
   }
   rspf_uint32 getNumberOfStyles()const
   {
      return (rspf_uint32)theStyles.size();
   }
protected:
   rspfWmsLayer* theParent;
   rspfString theName;
   rspfString theTitle;
   rspfString theAbstract;
   rspfString theSrs;
   rspfRefPtr<rspfWmsTimeExtent> theTimeExtent;
   rspfString theDimensionUnits;
   rspfString theDimensionName;
   rspfRefPtr<rspfWmsBoundingBox> theBoundingBox;
   rspfRefPtr<rspfWmsScaleHint>          theScaleHint;
   rspfWmsStyleListType theStyles;
   rspfWmsLayerListType theLayers;
};

class RSPF_DLL rspfWmsCapability : public rspfReferenced
{
public:
   virtual bool read(const rspfRefPtr<rspfXmlNode> node);
   void getNamedLayers(rspfWmsLayerListType&  layers);
   rspfRefPtr<rspfWmsRequest> getRequest()
   {
      return theRequest;
   }
   const rspfRefPtr<rspfWmsRequest> getRequest()const
   {
      return theRequest;
   }
protected:
   rspfRefPtr<rspfWmsRequest>             theRequest;
   std::vector<rspfRefPtr<rspfWmsLayer> > theLayers;
};

class RSPF_DLL rspfWmsCapabilitiesDocument : public rspfReferenced
{
public:
   rspfWmsCapabilitiesDocument()
      {
         
      }

   bool read(const std::string& inString);
   virtual bool read(rspfRefPtr<rspfXmlNode> node);
   void clearFields()
      {
         theVersion = "";
         theCapability=0;
      }
   rspfRefPtr<rspfWmsCapability> getCapability()
   {
      return theCapability;
   }
   const rspfRefPtr<rspfWmsCapability> getCapability()const
   {
      return theCapability;
   }

   rspfRefPtr<rspfWmsGetMap> getRequestGetMap();
   const rspfRefPtr<rspfWmsGetMap> getRequestGetMap()const;

   rspfRefPtr<rspfWmsGetCapabilities> getRequestGetCapabilities();
   const rspfRefPtr<rspfWmsGetCapabilities> getRequestGetCapabilities()const;

protected:
   rspfString theVersion;
   rspfRefPtr<rspfWmsCapability> theCapability;
};

#endif
