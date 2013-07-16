//-------------------------------------------------------------------
// License:  LGPL.  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
//-------------------------------------------------------------------
//  $Id$
#ifndef rspfTwoColorView_HEADER
#define rspfTwoColorView_HEADER 1
#include <rspf/base/rspfConstants.h>
#include <rspf/imaging/rspfImageCombiner.h>

/**
 * This is a 2 color view of the input.  It basically allows for a change detection and by default will take the first input and map
 * it to the blue channel and takes the second input and maps to the red channel.  The unused channel is mapped to a min pixel value.
 */
class RSPF_DLL rspfTwoColorView : public rspfImageCombiner
{
public:

   /**
    * @brief Enumerations for mapping inputs to red, green and blue
    * output channels.
    */
   enum rspfTwoColorMultiViewOutputSource
   {
      UNKNOWN = 0,
      MIN     = 1,
      OLD     = 2,
      NEW     = 3
   };
   
   rspfTwoColorView();
   virtual rspf_uint32 getNumberOfOutputBands() const;
   /**
    * @return the requested region of interest
    */
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& rect,
                                               rspf_uint32 resLevel=0);
   rspfScalarType getOutputScalarType() const;
   
   double getNullPixelValue(rspf_uint32 band)const;
   double getMinPixelValue(rspf_uint32 band)const;
   double getMaxPixelValue(rspf_uint32 band)const;
   
   /**
    * @brief Sets which bands to use from inputs, and which inputs to use for
    * output red, green and blue channels.
    *
    * This method does no error checking as the inputs may or may not be set
    * prior to this call.  rspfTwoColorView::initialize() will verify once
    * inputs are established.  So if an input band is out of range band
    * 0 (first band) will be used.
    *
    * Callers should ensure initialize is called after this.
    *
    * @param oldInputBandIndex Zero based band index for old input. Default=0
    * * 
    * @param newInputBandIndex Zero based band index for new input. Default=0.
    * 
    * @param redOutputSource default=OLD.  Can be:
    *    rspfTwoColorMultiViewOutputSource::OLD
    *    rspfTwoColorMultiViewOutputSource::NEW
    *    rspfTwoColorMultiViewOutputSource::MIN
    *    
    * @param grnOutputSource default=NEW.  Can be:
    *    rspfTwoColorMultiViewOutputSource::OLD
    *    rspfTwoColorMultiViewOutputSource::NEW
    *    rspfTwoColorMultiViewOutputSource::MIN
    *    
    * @param bluOutputSource default=NEW.  Can be:
    *    rspfTwoColorMultiViewOutputSource::OLD
    *    rspfTwoColorMultiViewOutputSource::NEW
    *    rspfTwoColorMultiViewOutputSource::MIN   
    */
   void setBandIndexMapping(
      rspf_uint32 oldInputBandIndex,
      rspf_uint32 newInputBandIndex,
      rspfTwoColorMultiViewOutputSource redOutputSource,
      rspfTwoColorMultiViewOutputSource grnOutputSource,
      rspfTwoColorMultiViewOutputSource bluOutputSource);

   virtual void initialize();
   
protected:
   void allocate();
   void runAlgorithm(rspfImageData* newData, rspfImageData* oldData);
   void runNative8(rspfImageData* newData,   rspfImageData* oldData);
   void runNorm(rspfImageData* newData,   rspfImageData* oldData);
   
   bool                               m_byPassFlag;
   bool                               m_nativeFlag;
   rspfRefPtr<rspfImageData>        m_twoColorTile;
   rspfRefPtr<rspfImageSource>      m_newInput;
   rspfRefPtr<rspfImageSource>      m_oldInput;
   rspf_uint32                       m_newInputBandIndex;
   rspf_uint32                       m_oldInputBandIndex;
   rspfTwoColorMultiViewOutputSource m_redSource;
   rspfTwoColorMultiViewOutputSource m_grnSource;
   rspfTwoColorMultiViewOutputSource m_bluSource;

TYPE_DATA
};

#endif /* #ifndef rspfTwoColorView_HEADER */
