//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfVpfAnnotationFeatureInfo.h 19636 2011-05-24 16:48:45Z gpotts $
#ifndef rspfVpfAnnotationFeatureInfo_HEADER
#define rspfVpfAnnotationFeatureInfo_HEADER
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfRgbVector.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfGeoPolygon.h>
#include <rspf/imaging/rspfGeoAnnotationObject.h>
#include <rspf/vec/rspfVpfCoverage.h>
#include <rspf/base/rspfFontInformation.h>

class rspfGeoAnnotationObject;
class rspfVpfCoverage;
class rspfVpfFeatureClassSchema;
class rspfPolyLine;

class RSPFDLLEXPORT rspfVpfAnnotationFeatureInfo : public rspfObject
{
public:
  enum rspfVpfAnnotationFeatureType
    {
      rspfVpfAnnotationFeatureType_UNKNOWN  = 0,
      rspfVpfAnnotationFeatureType_POINT    = 1,
      rspfVpfAnnotationFeatureType_LINE     = 2,
      rspfVpfAnnotationFeatureType_POLYGON  = 3,
      rspfVpfAnnotationFeatureType_TEXT     = 4,
    };

  rspfVpfAnnotationFeatureInfo(const rspfString& featureName   = "",
				const rspfRgbVector& penColor   = rspfRgbVector(255,255,255),
				const rspfRgbVector& brushColor = rspfRgbVector(255,255,255),
                                const rspfDpt& pointRadius=rspfDpt(1,1),
                                int   thickness=0,
				bool enabledFlag = true);
  ~rspfVpfAnnotationFeatureInfo();

  void setCoverage(const rspfVpfCoverage& coverage)
  {
    theCoverage = coverage;
  }

  const rspfVpfCoverage& getCoverage()const
  {
    return theCoverage;
  }
  rspfVpfCoverage& getCoverage()
  {
    return theCoverage;
  }

   void setColor(const rspfRgbVector& color)
      {
         setPenColor(color);
         setBrushColor(color);
      }
  void setPenColor(const rspfRgbVector& penColor)
  {
    thePenColor = penColor;
  }

  rspfRgbVector getPenColor()const
  {
    return thePenColor;
  }

  void setBrushColor(const rspfRgbVector& brushColor)
  {
    theBrushColor = brushColor;
  }

  rspfRgbVector getBrushColor()const
  {
    return theBrushColor;
  }

  void setFillEnabledFlag(bool flag)
  {
     theFillEnabledFlag = flag;
  }
  bool getFillEnabledFlag()const
  {
     return theFillEnabledFlag;
  }
  void setEnabledFlag(bool flag);
  bool getEnabledFlag()const
  {
    return theEnabledFlag;
  }

  void setThickness(int thickness)
  {
     theThickness = thickness;
  }

  int getThickness()const
  {
     return theThickness;
  }
  
  void setName(const rspfString& name)
  {
    theName = name;
  }
  const rspfString& getName()const
  {
    return theName;
  }
   
  bool isPoint()const
  {
     return (theFeatureType == rspfVpfAnnotationFeatureType_POINT);
  }
  bool isLine()const
  {
     return (theFeatureType == rspfVpfAnnotationFeatureType_LINE);
  }
  bool isPolygon()const
  {
     return (theFeatureType == rspfVpfAnnotationFeatureType_POLYGON);
  }
  bool isText()const
  {
     return (theFeatureType == rspfVpfAnnotationFeatureType_TEXT);
  }
  void getFontInformation(rspfFontInformation& fontInfo)const
  {
     fontInfo = theFontInformation;
  }
  void setFontInformation(const rspfFontInformation& fontInfo)
  {
     theFontInformation = fontInfo;
  }

  void setPointRadius(const rspfDpt& radius)
  {
    thePointRadius = radius;
  }
  rspfDpt getPointRadius()const
  {
    return thePointRadius;
  }
  void setDrawingFeaturesToAnnotation();
  rspfIrect getBoundingProjectedRect()const;
  void transform(rspfImageGeometry* proj);
  void buildFeature();
  void deleteAllObjects();
  void drawAnnotations(rspfRgbImage* tile);

  bool saveState(rspfKeywordlist& kwl,
		 const char* prefix=0)const;
  bool loadState(const rspfKeywordlist& kwl,
		 const char* prefix=0);
  rspfString getFeatureTypeAsString()const;
  rspfVpfAnnotationFeatureType getFeatureTypeFromString(const rspfString& featureType)const;

private:
  void readAttributes(rspfPolyLine& polyLine, rspfVpfTable& table, int row);
  void readAttributes(rspfGeoPolygon& polygon, rspfVpfTable& table, int row);

protected:
  rspfString                   theName;
  rspfString                   theDescription;
  rspfRgbVector                thePenColor;
  rspfRgbVector                theBrushColor;
  rspfVpfCoverage              theCoverage;
  rspfDpt                      thePointRadius;
  int                           theThickness; 
  bool                          theFillEnabledFlag;
  bool                          theEnabledFlag;
  rspfVpfAnnotationFeatureType theFeatureType;
  rspfFontInformation          theFontInformation;
   
  std::vector<rspfRefPtr<rspfGeoAnnotationObject> > theAnnotationArray;

  void buildTxtFeature(const rspfFilename& table,
                       const rspfString&   tableKey,
                       const rspfFilename& primitive,
                       const rspfString&   primitiveKey);
   
  void buildEdgFeature(const rspfFilename& table,
                       const rspfString&   tableKey,
		       const rspfFilename& primitive,
		       const rspfString&   primitiveKey);

  void buildPointFeature(const rspfString& primitiveName,
			 const rspfFilename& table,
			 const rspfString&   tableKey,
			 const rspfFilename& primitive,
			 const rspfString&   primitiveKey);
  void buildFaceFeature(const rspfFilename& table,
                        const rspfString&   tableKey,
                        const rspfFilename& primitive,
			const rspfString&   primitiveKey);
  

  void readEdge(rspfPolyLine& polyLine,
		int rowNumber,
		int colPosition,
		rspfVpfTable& edgeTable);
  int readTableCellAsInt (int rowNumber, // should replace calls to readRngId, readStartEdgeId with this
			  int colNumber,
			  rspfVpfTable& table);
  int readRngId(int rowNumber,
		int colNumber,
		rspfVpfTable& faceTable);
  int readStartEdgeId(int rowNumber,
		      int colNumber,
		      rspfVpfTable& rngTable);
  void readGeoPolygon(rspfGeoPolygon& polygon,
		      int faceId,
		      int startEdgeId,
		      rspfVpfTable& edgTable);

  rspfDpt* getXy(vpf_table_type table, row_type row, rspf_int32 pos, rspf_int32* count);
  int getEdgeKeyId (vpf_table_type& table, row_type& row, int col);

TYPE_DATA
};

#endif
