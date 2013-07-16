//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
// 
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfRpfTocEntry.cpp 19900 2011-08-04 14:19:57Z dburken $

#include <istream>
#include <ostream>
#include <iterator>

#include <rspf/support_data/rspfRpfTocEntry.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfErrorCodes.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/projection/rspfEquDistCylProjection.h>
#include <rspf/projection/rspfAzimEquDistProjection.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/support_data/rspfRpfFrameEntry.h>

std::ostream& operator <<(std::ostream& out,
                          const rspfRpfTocEntry& data)
{
   data.print(out);
   return out;
}

rspfRpfTocEntry::rspfRpfTocEntry()
{
   
}

rspfErrorCode rspfRpfTocEntry::parseStream(
   std::istream &in, rspfByteOrder byteOrder)
{
   rspfErrorCode result = rspfErrorCodes::RSPF_OK;
   
   result = theBoundaryInformation.parseStream(in, byteOrder);
   if(result == rspfErrorCodes::RSPF_OK)
   {
      allocateFrameEntryArray();
   }

   // Fetch the number of samples as data member since it may need to be adjusted due to 
   // wrap (OLK 10/10):
   theNumSamples =  theBoundaryInformation.getNumberOfFramesHorizontal() * 1536;
   rspfDpt ddpp;
   getDecimalDegreesPerPixel(ddpp);
   rspf_float64 width_in_deg = theNumSamples*ddpp.x;
   if (width_in_deg >= 360.0)
      theNumSamples -= (width_in_deg - 360.0)/ddpp.x;

   return result;
}

std::ostream& rspfRpfTocEntry::print(std::ostream& out,
                                      const std::string& prefix) const
{
   theBoundaryInformation.print(out, prefix);

   std::vector< std::vector<rspfRpfFrameEntry> >::const_iterator frameEntry =
      theFrameEntryArray.begin();

   while( frameEntry != theFrameEntryArray.end() )
   {
      std::vector<rspfRpfFrameEntry>::const_iterator i =
         (*frameEntry).begin();
      while ( i != (*frameEntry).end() ) 
      {
         (*i).print(out, prefix);
         ++i;
      }
      ++frameEntry;
   }
   return out;
}

rspf_uint32 rspfRpfTocEntry::getNumberOfFramesHorizontal()const
{
   return theBoundaryInformation.getNumberOfFramesHorizontal();
}

rspf_uint32 rspfRpfTocEntry::getNumberOfFramesVertical()const
{
   return theBoundaryInformation.getNumberOfFramesVertical();
}

rspf_uint32 rspfRpfTocEntry::getNumberOfLines() const
{
   return  theBoundaryInformation.getNumberOfFramesVertical() * 1536;
}

rspf_uint32 rspfRpfTocEntry::getNumberOfSamples() const
{
   return theNumSamples;
}

rspf_uint32 rspfRpfTocEntry::getNumberOfBands() const
{
   rspf_uint32 result = 0;
   rspfString productType = getProductType().trim().upcase();
   if( productType == "CIB" )
   {
      result = 1;
   }
   else if ( productType == "CADRG" )
   {
      result = 3;
   }
   return result;
}

void rspfRpfTocEntry::getBoundingRect(rspfIrect& rect) const
{
   rect = rspfIrect(0, 0, getNumberOfSamples()-1, getNumberOfLines()-1); 
}

const rspfRpfBoundaryRectRecord& rspfRpfTocEntry::getBoundaryInformation() const
{
   return theBoundaryInformation;
}

void rspfRpfTocEntry::setEntry(const rspfRpfFrameEntry& entry,
                                long row,
                                long col)
{
   if(row < (long)theFrameEntryArray.size() && row >= 0)
   {
      if(col < (long)theFrameEntryArray[row].size() && col >= 0)
      {
         theFrameEntryArray[row][col] = entry;
      }
   }
}

bool rspfRpfTocEntry::getEntry(long row,
                                long col,
                                rspfRpfFrameEntry& result)const
{
   if(row < (long)theFrameEntryArray.size() && row >= 0)
   {
      if(col < (long)theFrameEntryArray[row].size() && col >= 0)
      {
         result = theFrameEntryArray[row][col];
      }
      else
      {
         return false;
      }
   }
   else
   {
      return false;
   }
   
   return true;
}

rspfString rspfRpfTocEntry::getProductType() const
{
   return theBoundaryInformation.getProductType();
}

/*!
 * If there is an entry and all the files don't exist we will return
 * true.
 */
bool rspfRpfTocEntry::isEmpty()const
{
   long rows = (long)theFrameEntryArray.size();
   long cols = 0;
   if(rows > 0)
   {
      cols = (long)theFrameEntryArray[0].size();
      for(long rowIndex = 0; rowIndex < rows; ++ rowIndex)
      {
         for(long colIndex = 0; colIndex < cols; ++colIndex)
         {
            if(theFrameEntryArray[rowIndex][colIndex].exists())
            {
               return false;
            }
         }
      }
   }

   return true;
}

rspfRefPtr<rspfImageGeometry> rspfRpfTocEntry::getImageGeometry() const
{

   rspfGpt ul(theBoundaryInformation.getCoverage().getUlLat(), 
               theBoundaryInformation.getCoverage().getUlLon());

   // Decimal degrees per pixel:
   rspfDpt ddpp;
   getDecimalDegreesPerPixel(ddpp);
   
   // Tie point - Shifted to pixel-is-point:
   rspfGpt tie( (ul.latd() - (ddpp.y/2.0)), (ul.lond() + (ddpp.x/2.0)), 0.0 );
   
   // Origin - Use the center latitude for horizontal scale, and the left edge as origin longitude
   // (OLK 10/10)
   rspfGpt origin ((ul.lat + theBoundaryInformation.getCoverage().getLlLat())/2.0, tie.lon);

#if 0 /* Please leave for debug. (drb) */
   std::cout << "boundaryInfo:\n" << boundaryInfo << std::endl;
#endif

   int z = theBoundaryInformation.getZone();
   
   if (z == 74) z--; // Fix J to a zone.
   if (z > 64) z -= 64; // Below the equator
   else z -= 48; // Above the equator

   rspfRefPtr<rspfMapProjection> mapProj = 0;
   if ( z != 9 )
   {
      mapProj = new rspfEquDistCylProjection;
   }
   else
   {
      mapProj = new rspfAzimEquDistProjection;
   }

   // Set the origin:
   mapProj->setOrigin(origin);
   
   // Set the scale:
   mapProj->setDecimalDegreesPerPixel(ddpp);

   // Set the tie:
   mapProj->setUlTiePoints(tie);

   // Give projection to the geometry object.
   rspfRefPtr<rspfImageGeometry> geom =  new rspfImageGeometry;
   geom->setProjection( mapProj.get() );

   return geom;
}

void rspfRpfTocEntry::getDecimalDegreesPerPixel(rspfDpt& scale) const
{
#if 1
   scale.x = theBoundaryInformation.getCoverage().getHorizontalInterval();
   scale.y = theBoundaryInformation.getCoverage().getVerticalInterval();
#else
   rspf_float64 ulLat = theBoundaryInformation.getCoverage().getUlLat();
   rspf_float64 ulLon = theBoundaryInformation.getCoverage().getUlLon();
   rspf_float64 urLon = theBoundaryInformation.getCoverage().getUrLon();
   rspf_float64 llLat = theBoundaryInformation.getCoverage().getLrLat();
   rspf_float64 lines = getNumberOfLines();
   rspf_float64 samps = getNumberOfSamples();
   scale.x = (urLon - ulLon) / samps;
   scale.y = (ulLat - llLat) / lines;
#endif
}

void rspfRpfTocEntry::getMetersPerPixel(rspfDpt& scale) const
{
   scale.x = theBoundaryInformation.getCoverage().getVerticalResolution();
   scale.y = theBoundaryInformation.getCoverage().getHorizontalResolution();
}

void rspfRpfTocEntry::allocateFrameEntryArray()
{
   theFrameEntryArray.resize(theBoundaryInformation.getNumberOfFramesVertical());
   rspf_uint32 horizontalSize = theBoundaryInformation.getNumberOfFramesHorizontal();

   for(rspf_uint32 index = 0; index < theFrameEntryArray.size(); index++)
   {
      theFrameEntryArray[index].resize(horizontalSize);
   }
}
