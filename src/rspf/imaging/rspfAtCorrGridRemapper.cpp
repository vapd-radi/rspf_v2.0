//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts <gpotts@imagelinks.com>
//
// Description:
//*******************************************************************
//  $Id: rspfAtCorrGridRemapper.cpp 21631 2012-09-06 18:10:55Z dburken $
#include <rspf/imaging/rspfAtCorrGridRemapper.h>
#include <rspf/imaging/rspfImageData.h>

RTTI_DEF1(rspfAtCorrGridRemapper, "rspfAtCorrGridRemapper", rspfAtCorrRemapper);

rspfAtCorrGridRemapper::rspfAtCorrGridRemapper(rspfObject* owner,
                                                 rspfImageSource* inputSource,
                                                 const rspfString& sensorType)
   :rspfAtCorrRemapper(owner, inputSource, sensorType)
{
   theUseInterpolationFlag = true;
}

rspfAtCorrGridRemapper::~rspfAtCorrGridRemapper()
{
}

void rspfAtCorrGridRemapper::interpolate(const rspfDpt& pt,
                                          int band,
                                          double& a,
                                          double& b,
                                          double& c)const
{
   if(theGridBounds.pointWithin(pt))
   {
      double x = (((double)(pt.x - theUlGridBound.x))/theGridBounds.width())*(theGridSize.x-1);
      double y = (((double)(pt.y - theUlGridBound.y))/theGridBounds.height())*(theGridSize.y-1);

      int xidx = (int)x;
      int yidx = (int)y;
      
      double xt = x - xidx;
      double yt = y - yidx;

      double va00 = theAGrid[band][yidx][xidx];
      double va01 = theAGrid[band][yidx][xidx+1];
      double va11 = theAGrid[band][yidx+1][xidx+1];
      double va10 = theAGrid[band][yidx+1][xidx];
      double vb00 = theBGrid[band][yidx][xidx];
      double vb01 = theBGrid[band][yidx][xidx+1];
      double vb11 = theBGrid[band][yidx+1][xidx+1];
      double vb10 = theBGrid[band][yidx+1][xidx];
      double vc00 = theCGrid[band][yidx][xidx];
      double vc01 = theCGrid[band][yidx][xidx+1];
      double vc11 = theCGrid[band][yidx+1][xidx+1];
      double vc10 = theCGrid[band][yidx+1][xidx];

      double vaInterpH1 = va00 + (va01-va00)*xt;
      double vaInterpH2 = va10 + (va11-va10)*xt;
      double vbInterpH1 = vb00 + (vb01-vb00)*xt;
      double vbInterpH2 = vb10 + (vb11-vb10)*xt;
      double vcInterpH1 = vc00 + (vc01-vc00)*xt;
      double vcInterpH2 = vc10 + (vc11-vc10)*xt;
      
      a = vaInterpH1 + (vaInterpH2-vaInterpH1)*yt;
      b = vbInterpH1 + (vbInterpH2-vbInterpH1)*yt;
      c = vcInterpH1 + (vcInterpH2-vcInterpH1)*yt;
   }
   else
   {
      rspfAtCorrRemapper::interpolate(pt,
                                       band,
                                       a,
                                       b,
                                       c);
   }
}

void rspfAtCorrGridRemapper::initialize()
{
   rspfAtCorrRemapper::initialize();
   
   if(theInputConnection)
   {
      theGridBounds = theInputConnection->getBoundingRect();
      theUlGridBound = theGridBounds.ul();
   }
   else
   {
      theGridBounds.makeNan();
   }

   if(!theAGrid.size()||
      !theBGrid.size()||
      !theCGrid.size())
   {
      theUseInterpolationFlag=false;
   }
   setBaseToAverage();
}

void rspfAtCorrGridRemapper::setGridSize(rspf_uint32 numberOfBands, const rspfIpt& gridSize)
{
   
   if((theGridSize!=gridSize)||(numberOfBands != theAGrid.size()))
   {
      theAGrid.resize(numberOfBands);
      theBGrid.resize(numberOfBands);
      theCGrid.resize(numberOfBands);

      for(rspf_uint32 b = 0; b < numberOfBands;++b)
      {
         theAGrid[b].resize(gridSize.y);
         theBGrid[b].resize(gridSize.y);
         theCGrid[b].resize(gridSize.y);
         for(int r = 0; r < gridSize.y; ++r)
         {
            theAGrid[b][r].resize(gridSize.x);
            theBGrid[b][r].resize(gridSize.x);
            theCGrid[b][r].resize(gridSize.x);
         }
         
      }
      theGridSize = gridSize;
   }
   
}


void rspfAtCorrGridRemapper::setBaseToAverage()
{
   rspf_uint32 idxBand;
   vector<double> xaAverage(theAGrid.size());
   vector<double> xcAverage(theBGrid.size());
   vector<double> xbAverage(theCGrid.size());
   // double aAverage=0.0, bAverage=0.0, cAverag=0.0;


   if((theAGrid.size() != theBGrid.size())||
      (theAGrid.size() != theCGrid.size())||
      (theAGrid.size() == 0))
   {
      return;
   }
   
   for(idxBand = 0; idxBand < theAGrid.size(); ++idxBand)
   {
      rspf_uint32 idxRow = 0;
      xaAverage[idxBand] = 0.0;
      xbAverage[idxBand] = 0.0;
      xcAverage[idxBand] = 0.0;
      for(idxRow = 0; idxRow < theAGrid[idxBand].size();++idxRow)
      {
         rspf_uint32 idxCol = 0;
         for(idxCol = 0; idxCol < theAGrid[idxBand][idxRow].size();++idxCol)
         {
            xaAverage[idxBand] += theAGrid[idxBand][idxRow][idxCol];
            
         }
      }
      for(idxRow = 0; idxRow < theBGrid[idxBand].size();++idxRow)
      {
         rspf_uint32 idxCol = 0;
         for(idxCol = 0; idxCol < theBGrid[idxBand][idxRow].size();++idxCol)
         {
            xbAverage[idxBand] += theBGrid[idxBand][idxRow][idxCol];
         }
      }
      for(idxRow = 0; idxRow < theCGrid[idxBand].size();++idxRow)
      {
         rspf_uint32 idxCol = 0;
         for(idxCol = 0; idxCol < theCGrid[idxBand][idxRow].size();++idxCol)
         {
            xaAverage[idxBand] += theCGrid[idxBand][idxRow][idxCol];
            xbAverage[idxBand] += theCGrid[idxBand][idxRow][idxCol];
            xcAverage[idxBand] += theCGrid[idxBand][idxRow][idxCol];
            
         }
      }
   }

   for(idxBand = 0; idxBand < theAGrid.size(); ++idxBand)
   {
      xaAverage[idxBand] /=(double)(theGridSize.x*theGridSize.y);
      xbAverage[idxBand] /=(double)(theGridSize.x*theGridSize.y);
      xcAverage[idxBand] /=(double)(theGridSize.x*theGridSize.y);
   }

   theXaArray = xaAverage;
   theXbArray = xbAverage;
   theXcArray = xcAverage;
}

/*!
 * Method to the load (recreate) the state of an object from a keyword
 * list.  Return true if ok or false on error.
 */
bool rspfAtCorrGridRemapper::loadState(const rspfKeywordlist& kwl,
                                        const char* prefix)
{
   const char* MODULE = "rspfAtCorrGridRemapper::loadState";
   theAGrid.clear();
   theBGrid.clear();
   theCGrid.clear();
   
   if (!theTile || !theSurfaceReflectance)
   {
      cerr << MODULE << " ERROR:"
           << "Not initialized..." << endl;
      return false;
   }
   theUseInterpolationFlag=true;
   rspf_uint32 bands = theTile->getNumberOfBands();

   theAGrid.resize(bands);
   theBGrid.resize(bands);
   theCGrid.resize(bands);


   const char* rowsLookup = NULL;
   const char* colsLookup = NULL;
   for(rspf_uint32 band = 0; band < bands; ++band)
   {
      rspf_uint32 r = 0;
      rspf_uint32 c = 0;
      rspfString gridString = "band";
      gridString += rspfString::toString(band+1);
      gridString += ".grid";
      
      rowsLookup = kwl.find(prefix,
                            gridString + ".rows");
      
      colsLookup = kwl.find(prefix,
                            gridString + ".cols");
      if(rowsLookup&&colsLookup)
      {
         cout << "both keywords needed: " << (gridString+".rows") << endl
              << (gridString+".cols") << endl;

         theUseInterpolationFlag=false;
         return false;
      }
      
      rspf_uint32 rows = rspfString(rowsLookup).toULong();
      rspf_uint32 cols = rspfString(colsLookup).toULong();

      if(cols&&rows)
      {
         cout << "value for cols and rows keyword have 0" << endl;
         theUseInterpolationFlag=false;
         return false;
      }

      theAGrid[band].resize(rows);
      theBGrid[band].resize(rows);
      theCGrid[band].resize(rows);
      for(r = 0; r < rows; ++r)
      {
         theAGrid[band][r].resize(cols);
         theBGrid[band][r].resize(cols);
         theCGrid[band][r].resize(cols);
      }

      rspf_uint32 idx = 1;
      const char* aLookup;
      const char* bLookup;
      const char* cLookup;
      for(r = 0; r < rows; ++r)
      {
         for(c = 0; c < cols; ++c)
         {
            aLookup = kwl.find(prefix, gridString+".a"+rspfString::toString(idx));
            bLookup = kwl.find(prefix, gridString+".b"+rspfString::toString(idx));
            cLookup = kwl.find(prefix, gridString+".c"+rspfString::toString(idx));
            theAGrid[band][r][c] = rspfString(aLookup).toDouble();
            theBGrid[band][r][c] = rspfString(bLookup).toDouble();
            theCGrid[band][r][c] = rspfString(cLookup).toDouble();
            
            ++idx;
         }
      }

      theGridSize.x = cols;
      theGridSize.y = rows;
      setBaseToAverage();
   }

   return true;
}

bool rspfAtCorrGridRemapper::saveState(rspfKeywordlist& kwl,
                                        const char* prefix)const
{

   rspf_uint32 bands = (rspf_uint32)theAGrid.size();
   
   for(rspf_uint32 band = 0; band < bands; ++band)
   {
      int idx = 1;
      int r = 0;
      int c = 0;
      rspfString gridString = "band";
      gridString += rspfString::toString(band+1);
      gridString += ".grid";

      rspf_uint32 rows = (rspf_uint32)theAGrid[band].size();
      kwl.add(prefix,
              gridString+".rows",
              rows,
              true);
      rspf_uint32 cols = (rspf_uint32)theAGrid[band][0].size();
      kwl.add(prefix,
              gridString+".cols",
              cols,
              true);
      for(r = 0; r < theGridSize.y; ++r)
      {
         for(c = 0; c < theGridSize.x; ++c)
         {
            kwl.add(prefix,
                    gridString+".a"+rspfString::toString(idx),
                    theAGrid[band][r][c],
                    true);
            kwl.add(prefix,
                    gridString+".b"+rspfString::toString(idx),
                    theAGrid[band][r][c],
                    true);
            kwl.add(prefix,
                    gridString+".c"+rspfString::toString(idx),
                    theAGrid[band][r][c],
                    true);
            
            ++idx;
         }
      }
   }

   return rspfAtCorrRemapper::saveState(kwl, prefix);
}
