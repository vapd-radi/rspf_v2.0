//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts (gpotts@imagelinks.com)
//
// Description: 
//
//*******************************************************************
//  $Id: rspfMultiBandHistogram.h 17205 2010-04-24 18:10:01Z dburken $
#ifndef rspfMultiBandHistogram_HEADER
#define rspfMultiBandHistogram_HEADER
#include <vector>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfHistogram.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfXmlNode.h>
#include <rspf/base/rspfReferenced.h>

class rspfKeywordlist;
class rspfImageSource;

class RSPFDLLEXPORT rspfMultiBandHistogram : public rspfReferenced
{
public:
   rspfMultiBandHistogram();
   rspfMultiBandHistogram(const rspfMultiBandHistogram& rhs);
   rspfMultiBandHistogram(rspf_int32 numberOfBands,
                           rspf_int32 numberOfBuckets,
                           float minValue,
                           float maxValue);

   void create(const rspfImageSource* input);
   
   void create(rspf_int32 numberOfBands,
               rspf_int32 numberOfBuckets,
               float minValue,
               float maxValue);
   
   rspf_uint32 getNumberOfBands() const;

   void create(rspf_int32 numberOfBands);
   void setBinCount(double binNumber, double count);
   rspfRefPtr<rspfHistogram> getHistogram(rspf_int32 band);
   const rspfRefPtr<rspfHistogram> getHistogram(rspf_int32 band)const;

   rspfRefPtr<rspfMultiBandHistogram> createAccumulationLessThanEqual()const;
   rspfRefPtr<rspfMultiBandHistogram> createAccumulationGreaterThanEqual()const;
   /*!
    * Imports a text file that has histogram data
    */
   virtual bool importHistogram(const rspfFilename& inputFile);
   virtual bool importHistogram(std::istream& in);
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix = 0);
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix = 0)const;
   virtual bool saveState(rspfRefPtr<rspfXmlNode> xmlNode)const;
   virtual bool loadState(const rspfRefPtr<rspfXmlNode> xmlNode);
   
protected:
   virtual ~rspfMultiBandHistogram();
   class  rspfProprietaryHeaderInformation
   {
   public:
      rspfProprietaryHeaderInformation(){clear();}

      bool parseStream(std::istream& in);

      rspf_uint32 getNumberOfBands() const;

      void clear();

      rspfString theFileType;
      rspfString theVersion;
      rspfString theNumberOfBands;
      
   };
   /*!
    * Holds the histograms for each band
    */
   std::vector<rspfRefPtr<rspfHistogram> > theHistogramList;

   void deleteHistograms();

   bool parseProprietaryHistogram(std::istream& in);
                             
};

#endif
