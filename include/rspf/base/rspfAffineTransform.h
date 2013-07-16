//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfAffineTransform.h 15766 2009-10-20 12:37:09Z gpotts $

#ifndef rspfAffineTransform_HEADER
#define rspfAffineTransform_HEADER

#include <rspf/base/rspf2dTo2dTransform.h>
#include <rspf/matrix/newmat.h>

class RSPFDLLEXPORT rspfAffineTransform : public rspf2dTo2dTransform
{
public:
   rspfAffineTransform();
   rspfAffineTransform(const rspfAffineTransform& src);

   /**
    * @brief assignment operator=
    * @param rhs The data to assign from.
    * @param A reference to this object.
    */
   virtual const rspfAffineTransform& operator=(
      const rspfAffineTransform& rhs);
   
   virtual rspfObject* dup()const
   {
      return new rspfAffineTransform(*this);
   }
   virtual void forward(const rspfDpt& input,
                        rspfDpt& output)const;
   virtual void inverse(const rspfDpt& input,
                        rspfDpt&       output)const;

   /*!
    * Used by the inverse iterator.  This will give it a good initial value
    * that is within the input transforms
    */
   virtual rspfDpt getOrigin()const
      {
         return rspfDpt(0,0);
      }
   
   bool saveState(rspfKeywordlist& kwl,
                   const char* prefix = 0)const;
   
   bool loadState(const rspfKeywordlist& kwl,
                  const char* prefix = 0);
   
   void setMatrix(double rotation,
                  const rspfDpt& scale,
                  const rspfDpt& translation);
   
   void setScale(const rspfDpt& scale)
      {
         theScale       = scale;
         computeMatrix();
      }
   
   void setRotation(double rotation)
      {
         theRotation = rotation;
         computeMatrix();
      }

   void setTranslation(const rspfDpt& translation)
      {
         theTranslation = translation;
         computeMatrix();
      }
   rspfDpt getScale()const
      {
         return theScale;
      }
   rspfDpt getTranslation()const
      {
         return theTranslation;
      }
   double getRotation()const
      {
         return theRotation;
      }
   void setIdentity();

   void setAffineEnabledFlag(bool flag)
      {
         theAffineEnabledFlag = flag;
      }
   virtual std::ostream& print(std::ostream& out) const;
      
protected:
   void computeMatrix();

   rspfDpt       theScale;
   double         theRotation;
   rspfDpt       theTranslation;
   bool           theAffineEnabledFlag;

   NEWMAT::Matrix theForwardXform;
   NEWMAT::Matrix theInverseXform;
TYPE_DATA      
};

#endif
