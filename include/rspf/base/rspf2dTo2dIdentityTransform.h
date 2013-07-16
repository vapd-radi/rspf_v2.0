//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description:  Contains class definition for rspf2dTo2dIdentityTransform.
// 
//*******************************************************************
//  $Id$
#ifndef rspf2dTo2dIdentityTransform_HEADER
#define rspf2dTo2dIdentityTransform_HEADER
#include "rspf2dTo2dTransform.h"
/**
 * This is the identity transform and just passes the input to the output.
 */
class RSPF_DLL rspf2dTo2dIdentityTransform : public rspf2dTo2dTransform
{
public:
   /**
    * forward transform just passes the point to the output.
    */
   virtual void forward(const rspfDpt& input,
                        rspfDpt& output) const
   {
      output = input;
   }
   
   /**
    * forward transform nothing is modified on the input point.
    */
   virtual void forward(rspfDpt&  /* modify_this */) const
   {
      // do nothing this is identity
   }
   
   /**
    * inverse transform just passes the point to the output.
    */
   virtual void inverse(const rspfDpt& input,
                        rspfDpt&       output) const
   {
      output = input;
   }
   
   /**
    * inverse transform nothing is modified on the input point.
    */
   virtual void inverse(rspfDpt&  /* modify_this */) const
   {
      // do nothing this is identity
   }
   
   /**
    * Pass equality to the parent
    */
   virtual const rspf2dTo2dIdentityTransform& operator=(
                                                 const rspf2dTo2dIdentityTransform& rhs)
   {
      rspf2dTo2dTransform::operator =(rhs);
      
      return *this;
   }
   
protected:
   TYPE_DATA
};
#endif
