#include <rspf/support_data/rspfPpjFrameSensorFile.h>
#include <rspf/base/rspfXmlNode.h>
#include <fstream>
#include <rspf/matrix/newmatio.h>
#include <rspf/base/rspfEcefPoint.h>

rspfPpjFrameSensorFile::rspfPpjFrameSensorFile()
:m_radialDistortion(2),
m_tangentialDistortion(2),
m_averageProjectedHeight(0.0)
{
   reset();
}

const rspfString& rspfPpjFrameSensorFile::getBaseName()const
{
   return m_fileBaseName;
}

rspf_int64 rspfPpjFrameSensorFile::getImageNumber()const
{
   return m_imageNumber;
}

const rspfDpt& rspfPpjFrameSensorFile::getPrincipalPoint()const
{
   return m_principalPoint;
}
const rspfGpt& rspfPpjFrameSensorFile::getPlatformPosition()const
{
   return m_platformPosition;
}

const NEWMAT::Matrix& rspfPpjFrameSensorFile::getExtrinsic()const
{
   return m_extrinsicMatrix;
}

const NEWMAT::Matrix& rspfPpjFrameSensorFile::getIntrinsic()const
{
   return m_intrinsicMatrix;
}

const rspfDpt& rspfPpjFrameSensorFile::getImageSize()const
{
   return m_imageSize;
}

const rspfPpjFrameSensorFile::DoubleArrayType& rspfPpjFrameSensorFile::getRadialDistortion()const
{
   return m_radialDistortion;
}

const rspfPpjFrameSensorFile::DoubleArrayType& rspfPpjFrameSensorFile::getTangentialDistortion()const
{
   return m_tangentialDistortion;
}

double rspfPpjFrameSensorFile::getAverageProjectedHeight()const
{
   return m_averageProjectedHeight;
}

void rspfPpjFrameSensorFile::reset()
{
   m_ppjXml = 0;
   m_principalPoint = rspfDpt(0.0,0.0);
   m_platformPosition = rspfGpt(0.0,0.0);
   m_fileBaseName = "";
   m_imageNumber = -1;
   m_radialDistortion[0] = 0.0;
   m_radialDistortion[1] = 0.0;
   m_tangentialDistortion[0] = 0.0;
   m_tangentialDistortion[1] = 0.0;
   m_intrinsicMatrix = NEWMAT::Matrix(3,3);
   m_extrinsicMatrix = NEWMAT::Matrix(4,4);
   std::fill(m_intrinsicMatrix.Store(), m_intrinsicMatrix.Store() + 9, 0.0);
   std::fill(m_extrinsicMatrix.Store(), m_extrinsicMatrix.Store() + 16, 0.0);
   m_extrinsicMatrix[0][0] = 1.0;
   m_extrinsicMatrix[1][1] = 1.0;
   m_extrinsicMatrix[2][2] = 1.0;
   m_extrinsicMatrix[3][3] = 1.0;
   m_imageSize.makeNan();
   m_averageProjectedHeight = 0.0;
   m_pointMapList.clear();
}

bool rspfPpjFrameSensorFile::readFile(const rspfFilename& file)
{
   bool result = false;
   std::ifstream in(file.c_str(), std::ios::in|std::ios::binary);
   if(in.good()&&readStream(in))
   {

      result = true;
   }

   return result;
}

bool  rspfPpjFrameSensorFile::readStream(std::istream& is)
{
   reset();
   m_ppjXml = new rspfXmlDocument;
   if(m_ppjXml->read(is))
   {
      rspfRefPtr<rspfXmlNode> root = m_ppjXml->getRoot();
      if(root.valid()&&(root->getTag() == "pearlProjectionFile"))
      {
         rspfRefPtr<rspfXmlNode> image = root->findFirstNode("image");
         if(image.valid())
         {
            rspfRefPtr<rspfXmlNode> sensorData     = image->findFirstNode("sensorData");
            rspfRefPtr<rspfXmlNode> namingData     = image->findFirstNode("namingData");
            rspfRefPtr<rspfXmlNode> projectionData = image->findFirstNode("projectionData");
            if(namingData.valid())
            {
               rspfRefPtr<rspfXmlNode> fileBaseName = namingData->findFirstNode("fileBaseName");
               rspfRefPtr<rspfXmlNode> imageNumber  = namingData->findFirstNode("imageNumber");
               if(fileBaseName.valid())
               {
                  m_fileBaseName = fileBaseName->getText();
               }
               if(imageNumber.valid())
               {
                  m_imageNumber = imageNumber->getText().toInt64();
               }
            }
            if(projectionData.valid())
            {
               const rspfXmlNode::ChildListType& childNodes = projectionData->getChildNodes();
               rspf_uint32 nNodes = childNodes.size();
               rspf_uint32 idx = 0;
               rspf_uint32 averageHeightDivisor = 0;
               for(idx = 0; idx < nNodes; ++idx)
               {
                  if(childNodes[idx]->getAttributeValue("type").downcase() == "basic")
                  {
                     rspfRefPtr<rspfXmlNode> pixelPointX = childNodes[idx]->findFirstNode("pixelPointX");
                     rspfRefPtr<rspfXmlNode> pixelPointY = childNodes[idx]->findFirstNode("pixelPointY");
                     rspfRefPtr<rspfXmlNode> ecefX = childNodes[idx]->findFirstNode("ecefX");
                     rspfRefPtr<rspfXmlNode> ecefY = childNodes[idx]->findFirstNode("ecefY");
                     rspfRefPtr<rspfXmlNode> ecefZ = childNodes[idx]->findFirstNode("ecefZ");

                     if(pixelPointX.valid()&&pixelPointY.valid()&ecefX.valid()&&ecefY.valid()&&ecefZ.valid())
                     {
                        PointMap pointMap;
                        pointMap.m_type       = BASIC_POINT_TYPE;
                        pointMap.m_point      = rspfDpt3d(ecefX->getText().toDouble(), ecefY->getText().toDouble(), ecefZ->getText().toDouble());
                        pointMap.m_pixelPoint = rspfDpt(pixelPointX->getText().toDouble(),pixelPointY->getText().toDouble());

                        rspfGpt gpt(rspfEcefPoint(pointMap.m_point.x, pointMap.m_point.y, pointMap.m_point.z));
                        m_averageProjectedHeight += gpt.height();
                        ++averageHeightDivisor;
                        m_pointMapList.push_back(pointMap);
                     }
                  }
               }
               if(averageHeightDivisor > 0) m_averageProjectedHeight /= static_cast<double>(averageHeightDivisor); 
            }
            if(sensorData.valid())
            {
               rspfRefPtr<rspfXmlNode> sensorSize      = sensorData->findFirstNode("sensorSize");
               rspfRefPtr<rspfXmlNode> cameraIntrinsic = sensorData->findFirstNode("cameraIntrinsic");
               rspfRefPtr<rspfXmlNode> cameraExtrinsic = sensorData->findFirstNode("cameraExtrinsic");
               if(sensorSize.valid())
               {
                  rspfRefPtr<rspfXmlNode> width  = sensorSize->findFirstNode("SensorWidth");
                  rspfRefPtr<rspfXmlNode> height = sensorSize->findFirstNode("SensorHeight");
                  if(width.valid()&&height.valid())
                  {
                     m_imageSize.x = width->getText().toDouble();
                     m_imageSize.y = height->getText().toDouble();
                  }
                  else
                  {
                     m_ppjXml = 0;
                  }
               }
               else
               {
                  m_ppjXml = 0;
               }
               if(m_ppjXml.valid()&&cameraExtrinsic.valid())
               {
                  if(cameraExtrinsic->getChildNodes().size() == 4)
                  {
                     m_extrinsicMatrix = NEWMAT::Matrix(4,4);
                     std::vector<rspfString> inRow1 = cameraExtrinsic->getChildNodes()[0]->getText().split(" ");
                     std::vector<rspfString> inRow2 = cameraExtrinsic->getChildNodes()[1]->getText().split(" ");
                     std::vector<rspfString> inRow3 = cameraExtrinsic->getChildNodes()[2]->getText().split(" ");
                     std::vector<rspfString> inRow4 = cameraExtrinsic->getChildNodes()[3]->getText().split(" ");

                     if((inRow1.size() ==4)&&
                        (inRow2.size() ==4)&&
                        (inRow3.size() ==4)&&
                        (inRow4.size() ==4))
                     {
                        m_extrinsicMatrix << inRow1[0].toDouble() << inRow1[1].toDouble() << inRow1[2].toDouble() 
                                          << inRow1[3].toDouble() 
                                          << inRow2[0].toDouble() << inRow2[1].toDouble() << inRow2[2].toDouble() 
                                          << inRow2[3].toDouble()
                                          << inRow3[0].toDouble() << inRow3[1].toDouble() << inRow3[2].toDouble() 
                                          << inRow3[3].toDouble()
                                          << inRow4[0].toDouble() << inRow4[1].toDouble() << inRow4[2].toDouble() 
                                          << inRow4[3].toDouble();
                        NEWMAT::ColumnVector v(4);
                        v[0] = v[1] = v[2] = 0.0;
                        v[3] = 1.0;
                        NEWMAT::ColumnVector result = m_extrinsicMatrix*v;
                        m_platformPosition = rspfEcefPoint(result[0], result[1], result[2]);
                        //m_platformPosition;
                     }
                     else
                     {
                        m_ppjXml = 0;
                     }

                  } // end  if(cameraExtrinsic->getChildNodes().size() == 4)
                  else
                  {
                     m_ppjXml = 0;
                  }

                  if(m_ppjXml.valid())
                  {
                     if(cameraIntrinsic.valid())
                     {
                        m_intrinsicMatrix = NEWMAT::Matrix(3,3);
                        std::vector<rspfString> inRow1 = cameraIntrinsic->getChildNodes()[0]->getText().split(" ");
                        std::vector<rspfString> inRow2 = cameraIntrinsic->getChildNodes()[1]->getText().split(" ");
                        std::vector<rspfString> inRow3 = cameraIntrinsic->getChildNodes()[2]->getText().split(" ");
                        if((inRow1.size() ==3)&&
                           (inRow2.size() ==3)&&
                           (inRow3.size() ==3))
                        {
                           m_principalPoint.x = inRow1[2].toDouble();
                           m_principalPoint.y = inRow2[2].toDouble();
                           m_intrinsicMatrix << inRow1[0].toDouble() << inRow1[1].toDouble() << m_principalPoint.x
                                             << inRow2[0].toDouble() << inRow2[1].toDouble() << m_principalPoint.y
                                             << inRow3[0].toDouble() << inRow3[1].toDouble() << inRow3[2].toDouble(); 
                        }
                        else
                        {
                           m_ppjXml = 0;
                        }
                     } // end if(cameraIntrinsic .....)
                     else
                     {
                        m_ppjXml = 0;
                     }
                  } // end if m_ppjXml ...
               } // end if(m_ppjXml.valid()&&cameraExtrinsic.valid())
               else
               {
                  m_ppjXml = 0;
               }
            }
            else
            {
               m_ppjXml = 0;
            }
         }
         else
         {
            m_ppjXml = 0;
         }
      }
      else
      {
         m_ppjXml = 0;
      }
   }
   else
   {
      m_ppjXml = 0;
   }


   return m_ppjXml.valid();
}

// Hidden from use...
rspfPpjFrameSensorFile::rspfPpjFrameSensorFile(const rspfPpjFrameSensorFile& /* src */)
{
}

rspfPpjFrameSensorFile& rspfPpjFrameSensorFile::operator=(
   const rspfPpjFrameSensorFile& /* src */)
{
   return *this;
}

