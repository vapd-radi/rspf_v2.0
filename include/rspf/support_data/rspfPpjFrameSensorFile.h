#ifndef rspfPpjFrameSensorFile_HEADER
#define rspfPpjFrameSensorFile_HEADER
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfXmlDocument.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/matrix/newmat.h>
#include <rspf/base/rspfEcefPoint.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfDpt3d.h>
#include <iostream>

class RSPF_DLL rspfPpjFrameSensorFile : public rspfObject
{
public:
   enum PointType
   {
      UNKNOWN_POINT_TYPE = 0,
      BASIC_POINT_TYPE   = 1
   };
   class RSPF_DLL PointMap
   {
   public:
      PointType      m_type;
      rspfDpt3d     m_point;
      rspfDpt       m_pixelPoint;
   };
   typedef std::vector<PointMap> PointMapList;
   typedef std::vector<rspf_float64> DoubleArrayType;
   rspfPpjFrameSensorFile();

   virtual bool readFile(const rspfFilename& file);
   virtual bool readStream(std::istream& is); 

   const rspfString&     getBaseName()const;
   rspf_int64            getImageNumber()const;
   const rspfDpt&        getPrincipalPoint()const;
   const rspfGpt&        getPlatformPosition()const;
   const NEWMAT::Matrix&  getExtrinsic()const;
   const NEWMAT::Matrix&  getIntrinsic()const;
   const rspfDpt&        getImageSize()const;
   const DoubleArrayType& getRadialDistortion()const;
   const DoubleArrayType& getTangentialDistortion()const;

   double getAverageProjectedHeight()const;
protected:
   void reset();
   rspfPpjFrameSensorFile(const rspfPpjFrameSensorFile& src);
   rspfPpjFrameSensorFile& operator =(const rspfPpjFrameSensorFile& src);

   rspfGpt                      m_platformPosition;
   rspfFilename                 m_filename;
   rspfString                   m_fileBaseName;
   rspf_int64                   m_imageNumber;
   rspfRefPtr<rspfXmlDocument> m_ppjXml;
   rspfDpt                      m_imageSize;
   rspfDpt                      m_principalPoint;
   NEWMAT::Matrix                m_extrinsicMatrix;
   NEWMAT::Matrix                m_intrinsicMatrix;
   DoubleArrayType               m_radialDistortion;
   DoubleArrayType               m_tangentialDistortion;

   PointMapList                  m_pointMapList;
   double                        m_averageProjectedHeight;
};

#endif
