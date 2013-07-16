//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Oscar Kramer
//
//*******************************************************************
//  $Id: rspfDespeckleFilter.cpp 2644 2011-05-26 15:20:11Z oscar.kramer $

#include <rspf/imaging/rspfDespeckleFilter.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeyword.h>
#include <rspf/base/rspfNumericProperty.h>

static const rspfKeyword FILTER_RADIUS_KW = rspfKeyword("filter_radius", "");

RTTI_DEF1(rspfDespeckleFilter, "rspfDespeckleFilter", rspfImageSourceFilter);

rspfDespeckleFilter::rspfDespeckleFilter()
   : rspfImageSourceFilter(),
     theTile(NULL),
     theFilterRadius(1)
{
}

rspfDespeckleFilter::rspfDespeckleFilter(rspfImageSource* inputSource, rspf_uint32 filter_radius)
   : rspfImageSourceFilter(inputSource),
     theTile(NULL),
     theFilterRadius(filter_radius)
{
   initialize();
}

rspfDespeckleFilter::~rspfDespeckleFilter()
{
   theTile = 0;
}

rspfRefPtr<rspfImageData> 
rspfDespeckleFilter::getTile(const rspfIrect& tileRect,rspf_uint32 resLevel)
{
   if (!theInputConnection) 
      return NULL;

   if (!isSourceEnabled())
      return theInputConnection->getTile(tileRect, resLevel);
   
   if (!theTile.valid())
   {
      allocate();
      if (!theTile.valid())  // Throw exception???
         return theInputConnection->getTile(tileRect, resLevel);
   }
   
   // Fetch input tile with kernel margins:
   rspfIrect requestRect(tileRect.ul().x - theFilterRadius,
                          tileRect.ul().y - theFilterRadius,
                          tileRect.lr().x + theFilterRadius,
                          tileRect.lr().y + theFilterRadius);
   rspfRefPtr<rspfImageData> inputTile = theInputConnection->getTile(requestRect, resLevel);
   if (!inputTile.valid())
      return NULL;

   rspf_uint32 w  = tileRect.width();
   rspf_uint32 h  = tileRect.height();
   rspf_uint32 tw = theTile->getWidth();
   rspf_uint32 th = theTile->getHeight();
   theTile->setWidth(w);
   theTile->setHeight(h);
   if((w*h)!=(tw*th))
      theTile->initialize();
      
   theTile->makeBlank();
   theTile->setOrigin(tileRect.ul());
   
   rspfDataObjectStatus status = inputTile->getDataObjectStatus();
   if ((status == RSPF_NULL) || (status == RSPF_EMPTY))
      return inputTile;

   switch(inputTile->getScalarType())
   {
   case RSPF_UCHAR:
      despeckle(static_cast<rspf_uint8>(0), inputTile);
      break;
   
   case RSPF_USHORT16:
   case RSPF_USHORT11:
      despeckle(static_cast<rspf_uint16>(0), inputTile);
      break;

   case RSPF_SSHORT16:
      despeckle(static_cast<rspf_sint16>(0), inputTile);
      break;

   case RSPF_FLOAT:
   case RSPF_NORMALIZED_FLOAT:
      despeckle(static_cast<float>(0), inputTile);
      break;

   case RSPF_DOUBLE:
   case RSPF_NORMALIZED_DOUBLE:
      despeckle(static_cast<double>(0), inputTile);
      break;

   default:
      theTile->loadTile(inputTile.get());
   }
   
   theTile->validate();
   return theTile;
}
   
template <class T>
void rspfDespeckleFilter::despeckle(T /* dummyVariable */, rspfRefPtr<rspfImageData> inputTile)
{
   rspfIpt inUL  (inputTile->getImageRectangle().ul());
   rspfIpt inLR  (inputTile->getImageRectangle().lr());
   rspfIpt outUL (theTile->getImageRectangle().ul());
   rspfIpt outLR (theTile->getImageRectangle().lr());
   long inWidth   = inputTile->getWidth();
   long outWidth  = theTile->getWidth();
   long num_bands = theTile->getNumberOfBands();
   
      // Loop over all bands first:
   for(long b = 0; b < num_bands; ++b)
   {                  
      const T* inbuf = (const T*) inputTile->getBuf(b);
      T* outBuf = (T*) theTile->getBuf(b);
      T null_pixel = (T) inputTile->getNullPix(b);

      for (long y=outUL.y; y<=outLR.y; y++)
      {
         for (long x=outUL.x; x<=outLR.x; x++)
         {
            bool found_valid = false;
            long idx = (y - inUL.y)*inWidth + x - inUL.x; // index to input buffer
            long odx = (y-outUL.y)*outWidth + x - outUL.x;// index to output buffer
          
            T pixel = inbuf[idx];
            if (pixel != null_pixel)
            {
               // Inside loop over input "kernel" pixels centered at x, y:
               for (long iy=-theFilterRadius; (iy<=theFilterRadius) && !found_valid; iy++)
               {
                  // Compute row number in image space and skip this row if outside input tile:
                  long row = y + iy;
                  if ((row < inUL.y) || (row > inLR.y))
                     continue;

                  for (long ix=-theFilterRadius; (ix<=theFilterRadius) && !found_valid; ix++)
                  {
                     // Compute column number in image space and skip this col if outside input tile:
                     long col = x + ix;
                     if ((col < inUL.x) || (col > inLR.x))
                        continue;

                     // Compute offset into input buffer for the neighbor pixel. If we aren't at the 
                     // kernel center, consider if valid neighbor was found:
                     idx = (row - inUL.y)*inWidth + col - inUL.x;
                     if ( ((iy != 0) || (ix != 0)) && (inbuf[idx] != null_pixel) )
                        found_valid = true;
                  }
               }
            }

            // Finished scanning neighborhood. Save output to tile buffer:
            if (found_valid)
               outBuf[odx] = pixel;
            else
               outBuf[odx] = null_pixel;
         }
      }
   }
}

void rspfDespeckleFilter::initialize()
{
   rspfImageSourceFilter::initialize();
   theTile = NULL;
}

void rspfDespeckleFilter::allocate()
{
   if(theInputConnection)
   {
      theTile = rspfImageDataFactory::instance()->create(this, theInputConnection);
      theTile->initialize();
   }
}

bool rspfDespeckleFilter::saveState(rspfKeywordlist& kwl, const char* prefix) const
{
   kwl.add(prefix, FILTER_RADIUS_KW, theFilterRadius, true);
   return rspfImageSourceFilter::saveState(kwl, prefix);
}


bool rspfDespeckleFilter::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   const char* lookup = kwl.find(prefix, FILTER_RADIUS_KW);
   if (lookup)
      theFilterRadius = rspfString(lookup).toInt32();

   return rspfImageSourceFilter::loadState(kwl, prefix);
}

void rspfDespeckleFilter::setProperty(rspfRefPtr<rspfProperty> property)
{
   rspfString name = property->getName();
   if(name == "Filter Radius")
      theFilterRadius = property->valueToString().toInt32();
   else
      rspfImageSourceFilter::setProperty(property);
}

rspfRefPtr<rspfProperty> rspfDespeckleFilter::getProperty(const rspfString& name)const
{
   if(name == "Filter Radius")
   {
      rspfProperty* prop = new rspfNumericProperty(name, rspfString::toString(theFilterRadius), 1, 16.0);
      prop->setCacheRefreshBit();
      return prop;
   }
   return rspfImageSourceFilter::getProperty(name);
}

void rspfDespeckleFilter::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageSourceFilter::getPropertyNames(propertyNames);
   propertyNames.push_back("Filter Radius");
}

