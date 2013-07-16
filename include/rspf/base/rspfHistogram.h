//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Ken Melero 
//         Orginally developed by:
//                   Copyright (c) 1997 TargetJr Consortium
//               GE Corporate Research and Development (GE CRD)
//                             1 Research Circle
//                            Niskayuna, NY 12309
//         Adapted from:  IUE v4.1.2
// Description: 
//      A Histogram contains an array of "buckets", which represent finite
// segments of some value axis, along with a corresponding array of
// frequency counts for each of these buckets.
//
//********************************************************************
// $Id: rspfHistogram.h 19799 2011-06-30 18:41:26Z gpotts $
//

#ifndef rspfHistogram_HEADER
#define rspfHistogram_HEADER
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfXmlNode.h>
class RSPFDLLEXPORT rspfHistogram : public rspfObject
{
  private:

    mutable int stats_consistent; // A 2 bit state flag  Mean =1 | StandDev = 2

  protected:

   
   virtual void deleteAll();
   
    float * vals;            // histogram x array
                             // (value = midpoint of each bucket)
    float * counts;          // histogram y array ie. count[i] is
                             // the number of pixels with value within range
                             // of bucket i

    int num;                 // number of indices

    float delta;             // "Width" of each bucket on value axis
    float vmin, vmax;        // Maximum and minimum values on plot
    mutable float mean;               // Mean value of the distribution
    mutable float standard_dev;       // 
  protected:


   class rspfProprietaryHeaderInformation
   {
   public:
      rspfProprietaryHeaderInformation(){clear();}

      bool parseStream(istream& in);

      long getNumberOfBins()
         {
            return theNumberOfBins.toLong();
         }
      void clear()
         {
            theFileType      = "";
            theVersion       = "";
            theMapperType    = "";
            theNumberOfBins  = "";
         }
      rspfString theFileType;
      rspfString theVersion;
      rspfString theMapperType;
      rspfString theNumberOfBins;
   };
   
  public:
   enum FillAlgorithmType
   {
      HISTOGRAM_FILL_DEFAULT    = 0,
      HISTOGRAM_FILL_THIN_PLATE = 1
   };
// Constructors
    rspfHistogram();
    rspfHistogram(int xres, float min, float max);
    rspfHistogram(float*, float*, int);
    rspfHistogram(const rspfHistogram& his); // Copy constructor
    rspfHistogram(const rspfHistogram*, float width); // Resampling constructor

    virtual int GetIndex(float)const;
// Other histogram formation operations    
   rspfHistogram* fillInteriorEmptyBins(int type=HISTOGRAM_FILL_THIN_PLATE)const;
    rspfHistogram* Scale(float scale_factor); // Scale Transformation
    rspfHistogram* CumulativeGreaterThanEqual()const;// From density to cumulative
    rspfHistogram* CumulativeLessThanEqual()const;// From density to cumulative
    //Suppress non-peak values.
   rspfHistogram* NonMaximumSupress(int radius = 1, bool cyclic = false);
   void create(int xres, float val1, float val2);
  
// Attribute accessors
    void UpCount(float newval);
    float GetCount(float uval)const;
    float SetCount(float pixelval, float count);

    float GetMinVal()const;
    float GetMaxVal()const;
    float GetMaxCount()const;

    float GetRangeMin()const
    {
       return vmin;
    }
    float GetRangeMax()const
    {
       return vmax;
    }
    float * GetVals()
    {
	stats_consistent = 0; // Values might change.
	return vals; 
    }
    const float * GetVals()const
    {
	stats_consistent = 0; // Values might change.
	return vals; 
    }

    float * GetCounts()
    { 
	stats_consistent = 0; // Counts might change.
	return counts; 
    }

   const float * GetCounts()const
    { 
	stats_consistent = 0; // Counts might change.
	return counts; 
    }

    int GetRes()const
    { return num; }

    float GetBucketSize()const { return delta; }

    float * GetMinValAddr()
    { return vals+GetIndex(GetMinVal());  }

    float * GetMinCountAddr()
    { return counts+GetIndex(GetMinVal());  }

   const float * GetMinValAddr()const
    { return vals+GetIndex(GetMinVal());  }

    const float * GetMinCountAddr()const
    { return counts+GetIndex(GetMinVal());  }

    float ComputeArea(float low, float high)const;// bounded area
    float ComputeArea()const;//total area

    /*!
     * Returns the fraction of accumulation up to and including "val" bucket
     * from min divided by the total accumulation.
     * returns RSPF_FLT_NAN if "val" is not between GetMinVal and GetMaxVal.
     */
    float getLowFractionFromValue(float val) const;
    
    /*!
     * Returns the fraction of accumulation down to and including "val" bucket
     * from max divided by the total accumulation.
     * returns RSPF_FLT_NAN if "val" is not between GetMin() and GetMax().
     */
    float getHighFractionFromValue(float val) const;
    
    //Find bounds that clip off a given percent of the area
    float LowClipVal(float clip_fraction)const;
    float HighClipVal(float clip_fraction)const;

    int GetValIndex(float val)const;

    float GetMean()const;
    float GetStandardDev()const;

    void Print()const;
    void Dump(char *)const;
    int  WritePlot(const char* fname)const;
   virtual ~rspfHistogram();


   virtual bool importHistogram(const rspfFilename& inputFile);
   virtual bool importHistogram(istream& in);
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   virtual bool saveState(rspfRefPtr<rspfXmlNode> xmlNode)const;
   virtual bool loadState(const rspfRefPtr<rspfXmlNode> xmlNode);
TYPE_DATA   
};

#endif

