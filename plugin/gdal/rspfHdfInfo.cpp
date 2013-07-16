#include <gdal_priv.h>
#include <cpl_string.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspfHdfInfo.h>
static rspfTrace traceDebug("rspfHdfInfo:debug");
rspfHdfInfo::rspfHdfInfo()
   : rspfInfoBase(),
     theFile(),
     m_hdfReader(0),
     m_driverName()
{
}
rspfHdfInfo::~rspfHdfInfo()
{
   m_hdfReader = 0;
   m_globalMeta.clear();
   m_globalMetaVector.clear();
}
bool rspfHdfInfo::open(const rspfFilename& file)
{
   static const char MODULE[] = "rspfHdfInfo::open";
   if (traceDebug())
   {    
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " entered...\n"
         << "file: " << file << "\n";
   }
   bool result = false;
   
   rspfString ext = file.ext();
   ext.downcase();
   if ( ext == "hdf" || ext == "h4" || ext == "hdf4" || 
     ext == "he4" || ext == "hdf5" || ext == "he5" || ext == "h5" || 
     ext == "l1r")
   {
     m_hdfReader = new rspfHdfReader;
     m_hdfReader->setFilename(file);
     if ( m_hdfReader->open() )
     {
       m_driverName = m_hdfReader->getDriverName();
       if (m_driverName.contains("HDF4"))
       {
         m_driverName = "hdf4";
       }
       else if (m_driverName.contains("HDF5"))
       {
         m_driverName = "hdf5";
       }
       theFile = file;
       m_globalMeta.clear();
       m_globalMetaVector.clear();
       GDALDatasetH dataset = GDALOpen(theFile.c_str(), GA_ReadOnly);
       if (dataset != 0)
       {
          char** papszMetadata = GDALGetMetadata(dataset, NULL);
          if( CSLCount(papszMetadata) > 0 )
          {
             for(rspf_uint32 metaIndex = 0; papszMetadata[metaIndex] != 0; ++metaIndex)
             {
                rspfString metaInfo = papszMetadata[metaIndex];
                if (metaInfo.contains("="))
                {
                   std::vector<rspfString> metaInfos = metaInfo.split("=");
                   if (metaInfos.size() > 1)
                   {
                      rspfString key = metaInfos[0];
                      rspfString keyStr = key.substitute(":", ".", true);
                      keyStr = keyStr + ": ";
                      rspfString valueStr = metaInfos[1];
                      m_globalMeta[keyStr] = valueStr;
                      m_globalMetaVector.push_back(rspfString(keyStr + valueStr));
                   }
                }
             }
          }
       }
       GDALClose(dataset);
       result = true;
     }
   }
   if (traceDebug())
   {    
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " exit status = " << (result?"true\n":"false\n");
   }
   return result;
}
std::ostream& rspfHdfInfo::print(std::ostream& out) const
{
   static const char MODULE[] = "rspfHdfInfo::print";
   if (traceDebug())
   {    
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }
   rspfString prefix = rspfString(m_driverName + ".").downcase();
   for (rspf_uint32 i = 0; i < m_globalMetaVector.size(); i++)
   {
       out << prefix << m_globalMetaVector[i] << "\n";
   }
   rspf_uint32 entryNum = m_hdfReader->getNumberOfEntries();
   for (rspf_uint32 i = 0; i < entryNum; i++)
   {
      rspfString imagePrefix = "image" + rspfString::toString(i) + ".";
      rspfString fileName = m_hdfReader->getEntryString(i);
      if (!fileName.empty())
      {
         GDALDatasetH dataset = GDALOpen(fileName.c_str(), GA_ReadOnly);
         if (dataset != 0)
         {
            rspf_uint32 numOfBands = GDALGetRasterCount(dataset);
            for (rspf_uint32 j = 0; j < numOfBands; j++)
            {
               rspfString bandPrefix = "band" + rspfString::toString(j) + ".";
               rspfString prefixStr = prefix + imagePrefix + bandPrefix;
               rspfString nameStr = "name: ";
               rspfString subDatasetName = fileName;
               std::vector<rspfString> subFileList = fileName.split(":");
               if (subFileList.size() > 2)
               {
                  if (m_driverName == "hdf4")
                  {
                     subDatasetName = subFileList[1] + ":" + subFileList[subFileList.size() - 2] + ":" + subFileList[subFileList.size() - 1];
                  }
                  else if (m_driverName == "hdf5")
                  {
                     subDatasetName = subFileList[subFileList.size() - 1];
                  }
               }
               out << prefixStr << nameStr << subDatasetName << "\n";
               char** papszMetadata = GDALGetMetadata(dataset, NULL);
               if( CSLCount(papszMetadata) > 0 )
               {
                  for(rspf_uint32 metaIndex = 0; papszMetadata[metaIndex] != 0; ++metaIndex)
                  {
                     rspfString metaInfo = papszMetadata[metaIndex];
                     if (metaInfo.contains("="))
                     {
                        std::vector<rspfString> metaInfos = metaInfo.split("=");
                        if (metaInfos.size() > 1)
                        {
                           rspfString key = metaInfos[0];
                           rspfString keyStr = key.substitute(":", ".", true);
                           keyStr = keyStr + ": ";
                           rspfString valueStr = metaInfos[1];
                           std::map<rspfString, rspfString>::const_iterator itSub = m_globalMeta.find(keyStr);
                           if (itSub == m_globalMeta.end())//avoid to print global again
                           {
                              out << prefixStr << keyStr << valueStr << "\n";
                           } 
                        }
                     }
                  }
               }
            }
            GDALClose(dataset);
         }//end if (dataset != 0)
      }//end if (!fileName.empty())
   }//end for
   
   if (traceDebug())
   {    
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " exited...\n";
   }
   return out;
}
