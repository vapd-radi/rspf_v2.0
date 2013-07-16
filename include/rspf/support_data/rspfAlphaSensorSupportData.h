#ifndef rspfAlphaSensorSupportData_HEADER
#define rspfAlphaSensorSupportData_HEADER
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/matrix/newmat.h>
#include <rspf/base/rspfPolynom.h>
#include <iostream>

class rspfEnviHeader;

class RSPF_DLL rspfAlphaSensorSupportData : public rspfObject
{
public:
   enum PolynomialDegrees
   {
      MAX_DEGREE     = 10,
      MAX_TERMS      = MAX_DEGREE + 1,
      SCAN_ANGLE_DEG = 2,
      ROLL_DEG       = 7,
      PITCH_DEG      = 5,
      HEADING_DEG    = 4,
      LON_DEG        = 2,
      LAT_DEG        = 2,
      ALT_DEG        = 2
   };

   rspfAlphaSensorSupportData();

   virtual bool readSupportFiles(const rspfFilename& hdrFile);
   virtual bool readSupportFiles(const rspfEnviHeader& hdr);
   virtual bool readHdrFile(const rspfFilename& hdrFile);
   virtual bool readHdrFile(const rspfEnviHeader& hdr);
   virtual bool readInsGpsFile(const rspfFilename& txtFile);

   inline bool isHSI()const {return m_isHSI;}

   const rspfDpt&      getImageSize()const;
   const rspf_float64& getRollBias()const;
   const rspf_float64& getPitchBias()const;
   const rspf_float64& getHeadingBias()const;
   const rspf_float64& getFov()const;
   const rspf_float64& getSlitRot()const;

   const std::vector< rspf_float64 >& getRollPoly()const;
   const std::vector< rspf_float64 >& getPitchPoly()const;
   const std::vector< rspf_float64 >& getHeadingPoly()const;
   const std::vector< rspf_float64 >& getLonPoly()const;
   const std::vector< rspf_float64 >& getLatPoly()const;
   const std::vector< rspf_float64 >& getAltPoly()const;
   const std::vector< rspf_float64 >& getScanPoly()const;

protected:
   void reset();
   rspfAlphaSensorSupportData(const rspfAlphaSensorSupportData& src);
   rspfAlphaSensorSupportData& operator =(const rspfAlphaSensorSupportData& src);

   bool m_isHSI;

   // hdr data
   rspfString   m_imageID;
   rspfString   m_sensorType;
   rspfDpt      m_imageSize;
   rspf_float64 m_rollBias;
   rspf_float64 m_pitchBias;
   rspf_float64 m_headingBias;
   rspf_float64 m_fov;
   rspf_float64 m_slitRot;

   // insgps data
   std::vector< rspfPolynom<rspf_float64,1>::VAR_TUPLE > m_imgLine;
   std::vector< rspf_float64 > m_roll;
   std::vector< rspf_float64 > m_pitch;
   std::vector< rspf_float64 > m_heading;
   std::vector< rspf_float64 > m_lon;
   std::vector< rspf_float64 > m_lat;
   std::vector< rspf_float64 > m_alt;
   std::vector< rspf_float64 > m_scanAng;

   // insgps polynomials
   std::vector< rspf_float64 > m_rollPolyCoef;
   std::vector< rspf_float64 > m_pitchPolyCoef;
   std::vector< rspf_float64 > m_headingPolyCoef;
   std::vector< rspf_float64 > m_lonPolyCoef;
   std::vector< rspf_float64 > m_latPolyCoef;
   std::vector< rspf_float64 > m_altPolyCoef;
   std::vector< rspf_float64 > m_scanAngPolyCoef;
};

#endif
