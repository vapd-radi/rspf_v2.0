//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt.
//
// Author: Garrett Potts
// 
//********************************************************************
// $Id: rspfFontInformation.h 9094 2006-06-13 19:12:40Z dburken $
#ifndef rspfFontInformation_HEADER
#define rspfFontInformation_HEADER

#include <iosfwd>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfIpt.h>

class rspfKeywordlist;

class RSPFDLLEXPORT rspfFontInformation
{
public:
   friend std::ostream& operator << (std::ostream& out,
                                     const rspfFontInformation& rhs);

   rspfFontInformation();

   rspfFontInformation(const rspfString& family,
                        const rspfString& style,
                        const rspfIpt&    pointSize,
                        bool               fixedFlag,
                        const rspfDpt&    scale=rspfDpt(1.0,1.0),
                        double             rotation = 0.0,
                        const rspfDpt&    shear=rspfDpt(0.0, 0.0));

   rspfFontInformation(const rspfFontInformation& rhs);

   bool isFixed()const;

   /**
    * Saves the current state of this object.
    *
    * For keywords see loadState:
    */
   bool saveState(rspfKeywordlist& kwl, const char* prefix=0)const;

   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    *
    * Keywords:
    *
    * family_name:
    * style_name:
    * fixed_flag:
    * point_size: ( x, y )
    * scale: ( x, y )
    * shear: ( x, y )
    * rotation:
    *
    * point_size_x:  (deprecated)
    * point_size_y:  (deprecated)
    * scale_x: (deprecated)
    * scale_y: (deprecated)
    * shear_x: (deprecated)
    * shear_y: (deprecated)
    */   
   bool loadState(const rspfKeywordlist& kwl,
                  const char* prefix=0);

   bool operator ==(const rspfFontInformation& rhs)const;

   bool operator !=(const rspfFontInformation& rhs)const;

   const rspfFontInformation& operator =(const rspfFontInformation& rhs);
   
   static const char* FAMILY_NAME_KW;
   static const char* STYLE_NAME_KW;
   static const char* POINT_SIZE_KW;   // point_size: ( x, y ) 
   static const char* POINT_SIZE_X_KW; // deprecated
   static const char* POINT_SIZE_Y_KW; // deprecated
   static const char* FIXED_FLAG_KW;
   static const char* SHEAR_KW;        // shear: ( x, y ) 
   static const char* SHEAR_X_KW;      // deprecated
   static const char* SHEAR_Y_KW;      // deprecated
   static const char* SCALE_KW;        // scale: ( x, y ) 
   static const char* SCALE_X_KW;      // deprecated
   static const char* SCALE_Y_KW;      // deprecated
   static const char* ROTATION_KW;
   
   
   rspfString    theFamilyName;
   rspfString    theStyleName;
   rspfIpt       thePointSize;
   bool           theFixedFlag;
   rspfDpt       theScale;
   double         theRotation;
   rspfDpt       theShear;
};

#endif
