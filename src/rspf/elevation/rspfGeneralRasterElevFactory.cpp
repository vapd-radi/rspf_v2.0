#include <rspf/elevation/rspfGeneralRasterElevFactory.h>
#include <rspf/elevation/rspfGeneralRasterElevHandler.h>
#include <rspf/base/rspfDirectory.h>

RTTI_DEF1(rspfGeneralRasterElevFactory, "rspfGeneralRasterElevFactory", rspfElevSourceFactory)

rspfGeneralRasterElevFactory::rspfGeneralRasterElevFactory()
   :theHandlerReturnedFlag(false)
{
}

rspfGeneralRasterElevFactory::rspfGeneralRasterElevFactory(const rspfFilename& dir)
   :theHandlerReturnedFlag(false)
{
   setDirectory(dir);
}

rspfGeneralRasterElevFactory::~rspfGeneralRasterElevFactory()
{
}

void rspfGeneralRasterElevFactory::setDirectory(const rspfFilename& directory)
{
   rspfElevSourceFactory::setDirectory(directory);
   theGeneralRasterInfoList.clear();
   rspfRefPtr<rspfGeneralRasterElevHandler>  handler = new rspfGeneralRasterElevHandler;
  
   if(theDirectory.exists())
   {
      if(theDirectory.isDir())
      {
         rspfDirectory dir(theDirectory);
         
         rspfFilename file;
         rspf_uint32 maxCount = 10; // search at least the first 10 files to see if there are any here
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
                  if(handler->open(file))
                  {
                     foundOne = true;
                     addInfo(handler->generalRasterInfo());
                  }
               }
            } while(dir.getNext(file) &&
                    (foundOne ||
                     (!foundOne && (count < maxCount))));
         }
      }
   }  
}

rspfElevSource* rspfGeneralRasterElevFactory::getNewElevSource(const rspfGpt& gpt) const
{
   rspfDpt pt(gpt);
   std::vector<rspfGeneralRasterElevHandler::GeneralRasterInfo>::const_iterator i = theGeneralRasterInfoList.begin();
   while (i != theGeneralRasterInfoList.end())
   {
      if ((*i).theWgs84GroundRect.pointWithin(pt))
      {
         return new rspfGeneralRasterElevHandler((*i));
         
      }
      ++i;
   }
   return 0;
}

void rspfGeneralRasterElevFactory::addInfo(const rspfGeneralRasterElevHandler::GeneralRasterInfo& info)
{
   theGeneralRasterInfoList.push_back(info);
}

void rspfGeneralRasterElevFactory::createIndex()
{
}
