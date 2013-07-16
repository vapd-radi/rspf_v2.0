//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfAtCorrGridRemapper.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfAtCorrGridRemapper_HEADER
#define rspfAtCorrGridRemapper_HEADER
#include <rspf/imaging/rspfAtCorrRemapper.h>

/*!
 * The grid is encoded into band separate grid and allows bilinear interpolation
 * of atmospheric correction coefficients.  It uses the implementation found in
 * rspfAtCorrRemapper.  The base class will call interpolate to calculate the
 * a, b, c coefficients used.  This class just overrides the base interpolation method. For
 * gridded remappers
 *
 *
 *
 * Sample keywordlist includes:
 *
 * band1.grid.rows: 4
 * band1.grid.cols: 4
 * band1.grid.a1:
 * band1.grid.b1:
 * band1.grid.c1:
 * band1.grid.a2:
 * band1.grid.b2:
 * band1.grid.c2:
 * band1.grid.a3:
 * band1.grid.b3:
 * band1.grid.c3:
 * band1.grid.a4:
 * band1.grid.b4:
 * band1.grid.c4:
 * band2.grid.rows: 4
 * band2.grid.cols: 4
 * band2.grid.a1:
 * band2.grid.b1:
 * band2.grid.c1:
 * band2.grid.a2:
 * band2.grid.b2:
 * band2.grid.c2:
 * band2.grid.a3:
 * band2.grid.b3:
 * band2.grid.c3:
 * band2.grid.a4:
 * band2.grid.b4:
 * band2.grid.c4:
 *
 *  :
 *  :
 *  :
 */
class rspfAtCorrGridRemapper: public rspfAtCorrRemapper
{
public:
   rspfAtCorrGridRemapper(rspfObject* owner = NULL,
                           rspfImageSource* inputSource =  NULL,
                           const rspfString& sensorType = "");
   


   void setGridRect(const rspfIrect& rect)
      {
         theGridBounds  = rect;
         theUlGridBound = rect.ul();
      }
   void setGridSize(rspf_uint32 numberOfBands, const rspfIpt& gridSize);
   rspfIpt getGridSize()const
      {
         return theGridSize;
      }
   
   void getValues(rspf_uint32 band,
                  rspf_uint32 row,
                  rspf_uint32 col,
                  double& a,
                  double& b,
                  double& c)
      {
         a = theAGrid[band][row][col];
         b = theAGrid[band][row][col];
         c = theAGrid[band][row][col];
      }

   void getValues(const rspfDpt& pt,
                  rspf_uint32 band,
                  double& a,
                  double& b,
                  double& c)
      {
         interpolate(pt, band, a, b, c);
      }
   
   void setValues(rspf_uint32 band,
                  rspf_uint32 row,
                  rspf_uint32 col,
                  const double& a,
                  const double& b,
                  const double& c)
      {
         theAGrid[band][row][col] = a;
         theBGrid[band][row][col] = b;
         theCGrid[band][row][col] = c;
      }
   
   virtual void initialize();
   
   /*!
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;

protected:
   virtual ~rspfAtCorrGridRemapper();
  rspfIrect theGridBounds;
   rspfIpt   theUlGridBound;
   rspfIpt   theGridSize;
   std::vector< std::vector< std::vector<double> > >theAGrid;
   std::vector< std::vector< std::vector<double> > >theBGrid;
   std::vector< std::vector< std::vector<double> > >theCGrid;

   virtual void interpolate(const rspfDpt& pt,
                            int band,
                            double& a,
                            double& b,
                            double& c)const;
   virtual void setBaseToAverage();
   
TYPE_DATA
};

#endif
