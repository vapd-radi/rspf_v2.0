//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Dave Hicks
//
// Description:  Alpha Sensor Base Class
//
//*******************************************************************
//  $Id$
#ifndef rspfAlphaSensor_HEADER
#define rspfAlphaSensor_HEADER 1
#include <rspf/projection/rspfSensorModel.h>

class RSPF_DLL rspfAlphaSensor : public rspfSensorModel
{
public:
   rspfAlphaSensor();

   rspfAlphaSensor(const rspfAlphaSensor& src);

   virtual rspfObject* dup()const;
   
      
   virtual void lineSampToWorld(const rspfDpt& image_point,
                                rspfGpt&       worldPoint) const;

   virtual void lineSampleHeightToWorld(const rspfDpt& image_point,
                                const double&   heightEllipsoid,
                                rspfGpt&       worldPoint) const;

   virtual void updateModel();

   inline virtual bool useForward()const {return true;}
   
   virtual void initAdjustableParameters();
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=0)const;

   void setFov(const double fov);
   void setRollBias(const double rollBias);
   void setPitchBias(const double pitchBias);
   void setHeadingBias(const double headingBias);
   void setSlitRot(const double slitRot);

   void setRollPoly(const std::vector< rspf_float64 > rollPoly);
   void setPitchPoly(const std::vector< rspf_float64 > pitchPoly);
   void setHeadingPoly(const std::vector< rspf_float64 > headingPoly);
   void setLonPoly(const std::vector< rspf_float64 > lonPoly);
   void setLatPoly(const std::vector< rspf_float64 > latPoly);
   void setAltPoly(const std::vector< rspf_float64 > altPoly);
   void setScanPoly(const std::vector< rspf_float64 > scanPoly);

   void getPositionOrientation(const rspf_float64& line,
                                     rspfEcefPoint& pos,
                                     NEWMAT::Matrix& cam2EcfRot)const;

   rspfEcefPoint getCameraPosition(const rspf_float64& line)const;

   NEWMAT::Matrix getPlatform2LocalRot(const rspf_float64& line)const;

   rspf_float64  evalPoly(const std::vector<rspf_float64>& polyCoef,
                           const rspf_float64& line)const;
   
protected:
   NEWMAT::Matrix formHPRmat(const rspf_float64& roll,
                             const rspf_float64& pitch,
                             const rspf_float64& heading)const;

   NEWMAT::Matrix formLLAmat(const rspf_float64& lat,
                             const rspf_float64& lon,
                             const rspf_float64& az)const;

   rspf_float64 m_rollBias;
   rspf_float64 m_pitchBias;
   rspf_float64 m_headingBias;
   rspf_float64 m_fov;
   rspf_float64 m_slitRot;

   rspf_float64 m_focalLength;

   std::vector< rspf_float64 > m_rollPoly;
   std::vector< rspf_float64 > m_pitchPoly;
   std::vector< rspf_float64 > m_headingPoly;
   std::vector< rspf_float64 > m_lonPoly;
   std::vector< rspf_float64 > m_latPoly;
   std::vector< rspf_float64 > m_altPoly;
   std::vector< rspf_float64 > m_scanPoly;

   // Bias rotation matrix
   NEWMAT::Matrix m_cam2Platform;
  
   rspf_float64 m_adjustedFocalLength;
   TYPE_DATA
};

#endif // #ifndef rspfAlphaSensor_HEADER
