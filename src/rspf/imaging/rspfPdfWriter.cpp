//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file
//
// Author:  David Burken
//
// Description: RSPF Portable Document Format (PDF) writer.
//
//----------------------------------------------------------------------------
// $Id$

#include <rspf/imaging/rspfPdfWriter.h>
#include <rspf/base/rspfBooleanProperty.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfScalarTypeLut.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfNumericProperty.h>
#include <rspf/imaging/rspfBandSelector.h>
#include <rspf/imaging/rspfGeneralRasterWriter.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/imaging/rspfImageWriterFactoryRegistry.h>
#include <rspf/imaging/rspfJpegMemDest.h>
#include <rspf/imaging/rspfScalarRemapper.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfUtmProjection.h>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

RTTI_DEF1(rspfPdfWriter,
	  "rspfPdfWriter",
	  rspfImageFileWriter)

//---
// For trace debugging (to enable at runtime do:
// your_app -T "rspfPdfWriter:debug" your_app_args
//---
static rspfTrace traceDebug("rspfPdfWriter:debug");
static rspfTrace traceLog("rspfPdfWriter:log");

// Property keywords:
static const std::string AUTHOR_KW        = "Author";
static const std::string CREATOR_KW       = "Creator";
static const std::string CREATION_DATE_KW = "CreationDate";
static const std::string KEYWORDS_KW      = "Keywords";
static const std::string MOD_DATE_KW      = "ModDate";
static const std::string PRODUCER_KW      = "Producer";
static const std::string SUBJECT_KW       = "Subject";
static const std::string TITLE_KW         = "Title";

//---
// For the "ident" program which will find all exanded $Id$ macros and print them.
//---
#if RSPF_ID_ENABLED
static const char RSPF_ID[] = "$Id$";
#endif

rspfPdfWriter::rspfPdfWriter()
   : rspfImageFileWriter(),
     m_str(0),
     m_ownsStream(false),
     m_savedInput(0),
     m_kwl( new rspfKeywordlist() ),
     m_mutex()
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfPdfWriter::rspfPdfWriter entered" << std::endl;
#if RSPF_ID_ENABLED
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "RSPF_ID:  "
         << RSPF_ID
         << std::endl;
#endif
   }
   
   // Since there is no internal geometry set the flag to write out one.
   setWriteExternalGeometryFlag(true);
   
   theOutputImageType = "rspf_pdf";
}

rspfPdfWriter::~rspfPdfWriter()
{
   // This will flush stream and delete it if we own it.
   close();

   m_kwl = 0; // Not a leak, rspfRefPtr
}

rspfString rspfPdfWriter::getShortName() const
{
   return rspfString("rspf_pdf_writer");
}

rspfString rspfPdfWriter::getLongName() const
{
   return rspfString("rspf pdf writer");
}

rspfString rspfPdfWriter::getClassName() const
{
   return rspfString("rspfPdfWriter");
}

bool rspfPdfWriter::writeFile()
{
   bool result = false;
   
   if( theInputConnection.valid() && ( getErrorStatus() == rspfErrorCodes::RSPF_OK ) )
   {
      //---
      // Make sure we can open the file.  Note only the master process is used for
      // writing...
      //---
      if(theInputConnection->isMaster())
      {
         if (!isOpen())
         {
            open();
         }
      }
      
      result = writeStream();
   }

   return result;
}

bool rspfPdfWriter::writeStream()
{
   static const char MODULE[] = "rspfPdfWriter::writeStream";
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " entered..."
         << "\nCurrent options:\n"
         << *(m_kwl.get()) <<  std::endl;
   }
   
   bool result = false;

   if ( isOpen() )
   {
      // Make sure we have a region of interest.
      if( theAreaOfInterest.hasNans() )
      {
         theInputConnection->initialize();
         theAreaOfInterest = theInputConnection->getAreaOfInterest();
      }
      else
      {
         theInputConnection->setAreaOfInterest( theAreaOfInterest );
      }

      // Write the image:
      writeImage( m_str );

      close();

      // Set the status:
      result = true;

      // Reset the input to the image source sequence if it was modified.      
      if ( m_savedInput.get() != theInputConnection->getInput( 0 ) )
      {
         theInputConnection->connectMyInputTo( 0, m_savedInput.get() );  
      }
   }
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " exit status = " << (result?"true":"false") << std::endl;
   }

   return result;
   
} // bool rspfPdfWriter::writeStream()

void rspfPdfWriter::writeHeader( std::ostream* str )
{
   // Header:
   if ( str )
   {
      // Open in binary mode:
      std::ostringstream os( std::ios_base::out|std::ios_base::binary );
      
      os << "%PDF-1.7\n";

      //---
      // Add comment with four characters greater than 127 to flag "binary pdf".
      // Standard/norm from spec is "0xE2E3CFD3":
      //---
      std::vector<rspf_uint8> binData(4);
      binData[0] = 0xE2;
      binData[1] = 0xE3;
      binData[2] = 0xCF;
      binData[3] = 0xD3;

      os << "%";
      os.write( (char*)&binData.front(), binData.size() );
      os << "\n";

      // Write it:
      str->write( os.str().data(), os.str().size() );

      if ( traceLog() )
      {
         // Has binary data in it???
         rspfNotify(rspfNotifyLevel_DEBUG) << os.str() << "\n";
      }
   }
}

void rspfPdfWriter::writeCatalog( std::ostream* str,
                                   std::vector<std::streamoff>& xref )
{
   // Write the Catalog object.  This assumes an order of:
   if ( str )
   {
      // Capture position:
      xref.push_back( (std::streamoff)m_str->tellp() );

      std::ostringstream os; // tmp stream...
      os << xref.size() << " 0 obj\n"
         << "  << /Type /Catalog\n"
         << "     /Pages "    << xref.size()+1 << " 0 R\n" // Reference to Pages.
         << "  >>\n"
         << "endobj\n";

      // Write it:
      str->write( os.str().data(), os.str().size() );

      if ( traceLog() )
      {
         // Has binary data in it???
         rspfNotify(rspfNotifyLevel_DEBUG) << os.str() << "\n";
      }
   }
}

#if 0
void rspfPdfWriter::writeOutlines( std::ostream* str,
                                    std::vector<std::streamoff>& xref )
{
   // Write the Outlines object:
   if ( str )
   {
      // Capture position:
      xref.push_back( (std::streamoff)m_str->tellp() );

      std::ostringstream os; // tmp stream...
      os << xref.size() << " 0 obj\n"
         << "  << /Type /Outlines\n"
         << "     /Count 0\n"
         << "  >>\n"
         << "endobj\n";

      str->write( os.str().data(), os.str().size() );

      if ( traceLog() )
      {
         // Has binary data in it???
         rspfNotify(rspfNotifyLevel_DEBUG) << os.str() << "\n";
      }
   }
}
#endif

void rspfPdfWriter::writePages( std::ostream* str,
                                 std::vector<std::streamoff>& xref )
{
   // Write Pages object.
   if ( str )
   {
      // Capture position:
      xref.push_back( (std::streamoff)str->tellp() );

      std::ostringstream os;
      os << xref.size() << " 0 obj\n"
         << "  << /Type /Pages\n"
         << "     /Kids [ " << xref.size()+1 << " 0 R ]\n" // Reference to Page.
         << "     /Count 1\n"
         << "  >>\n"
         << "endobj\n";

      str->write( os.str().data(), os.str().size() );
      
      if ( traceLog() )
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << os.str() << "\n";
      }
   }
}

void rspfPdfWriter::writeInfo( std::ostream* str,
                                std::vector<std::streamoff>& xref )
{
   // Write Pages object.
   if ( str )
   {
      // Capture position:
      xref.push_back( (std::streamoff)str->tellp() );

      std::ostringstream os;
      os << xref.size() << " 0 obj % Document Information Dictionary\n";

      // Tile:
      std::string s;
      getTitle( s );
      if ( s.size() )
      {
         os << "   << /Title (" << s << ")\n";
      }
      
      // Author:
      getAuthor( s );
      if ( s.size() )
      {
         os << "      /Author (" << s << ")\n";
      }

      // Creator:
      getCreator( s );
      if ( s.size() )
      {
         os << "      /Creator (" << s << ")\n";
      }

      // Producer:
      getProducer( s );
      if ( s.size() )
      {
         os << "      /Producer (" << s << ")\n";
      }

      // Creation date ( D : YYYYMMDDHHmmSSOHH ' mm ' ):
      getCreationDate( s );
      if ( s.size() )
      {
         os << "      /CreationDate " << s << "\n";
      }

      // Modification date ( D : YYYYMMDDHHmmSSOHH ' mm ' ):
      getModDate( s );
      if ( s.size() )
      {
         os << "      /ModDate " << s << "\n";
      }

      os << "   >>\n"
            << "endobj\n";

      str->write( os.str().data(), os.str().size() );
      
      if ( traceLog() )
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << os.str() << "\n";
      }
   }
   
} // End: rspfPdfWriter::writeInfo

bool rspfPdfWriter::writeImage( std::ostream* str )
{
   bool status = false;
   
   if ( str )
   {
      //---
      // This will remap input to eight bit if not already and make input one or
      // three band depending on number of bands.
      //---
      setupInputChain();
         
      // Set the sequence tile size:
      rspfIpt tileSize;
      getTileSize( tileSize );
      theInputConnection->setTileSize( tileSize );
      
      // Start the sequence at the first tile.
      theInputConnection->setToStartOfSequence();
      
      const rspf_uint32 WIDTH  = theAreaOfInterest.width();
      const rspf_uint32 HEIGHT = theAreaOfInterest.height();
      const rspf_uint32 TW     = theInputConnection->getNumberOfTilesHorizontal();    
      const rspf_uint32 TH     = theInputConnection->getNumberOfTilesVertical();
      const rspf_uint32 TILES  = theInputConnection->getNumberOfTiles();

#if 0 /* Please save for debug. (drb) */
      rspfIrect   boundingRect  = theInputConnection->getBoundingRect();
      rspfNotify(rspfNotifyLevel_NOTICE)
         << "\nBounding rect: " << theInputConnection->getBoundingRect()
         << "\nAOI:           " << theAreaOfInterest  
         << "\nBands:         " << theInputConnection->getNumberOfOutputBands()
         << "\ntilesWide:     " << TW
         << "\ntilesHigh:     " << TH
         << "\ntileWidth:     " << tileWidth
         << "\ntileHeight     " << tileHeight << std::endl;
#endif

      if ( traceLog() )
      {
         rspfKeywordlist logKwl;
         theInputConnection->saveStateOfAllInputs(logKwl);      
         rspfFilename logFile = getFilename();
         logFile.setExtension("log");
         rspfKeywordlist kwl;
         theInputConnection->saveStateOfAllInputs(kwl);
         kwl.write(logFile.c_str() );
      }
      
      // Cross reference(xref) table.  Holds the offset of each object.  Written at the end.
      std::vector<std::streamoff> xref;
         
      //---
      // Header:
      //---
      writeHeader( str );
         
      //---
      // Catalog object:
      //---
      writeCatalog( str, xref );
         
      //---
      // Outlines object:
      //---
      // writeOutlines( m_str, xref );
         
      //---
      // Pages object:
      //---
      writePages( str, xref );
      
      //---
      // Page object:
      //---
      
      // Capture position:
      xref.push_back( (std::streamoff)str->tellp() );
      
      std::ostringstream os1;
      os1 << xref.size() << " 0 obj\n"
          << "  << /Type /Page\n"
          << "     /Parent "    << xref.size()-1 << " 0 R\n" // Parent always one back.
          << "     /Resources " << xref.size()+1 << " 0 R\n" // Next object.
          << "     /MediaBox[0.0 0.0 " << WIDTH << " " << HEIGHT << "]\n"
          // Contents = Resources_dictionary + tiles + base_form + 1):
          << "     /Contents["  << (xref.size()+TILES+3) << " 0 R]\n";
      
      rspfRefPtr<rspfImageGeometry> geom = theInputConnection->getImageGeometry();      
      if ( geom.valid() )
      {
         // Coordinate Transformation Matrix (CTM)
         std::string lgiDictString;
         getLgiDict( geom.get(), theAreaOfInterest, lgiDictString );
         
         if ( lgiDictString.size() )
         {
            os1 << lgiDictString;
         }
      }
      
      os1 << "  >>\n"
         << "endobj\n"; // Matches: <</Type /Page"
      
      str->write( os1.str().data(), os1.str().size() );
      if ( traceLog() )
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << os1.str() << "\n";
      }
      
      // End /Page object
      
      //---
      // Resource dictionary for page:
      //---
      
      // Capture position:
      xref.push_back( (std::streamoff)str->tellp() );

      std::ostringstream os2;
      os2 << xref.size() << " 0 obj % Resource dictionary for page.\n"
          << "  << /ProcSet[/PDF/ImageB/ImageC]\n"
          << "     /XObject << /Basemap_Form " << (xref.size()+TILES+1) << " 0 R ";

      // Write in the tiles:
      rspf_uint32 objIndex = static_cast<rspf_uint32>(xref.size()+1);
      rspf_uint32 x;
      rspf_uint32 y;
      for ( x = 0; x < TW; ++x )
      {
         for ( y = 0; y < TH; ++y )
         {
            os2 << getTileString( x, y ) << " " << objIndex << " 0 R ";

            ++objIndex;
         }
 
      }
      os2 << ">>\n" // Matches: /XObject<<
          << "  >>\nendobj\n"; // Matches: % Resources...";

      str->write( os2.str().data(), os2.str().size() );
      if ( traceLog() )
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << os2.str() << "\n";
      }
      
      // End Resources dictionary.

      //---
      // Now write the XObjects jpeg tiles:
      //---
      rspf_uint32 tileNumber = 0;
      std::vector<rspfIrect> outputTileRects;
      while ( tileNumber < TILES )
      {
         rspfRefPtr<rspfImageData> id = theInputConnection->getNextTile();
         if( id.valid() )
         {
            // To hold output tile rect clamped to AOI.
            rspfIrect outputTileRect;

            switch ( getImageType() )
            {
               case rspfPdfWriter::RAW:
               {
                  writeRawTile( str, xref, id.get(), theAreaOfInterest, outputTileRect );
                  break;
               }
               case rspfPdfWriter::JPEG:
               default:
               {
                  writeJpegTile( str, xref, id.get(), theAreaOfInterest, outputTileRect );
                  break;
               }
            }

            // Capture tile rect:
            outputTileRects.push_back( outputTileRect );
         }
         else
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfPdfWriter::writeImage ERROR:"
               << "\nNULL Tile encountered, tile number=" << tileNumber
               << std::endl;
            status = false;
            break;
         }

         ++tileNumber;

         setPercentComplete( tileNumber / TILES * 100.0 );
      }

      if ( tileNumber == TILES )
      {
         status = true;
      }

      if ( status )
      {
         //---
         // XObect Form for image tiles:
         //---
         
         // Capture position:
         xref.push_back( (std::streamoff)str->tellp() );

         rspfIpt llPageOffset;
         getLowerLeftPageOffset( theAreaOfInterest, llPageOffset );

         std::ostringstream os3;
         
         // Tile loop - ontents for each ile:
         tileNumber = 0;
         rspf_uint32 x;
         rspf_uint32 y;
         for ( x = 0; x < TW; ++x )
         {
            for ( y = 0; y < TH; ++y )
            {
               //---
               // PDF space is positive y = up, positive x = right.
               // rspf image space is positive y down.
               // 0, 0 is lower left corner in pdf space so "y of 0" in pdf space is
               // theAreaOfInterest.ll().y in image space.
               //---
               rspf_int32 width = outputTileRects[tileNumber].width();

               rspf_int32 height = outputTileRects[tileNumber].height();
               
               rspf_int32 llOffsetX =
                  llPageOffset.x + outputTileRects[tileNumber].ll().x - theAreaOfInterest.ll().x;

               rspf_int32 llOffsetY = llPageOffset.y + theAreaOfInterest.lr().y -
                  outputTileRects[tileNumber].ll().y;
               
               os3 << "   q\n"                      // Save graphics state
                   << "      " << width << " 0 0 " << height << " "
                   << llOffsetX << " " << llOffsetY << " cm\n"
                   << "      " << getTileString( x, y ) << " Do\n"
                   << "   Q\n"; // Restore graphics state.
               ++tileNumber;
            }
         }

         std::ostringstream os4;
         os4 << xref.size() << " 0 obj % /XObject/Form dictionary for image tiles.\n"
             << "   << /Type /XObject\n"
             << "      /Subtype /Form\n"
             << "      /Length " << os3.str().size() << "\n"
             << "      /BBox["
             << llPageOffset.x << " " << llPageOffset.y << " "
             << WIDTH << " " << HEIGHT << "]\n"
             << "   >>\n"
             << "stream\n";

         // Write the content stream to tmp stream.
         os4.write( os3.str().data(), os3.str().size() );
         
         os4 << "\n"
             << "endstream\n"
             << "endobj\n";

         // Write the tmp stream to output stream.
         str->write( os4.str().data(), os4.str().size() );
         
         if ( traceLog() )
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << os4.str() << "\n";
         }
         
         //---
         // Contents of page:
         //---
         
         // Capture position:
         xref.push_back( (std::streamoff)str->tellp() );

         std::ostringstream os5;
         os5 << "/Basemap_Form Do";
         
         std::ostringstream os6;
         os6 << xref.size() << " 0 obj % Contents of page.\n"
             << "   << /Length " << os5.str().size() << " >>\n"
             << "stream\n";

         os6.write( os5.str().data(), os5.str().size() );
         os6 << "\n"
             << "endstream\n"
             << "endobj\n";
         
         str->write( os6.str().data(), os6.str().size() );
         if ( traceLog() )
         {
            rspfNotify(rspfNotifyLevel_DEBUG) << os6.str() << "\n";
         }
         
         m_mutex.lock(); // Code not thread safe.
         
         // Document Information dictionary:
         writeInfo( str, xref );
         
         m_mutex.unlock();
         
         //---
         // Cross reference tables:
         //---
         
         // Capture start of xref for the second to last line:
         std::streamoff xrefOffset = (std::streamoff)m_str->tellp();
         
         writeXref( m_str, xref );
         
         //---
         // Trailer:
         //---
         writeTrailer( m_str, static_cast<rspf_uint32>(xref.size()+1), xrefOffset );
      }
      
   } // Matches: if ( str ...
   
   return status;

} // End: rspfPdfWriter::writeImage

void rspfPdfWriter::writeJpegTile( std::ostream* str,
                                    std::vector<std::streamoff>& xref,
                                    rspfImageData* id,
                                    const rspfIrect& aoi,
                                    rspfIrect& outputTileRect )
{
   //---
   // Most of code and comments below are from jpeg-6b "example.c" file. See 
   // http://www4.cs.fau.de/Services/Doc/graphics/doc/jpeg/libjpeg.html
   //---

   //---
   // Initialize JPEG compression library:
   // Note: JDIMENSION is typedefed to an unsigned int.
   //---
   struct jpeg_compress_struct cinfo;
   struct jpeg_error_mgr jerr;
   cinfo.err = jpeg_std_error( &jerr );
   jpeg_create_compress(&cinfo);

   //---
   // Need to define a custom ostream destination manager for jpeglib to write compressed block:
   // Note: Write the tile to a temp memory stream to get the size up front.
   //---
   std::ostringstream tileStream;
   jpeg_cpp_stream_dest(&cinfo, tileStream);
   
   
   // Setting the parameters of the output file here.
   const rspf_uint32 BANDS = id->getNumberOfBands();
   const rspf_uint32 TW    = id->getWidth();
   const rspf_uint32 TH    = id->getHeight();
   
   // Assign width, height clamping to the aoi.      
   cinfo.image_width = rspf::min<rspf_uint32>(
      TW, aoi.lr().x - id->getOrigin().x + 1 );
   cinfo.image_height = rspf::min<rspf_uint32>(
      TH, aoi.lr().y - id->getOrigin().y + 1 );
   cinfo.input_components = BANDS;

   // Capture the output tile rect:
   outputTileRect = rspfIrect( id->getOrigin(),
                                rspfIpt( id->getOrigin().x + cinfo.image_width - 1,
                                          id->getOrigin().y + cinfo.image_height - 1 ) );
   
   // colorspace of input image 
   if ( BANDS == 3 )
   {
      cinfo.in_color_space = JCS_RGB;
   }
   else
   {
      cinfo.in_color_space = JCS_GRAYSCALE;
   }
   
   // Default compression parameters...
   jpeg_set_defaults( &cinfo );
   
   /* Now do the compression .. */
   jpeg_start_compress( &cinfo, TRUE );
   
   // Compress the tile on line at a time:
   JSAMPLE** samp_array = new JSAMPLE* [cinfo.image_height];
   
   //---
   // Get a pointer to the source data.
   // Needs to be band interleaved by pixel(BIP), so convert if needed.
   //---
   rspfRefPtr<rspfImageData> tempTile = 0;
   rspf_uint8* tile = 0;
   if ( ( BANDS == 1 ) &&
        ( cinfo.image_width == id->getWidth() ) &&
        ( cinfo.image_height == id->getHeight() ) )
   {
      tile = id->getUcharBuf();
   }
   else
   {
      // Convert from RSPF's native BSQ interleave to BIP.
      tempTile = new rspfImageData(
         0, RSPF_UINT8, BANDS, cinfo.image_width, cinfo.image_height );
      tempTile->setOrigin( id->getOrigin() );
      tempTile->initialize();
      tile = tempTile->getUcharBuf();
      id->unloadTile( tile, tempTile->getImageRectangle(), RSPF_BIP );
   }
      
   for ( rspf_uint32 row=0; row<cinfo.image_height; ++row )
   {
      samp_array[row] = (JSAMPLE*) tile + row*cinfo.input_components*cinfo.image_width;
   }
   
   jpeg_write_scanlines( &cinfo, samp_array, cinfo.image_height );

   //---
   // Clean up after done compressing:
   // Note: MUST do before writing to output stream below.
   //---
   jpeg_finish_compress( &cinfo );
   jpeg_destroy_compress( &cinfo );
   delete [] samp_array;
   samp_array = 0;

   // Capture position:
   xref.push_back( (std::streamoff)str->tellp() );
   
   std::ostringstream os1;
   os1 << xref.size() << " 0 obj\n"
       << "   << /Type /XObject\n"
       << "      /Subtype /Image\n"
       << "      /Length " << tileStream.str().size() << "\n"
       << "      /Filter[/DCTDecode]\n"
       << "      /BitsPerComponent 8"
       << "      /ColorSpace " << (BANDS==1?"/DeviceGray\n":"/DeviceRGB\n")
       << "      /Width " << cinfo.image_width
       << "      /Height " << cinfo.image_height
       << "   >>\n"
       << "stream\n";
   
   str->write( os1.str().data(), os1.str().size() );
   
   if ( traceLog() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << os1.str()
         << "*** image stream not displayed ***\n"
         << "endstream\n"
         << "endobj\n\n";
   }

   // Write the image tile from memory to output stream.
   str->write( tileStream.str().data(), tileStream.str().size() );

   // Close out stream object.
   *str << "\n"
        << "endstream\n"
        << "endobj\n";
   
} // End: rspfPdfWriter::writeJpegTile( ... )

void rspfPdfWriter::writeRawTile( std::ostream* str,
                                   std::vector<std::streamoff>& xref,
                                   rspfImageData* id,
                                   const rspfIrect& aoi,
                                   rspfIrect& outputTileRect )
{
   const rspf_uint32 BANDS = id->getNumberOfBands();
   const rspf_uint32 TW    = id->getWidth();
   const rspf_uint32 TH    = id->getHeight();

   // Assign width, height clamping to the aoi.      
   rspf_uint32 width = rspf::min<rspf_uint32>(
      TW, aoi.lr().x - id->getOrigin().x + 1  );
   rspf_uint32 height = rspf::min<rspf_uint32>(
      TH, aoi.lr().y - id->getOrigin().y + 1 );

   // Capture the output tile rect:
   outputTileRect = rspfIrect( id->getOrigin(),
                                rspfIpt( id->getOrigin().x + width - 1,
                                          id->getOrigin().y + height - 1 ) );

   //---
   // Get a pointer to the source data.
   // Needs to be band interleaved by pixel(BIP), so convert if needed.
   //---
   rspfRefPtr<rspfImageData> tempTile = 0;
   rspf_uint8* tile = 0;
   rspf_uint32 length = 0;
   if ( ( BANDS == 1 ) && ( width == id->getWidth() ) && ( height == id->getHeight() ) )
   {
      tile   = id->getUcharBuf();
      length = id->getSizeInBytes();
   }
   else
   {
      // Convert from RSPF's native BSQ interleave to BIP.
      tempTile = new rspfImageData( 0, RSPF_UINT8, BANDS, width, height );
      tempTile->setOrigin( id->getOrigin() );
      tempTile->initialize();
      tile = tempTile->getUcharBuf();
      id->unloadTile( tile, tempTile->getImageRectangle(), RSPF_BIP );
      length = tempTile->getSizeInBytes();
   }

   // Capture position:
   xref.push_back( (std::streamoff)str->tellp() );
   
   std::ostringstream os1;
   os1 << xref.size() << " 0 obj\n"
       << "   << /Type /XObject\n"
       << "      /Subtype /Image\n"
       << "      /Length " << length << "\n"
       << "      /BitsPerComponent 8"
       << "      /ColorSpace " << (BANDS==1?"/DeviceGray\n":"/DeviceRGB\n")
       << "      /Width " << width
       << "      /Height " << height
       << "   >>\n"
       << "stream\n";

   str->write( os1.str().data(), os1.str().size() );
   
   if ( traceLog() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << os1.str()
         << "*** image stream not displayed ***\n"
         << "endstream\n"
         << "endobj\n\n";
   }

   // Write the image tile from memory to output stream.
   str->write( (char*)tile, length );
   
   // Close out stream object.
   *str << "\n"
        << "endstream\n"
        << "endobj\n";
   
} // End rspfPdfWriter::writeRawTile( ... )

void rspfPdfWriter::writeXref( std::ostream* str,
                                const std::vector<std::streamoff>& xref )
{
   if ( str )
   {
      std::ostringstream os;
      os << "xref\n"
         << "0 " << (xref.size()+1) << "\n"
         << "0000000000 65535 f \n";
            
      std::vector<std::streamoff>::const_iterator i = xref.begin();
      while ( i != xref.end() )
      {
         os << std::setiosflags(std::ios_base::fixed|std::ios_base::right)
            << std::setfill('0')
            << std::setw(10)
            << (*i)
            << " 00000 n \n";
         ++i;
      }
      
      // os << "\n";
      
      str->write( os.str().data(), os.str().size() );
      
      if ( traceLog() )
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << os.str() << "\n";
      }
   }
}

void rspfPdfWriter::writeTrailer( std::ostream* str,
                                   rspf_uint32 entrySize,
                                   std::streamoff xrefOffset )
{
   if ( str )
   {
      std::ostringstream os;
      os << "trailer\n"
         << "   << /Size " << entrySize << "\n"
         << "      /Root 1 0 R\n"
         << "      /Info " << entrySize-1 << " 0 R\n"
         << "   >>\n"
         << "startxref\n"
         << xrefOffset << "\n"
         << "%%EOF\n";
      // << "%%EOF";

      str->write( os.str().data(), os.str().size() );

      if ( traceLog() )
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << os.str() << "\n";
      }
   }
}

#if 0
void rspfPdfWriter::writeDummy( std::ostream* str,
                                 std::vector<std::streamoff>& xref  )
{
   // Write a blank object.
   if ( str )
   {
      // Capture position:
      xref.push_back( (std::streamoff)str->tellp() );
      
      std::ostringstream os;
      os << xref.size() << " 0 obj % dummy\n"
         << "endobj\n";
      
      str->write( os.str().data(), os.str().size() );
      
      if ( traceLog() )
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << os.str() << "\n";
      }
   }
}
#endif

bool rspfPdfWriter::saveState(rspfKeywordlist& kwl,
                               const char* prefix)const
{
   return rspfImageFileWriter::saveState(kwl, prefix);
}

bool rspfPdfWriter::loadState(const rspfKeywordlist& kwl,
                               const char* prefix)
{
   return rspfImageFileWriter::loadState(kwl, prefix);
}

bool rspfPdfWriter::isOpen() const
{
   bool result = false;
   if (m_str)
   {
      const std::ofstream* fs = dynamic_cast<const std::ofstream*>(m_str);
      if ( fs )
      {
         result = fs->is_open();
      }
      else
      {
         // Pointer good enough...
         result = true;
      }
   }
   return result;
}


bool rspfPdfWriter::open()
{
   bool result = false;
   
   close();

   // Check for empty filenames.
   if (theFilename.size())
   {
      std::ofstream* os = new std::ofstream();
      os->open(theFilename.c_str(), ios::out | ios::binary);
      if(os->is_open())
      {
         m_str = os;
         m_ownsStream = true;
         result = true;
      }
      else
      {
         delete os;
         os = 0;
      }
   }
   return result;
}

void rspfPdfWriter::close()
{
   if (m_str)      
   {
      m_str->flush();

      if (m_ownsStream)
      {
         delete m_str;
         m_str = 0;
         m_ownsStream = false;
      }
   }
}

void rspfPdfWriter::getImageTypeList(std::vector<rspfString>& imageTypeList)const
{
   imageTypeList.push_back(rspfString("rspf_pdf"));
}

rspfString rspfPdfWriter::getExtension() const
{
   return rspfString("pdf");
}

bool rspfPdfWriter::hasImageType(const rspfString& imageType) const
{
   bool result = false;
   if ( (imageType == "rspf_pdf") || (imageType == "application/pdf") )
   {
      result = true;
   }
   return result;
}

void rspfPdfWriter::setProperty( rspfRefPtr<rspfProperty> property )
{
   if ( property.valid() )
   {
      std::string name = property->getName().string();
      if ( name.size() )
      {
         if ( ( name == rspfKeywordNames::IMAGE_TYPE_KW ) ||
              ( name == AUTHOR_KW ) ||
              ( name == CREATOR_KW ) ||
              ( name == CREATION_DATE_KW ) ||
              ( name == KEYWORDS_KW ) ||
              ( name == MOD_DATE_KW ) ||
              ( name == rspfKeywordNames::OUTPUT_TILE_SIZE_KW ) ||
              ( name == PRODUCER_KW ) ||
              ( name == SUBJECT_KW ) ||
              ( name == TITLE_KW ) )
         {
            addOption( name, property->valueToString().string() );
         }
         else
         {
            rspfImageFileWriter::setProperty(property);
         }
      }
   }
   
} // rspfPdfWriter::setProperty

rspfRefPtr<rspfProperty> rspfPdfWriter::getProperty(const rspfString& name)const
{
   rspfRefPtr<rspfProperty> prop = 0;

   if ( name.size() )
   {
      if ( name == AUTHOR_KW )
      {
         rspfString value;
         getAuthor( value.string() );
         rspfRefPtr<rspfStringProperty> stringProp =
            new rspfStringProperty( name, value, true);
         prop = stringProp.get();
      }
      else if ( name == CREATOR_KW )
      {
         rspfString value;
         getCreator( value.string() );
         rspfRefPtr<rspfStringProperty> stringProp =
            new rspfStringProperty( name, value, true);
         prop = stringProp.get();
      }
      else if ( name == CREATION_DATE_KW )
      {
         rspfString value;
         getCreationDate( value.string() );
         rspfRefPtr<rspfStringProperty> stringProp =
            new rspfStringProperty( name, value, true);
         prop = stringProp.get();
      }
      else if ( name == rspfKeywordNames::IMAGE_TYPE_KW )
      {
         rspfString value;
         getImageType( value.string() );
         rspfRefPtr<rspfStringProperty> stringProp =
            new rspfStringProperty( name, value, false); //  editable flag = false
         
         // Alway support jpeg.
         stringProp->addConstraint( rspfString("jpeg") );
         
         // Alway support raw (general raster).
         stringProp->addConstraint( rspfString("raw") );

         prop = stringProp.get();
      }
      else if ( name == KEYWORDS_KW )
      {
         rspfString value;
         getKeywords( value.string() );
         rspfRefPtr<rspfStringProperty> stringProp =
            new rspfStringProperty( name, value, true);
         prop = stringProp.get();
      }
      else if ( name == MOD_DATE_KW )
      {
         rspfString value;
         getModDate( value.string() );
         rspfRefPtr<rspfStringProperty> stringProp =
            new rspfStringProperty( name, value, true);
         prop = stringProp.get();
      }
      else if( name == rspfKeywordNames::OUTPUT_TILE_SIZE_KW )
      {
         rspfString value;
         getTileSize( value.string() );
         rspfRefPtr<rspfStringProperty> stringProp =
            new rspfStringProperty( name, value, false);
         stringProp->addConstraint(rspfString("16"));
         stringProp->addConstraint(rspfString("32"));
         stringProp->addConstraint(rspfString("64"));
         stringProp->addConstraint(rspfString("128"));
         stringProp->addConstraint(rspfString("256"));      
         stringProp->addConstraint(rspfString("512"));      
         stringProp->addConstraint(rspfString("1024"));      
         stringProp->addConstraint(rspfString("2048"));      
         prop = stringProp.get();
      }
      else if ( name == SUBJECT_KW )
      {
         rspfString value;
         getSubject( value.string() );
         rspfRefPtr<rspfStringProperty> stringProp =
            new rspfStringProperty( name, value, true);
         prop = stringProp.get();
      }
      else if ( name == TITLE_KW )
      {
         rspfString value;
         getSubject( value.string() );
         rspfRefPtr<rspfStringProperty> stringProp =
            new rspfStringProperty( name, value, true);
         prop = stringProp.get();
      }
      else
      {
         prop = rspfImageFileWriter::getProperty(name);
      }
   }
   
   return prop;
   
} // End: rspfPdfWriter::getProperty

void rspfPdfWriter::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   propertyNames.push_back( rspfString( AUTHOR_KW ) );
   propertyNames.push_back( rspfString( CREATOR_KW ) );
   propertyNames.push_back( rspfString( CREATION_DATE_KW ) );
   propertyNames.push_back( rspfString( rspfKeywordNames::IMAGE_TYPE_KW ) );
   propertyNames.push_back( rspfString( KEYWORDS_KW ) );
   propertyNames.push_back( rspfString( MOD_DATE_KW ) );
   propertyNames.push_back( rspfString( rspfKeywordNames::OUTPUT_TILE_SIZE_KW ) );
   propertyNames.push_back( rspfString( PRODUCER_KW ) );
   propertyNames.push_back( rspfString( SUBJECT_KW ) );
   propertyNames.push_back( rspfString( TITLE_KW ) );

   rspfImageFileWriter::getPropertyNames(propertyNames);
}

bool rspfPdfWriter::setOutputStream(std::ostream& stream)
{
   if (m_ownsStream && m_str)
   {
      delete m_str;
   }
   m_str = &stream;
   m_ownsStream = false;
   return true;
}

void rspfPdfWriter::setupInputChain()
{
   if ( theInputConnection.get() )
   {
      // Capture the input to the image source sequencer(theInputConnection).
      m_savedInput = theInputConnection->getInput( 0 );
      
      rspfScalarType inputScalar = theInputConnection->getOutputScalarType();
      rspf_uint32 bands          = theInputConnection->getNumberOfOutputBands();

      if( inputScalar != RSPF_UINT8 )
      {
         // Make eight bit.
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfPdfWriter::setupInputChain WARNING:"
               << "\nData is being scaled to 8 bit!"
               << "\nOriginal scalar type:  "
               << rspfScalarTypeLut::instance()->
               getEntryString(inputScalar).c_str()
               << std::endl;
         }
         
         //---
         // Attach a scalar remapper to the end of the input chain.  This will
         // need to be unattached and deleted at the end of this.
         //---
         rspfRefPtr<rspfScalarRemapper> sr = new rspfScalarRemapper;

         // Connect remapper's input to sequencer input.
         sr->connectMyInputTo( 0, theInputConnection->getInput(0) );

         // Connet sequencer to remapper.
         theInputConnection->connectMyInputTo(0, sr.get() );

         // Initialize connections.
         theInputConnection->initialize();
      }

      // Must be one or three band. Note bands are zero based...      
      if ( ( bands != 1 ) && ( bands != 3 ) )
      {
         std::vector<rspf_uint32> bandList;
         
         // Always have one band.
         bandList.push_back( 0 );

         if ( bands > 3 )
         {
            // Use the first three bands.
            bandList.push_back( 1 );
            bandList.push_back( 2 );
         }

         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfPdfWriter::setupInputChain WARNING:"
               << "\nOutput bands must be 1 or 3 and are being filterd!"
               << "\nOriginal band count: " << bands
               << "\nNew band count: " << bandList.size()
               << std::endl;
         }
         
         rspfRefPtr<rspfBandSelector> bs = new rspfBandSelector();

         // Set the the band selector list.
         bs->setOutputBandList( bandList );
         
         // Connect band selector's input to sequencer input.
         bs->connectMyInputTo( 0, theInputConnection->getInput(0) );

         // Connet sequencer to band selector.
         theInputConnection->connectMyInputTo(0, bs.get() );

         // Initialize connections.
         theInputConnection->initialize();
      }

      if (  m_savedInput != theInputConnection->getInput( 0 ) )
      {
         //---
         // If the input to the image source sequencer change the AOI
         // MUST be reset as it gets wiped.
         //---
         theInputConnection->setAreaOfInterest( theAreaOfInterest );
      }
      
   } // Matches: if ( theInputConnection )

} // End: void rspfPdfWriter::setupInputChain()

rspfPdfWriter::rspfPdfImageType rspfPdfWriter::getImageType() const
{
   rspfPdfWriter::rspfPdfImageType result = rspfPdfWriter::UNKNOWN;

   // Get the type and downcase.
   rspfString os;
   getImageType( os.string() );
   os.downcase();

   if ( os == "jpeg" )
   {
      result = rspfPdfWriter::JPEG;
   }
   else if ( os == "raw" )
   {
      result = rspfPdfWriter::RAW;
   }
   else
   {
      // Unknown value:
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfPdfWriter::getImageCompression WARN\n"
         << "Unhandled image compression type: " << os << std::endl;
   }
   return result;
}

void rspfPdfWriter::getImageType( std::string& type ) const
{
   type = m_kwl->findKey( std::string(rspfKeywordNames::IMAGE_TYPE_KW) );
   if ( type.empty() )
   {
      type = "jpeg"; // default
   }
}

void rspfPdfWriter::addOption(  const std::string& key, const std::string& value )
{
   m_mutex.lock();
   if ( m_kwl.valid() )
   {
      if ( key.size() && value.size() )
      {
         m_kwl->addPair( key, value );
      }
   }
   m_mutex.unlock();
}

#if 0
void rspfPdfWriter::getMediaBoxSize(  rspf_uint32 imageWidth,
                                       rspf_uint32 imageHeight,
                                       rspfIpt& size ) const
{
   //---
   // This could be an option / property.
   // 612 x 792 = 8.5 x 11 inches at 72 dots per inch (612/72 x 792/72)
   //---
   size.x = static_cast<rspf_uint32>(imageWidth);
   size.y = static_cast<rspf_uint32>(imageHeight);
}
#endif

void rspfPdfWriter::getLowerLeftPageOffset( const rspfIrect& aoi, rspfIpt& offset ) const
{
   if ( aoi.hasNans() == false )
   {
      // Get the page size.
      // rspfIpt mediaBoxSize;
      // getMediaBoxSize( mediaBoxSize );

      // Temp hard coded. (drb)
      offset.x = 0;
      offset.y = 0;
   }
   else
   {
      offset.x = 0;
      offset.y = 0;
   }
}

void rspfPdfWriter::getLgiDict( rspfImageGeometry* geom,
                                 const rspfIrect& aoi,
                                 std::string& s ) const
{
   s.clear(); // Empty string indicates error.

   if ( geom && !aoi.hasNans() )
   {
      // Coordinate Transformation Matrix (CTM)
      std::string ctmString;
      getLgiDictCtm( geom, aoi, ctmString );

      if ( ctmString.size() )
      {
         // ProjectionType
         std::string projectionString;
         getLgiDictProjection( geom, projectionString );
         
         if ( projectionString.size() )
         {
            //---
            // Neatline: Not required for single frame.
            // std::string neatLineString;
            // getLgiDictNeatline( geom, neatLineString );
            //---
            
            // if ( neatLineString.size() )
            // {
            // cout << "d..." << endl;
               
            // Description
            std::string descriptionString;
            getLgiDictDescription( geom, descriptionString );
            
            if ( descriptionString.size() )
            {
               std::ostringstream os;
               os << "/LGIDict"
                  << "[<<"
                  << ctmString
                  << "/Version(2.1)"
                  << projectionString
                  // << neatLineString
                  << descriptionString
                  << "/Type/LGIDict>>]";
               
               s = os.str();
               // }
            }
         }
      }
   }
   
} // End: rspfPdfWriter::getLgiDictString

void rspfPdfWriter::getLgiDictDatum( rspfImageGeometry* geom,
                                      std::string& s ) const
{
   s.clear();
   if ( geom )
   {
      rspfRefPtr<const rspfMapProjection> mapProj = geom->getAsMapProjection();
      if ( mapProj.valid() )
      {
         const rspfDatum* datum = mapProj->getDatum();
         if ( datum )
         {
            s = "/Datum(";
            s += datum->code().string();
            s += ")";
         }
      }
   }
   
} // End: rspfPdfWriter::getLgiDictDatum

void rspfPdfWriter::getLgiDictDescription( rspfImageGeometry* geom,
                                            std::string& s ) const
{
   s.clear();
   if ( geom )
   {
      rspfRefPtr<const rspfMapProjection> mapProj = geom->getAsMapProjection();
      if ( mapProj.valid() )
      {
         const rspfDatum* datum = mapProj->getDatum();
         if ( datum )
         {
            std::string code = datum->code().string();

            if ( code == "WGE" )
            {
               s = "/Description(WGS 84)";
            }
         }
      }
   }
   
} // End: rspfPdfWriter::getLgiDictDescription

#if 0
void rspfPdfWriter::getLgiDictNeatline( rspfImageGeometry* geom,
                                         std::string& s ) const
{
   s.clear();
   if ( geom )
   {
      std::ostringstream os;

      // Tmp drb...
      os << "/Neatline[(0.0000000000)(0.0000000000)(0.0000000000)(718.0800000000)(609.8400000000)(718.0800000000)(609.8400000000)(0.0000000000)]";

      s = os.str();
   }
   
} // End: rspfPdfWriter::getLgiDictNeatline
#endif

void rspfPdfWriter::getLgiDictProjection( rspfImageGeometry* geom,
                                           std::string& s ) const
{
   s.clear(); // Empty string indicates error.
   
   // Projection:
   std::string projType;
   getLgiDictProjectionType( geom, projType );
   
   // Datum:
   std::string datumString;
   getLgiDictDatum( geom, datumString );
   
   // Description
   std::string descriptionString;
   getLgiDictDescription( geom, descriptionString );

   if ( projType.size() && datumString.size() )
   {
      std::ostringstream os;
      os << "/Projection<<"
         << projType
         << descriptionString
         << datumString
         << "/Type/Projection>>"
         << "/Display<<"
         << projType
         << descriptionString
         << datumString
         << "/Type/Projection>>";

      s = os.str();
   }
   
} // End: rspfPdfWriter::getLgiDictProjection

void rspfPdfWriter::getLgiDictCtm( rspfImageGeometry* geom,
                                    const rspfIrect& aoi,
                                    std::string& s ) const
{
   s.clear(); // Empty string indicates error.

   rspfDpt scale;
   scale.makeNan();
   rspfDpt dTie;
   dTie.makeNan();
   
   rspfRefPtr<rspfProjection> proj = geom->getProjection();
   if ( proj.valid() )
   {
      rspfRefPtr<rspfMapProjection> mapProj = dynamic_cast<rspfMapProjection*>( proj.get() );
      if ( mapProj.valid() )
      {
         if ( mapProj->isGeographic() )
         {
            // Degrees per pixel.
            geom->getDegreesPerPixel( scale );

            if ( !scale.hasNans() )
            {
               // Tie point: Lat, lon of lower left corner. 
               rspfGpt gTie;
               geom->localToWorld( rspfDpt( aoi.ll() ), gTie );

               // Check just lat and lon disregard height.
               if ( !rspf::isnan(gTie.lon) && !rspf::isnan(gTie.lon) )
               {
                  dTie.x = gTie.lon;
                  dTie.y = gTie.lat;
               }
            }
         }
      }
   }

   if ( !scale.hasNans() && !dTie.hasNans() )
   {
      //---
      // Example string:
      // (0.0000381944)(0.0000000000)(0.0000000000)(0.0000381944)(-77.0558525000)(38.8388091667)
      // NOTE: (0,0) is lower left corner in pdf space.
      rspf_float64 d = 0.0;
      std::ostringstream os;
      os << setprecision(15) // Not sure if this should be 10??? (drb)
         << std::setiosflags(std::ios_base::fixed|std::ios_base::right)
         << std::setfill('0')
         << "/CTM[(" << scale.x << ")(" << d << ")(" << d << ")("
         << scale.y << ")(" << dTie.x << ")(" << dTie.y << ")]";
      
      s = os.str();
   }
   
} // End: rspfPdfWriter::getCtmString

void rspfPdfWriter::getLgiDictProjectionType( rspfImageGeometry* geom, std::string& s ) const
{
   //---
   // Initialize the projection string from geometry:
   // Currently only coded for geographic and UTM projection.
   //---
   
   s.clear(); // Empty string indicates error.

   if ( geom )
   {
      rspfRefPtr<const rspfMapProjection> mapProj = geom->getAsMapProjection();
      if ( mapProj.valid() )
      {
         if ( mapProj->isGeographic() )
         {
            s = "/ProjectionType(GEOGRAPHIC)";
         }
         else
         {
            // Check for UTM:
            rspfRefPtr<const rspfUtmProjection> utmProj =
               dynamic_cast<const rspfUtmProjection*>( mapProj.get() );
            if ( utmProj.valid() )
            {
               rspf_int32 zone = utmProj->getZone();
               char hemisphere = utmProj->getHemisphere();
               std::ostringstream os;
               os << "/ProjectionType(UT)"
                  << "/Zone(" << zone << ")"
                  << "/Hemisphere(" << hemisphere << ")";
               s = os.str();
            }
         }
      }
      
   } // Matches if ( geom )
   
} // End: rspfPdfWriter::getProjectionType

rspf_uint32 rspfPdfWriter::getEpsgCode( const rspfImageGeometry* geom ) const
{
   rspf_uint32 code = 32767;
   if ( geom )
   {
      rspfRefPtr<const rspfProjection> proj = geom->getProjection();
      if ( proj.valid() )
      {
         // Map projection:
         rspfRefPtr<const rspfMapProjection> mapProj =
            dynamic_cast<const rspfMapProjection*>( proj.get() );
            
         if ( mapProj.valid() )
         {
            code = mapProj->getPcsCode();
         }
      }
   }
   return code;
}

#if 0
void rspfPdfWriter::getNumberOfTiles( rspf_uint32 imageWidth,
                                       rspf_uint32 imageHeight,
                                       rspfIpt size ) const
{
   const rspf_int32 DEFAULT_TILE_DIMENSION = 1024;

   size.x = imageWidth / DEFAULT_TILE_DIMENSION;
   size.y = imageHeight / DEFAULT_TILE_DIMENSION;   

   if ( imageWidth % DEFAULT_TILE_DIMENSION )
   {
      ++size.x;
   }
   if ( imageHeight % DEFAULT_TILE_DIMENSION )
   {
      ++size.y;
   }
   
} // End: getNumberOfTiles( ... )
#endif

void rspfPdfWriter::getTitle( std::string& s ) const
{
   // Check if in keyword list.
   s = m_kwl->findKey( TITLE_KW );
   
   if ( s.empty() )
   {
      if ( theFilename.size() )
      {
         // Set to basename of file.
         s = theFilename.file().string();
      }
   }
}

void rspfPdfWriter::getAuthor( std::string& s ) const
{
   // Check if in keyword list.
   s = m_kwl->findKey( AUTHOR_KW );
   
   if ( s.empty() )
   {
      s = "rspf"; // Default: ???
   }
}

void rspfPdfWriter::getSubject( std::string& s ) const
{
   // Check if in keyword list.
   s = m_kwl->findKey( SUBJECT_KW );
}

void rspfPdfWriter::getKeywords( std::string& s ) const
{
   // Check if in keyword list.
   s = m_kwl->findKey( KEYWORDS_KW ); 
}

void rspfPdfWriter::getCreator( std::string& s ) const
{
   // Check if in keyword list.
   s = m_kwl->findKey( CREATOR_KW );
   
   if ( s.empty() )
   {
      s = "rspf"; // Default: ???
   }
}

void rspfPdfWriter::getProducer( std::string& s ) const
{
   // Check if in keyword list.
   s = m_kwl->findKey( PRODUCER_KW );
   
   if ( s.empty() )
   {
      s = "rspf"; // Default: ???
   }
}

void rspfPdfWriter::getCreationDate( std::string& s ) const
{
   // Check if in keyword list.
   s = m_kwl->findKey( CREATION_DATE_KW );
   
   if ( s.empty() )
   {
      getGmtDate( s ); 
   }
}

void rspfPdfWriter::getModDate( std::string& s ) const
{
   // Check if in keyword list.
   s = m_kwl->findKey( MOD_DATE_KW );
   
   if ( s.empty() )
   {
      getGmtDate( s );
   }
}

void rspfPdfWriter::getGmtDate( std::string& s ) const
{
   // Get the time:
   time_t now;
   time(&now);
   
   // Convert to UTC:
   tm* gt = gmtime(&now);

   if ( gt )
   {
      //---
      // Format(26 charactors including trailing null):
      // (D:YYYYMMDDHHmmSSZ00'00')
      char dateChars[26];
      dateChars[25] = '\0';
      strftime(dateChars, 26, "(D:%Y%m%d%H%M%SZ00'00')", gt);
      s = dateChars;
   }
   
} // End: rspfPdfWriter::getGmtDate

void rspfPdfWriter::getTileSize( std::string& s ) const
{
   // Check if in keyword list.
   s = m_kwl->findKey( std::string( rspfKeywordNames::OUTPUT_TILE_SIZE_KW ) );
   if ( s.empty() )
   {
      rspfIpt tileSize;
      rspf::defaultTileSize( tileSize );
      s = rspfString::toString( tileSize.x ).string(); 
   }
}

void rspfPdfWriter::getTileSize( rspfIpt& tileSize ) const
{
   std::string s;
   getTileSize( s );
   if ( s.size() )
   {
      tileSize.x = rspfString(s).toInt32();
   }
   else
   {
      tileSize.x = 1024; // default
   }
   tileSize.y = tileSize.x;
}

std::string rspfPdfWriter::getTileString( rspf_uint32 x, rspf_uint32 y ) const
{
   // Format like:  /Tile_0001_0002
   std::ostringstream os;
   os << std::setiosflags(std::ios_base::fixed|std::ios_base::right)
      << std::setfill('0')
      << "/Tile_"
      << std::setw(4) << x
      << "_"
      << std::setw(4) << y;
   
   return os.str();
}
