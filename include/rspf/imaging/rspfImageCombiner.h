//*******************************************************************
//
// License:  See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfImageCombiner.h 17932 2010-08-19 20:34:35Z dburken $
#ifndef rspfImageCombiner_HEADER
#define rspfImageCombiner_HEADER
#include <vector>

#include <rspf/imaging/rspfImageSource.h>
#include <rspf/base/rspfConnectableObjectListener.h>
#include <rspf/base/rspfPropertyEvent.h>

/**
 * This will be a base for all combiners.  Combiners take N inputs and
 * will produce a single output.
 */
class RSPFDLLEXPORT rspfImageCombiner : public rspfImageSource,
                                          public rspfConnectableObjectListener
{
public:
   rspfImageCombiner();
   rspfImageCombiner(rspfObject* owner,
                      int numberOfInputs,
                      int numberOfOutputs,
                      bool inputListIsFixedFlag,
                      bool outputListIsFixedFlag);
   
   rspfImageCombiner(rspfConnectableObject::ConnectableObjectList& inputSources);

   virtual rspfIrect getBoundingRect(rspf_uint32 resLevel=0) const;

   virtual void initialize();
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=NULL);
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=NULL)const;

   /**
    * This will go through and precompute the
    * bounding rects of each input image.
    */
   virtual void updateRects();

   /**
    * Returns the number of bands available from the input.
    */
   virtual rspf_uint32 getNumberOfInputBands()const;   
   virtual rspfScalarType getOutputScalarType() const;
   virtual rspf_uint32 getTileWidth()const;  
   virtual rspf_uint32 getTileHeight()const;

   virtual double getNullPixelValue(rspf_uint32 band=0)const;
   virtual double getMinPixelValue(rspf_uint32 band=0)const;
   virtual double getMaxPixelValue(rspf_uint32 band=0)const;

  /**
   * Used to retrieve the number of overlapping images withint the given
   * rect.
   */
  virtual rspf_uint32 getNumberOfOverlappingImages(const rspfIrect& rect,
                                                    rspf_uint32 resLevel=0)const;

  /**
   * Used to populate the result with the index of the overlapping images.
   */
  virtual void getOverlappingImages(std::vector<rspf_uint32>& result,
				    const rspfIrect& rect,
                                    rspf_uint32 resLevel=0)const;
   
   
/*    virtual rspfRefPtr<rspfImageData> getNextTile(rspf_uint32& returnedIdx, */
/*                                                    rspf_uint32 startIdx, */
/*                                                    const rspfIpt& origin, */
/*                                                    rspf_uint32 resLevel=0); */
   
/*    virtual rspfRefPtr<rspfImageData> getNextTile(rspf_uint32& returnedIdx, */
/*                                                    const rspfIpt& origin, */
/*                                                    rspf_uint32 resLevel=0); */
   
   virtual rspfRefPtr<rspfImageData> getNextTile(rspf_uint32& returnedIdx,
                                                   const rspf_uint32 startIdx,
                                                   const rspfIrect& tileRect,
                                                   rspf_uint32 resLevel=0);

   virtual rspfRefPtr<rspfImageData> getNextTile(rspf_uint32& returnedIdx,
                                                   const rspfIrect& tileRect,
                                                   rspf_uint32 resLevel=0);

   
   virtual rspfRefPtr<rspfImageData> getNextNormTile(rspf_uint32& returnedIdx,
                                                       const rspf_uint32 index,
                                                       const rspfIrect& tileRect,
                                                       rspf_uint32 resLevel=0);
   
   virtual rspfRefPtr<rspfImageData> getNextNormTile(rspf_uint32& returnedIdx,
                                                       const rspfIrect& tileRect,
                                                       rspf_uint32 resLevel=0);
   
/*    virtual rspfRefPtr<rspfImageData> getNextNormTile(rspf_uint32& returnedIdx, */
/*                                                        rspf_uint32 index, */
/*                                                        const rspfIpt& origin, */
/*                                                        rspf_uint32 resLevel=0); */
/*    virtual rspfRefPtr<rspfImageData> getNextNormTile(rspf_uint32& returnedIdx, */
/*                                                        const rspfIpt& origin, */
/*                                                        rspf_uint32 resLevel=0); */
   
   
   virtual bool canConnectMyInputTo(rspf_int32 inputIndex,
                                    const rspfConnectableObject* object)const;
         
   virtual void connectInputEvent(rspfConnectionEvent& event);
   virtual void disconnectInputEvent(rspfConnectionEvent& event);
   virtual void propertyEvent(rspfPropertyEvent& event);
   virtual void refreshEvent(rspfRefreshEvent& event);
   virtual bool hasDifferentInputs()const;

   
protected:
   virtual ~rspfImageCombiner();   
   void precomputeBounds()const;

   rspf_uint32                theLargestNumberOfInputBands;
   rspf_uint32                theInputToPassThrough;
   bool                        theHasDifferentInputs;
   rspfRefPtr<rspfImageData> theNormTile;
   mutable std::vector<rspfIrect>     theFullResBounds;
   mutable bool                theComputeFullResBoundsFlag;
   rspf_uint32                theCurrentIndex;
   
TYPE_DATA  
};

#endif /* #ifndef rspfImageCombiner_HEADER */
