#ifndef rspfImageViewAffineTransform_HEADER
#define rspfImageViewAffineTransform_HEADER
#include <rspf/projection/rspfImageViewTransform.h>
#include <rspf/matrix/newmat.h>
class RSPFDLLEXPORT rspfImageViewAffineTransform: public rspfImageViewTransform
{
public:
   rspfImageViewAffineTransform(double rotateDegrees = 0,
                                 double scaleXValue = 1,
                                 double scaleYValue = 1,
                                 double translateXValue = 0,
                                 double translateYValue = 0,
                                 double pivotXValue = 0,
                                 double pivotYValue = 0);
   virtual ~rspfImageViewAffineTransform();
   
   rspfImageViewAffineTransform(const rspfImageViewAffineTransform& src)
   :rspfImageViewTransform(src),
   m_transform(src.m_transform),
   m_inverseTransform(src.m_inverseTransform),
   m_rotation(src.m_rotation),
   m_scale(src.m_scale),
   m_translate(src.m_translate),
   m_pivot(src.m_pivot)
   {
   }
   virtual rspfObject* dup()const
   {
      return new rspfImageViewAffineTransform(*this);
   }
   
   virtual void imageToView(const rspfDpt& imagePoint,
                            rspfDpt&       viewPoint)const;
   virtual void viewToImage(const rspfDpt& viewPoint,
                            rspfDpt&       imagePoint)const;
   void setMatrix(NEWMAT::Matrix& matrix);
   const NEWMAT::Matrix& getMatrix()const;
   
   virtual bool isIdentity()const
   {
      return ((m_transform[0][0] == 1.0)&&
              (m_transform[0][1] == 0.0)&&
              (m_transform[0][2] == 0.0)&&
              (m_transform[1][0] == 0.0)&&
              (m_transform[1][1] == 1.0)&&
              (m_transform[1][2] == 0.0)&&
              (m_transform[2][0] == 0.0)&&
              (m_transform[2][1] == 0.0)&&
              (m_transform[2][2] == 1.0));
   }
   virtual bool isValid()const;
   virtual bool setView(rspfObject* obj);
   virtual rspfObject* getView();
   virtual const rspfObject* getView()const;
   
   /** @return (1, 1) ???????(drb) */
   virtual rspfDpt getInputMetersPerPixel()const;
   /** @return (nan, nan) ????????? (drb) */
   virtual rspfDpt getOutputMetersPerPixel()const;
   
   /*!
    * Translate in the x and y direction.
    */ 
   virtual void translate(double deltaX, double deltaY);
   
   /*!
    * Translate in the x direction.
    */
   virtual void translateX(double deltaX);
   
   /*!
    * Translate in the Y direction.
    */
   virtual void translateY(double deltaY);
   
   /*!
    * Translate the origin for rotation in the x and y direction.
    */
   virtual void pivot(double originX, double originY);
   
   /*!
    * Translate the origin for rotation in the x direction.
    */
   virtual void pivotX(double originX);
   
   /*!
    * Translate the origin for rotation in the y direction.
    */
   virtual void pivotY(double originY);
   
   /*!
    * will allow you to specify a scale
    * for both the x and y direction.
    */
   virtual void scale(double x, double y);
   
   /*!
    * will alow you to specify a scale
    * along the X direction.
    */
   virtual void scaleX(double x);
   
   /*!
    * Will allow you to scale along the Y
    * direction.
    */
   virtual void scaleY(double y);
   
   /*!
    * Will apply a rotation
    */
   virtual void rotate(double degrees);
   
   rspf_float64 getRotation()const{return m_rotation;}
   const rspfDpt& getScale()const{return m_scale;}
   const rspfDpt& getTranslate()const{return m_translate;}
   const rspfDpt& getPivot()const{return m_pivot;}
   
   
   virtual bool isEqualTo(const rspfObject& obj, rspfCompareType compareType = RSPF_COMPARE_FULL)const;
   
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix =0);
   
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix =0)const;
   
   
protected:
   void buildCompositeTransform();
   
   /*!
    * This is the transformation from image to
    * viewing coordinates.  If this matrix is
    * changed it will perform an inverse to solve
    * the inverse transform.
    */
   NEWMAT::Matrix m_transform;
   NEWMAT::Matrix m_inverseTransform;
   rspf_float64 m_rotation;
   rspfDpt      m_scale;
   rspfDpt      m_translate;
   rspfDpt      m_pivot;
TYPE_DATA
};
#endif
