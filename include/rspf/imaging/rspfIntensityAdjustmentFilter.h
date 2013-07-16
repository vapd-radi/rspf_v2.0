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
// $Id: rspfIntensityAdjustmentFilter.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfIntensityAdjustmentFilter_HEADER
#define rspfIntensityAdjustmentFilter_HEADER
#include <rspf/imaging/rspfImageSourceFilter.h>
#include <rspf/base/rspfDblGrid.h>

class rspfIntensityAdjustmentFilter : public rspfImageSourceFilter
{
public:
   rspfIntensityAdjustmentFilter();
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& rect,
                                               rspf_uint32 resLevel=0);

   const rspfIrect& getGridBounds()const
      {
         return theGridBounds;
      }
   virtual void setIntensityGrid(const rspfDblGrid& grid)
      {
         theMeanIntensityGrid = grid;
      }
   void setMeanIntensityTarget(double targetMean)
      {
         theMeanIntensityTarget = targetMean;
      }
   double getMeanIntensityTarget()const
      {
         return theMeanIntensityTarget;
      }
   rspfDblGrid& getMeanIntensityGrid()
      {
         return theMeanIntensityGrid;
      }
   const rspfDblGrid& getMeanIntensityGrid()const
      {
         return theMeanIntensityGrid;
      }
   void createAndPopulateGrid(const rspfIpt& spacing, double targetMean = .5);
   
   virtual void initialize();
protected:
   virtual ~rspfIntensityAdjustmentFilter();

   virtual void allocate();
   
   rspfDblGrid                theMeanIntensityGrid;
   double                      theMeanIntensityTarget;
   rspfRefPtr<rspfImageData> theNormTile;
   rspfRefPtr<rspfImageData> theTile;
   rspfRefPtr<rspfImageData> theBlankTile;

   rspfIrect   theGridBounds;

   double computeMeanIntensity(rspfRefPtr<rspfImageData>& data);
   void loadNormTile(rspfRefPtr<rspfImageData>& data);
   double matchTargetMean(double inputValue,
                          double meanValue,
                          double targetMean,
                          double minValue,
                          double maxValue);
   
TYPE_DATA
};
#endif
