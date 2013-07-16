//*****************************************************************************
// FILE: rspfQuickbirdRpcModel.cpp
//
// License:  See top level LICENSE.txt file.
//
// DESCRIPTION: Contains declaration of class rspfQuickbirdRpcModel. This 
//    derived class implements the capability of reading Quickbird RPC support
//    data.
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfQuickbirdRpcModel.cpp 20606 2012-02-24 12:29:52Z gpotts $

#include <rspf/projection/rspfQuickbirdRpcModel.h>
#include <rspf/base/rspfException.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/support_data/rspfQuickbirdRpcHeader.h>
#include <rspf/support_data/rspfQuickbirdTile.h>
#include <rspf/support_data/rspfNitfFile.h>
#include <rspf/support_data/rspfNitfFileHeader.h>
#include <rspf/support_data/rspfNitfImageHeader.h>
#include <rspf/support_data/rspfNitfRpcBase.h>
#include <rspf/support_data/rspfNitfUse00aTag.h>
#include <rspf/support_data/rspfNitfPiaimcTag.h>
#include <rspf/imaging/rspfTiffTileSource.h>
#include <rspf/imaging/rspfQbTileFilesHandler.h>

static const char* RPC00A_TAG = "RPC00A";
static const char* RPC00B_TAG = "RPC00B";
static const char* PIAIMC_TAG = "PIAIMC";
static const char* USE00A_TAG = "USE00A";

RTTI_DEF1(rspfQuickbirdRpcModel, "rspfQuickbirdRpcModel", rspfRpcModel);

//*************************************************************************************************
// Constructor
//*************************************************************************************************
rspfQuickbirdRpcModel::rspfQuickbirdRpcModel()
   :rspfRpcModel(),
    theSupportData(new rspfQuickbirdMetaData())
{
}

//*************************************************************************************************
// Constructor
//*************************************************************************************************
rspfQuickbirdRpcModel::rspfQuickbirdRpcModel(const rspfQuickbirdRpcModel& rhs)
   : rspfRpcModel(rhs),
     theSupportData(new rspfQuickbirdMetaData())
{
}

//*************************************************************************************************
//! Constructor for multiple tile-files sharing common RPC model initializes given pointer
//! to multi-tile-files handler.
//*************************************************************************************************
rspfQuickbirdRpcModel::rspfQuickbirdRpcModel(const rspfQbTileFilesHandler* handler)
:  rspfRpcModel(),
   theSupportData(new rspfQuickbirdMetaData())
{
   setErrorStatus();
   if (!handler)
      return;


   // Make the gsd nan so it gets computed.
   theGSD.makeNan();

   theImageClipRect = handler->getImageRectangle();

   rspfFilename imageFile = handler->getFilename();
   if (!parseRpcData(imageFile))
      return;

   //loong
   if(!theSupportData->open(imageFile))
	   return;

   finishConstruction();
   clearErrorStatus();
   return;
}

//*************************************************************************************************
// Destructor
//*************************************************************************************************
rspfQuickbirdRpcModel::~rspfQuickbirdRpcModel()
{
   theSupportData = 0;
}

//*************************************************************************************************
// Infamous DUP 
//*************************************************************************************************
rspfObject* rspfQuickbirdRpcModel::dup() const
{
   return new rspfQuickbirdRpcModel(*this);
}

//*************************************************************************************************
//! Public method for parsing generic image file.
//*************************************************************************************************
bool rspfQuickbirdRpcModel::parseFile(const rspfFilename& file)
{
   if (!parseNitfFile(file))
   {
      return parseTiffFile(file);
   }
   return true;
}

//*************************************************************************************************
//! Parses a NITF image file for RPC info. Returns TRUE if successful.
//*************************************************************************************************
bool rspfQuickbirdRpcModel::parseNitfFile(const rspfFilename& file)
{
   setErrorStatus();
   rspfFilename nitfFile = file;
   
   rspfRefPtr<rspfNitfFile> nitfFilePtr = new rspfNitfFile;
   
   if(!nitfFilePtr->parseFile(nitfFile))
   {
      nitfFile = nitfFile.setExtension("NTF");
      if(!nitfFilePtr->parseFile(nitfFile))
      {
         nitfFile = nitfFile.setExtension("ntf");
         if(!nitfFilePtr->parseFile(nitfFile))
            return false;
      }
   }
   
   rspfRefPtr<rspfNitfImageHeader> ih = nitfFilePtr->getNewImageHeader(0);
   if (!ih)
      return false;

   theImageClipRect = ih->getImageRect();
 
   // Give preference to external RPC data file:
   bool useInternalRpcTags = false;
   if(!parseRpcData(file))
      useInternalRpcTags = true;
   
   if (!parseTileData(file))
      return false;
  
   // Check for IMD (metadata) file:
   parseMetaData(file);

   // Get the gsd.
   theGSD.line = rspf::nan();
   theGSD.samp = rspf::nan();
   
   rspfRefPtr<rspfNitfRegisteredTag> tag;
   tag = ih->getTagData(PIAIMC_TAG);
   if (tag.valid())
   {
      rspfNitfPiaimcTag* p = PTR_CAST(rspfNitfPiaimcTag, tag.get());
      if (p)
      {
         theGSD.line = p->getMeanGsdInMeters();
         theGSD.samp = theGSD.line;
      }
   }
   if (rspf::isnan(theGSD.line))
   {
      tag = ih->getTagData(USE00A_TAG);
      if (tag.valid())
      {
         rspfNitfUse00aTag* p = PTR_CAST(rspfNitfUse00aTag, tag.get());
         if (p)
         {
            theGSD.line = p->getMeanGsdInMeters();
            theGSD.samp = theGSD.line;
         }
      }
   }

   // If external RPC data file was correctly parsed, then we can bypass this code block. Otherwise
   // need to parse internal NITF tags for RPC data:
   if (useInternalRpcTags)
   {
      // Get the the RPC tag:
      rspfNitfRpcBase* rpcTag = NULL;
      
      // Look for the RPC00B tag first.
      tag = ih->getTagData(RPC00B_TAG);
      if (tag.valid())
         rpcTag = PTR_CAST(rspfNitfRpcBase, tag.get());
      
      if (!tag.valid())
      {
         // Look for RPC00A tag.
         tag = ih->getTagData(RPC00A_TAG);
         if (tag.valid())
            rpcTag = PTR_CAST(rspfNitfRpcBase, tag.get());
      }
      
      if (!rpcTag)
         return false;
      
      // Set the polynomial type.
      if (rpcTag->getRegisterTagName() == "RPC00B")
         thePolyType = B;
      else
         thePolyType = A;

      // Parse coefficients:
      for (rspf_uint32 i=0; i<20; ++i)
      {
         theLineNumCoef[i] = rpcTag->getLineNumeratorCoeff(i).toFloat64();
         theLineDenCoef[i] = rpcTag->getLineDenominatorCoeff(i).toFloat64();
         theSampNumCoef[i] = rpcTag->getSampleNumeratorCoeff(i).toFloat64();
         theSampDenCoef[i] = rpcTag->getSampleDenominatorCoeff(i).toFloat64();
      }

      // Initialize other items in tags:
      theLineScale  = rpcTag->getLineScale().toFloat64();
      theSampScale  = rpcTag->getSampleScale().toFloat64();
      theLatScale   = rpcTag->getGeodeticLatScale().toFloat64();
      theLonScale   = rpcTag->getGeodeticLonScale().toFloat64();
      theHgtScale   = rpcTag->getGeodeticHeightScale().toFloat64();
      theLineOffset = rpcTag->getLineOffset().toFloat64();
      theSampOffset = rpcTag->getSampleOffset().toFloat64();
      theLatOffset  = rpcTag->getGeodeticLatOffset().toFloat64();
      theLonOffset  = rpcTag->getGeodeticLonOffset().toFloat64();
      theHgtOffset  = rpcTag->getGeodeticHeightOffset().toFloat64();
      theImageID    = ih->getImageId();
   }

   finishConstruction();
   clearErrorStatus();
   return true;
}

//*************************************************************************************************
//! Parses a tagged TIFF image file for RPC info. Returns TRUE if successful.
//*************************************************************************************************
bool rspfQuickbirdRpcModel::parseTiffFile(const rspfFilename& file)
{
   setErrorStatus();

   // Make the gsd nan so it gets computed.
   theGSD.makeNan();

   rspfFilename tiffFile = file;
   rspfRefPtr<rspfTiffTileSource> tiff = new rspfTiffTileSource();
   if (!tiff->open(file))
   {
      return false;
   }

   theImageClipRect = tiff->getImageRectangle();

   parseMetaData(file);

   if (!parseRpcData(file))
      return false;

   if (!parseTileData(file))
      return false;

   finishConstruction();
   clearErrorStatus();
   return true;
}

//*************************************************************************************************
//! Given some base name for the image data, parses the associated RPC data file. Returns TRUE
//! if successful
//*************************************************************************************************
bool rspfQuickbirdRpcModel::parseRpcData(const rspfFilename& base_name)
{
   rspfFilename rpcFile (base_name);

   // There are two possibilities for RPC data files: either each image file has its own RPC data
   // file, or a single RPC file is provided for a multi-tile scene.
   rpcFile.setExtension("RPB");
   if (!findSupportFile(rpcFile))
   {
      rpcFile.setExtension("RPA");
      if (!findSupportFile(rpcFile))
         return false;
   }

   // An RPC file was located, open it:
   rspfQuickbirdRpcHeader hdr;
   if(!hdr.open(rpcFile))
      return false;

   if(hdr.isAPolynomial())
      thePolyType = A;
   else
      thePolyType = B;

   std::copy(hdr.theLineNumCoeff.begin(), hdr.theLineNumCoeff.end(), theLineNumCoef);
   std::copy(hdr.theLineDenCoeff.begin(), hdr.theLineDenCoeff.end(), theLineDenCoef);
   std::copy(hdr.theSampNumCoeff.begin(), hdr.theSampNumCoeff.end(), theSampNumCoef);
   std::copy(hdr.theSampDenCoeff.begin(), hdr.theSampDenCoeff.end(), theSampDenCoef);

   theLineScale  = hdr.theLineScale;
   theSampScale  = hdr.theSampScale;
   theLatScale   = hdr.theLatScale;
   theLonScale   = hdr.theLonScale;
   theHgtScale   = hdr.theHeightScale;
   theLineOffset = hdr.theLineOffset;
   theSampOffset = hdr.theSampOffset;
   theLatOffset  = hdr.theLatOffset;
   theLonOffset  = hdr.theLonOffset;
   theHgtOffset  = hdr.theHeightOffset;
   theImageID    = rpcFile.fileNoExtension();

   return true;
}


//*************************************************************************************************
//! Initializes the support data member with metadata file info. Returns TRUE if successful
//*************************************************************************************************
bool rspfQuickbirdRpcModel::parseMetaData(const rspfFilename& base_name)
{

   rspfFilename metadataFile (base_name);
   metadataFile.setExtension("IMD");
   if (!findSupportFile(metadataFile))
      return false;

   if ( !theSupportData.valid() )
      theSupportData = new rspfQuickbirdMetaData();

   if(!theSupportData->open(metadataFile))
   {
      theSupportData = 0; // rspfRefPtr
      rspfNotify(rspfNotifyLevel_WARN) << "rspfQuickbirdRpcModel::parseNitfFile WARNING:"
         << "\nCould not open IMD file.  Sensor ID unknown." << std::endl;
      return false;
   }

   theSensorID = theSupportData->getSatID();
   return true;
}

//*************************************************************************************************
//! Reads the TIL file for pertinent info. Returns TRUE if successful
//*************************************************************************************************
bool rspfQuickbirdRpcModel::parseTileData(const rspfFilename& image_file)
{
   rspfFilename tileFile (image_file);
   tileFile.setExtension("TIL");
   if (!findSupportFile(tileFile))
      return false;

   rspfQuickbirdTile tileHdr;
   if(!tileHdr.open(tileFile))
      return false;

   rspfQuickbirdTileInfo info;
   if(!tileHdr.getInfo(info, image_file.file()))
      return false;

   if((info.theUlXOffset != RSPF_INT_NAN) && (info.theUlYOffset != RSPF_INT_NAN) &&
      (info.theLrXOffset != RSPF_INT_NAN) && (info.theLrYOffset != RSPF_INT_NAN) &&
      (info.theLlXOffset != RSPF_INT_NAN) && (info.theLlYOffset != RSPF_INT_NAN) &&
      (info.theUrXOffset != RSPF_INT_NAN) && (info.theUrYOffset != RSPF_INT_NAN))
   {
      theImageClipRect = rspfIrect(rspfIpt(info.theUlXOffset, info.theUlYOffset),
                                    rspfIpt(info.theUrXOffset, info.theUrYOffset),
                                    rspfIpt(info.theLrXOffset, info.theLrYOffset),
                                    rspfIpt(info.theLlXOffset, info.theLlYOffset));
   }
   else if ((info.theUlXOffset != RSPF_INT_NAN) && (info.theUlYOffset != RSPF_INT_NAN) &&
      (theImageClipRect.width() != RSPF_INT_NAN) && (theImageClipRect.height() != RSPF_INT_NAN))
   {
      theImageClipRect = rspfIrect(info.theUlXOffset, info.theUlYOffset,
                                    info.theUlXOffset+theImageClipRect.width()-1, 
                                    info.theUlYOffset+theImageClipRect.height()-1);
   }

   return true;
}

//*************************************************************************************************
//! Collects common code among all parsers
//*************************************************************************************************
void rspfQuickbirdRpcModel::finishConstruction()
{
   theImageSize.line = theImageClipRect.height();
   theImageSize.samp = theImageClipRect.width();
   theRefImgPt.line = theImageClipRect.midPoint().y;
   theRefImgPt.samp = theImageClipRect.midPoint().x;
   theRefGndPt.lat = theLatOffset;
   theRefGndPt.lon = theLonOffset;
   theRefGndPt.hgt = theHgtOffset;

   //---
   // NOTE:  We must call "updateModel()" to set parameter used by base
   // rspfRpcModel prior to calling lineSampleHeightToWorld or all
   // the world points will be same.
   //---
   updateModel();

   rspfGpt v0, v1, v2, v3;
   lineSampleHeightToWorld(theImageClipRect.ul(), theHgtOffset, v0);
   lineSampleHeightToWorld(theImageClipRect.ur(), theHgtOffset, v1);
   lineSampleHeightToWorld(theImageClipRect.lr(), theHgtOffset, v2);
   lineSampleHeightToWorld(theImageClipRect.ll(), theHgtOffset, v3);

   theBoundGndPolygon = rspfPolygon (rspfDpt(v0), rspfDpt(v1), rspfDpt(v2), rspfDpt(v3));

   // Set the ground reference point using the model.
   lineSampleHeightToWorld(theRefImgPt, theHgtOffset, theRefGndPt);

   if( theGSD.hasNans() )
   {
      try
      {
         // This will set theGSD and theMeanGSD. Method throws rspfException.
         computeGsd();
      }
      catch (const rspfException& e)
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfQuickbirdRpcModel::finishConstruction -- caught exception:\n"
            << e.what() << std::endl;
      }
   }
}

bool rspfQuickbirdRpcModel::saveState(rspfKeywordlist& kwl,
                                       const char* prefix) const
{
   // sanity check only.  This shoulc always be true
   if(theSupportData.valid())
   {
      rspfString supportPrefix = rspfString(prefix) + "support_data.";
      theSupportData->saveState(kwl, supportPrefix);
   }

   return rspfRpcModel::saveState(kwl, prefix);
}

bool rspfQuickbirdRpcModel::loadState(const rspfKeywordlist& kwl,
                                       const char* prefix)
{
   // sanity check only.  This shoulc always be true
   if(theSupportData.valid())
   {
      rspfString supportPrefix = rspfString(prefix) + "support_data.";
      theSupportData->loadState(kwl, supportPrefix);
   }
   return rspfRpcModel::loadState(kwl, prefix);
}

//*************************************************************************************************
// Given an initial filename with case-agnostic extension, this method searches first for an
// image-specific instance of that file (i.e., with R*C* in the filename) before considering
// the mosaic-global support file (R*C* removed). If a file is found, the argument is modified to
// match the actual filename and TRUE is returned. Otherwise, argument filename is left unchanged
// and FALSE is returned.
//*************************************************************************************************
bool rspfQuickbirdRpcModel::findSupportFile(rspfFilename& filename) const
{
   rspfFilename f (filename);
   rspfString extension = f.ext();
   while (true)
   {
      // Search for support file with same basename as image:
      extension.upcase();
      f.setExtension(extension);
      if (f.exists())  
         break;
      extension.downcase();
      f.setExtension(extension);
      if (f.exists())  
         break;

      // None found so far, search for mosaic-global support file:
      f = f.replaceAllThatMatch("_R[0-9]+C[0-9]+");
      if (f.exists())  
         break;
      extension.upcase();
      f.setExtension(extension);
      if (f.exists())  
         break;

      // Nothing found:
      return false;
   }

   // Modify argument to match good filename:
   filename = f;
   return true;
}
