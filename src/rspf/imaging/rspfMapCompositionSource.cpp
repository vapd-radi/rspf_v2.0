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
// $Id: rspfMapCompositionSource.cpp 21631 2012-09-06 18:10:55Z dburken $

#include <rspf/imaging/rspfMapCompositionSource.h>
#include <rspf/imaging/rspfU8ImageData.h>
#include <rspf/imaging/rspfImageDataHelper.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/imaging/rspfAnnotationFontObject.h>
#include <rspf/imaging/rspfAnnotationLineObject.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/font/rspfFontFactoryRegistry.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfDms.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfColorProperty.h>
#include <rspf/base/rspfContainerProperty.h>
#include <rspf/base/rspfFontProperty.h>
#include <rspf/base/rspfBooleanProperty.h>
#include <rspf/base/rspfTextProperty.h>
#include <rspf/base/rspfNumericProperty.h>
#include <rspf/base/rspfStringProperty.h>
#include <sstream>

using namespace std;

static const char* GRID_TYPE_ENUM_NAMES[]     = {"none", "line", "reseaux"};
static const char* VIEW_WIDTH_KW              = "view_width";
static const char* VIEW_HEIGHT_KW             = "view_height";
static const char* METER_GRID_SPACING_X_KW    = "meter_grid_spacing_x";
static const char* METER_GRID_SPACING_Y_KW    = "meter_grid_spacing_y";
static const char* GEO_GRID_SPACING_LON_KW    = "geo_grid_spacing_lon";
static const char* GEO_GRID_SPACING_LAT_KW    = "geo_grid_spacing_lat";
static const char* METER_GRID_TYPE_KW         = "meter_grid_type";
static const char* GEO_GRID_TYPE_KW           = "geo_grid_type";
static const char* TOP_BORDER_LENGTH_KW       = "top_border_length";
static const char* BOTTOM_BORDER_LENGTH_KW    = "bottom_border_length";
static const char* LEFT_BORDER_LENGTH_KW      = "top_border_length";
static const char* RIGHT_BORDER_LENGTH_KW     = "right_border_length";
static const char* BORDER_COLOR_KW            = "border_color";
static const char* GEO_GRID_COLOR_KW          = "geo_grid_color";
static const char* METER_GRID_COLOR_KW        = "meter_grid_color";
static const char* TITLE_STRING_KW            = "title_string";
static const char* TITLE_COLOR_KW             = "title_color";

static const char* TOP_GEO_LABEL_COLOR_KW     = "top_geo_label_color";
static const char* BOTTOM_GEO_LABEL_COLOR_KW  = "bottom_geo_label_color";
static const char* LEFT_GEO_LABEL_COLOR_KW    = "left_geo_label_color";
static const char* RIGHT_GEO_LABEL_COLOR_KW   = "right_geo_label_color";

static const char* TOP_METER_LABEL_COLOR_KW     = "top_meter_label_color";
static const char* BOTTOM_METER_LABEL_COLOR_KW  = "bottom_meter_label_color";
static const char* LEFT_METER_LABEL_COLOR_KW    = "left_meter_label_color";
static const char* RIGHT_METER_LABEL_COLOR_KW   = "right_meter_label_color";

static const char* TOP_GEO_LABEL_FORMAT_KW    = "top_geo_label_format";
static const char* BOTTOM_GEO_LABEL_FORMAT_KW = "bottom_geo_label_format";
static const char* LEFT_GEO_LABEL_FORMAT_KW   = "left_geo_label_format";
static const char* RIGHT_GEO_LABEL_FORMAT_KW  = "right_geo_label_format";

static const char* TOP_GEO_LABEL_FLAG_KW      = "top_geo_label_flag";
static const char* BOTTOM_GEO_LABEL_FLAG_KW   = "bottom_geo_label_flag";
static const char* LEFT_GEO_LABEL_FLAG_KW     = "left_geo_label_flag";
static const char* RIGHT_GEO_LABEL_FLAG_KW    = "right_geo_label_flag";

static const char* TOP_METER_LABEL_FLAG_KW    = "top_meter_label_flag";
static const char* BOTTOM_METER_LABEL_FLAG_KW = "bottom_meter_label_flag";
static const char* LEFT_METER_LABEL_FLAG_KW   = "left_meter_label_flag";
static const char* RIGHT_METER_LABEL_FLAG_KW  = "right_meter_label_flag";

static const char* TOP_GEO_LABEL_FONT_KW      = "top_geo_label_font";
static const char* BOTTOM_GEO_LABEL_FONT_KW   = "bottom_geo_label_font";
static const char* LEFT_GEO_LABEL_FONT_KW     = "left_geo_label_font";
static const char* RIGHT_GEO_LABEL_FONT_KW    = "right_geo_label_font";
static const char* TOP_METER_LABEL_FONT_KW    = "top_meter_label_font";
static const char* BOTTOM_METER_LABEL_FONT_KW = "bottom_meter_label_font";
static const char* LEFT_METER_LABEL_FONT_KW   = "left_meter_label_font";
static const char* RIGHT_METER_LABEL_FONT_KW  = "right_meter_label_font";
static const char* TITLE_FONT_KW              = "title_font";

static rspfTrace traceDebug("rspfMapCompositionSource:debug");

RTTI_DEF1(rspfMapCompositionSource, "rspfMapCompositionSource", rspfAnnotationSource);
rspfMapCompositionSource::rspfMapCompositionSource()
   :rspfAnnotationSource(),
    theMeterGridType(RSPF_GRID_NONE),
    theGeographicGridType(RSPF_GRID_LINE),
    theTopBorderLength(200),
    theBottomBorderLength(200),
    theLeftBorderLength(300),
    theRightBorderLength(300),
    theBorderColor(255,255,255),
    theGeographicGridColor(255,255,255),
    theMeterGridColor(255,255,255),
    theTitleString(""),
    theTitleFont(NULL),
    theTitleColor(1, 1, 1),
    theTopGeographicLabelColor(1, 1, 1),
    theBottomGeographicLabelColor(1, 1, 1),
    theLeftGeographicLabelColor(1, 1, 1),
    theRightGeographicLabelColor(1, 1, 1),
    theTopMeterLabelColor(1, 1, 1),
    theBottomMeterLabelColor(1, 1, 1),
    theLeftMeterLabelColor(1, 1, 1),
    theRightMeterLabelColor(1, 1, 1),
    theGeographicTopLabelFont(NULL),
    theGeographicBottomLabelFont(NULL),
    theGeographicLeftLabelFont(NULL),
    theGeographicRightLabelFont(NULL),
    theMeterTopLabelFont(NULL),
    theMeterBottomLabelFont(NULL),
    theMeterLeftLabelFont(NULL),
    theMeterRightLabelFont(NULL),
    theTopGeographicFormat("dd@mm'ss\".ssssC"),
    theBottomGeographicFormat("dd@mm'ss\".ssssC"),
    theLeftGeographicFormat("dd@mm'ss\".ssssC"),
    theRightGeographicFormat("dd@mm'ss\".ssssC"),
    theTopGeographicLabelFlag(true),
    theBottomGeographicLabelFlag(true),
    theLeftGeographicLabelFlag(true),
    theRightGeographicLabelFlag(true),
    theTopGeographicTickFlag(true),
    theBottomGeographicTickFlag(true),
    theLeftGeographicTickFlag(true),
    theRightGeographicTickFlag(true),
    theTopMeterLabelFlag(false),
    theBottomMeterLabelFlag(false),
    theLeftMeterLabelFlag(false),
    theRightMeterLabelFlag(false),
    theTopMeterTickFlag(false),
    theBottomMeterTickFlag(false),
    theLeftMeterTickFlag(false),
    theRightMeterTickFlag(false),
    theGeographicSpacing(1.0, 1.0),
    theMeterSpacing(3600*30, 3600*30)
{
   theViewWidthHeight = rspfIpt(-1,-1);
   vector<rspfFontInformation> info;
   rspfFontFactoryRegistry::instance()->getFontInformation(info);

   if(info.size())
   {
      theGeographicTopLabelFont = rspfFontFactoryRegistry::instance()->createFont(info[0]);
      theGeographicTopLabelFontInfo = info[0];
      theGeographicTopLabelFontInfo.thePointSize = rspfIpt(12,12);

      if(theGeographicTopLabelFont.valid())
      {
         theGeographicBottomLabelFont     = (rspfFont*)theGeographicTopLabelFont->dup();
         theGeographicBottomLabelFontInfo = theGeographicTopLabelFontInfo;
         theGeographicLeftLabelFont       = (rspfFont*)theGeographicTopLabelFont->dup();
         theGeographicLeftLabelFontInfo   = theGeographicTopLabelFontInfo;
         theGeographicRightLabelFont      = (rspfFont*)theGeographicTopLabelFont->dup();
         theGeographicRightLabelFontInfo  = theGeographicTopLabelFontInfo;

         theMeterTopLabelFont             = (rspfFont*)theGeographicTopLabelFont->dup();
         theMeterTopLabelFontInfo         = theGeographicTopLabelFontInfo;
         theMeterBottomLabelFont          = (rspfFont*)theGeographicTopLabelFont->dup();
         theMeterBottomLabelFontInfo      = theGeographicTopLabelFontInfo;
         theMeterLeftLabelFont            = (rspfFont*)theGeographicTopLabelFont->dup();
         theMeterLeftLabelFontInfo        = theGeographicTopLabelFontInfo;
         theMeterRightLabelFont           = (rspfFont*)theGeographicTopLabelFont->dup();
         theMeterRightLabelFontInfo       = theGeographicTopLabelFontInfo;
         
         theTitleFont                     = (rspfFont*)theGeographicTopLabelFont->dup();
         theTitleFontInfo                 = theGeographicTopLabelFontInfo;
         theTitleFontInfo.thePointSize    = rspfIpt(48, 48);
      }
   }
}

rspfMapCompositionSource::~rspfMapCompositionSource()
{
   theGeographicTopLabelFont = 0;
   theGeographicBottomLabelFont = 0;
   theGeographicLeftLabelFont = 0;
   theGeographicRightLabelFont = 0;
   theMeterTopLabelFont = 0;
   theMeterBottomLabelFont = 0;
   theMeterLeftLabelFont = 0;

   theMeterRightLabelFont = 0;
   theTitleFont = 0;
   deleteFixedAnnotations();
}

rspfRefPtr<rspfImageData> rspfMapCompositionSource::getTile(const rspfIrect& tileRect,
                                                               rspf_uint32 resLevel)
{
   if(!theTile)
   {
      allocate(tileRect);
      if(!theTile)
      {
         if(theInputConnection)
         {
            return theInputConnection->getTile(tileRect, resLevel);
         }
      }
   }
   theTile->setImageRectangle(tileRect);
   theTile->makeBlank();
   
   if(theInputConnection)
   {
      rspfRefPtr<rspfImageData> tile = theInputConnection->getTile(tileRect,
                                                                     resLevel);

      if(tile.valid())
      {
         if(tile->getScalarType() != RSPF_UCHAR)
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfMapCompositionSource::getTile\n"
               << "Error input is not uchar" << endl;
            return tile;
         }
         else
         {
            if(tile->getBuf()&&(tile->getDataObjectStatus() != RSPF_EMPTY))
            {
               int band = 0;
               int outBands = theTile->getNumberOfBands();
               int minBand = std::min((long)theTile->getNumberOfBands(),
                                      (long)tile->getNumberOfBands());
               for(band = 0; band < minBand; ++band)
               {
                  theTile->loadBand(tile->getBuf(band),
                                    tile->getImageRectangle(),
                                    band);
               }
               // copy last band of input tile to all output
               // bands if the input tile did not have enough bands.
               //
               for(;band < outBands;++band)
               {
                  theTile->loadBand(tile->getBuf(minBand-1),
                                    tile->getImageRectangle(),
                                    band);
               }
            }
	    drawBorders();
	    drawAnnotations(theTile);
         }
      }
   }
   theTile->validate();
   return theTile;
}

void rspfMapCompositionSource::setGeographicLabelFormat(const rspfString format)
{
   if( (format == theTopGeographicFormat)&&
       (format == theBottomGeographicFormat)&&
       (format == theLeftGeographicFormat)&&
       (format == theRightGeographicFormat))
   {
      return;
   }
   theTopGeographicFormat    = format;
   theBottomGeographicFormat = format;
   theLeftGeographicFormat   = format;
   theRightGeographicFormat  = format;
}

void rspfMapCompositionSource::setTopGeographicLabelFormat(const rspfString& format)
{
   if(format == theTopGeographicFormat)
   {
      return;
   }
   theTopGeographicFormat = format;
}

void rspfMapCompositionSource::setBottomGeographicLabelFormat(const rspfString& format)
{
   if(format == theBottomGeographicFormat)
   {
      return;
   }
   theBottomGeographicFormat = format;

   // need to reset the fonts
//   layoutAnnotations();
}

void rspfMapCompositionSource::setLeftGeographicLabelFormat(const rspfString& format)
{
   if(format == theLeftGeographicFormat)
   {
      return;
   }
   theLeftGeographicFormat = format;

   // need to reset the fonts
//   layoutAnnotations();
}

void rspfMapCompositionSource::setRightGeographicLabelFormat(const rspfString& format)
{
   if(format == theRightGeographicFormat)
   {
      return;
   }
   theRightGeographicFormat = format;

}

void rspfMapCompositionSource::setTitleFont(const rspfFontInformation& fontInfo)
{
   if(theTitleFontInfo == fontInfo)
   {
      return;
   }
   rspfFont* font = rspfFontFactoryRegistry::instance()->createFont(fontInfo);
   if(font)
   {
 
      theTitleFont     = font;
      theTitleFontInfo = fontInfo;      
   }
}

void rspfMapCompositionSource::setGeographicTopLabelFont(const rspfFontInformation& fontInfo)
{
   const char* MODULE = "rspfMapCompositionSource::setGeographicTopLabelFont";
   
   if(traceDebug())
   {
      CLOG << "Entering...." <<endl;
   }
   if(theGeographicTopLabelFontInfo == fontInfo)
   {
      if(traceDebug())
      {
         CLOG << "Font info the same exiting...." <<endl
              << theGeographicTopLabelFontInfo << endl;
      }
      
      return;
   }
   rspfFont* font = rspfFontFactoryRegistry::instance()->createFont(fontInfo);

   if(font)
   {
      if(traceDebug())
      {
         CLOG << "Previous font info " <<endl
              << theGeographicTopLabelFont.get() << endl;
      }
      theGeographicTopLabelFont = font;
      theGeographicTopLabelFontInfo = fontInfo;

      if(traceDebug())
      {
         CLOG << "New font info " <<endl
              << theGeographicTopLabelFont.get() << endl;
      }      
   }
}

void rspfMapCompositionSource::setGeographicBottomLabelFont(const rspfFontInformation& fontInfo)
{
   if(theGeographicBottomLabelFontInfo == fontInfo)
   {
      return;
   }
   rspfFont* font = rspfFontFactoryRegistry::instance()->createFont(fontInfo);

   if(font)
   {
      theGeographicBottomLabelFont     = font;
      theGeographicBottomLabelFontInfo = fontInfo;

   }
}

void rspfMapCompositionSource::setGeographicLeftLabelFont(const rspfFontInformation& fontInfo)
{
   if(theGeographicRightLabelFontInfo == fontInfo)
   {
      return;
   }
   rspfFont* font = rspfFontFactoryRegistry::instance()->createFont(fontInfo);

   if(font)
   {
      theGeographicLeftLabelFont     = font;
      theGeographicLeftLabelFontInfo = fontInfo;

   }
}

void rspfMapCompositionSource::setGeographicRightLabelFont(const rspfFontInformation& fontInfo)
{
   if(theGeographicRightLabelFontInfo == fontInfo)
   {
      return;
   }
   
   rspfFont* font = rspfFontFactoryRegistry::instance()->createFont(fontInfo);

   if(font)
   {
      theGeographicRightLabelFont = font;
      theGeographicRightLabelFontInfo = fontInfo;
   }
}


void rspfMapCompositionSource::setMeterTopLabelFont(const rspfFontInformation& fontInfo)
{
   if(theMeterTopLabelFontInfo == fontInfo)
   {
      return;
   }
   rspfFont* font = rspfFontFactoryRegistry::instance()->createFont(fontInfo);

   if(font)
   {
      theMeterTopLabelFont     = font;
      theMeterTopLabelFontInfo = fontInfo;
   }
}

void rspfMapCompositionSource::setMeterBottomLabelFont(const rspfFontInformation& fontInfo)
{
   if(theMeterBottomLabelFontInfo == fontInfo)
   {
      return;
   }
   rspfFont* font = rspfFontFactoryRegistry::instance()->createFont(fontInfo);

   if(font)
   {
      theMeterBottomLabelFont     = font;
      theMeterBottomLabelFontInfo = fontInfo;
   }
}

void rspfMapCompositionSource::setMeterLeftLabelFont(const rspfFontInformation& fontInfo)
{
   if(theMeterLeftLabelFontInfo == fontInfo)
   {
      return;
   }
   rspfFont* font = rspfFontFactoryRegistry::instance()->createFont(fontInfo);

   if(font)
   {
      theMeterLeftLabelFont     = font;
      theMeterLeftLabelFontInfo = fontInfo;
   }
}

void rspfMapCompositionSource::setMeterRightLabelFont(const rspfFontInformation& fontInfo)
{
   if(theMeterRightLabelFontInfo == fontInfo)
   {
      return;
   }
   rspfFont* font = rspfFontFactoryRegistry::instance()->createFont(fontInfo);

   if(font)
   {
      theMeterRightLabelFont     = font;
      theMeterRightLabelFontInfo = fontInfo;
   }
}

rspfIrect rspfMapCompositionSource::getBoundingRect(rspf_uint32 resLevel)const
{
   rspfIrect result;

   result.makeNan();
   if(theInputConnection)
   {
      rspfIrect inputRect  = getViewingRect();

      if(!inputRect.hasNans())
      {
         rspfDpt decimation;
         getDecimationFactor(resLevel,
                             decimation);
         int leftBorderLength   = theLeftBorderLength;
         int rightBorderLength  = theRightBorderLength;
         int topBorderLength    = theTopBorderLength;
         int bottomBorderLength = theBottomBorderLength;

         rspfIpt p1(inputRect.ul().x - leftBorderLength,
                     inputRect.ul().y - topBorderLength);
         
         rspfIpt p2(inputRect.ur().x + rightBorderLength,
                     inputRect.ur().y - topBorderLength);
         
         rspfIpt p3(inputRect.lr().x + rightBorderLength,
                     inputRect.lr().y + bottomBorderLength);
         
         rspfIpt p4(inputRect.ll().x - leftBorderLength,
                     inputRect.ll().y + bottomBorderLength);

         result = rspfIrect(p1, p2, p3, p4);
         
         if(!decimation.hasNans())
         {
            result*=decimation;
         }
      }
   }
   return result;
}

void rspfMapCompositionSource::layoutAnnotations()
{
   deleteFixedAnnotations();

   addTitle();
   addGridLabels();
   if(theGeographicGridType == RSPF_GRID_LINE)
   {
      addGeographicGridLines();
   }
   else if(theGeographicGridType == RSPF_GRID_RESEAUX)
   {
      addGeographicGridReseaux();
   }
   
   if(theMeterGridType == RSPF_GRID_LINE)
   {
      addMeterGridLines();
   }
   else if(theMeterGridType == RSPF_GRID_RESEAUX)
   {
      addMeterGridReseaux();
   }
}


void rspfMapCompositionSource::initialize()
{
   rspfAnnotationSource::initialize();   

   theTile = 0;
   if(theInputConnection)
   {
      computeBorderRects();
      layoutAnnotations();
   }
   else 
   {
      deleteFixedAnnotations();
   }
}

void rspfMapCompositionSource::computeBorderRects()
{
   if(theInputConnection)
   {
      rspfIrect inputRect = getViewingRect();;
      
      theTopBorder = rspfIrect(inputRect.ul().x - theLeftBorderLength,
                                inputRect.ul().y - theTopBorderLength,
                                inputRect.ur().x + theRightBorderLength,
                                inputRect.ur().y);
      
      theBottomBorder = rspfIrect(inputRect.ll().x - theLeftBorderLength,
                                   inputRect.ll().y,
                                   inputRect.lr().x + theRightBorderLength,
                                   inputRect.lr().y + theBottomBorderLength);
      
      theLeftBorder   = rspfIrect(theTopBorder.ul().x,
                                   theTopBorder.ll().y,
                                   inputRect.ll().x,
                                   inputRect.ll().y);
      
      theRightBorder   = rspfIrect(inputRect.ur().x,
                                    inputRect.ur().y,
                                    theBottomBorder.lr().x,
                                    inputRect.lr().y);
   }
   else
   {
      theTopBorder.makeNan();
      theBottomBorder.makeNan();
      theLeftBorder.makeNan();
      theRightBorder.makeNan();
   }
      
}

void rspfMapCompositionSource::drawBorders()
{
   if(theTile.valid())
   {
      rspfIrect top    = theTopBorder;
      rspfIrect bottom = theBottomBorder;
      rspfIrect left   = theLeftBorder;
      rspfIrect right  = theRightBorder;

//       if(resLevel)
//       {
//          rspfDpt decimation;
//          getDecimationFactor(resLevel,
//                              decimation);

//          if(!decimation.hasNans())
//          {
//             top*=decimation;
//             bottom*=decimation;
//             left*=decimation;
//             right*=decimation;            
//          }
//       }
      rspfIrect tileRect = theTile->getImageRectangle();
      
      rspfImageDataHelper helper(theTile.get());
      
      if(top.intersects(tileRect))
      {
         helper.fill(theBorderColor,
                     top.clipToRect(tileRect),false);
      }
      if(bottom.intersects(tileRect))
      {
         helper.fill(theBorderColor,
                     bottom.clipToRect(tileRect),false);
      }
      if(left.intersects(tileRect))
      {
         helper.fill(theBorderColor,
                     left.clipToRect(tileRect),false);
      }
      if(right.intersects(tileRect))
      {
         helper.fill(theBorderColor,
                     right.clipToRect(tileRect),false);
      }
   }
}

void rspfMapCompositionSource::addGridLabels()
{
   addGeographicTopGridLabels();
   addGeographicBottomGridLabels();
   addGeographicLeftGridLabels();
   addGeographicRightGridLabels();

   addMeterGridLabels();
}

void rspfMapCompositionSource::addGeographicTopGridLabels()
{
   const char* MODULE = "rspfMapCompositionSource::addGeographicTopGridLabels";
   if(traceDebug())
   {
      CLOG << "Entering...." << endl;
   }
   if((!theTopGeographicLabelFlag)&&(!theTopGeographicTickFlag))
   {
      if(traceDebug())
      {
         CLOG << "Exiting, flags are false...." << endl;
      }
      return;
   }
   
   if(theInputConnection)
   { 
      const rspfMapProjection* mapProj = inputMapProjection();
      rspfGpt gpt[4];
      rspfDrect rect = getViewingRect();

      if(rect.hasNans()) return;
      mapProj->lineSampleToWorld(rect.ul(), gpt[0]);
      mapProj->lineSampleToWorld(rect.ur(), gpt[1]);
      mapProj->lineSampleToWorld(rect.lr(), gpt[2]);
      mapProj->lineSampleToWorld(rect.ll(), gpt[3]);
      
      rspfDrect grect(gpt[0], gpt[1], gpt[2], gpt[3], RSPF_RIGHT_HANDED);

      if(grect.hasNans()) return;
      rspfDpt ulLatLon(((int)((grect.ul().x-theGeographicSpacing.x)/theGeographicSpacing.x))*theGeographicSpacing.x,
                        ((int)((grect.ul().y+theGeographicSpacing.y)/theGeographicSpacing.y))*theGeographicSpacing.y);
      
      rspfDpt lrLatLon(((int)((grect.lr().x+theGeographicSpacing.x)/theGeographicSpacing.x))*theGeographicSpacing.x,
                        ((int)((grect.lr().y-theGeographicSpacing.y)/theGeographicSpacing.y))*theGeographicSpacing.y);

      while(ulLatLon.lat > 90)
      {
         ulLatLon.lat -= theGeographicSpacing.lat;
      }
      while(ulLatLon.lon < -180)
      {
         ulLatLon.lon += theGeographicSpacing.lon;
      }
      while(lrLatLon.lat < -90)
      {
         ulLatLon.lat += theGeographicSpacing.lat;
      }
      while(lrLatLon.lon > 180)
      {
         ulLatLon.lon -= theGeographicSpacing.lon;
      }
      
      rspfDrect latLonSpacing(ulLatLon,
                               lrLatLon,
                               RSPF_RIGHT_HANDED);

      if(traceDebug())
      {
         CLOG << "labeling bounding geo rect = " << latLonSpacing << endl;
      }
      // do the top side labels.
      for(double lon = latLonSpacing.ul().x; lon <= latLonSpacing.lr().x; lon+=theGeographicSpacing.x)
      {
         rspfDpt tipt; // top
         rspfDpt bipt; // bottom
         
         rspfGpt tgpt(latLonSpacing.ul().y,
                       lon,
                       0.0,
                       gpt[0].datum());

         rspfGpt bgpt(latLonSpacing.lr().y,
                       lon,
                       0.0,
                       gpt[0].datum());
         
         mapProj->worldToLineSample(tgpt, tipt);
         mapProj->worldToLineSample(bgpt, bipt);

         
         if((!tipt.hasNans()&& !bipt.hasNans()) && rect.clip(tipt, bipt))
         {
            rspfIpt rounded(tipt);
            if( (rounded.x >= rect.ul().x)&&
                (rounded.x <= rect.lr().x))
            {
               if(theTopGeographicLabelFlag)
               {
                  rspfDms dms(tgpt.lond(), false);
               
                  rspfString dmsString = dms.toString(theTopGeographicFormat.c_str());
                  rspfAnnotationFontObject* lonLabel = 
                     new rspfAnnotationFontObject(rspfIpt(0,0), dmsString);
                  lonLabel->setFont(theGeographicTopLabelFont.get());
                  lonLabel->setGeometryInformation(theGeographicTopLabelFontInfo);
                  lonLabel->computeBoundingRect();
                  rspfDrect boundsD;
                  lonLabel->getBoundingRect(boundsD);
                  
                  rspfIpt center( rounded.x, rspf::round<int>(rect.ul().y-(boundsD.height()/2)));
               
                  lonLabel->setColor(theTopGeographicLabelColor.getR(), theTopGeographicLabelColor.getG(), theTopGeographicLabelColor.getB());
                  lonLabel->setCenterPosition(center);
                  lonLabel->computeBoundingRect();

                  rspfDrect labelRect;
                  lonLabel->getBoundingRect(labelRect);
                  
                  if(rect.intersects(labelRect))
                  {
                     rspfIrect intersection = rect.clipToRect(labelRect);
                     center.y -= rspf::round<int>(rect.ul().y - intersection.lr().y);
                  }

                  center.y -= 24; // this will need to be the tick height later;
                  lonLabel->setCenterPosition(center);
                  lonLabel->computeBoundingRect();
                  
                  addFixedAnnotation(lonLabel);
               }
               if(theTopGeographicTickFlag)
               {
                  rspfAnnotationLineObject* lineLabel = new rspfAnnotationLineObject
                     (rspfIpt(rounded.x,rspf::round<int>(rect.ul().y-24)),
                      rspfIpt(rounded.x, rspf::round<int>(rect.ul().y)));
                  lineLabel->setColor(theTopGeographicLabelColor.getR(), theTopGeographicLabelColor.getG(), theTopGeographicLabelColor.getB());
                  addFixedAnnotation(lineLabel);
               }
            }
         }
      }
   }
   if(traceDebug())
   {
      CLOG << "Exiting...." << endl;
   }
}

void rspfMapCompositionSource::addGeographicBottomGridLabels()
{
   if((!theBottomGeographicLabelFlag)&&(!theBottomGeographicTickFlag))
   {
      return;
   }

   const rspfMapProjection* mapProj = inputMapProjection();
   if(mapProj)
   { 
      rspfGpt gpt[4];
      rspfDrect rect = getViewingRect();

      if(rect.hasNans()) return;
      mapProj->lineSampleToWorld(rect.ul(), gpt[0]);
      mapProj->lineSampleToWorld(rect.ur(), gpt[1]);
      mapProj->lineSampleToWorld(rect.lr(), gpt[2]);
      mapProj->lineSampleToWorld(rect.ll(), gpt[3]);
      
      rspfDrect grect(gpt[0], gpt[1], gpt[2], gpt[3], RSPF_RIGHT_HANDED);

      if(grect.hasNans()) return;
      rspfDpt ulLatLon(((int)((grect.ul().x-theGeographicSpacing.x)/theGeographicSpacing.x))*theGeographicSpacing.x,
                        ((int)((grect.ul().y+theGeographicSpacing.y)/theGeographicSpacing.y))*theGeographicSpacing.y);
      
      rspfDpt lrLatLon(((int)((grect.lr().x+theGeographicSpacing.x)/theGeographicSpacing.x))*theGeographicSpacing.x,
                        ((int)((grect.lr().y-theGeographicSpacing.y)/theGeographicSpacing.y))*theGeographicSpacing.y);

      while(ulLatLon.lat > 90)
      {
         ulLatLon.lat -= theGeographicSpacing.lat;
      }
      while(ulLatLon.lon < -180)
      {
         ulLatLon.lon += theGeographicSpacing.lon;
      }
      while(lrLatLon.lat < -90)
      {
         ulLatLon.lat += theGeographicSpacing.lat;
      }
      while(lrLatLon.lon > 180)
      {
         ulLatLon.lon -= theGeographicSpacing.lon;
      }
      
      rspfDrect latLonSpacing(ulLatLon,
                               lrLatLon,
                               RSPF_RIGHT_HANDED);

      // do the top side labels.
      for(double lon = latLonSpacing.ul().x; lon <= latLonSpacing.lr().x; lon+=theGeographicSpacing.x)
      {
         rspfDpt tipt; // top
         rspfDpt bipt; // bottom
         
         rspfGpt tgpt(latLonSpacing.ul().y,
                       lon,
                       0.0,
                       gpt[0].datum());

         rspfGpt bgpt(latLonSpacing.lr().y,
                       lon,
                       0.0,
                       gpt[0].datum());
         
         mapProj->worldToLineSample(tgpt, tipt);
         mapProj->worldToLineSample(bgpt, bipt);

         if(rect.clip(tipt, bipt))
         {
            rspfIpt rounded(bipt);
            if( (rounded.x >= rect.ul().x)&&
                (rounded.x <= rect.lr().x))
            {
               if(theBottomGeographicLabelFlag)
               {
                  rspfDms dms(bgpt.lond(), false);
                  
                  rspfString dmsString = dms.toString(theBottomGeographicFormat.c_str());
                  rspfAnnotationFontObject* lonLabel = new rspfAnnotationFontObject(rspfIpt(0,0),
                                                                                      dmsString);
                  lonLabel->setGeometryInformation(theGeographicBottomLabelFontInfo);
                  lonLabel->setFont(theGeographicBottomLabelFont.get());
//                  lonLabel->setFont(theGeographicBottomLabelFont->dup());
                  
                  lonLabel->computeBoundingRect();
                  rspfDrect boundsD;
                  lonLabel->getBoundingRect(boundsD);
                  rspfIrect bounds = boundsD;
                  
                  rspfIpt center( rounded.x,
                                   rspf::round<int>(rect.lr().y+(bounds.height()/2)));
                  
                  lonLabel->setColor(theBottomGeographicLabelColor.getR(), theBottomGeographicLabelColor.getG(), theBottomGeographicLabelColor.getB());
                  lonLabel->setCenterPosition(center);
                  lonLabel->computeBoundingRect();
                  lonLabel->getBoundingRect(boundsD);
                  bounds = boundsD;

                  rspfDrect labelRect;
                  lonLabel->getBoundingRect(labelRect);
                  
                  if(rect.intersects(labelRect))
                  {
                     rspfIrect intersection = rect.clipToRect(labelRect);
                     center.y += rspf::round<int>(rect.lr().y - intersection.ul().y);
                  }

                  center.y += 24; // this will need to be the tick height later;
                  lonLabel->setCenterPosition(center);
                  lonLabel->computeBoundingRect();


                  addFixedAnnotation(lonLabel);
               }
               if(theBottomGeographicTickFlag)
               {
                  rspfAnnotationLineObject* lineLabel = new rspfAnnotationLineObject(rspfIpt(rounded.x,
                                                                                                rspf::round<int>(rect.lr().y+24)),
                                                                                       rspfIpt(rounded.x,
                                                                                                rspf::round<int>(rect.lr().y)));
                  lineLabel->setColor(theBottomGeographicLabelColor.getR(), theBottomGeographicLabelColor.getG(), theBottomGeographicLabelColor.getB());
                  addFixedAnnotation(lineLabel);
               }
            }         
         }
      }
   }   
}

void rspfMapCompositionSource::addGeographicLeftGridLabels()
{
   if((!theLeftGeographicLabelFlag)&&(!theLeftGeographicTickFlag))
   {
      return;
   }
   
   const rspfMapProjection* mapProj = inputMapProjection();
   if(mapProj)
   { 
      rspfGpt gpt[4];
      rspfDrect rect = getViewingRect();

      if(rect.hasNans()) return;
      mapProj->lineSampleToWorld(rect.ul(), gpt[0]);
      mapProj->lineSampleToWorld(rect.ur(), gpt[1]);
      mapProj->lineSampleToWorld(rect.lr(), gpt[2]);
      mapProj->lineSampleToWorld(rect.ll(), gpt[3]);
      
      rspfDrect grect(gpt[0],
                       gpt[1],
                       gpt[2],
                       gpt[3],
                       RSPF_RIGHT_HANDED);

      if(grect.hasNans()) return;
      rspfDpt ulLatLon(((int)((grect.ul().x-theGeographicSpacing.x)/theGeographicSpacing.x))*theGeographicSpacing.x,
                        ((int)((grect.ul().y+theGeographicSpacing.y)/theGeographicSpacing.y))*theGeographicSpacing.y);
      
      rspfDpt lrLatLon(((int)((grect.lr().x+theGeographicSpacing.x)/theGeographicSpacing.x))*theGeographicSpacing.x,
                        ((int)((grect.lr().y-theGeographicSpacing.y)/theGeographicSpacing.y))*theGeographicSpacing.y);

      while(ulLatLon.lat > 90)
      {
         ulLatLon.lat -= theGeographicSpacing.lat;
      }
      while(ulLatLon.lon < -180)
      {
         ulLatLon.lon += theGeographicSpacing.lon;
      }
      while(lrLatLon.lat < -90)
      {
         ulLatLon.lat += theGeographicSpacing.lat;
      }
      while(lrLatLon.lon > 180)
      {
         ulLatLon.lon -= theGeographicSpacing.lon;
      }
      
      rspfDrect latLonSpacing(ulLatLon,
                               lrLatLon,
                               RSPF_RIGHT_HANDED);
      // do the left side labels
      for(double lat = latLonSpacing.ul().y; lat >= latLonSpacing.lr().y; lat-=theGeographicSpacing.y)
      {
         rspfDpt lipt;
         rspfDpt ript;
         rspfGpt lgpt(lat,
                       latLonSpacing.ul().x,
                       0.0,
                       gpt[0].datum());
         rspfGpt rgpt(lat,
                       latLonSpacing.ur().x,
                       0.0,
                       gpt[0].datum());

         
         mapProj->worldToLineSample(lgpt, lipt);
         mapProj->worldToLineSample(rgpt, ript);
         if(rect.clip(lipt, ript))
         {
            rspfIpt rounded(lipt);
            if( (rounded.y >= rect.ul().y)&&
                (rounded.y <= rect.lr().y))
            {
               if(theLeftGeographicLabelFlag)
               {
                  rspfDms dms(lgpt.latd());
                  
                  rspfString dmsString = dms.toString(theLeftGeographicFormat.c_str());
                  rspfAnnotationFontObject* latLabel = new rspfAnnotationFontObject(rspfIpt(0,0),
                                                                                      dmsString);
                  latLabel->setGeometryInformation(theGeographicLeftLabelFontInfo);
                  latLabel->setFont(theGeographicLeftLabelFont.get());
//                  latLabel->setFont(theGeographicLeftLabelFont->dup());
                  
                  latLabel->computeBoundingRect();
                  rspfDrect boundsD;
                  latLabel->getBoundingRect(boundsD);
                  rspfIrect bounds = boundsD;
                  
                  rspfIpt center( rspf::round<int>((rect.ul().x - (bounds.width()/2))),
                                   rounded.y);
                  
                  latLabel->setColor(theLeftGeographicLabelColor.getR(), theLeftGeographicLabelColor.getG(), theLeftGeographicLabelColor.getB());
                  latLabel->setCenterPosition(center);
                  latLabel->computeBoundingRect();
                  latLabel->getBoundingRect(boundsD);
                  bounds = boundsD;

                  rspfDrect labelRect;
                  latLabel->getBoundingRect(labelRect);
                  
                  if(rect.intersects(labelRect))
                  {
                     rspfIrect intersection = rect.clipToRect(labelRect);
                     center.x -= rspf::round<int>(rect.ul().x - intersection.ur().x);
                  }

                  center.x -= 24; // this will need to be the tick height later;
                  latLabel->setCenterPosition(center);
                  latLabel->computeBoundingRect();


                  
                  addFixedAnnotation(latLabel);
               }
               
               if(theLeftGeographicTickFlag)
               {
                  rspfAnnotationLineObject* lineLabel = new rspfAnnotationLineObject(rspfIpt(rspf::round<int>(rect.ul().x),
                                                                                                rounded.y),
                                                                                       rspfIpt(rspf::round<int>(rect.ul().x-23),
                                                                                                rounded.y));
                  lineLabel->setColor(theLeftGeographicLabelColor.getR(), theLeftGeographicLabelColor.getG(), theLeftGeographicLabelColor.getB());
                  addFixedAnnotation(lineLabel);
               }
            }
         }
      }
   }
}

void rspfMapCompositionSource::addGeographicRightGridLabels()
{
   if((!theRightGeographicLabelFlag)&&(!theRightGeographicTickFlag))
   {
      return;
   }

   const rspfMapProjection* mapProj = inputMapProjection();
   if(mapProj)
   { 
      rspfGpt gpt[4];
      rspfDrect rect = getViewingRect();

      if(rect.hasNans()) return;
      mapProj->lineSampleToWorld(rect.ul(), gpt[0]);
      mapProj->lineSampleToWorld(rect.ur(), gpt[1]);
      mapProj->lineSampleToWorld(rect.lr(), gpt[2]);
      mapProj->lineSampleToWorld(rect.ll(), gpt[3]);
      
      rspfDrect grect(gpt[0],
                       gpt[1],
                       gpt[2],
                       gpt[3],
                       RSPF_RIGHT_HANDED);

      if(grect.hasNans()) return;
      rspfDpt ulLatLon(((int)((grect.ul().x-theGeographicSpacing.x)/theGeographicSpacing.x))*theGeographicSpacing.x,
                        ((int)((grect.ul().y+theGeographicSpacing.y)/theGeographicSpacing.y))*theGeographicSpacing.y);
      
      rspfDpt lrLatLon(((int)((grect.lr().x+theGeographicSpacing.x)/theGeographicSpacing.x))*theGeographicSpacing.x,
                        ((int)((grect.lr().y-theGeographicSpacing.y)/theGeographicSpacing.y))*theGeographicSpacing.y);

      while(ulLatLon.lat > 90)
      {
         ulLatLon.lat -= theGeographicSpacing.lat;
      }
      while(ulLatLon.lon < -180)
      {
         ulLatLon.lon += theGeographicSpacing.lon;
      }
      while(lrLatLon.lat < -90)
      {
         ulLatLon.lat += theGeographicSpacing.lat;
      }
      while(lrLatLon.lon > 180)
      {
         ulLatLon.lon -= theGeographicSpacing.lon;
      }
      
      rspfDrect latLonSpacing(ulLatLon,
                               lrLatLon,
                               RSPF_RIGHT_HANDED);
      // do the left side labels
      for(double lat = latLonSpacing.ul().y; lat >= latLonSpacing.lr().y; lat-=theGeographicSpacing.y)
      {
         rspfDpt lipt;
         rspfDpt ript;
         rspfGpt lgpt(lat,
                       latLonSpacing.ul().x,
                       0.0,
                       gpt[0].datum());
         rspfGpt rgpt(lat,
                       latLonSpacing.ur().x,
                       0.0,
                       gpt[0].datum());
         
         
         mapProj->worldToLineSample(lgpt, lipt);
         mapProj->worldToLineSample(rgpt, ript);
         
         if(rect.clip(lipt, ript))
         {
            rspfIpt rounded(ript);
            if( (rounded.y >= rect.ul().y)&&
                (rounded.y <= rect.lr().y))
            {
               if(theRightGeographicLabelFlag)
               {
                  rspfDms dms(rgpt.latd());
                  
                  rspfString dmsString = dms.toString(theRightGeographicFormat.c_str());
                  rspfAnnotationFontObject* latLabel = new rspfAnnotationFontObject(rspfIpt(0,0),
                                                                                      dmsString);
                  latLabel->setGeometryInformation(theGeographicRightLabelFontInfo);
                  latLabel->setFont(theGeographicRightLabelFont.get());
//                  latLabel->setFont(theGeographicRightLabelFont->dup());
                  latLabel->computeBoundingRect();
                  rspfDrect boundsD;
                  latLabel->getBoundingRect(boundsD);
                  rspfIrect bounds = boundsD;
                  
                  rspfIpt center( rspf::round<int>((rect.ur().x + (bounds.width()/2))),
                                   rounded.y);
               
                  latLabel->setColor(theRightGeographicLabelColor.getR(), theRightGeographicLabelColor.getG(), theRightGeographicLabelColor.getB());
                  latLabel->setCenterPosition(center);
                  latLabel->computeBoundingRect();
                  latLabel->getBoundingRect(boundsD);
                  bounds = boundsD;

                  rspfDrect labelRect;
                  latLabel->getBoundingRect(labelRect);
                  
                  if(rect.intersects(labelRect))
                  {
                     rspfIrect intersection = rect.clipToRect(labelRect);
                     center.x -= rspf::round<int>(rect.ur().x - intersection.ul().x);
                  }

                  center.x += 24; // this will need to be the tick height later;
                  latLabel->setCenterPosition(center);
                  latLabel->computeBoundingRect();
                  

                  addFixedAnnotation(latLabel);
               }
               
               if(theRightGeographicTickFlag)
               {
                  rspfAnnotationLineObject* lineLabel = new rspfAnnotationLineObject(rspfIpt(rspf::round<int>(rect.lr().x),
                                                                                                rounded.y),
                                                                                       rspfIpt(rspf::round<int>(rect.lr().x+23),
                                                                                                rounded.y));
                  
                  lineLabel->setColor(theRightGeographicLabelColor.getR(), theRightGeographicLabelColor.getG(), theRightGeographicLabelColor.getB());
                  addFixedAnnotation(lineLabel);
               }
            }
         }
      }
   }
}

void rspfMapCompositionSource::addGeographicGridLines()
{
   const rspfMapProjection* mapProj = inputMapProjection();
   if(mapProj)
   { 
      rspfGpt gpt[4];
      rspfDrect rect = getViewingRect();

      if(rect.hasNans()) return;
      mapProj->lineSampleToWorld(rect.ul(), gpt[0]);
      mapProj->lineSampleToWorld(rect.ur(), gpt[1]);
      mapProj->lineSampleToWorld(rect.lr(), gpt[2]);
      mapProj->lineSampleToWorld(rect.ll(), gpt[3]);
      
      rspfDrect grect(gpt[0],
                       gpt[1],
                       gpt[2],
                       gpt[3],
                       RSPF_RIGHT_HANDED);

      if(grect.hasNans()) return;
      rspfDpt ulLatLon(((int)((grect.ul().x-theGeographicSpacing.x)/theGeographicSpacing.x))*theGeographicSpacing.x,
                        ((int)((grect.ul().y+theGeographicSpacing.y)/theGeographicSpacing.y))*theGeographicSpacing.y);
      
      rspfDpt lrLatLon(((int)((grect.lr().x+theGeographicSpacing.x)/theGeographicSpacing.x))*theGeographicSpacing.x,
                        ((int)((grect.lr().y-theGeographicSpacing.y)/theGeographicSpacing.y))*theGeographicSpacing.y);

      while(ulLatLon.lat > 90)
      {
         ulLatLon.lat -= theGeographicSpacing.lat;
      }
      while(ulLatLon.lon < -180)
      {
         ulLatLon.lon += theGeographicSpacing.lon;
      }
      while(lrLatLon.lat < -90)
      {
         ulLatLon.lat += theGeographicSpacing.lat;
      }
      while(lrLatLon.lon > 180)
      {
         ulLatLon.lon -= theGeographicSpacing.lon;
      }
      
      rspfDrect latLonSpacing(ulLatLon,
                               lrLatLon,
                               RSPF_RIGHT_HANDED);


      for(double lon = latLonSpacing.ul().x; lon <= latLonSpacing.lr().x; lon+=theGeographicSpacing.x)
      {
         rspfDpt tipt; // top
         rspfDpt bipt; // bottom
         
         rspfGpt tgpt(latLonSpacing.ul().y,
                       lon,
                       0.0,
                       gpt[0].datum());

         rspfGpt bgpt(latLonSpacing.lr().y,
                       lon,
                       0.0,
                       gpt[0].datum());
         
         mapProj->worldToLineSample(tgpt, tipt);
         mapProj->worldToLineSample(bgpt, bipt);

         if(rect.clip(tipt, bipt))
         {
            rspfAnnotationLineObject* line = new rspfAnnotationLineObject(tipt, bipt);
            line->setColor(theGeographicGridColor.getR(),
                           theGeographicGridColor.getG(),
                           theGeographicGridColor.getB());
            addFixedAnnotation(line);
         }
      }
      // do the left side labels
      for(double lat = latLonSpacing.ul().y; lat >= latLonSpacing.lr().y; lat-=theGeographicSpacing.y)
      {
         rspfDpt lipt;
         rspfDpt ript;
         rspfGpt lgpt(lat,
                       latLonSpacing.ul().x,
                       0.0,
                       gpt[0].datum());
         rspfGpt rgpt(lat,
                       latLonSpacing.ur().x,
                       0.0,
                       gpt[0].datum());

         mapProj->worldToLineSample(lgpt, lipt);
         mapProj->worldToLineSample(rgpt, ript);
         
         if(rect.clip(lipt, ript))
         {
            rspfAnnotationLineObject* line = new rspfAnnotationLineObject(lipt,
                                                                            ript);
         
            line->setColor(theGeographicGridColor.getR(),
                                theGeographicGridColor.getG(),
                                theGeographicGridColor.getB());
         
            addFixedAnnotation(line);
         }
      }
   }   
}

void rspfMapCompositionSource::addGeographicGridReseaux()
{
   const rspfMapProjection* mapProj = inputMapProjection();
   if(mapProj)
   { 
      rspfGpt gptArray[4];
      rspfDrect rect = getViewingRect();
      
      mapProj->lineSampleToWorld(rect.ul(), gptArray[0]);
      mapProj->lineSampleToWorld(rect.ur(), gptArray[1]);
      mapProj->lineSampleToWorld(rect.lr(), gptArray[2]);
      mapProj->lineSampleToWorld(rect.ll(), gptArray[3]);
      
      rspfDrect grect(gptArray[0],
                       gptArray[1],
                       gptArray[2],
                       gptArray[3],
                       RSPF_RIGHT_HANDED);

      rspfDpt ulLatLon(((int)((grect.ul().x-theGeographicSpacing.x)/theGeographicSpacing.x))*theGeographicSpacing.x,
                        ((int)((grect.ul().y+theGeographicSpacing.y)/theGeographicSpacing.y))*theGeographicSpacing.y);
      
      rspfDpt lrLatLon(((int)((grect.lr().x+theGeographicSpacing.x)/theGeographicSpacing.x))*theGeographicSpacing.x,
                        ((int)((grect.lr().y-theGeographicSpacing.y)/theGeographicSpacing.y))*theGeographicSpacing.y);
      
      rspfDrect latLonSpacing(ulLatLon,
                               lrLatLon,
                               RSPF_RIGHT_HANDED);

      rspfDpt ipt; // top
      for(double lat = latLonSpacing.ul().y; lat >= latLonSpacing.lr().y; lat-=theGeographicSpacing.y)
      {
         for(double lon = latLonSpacing.ul().x; lon <= latLonSpacing.lr().x; lon+=theGeographicSpacing.x)
         {
            
            rspfGpt gpt(lat,
                         lon,
                         0.0,
                         gptArray[0].datum());
            
            mapProj->worldToLineSample(gpt, ipt);

            rspfIpt rounded(ipt);
            if(rect.pointWithin(rspfDpt(rounded)))
            {
               rspfIpt horizontalStart(rounded.x-10, rounded.y);
               rspfIpt horizontalEnd(rounded.x+10, rounded.y);

               rspfIpt verticalStart(rounded.x, rounded.y-10);
               rspfIpt verticalEnd(rounded.x, rounded.y+10);
               
               rspfAnnotationLineObject* line1 = new rspfAnnotationLineObject(horizontalStart, horizontalEnd);
               rspfAnnotationLineObject* line2 = new rspfAnnotationLineObject(verticalStart,verticalEnd);

               line1->setColor(theGeographicGridColor.getR(),
                               theGeographicGridColor.getG(),
                               theGeographicGridColor.getB());
               line2->setColor(theGeographicGridColor.getR(),
                               theGeographicGridColor.getG(),
                               theGeographicGridColor.getB());
               addFixedAnnotation(line1);
               addFixedAnnotation(line2);
            }  
         }
      }
   }   
}

void rspfMapCompositionSource::addMeterGridLines()
{
   const rspfMapProjection* mapProj = inputMapProjection();
   if(mapProj)
   { 
      rspfDpt dptArray[4];
      rspfDrect rect = getViewingRect();
      
      mapProj->lineSampleToEastingNorthing(rect.ul(), dptArray[0]);
      mapProj->lineSampleToEastingNorthing(rect.ur(), dptArray[1]);
      mapProj->lineSampleToEastingNorthing(rect.lr(), dptArray[2]);
      mapProj->lineSampleToEastingNorthing(rect.ll(), dptArray[3]);
      
      rspfDrect drect(dptArray[0], dptArray[1], dptArray[2], dptArray[3], RSPF_RIGHT_HANDED);
      
      rspfDpt ulMeter(((int)((drect.ul().x-theMeterSpacing.x)/theMeterSpacing.x))*theMeterSpacing.x,
                       ((int)((drect.ul().y+theMeterSpacing.y)/theMeterSpacing.y))*theMeterSpacing.y);
      
      rspfDpt lrMeter(((int)((drect.lr().x+theMeterSpacing.x)/theMeterSpacing.x))*theMeterSpacing.x,
                        ((int)((drect.lr().y-theMeterSpacing.y)/theMeterSpacing.y))*theMeterSpacing.y);
      
      rspfDrect meterSpacing(ulMeter,
                              lrMeter,
                              RSPF_RIGHT_HANDED);

      for(double horizontal = meterSpacing.ul().x;
          horizontal <= meterSpacing.lr().x;
          horizontal+=theMeterSpacing.x)
      {
         rspfDpt tipt; // top
         rspfDpt bipt; // bottom

         rspfDpt tdpt(horizontal, meterSpacing.ul().y);

         rspfDpt bdpt(horizontal, meterSpacing.lr().y);
         
         mapProj->eastingNorthingToLineSample(tdpt, tipt);
         mapProj->eastingNorthingToLineSample(bdpt, bipt);
         
         if(rect.clip(tipt, bipt))
         {
            rspfAnnotationLineObject* line = new rspfAnnotationLineObject(tipt, bipt);
            line->setColor(theMeterGridColor.getR(),
                           theMeterGridColor.getG(),
                           theMeterGridColor.getB());
            addFixedAnnotation(line);
         }
      }
      // do the left side labels
      for(double vertical = meterSpacing.ul().y;
          vertical >= meterSpacing.lr().y;
          vertical-=theMeterSpacing.y)
      {
         rspfDpt lipt;
         rspfDpt ript;
         rspfDpt ldpt(meterSpacing.ul().x, vertical);
         rspfDpt rdpt(meterSpacing.ur().x, vertical);

         mapProj->eastingNorthingToLineSample(ldpt,  lipt);
         mapProj->eastingNorthingToLineSample(rdpt,  ript);
         
         if(rect.clip(lipt, ript))
         {
            rspfAnnotationLineObject* line = new rspfAnnotationLineObject(lipt, ript);
         
            line->setColor(theMeterGridColor.getR(),
                           theMeterGridColor.getG(),
                           theMeterGridColor.getB());
            
            addFixedAnnotation(line);
         }
      }
   }
}

void rspfMapCompositionSource::addMeterGridLabels()
{
   const rspfMapProjection* mapProj = inputMapProjection();
   if(mapProj)
   {
      rspfDpt dptArray[4];
      rspfDrect rect = getViewingRect();
      
      mapProj->lineSampleToEastingNorthing(rect.ul(), dptArray[0]);
      mapProj->lineSampleToEastingNorthing(rect.ur(), dptArray[1]);
      mapProj->lineSampleToEastingNorthing(rect.lr(), dptArray[2]);
      mapProj->lineSampleToEastingNorthing(rect.ll(), dptArray[3]);
      
      rspfDrect drect(dptArray[0],
                       dptArray[1],
                       dptArray[2],
                       dptArray[3],
                       RSPF_RIGHT_HANDED);
      
      rspfDpt ulMeter(((int)((drect.ul().x-theMeterSpacing.x)/theMeterSpacing.x))*theMeterSpacing.x,
                       ((int)((drect.ul().y+theMeterSpacing.y)/theMeterSpacing.y))*theMeterSpacing.y);
      
      rspfDpt lrMeter(((int)((drect.lr().x+theMeterSpacing.x)/theMeterSpacing.x))*theMeterSpacing.x,
                        ((int)((drect.lr().y-theMeterSpacing.y)/theMeterSpacing.y))*theMeterSpacing.y);
      
      rspfDrect meterSpacing(ulMeter,
                              lrMeter,
                              RSPF_RIGHT_HANDED);
      
      rspfDrect labelRect;

      // Since we support overlapping grid lables.  We will shift
      // the meter label if there exists a gegraphic label.  To
      // do this we need to compute an extra shift length based
      // on the font type on each of the sides.  We could actually make this
      // shift adjustable later????
      //
      long extraTopDelta     = 0;
      long extraBottomDelta  = 0;
      long extraLeftDelta    = 0;
      long extraRightDelta   = 0;
         
      if(theTopGeographicLabelFlag&&theGeographicTopLabelFont.valid())
      {  
         rspfDms dms(180, false);
         rspfString dmsString = dms.toString(theTopGeographicFormat.c_str());
         
         theGeographicTopLabelFont->setString(dmsString);
         rspfIrect box;
         
         theGeographicTopLabelFont->getBoundingBox(box);
         extraTopDelta = box.height();
      }
      if(theBottomGeographicLabelFlag&&theGeographicBottomLabelFont.valid())
      {  
         rspfDms dms(180, false);
         rspfString dmsString = dms.toString(theBottomGeographicFormat.c_str());
         theGeographicBottomLabelFont->setString(dmsString);
         rspfIrect box;
         
         theGeographicBottomLabelFont->getBoundingBox(box);
         extraBottomDelta = box.height();
      }
      if(theLeftGeographicLabelFlag&&theGeographicLeftLabelFont.valid())
      {  
         rspfDms dms(90.0, true);
         rspfString dmsString = dms.toString(theLeftGeographicFormat.c_str());
         theGeographicLeftLabelFont->setString(dmsString);
         rspfIrect box;
         
         theGeographicLeftLabelFont->getBoundingBox(box);
         extraLeftDelta = box.width();
      }
      if(theRightGeographicLabelFlag&&theGeographicRightLabelFont.valid())
      {  
         rspfDms dms(90.0, true);
         rspfString dmsString = dms.toString(theRightGeographicFormat.c_str());
         
         theGeographicRightLabelFont->setString(dmsString);
         rspfIrect box;
         
         theGeographicRightLabelFont->getBoundingBox(box);
         extraRightDelta = box.width();
      }

      if(theTopMeterLabelFlag || theBottomMeterLabelFlag)
      {
         for(double horizontal = meterSpacing.ul().x;
             horizontal <= meterSpacing.lr().x;
             horizontal+=theMeterSpacing.x)
         {
            rspfDpt tipt; // top
            rspfDpt bipt; // bottom
            
            rspfDpt tdpt(horizontal,
                          meterSpacing.ul().y);
            
            rspfDpt bdpt(horizontal,
                          meterSpacing.lr().y);
            
            mapProj->eastingNorthingToLineSample(tdpt, tipt);
            mapProj->eastingNorthingToLineSample(bdpt, bipt);
            
            rect.clip(tipt, bipt);
            
            tipt = rspfIpt(tipt);
            bipt = rspfIpt(bipt);
            
            if(theTopMeterLabelFlag&&rect.clip(tipt, bipt))
            {
               rspfAnnotationFontObject* topLabel = new rspfAnnotationFontObject(rspfIpt(0,0),
                                                                                   rspfString::toString(horizontal).c_str());
               
               
               topLabel->setFont(theMeterTopLabelFont.get());
               
               topLabel->setGeometryInformation(theMeterTopLabelFontInfo);
               topLabel->computeBoundingRect();
               rspfDrect boundsTop;
               topLabel->getBoundingRect(boundsTop);
               
               rspfIpt centerTop(rspf::round<int>(tipt.x),
                                  rspf::round<int>(rect.ul().y-(boundsTop.height()/2)));
               
               topLabel->setColor(theTopMeterLabelColor.getR(),
                                  theTopMeterLabelColor.getG(),
                                  theTopMeterLabelColor.getB());
               
               
               topLabel->setCenterPosition(centerTop);
               topLabel->computeBoundingRect();
               
               topLabel->getBoundingRect(labelRect);
               
               if(rect.intersects(labelRect))
               {
                  rspfIrect intersection = rect.clipToRect(labelRect);
                  centerTop.y -= rspf::round<int>(rect.ul().y - intersection.lr().y);
               }
               
               centerTop.y -= (24+extraTopDelta); // this will need to be the tick height later;
               topLabel->setCenterPosition(centerTop);
               topLabel->computeBoundingRect();
               addFixedAnnotation(topLabel);

               if(theTopMeterTickFlag)
               {
                  rspfAnnotationLineObject* lineLabel = new rspfAnnotationLineObject(rspfIpt(rspf::round<int>(tipt.x),
                                                                                                rspf::round<int>(tipt.y-24)),
                                                                                       rspfIpt(rspf::round<int>(tipt.x),
                                                                                                rspf::round<int>(tipt.y)));
                  
                  lineLabel->setColor(theTopMeterLabelColor.getR(), theTopMeterLabelColor.getG(), theTopMeterLabelColor.getB());
                  addFixedAnnotation(lineLabel);
               }
            }
         // ------------------------- BOTTOM ----------------------------
            
            if(theBottomMeterLabelFlag&&rect.pointWithin(bipt))
            {
               rspfAnnotationFontObject* bottomLabel = new rspfAnnotationFontObject(rspfIpt(0,0),
                                                                                      rspfString::toString(horizontal).c_str());
               
               
               bottomLabel->setFont(theMeterBottomLabelFont.get());
               
               bottomLabel->setGeometryInformation(theMeterBottomLabelFontInfo);
               bottomLabel->computeBoundingRect();
               rspfDrect boundsBottom;
               bottomLabel->getBoundingRect(boundsBottom);
               
               rspfIpt centerBottom(rspf::round<int>(bipt.x),
                                     rspf::round<int>(rect.lr().y+(boundsBottom.height()/2)));
               
               bottomLabel->setColor(theBottomMeterLabelColor.getR(),
                                     theBottomMeterLabelColor.getG(),
                                     theBottomMeterLabelColor.getB());
               
               
               bottomLabel->setCenterPosition(centerBottom);
               bottomLabel->computeBoundingRect();
               
               bottomLabel->getBoundingRect(labelRect);
               
               if(rect.intersects(labelRect))
               {
                  rspfIrect intersection = rect.clipToRect(labelRect);
                  centerBottom.y += rspf::round<int>(rect.lr().y - intersection.ul().y);
               }
               
               centerBottom.y += (24+extraBottomDelta); // this will need to be the tick height later;
               bottomLabel->setCenterPosition(centerBottom);
               bottomLabel->computeBoundingRect();
               addFixedAnnotation(bottomLabel);
               if(theBottomMeterTickFlag)
               {
                  rspfAnnotationLineObject* lineLabel = new rspfAnnotationLineObject(rspfIpt(rspf::round<int>(bipt.x),
                                                                                                rspf::round<int>(bipt.y+24)),
                                                                                       rspfIpt(rspf::round<int>(bipt.x),
                                                                                                rspf::round<int>(bipt.y)));
                  
                  lineLabel->setColor(theBottomMeterLabelColor.getR(), theBottomMeterLabelColor.getG(), theBottomMeterLabelColor.getB());
                  addFixedAnnotation(lineLabel);
               }
            }            
         }
      }
      if(theLeftMeterLabelFlag || theRightMeterLabelFlag)
      {
         for(double vertical = meterSpacing.ul().y;
             vertical >= meterSpacing.lr().y;
             vertical-=theMeterSpacing.y)
         {
            rspfDpt lipt;
            rspfDpt ript;
            rspfDpt ldpt(meterSpacing.ul().x,
                          vertical);
            rspfDpt rdpt(meterSpacing.ur().x,
                          vertical);
            
            mapProj->eastingNorthingToLineSample(ldpt, lipt);
            mapProj->eastingNorthingToLineSample(rdpt, ript);
            
            if(rect.clip(lipt, ript))
            {
               //------------------------------------------------ Left label ----------------------------------------------
               rspfAnnotationFontObject* leftLabel = new rspfAnnotationFontObject(rspfIpt(0,0),
                                                                                    rspfString::toString(vertical).c_str());
               
               
               leftLabel->setFont(theMeterLeftLabelFont.get());
               
               leftLabel->setGeometryInformation(theMeterLeftLabelFontInfo);
               leftLabel->computeBoundingRect();
               rspfDrect boundsLeft;
               leftLabel->getBoundingRect(boundsLeft);
               
               rspfIpt centerLeft(rspf::round<int>(rect.ul().x-(boundsLeft.width()/2)),
                                   rspf::round<int>(lipt.y));
                                  
               
               leftLabel->setColor(theLeftMeterLabelColor.getR(),
                                   theLeftMeterLabelColor.getG(),
                                   theLeftMeterLabelColor.getB());
               
               
               leftLabel->setCenterPosition(centerLeft);
               leftLabel->computeBoundingRect();
               
               leftLabel->getBoundingRect(labelRect);
               
               if(rect.intersects(labelRect))
               {
                  rspfIrect intersection = rect.clipToRect(labelRect);
                  centerLeft.x -= rspf::round<int>(intersection.lr().x - rect.ul().x);
               }
               
               centerLeft.x -= (24+extraLeftDelta); // this will need to be the tick height later;
               leftLabel->setCenterPosition(centerLeft);
               leftLabel->computeBoundingRect();
               addFixedAnnotation(leftLabel);
               if(theLeftMeterTickFlag)
               {
                  rspfAnnotationLineObject* lineLabel = new rspfAnnotationLineObject(rspfIpt(rspf::round<int>(lipt.x-24),
                                                                                                rspf::round<int>(lipt.y)),
                                                                                       rspfIpt(rspf::round<int>(lipt.x),
                                                                                                rspf::round<int>(lipt.y)));
                  
                  lineLabel->setColor(theLeftMeterLabelColor.getR(), theLeftMeterLabelColor.getG(), theLeftMeterLabelColor.getB());
                  addFixedAnnotation(lineLabel);
               }

               
               //--------------------------------RIGHT-----------------------------------------------------------
               rspfAnnotationFontObject* rightLabel = new rspfAnnotationFontObject(rspfIpt(0,0),
                                                                                     rspfString::toString(vertical).c_str());
               
               
               rightLabel->setFont(theMeterRightLabelFont.get());
               
               rightLabel->setGeometryInformation(theMeterRightLabelFontInfo);
               rightLabel->computeBoundingRect();
               rspfDrect boundsRight;
               rightLabel->getBoundingRect(boundsRight);
               
               rspfIpt centerRight(rspf::round<int>(rect.ur().x+(boundsRight.width()/2)),
                                    rspf::round<int>(ript.y));
                                  
               
               rightLabel->setColor(theRightMeterLabelColor.getR(),
                                    theRightMeterLabelColor.getG(),
                                    theRightMeterLabelColor.getB());
               
               
               rightLabel->setCenterPosition(centerRight);
               rightLabel->computeBoundingRect();
               
               rightLabel->getBoundingRect(labelRect);
               
               if(rect.intersects(labelRect))
               {
                  rspfIrect intersection = rect.clipToRect(labelRect);
                  centerRight.x += rspf::round<int>(intersection.ul().x - rect.ur().x);
               }
               
               centerRight.x += (24+extraRightDelta); // this will need to be the tick height later;
               rightLabel->setCenterPosition(centerRight);
               rightLabel->computeBoundingRect();
               addFixedAnnotation(rightLabel);
               if(theRightMeterTickFlag)
               {
                  rspfAnnotationLineObject* lineLabel = new rspfAnnotationLineObject(rspfIpt(rspf::round<int>(ript.x+24),
                                                                                                rspf::round<int>(ript.y)),
                                                                                       rspfIpt(rspf::round<int>(ript.x),
                                                                                                rspf::round<int>(ript.y)));
                  
                  lineLabel->setColor(theRightMeterLabelColor.getR(), theRightMeterLabelColor.getG(), theRightMeterLabelColor.getB());
                  addFixedAnnotation(lineLabel);
               }
            }
         }
      }
   }
}


void rspfMapCompositionSource::addMeterGridReseaux()
{
   const rspfMapProjection* mapProj = inputMapProjection();
   if(mapProj)
   { 
      rspfDpt dpt[4];
      rspfDrect rect = getViewingRect();
      
      mapProj->lineSampleToEastingNorthing(rect.ul(), dpt[0]);
      mapProj->lineSampleToEastingNorthing(rect.ur(), dpt[1]);
      mapProj->lineSampleToEastingNorthing(rect.lr(), dpt[2]);
      mapProj->lineSampleToEastingNorthing(rect.ll(), dpt[3]);
      
      rspfDrect drect(dpt[0],
                       dpt[1],
                       dpt[2],
                       dpt[3],
                       RSPF_RIGHT_HANDED);

      rspfDpt ulMeter(((int)((drect.ul().x-theMeterSpacing.x)/theMeterSpacing.x))*theMeterSpacing.x,
                       ((int)((drect.ul().y+theMeterSpacing.y)/theMeterSpacing.y))*theMeterSpacing.y);
      
      rspfDpt lrMeter(((int)((drect.lr().x+theMeterSpacing.x)/theMeterSpacing.x))*theMeterSpacing.x,
                        ((int)((drect.lr().y-theMeterSpacing.y)/theMeterSpacing.y))*theMeterSpacing.y);

      rspfDrect meterSpacing(ulMeter,
                              lrMeter,
                              RSPF_RIGHT_HANDED);

      rspfDpt ipt; // top

      for(double vertical = meterSpacing.ul().y; vertical >= meterSpacing.lr().y; vertical-=theMeterSpacing.y)
      {
         for(double horizontal = meterSpacing.ul().x;  horizontal <= meterSpacing.lr().x; horizontal+=theMeterSpacing.x)
         {            
            rspfDpt dpt(horizontal,
                         vertical);
            
            mapProj->eastingNorthingToLineSample(dpt, ipt);

            rspfIpt rounded(ipt);
            if(rect.pointWithin(rspfDpt(rounded)))
            {
               rspfIpt horizontalStart(rounded.x-10, rounded.y);
               rspfIpt horizontalEnd(rounded.x+10, rounded.y);

               rspfIpt verticalStart(rounded.x, rounded.y-10);
               rspfIpt verticalEnd(rounded.x, rounded.y+10);
               
               rspfAnnotationLineObject* line1 = new rspfAnnotationLineObject(horizontalStart, horizontalEnd);
               rspfAnnotationLineObject* line2 = new rspfAnnotationLineObject(verticalStart,verticalEnd);

               line1->setColor(theMeterGridColor.getR(),
                               theMeterGridColor.getG(),
                               theMeterGridColor.getB());

               line2->setColor(theMeterGridColor.getR(),
                               theMeterGridColor.getG(),
                               theMeterGridColor.getB());
               addFixedAnnotation(line1);
               addFixedAnnotation(line2);
            }  
         }
      }
   }   
}

void rspfMapCompositionSource::addTitle()
{
   rspfAnnotationFontObject* title = new rspfAnnotationFontObject(rspfIpt(0,0),
                                                                    theTitleString);
   title->setGeometryInformation(theTitleFontInfo);
   title->setFont(theTitleFont.get());
   title->setColor(theTitleColor.getR(),
                   theTitleColor.getG(),
                   theTitleColor.getB());
   title->setCenterPosition(theTopBorder.midPoint());
   title->computeBoundingRect();

   addFixedAnnotation(title);
}

rspfIrect rspfMapCompositionSource::getViewingRect()const
{
   rspfIrect result;
   result.makeNan();

   if(theInputConnection)
   {
      result = theInputConnection->getBoundingRect();

      if(!result.hasNans())
      {
         rspfIpt mid = result.midPoint();
         rspf_int32 w = result.width();
         rspf_int32 h = result.width();

         if(theViewWidthHeight.x >-1)
         {
            w = theViewWidthHeight.x;
         }
         if(theViewWidthHeight.y >-1)
         {
            h = theViewWidthHeight.y;
         }
         rspfIpt ul(mid.x - w/2,
                     mid.y - h/2);
         
         result = rspfIrect(ul.x,
                             ul.y,
                             ul.x + w-1,
                             ul.y + h -1);
      }
   }

   return result;
}

void rspfMapCompositionSource::drawAnnotations(
   rspfRefPtr<rspfImageData> tile)
{
   if (!theImage)
   {
      return;
   }
      
   theImage->setCurrentImageData(theTile);
   
   if(theImage->getImageData().valid())
   {
      rspfAnnotationSource::AnnotationObjectListType::iterator object = theFixedAnnotationList.begin();
      while(object != theFixedAnnotationList.end())
      {
         if((*object).valid())
         {
            (*object)->draw(*theImage);
         }
         ++object;
      }      
   }
   
   rspfAnnotationSource::drawAnnotations(tile);
}

void rspfMapCompositionSource::deleteFixedAnnotations()
{
   theFixedAnnotationList.clear();
}


void rspfMapCompositionSource::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property.valid()) return;
   
   rspfRefPtr<rspfProperty>        tempProp     = 0;
   rspfFontProperty*    fontProp     = 0;
   rspfColorProperty*   colorProp    = 0;
   rspfBooleanProperty* booleanProp  = 0;
   
   if(property->getName() == "Geographic label properties")
   {
      rspfContainerProperty* containerProp = PTR_CAST(rspfContainerProperty,
                                                       property.get());
      if(containerProp)
      {
         // first set the font format
         //
         tempProp = containerProp->getProperty("Top font");
         fontProp = PTR_CAST(rspfFontProperty, tempProp.get());
         if(fontProp)
         {
            setGeographicTopLabelFont(fontProp->getFontInformation());
         }
         tempProp = containerProp->getProperty("Bottom font");
         fontProp = PTR_CAST(rspfFontProperty, tempProp.get());
         if(fontProp)
         {
            setGeographicBottomLabelFont(fontProp->getFontInformation());
         }
         tempProp = containerProp->getProperty("Left font");
         fontProp = PTR_CAST(rspfFontProperty, tempProp.get());
         if(fontProp)
         {
            setGeographicLeftLabelFont(fontProp->getFontInformation());
         }
         tempProp = containerProp->getProperty("Right font");
         fontProp = PTR_CAST(rspfFontProperty, tempProp.get());
         if(fontProp)
         {
            setGeographicRightLabelFont(fontProp->getFontInformation());
         }

         // set color property
         tempProp = containerProp->getProperty("Top color");
         colorProp = PTR_CAST(rspfColorProperty, tempProp.get());
         if(colorProp)
         {
            setTopGeographicLabelColor(colorProp->getColor());
         }
         tempProp = containerProp->getProperty("Bottom color");
         colorProp = PTR_CAST(rspfColorProperty, tempProp.get());
         if(colorProp)
         {
            setBottomGeographicLabelColor(colorProp->getColor());
         }
         tempProp = containerProp->getProperty("Left color");
         colorProp = PTR_CAST(rspfColorProperty, tempProp.get());
         if(colorProp)
         {
            setLeftGeographicLabelColor(colorProp->getColor());
         }
         tempProp = containerProp->getProperty("Right color");
         colorProp = PTR_CAST(rspfColorProperty, tempProp.get());
         if(colorProp)
         {
            setRightGeographicLabelColor(colorProp->getColor());
         }

         // set enable flags
         tempProp = containerProp->getProperty("Top enabled");
         booleanProp = PTR_CAST(rspfBooleanProperty,
                                tempProp.get());
         if(booleanProp)
         {
            setTopGeographicLabelFlag(booleanProp->getBoolean());
         }

         tempProp = containerProp->getProperty("Bottom enabled");
         booleanProp = PTR_CAST(rspfBooleanProperty,
                                tempProp.get());
         if(booleanProp)
         {
            setBottomGeographicLabelFlag(booleanProp->getBoolean());
         }

         tempProp = containerProp->getProperty("Left enabled");
         booleanProp = PTR_CAST(rspfBooleanProperty,
                                tempProp.get());
         if(booleanProp)
         {
            setLeftGeographicLabelFlag(booleanProp->getBoolean());
         }

         tempProp = containerProp->getProperty("Right enabled");
         booleanProp = PTR_CAST(rspfBooleanProperty,
                                tempProp.get());
         if(booleanProp)
         {
            setRightGeographicLabelFlag(booleanProp->getBoolean());
         }
         
         tempProp = containerProp->getProperty("Top tick enabled");
         booleanProp = PTR_CAST(rspfBooleanProperty,
                                tempProp.get());
         if(booleanProp)
         {
            setTopGeographicTickMarkFlag(booleanProp->getBoolean());
         }
         tempProp = containerProp->getProperty("Bottom tick enabled");
         booleanProp = PTR_CAST(rspfBooleanProperty,
                                tempProp.get());
         if(booleanProp)
         {
            setBottomGeographicTickMarkFlag(booleanProp->getBoolean());
         }
         tempProp = containerProp->getProperty("Left tick enabled");
         booleanProp = PTR_CAST(rspfBooleanProperty,
                                tempProp.get());
         if(booleanProp)
         {
            setLeftGeographicTickMarkFlag(booleanProp->getBoolean());
         }
         tempProp = containerProp->getProperty("Right tick enabled");
         booleanProp = PTR_CAST(rspfBooleanProperty,
                                tempProp.get());
         if(booleanProp)
         {
            setRightGeographicTickMarkFlag(booleanProp->getBoolean());
         }
      }
   }
   else if(property->getName() == "Geographic grid properties")
   {
      rspfContainerProperty* containerProp = PTR_CAST(rspfContainerProperty,
                                                       property.get());
      if(containerProp)
      {
         tempProp = containerProp->getProperty("Grid type");
         if(tempProp.valid())
         {
            rspfString value = tempProp->valueToString();

            if(value == "none")
            {
               setGeographicGridType(RSPF_GRID_NONE);
            }
            else if(value == "line")
            {
               setGeographicGridType(RSPF_GRID_LINE);
            }
            else if(value == "reseaux")
            {
               setGeographicGridType(RSPF_GRID_RESEAUX);
            }
         }
         tempProp = containerProp->getProperty("Horizontal spacing");
         if(tempProp.valid())
         {
            setGeographicSpacingLon(tempProp->valueToString().toDouble());
         }

         tempProp = containerProp->getProperty("Vertical spacing");
         if(tempProp.valid())
         {
            setGeographicSpacingLat(tempProp->valueToString().toDouble());
         }

         tempProp = containerProp->getProperty("Color");
         colorProp = PTR_CAST(rspfColorProperty, tempProp.get());
         if(tempProp.valid())
         {
            setGeographicGridColor(colorProp->getColor());
         }
      }
   }
   else if(property->getName() == "Meter label properties")
   {
      rspfContainerProperty* containerProp = PTR_CAST(rspfContainerProperty,
                                                       property.get());
      if(containerProp)
      {
         // first set the font format
         //
         tempProp = containerProp->getProperty("Top font");
         fontProp = PTR_CAST(rspfFontProperty, tempProp.get());
         if(fontProp)
         {
            setMeterTopLabelFont(fontProp->getFontInformation());
         }
         tempProp = containerProp->getProperty("Bottom font");
         fontProp = PTR_CAST(rspfFontProperty, tempProp.get());
         if(fontProp)
         {
            setMeterBottomLabelFont(fontProp->getFontInformation());
         }
         tempProp = containerProp->getProperty("Left font");
         fontProp = PTR_CAST(rspfFontProperty, tempProp.get());
         if(fontProp)
         {
            setMeterLeftLabelFont(fontProp->getFontInformation());
         }
         tempProp = containerProp->getProperty("Right font");
         fontProp = PTR_CAST(rspfFontProperty, tempProp.get());
         if(fontProp)
         {
            setMeterRightLabelFont(fontProp->getFontInformation());
         }

         // set color property
         tempProp = containerProp->getProperty("Top color");
         colorProp = PTR_CAST(rspfColorProperty, tempProp.get());
         if(colorProp)
         {
            setTopMeterLabelColor(colorProp->getColor());
         }
         tempProp = containerProp->getProperty("Bottom color");
         colorProp = PTR_CAST(rspfColorProperty, tempProp.get());
         if(colorProp)
         {
            setBottomMeterLabelColor(colorProp->getColor());
         }
         tempProp = containerProp->getProperty("Left color");
         colorProp = PTR_CAST(rspfColorProperty, tempProp.get());
         if(colorProp)
         {
            setLeftMeterLabelColor(colorProp->getColor());
         }
         tempProp = containerProp->getProperty("Right color");
         colorProp = PTR_CAST(rspfColorProperty, tempProp.get());
         if(colorProp)
         {
            setRightMeterLabelColor(colorProp->getColor());
         }

         // set enable flags
         tempProp = containerProp->getProperty("Top enabled");
         booleanProp = PTR_CAST(rspfBooleanProperty,
                                tempProp.get());
         if(booleanProp)
         {
            setTopMeterLabelFlag(booleanProp->getBoolean());
         }

         tempProp = containerProp->getProperty("Bottom enabled");
         booleanProp = PTR_CAST(rspfBooleanProperty,
                                tempProp.get());
         if(booleanProp)
         {
            setBottomMeterLabelFlag(booleanProp->getBoolean());
         }

         tempProp = containerProp->getProperty("Left enabled");
         booleanProp = PTR_CAST(rspfBooleanProperty,
                                tempProp.get());
         if(booleanProp)
         {
            setLeftMeterLabelFlag(booleanProp->getBoolean());
         }

         tempProp = containerProp->getProperty("Right enabled");
         booleanProp = PTR_CAST(rspfBooleanProperty,
                                tempProp.get());
         if(booleanProp)
         {
            setRightMeterLabelFlag(booleanProp->getBoolean());
         }
         
         tempProp = containerProp->getProperty("Top tick enabled");
         booleanProp = PTR_CAST(rspfBooleanProperty,
                                tempProp.get());
         if(booleanProp)
         {
            setTopMeterTickMarkFlag(booleanProp->getBoolean());
         }
         tempProp = containerProp->getProperty("Bottom tick enabled");
         booleanProp = PTR_CAST(rspfBooleanProperty,
                                tempProp.get());
         if(booleanProp)
         {
            setBottomMeterTickMarkFlag(booleanProp->getBoolean());
         }
         tempProp = containerProp->getProperty("Left tick enabled");
         booleanProp = PTR_CAST(rspfBooleanProperty,
                                tempProp.get());
         if(booleanProp)
         {
            setLeftMeterTickMarkFlag(booleanProp->getBoolean());
         }
         tempProp = containerProp->getProperty("Right tick enabled");
         booleanProp = PTR_CAST(rspfBooleanProperty,
                                tempProp.get());
         if(booleanProp)
         {
            setRightMeterTickMarkFlag(booleanProp->getBoolean());
         }
      }
   }
   else if(property->getName() == "Meter grid properties")
   {
      rspfContainerProperty* containerProp = PTR_CAST(rspfContainerProperty,
                                                       property.get());
      if(containerProp)
      {
         tempProp = containerProp->getProperty("Grid type");
         if(tempProp.valid())
         {
            rspfString value = tempProp->valueToString();

            if(value == "none")
            {
               setMeterGridType(RSPF_GRID_NONE);
            }
            else if(value == "line")
            {
               setMeterGridType(RSPF_GRID_LINE);
            }
            else if(value == "reseaux")
            {
               setMeterGridType(RSPF_GRID_RESEAUX);
            }
         }
         tempProp = containerProp->getProperty("Horizontal spacing");
         if(tempProp.valid())
         {
            setMeterSpacingX(tempProp->valueToString().toDouble());
         }

         tempProp = containerProp->getProperty("Vertical spacing");
         if(tempProp.valid())
         {
            setMeterSpacingY(tempProp->valueToString().toDouble());
         }

         tempProp = containerProp->getProperty("Color");
         colorProp = PTR_CAST(rspfColorProperty, tempProp.get());
         if(tempProp.valid())
         {
            setMeterGridColor(colorProp->getColor());
         }
      }
   }
   else if(property->getName() == "Title properties")
   {
      rspfContainerProperty* container = PTR_CAST(rspfContainerProperty,
                                                   property.get());
      if(container)
      {
         rspfProperty* titleProp = container->getProperty("Title").get();
         if(titleProp)
         {
            setTitle(titleProp->valueToString());
         }
         rspfFontProperty* fontProp = PTR_CAST(rspfFontProperty,
                                                container->getProperty("Font").get());
         rspfColorProperty* colorProp = PTR_CAST(rspfColorProperty,
                                                  container->getProperty("Color").get());

         if(fontProp)
         {
            setTitleFont(fontProp->getFontInformation());
         }
         if(colorProp)
         {
            setTitleColor(colorProp->getColor());
         }
      }
   }
   else if(property->getName() == "Viewport properties")
   {
      rspfContainerProperty* container = PTR_CAST(rspfContainerProperty,
                                                   property.get());
      if(container)
      {
         int w = -1;
         int h = -1;
         tempProp = container->getProperty("Width");
         if(tempProp.valid())
         {
            w = tempProp->valueToString().toInt32();
         }
         tempProp = container->getProperty("Height");
         if(tempProp.valid())
         {
            h = tempProp->valueToString().toInt32();
         }
         setViewWidthHeight(rspfIpt(w, h));
      }
   }
   else if(property->getName() == "Border color")
   {
      colorProp = PTR_CAST(rspfColorProperty,
                           property.get());
      if(colorProp)
      {
         setBorderColor(colorProp->getColor());
      }
   }
   else if(property->getName() == "Border size")
   {
      rspfContainerProperty* container = PTR_CAST(rspfContainerProperty,
                                                   property.get());
      if(container)
      {
         tempProp = container->getProperty("Top");
         if(tempProp.valid())
         {
            setTopBorderLength(tempProp->valueToString().toInt32());
            
         }
         tempProp = container->getProperty("Bottom");
         if(tempProp.valid())
         {
            setBottomBorderLength(tempProp->valueToString().toInt32());
            
         }
         tempProp = container->getProperty("Left");
         if(tempProp.valid())
         {
            setLeftBorderLength(tempProp->valueToString().toInt32());
            
         }
         tempProp = container->getProperty("Right");
         if(tempProp.valid())
         {
            setRightBorderLength(tempProp->valueToString().toInt32());
         }
      }
   }
   else
   {
      rspfAnnotationSource::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfMapCompositionSource::getProperty(const rspfString& name)const
{
   if(name == "Viewport properties")
   {
      rspfContainerProperty* container = new rspfContainerProperty(name);

      container->addChild(new rspfNumericProperty("Width",
                                                   rspfString::toString(theViewWidthHeight.x),
                                                   -1,1.0/DBL_EPSILON));
      container->addChild(new rspfNumericProperty("Height",
                                                   rspfString::toString(theViewWidthHeight.y),
                                                   -1,1.0/DBL_EPSILON));
      container->setCacheRefreshBit();
      return container;
   }
   else if(name == "Geographic label properties")
   {

      rspfContainerProperty* container = new rspfContainerProperty(name);
      
      container->addChild(new rspfFontProperty("Top font",
                                                theGeographicTopLabelFontInfo));
      container->addChild(new rspfColorProperty("Top color",
                                                 theTopGeographicLabelColor));
      container->addChild(new rspfBooleanProperty("Top enabled",
                                                   theTopGeographicLabelFlag));
      container->addChild(new rspfBooleanProperty("Top tick enabled",
                                                   theTopGeographicTickFlag));
      
      container->addChild(new rspfFontProperty("Bottom font",
                                                theGeographicBottomLabelFontInfo));
      container->addChild(new rspfColorProperty("Bottom color",
                                                 theBottomGeographicLabelColor));
      container->addChild(new rspfBooleanProperty("Bottom enabled",
                                                   theBottomGeographicLabelFlag));
      container->addChild(new rspfBooleanProperty("Bottom tick enabled",
                                                   theBottomGeographicTickFlag));
      
      container->addChild(new rspfFontProperty("Left font",
                                                theGeographicLeftLabelFontInfo));
      container->addChild(new rspfColorProperty("Left color",
                                                 theLeftGeographicLabelColor));
      container->addChild(new rspfBooleanProperty("Left enabled",
                                                   theLeftGeographicLabelFlag));
      container->addChild(new rspfBooleanProperty("Left tick enabled",
                                                   theLeftGeographicTickFlag));
      
      container->addChild(new rspfFontProperty("Right font",
                                                theGeographicRightLabelFontInfo));
      container->addChild(new rspfColorProperty("Right color",
                                                 theRightGeographicLabelColor));
      container->addChild(new rspfBooleanProperty("Right enabled",
                                                   theRightGeographicLabelFlag));
      container->addChild(new rspfBooleanProperty("Right tick enabled",
                                                   theRightGeographicTickFlag));
      container->setCacheRefreshBit();
      return container;
   }
   else if(name == "Geographic grid properties")
   {
      rspfContainerProperty* container = new rspfContainerProperty(name);
      std::vector<rspfString> gridTypeConstraints;
      rspfString value = GRID_TYPE_ENUM_NAMES[(int)theGeographicGridType];

      gridTypeConstraints.push_back("none");
      gridTypeConstraints.push_back("line");
      gridTypeConstraints.push_back("reseaux");
      
      container->addChild(new rspfStringProperty("Grid type",
                                                  value,
                                                  false,
                                                  gridTypeConstraints));
      container->addChild(new rspfColorProperty("Color",
                                                 theGeographicGridColor));

      container->addChild(new rspfNumericProperty("Horizontal spacing",
                                                   rspfString::toString(theGeographicSpacing.x),
                                                   0.0, 180.0));

      container->addChild(new rspfNumericProperty("Vertical spacing",
                                                   rspfString::toString(theGeographicSpacing.y),
                                                   0.0, 180.0));
                                                   
      container->setCacheRefreshBit();
      return container;
   }
   else if(name == "Meter label properties")
   {
      rspfContainerProperty* container = new rspfContainerProperty(name);
      
      container->addChild(new rspfFontProperty("Top font",
                                                theMeterTopLabelFontInfo));
      container->addChild(new rspfColorProperty("Top color",
                                                 theTopMeterLabelColor));
      container->addChild(new rspfBooleanProperty("Top enabled",
                                                   theTopMeterLabelFlag));
      container->addChild(new rspfBooleanProperty("Top tick enabled",
                                                   theTopMeterTickFlag));
      
      container->addChild(new rspfFontProperty("Bottom font",
                                                theMeterBottomLabelFontInfo));
      container->addChild(new rspfColorProperty("Bottom color",
                                                 theBottomMeterLabelColor));
      container->addChild(new rspfBooleanProperty("Bottom enabled",
                                                   theBottomMeterLabelFlag));
      container->addChild(new rspfBooleanProperty("Bottom tick enabled",
                                                   theBottomMeterTickFlag));
      
      container->addChild(new rspfFontProperty("Left font",
                                                theMeterLeftLabelFontInfo));
      container->addChild(new rspfColorProperty("Left color",
                                                 theLeftMeterLabelColor));
      container->addChild(new rspfBooleanProperty("Left enabled",
                                                   theLeftMeterLabelFlag));
      container->addChild(new rspfBooleanProperty("Left tick enabled",
                                                   theLeftMeterTickFlag));
      
      container->addChild(new rspfFontProperty("Right font",
                                                theMeterRightLabelFontInfo));
      container->addChild(new rspfColorProperty("Right color",
                                                 theRightMeterLabelColor));
      container->addChild(new rspfBooleanProperty("Right enabled",
                                                   theRightMeterLabelFlag));
      container->addChild(new rspfBooleanProperty("Right tick enabled",
                                                   theRightMeterTickFlag));
      
      container->setCacheRefreshBit();
      return container;
   }
   else if(name == "Meter grid properties")
   {
      rspfContainerProperty* container = new rspfContainerProperty(name);
      std::vector<rspfString> gridTypeConstraints;
      rspfString value = GRID_TYPE_ENUM_NAMES[(int)theMeterGridType];

      gridTypeConstraints.push_back("none");
      gridTypeConstraints.push_back("line");
      gridTypeConstraints.push_back("reseaux");
      
      container->addChild(new rspfStringProperty("Grid type",
                                                  value,
                                                  false,
                                                  gridTypeConstraints));
      container->addChild(new rspfColorProperty("Color",
                                                 theMeterGridColor));
      
      container->addChild(new rspfNumericProperty("Horizontal spacing",
                                                   rspfString::toString(theMeterSpacing.x),
                                                   0.0, 1.0/DBL_EPSILON));

      container->addChild(new rspfNumericProperty("Vertical spacing",
                                                   rspfString::toString(theMeterSpacing.y),
                                                   0.0, 1.0/DBL_EPSILON));
      container->setCacheRefreshBit();
      return container;
   }
   else if(name == "Title properties")
   {
      rspfContainerProperty* container = new rspfContainerProperty(name);
      container->addChild(new rspfTextProperty("Title",
                                                theTitleString));
      container->addChild(new rspfFontProperty("Font",
                                                theTitleFontInfo));
      container->addChild(new rspfColorProperty("Color",
                                                 theTitleColor));
      
      container->setCacheRefreshBit();
      return container;
   }
   else if(name == "Border size")
   {
      rspfContainerProperty* container = new rspfContainerProperty(name);
      
      container->addChild(new rspfNumericProperty("Top",
                                                   rspfString::toString(theTopBorderLength),
                                                   0,10000));
      container->addChild(new rspfNumericProperty("Bottom",
                                                   rspfString::toString(theBottomBorderLength),
                                                   0,10000));
      container->addChild(new rspfNumericProperty("Left",
                                                   rspfString::toString(theLeftBorderLength),
                                                   0,10000));
      container->addChild(new rspfNumericProperty("Right",
                                                   rspfString::toString(theRightBorderLength),
                                                   0,10000));
      container->setCacheRefreshBit();
      return container;
   }
   else if(name == "Border color")
   {
      rspfColorProperty* colorProp = new rspfColorProperty(name,
                                                             theBorderColor);
      colorProp->setCacheRefreshBit();
      
      return colorProp;
   }
   else if(name == "Viewport properties")
   {
      
   }
   return rspfAnnotationSource::getProperty(name);
}

void rspfMapCompositionSource::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfAnnotationSource::getPropertyNames(propertyNames);

   
   propertyNames.push_back("Title properties");
   propertyNames.push_back("Border size");
   propertyNames.push_back("Border color");
   propertyNames.push_back("Viewport properties");
   propertyNames.push_back("Geographic label properties");
   propertyNames.push_back("Geographic grid properties");
   propertyNames.push_back("Meter label properties");
   propertyNames.push_back("Meter grid properties");
}

bool rspfMapCompositionSource::saveState(rspfKeywordlist& kwl,
                                          const char* prefix)const
{
   kwl.add(prefix,
           VIEW_WIDTH_KW,
           theViewWidthHeight.x,
           true);
   
   kwl.add(prefix,
           VIEW_HEIGHT_KW,
           theViewWidthHeight.x,
           true);
   
   kwl.add(prefix,
           METER_GRID_SPACING_X_KW,
           theMeterSpacing.x,
           true);

   kwl.add(prefix,
           METER_GRID_SPACING_Y_KW,
           theMeterSpacing.y,
           true);
   
   kwl.add(prefix,
           GEO_GRID_SPACING_LON_KW,
           theGeographicSpacing.lon,
           true);

   kwl.add(prefix,
           GEO_GRID_SPACING_LAT_KW,
           theGeographicSpacing.lat,
           true);

   rspfString meterGridType = "line";
   if(theMeterGridType == RSPF_GRID_NONE)
   {
      meterGridType = "none";
   }
   else if(theMeterGridType == RSPF_GRID_RESEAUX)
   {
      meterGridType = "reseaux";
   }
   
   kwl.add(prefix,
           METER_GRID_TYPE_KW,
           meterGridType.c_str(),
           true);

   rspfString geoGridType = "line";
   if(theGeographicGridType == RSPF_GRID_NONE)
   {
      geoGridType = "none";
   }
   else if(theGeographicGridType == RSPF_GRID_RESEAUX)
   {
      geoGridType = "reseaux";
   }

   kwl.add(prefix,
           GEO_GRID_TYPE_KW,
           geoGridType.c_str(),
           true);

   kwl.add(prefix,
           TOP_BORDER_LENGTH_KW,
           theTopBorderLength,
           true);
   
   kwl.add(prefix,
           BOTTOM_BORDER_LENGTH_KW,
           theBottomBorderLength,
           true);

   kwl.add(prefix,
           LEFT_BORDER_LENGTH_KW,
           theLeftBorderLength,
           true);

   kwl.add(prefix,
           RIGHT_BORDER_LENGTH_KW,
           theRightBorderLength,
           true);

   kwl.add(prefix,
           BORDER_COLOR_KW,
           (rspfString::toString(theBorderColor.getR()) +" "+
            rspfString::toString(theBorderColor.getG()) +" "+
            rspfString::toString(theBorderColor.getB())).c_str(),
           true);

   kwl.add(prefix,
           GEO_GRID_COLOR_KW,
           (rspfString::toString(theGeographicGridColor.getR()) +" "+
            rspfString::toString(theGeographicGridColor.getG()) +" "+
            rspfString::toString(theGeographicGridColor.getB())).c_str(),
           true);

   kwl.add(prefix,
           METER_GRID_COLOR_KW,
           (rspfString::toString(theMeterGridColor.getR()) +" "+
            rspfString::toString(theMeterGridColor.getG()) +" "+
            rspfString::toString(theMeterGridColor.getB())).c_str(),
           true);

   kwl.add(prefix,
           TITLE_STRING_KW,
           theTitleString.c_str(),
           true);
   
   kwl.add(prefix,
           TITLE_COLOR_KW,
           (rspfString::toString(theTitleColor.getR()) +" "+
            rspfString::toString(theTitleColor.getG()) +" "+
            rspfString::toString(theTitleColor.getB())).c_str(),
           true);

   kwl.add(prefix,
           TOP_GEO_LABEL_COLOR_KW,
           (rspfString::toString(theTopGeographicLabelColor.getR()) +" "+
            rspfString::toString(theTopGeographicLabelColor.getG()) +" "+
            rspfString::toString(theTopGeographicLabelColor.getB())).c_str(),
           true);

   kwl.add(prefix,
           BOTTOM_GEO_LABEL_COLOR_KW,
           (rspfString::toString(theBottomGeographicLabelColor.getR()) +" "+
            rspfString::toString(theBottomGeographicLabelColor.getG()) +" "+
            rspfString::toString(theBottomGeographicLabelColor.getB())).c_str(),
           true);
   kwl.add(prefix,
           LEFT_GEO_LABEL_COLOR_KW,
           (rspfString::toString(theLeftGeographicLabelColor.getR()) +" "+
            rspfString::toString(theLeftGeographicLabelColor.getG()) +" "+
            rspfString::toString(theLeftGeographicLabelColor.getB())).c_str(),
           true);
   kwl.add(prefix,
           RIGHT_GEO_LABEL_COLOR_KW,
           (rspfString::toString(theRightGeographicLabelColor.getR()) +" "+
            rspfString::toString(theRightGeographicLabelColor.getG()) +" "+
            rspfString::toString(theRightGeographicLabelColor.getB())).c_str(),
           true);

   kwl.add(prefix,
           TOP_GEO_LABEL_FORMAT_KW,
           theTopGeographicFormat,
           true);

   kwl.add(prefix,
           BOTTOM_GEO_LABEL_FORMAT_KW,
           theBottomGeographicFormat,
           true);
   kwl.add(prefix,
           LEFT_GEO_LABEL_FORMAT_KW,
           theLeftGeographicFormat,
           true);
   kwl.add(prefix,
           RIGHT_GEO_LABEL_FORMAT_KW,
           theRightGeographicFormat,
           true);

   kwl.add(prefix,
           TOP_GEO_LABEL_FLAG_KW,
           (int)theTopGeographicLabelFlag,
           true);

   kwl.add(prefix,
           BOTTOM_GEO_LABEL_FLAG_KW,
           (int)theBottomGeographicLabelFlag,
           true);

   kwl.add(prefix,
           LEFT_GEO_LABEL_FLAG_KW,
           (int)theLeftGeographicLabelFlag,
           true);

   kwl.add(prefix,
           RIGHT_GEO_LABEL_FLAG_KW,
           (int)theRightGeographicLabelFlag,
           true);

   kwl.add(prefix,
           TOP_METER_LABEL_COLOR_KW,
           (rspfString::toString(theTopMeterLabelColor.getR()) +" "+
            rspfString::toString(theTopMeterLabelColor.getG()) +" "+
            rspfString::toString(theTopMeterLabelColor.getB())).c_str(),
           true);

   kwl.add(prefix,
           BOTTOM_METER_LABEL_COLOR_KW,
           (rspfString::toString(theBottomMeterLabelColor.getR()) +" "+
            rspfString::toString(theBottomMeterLabelColor.getG()) +" "+
            rspfString::toString(theBottomMeterLabelColor.getB())).c_str(),
           true);
   kwl.add(prefix,
           LEFT_METER_LABEL_COLOR_KW,
           (rspfString::toString(theLeftMeterLabelColor.getR()) +" "+
            rspfString::toString(theLeftMeterLabelColor.getG()) +" "+
            rspfString::toString(theLeftMeterLabelColor.getB())).c_str(),
           true);
   kwl.add(prefix,
           RIGHT_METER_LABEL_COLOR_KW,
           (rspfString::toString(theRightMeterLabelColor.getR()) +" "+
            rspfString::toString(theRightMeterLabelColor.getG()) +" "+
            rspfString::toString(theRightMeterLabelColor.getB())).c_str(),
           true);


   kwl.add(prefix,
           TOP_METER_LABEL_FLAG_KW,
           (int)theTopMeterLabelFlag,
           true);

   kwl.add(prefix,
           BOTTOM_METER_LABEL_FLAG_KW,
           (int)theBottomMeterLabelFlag,
           true);

   kwl.add(prefix,
           LEFT_METER_LABEL_FLAG_KW,
           (int)theLeftMeterLabelFlag,
           true);

   kwl.add(prefix,
           RIGHT_METER_LABEL_FLAG_KW,
           (int)theRightMeterLabelFlag,
           true);
   
   theGeographicTopLabelFontInfo.saveState(kwl, (rspfString(prefix) + rspfString(TOP_GEO_LABEL_FONT_KW) + ".").c_str());
   theGeographicBottomLabelFontInfo.saveState(kwl, (rspfString(prefix) + rspfString(BOTTOM_GEO_LABEL_FONT_KW) + ".").c_str());
   theGeographicLeftLabelFontInfo.saveState(kwl, (rspfString(prefix) + rspfString(LEFT_GEO_LABEL_FONT_KW) + ".").c_str());
   theGeographicRightLabelFontInfo.saveState(kwl, (rspfString(prefix) + rspfString(RIGHT_GEO_LABEL_FONT_KW) + ".").c_str());

   theMeterTopLabelFontInfo.saveState(kwl, (rspfString(prefix) + rspfString(TOP_METER_LABEL_FONT_KW) + ".").c_str());
   theMeterBottomLabelFontInfo.saveState(kwl, (rspfString(prefix) + rspfString(BOTTOM_METER_LABEL_FONT_KW) + ".").c_str());
   theMeterLeftLabelFontInfo.saveState(kwl, (rspfString(prefix) + rspfString(LEFT_METER_LABEL_FONT_KW) + ".").c_str());
   theMeterRightLabelFontInfo.saveState(kwl, (rspfString(prefix) + rspfString(RIGHT_METER_LABEL_FONT_KW) + ".").c_str());
   
   theTitleFontInfo.saveState(kwl, (rspfString(prefix) + rspfString(TITLE_FONT_KW) + ".").c_str());
   
           
   return rspfImageSource::saveState(kwl, prefix);
   
}

bool rspfMapCompositionSource::loadState(const rspfKeywordlist& kwl,
                                          const char* prefix)
{
   const char* viewWidth            = kwl.find(prefix, VIEW_WIDTH_KW);
   const char* viewHeight           = kwl.find(prefix, VIEW_HEIGHT_KW);
   const char* meterGridSpacingX    = kwl.find(prefix, METER_GRID_SPACING_X_KW);
   const char* meterGridSpacingY    = kwl.find(prefix, METER_GRID_SPACING_Y_KW);
   const char* geoGridSpacingLon    = kwl.find(prefix, GEO_GRID_SPACING_LON_KW);
   const char* geoGridSpacingLat    = kwl.find(prefix, GEO_GRID_SPACING_LAT_KW);
   const char* meterGridType        = kwl.find(prefix, METER_GRID_TYPE_KW);
   const char* geoGridType          = kwl.find(prefix, GEO_GRID_TYPE_KW);
   const char* topBorderLength      = kwl.find(prefix, TOP_BORDER_LENGTH_KW);
   const char* bottomBorderLength   = kwl.find(prefix, BOTTOM_BORDER_LENGTH_KW);
   const char* leftBorderLength     = kwl.find(prefix, LEFT_BORDER_LENGTH_KW);
   const char* rightBorderLength    = kwl.find(prefix, RIGHT_BORDER_LENGTH_KW);
   
   const char* borderColor          = kwl.find(prefix, BORDER_COLOR_KW);
   const char* geoGridColor         = kwl.find(prefix, GEO_GRID_COLOR_KW);
   const char* meterGridColor       = kwl.find(prefix, METER_GRID_COLOR_KW);
   const char* titleString          = kwl.find(prefix, TITLE_STRING_KW);
   const char* titleColor           = kwl.find(prefix, TITLE_COLOR_KW);

   const char* topGeoLabelColor     = kwl.find(prefix, TOP_GEO_LABEL_COLOR_KW);
   const char* bottomGeoLabelColor  = kwl.find(prefix, BOTTOM_GEO_LABEL_COLOR_KW);
   const char* leftGeoLabelColor    = kwl.find(prefix, LEFT_GEO_LABEL_COLOR_KW);
   const char* rightGeoLabelColor   = kwl.find(prefix, RIGHT_GEO_LABEL_COLOR_KW);

   const char* topMeterLabelColor     = kwl.find(prefix, TOP_METER_LABEL_COLOR_KW);
   const char* bottomMeterLabelColor  = kwl.find(prefix, BOTTOM_METER_LABEL_COLOR_KW);
   const char* leftMeterLabelColor    = kwl.find(prefix, LEFT_METER_LABEL_COLOR_KW);
   const char* rightMeterLabelColor   = kwl.find(prefix, RIGHT_METER_LABEL_COLOR_KW);
   
   const char* topGeoLabelFormat    = kwl.find(prefix, TOP_GEO_LABEL_FORMAT_KW);
   const char* bottomGeoLabelFormat = kwl.find(prefix, BOTTOM_GEO_LABEL_FORMAT_KW);
   const char* leftGeoLabelFormat   = kwl.find(prefix, LEFT_GEO_LABEL_FORMAT_KW);
   const char* rightGeoLabelFormat  = kwl.find(prefix, RIGHT_GEO_LABEL_FORMAT_KW);

   const char* topGeoLabelFlag      = kwl.find(prefix, TOP_GEO_LABEL_FLAG_KW);
   const char* bottomGeoLabelFlag   = kwl.find(prefix, BOTTOM_GEO_LABEL_FLAG_KW);
   const char* leftGeoLabelFlag     = kwl.find(prefix, LEFT_GEO_LABEL_FLAG_KW);
   const char* rightGeoLabelFlag    = kwl.find(prefix, RIGHT_GEO_LABEL_FLAG_KW);

   const char* topMeterLabelFlag    = kwl.find(prefix, TOP_METER_LABEL_FLAG_KW);
   const char* bottomMeterLabelFlag = kwl.find(prefix, BOTTOM_METER_LABEL_FLAG_KW);
   const char* leftMeterLabelFlag   = kwl.find(prefix, LEFT_METER_LABEL_FLAG_KW);
   const char* rightMeterLabelFlag  = kwl.find(prefix, RIGHT_METER_LABEL_FLAG_KW);

   deleteFixedAnnotations();
   theViewWidthHeight.makeNan();
   
   if(viewWidth)
   {
      theViewWidthHeight.x = rspfString(viewWidth).toLong();
   }
   if(viewHeight)
   {
      theViewWidthHeight.y = rspfString(viewHeight).toLong();
   }
   
   if(meterGridSpacingX)
   {
      theMeterSpacing.x = rspfString(meterGridSpacingX).toDouble();
   }

   if(meterGridSpacingY)
   {
      theMeterSpacing.y = rspfString(meterGridSpacingY).toDouble();
   }
   if(geoGridSpacingLon)
   {
      theGeographicSpacing.lon = rspfString(geoGridSpacingLon).toDouble();
   }
   if(geoGridSpacingLat)
   {
      theGeographicSpacing.lat = rspfString(geoGridSpacingLat).toDouble();
   }
   if(meterGridType)
   {
      rspfString type = rspfString(meterGridType).trim().downcase();
      if(type == "reseaux")
      {
         theMeterGridType = RSPF_GRID_RESEAUX;
      }
      else if(type == "line")
      {
         theMeterGridType = RSPF_GRID_LINE;
      }
      else
      {
         theMeterGridType = RSPF_GRID_NONE;
      }
   }

   if(geoGridType)
   {
      rspfString type = rspfString(geoGridType).trim().downcase();
      if(type == "reseaux")
      {
         theGeographicGridType = RSPF_GRID_RESEAUX;
      }
      else if(type == "line")
      {
         theGeographicGridType = RSPF_GRID_LINE;
      }
      else
      {
         theGeographicGridType = RSPF_GRID_NONE;
      }
   }

   if(topBorderLength)
   {
      theTopBorderLength = rspfString(topBorderLength).toULong();
   }

   if(bottomBorderLength)
   {
      theBottomBorderLength = rspfString(bottomBorderLength).toULong();
   }
   if(leftBorderLength)
   {
      theLeftBorderLength = rspfString(leftBorderLength).toULong();
   }
   if(rightBorderLength)
   {
      theRightBorderLength = rspfString(rightBorderLength).toULong();
   }

   if(borderColor)
   {
      istringstream in(borderColor);

      rspfString r;
      rspfString g;
      rspfString b;
      in >> r >> g >> b;

      theBorderColor = rspfRgbVector((rspf_uint8)r.toUInt32(), 
                                      (rspf_uint8)g.toUInt32(), 
                                      (rspf_uint8)b.toUInt32());
   }

   if(geoGridColor)
   {
      istringstream in(geoGridColor);

      rspfString r;
      rspfString g;
      rspfString b;
      in >> r >> g >> b;
      
      theGeographicGridColor = rspfRgbVector((rspf_uint8)r.toUInt32(), (rspf_uint8)g.toUInt32(), (rspf_uint8)b.toUInt32());
   }

   if(meterGridColor)
   {
      istringstream in(meterGridColor);

      rspfString r;
      rspfString g;
      rspfString b;
      in >> r >> g >> b;
      
      theMeterGridColor = rspfRgbVector((rspf_uint8)r.toUInt32(), (rspf_uint8)g.toUInt32(), (rspf_uint8)b.toUInt32());      
   }

   if(titleString)
   {
      theTitleString = titleString;
   }
   
   if(titleColor)
   {
      istringstream in(titleColor);

      rspfString r;
      rspfString g;
      rspfString b;
      in >> r >> g >> b;
      
      
      theTitleColor = rspfRgbVector((rspf_uint8)r.toUInt32(), (rspf_uint8)g.toUInt32(), (rspf_uint8)b.toUInt32());
   }

   if(topGeoLabelColor)
   {
      istringstream in(topGeoLabelColor);

      rspfString r;
      rspfString g;
      rspfString b;
      in >> r >> g >> b;
      
      theTopGeographicLabelColor = rspfRgbVector((rspf_uint8)r.toUInt32(), (rspf_uint8)g.toUInt32(), (rspf_uint8)b.toUInt32());
   }

   if(bottomGeoLabelColor)
   {
      istringstream in(bottomGeoLabelColor);

      rspfString r;
      rspfString g;
      rspfString b;
      in >> r >> g >> b;
      
      theBottomGeographicLabelColor = rspfRgbVector((rspf_uint8)r.toUInt32(), (rspf_uint8)g.toUInt32(), (rspf_uint8)b.toUInt32());
   }

   if(leftGeoLabelColor)
   {
      istringstream in(leftGeoLabelColor);

      rspfString r;
      rspfString g;
      rspfString b;
      in >> r >> g >> b;

      theLeftGeographicLabelColor = rspfRgbVector((rspf_uint8)r.toUInt32(), (rspf_uint8)g.toUInt32(), (rspf_uint8)b.toUInt32());
   }

   if(rightGeoLabelColor)
   {
      istringstream in(rightGeoLabelColor);

      rspfString r;
      rspfString g;
      rspfString b;
      in >> r >> g >> b;
      
      theRightGeographicLabelColor = rspfRgbVector((rspf_uint8)r.toUInt32(), (rspf_uint8)g.toUInt32(), (rspf_uint8)b.toUInt32());
   }


/////
   if(topMeterLabelColor)
   {
      istringstream in(topMeterLabelColor);

      rspfString r;
      rspfString g;
      rspfString b;
      in >> r >> g >> b;
      
      theTopMeterLabelColor = rspfRgbVector((rspf_uint8)r.toUInt32(), (rspf_uint8)g.toUInt32(), (rspf_uint8)b.toUInt32());
   }

   if(bottomMeterLabelColor)
   {
      istringstream in(bottomMeterLabelColor);

      rspfString r;
      rspfString g;
      rspfString b;
      in >> r >> g >> b;
      
      theBottomMeterLabelColor = rspfRgbVector((rspf_uint8)r.toUInt32(), (rspf_uint8)g.toUInt32(), (rspf_uint8)b.toUInt32());
   }

   if(leftMeterLabelColor)
   {
      istringstream in(leftMeterLabelColor);

      rspfString r;
      rspfString g;
      rspfString b;
      in >> r >> g >> b;
      
      theLeftMeterLabelColor = rspfRgbVector((rspf_uint8)r.toUInt32(), (rspf_uint8)g.toUInt32(), (rspf_uint8)b.toUInt32());
   }

   if(rightMeterLabelColor)
   {
      istringstream in(rightMeterLabelColor);

      rspfString r;
      rspfString g;
      rspfString b;
      in >> r >> g >> b;
      
      theRightMeterLabelColor = rspfRgbVector((rspf_uint8)r.toUInt32(), (rspf_uint8)g.toUInt32(), (rspf_uint8)b.toUInt32());
   }

   if(topGeoLabelFormat)
   {
      theTopGeographicFormat = topGeoLabelFormat;
   }
   
   if(bottomGeoLabelFormat)
   {
      theBottomGeographicFormat = bottomGeoLabelFormat;
   }
   
   if(leftGeoLabelFormat)
   {
      theLeftGeographicFormat = leftGeoLabelFormat;
   }
   
   if(rightGeoLabelFormat)
   {
      theRightGeographicFormat = rightGeoLabelFormat;
   }

   if(topGeoLabelFlag)
   {
      theTopGeographicLabelFlag = rspfString(topGeoLabelFlag).toBool();
   }
   if(bottomGeoLabelFlag)
   {
      theBottomGeographicLabelFlag = rspfString(bottomGeoLabelFlag).toBool();
   }
   if(leftGeoLabelFlag)
   {
      theLeftGeographicLabelFlag = rspfString(leftGeoLabelFlag).toBool();
   }
   if(rightGeoLabelFlag)
   {
      theRightGeographicLabelFlag = rspfString(rightGeoLabelFlag).toBool();
   }

   if(topMeterLabelFlag)
   {
      theTopMeterLabelFlag = rspfString(topMeterLabelFlag).toBool();
   }
   if(bottomMeterLabelFlag)
   {
      theBottomMeterLabelFlag = rspfString(bottomMeterLabelFlag).toBool();
   }
   if(leftMeterLabelFlag)
   {
      theLeftMeterLabelFlag = rspfString(leftMeterLabelFlag).toBool();
   }
   if(rightMeterLabelFlag)
   {
      theRightMeterLabelFlag = rspfString(rightMeterLabelFlag).toBool();
   }
   
   theGeographicTopLabelFontInfo.loadState(kwl, (rspfString(prefix) + rspfString(TOP_GEO_LABEL_FONT_KW) + ".").c_str());
   theGeographicBottomLabelFontInfo.loadState(kwl, (rspfString(prefix) + rspfString(BOTTOM_GEO_LABEL_FONT_KW) + ".").c_str());
   theGeographicLeftLabelFontInfo.loadState(kwl, (rspfString(prefix) + rspfString(LEFT_GEO_LABEL_FONT_KW) + ".").c_str());
   theGeographicRightLabelFontInfo.loadState(kwl, (rspfString(prefix) + rspfString(RIGHT_GEO_LABEL_FONT_KW) + ".").c_str());

   theMeterTopLabelFontInfo.loadState(kwl, (rspfString(prefix) + rspfString(TOP_METER_LABEL_FONT_KW) + ".").c_str());
   theMeterBottomLabelFontInfo.loadState(kwl, (rspfString(prefix) + rspfString(BOTTOM_METER_LABEL_FONT_KW) + ".").c_str());
   theMeterLeftLabelFontInfo.loadState(kwl, (rspfString(prefix) + rspfString(LEFT_METER_LABEL_FONT_KW) + ".").c_str());
   theMeterRightLabelFontInfo.loadState(kwl, (rspfString(prefix) + rspfString(RIGHT_METER_LABEL_FONT_KW) + ".").c_str());
   
   theTitleFontInfo.loadState(kwl, (rspfString(prefix) + rspfString(TITLE_FONT_KW) + ".").c_str());
   theTitleFont = rspfFontFactoryRegistry::instance()->createFont(theTitleFontInfo);
   theGeographicTopLabelFont = rspfFontFactoryRegistry::instance()->createFont(theGeographicTopLabelFontInfo);
   theGeographicBottomLabelFont = rspfFontFactoryRegistry::instance()->createFont(theGeographicBottomLabelFontInfo);
   theGeographicLeftLabelFont = rspfFontFactoryRegistry::instance()->createFont(theGeographicLeftLabelFontInfo);
   theGeographicRightLabelFont = rspfFontFactoryRegistry::instance()->createFont(theGeographicRightLabelFontInfo);

   theMeterTopLabelFont = rspfFontFactoryRegistry::instance()->createFont(theMeterTopLabelFontInfo);
   theMeterBottomLabelFont = rspfFontFactoryRegistry::instance()->createFont(theMeterBottomLabelFontInfo);
   theMeterLeftLabelFont = rspfFontFactoryRegistry::instance()->createFont(theMeterLeftLabelFontInfo);
   theMeterRightLabelFont = rspfFontFactoryRegistry::instance()->createFont(theMeterRightLabelFontInfo);
   
   vector<rspfFontInformation> info;

   
   if(!theTitleFont)
   {
      theTitleFont = (rspfFont*)rspfFontFactoryRegistry::instance()->getDefaultFont()->dup();
      theTitleFont->getFontInformation(info);
      theTitleFontInfo = info[0];
   }
   if(!theGeographicTopLabelFont)
   {
      info.clear();
      theGeographicTopLabelFont = (rspfFont*)rspfFontFactoryRegistry::instance()->getDefaultFont()->dup();
      theGeographicTopLabelFont->getFontInformation(info);
      theGeographicTopLabelFontInfo = info[0];
   }
   if(!theGeographicBottomLabelFont)
   {
      info.clear();
      theGeographicBottomLabelFont = (rspfFont*)rspfFontFactoryRegistry::instance()->getDefaultFont()->dup();
      theGeographicBottomLabelFont->getFontInformation(info);
      theGeographicBottomLabelFontInfo = info[0];
   }
   if(!theGeographicLeftLabelFont)
   {
      info.clear();
      theGeographicLeftLabelFont = (rspfFont*)rspfFontFactoryRegistry::instance()->getDefaultFont()->dup();
      theGeographicLeftLabelFont->getFontInformation(info);
      theGeographicLeftLabelFontInfo = info[0];
   }
   if(!theGeographicRightLabelFont)
   {
      info.clear();
      theGeographicRightLabelFont = (rspfFont*)rspfFontFactoryRegistry::instance()->getDefaultFont()->dup();
      theGeographicRightLabelFont->getFontInformation(info);
      theGeographicRightLabelFontInfo = info[0];
   }

   if(!theMeterTopLabelFont)
   {
      info.clear();
      theMeterTopLabelFont = (rspfFont*)rspfFontFactoryRegistry::instance()->getDefaultFont()->dup();
      theMeterTopLabelFont->getFontInformation(info);
      theMeterTopLabelFontInfo = info[0];
   }
   if(!theMeterBottomLabelFont)
   {
      info.clear();
      theMeterBottomLabelFont = (rspfFont*)rspfFontFactoryRegistry::instance()->getDefaultFont()->dup();
      theMeterBottomLabelFont->getFontInformation(info);
      theMeterBottomLabelFontInfo = info[0];
   }
   if(!theMeterLeftLabelFont)
   {
      info.clear();
      theMeterLeftLabelFont = (rspfFont*)rspfFontFactoryRegistry::instance()->getDefaultFont()->dup();
      theMeterLeftLabelFont->getFontInformation(info);
      theMeterLeftLabelFontInfo = info[0];
   }
   if(!theMeterRightLabelFont)
   {
      info.clear();
      theMeterRightLabelFont = (rspfFont*)rspfFontFactoryRegistry::instance()->getDefaultFont()->dup();
      theMeterRightLabelFont->getFontInformation(info);
      theMeterRightLabelFontInfo = info[0];
   }

   bool result =  rspfAnnotationSource::loadState(kwl,
                                                   prefix);
   layoutAnnotations();

   return result;
}

rspfScalarType rspfMapCompositionSource::getOutputScalarType() const
{
   return RSPF_UCHAR;
}

rspf_uint32 rspfMapCompositionSource::getNumberOfOutputBands() const
{
   return 3;
}

rspfIpt rspfMapCompositionSource::getViewWidthHeight()const
{
   return theViewWidthHeight;
}

void rspfMapCompositionSource::getViewWidthHeight(rspfIpt& widthHeight)const
{
   widthHeight = theViewWidthHeight;
}

void rspfMapCompositionSource::setViewWidthHeight(const rspfIpt& widthHeight)
{
   theViewWidthHeight = widthHeight;
}

rspf_int32 rspfMapCompositionSource::getTopBorderLength()const
{
   return theTopBorderLength;
}

rspf_int32 rspfMapCompositionSource::getBottomBorderLength()const
{
   return theBottomBorderLength;
}

rspf_int32 rspfMapCompositionSource::getLeftBorderLength()const
{
   return theLeftBorderLength;
}

rspf_int32 rspfMapCompositionSource::getRightBorderLength()const
{
   return theRightBorderLength;
}

void rspfMapCompositionSource::setTopBorderLength(rspf_int32 length)
{
   theTopBorderLength = length;
}

void rspfMapCompositionSource::setBottomBorderLength(rspf_int32 length)
{
   theBottomBorderLength = length;
}

void rspfMapCompositionSource::setLeftBorderLength(rspf_int32 length)
{
   theLeftBorderLength = length;
}

void rspfMapCompositionSource::setRightBorderLength(rspf_int32 length)
{
   theRightBorderLength = length;
}

void rspfMapCompositionSource::setGeographicSpacingLat(double value)
{
   theGeographicSpacing.lat = value;
}

void rspfMapCompositionSource::setGeographicSpacingLon(double value)
{
   theGeographicSpacing.lon = value;
}

void rspfMapCompositionSource::setMeterSpacingX(double value)
{
   theMeterSpacing.x = value;
}

void rspfMapCompositionSource::setMeterSpacingY(double value)
{
   theMeterSpacing.y = value;
}

void rspfMapCompositionSource::setMeterSpacing(double x, double y)
{
   theMeterSpacing.x = x;
   theMeterSpacing.y = y;
}

void rspfMapCompositionSource::setGeographicSpacing(double lat, double lon)
{
   theGeographicSpacing.lat = lat;
   theGeographicSpacing.lon = lon;
}

rspfDpt rspfMapCompositionSource::getGeographicSpacing()const
{
   return theGeographicSpacing;
}

rspfDpt rspfMapCompositionSource::getMeterSpacing()const
{
   return theMeterSpacing;
}

rspfString rspfMapCompositionSource::getTopGeographicLabelFormat()const
{
   return theTopGeographicFormat;
}

rspfString rspfMapCompositionSource::getBottomGeographicLabelFormat()const
{
   return theBottomGeographicFormat;
}

rspfString rspfMapCompositionSource::getLeftGeographicLabelFormat()const
{
   return theLeftGeographicFormat;
}

rspfString rspfMapCompositionSource::getRightGeographicLabelFormat()const
{
   return theRightGeographicFormat;
}

rspfString rspfMapCompositionSource::getTitle()const
{
   return theTitleString;
}

rspfRgbVector rspfMapCompositionSource::getTitleColor()const
{
   return theTitleColor;
}

void rspfMapCompositionSource::setTitleColor(const rspfRgbVector& color)
{
   if(theTitleColor != color)
   {
      theTitleColor = color;
   }
}

void rspfMapCompositionSource::setTitle(const rspfString& s)
{
   if(theTitleString != s)
   {
      theTitleString = s;
   }
         
}

rspfFontInformation rspfMapCompositionSource::getTitleFont()const
{
   return theTitleFontInfo;
}

rspfFontInformation rspfMapCompositionSource::getGeographicTopLabelFontInfo()const
{
   return theGeographicTopLabelFontInfo;
}

rspfFontInformation rspfMapCompositionSource::getGeographicBottomLabelFontInfo()const
{
   return theGeographicBottomLabelFontInfo;
}

rspfFontInformation rspfMapCompositionSource::getGeographicLeftLabelFontInfo()const
{
   return theGeographicLeftLabelFontInfo;
}

rspfFontInformation rspfMapCompositionSource::getGeographicRightLabelFontInfo()const
{
   return theGeographicRightLabelFontInfo;
}

rspfFontInformation rspfMapCompositionSource::getMeterTopLabelFontInfo()const
{
   return theMeterTopLabelFontInfo;
}

rspfFontInformation rspfMapCompositionSource::getMeterBottomLabelFontInfo()const
{
   return theMeterBottomLabelFontInfo;
}

rspfFontInformation rspfMapCompositionSource::getMeterLeftLabelFontInfo()const
{
   return theMeterLeftLabelFontInfo;
}

rspfFontInformation rspfMapCompositionSource::getMeterRightLabelFontInfo()const
{
   return theMeterRightLabelFontInfo;
}
   
rspfRgbVector rspfMapCompositionSource::getBorderColor()const
{
   return theBorderColor;
}

void rspfMapCompositionSource::setBorderColor(const rspfRgbVector& color)
{
   theBorderColor=color;
}

rspfRgbVector rspfMapCompositionSource::getGeographicGridColor()const
{
   return theGeographicGridColor;
}

rspfRgbVector rspfMapCompositionSource::getMeterGridColor()const
{
   return theMeterGridColor;
}

rspfRgbVector rspfMapCompositionSource::getTopGeographicLabelColor()const
{
   return theTopGeographicLabelColor;
}

rspfRgbVector rspfMapCompositionSource::getBottomGeographicLabelColor()const
{
   return theBottomGeographicLabelColor;
}

rspfRgbVector rspfMapCompositionSource::getLeftGeographicLabelColor()const
{
   return theLeftGeographicLabelColor;
}

rspfRgbVector rspfMapCompositionSource::getRightGeographicLabelColor()const
{
   return theRightGeographicLabelColor;
}

void rspfMapCompositionSource::setGeographicGridColor(const rspfRgbVector& color)
{
   theGeographicGridColor = color;
}

void rspfMapCompositionSource::setTopGeographicLabelColor(const rspfRgbVector& color)
{
   theTopGeographicLabelColor = color;
}

void rspfMapCompositionSource::setBottomGeographicLabelColor(const rspfRgbVector& color)
{
   theBottomGeographicLabelColor = color;
}

void rspfMapCompositionSource::setLeftGeographicLabelColor(const rspfRgbVector& color)
{
   theLeftGeographicLabelColor = color;
}

void rspfMapCompositionSource::setRightGeographicLabelColor(const rspfRgbVector& color)
{
   theRightGeographicLabelColor = color;
}

void rspfMapCompositionSource::setGeographicLabelColor(const rspfRgbVector& color)
{
   setTopGeographicLabelColor(color);
   setBottomGeographicLabelColor(color);
   setLeftGeographicLabelColor(color);
   setRightGeographicLabelColor(color);
}

rspfRgbVector rspfMapCompositionSource::getTopMeterLabelColor()const
{
   return theTopMeterLabelColor;
}

rspfRgbVector rspfMapCompositionSource::getBottomMeterLabelColor()const
{
   return theBottomMeterLabelColor;
}

rspfRgbVector rspfMapCompositionSource::getLeftMeterLabelColor()const
{
   return theLeftMeterLabelColor;
}

rspfRgbVector rspfMapCompositionSource::getRightMeterLabelColor()const
{
   return theRightMeterLabelColor;
}
   
void rspfMapCompositionSource::setTopMeterLabelColor(const rspfRgbVector& color)
{
   theTopMeterLabelColor = color;
}

void rspfMapCompositionSource::setBottomMeterLabelColor(const rspfRgbVector& color)
{
   theBottomMeterLabelColor = color;
}

void rspfMapCompositionSource::setLeftMeterLabelColor(const rspfRgbVector& color)
{
   theLeftMeterLabelColor = color;
}

void rspfMapCompositionSource::setRightMeterLabelColor(const rspfRgbVector& color)
{
   theRightMeterLabelColor = color;
}

void rspfMapCompositionSource::setMeterLabelColor(const rspfRgbVector& color)
{
   setTopMeterLabelColor(color);
   setBottomMeterLabelColor(color);
   setLeftMeterLabelColor(color);
   setRightMeterLabelColor(color);
}
   
void rspfMapCompositionSource::setMeterGridColor(const rspfRgbVector& color)
{
   theMeterGridColor = color;
}

void rspfMapCompositionSource::setMeterGridType(rspfGridLineType gridType)
{
   theMeterGridType = gridType;
}

void rspfMapCompositionSource::setGeographicGridType(rspfGridLineType gridType)
{
   theGeographicGridType = gridType;
}

void rspfMapCompositionSource::setBorderColor(rspfRgbVector& color)
{
   theBorderColor = color;
}

rspfMapCompositionSource::rspfGridLineType rspfMapCompositionSource::getGeographicGridType()const
{
   return theGeographicGridType;
}

rspfMapCompositionSource::rspfGridLineType rspfMapCompositionSource::getMeterGridType()const
{
   return theMeterGridType;
}
 
void rspfMapCompositionSource::setGeographicTickMarkFlag(bool flag)
{
   setTopGeographicTickMarkFlag(flag);
   setBottomGeographicTickMarkFlag(flag);
   setLeftGeographicTickMarkFlag(flag);
   setRightGeographicTickMarkFlag(flag);
}

void rspfMapCompositionSource::setTopGeographicTickMarkFlag(bool flag)
{
   theTopGeographicTickFlag = flag;
}

void rspfMapCompositionSource::setBottomGeographicTickMarkFlag(bool flag)
{
   theBottomGeographicTickFlag = flag;
}

void rspfMapCompositionSource::setLeftGeographicTickMarkFlag(bool flag)
{
   theLeftGeographicTickFlag = flag;
}

void rspfMapCompositionSource::setRightGeographicTickMarkFlag(bool flag)
{
   theRightGeographicTickFlag = flag;
}

void rspfMapCompositionSource::setTopGeographicLabelFlag(bool flag)
{
   theTopGeographicLabelFlag = flag;
}

void rspfMapCompositionSource::setBottomGeographicLabelFlag(bool flag)
{
   theBottomGeographicLabelFlag = flag;
}

void rspfMapCompositionSource::setLeftGeographicLabelFlag(bool flag)
{
   theLeftGeographicLabelFlag = flag;
}

void rspfMapCompositionSource::setRightGeographicLabelFlag(bool flag)
{
   theRightGeographicLabelFlag = flag;
}

bool rspfMapCompositionSource::getTopGeographicTickMarkFlag()const
{
   return theTopGeographicTickFlag;
}

bool rspfMapCompositionSource::getBottomGeographicTickMarkFlag()const
{
   return theBottomGeographicTickFlag;
}

bool rspfMapCompositionSource::getLeftGeographicTickMarkFlag()const
{
   return theLeftGeographicTickFlag;
}

bool rspfMapCompositionSource::getRightGeographicTickMarkFlag()const
{
   return theRightGeographicTickFlag;
}

bool rspfMapCompositionSource::getTopGeographicLabelFlag()const
{
   return theTopGeographicLabelFlag;
}

bool rspfMapCompositionSource::getBottomGeographicLabelFlag()const
{
   return theBottomGeographicLabelFlag;
}

bool rspfMapCompositionSource::getLeftGeographicLabelFlag()const
{
   return theLeftGeographicLabelFlag;
}

bool rspfMapCompositionSource::getRightGeographicLabelFlag()const
{
   return theRightGeographicLabelFlag;
}

void rspfMapCompositionSource::setMeterTickMarkFlag(bool flag)
{
   setTopMeterTickMarkFlag(flag);
   setBottomMeterTickMarkFlag(flag);
   setLeftMeterTickMarkFlag(flag);
   setRightMeterTickMarkFlag(flag);
}

void rspfMapCompositionSource::setTopMeterTickMarkFlag(bool flag)
{
   theTopMeterTickFlag = flag;
}

void rspfMapCompositionSource::setBottomMeterTickMarkFlag(bool flag)
{
   theBottomMeterTickFlag = flag;
}

void rspfMapCompositionSource::setLeftMeterTickMarkFlag(bool flag)
{
   theLeftMeterTickFlag = flag;
}

void rspfMapCompositionSource::setRightMeterTickMarkFlag(bool flag)
{
   theRightMeterTickFlag = flag;
}

void rspfMapCompositionSource::setTopMeterLabelFlag(bool flag)
{
   theTopMeterLabelFlag = flag;
}

void rspfMapCompositionSource::setBottomMeterLabelFlag(bool flag)
{
   theBottomMeterLabelFlag = flag;
}

void rspfMapCompositionSource::setLeftMeterLabelFlag(bool flag)
{
   theLeftMeterLabelFlag = flag;
}

void rspfMapCompositionSource::setRightMeterLabelFlag(bool flag)
{
   theRightMeterLabelFlag = flag;
}
   
bool rspfMapCompositionSource::getTopMeterTickMarkFlag()const
{
   return theTopMeterTickFlag;
}

bool rspfMapCompositionSource::getBottomMeterTickMarkFlag()const
{
   return theBottomMeterTickFlag;
}

bool rspfMapCompositionSource::getLeftMeterTickMarkFlag()const
{
   return theLeftMeterTickFlag;
}

bool rspfMapCompositionSource::getRightMeterTickMarkFlag()const
{
   return theRightMeterTickFlag;
}

bool rspfMapCompositionSource::getTopMeterLabelFlag()const
{
   return theTopMeterLabelFlag;
}

bool rspfMapCompositionSource::getBottomMeterLabelFlag()const
{
   return theBottomMeterLabelFlag;
}

bool rspfMapCompositionSource::getLeftMeterLabelFlag()const
{
   return theLeftMeterLabelFlag;
}

bool rspfMapCompositionSource::getRightMeterLabelFlag()const
{
   return theRightMeterLabelFlag;
}

void rspfMapCompositionSource::addFixedAnnotation(rspfAnnotationObject* obj)
{
   if(obj)
   {
      theFixedAnnotationList.push_back(obj);
   }
}

//**************************************************************************************************
//! Fetches the input connection's image geometry and verifies that it is a map projection.
//! Returns NULL if no valid projection found.
//**************************************************************************************************
const rspfMapProjection* rspfMapCompositionSource::inputMapProjection() const
{
   if (!theInputConnection)
      return 0;

   rspfRefPtr<rspfImageGeometry> inputGeom = theInputConnection->getImageGeometry();
   if (!inputGeom)
      return 0;

   return PTR_CAST(rspfMapProjection, inputGeom->getProjection());
}

