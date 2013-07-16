#include <rspf/projection/rspfNitfRpcModel.h>
RTTI_DEF1(rspfNitfRpcModel, "rspfNitfRpcModel", rspfRpcModel);
#include <rspf/base/rspfException.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/support_data/rspfNitfFile.h>
#include <rspf/support_data/rspfNitfFileHeader.h>
#include <rspf/support_data/rspfNitfImageHeader.h>
#include <rspf/support_data/rspfNitfUse00aTag.h>
#include <rspf/support_data/rspfNitfPiaimcTag.h>
#include <rspf/support_data/rspfNitfStdidcTag.h>
#include <rspf/support_data/rspfNitfRpcBase.h>
#include <rspf/base/rspfTrace.h>
static rspfTrace traceExec  ("rspfNitfRpcModel:exec");
static rspfTrace traceDebug ("rspfNitfRpcModel:debug");
static const char* RPC00A_TAG = "RPC00A";
static const char* RPC00B_TAG = "RPC00B";
static const char* PIAIMC_TAG = "PIAIMC";
static const char* STDIDC_TAG = "STDIDC";
static const char* USE00A_TAG = "USE00A";
rspfNitfRpcModel::rspfNitfRpcModel()
   :
   rspfRpcModel(),
   theDecimation(1.0)
{
}
rspfNitfRpcModel::rspfNitfRpcModel(const rspfNitfRpcModel& rhs)
  :
   rspfRpcModel(rhs),
   theDecimation(1.0)
{
}
rspfNitfRpcModel::rspfNitfRpcModel(const rspfFilename& nitfFile)
   :
   rspfRpcModel(),
   theDecimation(1.0)
{
   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfNitfRpcModel::rspfNitfRpcModel(nitfFile): entering..."
         << std::endl;
   }
   if(!parseFile(nitfFile))
   {
      if (traceExec())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG rspfNitfRpcModel::rspfNitfRpcModel(nitfFile): Unable to parse file " << nitfFile
            << std::endl;
      }
      ++theErrorStatus;
   }
   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfNitfRpcModel::rspfNitfRpcModel(nitfFile): returning..."
         << std::endl;
   }
}
rspfObject* rspfNitfRpcModel::dup() const
{
   return new rspfNitfRpcModel(*this);
}
bool rspfNitfRpcModel::parseFile(const rspfFilename& nitfFile,
                                  rspf_uint32 entryIndex)
{
   rspfRefPtr<rspfNitfFile> file = new rspfNitfFile;
   if(!file->parseFile(nitfFile))
   {
      setErrorStatus();
      return false;
   }
   
   rspfRefPtr<rspfNitfImageHeader> ih = file->getNewImageHeader(entryIndex);
   if(!ih)
   {
      setErrorStatus();
      return false;
   }
   return parseImageHeader(ih.get());
   
}
bool rspfNitfRpcModel::parseImageHeader(const rspfNitfImageHeader* ih)
{
   if (getRpcData(ih) == false)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfNitfRpcModel::parseFile DEBUG:"
         << "\nError parsing rpc tags.  Aborting with error."
         << std::endl;
      }
      setErrorStatus();
      return false;
   }
   
   
   rspfString os = ih->getImageMagnification();
   if ( os.contains("/") )
   {
      os = os.after("/");
      rspf_float64 d = os.toFloat64();
      if (d)
      {
         theDecimation = 1.0 / d;
      }
   }
   
   theImageID = ih->getImageId();
   
   rspfIrect imageRect = ih->getImageRect();
   
   theImageSize.line =
   static_cast<rspf_int32>(imageRect.height() / theDecimation);
   theImageSize.samp =
   static_cast<rspf_int32>(imageRect.width() / theDecimation);
   
   getSensorID(ih);
   
   theRefImgPt.line = theImageSize.line/2.0;
   theRefImgPt.samp = theImageSize.samp/2.0;
   theRefGndPt.lat  = theLatOffset;
   theRefGndPt.lon  = theLonOffset;
   theRefGndPt.hgt  = theHgtOffset;
   
   theImageClipRect = rspfDrect(0.0, 0.0,
                                 theImageSize.samp-1, theImageSize.line-1);
   
   rspfGpt v0, v1, v2, v3;
   rspfDpt ip0 (0.0, 0.0);
   rspfRpcModel::lineSampleHeightToWorld(ip0, theHgtOffset, v0);
   rspfDpt ip1 (theImageSize.samp-1.0, 0.0);
   rspfRpcModel::lineSampleHeightToWorld(ip1, theHgtOffset, v1);
   rspfDpt ip2 (theImageSize.samp-1.0, theImageSize.line-1.0);
   rspfRpcModel::lineSampleHeightToWorld(ip2, theHgtOffset, v2);
   rspfDpt ip3 (0.0, theImageSize.line-1.0);
   rspfRpcModel::lineSampleHeightToWorld(ip3, theHgtOffset, v3);
   
   theBoundGndPolygon
   = rspfPolygon (rspfDpt(v0), rspfDpt(v1), rspfDpt(v2), rspfDpt(v3));
   
   updateModel();
   
   rspfRpcModel::lineSampleHeightToWorld(theRefImgPt,
                                          theHgtOffset,
                                          theRefGndPt);
   if ( theRefGndPt.isLatNan() || theRefGndPt.isLonNan() )
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfNitfRpcModel::rspfNitfRpcModel DEBUG:"
         << "\nGround Reference Point not valid." 
         << " Aborting with error..."
         << std::endl;
      }
      setErrorStatus();
      return false;
   }
   
   try
   {
      computeGsd();
   }
   catch (const rspfException& e)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfNitfRpcModel::rspfNitfRpcModel DEBUG:\n"
         << e.what() << std::endl;
      }
   }
   
   if (traceExec())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG rspfNitfRpcModel::parseFile: returning..."
      << std::endl;
   }
   
   return true;
}
void rspfNitfRpcModel::worldToLineSample(const rspfGpt& world_point,
                                          rspfDpt&       image_point) const
{
   rspfRpcModel::worldToLineSample(world_point, image_point);
   image_point.x = image_point.x * theDecimation;
   image_point.y = image_point.y * theDecimation;
}
void rspfNitfRpcModel::lineSampleHeightToWorld(
   const rspfDpt& image_point,
   const double&   heightEllipsoid,
   rspfGpt&       worldPoint) const
{
   rspfDpt pt;
   pt.x = image_point.x / theDecimation;
   pt.y = image_point.y / theDecimation;
   rspfRpcModel::lineSampleHeightToWorld(pt, heightEllipsoid, worldPoint);
}
bool rspfNitfRpcModel::saveState(rspfKeywordlist& kwl,
                                  const char* prefix) const
{
   kwl.add(prefix, "decimation", theDecimation);
   return rspfRpcModel::saveState(kwl, prefix);
}
bool rspfNitfRpcModel::loadState(const rspfKeywordlist& kwl,
                                  const char* prefix)
{
   const char* value = kwl.find(prefix, "decimation");
   if (value)
   {
      theDecimation = rspfString(value).toFloat64();
      if (theDecimation <= 0.0)
      {
         theDecimation = 1.0;
      }
   }
   return rspfRpcModel::loadState(kwl, prefix);
}
void rspfNitfRpcModel::getGsd(const rspfNitfImageHeader* ih)
{
   theGSD.line = rspf::nan();
   theGSD.samp = rspf::nan();
   if (!ih)
   {
      return;
   }
   
   rspfRefPtr<rspfNitfRegisteredTag> tag;
   tag = ih->getTagData(PIAIMC_TAG);
   if (tag.valid())
   {
      rspfNitfPiaimcTag* p = PTR_CAST(rspfNitfPiaimcTag, tag.get());
      if (p)
      {
         theGSD.line = p->getMeanGsdInMeters();
         theGSD.samp = theGSD.line;
         return;
      }
   }
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
void rspfNitfRpcModel::getSensorID(const rspfNitfImageHeader* ih)
{
   theSensorID = "UNKNOWN";
   if (!ih)
   {
      return;
   }
   
   rspfRefPtr<rspfNitfRegisteredTag> tag;
   tag = ih->getTagData(STDIDC_TAG);
   if (tag.valid())
   {
      rspfNitfStdidcTag* p = PTR_CAST(rspfNitfStdidcTag, tag.get());
      if (p)
      {
         theSensorID = p->getMission();
         return;
      }
   }
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfNitfRpcModel::getSensorID DEBUG:"
            << "\ntheSensorID set to: " << theSensorID << endl;
   }
}
bool rspfNitfRpcModel::getRpcData(const rspfNitfImageHeader* ih)
{
   if (!ih)
   {
      return false;
   }
   rspfNitfRpcBase* rpcTag = 0;
   rspfRefPtr<rspfNitfRegisteredTag> tag = ih->getTagData(RPC00B_TAG);
   if (tag.valid())
   {
      rpcTag = PTR_CAST(rspfNitfRpcBase, tag.get());
   }
   if (!rpcTag)
   {
      tag = ih->getTagData(RPC00A_TAG);
      if (tag.valid())
      {
         rpcTag = PTR_CAST(rspfNitfRpcBase, tag.get());
      }
   }
   if (!rpcTag)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfNitfRpcModel::getRpcData DEBUG:"
            << "\nCould not find neither RPC tags <" << RPC00A_TAG
            << "> nor <" << RPC00B_TAG
            << "\nAborting with error..."
            << std::endl;
      }
      return false;
   }
   if ( rpcTag->getSuccess() == false )
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfNitfRpcModel::getRpcData DEBUG:"
            << "\nSuccess flag set to false."
            << "\nAborting with error..."
            << std::endl;
      }
      return false;
   }
   if (rpcTag->getRegisterTagName() == "RPC00B")
   {
      thePolyType = B;
   }
   else
   {
      thePolyType = A;
   }
   
   theBiasError  = rpcTag->getErrorBias().toFloat64();
   theRandError  = rpcTag->getErrorRand().toFloat64();
   theLineOffset = rpcTag->getLineOffset().toFloat64();
   theSampOffset = rpcTag->getSampleOffset().toFloat64();
   theLatOffset  = rpcTag->getGeodeticLatOffset().toFloat64();
   theLonOffset  = rpcTag->getGeodeticLonOffset().toFloat64();
   theHgtOffset  = rpcTag->getGeodeticHeightOffset().toFloat64();
   theLineScale  = rpcTag->getLineScale().toFloat64();
   theSampScale  = rpcTag->getSampleScale().toFloat64();
   theLatScale   = rpcTag->getGeodeticLatScale().toFloat64();
   theLonScale   = rpcTag->getGeodeticLonScale().toFloat64();
   theHgtScale   = rpcTag->getGeodeticHeightScale().toFloat64();
   rspf_uint32 i;
   
   for (i=0; i<20; ++i)
   {
      theLineNumCoef[i] = rpcTag->getLineNumeratorCoeff(i).toFloat64();
      theLineDenCoef[i] = rpcTag->getLineDenominatorCoeff(i).toFloat64();
      theSampNumCoef[i] = rpcTag->getSampleNumeratorCoeff(i).toFloat64();
      theSampDenCoef[i] = rpcTag->getSampleDenominatorCoeff(i).toFloat64();
   }
   
   theNominalPosError = sqrt(theBiasError*theBiasError +
                             theRandError*theRandError); // meters
   return true;
}
