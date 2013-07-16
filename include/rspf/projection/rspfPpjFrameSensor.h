//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Dave Hicks
//
// Description:  PPJ Frame Model
//
//*******************************************************************
//  $Id$
#ifndef rspfPpjFrameSensor_HEADER
#define rspfPpjFrameSensor_HEADER 1
#include <rspf/projection/rspfSensorModel.h>

class RSPF_DLL rspfPpjFrameSensor : public rspfSensorModel
{
public:
   rspfPpjFrameSensor();

   rspfPpjFrameSensor(const rspfPpjFrameSensor& src);

   virtual rspfObject* dup()const;
   
   virtual void imagingRay(const rspfDpt& image_point,
                           rspfEcefRay&   image_ray) const;
   
   virtual void lineSampleToWorld(const rspfDpt& image_point,
                                 rspfGpt&       worldPoint) const;
   
   virtual void lineSampleHeightToWorld(const rspfDpt& image_point,
                                const double&   heightEllipsoid,
                                rspfGpt&       worldPoint) const;

   virtual void worldToLineSample(const rspfGpt& world_point,
                                  rspfDpt&       image_point) const;

   virtual void updateModel();
   
   void setFocalLength(double focX, double focY);

   void setCameraPosition(const rspfGpt& value);
   
   void setPrincipalPoint(const rspfDpt& value);

   void setecef2CamMatrix(const NEWMAT::Matrix& value);
   void setAveragePrjectedHeight(double averageProjectedHeight);

   inline virtual bool useForward()const {return true;}
   
   virtual void initAdjustableParameters();
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=0)const;
   
protected:
   NEWMAT::Matrix m_ecef2Cam;
   NEWMAT::Matrix m_ecef2CamInverse;
   rspfDpt       m_principalPoint;   // pixels
   double         m_focalLengthX;     // pixels
   double         m_focalLengthY;     // pixels
   double         m_focalLength;      // pixels
   rspfEcefPoint m_ecefCameraPosition;
   rspfGpt       m_cameraPositionEllipsoid;

   // Not used yet
   double         m_radialK1;
   double         m_radialK2;
   double         m_radialP1;
   double         m_radialP2;
   
   rspfGpt       m_adjustedCameraPosition;
   double         m_adjustedFocalLength;
   double         m_averageProjectedHeight;
   TYPE_DATA
};

#endif /* #ifndef rspfPpjFrameSensor_HEADER */
