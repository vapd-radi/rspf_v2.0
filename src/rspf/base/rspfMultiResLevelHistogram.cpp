//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
//
// Description: 
//
//*******************************************************************
//  $Id: rspfMultiResLevelHistogram.cpp 20610 2012-02-27 12:19:25Z gpotts $
#include <rspf/base/rspfMultiResLevelHistogram.h>
#include <rspf/base/rspfMultiBandHistogram.h>
#include <rspf/base/rspfKeyword.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <fstream>
using namespace std;

static const rspfKeyword NUMBER_OF_RES_LEVELS("number_of_res_levels",
                                               "");

rspfMultiResLevelHistogram::rspfMultiResLevelHistogram()
   :
      theHistogramList(),
      theHistogramFile()
{  
}

rspfMultiResLevelHistogram::rspfMultiResLevelHistogram(rspf_uint32 numberOfResLevels)
   :
      theHistogramList(),
      theHistogramFile()
{
   create(numberOfResLevels);
}

rspfMultiResLevelHistogram::rspfMultiResLevelHistogram(const rspfMultiResLevelHistogram& rhs)
   :
      theHistogramList(),
      theHistogramFile()
{
   
   create(rhs.getNumberOfResLevels());
   for(rspf_uint32 i = 0; i < theHistogramList.size(); ++i)
   {
      theHistogramList[i] = rhs.theHistogramList[i].valid()?
                            new rspfMultiBandHistogram(*rhs.theHistogramList[i]):
                            (rspfMultiBandHistogram*)NULL;
   }
}

rspfMultiResLevelHistogram::~rspfMultiResLevelHistogram()
{
   deleteHistograms();
}

void rspfMultiResLevelHistogram::create(rspf_uint32 numberOfResLevels)
{
   deleteHistograms();

   for(rspf_uint32 idx = 0; idx < numberOfResLevels; ++idx)
   {
      theHistogramList.push_back(new rspfMultiBandHistogram);      
   }
}

rspfRefPtr<rspfMultiResLevelHistogram> rspfMultiResLevelHistogram::createAccumulationLessThanEqual()const
{
   rspfRefPtr<rspfMultiResLevelHistogram> result = 0;

   if(theHistogramList.size() > 0)
   {
      result = new rspfMultiResLevelHistogram((rspf_uint32)theHistogramList.size());

      for(rspf_uint32 idx=0; idx < (rspf_uint32) theHistogramList.size(); ++ idx)
      {
         if(theHistogramList[idx].valid())
         {
            rspfRefPtr<rspfMultiBandHistogram> multiBandAccumulator = theHistogramList[idx]->createAccumulationLessThanEqual();
            result->theHistogramList[idx] = multiBandAccumulator;
         }
         else
         {
            // we will push on a null since someone could
            // turn off a band.  A null accumulation will
            // indicate no histogram data.
            //
            result->theHistogramList[idx] = 0;
         }
      }
   }
   
   return result;
}

void rspfMultiResLevelHistogram::setBinCount(double binNumber, double count)
{
   if(theHistogramList.size() > 0)
   {
      for(rspf_uint32 idx=0; idx < (rspf_uint32) theHistogramList.size(); ++ idx)
      {
         if(theHistogramList[idx].valid())
         {
            theHistogramList[idx]->setBinCount(binNumber, count);
         }
      }
   }   
}

rspfRefPtr<rspfMultiResLevelHistogram> rspfMultiResLevelHistogram::createAccumulationGreaterThanEqual()const
{
   rspfRefPtr<rspfMultiResLevelHistogram> result = NULL;

   if(theHistogramList.size() > 0)
   {
      result = new rspfMultiResLevelHistogram((rspf_uint32)theHistogramList.size());

      for(rspf_uint32 idx=0; idx < (rspf_uint32) theHistogramList.size(); ++ idx)
      {
         if(theHistogramList[idx].valid())
         {
            rspfRefPtr<rspfMultiBandHistogram> multiBandAccumulator = theHistogramList[idx]->createAccumulationGreaterThanEqual();
            result->theHistogramList[idx]=multiBandAccumulator;
         }
         else
         {
            // we will push on a null since someone could
            // turn off a band.  A null accumulation will
            // indicate no histogram data.
            //
            result->theHistogramList[idx] = 0;
         }
      }
      result = 0;
   }
   
   return result;
}

void rspfMultiResLevelHistogram::addHistogram(rspfMultiBandHistogram* histo)
{
   theHistogramList.push_back(histo);
}

rspfRefPtr<rspfMultiBandHistogram> rspfMultiResLevelHistogram::addHistogram()
{
   rspfRefPtr<rspfMultiBandHistogram> result = new rspfMultiBandHistogram;
   theHistogramList.push_back(result);
   return result;
}

bool rspfMultiResLevelHistogram::setHistogram(rspfRefPtr<rspfMultiBandHistogram> histo, 
                                               rspf_uint32 resLevel)
{
   if(resLevel < getNumberOfResLevels())
   {
      theHistogramList[resLevel] = histo;
      return true;
   }
   return false;
}

void rspfMultiResLevelHistogram::deleteHistograms()
{
   for(rspf_uint32 idx = 0; idx < (rspf_uint32)theHistogramList.size(); ++idx)
   {
      if(theHistogramList[idx].valid())
      {
         theHistogramList[idx] = NULL;
      }
   }
   
   theHistogramList.clear();
}

rspfRefPtr<rspfHistogram> rspfMultiResLevelHistogram::getHistogram(rspf_uint32 band,
                                                          rspf_uint32 resLevel)
{
   rspfRefPtr<rspfMultiBandHistogram> temp = getMultiBandHistogram(resLevel);

   if(temp.valid())
   {
      return temp->getHistogram(band);
   }
   
   return (rspfHistogram*)0;
}
const rspfRefPtr<rspfHistogram> rspfMultiResLevelHistogram::getHistogram(rspf_uint32 band,
                                                                            rspf_uint32 resLevel)const
{
   const rspfRefPtr<rspfMultiBandHistogram> temp = getMultiBandHistogram(resLevel);

   if(temp.valid())
   {
      return temp->getHistogram(band);
   }
   
   return (rspfHistogram*)0;
}

rspf_uint32 rspfMultiResLevelHistogram::getNumberOfResLevels()const
{
   return (rspf_uint32)theHistogramList.size();
}

rspf_uint32 rspfMultiResLevelHistogram::getNumberOfBands(rspf_uint32 resLevel) const
{
   const rspfRefPtr<rspfMultiBandHistogram> h = getMultiBandHistogram(resLevel);
   if (h.valid())
   {
      return h->getNumberOfBands();
   }

   return 0;
}  

rspfRefPtr<rspfMultiBandHistogram> rspfMultiResLevelHistogram::getMultiBandHistogram(rspf_uint32 resLevel) const
{
   if(resLevel < (rspf_uint32)theHistogramList.size())
   {
      return theHistogramList[resLevel];
   }

   return NULL;
}

bool rspfMultiResLevelHistogram::importHistogram(std::istream& in)
{
   if (!in) // Check stream state.
   {
      return false;
   }
   
   rspfString buffer;
   getline(in, buffer);

   if ( in.eof() ) // Zero byte files will hit here.
   {
      return false;
   }

   // check to see if it is a proprietary histogram file
   // 
   if((buffer =="") || (buffer.c_str()[0] != 'F' ||
      buffer.c_str()[1] != 'i'))
   {
      in.seekg(0, ios::beg);
      rspfKeywordlist kwl;
      if (kwl.parseStream(in) == true)
      {
         return loadState(kwl);
      }
      else
      {
         return false;
      }
   }
   
   rspfProprietaryHeaderInformation header;
   in.seekg(0, ios::beg);
   deleteHistograms();
   if(header.parseStream(in))
   {
      rspf_uint32 numberOfResLevels = header.getNumberOfResLevels();
      
      if(numberOfResLevels)
      {
         theHistogramList.resize(numberOfResLevels);

         for(rspf_uint32 counter = 0; counter < (rspf_uint32)theHistogramList.size(); ++counter)
         {
            theHistogramList[counter] = NULL;
         }
         rspfString reslevelBuffer;
         rspfString buffer;
         
         for(rspf_uint32 idx = 0; idx < numberOfResLevels; ++idx)
         {
            getline(in, buffer);
            if(buffer.find("RR Level") != string::npos)
            {
               std::string::size_type offset = buffer.find(":");
               if(offset != string::npos)
               {
                  reslevelBuffer = buffer.substr(offset+1);
               }
               else
               {
                  deleteHistograms();
                  return false;
               }
            }
            else
            {
               deleteHistograms();
               return false;
            }
            rspf_uint32 resLevelIdx = reslevelBuffer.toUInt32();

            if(resLevelIdx < (rspf_uint32)theHistogramList.size())
            {
               if(!theHistogramList[resLevelIdx])
               {
                  rspfRefPtr<rspfMultiBandHistogram> histogram = new rspfMultiBandHistogram;
                  if(histogram->importHistogram(in))
                  {
                     theHistogramList[resLevelIdx] = histogram;
                  }
                  else
                  {
                     deleteHistograms();
                     return false;
                  }
               }
            }
            else
            {
               deleteHistograms();
               return false;
            }
            rspfString skipDot;
            getline(in, skipDot);
         }
      }
      else
      {
         return false;
      }
   }  

   return true;
}

bool rspfMultiResLevelHistogram::importHistogram(const rspfFilename& file)
{
   if( file.fileSize() > 0 )
   {
      theHistogramFile = file;
      
      ifstream input(file.c_str());
      return importHistogram(input);
   }
   return false;
}

bool rspfMultiResLevelHistogram::rspfProprietaryHeaderInformation::parseStream(std::istream& in)
{
   rspfString inputLine;

   getline(in, inputLine);  
   if(inputLine.find("File Type") != string::npos)
   {
      std::string::size_type idx = inputLine.find(":");
      if(idx != string::npos)
      {
         theFileType = inputLine.substr(idx+1);
         theFileType = theFileType.trim();
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

   getline(in, inputLine);  
   if(inputLine.find("Version") != string::npos)
   {
      std::string::size_type idx = inputLine.find(":");
      if(idx != string::npos)
      {
         theVersion = inputLine.substr(idx+1);
         theVersion = theVersion.trim();
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

   getline(in, inputLine);  
   if(inputLine.find("Creator ID") != string::npos)
   {
      std::string::size_type idx = inputLine.find(":");
      if(idx != string::npos)
      {
         theCreatorId = inputLine.substr(idx+1);
         theCreatorId = theCreatorId.trim();
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
   
   getline(in, inputLine);  
   if(inputLine.find("RR Levels") != string::npos)
   {
      std::string::size_type idx = inputLine.find(":");
      if(idx != string::npos)
      {
         theNumberOfResLevels = inputLine.substr(idx+1);
         theNumberOfResLevels = theNumberOfResLevels.trim();
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

bool rspfMultiResLevelHistogram::saveState(rspfKeywordlist& kwl,
                                            const char* prefix)const
{
   bool result = true;
   if(theHistogramList.size() > 0)
   {
      rspfString s1 = prefix;
      kwl.add(prefix,
              rspfKeywordNames::TYPE_KW,
              "rspfMultiResLevelHistogram",
              true);
      kwl.add(prefix,
              NUMBER_OF_RES_LEVELS.key(),
              static_cast<rspf_uint32>(theHistogramList.size()),
              true);

      for(rspf_uint32 idx = 0; idx < theHistogramList.size(); ++idx)
      {
         rspfString rr_level = rspfString(prefix) + "rr_level";
         rr_level += rspfString::toString(idx)   + ".";
         if(theHistogramList[idx].valid())
         {
            result = theHistogramList[idx]->saveState(kwl, rr_level.c_str());
            
            if(!result)
            {
               return result;
            }
         }
      }    
   }

   return result;
}

bool rspfMultiResLevelHistogram::loadState(const rspfKeywordlist& kwl,
                                            const char* prefix)
{
   deleteHistograms();
   const char* number_of_res_levels = kwl.find(prefix, NUMBER_OF_RES_LEVELS.key());

   if(number_of_res_levels)
   {
      rspf_uint32 numberOfResLevels = rspfString(number_of_res_levels).toUInt32();

      if(numberOfResLevels)
      {
         rspfString newPrefix;
         for(rspf_uint32 idx = 0; idx < numberOfResLevels; ++idx)
         {
            rspfRefPtr<rspfMultiBandHistogram> histo = new rspfMultiBandHistogram;

            if (prefix) newPrefix = prefix;
            
            newPrefix += "rr_level";
            newPrefix += rspfString::toString(idx);
            newPrefix += ".";
            
            histo->loadState(kwl, newPrefix.c_str());
            
            theHistogramList.push_back(histo);
         }
      }
   }
   return true;
}

rspfFilename rspfMultiResLevelHistogram::getHistogramFile() const
{
   return theHistogramFile;
}
