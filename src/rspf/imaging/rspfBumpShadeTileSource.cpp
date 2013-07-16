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
// $Id: rspfBumpShadeTileSource.cpp 20198 2011-11-03 13:23:40Z dburken $

#include <rspf/imaging/rspfBumpShadeTileSource.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfTilePatch.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/base/rspfColumnVector3d.h>
#include <rspf/base/rspfNumericProperty.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfKeyword.h>
#include <rspf/base/rspfMatrix3x3.h>
#include <rspf/base/rspfRgbVector.h>

static const char COLOR_RED_KW[]   = "color_red";
static const char COLOR_GREEN_KW[] = "color_green";
static const char COLOR_BLUE_KW[]  = "color_blue";

RTTI_DEF1(rspfBumpShadeTileSource,
          "rspfBumpShadeTileSource",
          rspfImageCombiner);


rspfBumpShadeTileSource::rspfBumpShadeTileSource()
   :rspfImageCombiner(0, 2, 0, true, false),
    m_tile(0),
    m_lightSourceElevationAngle(45.0),
    m_lightSourceAzimuthAngle(45.0),
    m_lightDirection(3),
    m_r(255),
    m_g(255),
    m_b(255)
{
   initialize();
}

rspfBumpShadeTileSource::~rspfBumpShadeTileSource()
{
}

rspfRefPtr<rspfImageData> rspfBumpShadeTileSource::getTile(
   const  rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   if(!getInput(0)) return 0;

   if(!m_tile.get())
   {
      allocate();
   }
   if(!m_tile.valid())
   {
      return m_tile;
   }
   rspfImageSource* normalSource =
      PTR_CAST(rspfImageSource, getInput(0));
   rspfImageSource* colorSource =
      PTR_CAST(rspfImageSource, getInput(1));
   
   if(!m_tile.get())
   {
      return rspfRefPtr<rspfImageData>();
   }

   m_tile->setImageRectangle(tileRect);
 
   rspfRefPtr<rspfImageData> inputTile = 0;

   if(isSourceEnabled())
   {
      m_tile->makeBlank();
      
      if(colorSource)
      {
         
         inputTile = colorSource->getTile(tileRect, resLevel);
      }
      rspfRefPtr<rspfImageData> normalData = normalSource->getTile(tileRect, resLevel);
      if(!normalData)
      {
         return rspfRefPtr<rspfImageData>();
      }
      if ( (normalData->getDataObjectStatus() == RSPF_NULL) ||
           (normalData->getDataObjectStatus() == RSPF_EMPTY)||
           (normalData->getNumberOfBands() != 3)||
           (normalData->getScalarType() != RSPF_DOUBLE))
      {
         return rspfRefPtr<rspfImageData>();
      }
      rspf_float64* normalBuf[3];
      normalBuf[0] = static_cast<rspf_float64*>(normalData->getBuf(0));
      normalBuf[1] = static_cast<rspf_float64*>(normalData->getBuf(1));
      normalBuf[2] = static_cast<rspf_float64*>(normalData->getBuf(2));
      rspf_float64 normalNp = normalData->getNullPix(0);
      // if we have some color data then use it for the bump
      // else we will default to a grey scale bump shade
      //
      if(inputTile.get() &&
         (inputTile->getDataObjectStatus() != RSPF_EMPTY) &&
         (inputTile->getDataObjectStatus() != RSPF_NULL))
         
      {
         switch(inputTile->getScalarType())
         {
            case RSPF_UCHAR:
            {
               rspf_uint8* resultBuf[3];
               rspf_uint8* colorBuf[3];
               resultBuf[0] = static_cast<rspf_uint8*>(m_tile->getBuf(0));
               resultBuf[1] = static_cast<rspf_uint8*>(m_tile->getBuf(1));
               resultBuf[2] = static_cast<rspf_uint8*>(m_tile->getBuf(2));
               colorBuf[0]  = static_cast<rspf_uint8*>(inputTile->getBuf(0));
               if(inputTile->getBuf(1))
               {
                  colorBuf[1] = static_cast<rspf_uint8*>(inputTile->getBuf(1));
               }
               else
               {
                  colorBuf[1] = colorBuf[0];
               }
               if(inputTile->getBuf(2))
               {
                  colorBuf[2] = static_cast<rspf_uint8*>(inputTile->getBuf(2));
               }
               else
               {
                  colorBuf[2] = colorBuf[0];
               }
            
               long h = m_tile->getHeight();
               long w = m_tile->getWidth();
               for(long y = 0; y < h; ++y)
               {
                  for(long x = 0; x < w; ++x)
                  {
                     if((*normalBuf[0] != normalNp) &&
                        (*normalBuf[1] != normalNp) &&
                        (*normalBuf[2] != normalNp) )
                     {
                        if((*colorBuf[0])||(*colorBuf[1])||(*colorBuf[2]))
                        {
                           computeColor(*resultBuf[0],
                                        *resultBuf[1],
                                        *resultBuf[2],
                                        *normalBuf[0],
                                        *normalBuf[1],
                                        *normalBuf[2],
                                        *colorBuf[0],
                                        *colorBuf[1],
                                        *colorBuf[2]);
                        }
                        else 
                        {
                           computeColor(*resultBuf[0],
                                        *resultBuf[1],
                                        *resultBuf[2],
                                        *normalBuf[0],
                                        *normalBuf[1],
                                        *normalBuf[2],
                                        m_r,
                                        m_g,
                                        m_b);
                        }
                     }
                     else
                     {
                        *resultBuf[0] = *colorBuf[0];
                        *resultBuf[1] = *colorBuf[1];
                        *resultBuf[2] = *colorBuf[2];
                     }
                     resultBuf[0]++;
                     resultBuf[1]++;
                     resultBuf[2]++;
                     colorBuf[0]++;
                     colorBuf[1]++;
                     colorBuf[2]++;
                     normalBuf[0]++;
                     normalBuf[1]++;
                     normalBuf[2]++;
                  }
               }
               break;
            }
            default:
            {
               rspfNotify(rspfNotifyLevel_NOTICE)
                  << "rspfBumpShadeTileSource::getTile NOTICE:\n"
                  << "only 8-bit unsigned char is supported." << endl;
            }
         }
      }
      else
      {
         rspf_uint8* resultBuf[3];
         resultBuf[0] = static_cast<rspf_uint8*>(m_tile->getBuf(0));
         resultBuf[1] = static_cast<rspf_uint8*>(m_tile->getBuf(1));
         resultBuf[2] = static_cast<rspf_uint8*>(m_tile->getBuf(2));
         long h = m_tile->getHeight();
         long w = m_tile->getWidth();
         for(long y = 0; y < h; ++y)
         {
            for(long x = 0; x < w; ++x)
            {
               if((*normalBuf[0] != normalNp) &&
                  (*normalBuf[1] != normalNp) &&
                  (*normalBuf[2] != normalNp) )
               {
                     computeColor(*resultBuf[0],
                                  *resultBuf[1],
                                  *resultBuf[2],
                                  *normalBuf[0],
                                  *normalBuf[1],
                                  *normalBuf[2],
                                  m_r,
                                  m_g,
                                  m_b);
               }
               else
               {
                  *resultBuf[0] = 0;
                  *resultBuf[1] = 0;
                  *resultBuf[2] = 0;
               }
               resultBuf[0]++;
               resultBuf[1]++;
               resultBuf[2]++;
               normalBuf[0]++;
               normalBuf[1]++;
               normalBuf[2]++;
            }
         }
      }      
   }
   m_tile->validate();
   return m_tile;
}

void rspfBumpShadeTileSource::computeColor(rspf_uint8& r,
                                            rspf_uint8& g,
                                            rspf_uint8& b,
                                            rspf_float64 normalX,
                                            rspf_float64 normalY,
                                            rspf_float64 normalZ,
                                            rspf_uint8 dr,
                                            rspf_uint8 dg,
                                            rspf_uint8 db)const
{
   double c = /*fabs*/(normalX*m_lightDirection[0] +
                   normalY*m_lightDirection[1] +
                   normalZ*m_lightDirection[2]);
   
   r = rspfRgbVector::clamp(rspf::round<int>(c*dr), 1, 255);
   g = rspfRgbVector::clamp(rspf::round<int>(c*dg), 1, 255);
   b = rspfRgbVector::clamp(rspf::round<int>(c*db), 1, 255);
}

void rspfBumpShadeTileSource::initialize()
{
   rspfImageCombiner::initialize();

   rspfImageSource* normalSource = dynamic_cast<rspfImageSource*>( getInput(0) );
   if ( normalSource )
   {
      normalSource->initialize();
   }
   
   rspfImageSource* colorSource = dynamic_cast<rspfImageSource*>( getInput(1) );
   if ( colorSource )
   {
      colorSource->initialize();
   }
   
   m_tile = 0;
   
   computeLightDirection();
}

void rspfBumpShadeTileSource::allocate()
{
   m_tile = rspfImageDataFactory::instance()->create(this, this);
   m_tile->initialize();
}

void rspfBumpShadeTileSource::computeLightDirection()
{
   NEWMAT::Matrix m = rspfMatrix3x3::createRotationMatrix(m_lightSourceElevationAngle,
                                                           0.0,
                                                           -m_lightSourceAzimuthAngle); // make positive rotation clockwise for azimuth
   NEWMAT::ColumnVector v(3);
   v[0] = 0;
   v[1] = 1;
   v[2] = 0;
   v = m*v;
   // reflect Z.  We need the Z pointing up from the surface and not into it.
   //
   rspfColumnVector3d d(v[0], v[1], -v[2]);
   d = d.unit();
   m_lightDirection[0] = d[0];
   m_lightDirection[1] = d[1];
   m_lightDirection[2] = d[2];
}

bool rspfBumpShadeTileSource::loadState(const rspfKeywordlist& kwl,
                                         const char* prefix)
{
   const char* elevationAngle = kwl.find(prefix, rspfKeywordNames::ELEVATION_ANGLE_KW);
   const char* azimuthAngle   = kwl.find(prefix, rspfKeywordNames::AZIMUTH_ANGLE_KW);
   
   
   if(elevationAngle)
   {
      m_lightSourceElevationAngle = rspfString(elevationAngle).toDouble();
   }

   if(azimuthAngle)
   {
      m_lightSourceAzimuthAngle = rspfString(azimuthAngle).toDouble();
   }

   const char* lookup = kwl.find(prefix, COLOR_RED_KW);
   if (lookup)
   {
      m_r = rspfString(lookup).toUInt8();
   }
   
   lookup = kwl.find(prefix, COLOR_GREEN_KW);
   if (lookup)
   {
      m_g = rspfString(lookup).toUInt8();
   }
   
   lookup = kwl.find(prefix, COLOR_BLUE_KW);
   if (lookup)
   {
      m_b = rspfString(lookup).toUInt8();
   }
    

   computeLightDirection();

   bool result = rspfImageSource::loadState(kwl, prefix);

   theInputListIsFixedFlag  = true;
   theOutputListIsFixedFlag = false;
   if(!getNumberOfInputs()) setNumberOfInputs(2);
   
   return result;
}

bool rspfBumpShadeTileSource::saveState(rspfKeywordlist& kwl,
                                         const char* prefix)const
{
   kwl.add(prefix,
           rspfKeywordNames::ELEVATION_ANGLE_KW,
           m_lightSourceElevationAngle,
           true);

   kwl.add(prefix,
           rspfKeywordNames::AZIMUTH_ANGLE_KW,
           m_lightSourceAzimuthAngle,
           true);

   kwl.add(prefix, COLOR_RED_KW,   m_r, true);
   kwl.add(prefix, COLOR_GREEN_KW, m_g, true);
   kwl.add(prefix, COLOR_BLUE_KW,  m_b, true);
   
   return rspfImageSource::saveState(kwl, prefix);
}

rspfString rspfBumpShadeTileSource::getShortName()const
{
   return rspfString("bump shader");
}

rspfString rspfBumpShadeTileSource::getLongName()const
{
   return rspfString("Blinn's bump map filter");
}

rspf_uint32 rspfBumpShadeTileSource::getNumberOfOutputBands() const
{
   return 3;
}

rspfScalarType rspfBumpShadeTileSource::getOutputScalarType() const
{
   return RSPF_UCHAR;
}

double rspfBumpShadeTileSource::getNullPixelValue()const
{
   return 0.0;
}

double rspfBumpShadeTileSource::getMinPixelValue(rspf_uint32 /* band */)const
{
   return 1.0;
}

double rspfBumpShadeTileSource::getMaxPixelValue(rspf_uint32 /* band */)const
{
   return 255.0;
}

double rspfBumpShadeTileSource::getAzimuthAngle()const
{
   return m_lightSourceAzimuthAngle;
}

double rspfBumpShadeTileSource::getElevationAngle()const
{
   return m_lightSourceElevationAngle;
}

void rspfBumpShadeTileSource::setAzimuthAngle(double angle)
{
   m_lightSourceAzimuthAngle = angle;
}

void rspfBumpShadeTileSource::setElevationAngle(double angle)
{
   m_lightSourceElevationAngle = angle;
}

bool rspfBumpShadeTileSource::canConnectMyInputTo(rspf_int32 inputIndex,
                                                   const rspfConnectableObject* object)const
{
   return (object&&
           ( (inputIndex>=0) && inputIndex < 2)&&
           PTR_CAST(rspfImageSource, object));
   
}

void rspfBumpShadeTileSource::connectInputEvent(rspfConnectionEvent& /* event */)
{
   initialize();
}

void rspfBumpShadeTileSource::disconnectInputEvent(rspfConnectionEvent& /* event */)
{
   initialize();
}

void rspfBumpShadeTileSource::propertyEvent(rspfPropertyEvent& /* event */)
{
   initialize();
}

void rspfBumpShadeTileSource::refreshEvent(rspfRefreshEvent& /* event */)
{
   initialize();
}

void rspfBumpShadeTileSource::setProperty(rspfRefPtr<rspfProperty> property)
{
   rspfString name = property->getName();
   if(name == "lightSourceElevationAngle")
   {
      m_lightSourceElevationAngle = property->valueToString().toDouble();
   }
   else if(name == "lightSourceAzimuthAngle")
   {
      m_lightSourceAzimuthAngle = property->valueToString().toDouble();
   }
   else
   {
      rspfImageCombiner::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfBumpShadeTileSource::getProperty(const rspfString& name)const
{
   if(name == "lightSourceElevationAngle")
   {
      rspfProperty* prop = new rspfNumericProperty(name, rspfString::toString(m_lightSourceElevationAngle), 0.0, 90.0);
      prop->setCacheRefreshBit();
      return prop;
   }
   else if(name == "lightSourceAzimuthAngle")
   {
      rspfProperty* prop = new rspfNumericProperty(name, rspfString::toString(m_lightSourceAzimuthAngle), 0, 360);
      prop->setCacheRefreshBit();
      return prop;
   }
   
   return rspfImageCombiner::getProperty(name);
}

void rspfBumpShadeTileSource::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageCombiner::getPropertyNames(propertyNames);
   propertyNames.push_back("lightSourceElevationAngle");
   propertyNames.push_back("lightSourceAzimuthAngle");
}

void rspfBumpShadeTileSource::setRgbColorSource(rspf_uint8 r,
                                                 rspf_uint8 g,
                                                 rspf_uint8 b)
{
   m_r = r;
   m_g = g;
   m_b = b;
}

void rspfBumpShadeTileSource::getRgbColorSource(rspf_uint8& r,
                                                 rspf_uint8& g,
                                                 rspf_uint8& b) const
{
   r = m_r;
   g = m_g;
   b = m_b;
}
