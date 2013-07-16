#include <rspf/elevation/rspfGeneralRasterElevHandler.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfStreamFactoryRegistry.h>
#include <rspf/base/rspfDirectory.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfUnitTypeLut.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfImageViewTransform.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfGpt.h>
#include <OpenThreads/ScopedLock>

RTTI_DEF1(rspfGeneralRasterElevHandler, "rspfGeneralRasterElevHandler", rspfElevCellHandler);

rspfGeneralRasterElevHandler::rspfGeneralRasterElevHandler(const rspfFilename& file)
   :rspfElevCellHandler(file.c_str()),
    m_streamOpen(false)
{
   if(!open(file))
   {
      setErrorStatus();
   }
}

rspfGeneralRasterElevHandler::rspfGeneralRasterElevHandler(const rspfGeneralRasterElevHandler& src)
   :rspfElevCellHandler(src),
    theGeneralRasterInfo(src.theGeneralRasterInfo),
    m_streamOpen(false), // ????
    m_memoryMap(src.m_memoryMap)
{
}

rspfGeneralRasterElevHandler::rspfGeneralRasterElevHandler(const rspfGeneralRasterElevHandler::GeneralRasterInfo& generalRasterInfo)
   : m_streamOpen(false)
{

   close();
   theGeneralRasterInfo = generalRasterInfo;
   if(!open(theGeneralRasterInfo.theFilename))
   {
      theErrorStatus = rspfErrorCodes::RSPF_ERROR;
   }
}

rspfObject* rspfGeneralRasterElevHandler::dup()const
{
   return new rspfGeneralRasterElevHandler(*this);
}

const rspfGeneralRasterElevHandler& rspfGeneralRasterElevHandler::operator=(const rspfGeneralRasterElevHandler& src)
{
   rspfElevCellHandler::operator=(*(const rspfElevCellHandler*)&src);

   return *this;
}

rspfGeneralRasterElevHandler::~rspfGeneralRasterElevHandler()
{
}

double rspfGeneralRasterElevHandler::getHeightAboveMSL(const rspfGpt& gpt)
{
   rspf_float64 result = theGeneralRasterInfo.theNullHeightValue;

   if(m_memoryMap.empty())
   {
      switch(theGeneralRasterInfo.theScalarType)
      {
         case RSPF_SINT8:
         {
            result = getHeightAboveMSLFileTemplate((rspf_sint8)0,
                                                   theGeneralRasterInfo,
                                                   gpt);
            break;
         }
         case RSPF_UINT8:
         {
            result = getHeightAboveMSLFileTemplate((rspf_uint8)0,
                                                   theGeneralRasterInfo,
                                                   gpt);
            break;
         }
         case RSPF_SINT16:
         {
            result = getHeightAboveMSLFileTemplate((rspf_sint16)0,
                                                   theGeneralRasterInfo,
                                                   gpt);
            break;
         }
         case RSPF_UINT16:
         {
            result = getHeightAboveMSLFileTemplate((rspf_uint16)0,
                                                   theGeneralRasterInfo,
                                                   gpt);
            break;
         }
         case RSPF_SINT32:
         {
            result = getHeightAboveMSLFileTemplate((rspf_sint32)0,
                                                   theGeneralRasterInfo,
                                                   gpt);
            break;
         }
         case RSPF_UINT32:
         {
            result = getHeightAboveMSLFileTemplate((rspf_uint32)0,
                                                   theGeneralRasterInfo,
                                                   gpt);
            break;
         }
         case RSPF_FLOAT32:
         {
            result =  getHeightAboveMSLFileTemplate((rspf_float32)0,
                                                    theGeneralRasterInfo,
                                                    gpt);
            break;
         }
         case RSPF_FLOAT64:
         {
            result = getHeightAboveMSLFileTemplate((rspf_float64)0,
                                                   theGeneralRasterInfo,
                                                   gpt);
            break;
         }
         default:
         {
            break;
         }
      }
   }
   else
   {
      switch(theGeneralRasterInfo.theScalarType)
      {
         case RSPF_SINT8:
         {
            result = getHeightAboveMSLMemoryTemplate((rspf_sint8)0,
                                                   theGeneralRasterInfo,
                                                   gpt);
            break;
         }
         case RSPF_UINT8:
         {
            result = getHeightAboveMSLMemoryTemplate((rspf_uint8)0,
                                                   theGeneralRasterInfo,
                                                   gpt);
            break;
         }
         case RSPF_SINT16:
         {
            result = getHeightAboveMSLMemoryTemplate((rspf_sint16)0,
                                                   theGeneralRasterInfo,
                                                   gpt);
            break;
         }
         case RSPF_UINT16:
         {
            result = getHeightAboveMSLMemoryTemplate((rspf_uint16)0,
                                                   theGeneralRasterInfo,
                                                   gpt);
            break;
         }
         case RSPF_SINT32:
         {
            result = getHeightAboveMSLMemoryTemplate((rspf_sint32)0,
                                                   theGeneralRasterInfo,
                                                   gpt);
            break;
         }
         case RSPF_UINT32:
         {
            result = getHeightAboveMSLMemoryTemplate((rspf_uint32)0,
                                                   theGeneralRasterInfo,
                                                   gpt);
            break;
         }
         case RSPF_FLOAT32:
         {
            result =  getHeightAboveMSLMemoryTemplate((rspf_float32)0,
                                                    theGeneralRasterInfo,
                                                    gpt);
            break;
         }
         case RSPF_FLOAT64:
         {
            result = getHeightAboveMSLMemoryTemplate((rspf_float64)0,
                                                   theGeneralRasterInfo,
                                                   gpt);
            break;
         }
         default:
         {
            break;
         }
      }
      
   }

   return result;
}

rspfIpt rspfGeneralRasterElevHandler::getSizeOfElevCell() const
{
   return rspfIpt(theGeneralRasterInfo.theWidth, theGeneralRasterInfo.theHeight);
}
   
double rspfGeneralRasterElevHandler::getPostValue(const rspfIpt& /* gridPt */) const
{
   rspfNotify(rspfNotifyLevel_WARN)
      << "rspfGeneralRasterElevHandler::getPostValue(const rspfIpt& gridPt):  NEED TO IMPLEMENT TO NEW INTERFACE\n";
   return rspf::nan();
}

bool rspfGeneralRasterElevHandler::isOpen()const
{
   if(!m_memoryMap.empty()) return true;
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_inputStreamMutex);

   //---
   // Change to use flag as is_open is non-const on some old compilers.
   return m_streamOpen;
   // return (m_inputStream.is_open());
}

bool rspfGeneralRasterElevHandler::open(const rspfFilename& file, bool memoryMapFlag)
{
   close();
   if(!setFilename(file)) return false;
   m_inputStream.clear();
   m_inputStream.open(theGeneralRasterInfo.theFilename.c_str(), ios::in | ios::binary);

   if(memoryMapFlag)
   {
      if(!m_inputStream.bad())
      {
         m_memoryMap.resize(theGeneralRasterInfo.theFilename.fileSize());
         if(!m_memoryMap.empty())
         {
           m_inputStream.read((char*)(&m_memoryMap.front()), (streamsize)m_memoryMap.size());
         }
      }
      m_inputStream.close();
   }

   // Capture the stream state for non-const is_open on old compiler.
   m_streamOpen = m_inputStream.is_open();
   
   return m_streamOpen;
}

/**
 * Closes the stream to the file.
 */
void rspfGeneralRasterElevHandler::close()
{
   m_inputStream.close();
   m_memoryMap.clear();
   m_streamOpen = false;
}

bool rspfGeneralRasterElevHandler::setFilename(const rspfFilename& file)
{
   if(file.trim() == "")
   {
      return false;
   }
   theFilename = file;
   rspfFilename hdrFile  = file;
   rspfFilename geomFile = file;
   theGeneralRasterInfo.theFilename = file;
   theGeneralRasterInfo.theWidth = 0;
   theGeneralRasterInfo.theHeight = 0;
   theNullHeightValue = rspf::nan();
   hdrFile = hdrFile.setExtension("omd");
   geomFile = geomFile.setExtension("geom");

   if(!hdrFile.exists()||
      !geomFile.exists())
   {
      return false;
   }
   rspfKeywordlist kwl(hdrFile);
   if (kwl.getErrorStatus() == rspfErrorCodes::RSPF_ERROR)
   {
      return false;
   }
   
   kwl.add(rspfKeywordNames::FILENAME_KW, file.c_str(), true);
   rspfGeneralRasterInfo generalInfo;
   
   if(!generalInfo.loadState(kwl))
   {
      return false;
   }
   if(generalInfo.numberOfBands() != 1)
   {
      rspfNotify(rspfNotifyLevel_WARN) << "rspfGeneralRasterElevHandler::initializeInfo WARNING:The number of bands are not specified in the header file" << std::endl;
      return false;
   }

   kwl.clear();
   if(kwl.addFile(geomFile))
   {
      theGeneralRasterInfo.theNullHeightValue = generalInfo.getImageMetaData().getNullPix(0);
      theGeneralRasterInfo.theImageRect       = generalInfo.imageRect();
      theGeneralRasterInfo.theUl              = theGeneralRasterInfo.theImageRect.ul();
      theGeneralRasterInfo.theLr              = theGeneralRasterInfo.theImageRect.lr();
      theGeneralRasterInfo.theWidth           = theGeneralRasterInfo.theImageRect.width();
      theGeneralRasterInfo.theHeight          = theGeneralRasterInfo.theImageRect.height();
      theGeneralRasterInfo.theImageRect       = generalInfo.imageRect();
      theGeneralRasterInfo.theByteOrder       = generalInfo.getImageDataByteOrder();
      theGeneralRasterInfo.theScalarType      = generalInfo.getImageMetaData().getScalarType();
      theGeneralRasterInfo.theBytesPerRawLine = generalInfo.bytesPerRawLine();

      //add  by simbla

      // ---
      // Try to determine if there is a prefix like "image0." or not.
      // ---
      std::string prefix = "";
      if ( kwl.find("image0.type") )
      {
         prefix = "image0.";
      }
      
      theGeneralRasterInfo.theGeometry = new rspfImageGeometry;
      if(!theGeneralRasterInfo.theGeometry->loadState( kwl, prefix.c_str() ))
      {
         theGeneralRasterInfo.theGeometry = 0;
      }
      
      if(!theGeneralRasterInfo.theGeometry.valid())
      {
         return false;
      }
      rspfGpt defaultDatum;
      rspfGpt ulGpt;
      rspfGpt urGpt;
      rspfGpt lrGpt;
      rspfGpt llGpt;
      theGeneralRasterInfo.theDatum = defaultDatum.datum();
      theGeneralRasterInfo.theGeometry->localToWorld(theGeneralRasterInfo.theImageRect.ul(), ulGpt);
      theGeneralRasterInfo.theGeometry->localToWorld(theGeneralRasterInfo.theImageRect.ur(), urGpt);
      theGeneralRasterInfo.theGeometry->localToWorld(theGeneralRasterInfo.theImageRect.lr(), lrGpt);
      theGeneralRasterInfo.theGeometry->localToWorld(theGeneralRasterInfo.theImageRect.ll(), llGpt);
      
      ulGpt.changeDatum(theGeneralRasterInfo.theDatum);
      urGpt.changeDatum(theGeneralRasterInfo.theDatum);
      lrGpt.changeDatum(theGeneralRasterInfo.theDatum);
      llGpt.changeDatum(theGeneralRasterInfo.theDatum);
      theMeanSpacing = theGeneralRasterInfo.theGeometry->getMetersPerPixel().y;
      theGroundRect = rspfGrect(ulGpt, urGpt, lrGpt, llGpt);
      theGeneralRasterInfo.theWgs84GroundRect = rspfDrect(ulGpt, urGpt, lrGpt, llGpt, RSPF_RIGHT_HANDED);
      theNullHeightValue = theGeneralRasterInfo.theNullHeightValue;
   }
   else
   {
      return false;
   }
   
   return true;
}

#if 0
void rspfGeneralRasterElevHandler::initializeList(const rspfFilename& file)
{
   close();
   theFilename = file;
   theBoundingRectInfoList.clear();
   if(theFilename.exists())
   {
      if(theFilename.isDir())
      {
         rspfDirectory dir(theFilename);

         rspfFilename file;
         rspf_uint32 maxCount = 10;
         rspf_uint32 count = 0;
         bool foundOne = false;
         if(dir.getFirst(file))
         {
            do
            {
               ++count;
               rspfString ext = file.ext();
               ext = ext.downcase();
               if(ext == "ras")
               {
                  rspfGeneralRasterElevHandler::BoundingRectInfo info;
                  if(initializeInfo(info, file))
                  {
                     foundOne = true;
                     addInfo(info);
                  }
               }
            } while(dir.getNext(file) &&
                   (!foundOne &&
                    (count < maxCount)));
         }
#if 0
         std::vector<rspfFilename> result;
         
         dir.findAllFilesThatMatch(result,
                                   ".*\\.ras");
      
         if(result.size())
         {
            rspfGeneralRasterElevHandler::BoundingRectInfo info;
            rspf_uint32 idx = 0;
            for(idx = 0; idx < result.size(); ++idx)
            {
               if(initializeInfo(info, rspfFilename(result[idx])))
               {
                  addInfo(info);
               }
            }
         }
#endif
         if(theBoundingRectInfoList.size()>0)
         {
            theNullHeightValue = theBoundingRectInfoList[0].theNullHeightValue;
         }
      }
      else
      {
         rspfGeneralRasterElevHandler::BoundingRectInfo info;
         if(initializeInfo(info, theFilename))
         {
            addInfo(info);
         }
      }
   }
}
#endif
template <class T>
double rspfGeneralRasterElevHandler::getHeightAboveMSLFileTemplate(
   T /* dummy */,
   const rspfGeneralRasterElevHandler::GeneralRasterInfo& info,
   const rspfGpt& gpt)
{
   rspfEndian endian;
   
   rspfGpt shiftedPoint = gpt;
   shiftedPoint.changeDatum(info.theDatum);
   if(!info.theWgs84GroundRect.pointWithin(shiftedPoint))
   {
      return rspf::nan();
   }
   
   rspfDpt pt;
   info.theGeometry->worldToLocal(shiftedPoint,pt);
   double xi = pt.x;
   double yi = pt.y;
   
   xi -= info.theUl.x;
   yi -= info.theUl.y;
   
   //modifed by simbla  2008 7.17
   //double xi = (shiftedPoint.lond() - info.theUlGpt.lond())/info.thePostSpacing.x;
   //double yi = (info.theUlGpt.latd() -
   //             shiftedPoint.latd())/info.thePostSpacing.y;
   
   
   rspf_sint64 x0 = static_cast<rspf_sint64>(xi);
   rspf_sint64 y0 = static_cast<rspf_sint64>(yi);
   
   double xt0 = xi - x0;
   double yt0 = yi - y0;
   double xt1 = 1-xt0;
   double yt1 = 1-yt0;
   
   double w00 = xt1*yt1;
   double w01 = xt0*yt1;
   double w10 = xt1*yt0;
   double w11 = xt0*yt0;
   
   
   if ( xi < 0 || yi < 0 ||
       x0 > (info.theWidth  - 1.0) ||
       y0 > (info.theHeight  - 1.0) )
   {
      return rspf::nan();
   }
   
   if(x0 == (info.theWidth  - 1.0))
   {
      --x0;
   }
   if(y0 == (info.theHeight  - 1.0))
   {
      --y0;
   }
   T p[4];
   
   rspf_uint64 bytesPerLine  = info.theBytesPerRawLine;
   
   std::streampos offset = y0*bytesPerLine + x0*sizeof(T);
   
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_inputStreamMutex);
      if(m_inputStream.fail())
      {
         m_inputStream.clear();
      }
      m_inputStream.seekg(offset, ios::beg);
      m_inputStream.read((char*)p, sizeof(T));
      
      // Get the second post.
      m_inputStream.read((char*)(p+1), sizeof(T));
      
      //   offset += (bytesPerLine-2*sizeof(T));
      
      m_inputStream.ignore(bytesPerLine-2*sizeof(T));
      // Get the third post.
      m_inputStream.read((char*)(p+2), sizeof(T));
      
      // Get the fourth post.
      m_inputStream.read((char*)(p+3), sizeof(T));
      
      if(m_inputStream.fail())
      {
         m_inputStream.clear();
         return rspf::nan();
      }
   }
   if(endian.getSystemEndianType() != info.theByteOrder)
   {
      endian.swap((T*)p, (rspf_uint32)4);
   }
   double p00 = p[0];
   double p01 = p[1];
   double p10 = p[2];
   double p11 = p[3];
   
   if (p00 == info.theNullHeightValue)
      w00 = 0.0;
   if (p01 == info.theNullHeightValue)
      w01 = 0.0;
   if (p10 == info.theNullHeightValue)
      w10 = 0.0;
   if (p11 == info.theNullHeightValue)
      w11 = 0.0;
   
#if 0 /* Serious debug only... */
   cout << "\np00:  " << p00
   << "\np01:  " << p01
   << "\np10:  " << p10
   << "\np11:  " << p11
   << "\nw00:  " << w00
   << "\nw01:  " << w01
   << "\nw10:  " << w10
   << "\nw11:  " << w11
   << endl;
#endif
   
   double sum_weights = w00 + w01 + w10 + w11;
   
   if (sum_weights)
   {
      return (p00*w00 + p01*w01 + p10*w10 + p11*w11) / sum_weights;
   }
   
   return rspf::nan();
}

template <class T>
double rspfGeneralRasterElevHandler::getHeightAboveMSLMemoryTemplate(
   T /* dummy */,
   const rspfGeneralRasterElevHandler::GeneralRasterInfo& info,
   const rspfGpt& gpt)
{
   rspfEndian endian;
   
   rspfGpt shiftedPoint = gpt;
   shiftedPoint.changeDatum(info.theDatum);
   if(!info.theWgs84GroundRect.pointWithin(shiftedPoint))
   {
      return rspf::nan();
   }
   
   rspfDpt pt;
   info.theGeometry->worldToLocal(shiftedPoint,pt);
   double xi = pt.x;
   double yi = pt.y;
   
   xi -= info.theUl.x;
   yi -= info.theUl.y;
   
   //modifed by simbla  2008 7.17
   //double xi = (shiftedPoint.lond() - info.theUlGpt.lond())/info.thePostSpacing.x;
   //double yi = (info.theUlGpt.latd() -
   //             shiftedPoint.latd())/info.thePostSpacing.y;
   
   
   rspf_sint64 x0 = static_cast<rspf_sint64>(xi);
   rspf_sint64 y0 = static_cast<rspf_sint64>(yi);
   
   double xt0 = xi - x0;
   double yt0 = yi - y0;
   double xt1 = 1-xt0;
   double yt1 = 1-yt0;
   
   double w00 = xt1*yt1;
   double w01 = xt0*yt1;
   double w10 = xt1*yt0;
   double w11 = xt0*yt0;
   if ( xi < 0 || yi < 0 ||
       x0 > (info.theWidth  - 1.0) ||
       y0 > (info.theHeight  - 1.0) )
   {
      return rspf::nan();
   }
   
   if(x0 == (info.theWidth  - 1.0))
   {
      --x0;
   }
   if(y0 == (info.theHeight  - 1.0))
   {
      --y0;
   }
   rspf_uint64 bytesPerLine  = info.theBytesPerRawLine;
   
   rspf_uint64 offset = y0*bytesPerLine + x0*sizeof(T);
   rspf_uint64 offset2 = offset+bytesPerLine;
   
   T v00 = *(reinterpret_cast<T*> (&m_memoryMap[offset]));
   T v01 = *(reinterpret_cast<T*> (&m_memoryMap[offset + sizeof(T)]));
   T v10 = *(reinterpret_cast<T*> (&m_memoryMap[offset2]));
   T v11 = *(reinterpret_cast<T*> (&m_memoryMap[offset2 + sizeof(T)]));
   if(endian.getSystemEndianType() != info.theByteOrder)
   {
      endian.swap(v00);
      endian.swap(v01);
      endian.swap(v10);
      endian.swap(v11);
   }
   double p00 = v00;
   double p01 = v01;
   double p10 = v10;
   double p11 = v11;
   
   if (p00 == info.theNullHeightValue)
      w00 = 0.0;
   if (p01 == info.theNullHeightValue)
      w01 = 0.0;
   if (p10 == info.theNullHeightValue)
      w10 = 0.0;
   if (p11 == info.theNullHeightValue)
      w11 = 0.0;
   
#if 0 /* Serious debug only... */
   cout << "\np00:  " << p00
   << "\np01:  " << p01
   << "\np10:  " << p10
   << "\np11:  " << p11
   << "\nw00:  " << w00
   << "\nw01:  " << w01
   << "\nw10:  " << w10
   << "\nw11:  " << w11
   << endl;
#endif
   
   double sum_weights = w00 + w01 + w10 + w11;
   
   if (sum_weights)
   {
      return (p00*w00 + p01*w01 + p10*w10 + p11*w11) / sum_weights;
   }
   
   return rspf::nan();
}
#if 0
void rspfGeneralRasterElevHandler::addInfo(const rspfGeneralRasterElevHandler::BoundingRectInfo& info)
{
   if(theBoundingRectInfoList.empty())
   {
      theBoundingRectInfoList.push_back(info);
      theNullHeightValue = info.theNullHeightValue;
      theBoundingRect = info.theBounds;
   }
   else
   {
      BoundingRectListType::iterator iter = theBoundingRectInfoList.begin();
      bool found = false;
//       if(theNullHeightValue ==  info.theNullHeightValue)
//       {
      while((iter!=theBoundingRectInfoList.end())&&
            (!found))
      {
         if(info.thePostSpacing.length() <= iter->thePostSpacing.length())
         {
            theBoundingRectInfoList.insert(iter, info);
            theBoundingRect = theBoundingRect.combine(info.theBounds);
            found = true;
         }
         else
         {
            ++iter;
         }
      }
      if(!found)
      {
         theBoundingRectInfoList.push_back(info);
      }
   }
}
#endif
rspfDrect rspfGeneralRasterElevHandler::getBoundingRect()const
{
   return theGeneralRasterInfo.theWgs84GroundRect;
}

bool rspfGeneralRasterElevHandler::pointHasCoverage(const rspfGpt& gpt) const
{
   rspfDpt pt = gpt;

   return theGeneralRasterInfo.theWgs84GroundRect.pointWithin(pt);
//    BoundingRectListType::const_iterator i = theBoundingRectInfoList.begin();
//    while (i != theBoundingRectInfoList.end())
//    {
//       if ((*i).theBounds.pointWithin(pt))
//       {
//          return true;
//       }
//       ++i;
//    }
//    return false;
}
const rspfGeneralRasterElevHandler::GeneralRasterInfo& rspfGeneralRasterElevHandler::generalRasterInfo()const
{
   return theGeneralRasterInfo;
}
