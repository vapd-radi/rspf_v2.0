//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts (gpotts@imagelinks.com)
//
// Description: 
//
//*******************************************************************
//  $Id: rspfMultiResLevelHistogram.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfMultiResLevelHistogram_HEADER
#define rspfMultiResLevelHistogram_HEADER
#include <vector>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfReferenced.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfHistogram.h>
#include <rspf/base/rspfMultiBandHistogram.h>

class RSPFDLLEXPORT rspfMultiResLevelHistogram : public rspfReferenced
{
public:
   rspfMultiResLevelHistogram();
   rspfMultiResLevelHistogram(rspf_uint32 numberOfResLevels);
   rspfMultiResLevelHistogram(const rspfMultiResLevelHistogram& rhs);
   

   rspfRefPtr<rspfHistogram> getHistogram(rspf_uint32 band,
                                            rspf_uint32 resLevel=0);
   const rspfRefPtr<rspfHistogram> getHistogram(rspf_uint32 band,
                                                  rspf_uint32 resLevel=0)const;
   rspf_uint32 getNumberOfResLevels()const;
   rspf_uint32 getNumberOfBands(rspf_uint32 resLevel=0) const;
   void create(rspf_uint32 numberOfResLevels);
   rspfRefPtr<rspfMultiBandHistogram> getMultiBandHistogram(rspf_uint32 resLevel) const;
   rspfRefPtr<rspfMultiResLevelHistogram> createAccumulationLessThanEqual()const;
   rspfRefPtr<rspfMultiResLevelHistogram> createAccumulationGreaterThanEqual()const;

   void setBinCount(double binNumber, double count);
   /*!
    * Will append to the list the passed in histogram.
    */
   virtual void addHistogram(rspfMultiBandHistogram* histo);

   /*!
    * This will create and append a histogram to the
    * list and return a pointer to the new histogram.
    */
   virtual rspfRefPtr<rspfMultiBandHistogram> addHistogram();

   /*!
    * Will set the histogram to the passed in res level.
    * It will delete the histogram at the res level before
    * setting the pointer to the passed in histogram.  if
    * the resLevel is out of range it will return false meaning
    * it was unable to assign the pointer.
    */
   virtual bool setHistogram(rspfRefPtr<rspfMultiBandHistogram> histo, rspf_uint32 resLevel);

   virtual rspfFilename getHistogramFile() const;
   virtual bool importHistogram(const rspfFilename& inputFile);
   virtual bool importHistogram(std::istream& in);

   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   
protected:
   virtual ~rspfMultiResLevelHistogram();
  class  rspfProprietaryHeaderInformation
   {
   public:
      rspfProprietaryHeaderInformation(){clear();}

      bool parseStream(std::istream& in);

      rspf_uint32 getNumberOfResLevels()
         {
            return theNumberOfResLevels.toUInt32();
         }
      void clear()
         {
            theFileType          = "";
            theVersion           = "";
            theCreatorId         = "";
            theNumberOfResLevels = "";
         }
      rspfString theFileType;
      rspfString theVersion;
      rspfString theCreatorId;
      rspfString theNumberOfResLevels;
   };

   std::vector<rspfRefPtr<rspfMultiBandHistogram> > theHistogramList;
   rspfFilename                    theHistogramFile;
   
   void deleteHistograms();
   bool parseProprietaryHistogram(std::istream& in);
};

#endif
