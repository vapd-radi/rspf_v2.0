//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
// Description:
//
// Base class for all map projections.
// 
//*******************************************************************
//  $Id: rspfMapProjection.cpp 20154 2011-10-13 19:00:18Z dburken $

#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <sstream>

#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfEpsgProjectionFactory.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfDatumFactoryRegistry.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfEllipsoid.h>
#include <rspf/base/rspfString.h>
#include <rspf/elevation/rspfElevManager.h>
#include <rspf/base/rspfMatrix3x3.h>
#include <rspf/base/rspfUnitConversionTool.h>
#include <rspf/base/rspfUnitTypeLut.h>
#include <rspf/base/rspfTrace.h>

static rspfTrace traceDebug("rspfMapProjection:debug");

// RTTI information for the rspfMapProjection
RTTI_DEF1(rspfMapProjection, "rspfMapProjection" , rspfProjection);

rspfMapProjection::rspfMapProjection(const rspfEllipsoid& ellipsoid,
                                       const rspfGpt& origin)
   :theEllipsoid(ellipsoid),
    theOrigin(origin),
    theDatum(origin.datum()), // force no shifting
    theUlGpt(0, 0),
    theUlEastingNorthing(0, 0),
    theFalseEastingNorthing(0, 0),
    thePcsCode(0),
    theElevationLookupFlag(false),
    theModelTransform(),
    theInverseModelTransform(),
    theModelTransformUnitType(RSPF_UNIT_UNKNOWN),
    theProjectionUnits(RSPF_METERS) 
{
   theUlGpt = theOrigin;
   theUlEastingNorthing.makeNan();
   theMetersPerPixel.makeNan();
   theDegreesPerPixel.makeNan();
}

rspfMapProjection::rspfMapProjection(const rspfMapProjection& src)
      : rspfProjection(src),
        theEllipsoid(src.theEllipsoid),
        theOrigin(src.theOrigin),
        theDatum(src.theDatum),
        theMetersPerPixel(src.theMetersPerPixel),
        theDegreesPerPixel(src.theDegreesPerPixel),
        theUlGpt(src.theUlGpt),
        theUlEastingNorthing(src.theUlEastingNorthing),
        theFalseEastingNorthing(src.theFalseEastingNorthing),
        thePcsCode(src.thePcsCode),
        theElevationLookupFlag(false),
        theModelTransform(src.theModelTransform),
        theInverseModelTransform(src.theInverseModelTransform),
        theModelTransformUnitType(src.theModelTransformUnitType),
        theProjectionUnits(src.theProjectionUnits)
{
}

rspfMapProjection::~rspfMapProjection()
{
}

rspfGpt rspfMapProjection::origin()const
{
   return theOrigin;
}

void rspfMapProjection::setPcsCode(rspf_uint32 pcsCode)
{
   thePcsCode = pcsCode;
}

rspf_uint32 rspfMapProjection::getPcsCode() const
{
   // The PCS code is not always set when the projection is instantiated with explicit parameters,
   // since the code is only necessary when looking up those parameters in a database. However, it
   // is still necessary to recognize when an explicit projection coincides with an EPSG-specified
   // projection, and assign our PCS code to match it. So let's take this opportunity now to make 
   // sure the PCS code is properly initialized.
   if (thePcsCode == 0)
   {
      thePcsCode = rspfEpsgProjectionDatabase::instance()->findProjectionCode(*this);
      if (thePcsCode == 0)
         thePcsCode = 32767; // user-defined (non-EPSG) projection
   }
  
   if (thePcsCode == 32767)
      return 0; // 32767 only used internally. To the rest of RSPF, the PCS=0 is undefined
   
   return thePcsCode;
}

rspfString rspfMapProjection::getProjectionName() const
{
   return getClassName();
}

double rspfMapProjection::getA() const
{
   return theEllipsoid.getA();
}

double rspfMapProjection::getB() const
{
   return theEllipsoid.getB();
}

double rspfMapProjection::getF() const
{
   return theEllipsoid.getFlattening();
}

rspfDpt rspfMapProjection::getMetersPerPixel() const
{
   return theMetersPerPixel;
}

const rspfDpt& rspfMapProjection::getDecimalDegreesPerPixel() const
{
   return theDegreesPerPixel;
}

const rspfDpt& rspfMapProjection::getUlEastingNorthing() const
{
   return theUlEastingNorthing;
}

const rspfGpt&   rspfMapProjection::getUlGpt() const
{
   return theUlGpt;
}

const rspfGpt& rspfMapProjection::getOrigin() const
{
  return theOrigin;
}

const rspfDatum* rspfMapProjection::getDatum() const
{
   return theDatum;
}

bool rspfMapProjection::isGeographic()const
{
   return false;
}

void rspfMapProjection::setEllipsoid(const rspfEllipsoid& ellipsoid)
{
   theEllipsoid = ellipsoid; update();
}

void rspfMapProjection::setAB(double a, double b)
{
   theEllipsoid.setA(a); theEllipsoid.setB(b); update();
}

void rspfMapProjection::setDatum(const rspfDatum* datum)
{

   if (!datum || (*theDatum == *datum))
      return;

   theDatum = datum; 
   theEllipsoid = *(theDatum->ellipsoid());

   // Change the datum of the rspfGpt data members:
   theOrigin.changeDatum(theDatum);
   theUlGpt.changeDatum(theDatum);

   update();

   // A change of datum usually implies a change of EPSG codes. Reset the PCS code. It will be
   // reestablished as needed in the getPcsCode() method:
   thePcsCode = 0;
}

void rspfMapProjection::setOrigin(const rspfGpt& origin)
{
   // Set the origin and since the origin has a datum which in turn has
   // an ellipsoid, sync them up.
   // NOTE: Or perhaps we need to change the datum of the input origin to that of theDatum? (OLK 05/11)
   theOrigin    = origin;
   theOrigin.changeDatum(theDatum);
      
   update();
}

//*****************************************************************************
//  METHOD: rspfMapProjection::assign
//
//*****************************************************************************
void rspfMapProjection::assign(const rspfProjection &aProjection)
{
   if(&aProjection!=this)
   {
      rspfKeywordlist kwl;

      aProjection.saveState(kwl);
      loadState(kwl);
   }
}

//*****************************************************************************
//  METHOD: rspfMapProjection::update
//
//*****************************************************************************
void rspfMapProjection::update()
{
   // if the delta lat and lon per pixel is set then
   // check to see if the meters were set.
   //
   if( hasModelTransform() )
   {
      updateFromTransform();
   }
   else if( theDegreesPerPixel.hasNans() == false )
   {
      if(theMetersPerPixel.hasNans())
      {
         computeMetersPerPixel();
      }
   }
   else if(!theMetersPerPixel.hasNans())
   {
      computeDegreesPerPixel();
   }
   // compute the tie points if not already computed
   //
   // The tiepoint was specified either as easting/northing or lat/lon. Need to initialize the one
   // that has not been assigned yet:
   if (theUlEastingNorthing.hasNans() && !theUlGpt.hasNans())
      theUlEastingNorthing = forward(theUlGpt);
   else if (theUlGpt.hasNans() && !theUlEastingNorthing.hasNans())
      theUlGpt = inverse(theUlEastingNorthing);
   else if (theUlGpt.hasNans() && theUlEastingNorthing.hasNans())
   {
      theUlGpt = theOrigin;
      theUlEastingNorthing = forward(theUlGpt);
   }
}

void rspfMapProjection::updateFromTransform()
{
   if ( hasModelTransform() )
   {
      const NEWMAT::Matrix& m = theModelTransform.getData();
      rspfDpt ls1(0, 0);
      rspfDpt ls2(1, 0);
      rspfDpt ls3(0, 1);
      rspfGpt wpt1;
      rspfGpt wpt2;
      lineSampleToWorld(ls1, wpt1);
      rspfDpt mpt1(m[0][0]*ls1.x + m[0][1]*ls1.y + m[0][3],
                    m[1][0]*ls1.x + m[1][1]*ls1.y + m[1][3]);
      rspfDpt mpt2(m[0][0]*ls2.x + m[0][1]*ls2.y + m[0][3],
                    m[1][0]*ls2.x + m[1][1]*ls2.y + m[1][3]);
      rspfDpt mpt3(m[0][0]*ls3.x + m[0][1]*ls3.y + m[0][3],
                    m[1][0]*ls3.x + m[1][1]*ls3.y + m[1][3]);
      
      double len = 1.0;
      double len2 = 1.0;
      switch(theModelTransformUnitType)
      {
         case RSPF_DEGREES:
         case RSPF_MINUTES:
         case RSPF_SECONDS:
         case RSPF_RADIANS:
         {
            rspfUnitConversionTool ut;
            len  = (mpt1-mpt2).length();
            len2  = (mpt1-mpt3).length();
            ut.setValue((len+len2)*.5, theModelTransformUnitType);
            len = ut.getValue(RSPF_DEGREES);
            theDegreesPerPixel = rspfDpt(len, len);
            theUlGpt = wpt1;
            computeMetersPerPixel();
            break;
         }
         default:
         {
            rspfUnitConversionTool ut;
            len  = (mpt1-mpt2).length();
            len2  = (mpt1-mpt3).length();
            ut.setValue(mpt1.x, theModelTransformUnitType);
            mpt1.x = ut.getValue(RSPF_METERS);
            ut.setValue(mpt1.y, theModelTransformUnitType);
            mpt1.y = ut.getValue(RSPF_METERS);
            ut.setValue((len+len2)*.5, theModelTransformUnitType);
            len = ut.getValue(RSPF_METERS);
            theMetersPerPixel = rspfDpt(len, len);
            theUlEastingNorthing = mpt1;
            computeDegreesPerPixel();
            break;
         }
      }
      theUlGpt = wpt1;
   }
}

void rspfMapProjection::applyScale(const rspfDpt& scale,
                                    bool recenterTiePoint)
{
   rspfDpt mapTieDpt;
   rspfGpt mapTieGpt;
   if (recenterTiePoint)
   {
      if (isGeographic())
      {
         mapTieGpt = getUlGpt();
         mapTieGpt.lat += theDegreesPerPixel.lat/2.0;
         mapTieGpt.lon -= theDegreesPerPixel.lon/2.0;
      }
      else
      {
         mapTieDpt = getUlEastingNorthing();
         mapTieDpt.x -= theMetersPerPixel.x/2.0;
         mapTieDpt.y += theMetersPerPixel.y/2.0;
      }
   }

   theDegreesPerPixel.x *= scale.x;
   theDegreesPerPixel.y *= scale.y;
   theMetersPerPixel.x  *= scale.x;
   theMetersPerPixel.y  *= scale.y;

   if ( recenterTiePoint )
   {
      if (isGeographic())
      {
         mapTieGpt.lat -= theDegreesPerPixel.lat/2.0;
         mapTieGpt.lon += theDegreesPerPixel.lon/2.0;
         setUlTiePoints(mapTieGpt);
      }
      else
      {
         mapTieDpt.x += theMetersPerPixel.x/2.0;
         mapTieDpt.y -= theMetersPerPixel.y/2.0;
         setUlTiePoints(mapTieDpt);
      }
   }

   if (theModelTransformUnitType != RSPF_UNIT_UNKNOWN)
   {
      theModelTransform.getData()[0][0] = theModelTransform.getData()[0][0]*scale.x;
      theModelTransform.getData()[1][1] = theModelTransform.getData()[1][1]*scale.y;

      theInverseModelTransform = theModelTransform;
      theInverseModelTransform.i();

      updateFromTransform();
   }
}

//*****************************************************************************
//  METHOD: rspfMapProjection::worldToLineSample
//
//*****************************************************************************
rspfDpt rspfMapProjection::worldToLineSample(const rspfGpt &worldPoint)const
{
   rspfDpt result;

   worldToLineSample(worldPoint, result);

   return result;
}

//*****************************************************************************
//  METHOD: rspfMapProjection::lineSampleToWorld
//
//*****************************************************************************
rspfGpt rspfMapProjection::lineSampleToWorld(const rspfDpt &lineSample)const
{
   rspfGpt result;

   lineSampleToWorld(lineSample, result);

   return result;
}

//*****************************************************************************
//  METHOD: rspfMapProjection::worldToLineSample
//
//*****************************************************************************
void rspfMapProjection::worldToLineSample(const rspfGpt &worldPoint,
                                           rspfDpt&       lineSample)const
{

   if(theModelTransformUnitType != RSPF_UNIT_UNKNOWN)
   {
      rspfGpt gpt = worldPoint;
      if(theDatum)
      {
         gpt.changeDatum(theDatum);
      }
      switch(theModelTransformUnitType)
      {
         case RSPF_METERS:
         case RSPF_FEET:
         case RSPF_US_SURVEY_FEET:
         {
            rspfDpt modelPoint = forward(gpt);
            rspfUnitConversionTool ut;
            ut.setValue(modelPoint.x, RSPF_METERS);
            modelPoint.x = ut.getValue(theModelTransformUnitType);
            ut.setValue(modelPoint.y, RSPF_METERS);
            modelPoint.y = ut.getValue(theModelTransformUnitType);
            const NEWMAT::Matrix& m = theInverseModelTransform.getData();
            
            lineSample.x = m[0][0]*modelPoint.x + m[0][1]*modelPoint.y + m[0][3];
            lineSample.y = m[1][0]*modelPoint.x + m[1][1]*modelPoint.y + m[1][3];
            
            return;
         }
         case RSPF_DEGREES:
         case RSPF_RADIANS:
         case RSPF_MINUTES:
         case RSPF_SECONDS:
         {
            rspfUnitConversionTool ut;
            rspfDpt modelPoint;
            modelPoint.lat = gpt.latd();
            modelPoint.lon = gpt.lond();
            ut.setValue(modelPoint.lat, RSPF_DEGREES);
            modelPoint.lat = ut.getValue(theModelTransformUnitType);
            ut.setValue(modelPoint.lon, RSPF_DEGREES);
            modelPoint.lon = ut.getValue(theModelTransformUnitType);
            const NEWMAT::Matrix& m = theInverseModelTransform.getData();
            
            lineSample.x = m[0][0]*modelPoint.x + m[0][1]*modelPoint.y + m[0][3];
            lineSample.y = m[1][0]*modelPoint.x + m[1][1]*modelPoint.y + m[1][3];
            return;
         }
         default:
         {
            lineSample.makeNan();
            return;
         }
      }
   }
   else if(isGeographic())
   {
      rspfGpt gpt = worldPoint;
      
      if (theOrigin.datum() != gpt.datum())
      {
         // Apply datum shift if it's not the same.
         gpt.changeDatum(theOrigin.datum());
      }
      
      lineSample.line = (theUlGpt.latd() - gpt.latd()) / theDegreesPerPixel.y;
      lineSample.samp = (gpt.lond() - theUlGpt.lond()) / theDegreesPerPixel.x;
   }
   else
   {
      // make sure our tie point is good and world point
      // is good.
      //
      if(theUlEastingNorthing.isNan()||
         worldPoint.isLatNan() || worldPoint.isLonNan())
      {
         lineSample.makeNan();
         return;
      }
      // initialize line sample
      //   lineSample = rspfDpt(0,0);
      
      // I am commenting this code out because I am going to
      // move it to the rspfImageViewProjectionTransform.
      //
      // see if we have a datum set and if so
      // shift the world to our datum.  If not then
      // find the easting northing value for the world
      // point.
      if(theDatum)
      {
         rspfGpt gpt = worldPoint;
         
         gpt.changeDatum(theDatum);
         
         // lineSample is currently in easting northing
         // and will need to be converted to line sample.
         lineSample = forward(gpt);
      }
      else
      {
         // lineSample is currently in easting northing
         // and will need to be converted to line sample.
         lineSample = forward(worldPoint);
      }
      
      // check the final result to make sure there were no
      // problems.
      //
      if(!lineSample.isNan())
      {
//       if(!isIdentityMatrix())
//       {
//          rspfDpt temp = lineSample;
         
//          lineSample.x = theInverseTrans[0][0]*temp.x+
//                         theInverseTrans[0][1]*temp.y+
//                         theInverseTrans[0][2];
         
//          lineSample.y = theInverseTrans[1][0]*temp.x+
//                         theInverseTrans[1][1]*temp.y+
//                         theInverseTrans[1][2];
//       }
//       else
         {
            lineSample.x = ((lineSample.x  - theUlEastingNorthing.x)/theMetersPerPixel.x);
            
            // We must remember that the Northing is negative since the positive
            // axis for an image is assumed to go down since it's image space.
            lineSample.y = (-(lineSample.y - theUlEastingNorthing.y)/theMetersPerPixel.y);
         }
      }
   }
}

//*****************************************************************************
//  METHOD: rspfMapProjection::lineSampleHeightToWorld
//
//*****************************************************************************
void rspfMapProjection::lineSampleHeightToWorld(const rspfDpt &lineSample,
                                                 const double&  hgtEllipsoid,
                                                 rspfGpt&      gpt)const
{
   // make sure that the passed in lineSample is good and
   // check to make sure our easting northing is good so
   // we can compute the line sample.
   if(lineSample.hasNans())
   {
      gpt.makeNan();
      return;
   }
   if(theModelTransformUnitType != RSPF_UNIT_UNKNOWN)
   {
      const NEWMAT::Matrix& m = theModelTransform.getData();
      // map transforms can only be 2-D for now so we will look at
      // the first 2 rows only
      rspfDpt modelPoint(m[0][0]*lineSample.x + m[0][1]*lineSample.y + m[0][3],
                          m[1][0]*lineSample.x + m[1][1]*lineSample.y + m[1][3]);
      switch(theModelTransformUnitType)
      {
         case RSPF_DEGREES:
         {
            gpt.latd(modelPoint.lat);
            gpt.lond(modelPoint.lon);
            gpt.datum(theDatum);
            return;
         }
         case RSPF_MINUTES:
         case RSPF_SECONDS:
         case RSPF_RADIANS:
         {
            rspfUnitConversionTool ut;
            ut.setValue(modelPoint.x, theModelTransformUnitType);
            modelPoint.x = ut.getValue(RSPF_DEGREES);
            ut.setValue(modelPoint.y, theModelTransformUnitType);
            modelPoint.y = ut.getValue(RSPF_DEGREES);
            gpt.latd(modelPoint.lat);
            gpt.lond(modelPoint.lon);
            gpt.datum(theDatum);
            return;
         }
         default:
         {
            rspfUnitConversionTool ut;
            ut.setValue(modelPoint.x, theModelTransformUnitType);
            modelPoint.x = ut.getValue(RSPF_METERS);
            ut.setValue(modelPoint.y, theModelTransformUnitType);
            modelPoint.y = ut.getValue(RSPF_METERS);
            gpt = inverse(modelPoint);
            break;
         }
      }
      gpt.datum(theDatum);
   }
   else if(isGeographic())
   {
      double lat = theUlGpt.latd() - (lineSample.line * theDegreesPerPixel.y);
      double lon = theUlGpt.lond() + (lineSample.samp * theDegreesPerPixel.x);
      
      gpt.latd(lat);
      gpt.lond(lon);
      gpt.hgt = hgtEllipsoid;
   }
   else
   {
      if(theUlEastingNorthing.hasNans())
      {
         gpt.makeNan();
         return;
      }
      rspfDpt eastingNorthing;
      
      eastingNorthing = (theUlEastingNorthing);
      
      eastingNorthing.x += (lineSample.x*theMetersPerPixel.x);
      
      //
      // Note:  the Northing is positive up.  In image space
      // the positive axis is down so we must multiply by
      // -1
      //
      eastingNorthing.y += (-lineSample.y*theMetersPerPixel.y);
      
      
      //
      // now invert the meters into a ground point.
      //
      gpt = inverse(eastingNorthing);
      gpt.datum(theDatum);
      
      if(gpt.isLatNan() && gpt.isLonNan())
      {
         gpt.makeNan();
      }
      else
      {
         gpt.clampLat(-90, 90);
         gpt.clampLon(-180, 180);
         
         // Finally assign the specified height:
         gpt.hgt = hgtEllipsoid;
      }
   }
   if(theElevationLookupFlag)
   {
      gpt.hgt = rspfElevManager::instance()->getHeightAboveEllipsoid(gpt);
   }
}

//*****************************************************************************
//  METHOD: rspfMapProjection::lineSampleToWorld
//
//  Implements the base class pure virtual. Simply calls lineSampleToWorld()
//  and assigns argument height to the resultant groundpoint.
//
//*****************************************************************************
void rspfMapProjection::lineSampleToWorld (const rspfDpt& lineSampPt,
                                            rspfGpt&       worldPt) const
{
   double elev = rspf::nan();

//    if(theElevationLookupFlag)
//    {
//       elev =  rspfElevManager::instance()->getHeightAboveEllipsoid(worldPt);
//    }

   lineSampleHeightToWorld(lineSampPt, elev, worldPt);

}

//*****************************************************************************
//  METHOD: rspfMapProjection::lineSampleToEastingNorthing
//
//*****************************************************************************
void rspfMapProjection::lineSampleToEastingNorthing(const rspfDpt& lineSample,
                                                     rspfDpt&       eastingNorthing)const
{
   // make sure that the passed in lineSample is good and
   // check to make sure our easting northing is good so
   // we can compute the line sample.
   //
   if(lineSample.hasNans()||theUlEastingNorthing.hasNans())
   {
      eastingNorthing.makeNan();
      return;
   }
   rspfDpt deltaPoint = lineSample;

   eastingNorthing.x = theUlEastingNorthing.x + deltaPoint.x*theMetersPerPixel.x;
   eastingNorthing.y = theUlEastingNorthing.y + (-deltaPoint.y)*theMetersPerPixel.y ;

   //   eastingNorthing.x += (lineSample.x*theMetersPerPixel.x);

   // Note:  the Northing is positive up.  In image space
   // the positive axis is down so we must multiply by
   // -1
   //   eastingNorthing.y += (-lineSample.y*theMetersPerPixel.y);
}


void rspfMapProjection::setMetersPerPixel(const rspfDpt& resolution)
{
   theMetersPerPixel = resolution;
   computeDegreesPerPixel();
}

void rspfMapProjection::setDecimalDegreesPerPixel(const rspfDpt& resolution)
{
   theDegreesPerPixel = resolution;
   computeMetersPerPixel();
}

void rspfMapProjection::eastingNorthingToWorld(const rspfDpt& eastingNorthing,
                                                rspfGpt&       worldPt)const
{
   rspfDpt lineSample;
   eastingNorthingToLineSample(eastingNorthing, lineSample);
   lineSampleToWorld(lineSample, worldPt);
}


//*****************************************************************************
//  METHOD: rspfMapProjection::eastingNorthingToLineSample
//
//*****************************************************************************
void rspfMapProjection::eastingNorthingToLineSample(const rspfDpt& eastingNorthing,
                                                     rspfDpt&       lineSample)const
{
   if(eastingNorthing.hasNans())
   {
      lineSample.makeNan();
      return;
   }
   // check the final result to make sure there were no
   // problems.
   //
   if(!eastingNorthing.isNan())
   {
     lineSample.x = (eastingNorthing.x - theUlEastingNorthing.x)/theMetersPerPixel.x;

     // We must remember that the Northing is negative since the positive
     // axis for an image is assumed to go down since it's image space.
     lineSample.y = (-(eastingNorthing.y-theUlEastingNorthing.y))/theMetersPerPixel.y;
   }
}

void rspfMapProjection::setUlTiePoints(const rspfGpt& gpt)
{
   setUlGpt(gpt);
   setUlEastingNorthing(forward(gpt));
}

void rspfMapProjection::setUlTiePoints(const rspfDpt& eastingNorthing)
{
   setUlEastingNorthing(eastingNorthing);
   setUlGpt(inverse(eastingNorthing));
}


//*****************************************************************************
//  METHOD: rspfMapProjection::setUlEastingNorthing
//
//*****************************************************************************
void rspfMapProjection::setUlEastingNorthing(const rspfDpt& ulEastingNorthing)
{
   theUlEastingNorthing = ulEastingNorthing;
}

//*****************************************************************************
//  METHOD: rspfMapProjection::setUlGpt
//
//*****************************************************************************
void rspfMapProjection::setUlGpt(const rspfGpt& ulGpt)
{
   theUlGpt = ulGpt;

   // The rspfGpt data members need to use the same datum as this projection:
   if (*theDatum != *(ulGpt.datum()))
      theUlGpt.changeDatum(theDatum);
}

//*****************************************************************************
//  METHOD: rspfMapProjection::saveState
//
//*****************************************************************************
bool rspfMapProjection::saveState(rspfKeywordlist& kwl, const char* prefix) const
{
   rspfProjection::saveState(kwl, prefix);

   kwl.add(prefix,
           rspfKeywordNames::ORIGIN_LATITUDE_KW,
           theOrigin.latd(),
           true);

   kwl.add(prefix,
           rspfKeywordNames::CENTRAL_MERIDIAN_KW,
           theOrigin.lond(),
           true);

   theEllipsoid.saveState(kwl, prefix);

   if(theDatum)
   {
      kwl.add(prefix,
              rspfKeywordNames::DATUM_KW,
              theDatum->code(),
              true);
   }

   // Calling access method to give it an opportunity to update the code in case of param change:
   rspf_uint32 code = getPcsCode();
   if (code)
   {
      rspfString epsg_spec = rspfString("EPSG:") + rspfString::toString(code);
      kwl.add(prefix, rspfKeywordNames::SRS_NAME_KW, epsg_spec, true);
   }
   
   if(isGeographic())
   {
      kwl.add(prefix,
              rspfKeywordNames::TIE_POINT_XY_KW,
              rspfDpt(theUlGpt).toString().c_str(),
              true);
      kwl.add(prefix,
              rspfKeywordNames::TIE_POINT_UNITS_KW,
              rspfUnitTypeLut::instance()->getEntryString(RSPF_DEGREES),
              true);
      kwl.add(prefix,
              rspfKeywordNames::PIXEL_SCALE_XY_KW,
              theDegreesPerPixel.toString().c_str(),
              true);
      kwl.add(prefix,
              rspfKeywordNames::PIXEL_SCALE_UNITS_KW,
              rspfUnitTypeLut::instance()->getEntryString(RSPF_DEGREES),
              true);
   }
   else
   {
      kwl.add(prefix,
              rspfKeywordNames::TIE_POINT_XY_KW,
              theUlEastingNorthing.toString().c_str(),
              true);
      kwl.add(prefix,
              rspfKeywordNames::TIE_POINT_UNITS_KW,
              rspfUnitTypeLut::instance()->getEntryString(RSPF_METERS),
              true);
      kwl.add(prefix,
              rspfKeywordNames::PIXEL_SCALE_XY_KW,
              theMetersPerPixel.toString().c_str(),
              true);
      kwl.add(prefix,
              rspfKeywordNames::PIXEL_SCALE_UNITS_KW,
              rspfUnitTypeLut::instance()->getEntryString(RSPF_METERS),
              true);  
   }

   kwl.add(prefix, rspfKeywordNames::PCS_CODE_KW, code, true);
   kwl.add(prefix, rspfKeywordNames::FALSE_EASTING_NORTHING_KW,
           theFalseEastingNorthing.toString().c_str(), true);
   kwl.add(prefix, rspfKeywordNames::FALSE_EASTING_NORTHING_UNITS_KW,
           rspfUnitTypeLut::instance()->getEntryString(RSPF_METERS), true);
   kwl.add(prefix, rspfKeywordNames::ELEVATION_LOOKUP_FLAG_KW,
           rspfString::toString(theElevationLookupFlag), true);

   if(theModelTransformUnitType != RSPF_UNIT_UNKNOWN)
   {
      const NEWMAT::Matrix& m = theModelTransform.getData();
      ostringstream out;
      rspf_uint32 row, col;
      for(row = 0; row < 4; ++row)
      {
         for(col = 0; col < 4; ++col)
         {
            out << std::setprecision(20) << m[row][col] << " ";
         }
      }
      kwl.add(prefix,
              rspfKeywordNames::IMAGE_MODEL_TRANSFORM_MATRIX_KW,
              out.str().c_str(),
              true);
      kwl.add(prefix,
              rspfKeywordNames::IMAGE_MODEL_TRANSFORM_UNIT_KW,
              rspfUnitTypeLut::instance()->getEntryString(theModelTransformUnitType),
              true);
   }

   return true;
}

//*****************************************************************************
//  METHOD: rspfMapProjection::loadState
//
//*****************************************************************************
bool rspfMapProjection::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   rspfProjection::loadState(kwl, prefix);

   const char* elevLookupFlag = kwl.find(prefix, rspfKeywordNames::ELEVATION_LOOKUP_FLAG_KW);
   if(elevLookupFlag)
   {
      theElevationLookupFlag = rspfString(elevLookupFlag).toBool();
   }
   // Get the ellipsoid.
   theEllipsoid.loadState(kwl, prefix);

   const char *lookup;

   // Get the Projection Coordinate System (assumed from EPSG database). 
   // NOTE: the code is read here for saving in this object only. 
   // The code is not verified until a call to getPcs() is called. If ONLY this code
   // had been provided, then the EPSG projection factory would populate a new instance of the 
   // corresponding map projection and have it saveState for constructing again later in the 
   // conventional fashion here
   thePcsCode = 0; 
   lookup = kwl.find(prefix, rspfKeywordNames::PCS_CODE_KW);
   if(lookup)
      thePcsCode = rspfString(lookup).toUInt32(); // EPSG PROJECTION CODE

   // The datum can be specified in 2 ways: either via RSPF/geotrans alpha-codes or EPSG code.
   // Last resort use WGS 84 (consider throwing an exception to catch any bad datums): 
   theDatum = rspfDatumFactoryRegistry::instance()->create(kwl, prefix);
   if (theDatum == NULL)
   {
      theDatum = rspfDatumFactory::instance()->wgs84();
   }

   // Set all rspfGpt-type members to use this datum:
   theOrigin.datum(theDatum);
   theUlGpt.datum(theDatum);

   // Fetch the ellipsoid from the datum:
   const rspfEllipsoid* ellipse = theDatum->ellipsoid();
   if(ellipse)
      theEllipsoid = *ellipse;
   
   // Get the latitude of the origin.
   lookup = kwl.find(prefix, rspfKeywordNames::ORIGIN_LATITUDE_KW);
   if (lookup)
   {
      theOrigin.latd(rspfString(lookup).toFloat64());
   }
   // else ???

   // Get the central meridian.
   lookup = kwl.find(prefix, rspfKeywordNames::CENTRAL_MERIDIAN_KW);
   if (lookup)
   {
      theOrigin.lond(rspfString(lookup).toFloat64());
   }
   // else ???


   // Get the pixel scale.
   theMetersPerPixel.makeNan();
   theDegreesPerPixel.makeNan();
   lookup = kwl.find(prefix, rspfKeywordNames::PIXEL_SCALE_UNITS_KW);
   if (lookup)
   {
      rspfUnitType units =
         static_cast<rspfUnitType>(rspfUnitTypeLut::instance()->
                                    getEntryNumber(lookup));
      
      lookup = kwl.find(prefix, rspfKeywordNames::PIXEL_SCALE_XY_KW);
      if (lookup)
      {
         rspfDpt scale;
         scale.toPoint(std::string(lookup));

         switch (units)
         {
            case RSPF_METERS:
            {
               theMetersPerPixel = scale;
               break;
            }
            case RSPF_DEGREES:
            {
               theDegreesPerPixel.x = scale.x;
               theDegreesPerPixel.y = scale.y;
               break;
            }
            case RSPF_FEET:
            case RSPF_US_SURVEY_FEET:
            {
               rspfUnitConversionTool ut;
               ut.setValue(scale.x, units);
               theMetersPerPixel.x = ut.getValue(RSPF_METERS);
               ut.setValue(scale.y, units);
               theMetersPerPixel.y = ut.getValue(RSPF_METERS);
               break;
            }
            default:
            {
               if(traceDebug())
               {
                  // Unhandled unit type!
                  rspfNotify(rspfNotifyLevel_WARN)
                  << "rspfMapProjection::loadState WARNING!"
                  << "Unhandled unit type for "
                  << rspfKeywordNames::PIXEL_SCALE_UNITS_KW << ":  "
                  << ( rspfUnitTypeLut::instance()->
                      getEntryString(units).c_str() )
                  << endl;
               }
               break;
            }
         } // End of switch (units)
         
      }  // End of if (PIXEL_SCALE_XY)

   } // End of if (PIXEL_SCALE_UNITS)
   else
   {
      // BACKWARDS COMPATIBILITY LOOKUPS...
      lookup =  kwl.find(prefix, rspfKeywordNames::METERS_PER_PIXEL_X_KW);
      if(lookup)
      {
         theMetersPerPixel.x = fabs(rspfString(lookup).toFloat64());
      }
      
      lookup =  kwl.find(prefix, rspfKeywordNames::METERS_PER_PIXEL_Y_KW);
      if(lookup)
      {
         theMetersPerPixel.y = fabs(rspfString(lookup).toFloat64());
      }
      
      lookup = kwl.find(prefix,
                        rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LAT);
      if(lookup)
      {
         theDegreesPerPixel.y = fabs(rspfString(lookup).toFloat64());
      }
      
      lookup = kwl.find(prefix,
                        rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LON);
      if(lookup)
      {
         theDegreesPerPixel.x = fabs(rspfString(lookup).toFloat64());
      }
   }            

   // Get the tie point.
   theUlGpt.makeNan();

    // Since this won't be picked up from keywords set to 0 to keep nan out.
   theUlGpt.hgt = 0.0;
   
   theUlEastingNorthing.makeNan();
   lookup = kwl.find(prefix, rspfKeywordNames::TIE_POINT_UNITS_KW);
   if (lookup)
   {
      rspfUnitType units =
         static_cast<rspfUnitType>(rspfUnitTypeLut::instance()->
                                    getEntryNumber(lookup));
      
      lookup = kwl.find(prefix, rspfKeywordNames::TIE_POINT_XY_KW);
      if (lookup)
      {
         rspfDpt tie;
         tie.toPoint(std::string(lookup));

         switch (units)
         {
            case RSPF_METERS:
            {
               theUlEastingNorthing = tie;
               break;
            }
            case RSPF_DEGREES:
            {
               theUlGpt.lond(tie.x);
               theUlGpt.latd(tie.y);
               break;
            }
            case RSPF_FEET:
            case RSPF_US_SURVEY_FEET:
            {
               rspfUnitConversionTool ut;
               ut.setValue(tie.x, units);
               theUlEastingNorthing.x = ut.getValue(RSPF_METERS);
               ut.setValue(tie.y, units);
               theUlEastingNorthing.y = ut.getValue(RSPF_METERS);
               break;
            }
            default:
            {
               if(traceDebug())
               {
                  // Unhandled unit type!
                  rspfNotify(rspfNotifyLevel_WARN)
                  << "rspfMapProjection::loadState WARNING!"
                  << "Unhandled unit type for "
                  << rspfKeywordNames::TIE_POINT_UNITS_KW << ": " 
                  << ( rspfUnitTypeLut::instance()->
                      getEntryString(units).c_str() )
                  << endl;
               }
               break;
            }
         } // End of switch (units)
         
      }  // End of if (TIE_POINT_XY)

   } // End of if (TIE_POINT_UNITS)
   else
   {
      // BACKWARDS COMPATIBILITY LOOKUPS...
      lookup =  kwl.find(prefix, rspfKeywordNames::TIE_POINT_EASTING_KW);
      if(lookup)
      {
         theUlEastingNorthing.x = (rspfString(lookup).toFloat64());
      }

      lookup =  kwl.find(prefix, rspfKeywordNames::TIE_POINT_NORTHING_KW);
      if(lookup)
      {
         theUlEastingNorthing.y = (rspfString(lookup).toFloat64());
      }

      lookup = kwl.find(prefix, rspfKeywordNames::TIE_POINT_LAT_KW);
      if (lookup)
      {
         theUlGpt.latd(rspfString(lookup).toFloat64());
      }

      lookup = kwl.find(prefix, rspfKeywordNames::TIE_POINT_LON_KW);
      if (lookup)
      {
         theUlGpt.lond(rspfString(lookup).toFloat64());
      }
   }
   
   // Get the false easting northing.
   theFalseEastingNorthing.x = 0.0;
   theFalseEastingNorthing.y = 0.0;
   theProjectionUnits = RSPF_METERS;
   lookup = kwl.find(prefix, rspfKeywordNames::FALSE_EASTING_NORTHING_UNITS_KW);
   if (lookup)
   {
      theProjectionUnits =
         static_cast<rspfUnitType>(rspfUnitTypeLut::instance()->getEntryNumber(lookup));
   }

   lookup = kwl.find(prefix, rspfKeywordNames::FALSE_EASTING_NORTHING_KW);
   if (lookup)
   {
      rspfDpt eastingNorthing;
      eastingNorthing.toPoint(std::string(lookup));

      switch (theProjectionUnits)
      {
         case RSPF_METERS:
         {
            theFalseEastingNorthing = eastingNorthing;
            break;
         }
         case RSPF_FEET:
         case RSPF_US_SURVEY_FEET:
         {
            rspfUnitConversionTool ut;
            ut.setValue(eastingNorthing.x, theProjectionUnits);
            theFalseEastingNorthing.x = ut.getValue(RSPF_METERS);
            ut.setValue(eastingNorthing.y, theProjectionUnits);
            theFalseEastingNorthing.y = ut.getValue(RSPF_METERS);
            theProjectionUnits = RSPF_METERS;
            break;
         }
         default:
         {
            if(traceDebug())
            {
               // Unhandled unit type!
               rspfNotify(rspfNotifyLevel_WARN)
                  << "rspfMapProjection::loadState WARNING! Unhandled unit type for "
                  << rspfKeywordNames::FALSE_EASTING_NORTHING_UNITS_KW << ":  " 
                  << (rspfUnitTypeLut::instance()->getEntryString(theProjectionUnits).c_str())
                  << endl;
            }
            break;
         }
      } // End of switch (units)
   }  // End of if (FALSE_EASTING_NORTHING_KW)
   else
   {
      // BACKWARDS COMPATIBILITY LOOKUPS...
      lookup =  kwl.find(prefix, rspfKeywordNames::FALSE_EASTING_KW);
      if(lookup)
      {
         theFalseEastingNorthing.x = (rspfString(lookup).toFloat64());
      }
      
      lookup =  kwl.find(prefix, rspfKeywordNames::FALSE_NORTHING_KW);
      if(lookup)
      {
         theFalseEastingNorthing.y = (rspfString(lookup).toFloat64());
      }
   }            

//    if((theDegreesPerPixel.x!=RSPF_DBL_NAN)&&
//       (theDegreesPerPixel.y!=RSPF_DBL_NAN)&&
//       theMetersPerPixel.hasNans())
//    {
//       theMetersPerPixel    = theOrigin.metersPerDegree();
//       theMetersPerPixel.x *= theDegreesPerPixel.x;
//       theMetersPerPixel.y *= theDegreesPerPixel.y;
//    }

   lookup = kwl.find(prefix, rspfKeywordNames::PIXEL_TYPE_KW);
   if (lookup)
   {
      rspfString pixelType = lookup;
      pixelType=pixelType.trim();
      if(pixelType!="")
      {
         pixelType.downcase();
         if(pixelType.contains("area"))
         {
            if( theMetersPerPixel.hasNans() == false)
            {
               if(!theUlEastingNorthing.hasNans())
               {
                  theUlEastingNorthing.x += (theMetersPerPixel.x*0.5);
                  theUlEastingNorthing.y -= (theMetersPerPixel.y*0.5);
               }
            }
            if(theDegreesPerPixel.hasNans() == false)
            {
               theUlGpt.latd( theUlGpt.latd() - (theDegreesPerPixel.y*0.5) );
               theUlGpt.lond( theUlGpt.lond() + (theDegreesPerPixel.x*0.5) );
            }
         }
      }
   }
   
   theModelTransformUnitType = RSPF_UNIT_UNKNOWN;
   const char* modelTransform = kwl.find(prefix, rspfKeywordNames::IMAGE_MODEL_TRANSFORM_MATRIX_KW);
   const char* modelTransformUnit = kwl.find(prefix, rspfKeywordNames::IMAGE_MODEL_TRANSFORM_UNIT_KW);
   if(modelTransform&&modelTransformUnit) // row ordered 4x4 matrix.  Should be 16 values
   {
      
      NEWMAT::Matrix& m = theModelTransform.getData();
      istringstream in(modelTransform);
      rspf_uint32 row, col;
      rspfString value;
      for(row = 0; row < 4; ++row)
      {
         for(col = 0; col < 4; ++col)
         {
            in >> value;
            m[row][col] = value.toDouble();
         }
      }
      // make sure these have the identity and all unused are 0.0
      m[2][2] = 1.0;
      m[2][0] = 0.0;
      m[2][1] = 0.0;
      m[2][3] = 0.0;
      m[3][3] = 1.0;
      m[3][2] = 0.0;
      m[3][1] = 0.0;
      m[3][0] = 0.0;
      
      if(!in.fail())
      {
         try
         {
            theInverseModelTransform = theModelTransform;
            theInverseModelTransform.i();
            theModelTransformUnitType = static_cast<rspfUnitType>(rspfUnitTypeLut::instance()->
                                                                   getEntryNumber(modelTransformUnit));
         }
         catch(...)
         {
            theModelTransformUnitType = RSPF_UNIT_UNKNOWN;   
         }
      }
   }

   //---
   // Set the datum of the origin and tie point.
   // Use method that does NOT perform a shift.
   //---
   if(theDatum)
   {
      theOrigin.datum(theDatum);
      theUlGpt.datum(theDatum);
   }

   if(theMetersPerPixel.hasNans() &&
      theDegreesPerPixel.hasNans())
   {
      rspfDpt mpd = rspfGpt().metersPerDegree();
      if(isGeographic())
      {
         theDegreesPerPixel.lat = 1.0/mpd.y;
         theDegreesPerPixel.lon = 1.0/mpd.y;
      }
      else
      {
         theMetersPerPixel.x = 1.0;
         theMetersPerPixel.y = 1.0;
      }
   }

   //---
   // Final sanity check:
   //---
   if ( theOrigin.hasNans() )
   {
      if ( theModelTransformUnitType == RSPF_DEGREES )
      {
         const NEWMAT::Matrix& m = theModelTransform.getData();
         theOrigin.lon = m[0][3];
         theOrigin.lat = m[1][3];
      }
      else
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << __FILE__ << ": " << __LINE__
            << "\nrspfMapProjection::loadState ERROR: Origin is not set!"
            << std::endl;
      }
   }

   return true;
}

//*****************************************************************************
//  METHOD: rspfMapProjection::print
//*****************************************************************************
std::ostream& rspfMapProjection::print(std::ostream& out) const
{
   const char MODULE[] = "rspfMapProjection::print";

   out << setiosflags(ios::fixed) << setprecision(15)
       << "\n// " << MODULE
       << "\n" << rspfKeywordNames::TYPE_KW               << ":  "
       << getClassName()
       << "\n" << rspfKeywordNames::MAJOR_AXIS_KW         << ":  "
       << theEllipsoid.getA()
       << "\n" << rspfKeywordNames::MINOR_AXIS_KW         << ":  "
       << theEllipsoid.getB()
       << "\n" << rspfKeywordNames::ORIGIN_LATITUDE_KW    << ":  "
       << theOrigin.latd()
       << "\n" << rspfKeywordNames::CENTRAL_MERIDIAN_KW   << ":  "
       << theOrigin.lond()
       << "\norigin: " << theOrigin
       << "\n" << rspfKeywordNames::DATUM_KW              << ":  "
       << (theDatum?theDatum->code().c_str():"unknown")
       << "\n" << rspfKeywordNames::METERS_PER_PIXEL_X_KW << ":  "
       << ((rspf::isnan(theMetersPerPixel.x))?rspfString("nan"):rspfString::toString(theMetersPerPixel.x, 15))
       << "\n" << rspfKeywordNames::METERS_PER_PIXEL_Y_KW << ":  "
       << ((rspf::isnan(theMetersPerPixel.y))?rspfString("nan"):rspfString::toString(theMetersPerPixel.y, 15))
       << "\n" << rspfKeywordNames::FALSE_EASTING_NORTHING_KW << ": "
       << theFalseEastingNorthing.toString().c_str()
       << "\n" << rspfKeywordNames::FALSE_EASTING_NORTHING_UNITS_KW << ": "
       << rspfUnitTypeLut::instance()->getEntryString(RSPF_METERS)
       << "\n" << rspfKeywordNames::PCS_CODE_KW << ": " << thePcsCode;

   if(isGeographic())
   {
      out << "\n" << rspfKeywordNames::TIE_POINT_XY_KW << ": " 
          << rspfDpt(theUlGpt).toString().c_str()
          << "\n" << rspfKeywordNames::TIE_POINT_UNITS_KW << ": " 
          << rspfUnitTypeLut::instance()->getEntryString(RSPF_DEGREES)
          << "\n" << rspfKeywordNames::PIXEL_SCALE_XY_KW << ": "
          << theDegreesPerPixel.toString().c_str()
          << "\n" << rspfKeywordNames::PIXEL_SCALE_UNITS_KW << ": "
          << rspfUnitTypeLut::instance()->getEntryString(RSPF_DEGREES)
          << std::endl;
   }
   else
   {
      out << "\n" << rspfKeywordNames::TIE_POINT_XY_KW << ": " 
          << theUlEastingNorthing.toString().c_str()
          << "\n" << rspfKeywordNames::TIE_POINT_UNITS_KW << ": " 
          << rspfUnitTypeLut::instance()->getEntryString(RSPF_METERS)
          << "\n" << rspfKeywordNames::PIXEL_SCALE_XY_KW << ": "
          << theMetersPerPixel.toString().c_str()
          << "\n" << rspfKeywordNames::PIXEL_SCALE_UNITS_KW << ": "
          << rspfUnitTypeLut::instance()->getEntryString(RSPF_METERS)
          << std::endl;
   }
   
   return rspfProjection::print(out);
}

//*****************************************************************************
//  METHOD: rspfMapProjection::computeDegreesPerPixel
//
//*****************************************************************************
void rspfMapProjection::computeDegreesPerPixel()
{
   rspfDpt eastNorthGround = forward(theOrigin);
   rspfDpt rightEastNorth  =  eastNorthGround;
   rspfDpt downEastNorth   =  eastNorthGround;
   rightEastNorth.x += theMetersPerPixel.x;
   downEastNorth.y  -= theMetersPerPixel.y;

   rspfGpt rightGpt = inverse(rightEastNorth);
   rspfGpt downGpt  = inverse(downEastNorth);

   // use euclidean distance to get length along the horizontal (lon)
   // and vertical (lat) directions
   //
   double tempDeltaLat = rightGpt.latd() - theOrigin.latd();
   double tempDeltaLon = rightGpt.lond() - theOrigin.lond();
   theDegreesPerPixel.lon = sqrt(tempDeltaLat*tempDeltaLat + tempDeltaLon*tempDeltaLon);

   tempDeltaLat = downGpt.latd() - theOrigin.latd();
   tempDeltaLon = downGpt.lond() - theOrigin.lond();
   theDegreesPerPixel.lat = sqrt(tempDeltaLat*tempDeltaLat + tempDeltaLon*tempDeltaLon);
}

//*****************************************************************************
//  METHOD: rspfMapProjection::computeMetersPerPixel
//
//*****************************************************************************
void rspfMapProjection::computeMetersPerPixel()
{
//#define USE_RSPFGPT_METERS_PER_DEGREE
#ifdef USE_RSPFGPT_METERS_PER_DEGREE
   rspfDpt metersPerDegree (theOrigin.metersPerDegree());
   theMetersPerPixel.x = metersPerDegree.x * theDegreesPerPixel.lon;
   theMetersPerPixel.y = metersPerDegree.y * theDegreesPerPixel.lat;
#else
   rspfGpt right=theOrigin;
   rspfGpt down=theOrigin;

   down.latd(theOrigin.latd()  + theDegreesPerPixel.lat);
   right.lond(theOrigin.lond() + theDegreesPerPixel.lon);

   rspfDpt centerMeters = forward(theOrigin);
   rspfDpt rightMeters = forward(right);
   rspfDpt downMeters  = forward(down);

   theMetersPerPixel.x = (rightMeters - centerMeters).length();
   theMetersPerPixel.y = (downMeters  - centerMeters).length();
#endif
}

//**************************************************************************************************
//  METHOD: rspfMapProjection::operator==
//! Compares this to arg projection and returns TRUE if the same. 
//! NOTE: As currently implemented in RSPF, map projections also contain image geometry 
//! information like tiepoint and scale. This operator is only concerned with the map 
//! specification and ignores image geometry differences.
//**************************************************************************************************
bool rspfMapProjection::operator==(const rspfProjection& projection) const
{
   // Verify that derived types match:
   if (getClassName() != projection.getClassName())
      return false;

   // If both PCS codes are non-zero, that's all we need to check:
   const rspfMapProjection* mapProj = PTR_CAST(rspfMapProjection, &projection);
   if (thePcsCode && mapProj->thePcsCode && (thePcsCode != 32767) && 
       (thePcsCode == mapProj->thePcsCode) )
   {
      return true;
   }

   if ( *theDatum != *(mapProj->theDatum) )
      return false;
   
   if (theOrigin != mapProj->theOrigin)
      return false;

   if (theFalseEastingNorthing != mapProj->theFalseEastingNorthing)
      return false;

#if 0
   THIS SECTION IGNORED SINCE IT DEALS WITH IMAGE GEOMETRY, NOT MAP PROJECTION
   if (isGeographic())
   {
      if ((theDegreesPerPixel != mapProj->theDegreesPerPixel) ||
          (theUlGpt != mapProj->theUlGpt))
         return false;
   }
   else
   {
      if ((theMetersPerPixel != mapProj->theMetersPerPixel) ||
         (theUlEastingNorthing != mapProj->theUlEastingNorthing))
         return false;
   }
#endif

   // Units must match:
   if ((theProjectionUnits != RSPF_UNIT_UNKNOWN) && 
       (mapProj->theProjectionUnits != RSPF_UNIT_UNKNOWN) &&
       (theProjectionUnits != mapProj->theProjectionUnits))
       return false;

   // Check transform if present and compare it also:
   if (hasModelTransform() && mapProj->hasModelTransform() &&
      (theModelTransform.getData() != mapProj->theModelTransform.getData()))
      return false;

   return true;
}

bool rspfMapProjection::isEqualTo(const rspfObject& obj, rspfCompareType compareType)const
{
   const rspfMapProjection* mapProj = dynamic_cast<const rspfMapProjection*>(&obj);
   bool result = mapProj&&rspfProjection::isEqualTo(obj, compareType);
   
   if(result)
   {
      result = (theEllipsoid.isEqualTo(mapProj->theEllipsoid, compareType)&&
                theOrigin.isEqualTo(mapProj->theOrigin, compareType)&&
                theMetersPerPixel.isEqualTo(mapProj->theMetersPerPixel, compareType)&&             
                theDegreesPerPixel.isEqualTo(mapProj->theDegreesPerPixel, compareType)&&             
                theUlGpt.isEqualTo(mapProj->theUlGpt, compareType)&&             
                theUlEastingNorthing.isEqualTo(mapProj->theUlEastingNorthing, compareType)&&             
                theFalseEastingNorthing.isEqualTo(mapProj->theFalseEastingNorthing, compareType)&&             
                (thePcsCode == mapProj->thePcsCode)&&
                (theElevationLookupFlag == mapProj->theElevationLookupFlag)&&
                (theElevationLookupFlag == mapProj->theElevationLookupFlag)&&
                (theModelTransform.isEqualTo(mapProj->theModelTransform))&&
                (theInverseModelTransform.isEqualTo(mapProj->theInverseModelTransform))&&
                (theModelTransformUnitType == mapProj->theModelTransformUnitType)&&
                (theProjectionUnits == mapProj->theProjectionUnits));
      
      if(result)
      {
         if(compareType == RSPF_COMPARE_FULL)
         {
            if(theDatum&&mapProj->theDatum)
            {
               result = theDatum->isEqualTo(*mapProj->theDatum, compareType);
            }
         }
         else 
         {
            result = (theDatum==mapProj->theDatum);
         }
      }
   }
   return result;
}

double rspfMapProjection::getFalseEasting() const
{
   return theFalseEastingNorthing.x;
}

double rspfMapProjection::getFalseNorthing() const
{
   return theFalseEastingNorthing.y;
}

double rspfMapProjection::getStandardParallel1() const
{
   return 0.0;
}

double rspfMapProjection::getStandardParallel2() const
{
   return 0.0;
}

void rspfMapProjection::snapTiePointTo(rspf_float64 multiple,
                                        rspfUnitType unitType)
{
   rspf_float64 convertedMultiple = multiple;
   
   if (isGeographic() && (unitType != RSPF_DEGREES) )
   {
      // Convert to degrees.
      rspfUnitConversionTool convertor;
      convertor.setOrigin(theOrigin);
      convertor.setValue(multiple, unitType);
      convertedMultiple = convertor.getDegrees();
   }
   else if ( !isGeographic() && (unitType != RSPF_METERS) )
   {
      // Convert to meters.
      rspfUnitConversionTool convertor;
      convertor.setOrigin(theOrigin);
      convertor.setValue(multiple, unitType);
      convertedMultiple = convertor.getMeters();
   }

   // Convert the tie point.
   if (isGeographic())
   {
      // Snap the latitude.
      rspf_float64 d = theUlGpt.latd();
      d = rspf::round<int>(d / convertedMultiple) * convertedMultiple;
      theUlGpt.latd(d);

      // Snap the longitude.
      d = theUlGpt.lond();
      d = rspf::round<int>(d / convertedMultiple) * convertedMultiple;
      theUlGpt.lond(d);

      // Adjust the stored easting / northing.
      theUlEastingNorthing = forward(theUlGpt);
   }
   else
   {
      // Snap the easting.
      rspf_float64 d = theUlEastingNorthing.x - getFalseEasting();
      d = rspf::round<int>(d / convertedMultiple) * convertedMultiple;
      theUlEastingNorthing.x = d + getFalseEasting();

      // Snap the northing.
      d = theUlEastingNorthing.y - getFalseNorthing();
      d = rspf::round<int>(d / convertedMultiple) * convertedMultiple;
      theUlEastingNorthing.y = d + getFalseNorthing();

      // Adjust the stored upper left ground point.
      theUlGpt = inverse(theUlEastingNorthing);
   }
}

void rspfMapProjection::snapTiePointToOrigin()
{
   // Convert the tie point.
   if (isGeographic())
   {
      // Note the origin may not be 0.0, 0.0:
      
      // Snap the latitude.
      rspf_float64 d = theUlGpt.latd() - origin().latd();
      d = rspf::round<int>(d / theDegreesPerPixel.y) * theDegreesPerPixel.y;
      theUlGpt.latd(d + origin().latd());

      // Snap the longitude.
      d = theUlGpt.lond() - origin().lond();
      d = rspf::round<int>(d / theDegreesPerPixel.x) * theDegreesPerPixel.x;
      theUlGpt.lond(d + origin().lond());

      // Adjust the stored easting / northing.
      theUlEastingNorthing = forward(theUlGpt);
   }
   else
   {
      // Snap the easting.
      rspf_float64 d = theUlEastingNorthing.x - getFalseEasting();
      d = rspf::round<int>(d / theMetersPerPixel.x) * theMetersPerPixel.x;
      theUlEastingNorthing.x = d + getFalseEasting();

      // Snap the northing.
      d = theUlEastingNorthing.y - getFalseNorthing();
      d = rspf::round<int>(d / theMetersPerPixel.y) * theMetersPerPixel.y;
      theUlEastingNorthing.y = d + getFalseNorthing();

      // Adjust the stored upper left ground point.
      theUlGpt = inverse(theUlEastingNorthing);
   }
}

void rspfMapProjection::setElevationLookupFlag(bool flag)
{
   theElevationLookupFlag = flag;
}

bool rspfMapProjection::getElevationLookupFlag()const
{
   return theElevationLookupFlag;
}
   

