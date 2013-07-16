//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
//
// Description: 
//
//*******************************************************************
//  $Id: rspfMultiBandHistogram.cpp 21631 2012-09-06 18:10:55Z dburken $

#include <rspf/base/rspfMultiBandHistogram.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/imaging/rspfImageSource.h>
#include <fstream>

rspfMultiBandHistogram::rspfMultiBandHistogram()
{  
}

rspfMultiBandHistogram::~rspfMultiBandHistogram()
{
   deleteHistograms();
}

rspfMultiBandHistogram::rspfMultiBandHistogram(const rspfMultiBandHistogram& rhs)
{
   theHistogramList.resize(rhs.theHistogramList.size());

   for(rspf_uint32 i = 0; i < theHistogramList.size(); ++i)
   {
      theHistogramList[i] =  rhs.theHistogramList[i].valid()?
         new rspfHistogram(*rhs.theHistogramList[i].get()):(rspfHistogram*)0;
   }
}

rspfMultiBandHistogram::rspfMultiBandHistogram(rspf_int32 numberOfBands,
                                                 rspf_int32 numberOfBuckets,
                                                 float minValue,
                                                 float maxValue)
{
   if(numberOfBands > 0)
   {
      create(numberOfBands, numberOfBuckets, minValue, maxValue);
   }
}

void rspfMultiBandHistogram::create(const rspfImageSource* input)
{
   if (input)
   {
      rspf_uint32 bands = input->getNumberOfOutputBands();;
      rspf_uint32 numberOfBins = 0;
      rspf_float64 minValue = 0.0;
      rspf_float64 maxValue = 0.0;
      
      switch(input->getOutputScalarType())
      {
         case RSPF_UINT8:
         {
            minValue     = 0;
            maxValue     = RSPF_DEFAULT_MAX_PIX_UCHAR;
            numberOfBins = 256;
            break;
         }
         case RSPF_USHORT11:
         {
            minValue     = 0;
            maxValue     = RSPF_DEFAULT_MAX_PIX_UINT11;
            numberOfBins = RSPF_DEFAULT_MAX_PIX_UINT11 + 1;
            break;
         }
         case RSPF_UINT16:
         case RSPF_UINT32:
         {
            minValue     = 0;
            maxValue     = RSPF_DEFAULT_MAX_PIX_UINT16;
            numberOfBins = RSPF_DEFAULT_MAX_PIX_UINT16 + 1;
            break;
         }
         case RSPF_SINT16:
         case RSPF_SINT32:
         case RSPF_FLOAT32:
         case RSPF_FLOAT64:
         {
            minValue     = RSPF_DEFAULT_MIN_PIX_SINT16;
            maxValue     = RSPF_DEFAULT_MAX_PIX_SINT16;
            numberOfBins = (RSPF_DEFAULT_MAX_PIX_SINT16-RSPF_DEFAULT_MIN_PIX_SINT16) + 1;
            break;
         }
         case RSPF_NORMALIZED_FLOAT:
         case RSPF_NORMALIZED_DOUBLE:
         {
            minValue     = 0;
            maxValue     = 1.0;
            numberOfBins = RSPF_DEFAULT_MAX_PIX_UINT16+1;
            break;
         }
         default:
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "Unsupported scalar type in rspfMultiBandHistogram::create()"
               << std::endl;
            return;
         }
         
      }  // switch(input->getOutputScalarType())

      create(bands, numberOfBins, minValue, maxValue);
      
   } // if (input)
}

void rspfMultiBandHistogram::create(rspf_int32 numberOfBands,
                                     rspf_int32 numberOfBuckets,
                                     float minValue,
                                     float maxValue)
{
   // make sure we clear our internal lists before
   // we start.
   //
   deleteHistograms();

   if(numberOfBands > 0)
   {
      numberOfBuckets = numberOfBuckets>0?numberOfBuckets:1;

      for(rspf_int32 bands = 0; bands < numberOfBands; ++bands)
      {
         theHistogramList.push_back(new rspfHistogram(numberOfBuckets,
                                                       minValue,
                                                       maxValue));
      }
   }
}

void rspfMultiBandHistogram::create(rspf_int32 numberOfBands)
{
   deleteHistograms();
   for(rspf_int32 bands = 0; bands < numberOfBands; ++bands)
   {
      theHistogramList.push_back(new rspfHistogram);
   }
}

rspf_uint32 rspfMultiBandHistogram::getNumberOfBands() const
{
   return (rspf_uint32)theHistogramList.size();
}

rspfRefPtr<rspfHistogram> rspfMultiBandHistogram::getHistogram(rspf_int32 band)
{
   if((band >=0) && (band < (rspf_int32)theHistogramList.size()))
   {
      return theHistogramList[band];
   }

   return NULL;
}

const rspfRefPtr<rspfHistogram> rspfMultiBandHistogram::getHistogram(rspf_int32 band)const
{
   if((band >=0) && (band < (rspf_int32)theHistogramList.size()))
   {
      return theHistogramList[band];
   }

   return 0;
}

void rspfMultiBandHistogram::setBinCount(double binNumber, double count)
{
   if(theHistogramList.size() > 0)
   {
      for(rspf_uint32 idx = 0; idx < theHistogramList.size(); ++idx)
      {
         if(theHistogramList[idx].valid())
         {
            theHistogramList[idx]->SetCount(binNumber, count);
         }
      }
   }   
}

rspfRefPtr<rspfMultiBandHistogram> rspfMultiBandHistogram::createAccumulationLessThanEqual()const
{
   rspfRefPtr<rspfMultiBandHistogram> result = NULL;

   if(theHistogramList.size() > 0)
   {
      result = new rspfMultiBandHistogram;
      result->theHistogramList.resize(theHistogramList.size());
      
      for(rspf_uint32 idx = 0; idx < theHistogramList.size(); ++idx)
      {
         if(theHistogramList[idx].valid())
         {
            result->theHistogramList[idx] = theHistogramList[idx]->CumulativeLessThanEqual();
         }
         else
         {
            result->theHistogramList[idx] = 0;
         }     
      }
   }

   return result;
}

rspfRefPtr<rspfMultiBandHistogram> rspfMultiBandHistogram::createAccumulationGreaterThanEqual()const
{
   rspfRefPtr<rspfMultiBandHistogram> result = NULL;

   if(theHistogramList.size() > 0)
   {
      result = new rspfMultiBandHistogram;
      
      for(rspf_uint32 idx = 0; idx < theHistogramList.size(); ++idx)
      {
         if(theHistogramList[idx].valid())
         {
            result->theHistogramList[idx] = theHistogramList[idx]->CumulativeGreaterThanEqual();
         }
         else
         {
            result->theHistogramList[idx] = 0;
         }     
      }
   }

   return result;
}

void rspfMultiBandHistogram::deleteHistograms()
{
   theHistogramList.clear();
}


bool rspfMultiBandHistogram::importHistogram(std::istream& in)
{
   rspfProprietaryHeaderInformation header;
   deleteHistograms();
   
   if(header.parseStream(in))
   {
      rspf_int32 numberOfBands = header.getNumberOfBands();
      
      if(numberOfBands)
      {
         theHistogramList.resize(numberOfBands);

         for(rspf_int32 counter = 0; counter < (rspf_int32)theHistogramList.size(); ++counter)
         {
            theHistogramList[counter] = 0;
         }
         rspfString bandBuffer;
         rspfString buffer;
         
         for(rspf_int32 idx = 0; idx < numberOfBands; ++idx)
         {
            getline(in, buffer);
            if(buffer.find("Band") != string::npos)
            {
				string::size_type offset = buffer.find(":");
               if(offset != string::npos)
               {
                  bandBuffer = buffer.substr(offset+1);
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
            rspf_uint32 bandIdx = bandBuffer.toUInt32();

            if(bandIdx < theHistogramList.size())
            {
               if(!theHistogramList[bandIdx].valid())
               {
                  rspfRefPtr<rspfHistogram> histogram = new rspfHistogram;
                  if(histogram->importHistogram(in))
                  {
                     theHistogramList[bandIdx] = histogram;
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
         }
      }
      else
      {
         return false;
      }
   }  
   return true;
}

bool rspfMultiBandHistogram::importHistogram(const rspfFilename& file)
{
   if(file.exists())
   {
      std::ifstream input(file.c_str());

      return importHistogram(input);
   }

   return false;
}

bool rspfMultiBandHistogram::rspfProprietaryHeaderInformation::parseStream(std::istream& in)
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
   if(inputLine.find("Number of Bands") != string::npos)
   {
      std::string::size_type idx = inputLine.find(":");
      if(idx != string::npos)
      {
         theNumberOfBands = inputLine.substr(idx+1);
         theNumberOfBands = theNumberOfBands.trim();
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

rspf_uint32 rspfMultiBandHistogram::rspfProprietaryHeaderInformation::getNumberOfBands() const
{
   return theNumberOfBands.toUInt32();
}

void rspfMultiBandHistogram::rspfProprietaryHeaderInformation::clear()
{
   theFileType      = "";
   theVersion       = "";
   theNumberOfBands = "";
}

bool rspfMultiBandHistogram::saveState(rspfKeywordlist& kwl,
                                        const char* prefix)const
{
   kwl.add(prefix,
           "type",
           "rspfMultiBandHistogram",
           true);
   kwl.add(prefix,
           "number_of_bands",
           static_cast<rspf_uint32>(theHistogramList.size()),
           true);
   
   for(rspf_uint32 idx = 0; idx < theHistogramList.size(); ++idx)
   {
      rspfString band = rspfString(prefix) + "band";
      band += (rspfString::toString(idx) + ".");
      if(theHistogramList[idx].valid())
      {
         rspfString newPrefix = (rspfString(prefix) + rspfString::toString(idx) + ".");
         
         theHistogramList[idx]->saveState(kwl, band.c_str());
      }
   }
   
   return true;
}

bool rspfMultiBandHistogram::loadState(const rspfKeywordlist& kwl,
                                        const char* prefix)
{
   deleteHistograms();
   const char* number_of_bands = kwl.find(prefix, "number_of_bands");
   if(number_of_bands)
   {
      rspf_uint32 numberOfBands = rspfString(number_of_bands).toUInt32();

      if(numberOfBands>0)
      {
         rspfString newPrefix;
         for(rspf_uint32 idx = 0; idx < numberOfBands; ++idx)
         {
            rspfHistogram* histo = new rspfHistogram;

            newPrefix = prefix;
            newPrefix += "band";
            newPrefix += rspfString::toString(idx);
            newPrefix += ".";
            
            histo->loadState(kwl, newPrefix.c_str());
            
            theHistogramList.push_back(histo);
         }
      }
   }
   
   return true;
}

bool rspfMultiBandHistogram::saveState(rspfRefPtr<rspfXmlNode> xmlNode)const
{
   xmlNode->setTag("rspfMutliBandHistogram");
   for(rspf_uint32 idx = 0; idx < theHistogramList.size(); ++idx)
   {
      rspfRefPtr<rspfXmlNode> band = new rspfXmlNode;
      rspfRefPtr<rspfXmlNode> newNode = new rspfXmlNode;

      band->setTag("Band");
      band->addAttribute("idx", rspfString::toString(idx));
      band->addChildNode(newNode.get());
      if(theHistogramList[idx].valid())
      {         
         theHistogramList[idx]->saveState(newNode);
      }
      else
      {
         newNode->setTag("rspfHistogram");
      }
      xmlNode->addChildNode(band.get());
   }

   return true;
}

bool rspfMultiBandHistogram::loadState(const rspfRefPtr<rspfXmlNode> xmlNode)
{
   theHistogramList.clear();
   const vector<rspfRefPtr<rspfXmlNode> >& childNodes = xmlNode->getChildNodes();

   rspf_uint32 idx = 0;
   rspf_uint32 maxCount = (rspf_uint32)childNodes.size();
   for(idx = 0; idx < maxCount; ++idx)
   {
      if(childNodes[idx]->getTag() == "Band")
      {
         if(childNodes[idx]->getChildNodes().size())
         {
            rspfRefPtr<rspfHistogram> histo = new rspfHistogram;
            histo->loadState(childNodes[idx]->getChildNodes()[0]);
            theHistogramList.push_back(histo);
         }
      }
   }

   return true;
}
