//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// AUTHOR: Oscar Kramer (okramer@imagelinks.com)
//
// DESCRIPTION: Contains implementation of class rspfAffineTransform
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfAffineTransform.cpp 15766 2009-10-20 12:37:09Z gpotts $

#include <cstdlib>
#include <rspf/base/rspfAffineTransform.h>
RTTI_DEF1(rspfAffineTransform, "rspfAffineTransform", rspf2dTo2dTransform);

#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfMatrix3x3.h>
#include <rspf/base/rspfString.h>
//*****************************************************************************
//  CONSTRUCTOR: 
//*****************************************************************************
rspfAffineTransform::rspfAffineTransform()
   :
      rspf2dTo2dTransform(),
      theScale(1.0, 1.0),
      theRotation(0.0),
      theTranslation(0.0, 0.0),
      theAffineEnabledFlag(true),      
      theForwardXform(3, 3),
      theInverseXform(3, 3)
{
   setIdentity();
}

rspfAffineTransform::rspfAffineTransform(const rspfAffineTransform& src)
   :
      rspf2dTo2dTransform(src),
      theScale(src.theScale),
      theRotation(src.theRotation),
      theTranslation(src.theTranslation),
      theAffineEnabledFlag(src.theAffineEnabledFlag),
      theForwardXform(src.theForwardXform),
      theInverseXform(src.theInverseXform)
{
}

const rspfAffineTransform& rspfAffineTransform::operator=(
   const rspfAffineTransform& rhs)
{
   if (this != &rhs)
   {
      rspf2dTo2dTransform::operator=(rhs);
      
      theScale             = rhs.theScale;
      theRotation          = rhs.theRotation;
      theTranslation       = rhs.theTranslation;
      theAffineEnabledFlag = rhs.theAffineEnabledFlag;
      theForwardXform      = rhs.theForwardXform;
      theInverseXform      = rhs.theInverseXform;
   }
   return *this;
}

//*****************************************************************************
//  METHOD: 
//*****************************************************************************
void rspfAffineTransform::forward(const rspfDpt& input,
                                   rspfDpt& output) const
{
   if(theAffineEnabledFlag)
   {
      output.x = theForwardXform[0][0]*input.x+
                 theForwardXform[0][1]*input.y+
                 theForwardXform[0][2];
      output.y = theForwardXform[1][0]*input.x+
                 theForwardXform[1][1]*input.y+
                 theForwardXform[1][2];
   }
   else
   {
      output = input;
   }
}

//*****************************************************************************
//  METHOD: 
//*****************************************************************************
void rspfAffineTransform::inverse(const rspfDpt& input,
                                   rspfDpt&       output) const
{
   if(theAffineEnabledFlag)
   {
      output.x = theInverseXform[0][0]*input.x+
                 theInverseXform[0][1]*input.y+
                 theInverseXform[0][2];
      output.y = theInverseXform[1][0]*input.x+
                 theInverseXform[1][1]*input.y+
                 theInverseXform[1][2];
   }
   else
   {
      output = input;
   }
}

//*****************************************************************************
//  METHOD: 
//*****************************************************************************
bool rspfAffineTransform::saveState(rspfKeywordlist& kwl,
                                     const char* prefix) const
{
   kwl.add(prefix,
           rspfKeywordNames::SCALE_X_KW,
           theScale.x,
           true);
   kwl.add(prefix,
           rspfKeywordNames::SCALE_Y_KW,
           theScale.y,
           true);
   kwl.add(prefix,
           rspfKeywordNames::TRANSLATION_X_KW,
           theTranslation.x,
           true);
   kwl.add(prefix,
           rspfKeywordNames::TRANSLATION_Y_KW,
           theTranslation.y,
           true);
   kwl.add(prefix,
           rspfKeywordNames::ROTATION_KW,
           theRotation,
           true);

   rspf2dTo2dTransform::saveState(kwl, prefix);
   
   return true;
}

//*****************************************************************************
//  METHOD: 
//*****************************************************************************
bool rspfAffineTransform::loadState(const rspfKeywordlist& kwl,
               const char* prefix)
{
   const char* buf;

   if (!rspf2dTo2dTransform::loadState(kwl, prefix))
      goto BAD_KEYWORD;
       
   buf= kwl.find(prefix, rspfKeywordNames::SCALE_X_KW);
   if (!buf) goto BAD_KEYWORD;
   theScale.x = std::atof(buf);

   buf= kwl.find(prefix, rspfKeywordNames::SCALE_Y_KW);
   if (!buf) goto BAD_KEYWORD;
   theScale.y = std::atof(buf);

   buf= kwl.find(prefix, rspfKeywordNames::TRANSLATION_X_KW);
   if (!buf) goto BAD_KEYWORD;
   theTranslation.x = std::atof(buf);

   buf= kwl.find(prefix, rspfKeywordNames::TRANSLATION_Y_KW);
   if (!buf) goto BAD_KEYWORD;
   theTranslation.y = std::atof(buf);

   buf= kwl.find(prefix, rspfKeywordNames::ROTATION_KW);
   if (!buf) goto BAD_KEYWORD;
   theRotation = std::atof(buf);

   computeMatrix();   
   return true;

 BAD_KEYWORD:
   return false;
}
   
//*****************************************************************************
//  METHOD: 
//*****************************************************************************
void rspfAffineTransform::setMatrix(double rotation,
                                     const rspfDpt& scale,
                                     const rspfDpt& translation)
{
   theScale       = scale;
   theRotation    = rotation;
   theTranslation = translation;
   
   computeMatrix();
}
   
//*****************************************************************************
//  METHOD: 
//*****************************************************************************
void rspfAffineTransform::setIdentity()
{
   theScale.x       = 1;
   theScale.y       = 1;
   theTranslation.x = 0;
   theTranslation.y = 0;
   theRotation      = 0.0;
   
   computeMatrix();
}

//*****************************************************************************
//  METHOD: 
//*****************************************************************************
void rspfAffineTransform::computeMatrix()
{
   theForwardXform =
      rspfMatrix3x3::createTranslationMatrix(theTranslation.x, 
                                              theTranslation.y)* 
      rspfMatrix3x3::createRotationZMatrix(theRotation)* 
      rspfMatrix3x3::createScaleMatrix(theScale.x,
                                        theScale.y,
                                        1.0);

   //***
   // The inverse transform is just the matrix inverse:
   //***
   theInverseXform = theForwardXform.i();
}


//*****************************************************************************
//  METHOD: 
//*****************************************************************************
std::ostream& rspfAffineTransform::print(std::ostream& os) const
{
   os << "rspfAffineTransform:"
      << "\n  theScale       = " << theScale
      << "\n  theTranslation = " << theTranslation
      << "\n  theRotation    = " << theRotation << std::endl;
   return os;
}
