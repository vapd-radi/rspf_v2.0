//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Ken Melero
// 
// Description: Container class for a tiff world file data.
//
//********************************************************************
// $Id: rspfTiffWorld.h 18693 2011-01-17 18:49:15Z dburken $
#ifndef rspfTiffWorld_HEADER
#define rspfTiffWorld_HEADER

#include <iostream>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfDpt.h>
using namespace std;

//***************************************************************************
// CLASS:  rspfTiffWorld
//***************************************************************************
class RSPFDLLEXPORT rspfTiffWorld
{
public:
   rspfTiffWorld();
   rspfTiffWorld(const char* source, 
		  rspfPixelType ptype = RSPF_PIXEL_IS_POINT,
		  rspfUnitType  unit = RSPF_METERS);
   
   ~rspfTiffWorld();
   
   bool open(const rspfFilename& file,
             rspfPixelType ptype,
             rspfUnitType unit);
   
   bool saveToOssimGeom(rspfKeywordlist& kwl, const char* prefix=NULL)const;
   bool loadFromOssimGeom(const rspfKeywordlist& kwl, const char* prefix=NULL);
   
   std::ostream& print(std::ostream& out) const;
   
   friend RSPFDLLEXPORT std::ostream& operator<<(std::ostream& out,
                                                  const rspfTiffWorld& obj);
   
   /*!
    * transformed.x =
    * imagePoint.x*theX_scale + imagePoint.y*the3rdValue + theTranslateX
    * 
    * transformed.y =
    * imagePoint.x*the2ndValue + imagePoint.y*theY_scale + theTranslateY
    */
   void forward(const rspfDpt& imagePoint,
                rspfDpt& transformedPoint);
   
   //! Converts world file parameters into x, y scale (for use in affine transform) 
   const rspfDpt& getScale() const { return theComputedScale; }
   
   //! Converts world file parameters into RH rotation in radians (for use in affine transform) 
   double getRotation() const { return theComputedRotation; }
   
   //! Provides access to the translation (for use in affine transform) 
   const rspfDpt& getTranslation() const { return theTranslation; }

protected:
   double theXform1;     
   double theXform2;   
   double theXform3;   
   double theXform4;   
   rspfDpt theTranslation;

   rspfPixelType thePixelType;
   rspfUnitType  theUnit;
   rspfDpt theComputedScale;
   double   theComputedRotation;  //!< Radians
};

#endif
