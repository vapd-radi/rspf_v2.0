//*******************************************************************. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
//
#ifndef rspfMemoryImageSource_HEADER
#define rspfMemoryImageSource_HEADER 1

#include <rspf/base/rspfConstants.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/imaging/rspfImageSource.h>

class RSPF_DLL rspfMemoryImageSource : public rspfImageSource
{
public:
   rspfMemoryImageSource();
   rspfMemoryImageSource(const rspfMemoryImageSource& src)
   :rspfImageSource(src),
   m_image(src.m_image.valid()?(rspfImageData*)src.m_image->dup():(rspfImageData*)0),
   m_result(0),
   m_geometry(m_geometry.valid()?(rspfImageGeometry*)src.m_geometry->dup():(rspfImageGeometry*)0),
   m_boundingRect(src.m_boundingRect)
   {
   }
   rspfMemoryImageSource* dup()const
   {
      return new rspfMemoryImageSource(*this);
   }
   void setImage(rspfRefPtr<rspfImageData> image);
   void setImage(rspfScalarType scalarType,
                 rspf_uint32 numberOfBands,
                 rspf_uint32 width,
                 rspf_uint32 height);
   void setRect(rspf_uint32 ulx,
                rspf_uint32 uly,
                rspf_uint32 width,
                rspf_uint32 height);
   virtual rspf_uint32 getNumberOfInputBands() const;
   virtual rspf_uint32 getNumberOfOutputBands() const;
   virtual rspfScalarType getOutputScalarType() const;
   virtual double getNullPixelValue(rspf_uint32 band=0)const;
   virtual double getMinPixelValue(rspf_uint32 band=0)const;
   virtual double getMaxPixelValue(rspf_uint32 band=0)const;
   
   virtual rspfIrect getBoundingRect(rspf_uint32 resLevel=0)const;
   
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& rect,
                                               rspf_uint32 resLevel=0);
   
   virtual bool canConnectMyInputTo(rspf_int32 myInputIndex,
                                    const rspfConnectableObject* object)const;
   
   virtual void initialize();
	virtual rspf_uint32 getNumberOfDecimationLevels() const;
   virtual void getDecimationFactor(rspf_uint32 resLevel,
                                    rspfDpt& result) const;
   virtual void getDecimationFactors(std::vector<rspfDpt>& decimations)const;
   
   //! Returns the image geometry object associated with this tile source or NULL if not defined.
   //! The geometry contains full-to-local image transform as well as projection (image-to-world)
   //! Default implementation returns the image geometry object associated with the next  
   //! (left) input source (if any) connected to this source in the chain, or NULL.
   virtual rspfRefPtr<rspfImageGeometry> getImageGeometry()
   {
      return m_geometry;
   }
   
   //! Default implementation sets geometry of the first input to the geometry specified.
   virtual void setImageGeometry(rspfImageGeometry* geom)
   {
      m_geometry = geom;
   }
   
   /**
    *  save state is not implemented for this source and only passes to the base.
    */
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=0)const;
   
   /**
    * loadState is here so one can construct a memory source as a blank tile.
    */
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);
   
protected:
   rspfRefPtr<rspfImageData> m_image;
   rspfRefPtr<rspfImageData> m_result;
   rspfRefPtr<rspfImageGeometry> m_geometry;
   rspfIrect m_boundingRect;
   TYPE_DATA
};

#endif
