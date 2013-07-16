//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
// Description:
//
// Contains class implementaiton for the class "rspf LandsatTileSource".
//
//*******************************************************************
//  $Id: rspfLandsatTileSource.cpp 21512 2012-08-22 11:53:57Z dburken $

#include <rspf/imaging/rspfLandsatTileSource.h>
#include <rspf/base/rspfDirectory.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfContainerProperty.h>
#include <rspf/imaging/rspfImageGeometryRegistry.h>
#include <rspf/support_data/rspfFfL7.h>
#include <rspf/support_data/rspfFfL5.h>
#include <rspf/projection/rspfLandSatModel.h>

RTTI_DEF1_INST(rspfLandsatTileSource,
               "rspfLandsatTileSource",
               rspfGeneralRasterTileSource)

static rspfTrace traceDebug("rspfLandsatTileSource:debug");


//*******************************************************************
// Public Constructor:
//*******************************************************************
rspfLandsatTileSource::rspfLandsatTileSource()
   :
      rspfGeneralRasterTileSource(),
      theFfHdr(NULL)
{
}

//*******************************************************************
// Public Constructor:
//*******************************************************************
rspfLandsatTileSource::rspfLandsatTileSource(const rspfKeywordlist& kwl,
                                               const char* prefix)
   :
      rspfGeneralRasterTileSource(),
      theFfHdr(NULL)
{
   if (loadState(kwl, prefix) == false)
   {
      theErrorStatus = rspfErrorCodes::RSPF_ERROR;
   }
}

//*******************************************************************
// Destructor:
//*******************************************************************
rspfLandsatTileSource::~rspfLandsatTileSource()
{
   theFfHdr = NULL;
}

bool rspfLandsatTileSource::open()
{
   static const char MODULE[] = "rspfLandsatTileSource::open";

   if (traceDebug())
   {
      CLOG << " Entered..." << std::endl
           << " trying to open file " << theImageFile << std::endl;
   }
   
   rspfFilename tempFilename = theImageFile;
   // See if the file passed in is a header file.
   
   openHeader(theImageFile);

   if (!theFfHdr) return false;

   // Start building the keyword list for the general raster base class.
   rspfKeywordlist kwl;

   //***
   // There can be up to seven (six for L7) files that belong to the header.
   // Note that it seems the file names in the header are always upper case.
   // So test the file given to us to see if they should be downcased.  This
   // is assuming that all files in the directory have the same case.
   //***
   vector<rspfFilename> fileList;
   
   for (rspf_uint32 i=0; i<theFfHdr->getBandCount(); ++i)
   {
      bool addFile = false;
      rspfFilename f1 = theFfHdr->getBandFilename(i);
      if (f1.trim() != "")
      {
         // Make the file name.
         rspfFilename f2 = theImageFile.path();
         f2 = f2.dirCat(f1);
         
         if (f2.exists())
         {
            addFile = true;
         }
         else
         {
            // Try it downcased...
            f2 = theImageFile.path();
            f1.downcase();
            f2 = f2.dirCat(f1);
            if (f2.exists())
            {
               addFile = true;
            }
            else
            {
               // Try is upcased...
               f2 = theImageFile.path();
               f1.upcase();
               f2 = f2.dirCat(f1);
               if (f2.exists())
               {
                  addFile = true;
               }
            }
         }
         
         if (addFile)
         {
            if (traceDebug())
            {
               CLOG << "\nAdding file:  " << f2 << std::endl;
            }
            fileList.push_back(f2);
         }
         else 
         {
            if (traceDebug())
            {
               f2 = theImageFile.path();
               f1 = theFfHdr->getBandFilename(i);
               f1.trim();
               f2 = f2.dirCat(f1);
               CLOG << "\nCould not find:  " << f2 << std::endl;
            }
         }
      }
   }
   
   if(fileList.size() == 0)
   {
      close();
      return false;
   }
   
   rspfGeneralRasterInfo generalRasterInfo(fileList,
					    RSPF_UINT8,
					    RSPF_BSQ_MULTI_FILE,
					    (rspf_uint32)fileList.size(),
					    theFfHdr->getLinesPerBand(),
					    theFfHdr->getPixelsPerLine(),
					    0,
					    rspfGeneralRasterInfo::NONE,
					    0);
   if(fileList.size() == 1)
   {
      generalRasterInfo = rspfGeneralRasterInfo(fileList,
                                                 RSPF_UINT8,
                                                 RSPF_BSQ,
                                                 (rspf_uint32)fileList.size(),
                                                 theFfHdr->getLinesPerBand(),
                                                 theFfHdr->getPixelsPerLine(),
                                                 0,
                                                 rspfGeneralRasterInfo::NONE,
                                                 0);
   }
   theMetaData.clear();
   theMetaData.setScalarType(RSPF_UINT8);
   theMetaData.setNumberOfBands((rspf_uint32)fileList.size());   
   m_rasterInfo = generalRasterInfo;
   if(initializeHandler())
   {
      theImageFile = tempFilename;
      
      completeOpen();
   }
   else
   {
      if (traceDebug()) CLOG << " Exited..." << std::endl;
      return false;
   }
   
   if (traceDebug()) CLOG << " Exited..." << std::endl;
   
   return true;
}
   
void rspfLandsatTileSource::openHeader(const rspfFilename& file)
{
   //***
   // Landsat file name example:  l71024031_03119990929_hpn.fst
   // Three header header file type substrings:
   // HPN = Pan
   // HRF = VNIR/SWIR (visible near infrared/shortwave infrared)
   // HTM = Thermal
   //***
   rspfFilename hdr = file.file();
   hdr.downcase();
   theFfHdr = 0;
   if ( hdr.contains("hpn") || hdr.contains("hrf") || hdr.contains("htm") )
   {
      theFfHdr = new rspfFfL7(file.c_str());      
   } 
	else if (hdr.contains("header.dat"))
   {
      theFfHdr = new rspfFfL5(file.c_str());
   } 
	else 
	{
      theFfHdr = 0;
      return;
   }
   if (theFfHdr->getErrorStatus() != rspfErrorCodes::RSPF_OK)
   {
      theFfHdr = 0;
   }
   return;

   // I had to force the open to go with a header since there are duplicate entries when scanning
   // landsat directories.
   //  For now I am commenting this code out.
   //
#if 0
   //***
   // User may have passed in an image file name in which case the header file
   // must be derived from it.
   //***
   if (hdr.size() < 25)
   {
      // file name not long enough...
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfLandsatTileSource::openHeader DEBUG:"
            << "\nNot a standard landsat 7 file name:  " << hdr << std::endl;
         return;
      }
   }
   
   char substr[4];
   const char* f = hdr.c_str();
   strncpy(substr, (f+22), 3);
   substr[3] = '\0';
   rspfString s1 = substr;
   rspfString s2;
   s1.downcase();
   if (s1 == "b80")
   {
      s2 = "hpn";
   }
   else if (s1 == "b61" || s1 == "b62")
   {
      s2 = "htm";
   }
   else if (s1 == "b10" || s1 == "b20" || s1 == "b30" ||
            s1 == "b40" || s1 == "b50" || s1 == "b70")
   {
      s2 = "hrf";
   }
   else
   {
      // Not of any format we know of...
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfLandsatTileSource::openHeader DEBUG:"
            << "\nCould not derive header name from:  " << file
            << std::endl;
      }

      return;
   }

   // Set the case to be the same as the file passed in.
   if (substr[0] == 0x42) // ascii "B"
   {
      s1.upcase();
      s2.upcase();
      hdr.upcase();

      // Header files alway start with "L71"
      hdr = hdr.substitute(rspfString("L72"), rspfString("L71"));  
   }
   else
   {
      // Header files alway start with "l71"
      hdr = hdr.substitute(rspfString("l72"), rspfString("l71")); 
   }

   // Make the hdr file name.
   hdr = hdr.substitute(s1, s2);

   rspfFilename f1 = file.drive();
   f1 += file.path();
   hdr = f1.dirCat(hdr);
   theFfHdr = new rspfFfL7(hdr.c_str());

   if (theFfHdr->getErrorStatus() != rspfErrorCodes::RSPF_OK)
   {
      delete theFfHdr;
      theFfHdr = NULL;
   }
#endif
}
   
rspfRefPtr<rspfImageGeometry>  rspfLandsatTileSource::getImageGeometry()
{
   if ( !theGeometry )
   {
      // Check for external geom:
      theGeometry = getExternalImageGeometry();
      
      if ( !theGeometry )
      {
         theGeometry = new rspfImageGeometry();

         if ( theFfHdr.valid() )
         {
            // Make a model
            rspfLandSatModel* model = new rspfLandSatModel (*theFfHdr);
            if (model->getErrorStatus() == rspfErrorCodes::RSPF_OK)
            {
               //initialize the image geometry object with the model
               theGeometry->setProjection(model);
            }
         }

         //---
         // WARNING:
         // Must create/set the geometry at this point or the next call to
         // rspfImageGeometryRegistry::extendGeometry will put us in an infinite loop
         // as it does a recursive call back to rspfImageHandler::getImageGeometry().
         //---         
         
         // Check for set projection.
         if ( !theGeometry->getProjection() )
         {
            // Try factories for projection.
            rspfImageGeometryRegistry::instance()->extendGeometry(this);
         }
      }

      // Set image things the geometry object should know about.
      initImageParameters( theGeometry.get() );
   }
   
   return theGeometry;
}

bool rspfLandsatTileSource::loadState(const rspfKeywordlist& kwl,
                                       const char* prefix)
{
   const char* lookup = kwl.find(prefix, rspfKeywordNames::FILENAME_KW);
   if (lookup)
   {
      rspfFilename fileName = lookup;

      rspfString ext = fileName.ext();

      if((ext.upcase() == "FST") || (ext.upcase() == "DAT"))
      {
         //---
         // This will call:
         // rspfImageHandler::loadState() the open()
         //---
         if (rspfGeneralRasterTileSource::loadState(kwl, prefix))
         {
            return true;
         }
      }
   }
   return false;
}

rspfRefPtr<rspfProperty> rspfLandsatTileSource::getProperty(
   const rspfString& name)const
{
   rspfRefPtr<rspfProperty> result = 0;

	if(name == "file_type")
	{
		if(rspfString(getFilename()).downcase().contains("header"))
		{
			return (new rspfStringProperty(name, "landsat5"));
		}
		else
		{
			return (new rspfStringProperty(name, "landsat7"));
		}
	}
   else if (theFfHdr.valid())
   {
      result = theFfHdr->getProperty(name);
   }

   if ( result.valid() == false )
   {
      result = rspfGeneralRasterTileSource::getProperty(name);
   }

   return result;
}

void rspfLandsatTileSource::getPropertyNames(
   std::vector<rspfString>& propertyNames)const
{
	propertyNames.push_back("file_type");
   if (theFfHdr.valid())
   {
      theFfHdr->getPropertyNames(propertyNames);
   }
   rspfGeneralRasterTileSource::getPropertyNames(propertyNames);
}

rspfString rspfLandsatTileSource::getShortName() const
{
   return rspfString("Landsat");
}
   
rspfString rspfLandsatTileSource::getLongName() const
{
   return rspfString("Landsat reader");
}

double rspfLandsatTileSource::getNullPixelValue(rspf_uint32)const
{
   return 0.0;
}

double rspfLandsatTileSource::getMinPixelValue(rspf_uint32)const
{
   return 1.0;
}

double rspfLandsatTileSource::getMaxPixelValue(rspf_uint32)const
{
   return 255.0;
}

rspfScalarType rspfLandsatTileSource::getOutputScalarType() const
{
   return RSPF_UINT8;
}

bool rspfLandsatTileSource::getAcquisitionDate(rspfDate& date)const
{
   if(!theFfHdr) return false;

   theFfHdr->getAcquisitionDate(date);

   return true;
}

rspfString rspfLandsatTileSource::getSatelliteName()const
{
   if(!theFfHdr) return "";

   return theFfHdr->getSatelliteName();
}

rspfFilename rspfLandsatTileSource::getBandFilename(rspf_uint32 idx)const
{
   rspf_uint32 maxIdx = getNumberOfInputBands();

   if(!theFfHdr||(idx > maxIdx))
   {
      return "";
   }

   rspfFilename path = getFilename().path();
   rspfString filename = theFfHdr->getBandFilename(idx);
   filename = filename.trim();
   rspfFilename file = path.dirCat(filename);

   if (file.exists())
   {
      return file;
   }

   // Try downcased name.
   file = path.dirCat(filename.downcase());
   if (file.exists())
   {
      return file;
   }
   
   // Try upcase name.
   file = path.dirCat(filename.upcase());
   if (file.exists())
   {
      return file;
   }
   
   return rspfFilename();
}

bool rspfLandsatTileSource::isPan()const
{
   return (getNumberOfInputBands() == 1);
}

bool rspfLandsatTileSource::isVir()const
{
   return (getNumberOfInputBands() == 6);
}

bool rspfLandsatTileSource::isTm()const
{
   return (getNumberOfInputBands() == 2);
}
   
