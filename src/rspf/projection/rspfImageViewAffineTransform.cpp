#include <rspf/projection/rspfImageViewAffineTransform.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfMatrix3x3.h>
#include <rspf/base/rspfString.h>
RTTI_DEF1(rspfImageViewAffineTransform, "rspfImageViewAffineTransform", rspfImageViewTransform)
rspfImageViewAffineTransform::rspfImageViewAffineTransform(double rotateDegrees,
                                                             double scaleXValue,
                                                             double scaleYValue,
                                                             double translateXValue,
                                                             double translateYValue,
                                                             double pivotXValue,
                                                             double pivotYValue)
   :m_transform(3,3),
    m_inverseTransform(3,3),
    m_rotation(rotateDegrees),
    m_scale(scaleXValue, scaleYValue),
    m_translate(translateXValue, translateYValue),
    m_pivot(pivotXValue, pivotYValue)
{
   m_transform << 1 << 0 << 0
                << 0 << 1 << 0
                << 0 << 0 << 1;
   
   m_inverseTransform << 1 << 0 << 0
                       << 0 << 1 << 0
                       << 0 << 0 << 1;
   
   rotate(rotateDegrees);
   scale(scaleXValue, scaleYValue);
   translate(translateXValue, translateYValue);
   pivot(m_pivot.x, m_pivot.y);
}
rspfImageViewAffineTransform::~rspfImageViewAffineTransform()
{
}
void rspfImageViewAffineTransform::imageToView(const rspfDpt& imagePoint,
                                                rspfDpt&       viewPoint)const
{
   
    viewPoint.x = m_transform[0][0]*imagePoint.x + m_transform[0][1]*imagePoint.y + m_transform[0][2];
    viewPoint.y = m_transform[1][0]*imagePoint.x + m_transform[1][1]*imagePoint.y + m_transform[1][2];
}
void rspfImageViewAffineTransform::viewToImage(const rspfDpt& viewPoint,
                                                rspfDpt&       imagePoint)const
{
   imagePoint.x = m_inverseTransform[0][0]*viewPoint.x + m_inverseTransform[0][1]*viewPoint.y + m_inverseTransform[0][2];
   imagePoint.y = m_inverseTransform[1][0]*viewPoint.x + m_inverseTransform[1][1]*viewPoint.y + m_inverseTransform[1][2];
}
void rspfImageViewAffineTransform::setMatrix(NEWMAT::Matrix& matrix)
{
   m_transform        = matrix;
   m_inverseTransform = m_transform.i();
}
const NEWMAT::Matrix& rspfImageViewAffineTransform::getMatrix()const
{
   return m_transform;
}
void rspfImageViewAffineTransform::scale(double x, double y)
{
   
   m_scale = rspfDpt(x,y);
   buildCompositeTransform();
}
void rspfImageViewAffineTransform::translate(double deltaX,
                                              double deltaY)
{
   m_translate = rspfDpt(deltaX, deltaY);
   buildCompositeTransform();
   
   
}
void rspfImageViewAffineTransform::translateX(double deltaX)
{
   translate(deltaX, m_translate.y);
}
void rspfImageViewAffineTransform::translateY(double deltaY)
{
   translate(m_translate.x, deltaY);
}
void rspfImageViewAffineTransform::scaleX(double x)
{
   scale(x, m_scale.y);
}
void rspfImageViewAffineTransform::scaleY(double y)
{
   scale(m_scale.x, y);
}
void rspfImageViewAffineTransform::pivot(double originX, double originY)
{
   m_pivot.x = originX;
   m_pivot.y = originY;
   buildCompositeTransform();
}
void rspfImageViewAffineTransform::pivotX(double originX)
{
   pivot(originX,m_pivot.y);
}
void rspfImageViewAffineTransform::pivotY(double originY)
{
   pivot(m_pivot.x, originY);
}
void rspfImageViewAffineTransform::rotate(double degrees)
{
   m_rotation = degrees;
   buildCompositeTransform();
}
void rspfImageViewAffineTransform::buildCompositeTransform()
{
   NEWMAT::Matrix scaleM(3, 3);
   NEWMAT::Matrix rotzM = rspfMatrix3x3::createRotationZMatrix(m_rotation);
   NEWMAT::Matrix transM(3,3);
   NEWMAT::Matrix transOriginM(3,3);
   NEWMAT::Matrix transOriginNegatedM(3,3);
   
   transM << 1 << 0 << m_translate.x
          << 0 << 1 << m_translate.y
          << 0 << 0 << 1;
   
   transOriginM << 1 << 0 << m_pivot.x
                << 0 << 1 << m_pivot.y
                << 0 << 0 << 1;
   
   transOriginNegatedM << 1 << 0 << -m_pivot.x
                       << 0 << 1 << -m_pivot.y
                       << 0 << 0 << 1;
   
   scaleM << m_scale.x << 0 << 0
          << 0 << m_scale.y << 0
          << 0 << 0 << 1;
   m_transform        = transM*scaleM*transOriginM*rotzM*transOriginNegatedM;
   m_inverseTransform = m_transform.i();
   
}
bool rspfImageViewAffineTransform::loadState(const rspfKeywordlist& kwl,
                                              const char* prefix)
{
   rspfString scaleString(kwl.find(prefix,"scale"));
   rspfString pivotString(kwl.find(prefix,"pivot"));
   rspfString translateString(kwl.find(prefix,"translate"));
   rspfString rotateString(kwl.find(prefix,"rotate"));
   
   if(!scaleString.empty())
   {
      m_scale.toPoint(scaleString);
   }
   if(!pivotString.empty())
   {
      m_pivot.toPoint(pivotString);
   }
   if(!translateString.empty())
   {
      m_translate.toPoint(translateString);
   }
   if(!rotateString.empty())
   {
      m_rotation = rotateString.toDouble();
   }
   buildCompositeTransform();
   rspfImageViewTransform::loadState(kwl, prefix);
   return true;
}
bool rspfImageViewAffineTransform::saveState(rspfKeywordlist& kwl,
                                              const char* prefix)const
{
   kwl.add(prefix,
           "rotation",
           m_rotation,
           true);
   kwl.add(prefix,
           "pivot",
           m_pivot.toString(),
           true);
   kwl.add(prefix,
           "scale",
           m_scale.toString(),
           true);
   kwl.add(prefix,
           "translate",
           m_translate.toString(),
           true);
           
   return rspfImageViewTransform::saveState(kwl,prefix);
}
bool rspfImageViewAffineTransform::isValid()const
{
   return true;
}
bool rspfImageViewAffineTransform::setView(rspfObject* obj)
{
   rspfImageViewAffineTransform* view = dynamic_cast<rspfImageViewAffineTransform*> (obj);
   if(view)
   {  
      m_transform = view->m_transform;
      m_inverseTransform = view->m_inverseTransform;
      m_rotation = view->m_rotation;
      m_scale = view->m_scale;
      m_translate = view->m_translate;
      m_pivot = view->m_pivot;
   }
   return (view!=0);
}
rspfObject* rspfImageViewAffineTransform::getView()
{
   return this;
}
const rspfObject* rspfImageViewAffineTransform::getView()const
{
   return this;
}
rspfDpt rspfImageViewAffineTransform::getInputMetersPerPixel()const
{
   return rspfDpt(1,1);
}
rspfDpt rspfImageViewAffineTransform::getOutputMetersPerPixel()const
{
   rspfDpt result;
   
   result.makeNan();
   
   return result;
}
bool rspfImageViewAffineTransform::isEqualTo(const rspfObject& obj, rspfCompareType compareType)const
{
   bool result = rspfImageViewTransform::isEqualTo(obj,compareType);
   
   if(result)
   {
      result = false;
      const rspfImageViewAffineTransform* rhs = dynamic_cast<const rspfImageViewAffineTransform*> (&obj);
      if(rhs)
      {
         result = (rspf::almostEqual(m_rotation, rhs->m_rotation)&&
                   (m_scale.isEqualTo(rhs->m_scale))&&
                   (m_translate.isEqualTo(rhs->m_translate))&&
                   (m_pivot.isEqualTo(rhs->m_pivot))
                   );  
      }
   }
   
   return result;
}
