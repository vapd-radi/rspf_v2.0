//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
//*******************************************************************
//  $Id: rspfJpegWriter.cpp 21184 2012-06-29 15:13:09Z dburken $

#include <cstdlib>
#include <cstdio>

//---
// Using windows .NET compiler there is a conflict in the libjpeg with INT32
// in the file jmorecfg.h.  Defining XMD_H fixes this.
extern "C"
{
#if defined(_MSC_VER) || defined(__MINGW32__)
#  ifndef XMD_H
#    define XMD_H
#  endif
#endif
#include <jpeglib.h>
}

#include <rspf/imaging/rspfJpegWriter.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfScalarTypeLut.h>
#include <rspf/base/rspfNumericProperty.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/imaging/rspfScalarRemapper.h>

RTTI_DEF1_INST(rspfJpegWriter, "rspfJpegWriter", rspfImageFileWriter)

static const char DEFAULT_FILE_NAME[] = "output.jpg";
static const rspf_int32 DEFAULT_JPEG_QUALITY = 100;
static rspfTrace traceDebug("rspfJpegWriter:debug");

rspfJpegWriter::rspfJpegWriter()
   : rspfImageFileWriter(),
     theQuality(DEFAULT_JPEG_QUALITY),
     theOutputFilePtr(NULL)
{
   // Since there is no internal geometry set the flag to write out one.
   setWriteExternalGeometryFlag(true);
}

rspfJpegWriter::~rspfJpegWriter()
{
   close();
}

bool rspfJpegWriter::writeFile()
{
   static const char MODULE[] = "rspfJpegWriter::writeFile";
   
   if( !theInputConnection || (getErrorStatus() != rspfErrorCodes::RSPF_OK))
   {
      return false;
   }
   
   bool bandsDontMatch = false;
   // Get the number of bands.  Must be one or three for this writer.
   rspf_int32 components   = theInputConnection->getNumberOfOutputBands();
   rspf_int32 outputComponents = components;
   if (components != 1 && components != 3)
   {
      bandsDontMatch = true;
      if(components < 3)
      {
         outputComponents = 1;
      }
      else if(components > 3)
      {
         outputComponents = 3;
      }
//      rspfNotify(rspfNotifyLevel_WARN)
//         << MODULE << " Range Error:"
//         << "\nInvalid number of input bands!  Must be one or three."
//         << "\nInput bands = " << components
//         << "\nReturning from method." << endl;
      
//      return false;
   }
   
   if(theInputConnection->isMaster())
   {
      if (!isOpen())
      {
         open();
      }
      
      if (!theOutputFilePtr)
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << MODULE << " ERROR:"
            << "\nCannot open:  " << theFilename.c_str()
            << "\nReturning from method." << endl;
         
         return false;
      }
   }
   rspfRefPtr<rspfImageSource> savedInput;
   
   if(theInputConnection->getOutputScalarType() != RSPF_UINT8)
   {
      savedInput = new rspfScalarRemapper;
      
      savedInput->connectMyInputTo(0, theInputConnection->getInput(0));
      theInputConnection->connectMyInputTo(0, savedInput.get());
      theInputConnection->initialize();
   }
   
   if(theInputConnection->isMaster())
   {
      if (traceDebug())
      {
         CLOG << " DEBUG:"
              << "\nOutput Rect:  " << theAreaOfInterest << endl;
      }
      
      rspf_int32 image_height
         = theAreaOfInterest.lr().y - theAreaOfInterest.ul().y + 1;
      rspf_int32 image_width
         = theAreaOfInterest.lr().x - theAreaOfInterest.ul().x + 1;
      rspf_int32 tileWidth    = theInputConnection->getTileWidth();
      rspf_int32 tileHeight   = theInputConnection->getTileHeight();
      
      // Allocate a buffer to hold a row of tiles.
      rspf_uint32 buf_size   = tileHeight*image_width*outputComponents;
      if (traceDebug())
      {
         CLOG << "DEBUG:"
              << "\nbuf_size:  " << buf_size
              << endl;
      }
      
      std::vector<rspf_uint8> buf(buf_size);
      rspfRefPtr<rspfImageData> blankTile =
         new rspfImageData(NULL,
                            RSPF_UINT8,
                            outputComponents,
                            tileWidth,
                            tileHeight);
      rspfRefPtr<rspfImageData> tempTile;
      std::vector<rspf_uint8> tempBuf;
      if(bandsDontMatch)
      {
         tempTile = new rspfImageData(NULL,
                                       RSPF_UINT8,
                                       outputComponents,
                                       tileWidth,
                                       tileHeight);
         tempTile->initialize();
      }
      blankTile->initialize();
      //
      // Stuff needed for the jpeg library.
      // Taken from "example.c" file of the "jpeg-6b" package.
      //
      struct jpeg_compress_struct cinfo;
      struct jpeg_error_mgr jerr;
      JSAMPROW row_pointer[1];  // pointer to JSAMPLE row[s]
      
      // physical row width in image buffer
      int row_stride = image_width * outputComponents;
      
      cinfo.err = jpeg_std_error(&jerr);
      jpeg_create_compress(&cinfo);
      jpeg_stdio_dest(&cinfo, theOutputFilePtr);
      cinfo.image_width = image_width;   // image width and height, in pixels
      cinfo.image_height = image_height;
      cinfo.input_components = outputComponents; // # of color components per pixel
      
      // colorspace of input image 
      if (outputComponents == 3)
      {
         cinfo.in_color_space = JCS_RGB;
      }
      else
      {
         cinfo.in_color_space = JCS_GRAYSCALE;
      }

      jpeg_set_defaults(&cinfo);
      jpeg_set_quality(&cinfo, theQuality, TRUE); //limit to baseline-JPEG values 
      jpeg_start_compress(&cinfo, TRUE);
      
      theInputConnection->setAreaOfInterest(theAreaOfInterest);
      theInputConnection->setToStartOfSequence();
      long maxY = (long)theInputConnection->getNumberOfTilesVertical();
      long maxX = (long)theInputConnection->getNumberOfTilesHorizontal();
      
      double tileCount = 0.0;
      double totalTiles = theInputConnection->getNumberOfTiles();
      //
      // Ok the jpeg stuff should be set.  Loop through and grab a row of tiles
      // and copy to the buffer.  Then write the buffer to the jpeg file.
      // Get the next row... until finished.
      //
      for (rspf_int32 i=0;
           ((i<maxY)&&
            (!needsAborting()));
           ++i)
      {
         rspfIrect buf_rect = theAreaOfInterest;
         buf_rect.set_uly(theAreaOfInterest.ul().y+i*tileHeight);
         buf_rect.set_lry(buf_rect.ul().y + tileHeight - 1);
         
         for (rspf_int32 j=0; ((j<maxX)&& (!needsAborting())); ++j)
         {
            // Grab the tile.
            const rspfRefPtr<rspfImageData> t =
               theInputConnection->getNextTile();
            
            if ( t.valid() )
            {
               if (t->getDataObjectStatus() != RSPF_NULL)
               {
                  if(bandsDontMatch)
                  {
                     tempTile->setImageRectangle(t->getImageRectangle());
                     tempTile->setDataObjectStatus(t->getDataObjectStatus());
                     memcpy(tempTile->getBuf(), 
                            t->getBuf(),  
                            t->getSizePerBandInBytes()*outputComponents);
                     tempTile->unloadTile(&buf.front(), buf_rect, RSPF_BIP);
                  }
                  else
                  {
                     // Copy the tile to the buffer.
                     t->unloadTile(&buf.front(), buf_rect, RSPF_BIP);
                  }
               }
               else
               {
                  blankTile->setOrigin(t->getOrigin());
                  if(bandsDontMatch)
                  {
                     tempTile->setImageRectangle(blankTile->getImageRectangle());
                     tempTile->setDataObjectStatus(t->getDataObjectStatus());
                     memcpy(tempTile->getBuf(), t->getBuf(),  t->getSizePerBandInBytes()*outputComponents);
                     blankTile->unloadTile(&buf.front(), buf_rect, RSPF_BIP);
                  }
                  else
                  {
                     // Copy the tile to the buffer.
                     blankTile->unloadTile(&buf.front(), buf_rect, RSPF_BIP);
                  }
               }
            }
            ++tileCount;
            setPercentComplete((100.0*(tileCount/totalTiles)));
            
         }  // End of loop through tiles in the x direction.
         
         // Copy the buffer to the jpeg file.
         rspf_int32 lines_to_copy =
            min( (buf_rect.lr().y-buf_rect.ul().y+1),
                 (theAreaOfInterest.lr().y-buf_rect.ul().y+1) );
         
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << MODULE
               << "buf_rect:       " << buf_rect
               << "lines_to_copy:  " << lines_to_copy << endl;
         }
         
         if(!needsAborting())
         {
            // Now copy the buffer that should be full to the jpeg file.
            rspf_int32 buf_offset = 0;
            for (rspf_int32 line=0; line<lines_to_copy; ++line)
            {
               row_pointer[0] = &buf[buf_offset];
               jpeg_write_scanlines(&cinfo, row_pointer, 1);
               buf_offset += row_stride;
            }
         }
         
      } // End of loop through tiles in the y direction.
      
      // Free memory.
      if(!needsAborting())
      {
         jpeg_finish_compress(&cinfo);
      }
      
      fclose(theOutputFilePtr);
      theOutputFilePtr = NULL;
      
      jpeg_destroy_compress(&cinfo);
   }
   else
   {
      theInputConnection->slaveProcessTiles();
   }
   if(savedInput.valid())
   {
      rspfConnectableObject* obj = theInputConnection->getInput(0);
      if(obj)
      {
         theInputConnection->connectMyInputTo(0, obj->getInput(0));
      }
   }
   savedInput = 0;   
   return true;
}

bool rspfJpegWriter::saveState(rspfKeywordlist& kwl,
                                const char* prefix)const
{
   kwl.add( prefix,
            rspfKeywordNames::COMPRESSION_QUALITY_KW,
            theQuality,
            true );
   return rspfImageFileWriter::saveState(kwl, prefix);
}

bool rspfJpegWriter::loadState(const rspfKeywordlist& kwl,
                                const char* prefix)
{
   const char* value =
      kwl.find(prefix, rspfKeywordNames::COMPRESSION_QUALITY_KW);
   if(value)
   {
      setQuality(atoi(value));
   }
   
   theOutputImageType = "jpeg";
   
   return rspfImageFileWriter::loadState(kwl, prefix);
}

void rspfJpegWriter::setQuality(rspf_int32 quality)
{
   static const char MODULE[] = "rspfJpegWriter::setQuality";
   
   // Range 1 to 100 with 100 being best.
   if (quality > 0 && quality < 101)
   {
      theQuality = quality;
   }
   else
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " DEBUG:"
            << "\nquality out of range:  " << quality
            << "\nquality has been set to default:  " 
            << DEFAULT_JPEG_QUALITY
            << "\nvalid range:  1 to 100 with 100 being best."
            << endl;
      }
      
      theQuality = DEFAULT_JPEG_QUALITY;
   }

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " DEBUG:"
         << "\nQuality set to:  "
         << theQuality << endl;
   }
}

bool rspfJpegWriter::isOpen() const
{
   return (theOutputFilePtr!=NULL);
}

bool rspfJpegWriter::open()
{
   close();

   // Check for empty filenames.
   if (theFilename.empty())
   {
      return false;
   }
   
   theOutputFilePtr = fopen(theFilename.c_str(), "wb");
   if(theOutputFilePtr)
   {
      return true;
   }
   return false;
}

void rspfJpegWriter::close()
{
   if(theOutputFilePtr)
   {
      fclose(theOutputFilePtr);
      theOutputFilePtr = NULL;
   }
}

void rspfJpegWriter::getImageTypeList(std::vector<rspfString>& imageTypeList)const
{
   imageTypeList.push_back(rspfString("jpeg"));
}

rspfString rspfJpegWriter::getExtension() const
{
   return rspfString("jpg");
}

bool rspfJpegWriter::hasImageType(const rspfString& imageType) const
{
   if((imageType == "image/jpeg")||
      (imageType == "image/jpg"))
   {
      return true;
   }

   return rspfImageFileWriter::hasImageType(imageType);
}

void rspfJpegWriter::setProperty(rspfRefPtr<rspfProperty> property)
{
   if ( property.valid() )
   {
      if(property->getName() == rspfKeywordNames::COMPRESSION_QUALITY_KW)
      {
         rspfString stringValue;
         property->valueToString(stringValue);
         setQuality( stringValue.toInt32() );
      }
      else
      {
         rspfImageFileWriter::setProperty(property);
      }
   }
}

rspfRefPtr<rspfProperty> rspfJpegWriter::getProperty(const rspfString& name)const
{
   if (name == rspfKeywordNames::COMPRESSION_QUALITY_KW)
   {
      rspfNumericProperty* prop =
         new rspfNumericProperty(name,
                                  rspfString::toString(theQuality),
                                  1.0,
                                  100.0);
      prop->setNumericType(rspfNumericProperty::rspfNumericPropertyType_INT);
      return prop;
   }

   return rspfImageFileWriter::getProperty(name);
}

void rspfJpegWriter::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfString name = rspfKeywordNames::COMPRESSION_QUALITY_KW;
   propertyNames.push_back(name);
   
   rspfImageFileWriter::getPropertyNames(propertyNames);
}
