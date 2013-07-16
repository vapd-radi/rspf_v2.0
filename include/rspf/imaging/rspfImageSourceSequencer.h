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
//  $Id: rspfImageSourceSequencer.h 20302 2011-11-29 14:21:12Z dburken $
#ifndef rspfImageSourceSequencer_HEADER
#define rspfImageSourceSequencer_HEADER
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfConnectableObjectListener.h>


class RSPFDLLEXPORT rspfImageSourceSequencer
   :
      public rspfImageSource,
      public rspfConnectableObjectListener
{
public:
   rspfImageSourceSequencer(rspfImageSource* inputSource=NULL,
                             rspfObject* owner=NULL);

   virtual ~rspfImageSourceSequencer();
  /*!
    * This will return the number of tiles within the
    * area of interest.
    */
   rspf_uint32 getNumberOfTiles()const;

   /*!
    * Will return the number of tiles along the
    * x or horizontal direction.
    */
   rspf_uint32 getNumberOfTilesHorizontal()const;

   /*!
    * Will return the number of tiles along the
    * y or vertical direction.
    */
   rspf_uint32 getNumberOfTilesVertical()const;

   /*!
    * This must be called.  We can only initialize this
    * object completely if we know all connections
    * are valid.  Some other object drives this and so the
    * connection's initialize will be called after.  The job
    * of this connection is to set up the sequence.  It will
    * default to the bounding rect.  The area of interest can be
    * set to some other rectagle (use setAreaOfInterest).
    */
   virtual void initialize();

   /*!
    * Will set the current area of interest.
    */
   virtual void setAreaOfInterest(const rspfIrect& areaOfInterest);

   /*!
    * Just returns the current area of interest.
    */
   const rspfIrect& getAreaOfInterest()const;

   /*!
    * Will set the internal pointers to the upperleft
    * tile number.  To go to the next tile in the sequence
    * just call getNextTile.
    */
   virtual void setToStartOfSequence();

   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& rect,
                                               rspf_uint32 resLevel=0);

   /*!
    * Will allow you to get the next tile in the sequence.
    * Note the last tile returned will be an invalid
    * rspfRefPtr<rspfImageData>.  Callers should be able to do:
    * 
    * rspfRefPtr<rspfImageData> id = sequencer->getNextTile();
    * while (id.valid())
    * {
    *    doSomething;
    *    id = sequencer->getNextTile();
    * }
    * 
    */
   virtual rspfRefPtr<rspfImageData> getNextTile(rspf_uint32 resLevel=0);

   virtual bool getTileOrigin(rspf_int32 id, rspfIpt& origin)const;

   /*!
    * @brief Establishes a tile rect given tile ID.
    * @param tile_id
    * @param rect Rectangle to initialize.
    * @return true if valid; else, false.
    */
   bool getTileRect(rspf_uint32 tile_id, rspfIrect& rect) const;

   virtual rspfRefPtr<rspfImageData> getTile(rspf_int32 id,
                                               rspf_uint32 resLevel=0);

   virtual rspfIrect getBoundingRect(rspf_uint32 resLevel=0)const;
   virtual void getDecimationFactor(rspf_uint32 resLevel,
                                    rspfDpt& result) const;
   virtual void getDecimationFactors(vector<rspfDpt>& decimations) const;
   virtual rspf_uint32 getNumberOfDecimationLevels()const;
   
   /*!
    * Returns the number of bands available from the input.
    */
   virtual rspf_uint32 getNumberOfInputBands()const;   

   virtual rspfScalarType getOutputScalarType() const;
      
   virtual rspf_uint32 getTileWidth()const  { return theTileSize.x; }
   virtual rspf_uint32 getTileHeight()const { return theTileSize.y; }
   
   virtual void slaveProcessTiles()
      {
         return;
      }
   virtual bool isMaster()const
      {
         return true;
      }

   virtual rspfIpt getTileSize()const;
   virtual void setTileSize(const rspfIpt& tileSize);
   virtual void setTileSize(int width, int height);
   
   virtual void connectInputEvent(rspfConnectionEvent& event);
   virtual void disconnectInputEvent(rspfConnectionEvent& event);
   
   virtual bool canConnectMyInputTo(rspf_int32 inputIndex,
                                    const rspfConnectableObject* object)const;
   
   virtual double getNullPixelValue(rspf_uint32 band=0)const;
   virtual double getMinPixelValue(rspf_uint32 band=0)const;
   virtual double getMaxPixelValue(rspf_uint32 band=0)const;
   
protected:
   rspfImageSource*  theInputConnection;
   rspfRefPtr<rspfImageData> theBlankTile;
   /*!
    * Is the area of interest.  The default will
    * 
    */
   rspfIrect theAreaOfInterest;

   /*!
    * Called during initialize.
    */
   rspfIpt theTileSize;
   
   rspf_uint32 theNumberOfTilesHorizontal;
   rspf_uint32 theNumberOfTilesVertical;
   rspf_uint32 theCurrentTileNumber;

   virtual void updateTileDimensions();

TYPE_DATA
};

#endif
