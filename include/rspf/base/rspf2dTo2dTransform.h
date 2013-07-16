//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspf2dTo2dTransform.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspf2dTo2dTransform_HEADER
#define rspf2dTo2dTransform_HEADER
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfDpt.h>

class RSPFDLLEXPORT rspf2dTo2dTransform : public rspfObject
{
public:
   virtual void forward(const rspfDpt& input,
                        rspfDpt& output) const=0;
   
   virtual void forward(rspfDpt&  modify_this) const;
      
   virtual void inverse(const rspfDpt& input,
                        rspfDpt&       output) const;

   virtual void inverse(rspfDpt&  modify_this) const;
      
   virtual const rspf2dTo2dTransform& operator=(
      const rspf2dTo2dTransform& rhs);
   
   /*!
    * Used by the inverse iterator.  This will give it a good initial value
    * that is within the input transforms
    */
   virtual rspfDpt getOrigin() const;

   virtual std::ostream& print(std::ostream& out) const;
   
   virtual bool saveState(rspfKeywordlist& kwl,
                   const char* prefix = 0)const;
   
   virtual bool loadState(const rspfKeywordlist& kwl,
                  const char* prefix = 0);
   
   void setConvergenceThreshold(const double& new_threshold);
   
   void setMaxIterations(int new_max_iters);

   void setDxDy(const rspfDpt& dxdy);

protected:
   rspf2dTo2dTransform();
   virtual ~rspf2dTo2dTransform(){}
   double theConvergenceThreshold;
   int    theMaxIterations;

   rspfDpt theDxDy;
   
TYPE_DATA      
};

#endif
