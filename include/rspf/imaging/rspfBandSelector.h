//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  See top level LICENSE.txt.
// 
// Author:  Garrett Potts
//
// Description:  Contains class declaration for rspfBandSelector.
// 
//*******************************************************************
//  $Id: rspfBandSelector.h 21631 2012-09-06 18:10:55Z dburken $
#ifndef rspfBandSelector_HEADER
#define rspfBandSelector_HEADER 1


#include <rspf/imaging/rspfImageSourceFilter.h>
#include <vector>
 
class rspfImageData;
class rspfImageHandler;

class RSPF_DLL rspfBandSelector : public rspfImageSourceFilter
{
   
public:
   enum rspfBandSelectorWithinRangeFlagState
   {
      rspfBandSelectorWithinRangeFlagState_NOT_SET      = 0,
      rspfBandSelectorWithinRangeFlagState_IN_RANGE     = 1,
      rspfBandSelectorWithinRangeFlagState_OUT_OF_RANGE = 2,

   };

   rspfBandSelector();
   virtual ~rspfBandSelector();
   
   
   virtual rspfString getLongName()  const;
   virtual rspfString getShortName() const;

   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                               rspf_uint32 resLevel=0);

   /**
    * Initializes bandList to the zero based order of output bands.
    */
   virtual void getOutputBandList(std::vector<rspf_uint32>& bandList) const;

   /**
    * Sets the output band list.
    *
    * @param outputBandList List of zero based bands.
    *
    * @note The input connection should be established prior to calling this.
    *
    * @note Bands can be duplicated.  So if you have a single band input you
    * can do a band list like "1, 1, 1".   Any band within the list must be
    * available from the input.  So if you have a 3 band input bands must be
    * constrained to zero based bands "0, 1, or 2".
    *
    * @note Calling this method does not enable filter so callers should do:
    * theBandSelector->enableSource();
    * theBandSelector->setOutputBandList(bandList);
    */
   virtual void setOutputBandList(const vector<rspf_uint32>& outputBandList);
   
   /**
    * Returns the number of bands in a tile returned from this TileSource.
    */
   virtual rspf_uint32 getNumberOfOutputBands() const;

   /**
    * @brief Returns the number of input bands.
    *
    * Overrides rspfImageSourceFilter::getNumberOfInputBands to check for a
    * single image chain band selectable image handler on the input.  If one
    * is present it returns it's number of input bands instead of the number
    * of the input connection's output bands(our input).  This is needed so
    * callers, e.g. band selector dialog box can query the number of bands
    * available.
    *
    * @return Number of bands available.
    */
   virtual rspf_uint32 getNumberOfInputBands()const;   

   virtual void initialize();
   
   virtual double getMinPixelValue(rspf_uint32 band=0)const;
   virtual double getMaxPixelValue(rspf_uint32 band=0)const;
   virtual double getNullPixelValue(rspf_uint32 band=0)const;
   

   /**
    * Method to save the state of an object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=NULL)const;
   
   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=NULL);
   /**
    *   Override base class so that a disableSource event does not
    *   reinitialize the object and enable itself.
    */
   //virtual void propertyEvent(rspfPropertyEvent& event);
   //virtual void refreshEvent(rspfRefreshEvent& event);   

   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;						  
   virtual bool isSourceEnabled()const;

protected:

   /**
    * Called on first getTile, will initialize all data needed.
    */
   void allocate();

   /**
    * Will check all combinations of the band list to see if its 
    */
   void checkPassThrough();

   /**
    * @return true if all bands in theOutputBandList are less than the
    * number of input's bands.
    */
   bool outputBandsWithinInputRange() const;

   /**
    * @brief Finds and returns band selectable image handler.
    *
    * To get a valid pointer there must be one and only one image handler on
    * the input connection and it must be a band selector.  In other words,
    * must be a single image chain input and
    * rspfImageHandler::isBandSelector() must return true.
    *
    * @return Pointer to image handler wrapped in an rspfRefPtr on success;
    * null, on error.
    */
   rspfRefPtr<rspfImageHandler> getBandSelectableImageHandler() const;
   
   
   rspfRefPtr<rspfImageData>           theTile;
   std::vector<rspf_uint32>             theOutputBandList;
   rspfBandSelectorWithinRangeFlagState theWithinRangeFlag;
   bool                                  thePassThroughFlag;

TYPE_DATA
};

#endif /* #ifndef rspfBandSelector_HEADER */
