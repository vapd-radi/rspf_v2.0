//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfMapCompositionSource.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfMapCompositionSource_HEADER
#define rspfMapCompositionSource_HEADER
#include <rspf/imaging/rspfAnnotationSource.h>
#include <rspf/base/rspfRgbVector.h>
#include <rspf/base/rspfPolyArea2d.h>
#include <rspf/font/rspfFont.h>
#include <rspf/base/rspfFontInformation.h>

class rspfFont;
class rspfMapProjection;
class rspfU8ImageData;

class rspfMapCompositionSource : public rspfAnnotationSource
{
public:
   enum rspfGridLineType
   {
      RSPF_GRID_NONE    = 0,
      RSPF_GRID_LINE    = 1,
      RSPF_GRID_RESEAUX = 2
   };

   rspfMapCompositionSource();
   
   virtual rspfScalarType getOutputScalarType() const;
   virtual rspf_uint32    getNumberOfOutputBands() const;
   
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                               rspf_uint32 resLevel=0);


   rspfIpt getViewWidthHeight()const;

   void getViewWidthHeight(rspfIpt& widthHeight)const;
   
   void setViewWidthHeight(const rspfIpt& widthHeight);
   
   // all borders here
   virtual rspf_int32 getTopBorderLength()const;
   virtual rspf_int32 getBottomBorderLength()const;
   virtual rspf_int32 getLeftBorderLength()const;
   virtual rspf_int32 getRightBorderLength()const;

   virtual void setTopBorderLength(rspf_int32 length);
   virtual void setBottomBorderLength(rspf_int32 length);
   virtual void setLeftBorderLength(rspf_int32 length);
   virtual void setRightBorderLength(rspf_int32 length);

   //all spacing here
   void setGeographicSpacingLat(double value);
   void setGeographicSpacingLon(double value);
   void setMeterSpacingX(double value);
   void setMeterSpacingY(double value);
   void setMeterSpacing(double x, double y);
   void setGeographicSpacing(double lat, double lon);

   rspfDpt getGeographicSpacing()const;
   rspfDpt getMeterSpacing()const;
   
   // All geographic formatting here
   virtual rspfString getTopGeographicLabelFormat()const;
   virtual rspfString getBottomGeographicLabelFormat()const;
   virtual rspfString getLeftGeographicLabelFormat()const;
   virtual rspfString getRightGeographicLabelFormat()const;

   virtual void setGeographicLabelFormat(const rspfString format);
   virtual void setTopGeographicLabelFormat(const rspfString& format);
   virtual void setBottomGeographicLabelFormat(const rspfString& format);
   virtual void setLeftGeographicLabelFormat(const rspfString& format);
   virtual void setRightGeographicLabelFormat(const rspfString& format);
   
   // All font funtions here.
   virtual rspfString    getTitle()const;
   virtual rspfRgbVector getTitleColor()const;

   virtual void setTitleFont(const rspfFontInformation& fontInfo);
   virtual void setTitleColor(const rspfRgbVector& color);
   virtual void setTitle(const rspfString& s);
   virtual void setGeographicTopLabelFont(const rspfFontInformation& fontInfo);
   virtual void setGeographicBottomLabelFont(const rspfFontInformation& fontInfo);
   virtual void setGeographicLeftLabelFont(const rspfFontInformation& fontInfo);
   virtual void setGeographicRightLabelFont(const rspfFontInformation& fontInfo);

   virtual void setMeterTopLabelFont(const rspfFontInformation& fontInfo);
   virtual void setMeterBottomLabelFont(const rspfFontInformation& fontInfo);
   virtual void setMeterLeftLabelFont(const rspfFontInformation& fontInfo);
   virtual void setMeterRightLabelFont(const rspfFontInformation& fontInfo);

   virtual rspfFontInformation getTitleFont()const;
   virtual rspfFontInformation getGeographicTopLabelFontInfo()const;
   virtual rspfFontInformation getGeographicBottomLabelFontInfo()const;
   virtual rspfFontInformation getGeographicLeftLabelFontInfo()const;
   virtual rspfFontInformation getGeographicRightLabelFontInfo()const;
   virtual rspfFontInformation getMeterTopLabelFontInfo()const;
   virtual rspfFontInformation getMeterBottomLabelFontInfo()const;
   virtual rspfFontInformation getMeterLeftLabelFontInfo()const;
   virtual rspfFontInformation getMeterRightLabelFontInfo()const;

   // All color funtions here
   virtual rspfRgbVector getBorderColor()const;
   virtual void setBorderColor(const rspfRgbVector& color);

   virtual rspfRgbVector getGeographicGridColor()const;
   virtual rspfRgbVector getMeterGridColor()const;
   virtual rspfRgbVector getTopGeographicLabelColor()const;
   virtual rspfRgbVector getBottomGeographicLabelColor()const;
   virtual rspfRgbVector getLeftGeographicLabelColor()const;
   virtual rspfRgbVector getRightGeographicLabelColor()const;

   virtual void setGeographicGridColor(const rspfRgbVector& color);
   virtual void setTopGeographicLabelColor(const rspfRgbVector& color);
   virtual void setBottomGeographicLabelColor(const rspfRgbVector& color);
   virtual void setLeftGeographicLabelColor(const rspfRgbVector& color);
   virtual void setRightGeographicLabelColor(const rspfRgbVector& color);
   virtual void setGeographicLabelColor(const rspfRgbVector& color);

   virtual rspfRgbVector getTopMeterLabelColor()const;
   virtual rspfRgbVector getBottomMeterLabelColor()const;
   virtual rspfRgbVector getLeftMeterLabelColor()const;
   virtual rspfRgbVector getRightMeterLabelColor()const;
   
   virtual void setTopMeterLabelColor(const rspfRgbVector& color);
   virtual void setBottomMeterLabelColor(const rspfRgbVector& color);
   virtual void setLeftMeterLabelColor(const rspfRgbVector& color);
   virtual void setRightMeterLabelColor(const rspfRgbVector& color);
   virtual void setMeterLabelColor(const rspfRgbVector& color);
   virtual void setMeterGridColor(const rspfRgbVector& color);

   // All grid types here
   virtual void setMeterGridType(rspfGridLineType gridType);
   virtual void setGeographicGridType(rspfGridLineType gridType);
   virtual void setBorderColor(rspfRgbVector& color);

   virtual rspfGridLineType getGeographicGridType()const;
   virtual rspfGridLineType getMeterGridType()const;
   
   // all grid flags here
   void setGeographicTickMarkFlag(bool flag);
   void setTopGeographicTickMarkFlag(bool flag);
   void setBottomGeographicTickMarkFlag(bool flag);
   void setLeftGeographicTickMarkFlag(bool flag);
   void setRightGeographicTickMarkFlag(bool flag);
   void setTopGeographicLabelFlag(bool flag);
   void setBottomGeographicLabelFlag(bool flag);
   void setLeftGeographicLabelFlag(bool flag);
   void setRightGeographicLabelFlag(bool flag);

   virtual bool getTopGeographicTickMarkFlag()const;
   virtual bool getBottomGeographicTickMarkFlag()const;
   virtual bool getLeftGeographicTickMarkFlag()const;
   virtual bool getRightGeographicTickMarkFlag()const;
   virtual bool getTopGeographicLabelFlag()const;
   virtual bool getBottomGeographicLabelFlag()const;
   virtual bool getLeftGeographicLabelFlag()const;
   virtual bool getRightGeographicLabelFlag()const;

   void setMeterTickMarkFlag(bool flag);
   virtual void setTopMeterTickMarkFlag(bool flag);
   virtual void setBottomMeterTickMarkFlag(bool flag);
   virtual void setLeftMeterTickMarkFlag(bool flag);
   virtual void setRightMeterTickMarkFlag(bool flag);
   virtual void setTopMeterLabelFlag(bool flag);
   virtual void setBottomMeterLabelFlag(bool flag);
   virtual void setLeftMeterLabelFlag(bool flag);
   virtual void setRightMeterLabelFlag(bool flag);
   
   virtual bool getTopMeterTickMarkFlag()const;
   virtual bool getBottomMeterTickMarkFlag()const;
   virtual bool getLeftMeterTickMarkFlag()const;
   virtual bool getRightMeterTickMarkFlag()const;
   virtual bool getTopMeterLabelFlag()const;
   virtual bool getBottomMeterLabelFlag()const;
   virtual bool getLeftMeterLabelFlag()const;
   virtual bool getRightMeterLabelFlag()const;
   virtual rspfIrect getBoundingRect(rspf_uint32 resLevel=0)const;

   virtual void initialize();

   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=0)const;
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);

protected:
   virtual ~rspfMapCompositionSource();
   rspfIpt            theViewWidthHeight;
   rspfGridLineType   theMeterGridType;
   rspfGridLineType   theGeographicGridType;
   
   rspf_uint32        theTopBorderLength;
   rspf_uint32        theBottomBorderLength;
   rspf_uint32        theLeftBorderLength;
   rspf_uint32        theRightBorderLength;
   
   rspfRgbVector      theBorderColor;

   rspfRgbVector      theGeographicGridColor;
   rspfRgbVector      theMeterGridColor;

   // title info
   //
   rspfString          theTitleString;
   rspfRefPtr<rspfFont>           theTitleFont;
   rspfFontInformation theTitleFontInfo;
   rspfRgbVector       theTitleColor;
   
   // grid label colors
   //
   rspfRgbVector      theTopGeographicLabelColor;
   rspfRgbVector      theBottomGeographicLabelColor;
   rspfRgbVector      theLeftGeographicLabelColor;
   rspfRgbVector      theRightGeographicLabelColor;
   rspfRgbVector      theTopMeterLabelColor;
   rspfRgbVector      theBottomMeterLabelColor;
   rspfRgbVector      theLeftMeterLabelColor;
   rspfRgbVector      theRightMeterLabelColor;

   // grid label font
   //
   rspfFontInformation theGeographicTopLabelFontInfo;
   rspfRefPtr<rspfFont>           theGeographicTopLabelFont;
   rspfFontInformation theGeographicBottomLabelFontInfo;
   rspfRefPtr<rspfFont>           theGeographicBottomLabelFont;
   rspfFontInformation theGeographicLeftLabelFontInfo;
   rspfRefPtr<rspfFont>           theGeographicLeftLabelFont;
   rspfFontInformation theGeographicRightLabelFontInfo;
   rspfRefPtr<rspfFont>           theGeographicRightLabelFont;

   rspfFontInformation theMeterTopLabelFontInfo;
   rspfRefPtr<rspfFont>           theMeterTopLabelFont;
   rspfFontInformation theMeterBottomLabelFontInfo;
   rspfRefPtr<rspfFont>           theMeterBottomLabelFont;
   rspfFontInformation theMeterLeftLabelFontInfo;
   rspfRefPtr<rspfFont>           theMeterLeftLabelFont;
   rspfFontInformation theMeterRightLabelFontInfo;
   rspfRefPtr<rspfFont>           theMeterRightLabelFont;
   
   // grid label dms format strings
   //
   rspfString         theTopGeographicFormat;
   rspfString         theBottomGeographicFormat;
   rspfString         theLeftGeographicFormat;
   rspfString         theRightGeographicFormat;

   // grid label flag
   //
   bool                theTopGeographicLabelFlag;
   bool                theBottomGeographicLabelFlag;
   bool                theLeftGeographicLabelFlag;
   bool                theRightGeographicLabelFlag;

   bool                theTopGeographicTickFlag;
   bool                theBottomGeographicTickFlag;
   bool                theLeftGeographicTickFlag;
   bool                theRightGeographicTickFlag;

   bool                theTopMeterLabelFlag;
   bool                theBottomMeterLabelFlag;
   bool                theLeftMeterLabelFlag;
   bool                theRightMeterLabelFlag;

   bool                theTopMeterTickFlag;
   bool                theBottomMeterTickFlag;
   bool                theLeftMeterTickFlag;
   bool                theRightMeterTickFlag;
   
   
   rspfIrect          theTopBorder;
   rspfIrect          theBottomBorder;
   rspfIrect          theLeftBorder;
   rspfIrect          theRightBorder;

   // geo-tick spacing in lat lon
   rspfDpt            theGeographicSpacing;

   //  tick spacing in meter units
   rspfDpt            theMeterSpacing;
   
   rspfAnnotationSource::AnnotationObjectListType theFixedAnnotationList;

   /*!
    * Override base classes drawAnnotations so we can layout
    * any fixed annotations first.
    */
   virtual void drawAnnotations(rspfRefPtr<rspfImageData> tile);
   
   virtual void computeBorderRects();
   virtual void drawBorders();
   virtual void addGridLabels();
   
   virtual void addGeographicTopGridLabels();
   virtual void addGeographicBottomGridLabels();
   virtual void addGeographicLeftGridLabels();
   virtual void addGeographicRightGridLabels();

   virtual void addGeographicGridLines();
   virtual void addGeographicGridReseaux();
   
   virtual void addMeterGridLabels();
   
   virtual void addMeterGridLines();
   virtual void addMeterGridReseaux();

   
   virtual void addTitle();
   
   virtual void layoutAnnotations();

   virtual void addFixedAnnotation(rspfAnnotationObject* obj);
      
   virtual void deleteFixedAnnotations();
   
   rspfIrect getViewingRect()const;
   
   //! Fetches the input connection's image geometry and verifies that it is a map projection.
   //! Returns NULL if no valid projection found.
   const rspfMapProjection* inputMapProjection() const;

// For RTTI
TYPE_DATA
};
#endif /* #ifndef rspfMapCompositionSource_HEADER */

