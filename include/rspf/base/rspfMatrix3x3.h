//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts (gpotts@imagelinks.com)
//
// Description:
//
// Helper class for using NEWMAT::Matrix's.
//
//*******************************************************************
//  $Id: rspfMatrix3x3.h 14789 2009-06-29 16:48:14Z dburken $

#ifndef rspfMatrix3x3_HEADER
#define rspfMatrix3x3_HEADER

#include <iostream>

#include <rspf/matrix/newmat.h>
#include <rspf/matrix/newmatio.h>
#include <rspf/base/rspfConstants.h>

class rspfColumnVector3d;

class RSPFDLLEXPORT rspfMatrix3x3
{
 public:

   
   /*!
    *  Uses the matrix package to compute the eigenvalues for this
    *  matrix
    */ 
   rspfColumnVector3d getEigenValues(const NEWMAT::Matrix& rhs);

   /*!
    *  Returns a blank 3x3 matrix.
    */
   static NEWMAT::Matrix create();

   /*!
    *  Returns a copy of "m".
    *  Note:
    *  If "m" is not a 3x3 that's an error and a blank matrix is returned.
    */
   static NEWMAT::Matrix create(const NEWMAT::Matrix& m);

   /*!
    *  Create a 3x3 with assigned values.
    */
   static NEWMAT::Matrix create(double v00, double v01, double v02,
                                double v10, double v11, double v12,
                                double v20, double v21, double v22);
   
   static NEWMAT::Matrix createIdentity();
   
   static NEWMAT::Matrix createZero();
   static NEWMAT::Matrix createRotationMatrix(double angleX,
                                              double angleY,
                                              double angleZ,
                                              rspfCoordSysOrientMode orientationMode=RSPF_RIGHT_HANDED);
   static NEWMAT::Matrix createRotationXMatrix(double angle,
                                               rspfCoordSysOrientMode orientationMode=RSPF_RIGHT_HANDED);
   static NEWMAT::Matrix createRotationYMatrix(double angle,
                                               rspfCoordSysOrientMode orientationMode=RSPF_RIGHT_HANDED);
   static NEWMAT::Matrix createRotationZMatrix(double angle,
                                               rspfCoordSysOrientMode orientationMode=RSPF_RIGHT_HANDED);

   static NEWMAT::Matrix createScaleMatrix(double X, double Y, double Z);

   static NEWMAT::Matrix createTranslationMatrix(double dx,double dy);
   
protected:
   rspfMatrix3x3(){}  // To squash un-warranted compiler warnings...
   
private:
   rspfMatrix3x3(const rspfMatrix3x3& rhs);
   const rspfMatrix3x3& operator=(const rspfMatrix3x3& rhs);
};

#endif
