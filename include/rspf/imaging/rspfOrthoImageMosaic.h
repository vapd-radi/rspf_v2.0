//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfOrthoImageMosaic.h 17932 2010-08-19 20:34:35Z dburken $
#ifndef rspfOrthoImageMosaic_HEADER
#define rspfOrthoImageMosaic_HEADER

#include <rspf/imaging/rspfImageMosaic.h>

class RSPFDLLEXPORT rspfOrthoImageMosaic : public rspfImageMosaic
{
public:
   rspfOrthoImageMosaic();
   rspfOrthoImageMosaic(rspfConnectableObject::ConnectableObjectList& inputSources);
   virtual rspfIrect getBoundingRect(rspf_uint32 resLevel=0) const;
   virtual void initialize();

   //! Returns the geometry associated with the full mosaic. This object maintains it's own 
   //! geometry instance.
   virtual rspfRefPtr<rspfImageGeometry> getImageGeometry();

   //! Used to retrieve the number of overlapping images withint the given rect.
   virtual rspf_uint32 getNumberOfOverlappingImages(const rspfIrect& rect, rspf_uint32 resLevel=0)const;

   //! Used to populate the result with the index of the overlapping images.
   virtual void getOverlappingImages(std::vector<rspf_uint32>& result,
                                     const rspfIrect& rect,
                                     rspf_uint32 resLevel=0)const;
   
   virtual rspfRefPtr<rspfImageData> getNextTile(rspf_uint32& returnedIdx,
                                                   const rspfIrect& origin,
                                                   rspf_uint32 resLevel=0);
   
   rspfIrect getRelativeRect(rspf_uint32 index,
                              rspf_uint32 resLevel = 0)const;
protected:
   virtual ~rspfOrthoImageMosaic();   
   void computeBoundingRect(rspf_uint32 resLevel=0);
   
   //! If this object is maintaining an rspfImageGeometry, this method needs to be called after 
   //! each time the contents of the mosaic changes.
   void updateGeometry();

   std::vector<rspfDpt>  m_InputTiePoints;
   rspfDpt    m_Delta; //!< Holds R0 delta and will be scaled for different r-level requests
   rspfDpt    m_UpperLeftTie; //!< Will hold the upper left tie of the mosaic.
   rspfIrect  m_BoundingRect;
   rspfString m_Units;
   rspfRefPtr<rspfImageGeometry> m_Geometry; //!< The input image geometry, altered by the map tiepoint

TYPE_DATA
};

#endif
