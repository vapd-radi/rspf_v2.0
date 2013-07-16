//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
// $Id: rspfImageWriterFactory.cpp 22221 2013-04-11 15:30:08Z dburken $

#include <rspf/imaging/rspfImageWriterFactory.h>
#include <rspf/base/rspfImageTypeLut.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/imaging/rspfGeneralRasterWriter.h>
#include <rspf/imaging/rspfImageWriterFactoryRegistry.h>
#include <rspf/imaging/rspfJpegWriter.h>
#include <rspf/imaging/rspfNitfWriter.h>
#include <rspf/imaging/rspfNitf20Writer.h>
#include <rspf/imaging/rspfPdfWriter.h>
#include <rspf/imaging/rspfTiffWriter.h>

rspfImageWriterFactory* rspfImageWriterFactory::theInstance = (rspfImageWriterFactory*)NULL;


rspfImageWriterFactory* rspfImageWriterFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfImageWriterFactory;
      rspfImageWriterFactoryRegistry::instance()->registerFactory(theInstance);
   }

   return theInstance;
}

rspfImageWriterFactory::~rspfImageWriterFactory()
{
   theInstance = (rspfImageWriterFactory*)NULL;
}

rspfImageFileWriter *rspfImageWriterFactory::createWriterFromExtension(
   const rspfString& fileExtension)const
{
   rspfImageFileWriter* result = 0;

   rspfString ext = fileExtension;
   ext.downcase();
   
   if( (ext == "tif")|| (ext == "tiff") )
   {
      result = new rspfTiffWriter;
   }
   else if( (ext == "jpg")|| (ext == "jpeg") )
   {
      result = new rspfJpegWriter;
   }
   else if( (ext == "ras") || (ext == "bsq") )
   {
      // Default is RSPF_GENERAL_RASTER_BSQ
      result = new rspfGeneralRasterWriter;
   }
   else if(ext == "bil")
   {
      result = new rspfGeneralRasterWriter;
      result->setOutputImageType(RSPF_GENERAL_RASTER_BIL);
   }
   else if(ext == "bip")
   {
      result = new rspfGeneralRasterWriter;
      result->setOutputImageType(RSPF_GENERAL_RASTER_BIP);
   }
   else if((ext == "ntf")||
           (ext == "nitf"))
   {
      result = new rspfNitfWriter;
   }
   else if( ext == "pdf" )
   {
      result = new rspfPdfWriter;
   }
   
   return result;
}

rspfImageFileWriter*
rspfImageWriterFactory::createWriter(const rspfKeywordlist& kwl,
                                      const char *prefix)const
{
   rspfString type = kwl.find(prefix, rspfKeywordNames::TYPE_KW);
   rspfImageFileWriter* result = (rspfImageFileWriter*)NULL;
   
   if(type != "")
   {
      result = createWriter(type);
      if (result)
      {
         if (result->hasImageType(type))
         {
            rspfKeywordlist kwl2(kwl);
            kwl2.add(prefix,
                     rspfKeywordNames::IMAGE_TYPE_KW,
                     type,
                     true);
         
            result->loadState(kwl2, prefix);
         }
         else
         {
            result->loadState(kwl, prefix);
         }
      }
   }

   return result;
}

rspfImageFileWriter*
rspfImageWriterFactory::createWriter(const rspfString& typeName)const
{
   rspfString mimeType = typeName;
   mimeType = mimeType.downcase();
	
   // Check for tiff writer.
   rspfRefPtr<rspfImageFileWriter> writer = createFromMimeType(mimeType);
	
   if(writer.valid())
   {
      return writer.release();
   }
 
   writer = new rspfTiffWriter;
   
   if (STATIC_TYPE_NAME(rspfTiffWriter) == typeName )
   {
      return writer.release();
   }
   else
   {
      // See if the type name is supported by the writer.
      if ( writer->hasImageType(typeName) )
      {
         writer->setOutputImageType(typeName);
         return writer.release();
      }
   }

   // Check for jpeg writer.
   writer = new rspfJpegWriter;
   if ( writer->getClassName() == typeName )
   {
      return writer.release();
   }
   else
   {
      // See if the type name is supported by the writer.
      if ( writer->hasImageType(typeName) )
      {
         writer->setOutputImageType(typeName);
         return writer.release();
      }
   }
	
   // Check for general raster writer.
   writer = new rspfGeneralRasterWriter;
   if ( writer->getClassName() == typeName )
   {
      return writer.release();
   }
   else
   {
      // See if the type name is supported by the writer.
      if ( writer->hasImageType(typeName) )
      {
         writer->setOutputImageType(typeName);
         return writer.release();
      }
   }

   // Check for nitf writer.
   writer = new rspfNitfWriter;
   if ( writer->getClassName() == typeName )
   {
      return writer.release();
   }
   else
   {
      // See if the type name is supported by the writer.
      if ( writer->hasImageType(typeName) )
      {
         writer->setOutputImageType(typeName);
         return writer.release();
      }
   }

   // Check for nitf writer.
   writer = new rspfNitf20Writer;
   if ( writer->getClassName() == typeName )
   {
      return writer.release();
   }
   else
   {
      // See if the type name is supported by the writer.
      if ( writer->hasImageType(typeName) )
      {
         writer->setOutputImageType(typeName);
         return writer.release();
      }
   }

   // Check for pdf writer.
   writer = new rspfPdfWriter;
   if ( writer->getClassName() == typeName )
   {
      return writer.release();
   }
   else
   {
      // See if the type name is supported by the writer.
      if ( writer->hasImageType(typeName) )
      {
         writer->setOutputImageType(typeName);
         return writer.release();
      }
   }
   
   writer = 0; // Not a nitf writer.   

   return writer.release(); // Return a null writer.
}

rspfImageFileWriter* rspfImageWriterFactory::createFromMimeType(
   const rspfString& mimeType)const
{
   if((mimeType == "image/jpeg")||
      (mimeType == "image/jpg"))
   {
      rspfJpegWriter* writer = new rspfJpegWriter;
      writer->setOutputImageType("jpeg");
      return writer;
   }
   else if((mimeType == "image/tiff")||
           (mimeType == "image/tif")||
           (mimeType == "image/gtif")||
           (mimeType == "image/gtiff"))
   {
      rspfTiffWriter* writer = new rspfTiffWriter;
      writer->setOutputImageType("tiff_tiled_band_separate");
      return writer;
   }
   else if((mimeType == "image/nitf") ||
           (mimeType == "image/ntf"))
   {
      rspfNitfWriter* writer = new rspfNitfWriter;
      writer->setOutputImageType("nitf_block_band_separate");
      return writer;
   }
   else if((mimeType == "image/ras"))
   {
      rspfGeneralRasterWriter* writer = new rspfGeneralRasterWriter;
      writer->setOutputImageType("general_raster_bsq");
      return writer;
   }
   else if((mimeType == "application/pdf"))
   {
      rspfPdfWriter* writer = new rspfPdfWriter;
      writer->setOutputImageType("rspf_pdf");
      return writer;
   }
   return 0;
}

rspfObject* rspfImageWriterFactory::createObject(const rspfKeywordlist& kwl,
                                                   const char *prefix)const
{
   return createWriter(kwl, prefix);
}

rspfObject* rspfImageWriterFactory::createObject(const rspfString& typeName)const
{
   return createWriter(typeName);
}


void rspfImageWriterFactory::getExtensions(std::vector<rspfString>& result)const
{
   result.push_back("ras");
   result.push_back("bsq");
   result.push_back("bil");
   result.push_back("bip");
   result.push_back("tif");
   result.push_back("jpg");
   result.push_back("ntf");
   result.push_back("pdf");
}

void rspfImageWriterFactory::getTypeNameList(std::vector<rspfString>& typeList)const
{
   typeList.push_back(STATIC_TYPE_NAME(rspfTiffWriter));
   typeList.push_back(STATIC_TYPE_NAME(rspfJpegWriter));
   typeList.push_back(STATIC_TYPE_NAME(rspfGeneralRasterWriter));
   typeList.push_back(STATIC_TYPE_NAME(rspfNitfWriter));
   typeList.push_back(STATIC_TYPE_NAME(rspfNitf20Writer));
   typeList.push_back(STATIC_TYPE_NAME(rspfPdfWriter));   
}

void rspfImageWriterFactory::getImageFileWritersBySuffix(
   rspfImageWriterFactoryBase::ImageFileWriterList& result, const rspfString& ext)const
{
   rspfString testExt = ext.downcase();
   if(testExt == "tiff" || testExt == "tif")
   {
      result.push_back(new rspfTiffWriter);
   }
   else if(testExt == "ntf" || testExt == "nitf")
   {
      result.push_back(new rspfNitfWriter);
   }
   else if(testExt == "jpg" || testExt == "jpeg")
   {
      result.push_back(new rspfJpegWriter);
   }
   else if(testExt == "ras" || testExt == "bsq" || testExt == "bil" || testExt == "bip")
   {
      result.push_back(new rspfGeneralRasterWriter);
   }
   else if( testExt == "pdf" )
   {
      result.push_back( new rspfPdfWriter );
   }
}

void rspfImageWriterFactory::getImageFileWritersByMimeType(
   rspfImageWriterFactoryBase::ImageFileWriterList& result, const rspfString& mimeType ) const
{
   rspfString testExt = mimeType.downcase();
   testExt = rspfString(testExt.begin()+6, testExt.end());
   
   getImageFileWritersBySuffix(result, testExt);
}

void rspfImageWriterFactory::getImageTypeList( std::vector<rspfString>& imageTypeList ) const
{
   // Add the tiff writer types.
   rspfRefPtr<rspfImageFileWriter> writer = new rspfTiffWriter;
   writer->getImageTypeList(imageTypeList);

   // Add the jpeg writer types.
   writer = new rspfJpegWriter;
   writer->getImageTypeList(imageTypeList);

   // Add the general raster writer types.
   writer = new rspfGeneralRasterWriter;
   writer->getImageTypeList(imageTypeList);

   // Add the nitf writer types.
   writer = new rspfNitfWriter;
   writer->getImageTypeList(imageTypeList);
   
   // Add the nitf writer types.
   writer = new rspfNitf20Writer;
   writer->getImageTypeList(imageTypeList);
   
   // Add the pdf writer types.
   writer = new rspfPdfWriter;
   writer->getImageTypeList(imageTypeList);
   
   writer = 0;
}
