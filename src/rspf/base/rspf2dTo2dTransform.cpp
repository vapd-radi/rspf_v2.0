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
// $Id: rspf2dTo2dTransform.cpp 15766 2009-10-20 12:37:09Z gpotts $

#include <cstdlib>
#include <sstream>
#include <rspf/base/rspf2dTo2dTransform.h>


RTTI_DEF1(rspf2dTo2dTransform, "rspf2dTo2dTransform", rspfObject);

#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfNotifyContext.h>

// ### CREATE_COPY ###
// Implementation of static factory method createCopy() requires includes here of ALL 2D-to-2D
// transform types that need a deep copy capability:
#include <rspf/base/rspfAffineTransform.h>
#include <rspf/projection/rspfImageViewAffineTransform.h>
#include <rspf/projection/rspfImageViewProjectionTransform.h>
#include <rspf/projection/rspfMeanRadialLensDistortion.h>
#include <rspf/base/rspfQuadTreeWarp.h>
#include <rspf/projection/rspfRadialDecentLensDistortion.h>

//***
// Define Trace flags for use within this file:
//***
#include <rspf/base/rspfTrace.h>
static rspfTrace traceExec  ("rspf2dTo2dTransform:exec");
static rspfTrace traceDebug ("rspf2dTo2dTransform:debug");

static const double DEFAULT_THRESHOLD      = 1000.0*DBL_EPSILON;
static const int    DEFAULT_MAX_ITERATIONS = 10;

//*****************************************************************************
//  CONSTRUCTOR: 
//*****************************************************************************
rspf2dTo2dTransform::rspf2dTo2dTransform()
   :
      theConvergenceThreshold (DEFAULT_THRESHOLD),
      theMaxIterations        (DEFAULT_MAX_ITERATIONS),
      theDxDy(1.0, 1.0)
{
}
 
//*****************************************************************************
//  METHOD: 
//*****************************************************************************
void rspf2dTo2dTransform::inverse(const rspfDpt& input,
                                   rspfDpt&       output) const
{
   static const char MODULE[] = "rspf2dTo2dTransform::inverse";
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG: " << MODULE << ", entering...\n";
   
   //***
   // Begin with guess. Forward transform is defined as trasforming left to
   // right. We are therefore solving for left:
   //***
   rspfDpt left = getOrigin();
   rspfDpt left_dx;
   rspfDpt left_dy;
   rspfDpt right;
   rspfDpt right_dx;
   rspfDpt right_dy;
   rspfDpt dr_dx;
   rspfDpt dr_dy;
   rspfDpt r_diff;
   rspfDpt l_diff;
   double inverse_norm;
   int iters=0;
   //***
   // Begin iterations:
   //***
   do
   {
      //***
      // establish perturbed image points about the guessed point:
      //***
      left_dx.x = left.x + theDxDy.x;
      left_dx.y = left.y;
      left_dy.x = left.x;
      left_dy.y = left.y + theDxDy.y;
      
      //***
      // Compute numerical partials at current guessed point:
      //***
      forward(left,    right);
      forward(left_dx, right_dx);
      forward(left_dy, right_dy);

      dr_dx.x = (right_dx.x - right.x)/theDxDy.x; //e
      dr_dx.y = (right_dx.y - right.y)/theDxDy.y; //g
      dr_dy.x = (right_dy.x - right.x)/theDxDy.x; //f
      dr_dy.y = (right_dy.y - right.y)/theDxDy.y; //h

      //***
      // Test for convergence:
      //***
      r_diff = input - right;
      
      //***
      // Compute linearized estimate of image point given gp delta:
      //***
      inverse_norm = dr_dy.x*dr_dx.y - dr_dx.x*dr_dy.y; // fg-eh

      if (inverse_norm != 0)
      {
         l_diff.x = (-dr_dy.y*r_diff.x + dr_dy.x*r_diff.y)/inverse_norm;
         l_diff.y = ( dr_dx.y*r_diff.x - dr_dx.x*r_diff.y)/inverse_norm;

         left += l_diff;
      }
      else
      {
         l_diff.x = 0;
         l_diff.y = 0;
      }

      iters++;
      
   } while (((fabs(l_diff.x) > theConvergenceThreshold) ||
             (fabs(l_diff.y) > theConvergenceThreshold)) &&
            (iters < theMaxIterations));

   //***
   // Note that this error mesage appears only if max count was reached while
   // iterating. A linear (no iteration) solution would finish with iters =
   // MAX_NUM_ITERATIONS + 1:
   //***
    if (iters == theMaxIterations)
    {
       rspfNotify(rspfNotifyLevel_WARN) << "WARNING: " << MODULE << ", exceeded max number of iterations computing inverse "
                                          << "transform for point: " << input << "\n";
    }

   output = left;
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG: " << MODULE << ", returning...\n";
   return;
   
}

//*****************************************************************************
//  METHOD: 
//*****************************************************************************
bool rspf2dTo2dTransform::saveState(rspfKeywordlist& kwl,
                                     const char* prefix) const
{
   kwl.add(prefix,
           rspfKeywordNames::CONVERGENCE_THRESHOLD_KW,
           theConvergenceThreshold,
           true);
   kwl.add(prefix,
           rspfKeywordNames::MAX_ITERATIONS_KW,
           theMaxIterations,
           true);
   kwl.add(prefix,
           "dxdy",
           rspfString::toString(theDxDy.x) + " " +
           rspfString::toString(theDxDy.y),
           true);
           
   return rspfObject::saveState(kwl, prefix);
}

//*****************************************************************************
//  METHOD: 
//*****************************************************************************
bool rspf2dTo2dTransform::loadState(const rspfKeywordlist& kwl,
               const char* prefix)
{
  bool result = true;

   const char* buf;

   buf= kwl.find(prefix, rspfKeywordNames::CONVERGENCE_THRESHOLD_KW);

   if (buf)
   {
      theConvergenceThreshold = atof(buf);
   }
   else
   {
      theConvergenceThreshold = .00000000000002;
   }
   
   buf= kwl.find(prefix, rspfKeywordNames::MAX_ITERATIONS_KW);
   if(buf)
   {
      theMaxIterations = atoi(buf);
   }
   else
   {
      theMaxIterations = 10;
   }
   const char* dxdy = kwl.find(prefix, "dxdy");
   if(dxdy)
   {
      rspfString tempString(dxdy);
      std::vector<rspfString> splitArray;
      tempString = tempString.trim();
      tempString.split(splitArray, " ");
      if(splitArray.size()==2)
      {
         theDxDy.x = splitArray[0].toDouble();
         theDxDy.y = splitArray[1].toDouble();
      }
   }
   if(result)
   {
      rspfObject::loadState(kwl, prefix);
   }
   
   return result;
}
   
void rspf2dTo2dTransform::forward(rspfDpt&  modify_this) const 
{
   rspfDpt output;
   forward(modify_this, output);
   modify_this = output;
}

void rspf2dTo2dTransform::inverse(rspfDpt&  modify_this) const 
{
   rspfDpt output;
   inverse(modify_this, output);
   modify_this = output;
}

rspfDpt rspf2dTo2dTransform::getOrigin()const
{
   return rspfDpt(0,0);
}

void rspf2dTo2dTransform::setConvergenceThreshold(const double& new_threshold)
{
   theConvergenceThreshold = new_threshold;
}

void rspf2dTo2dTransform::setMaxIterations(int new_max_iters)
{
   theMaxIterations = new_max_iters;
}

void rspf2dTo2dTransform::setDxDy(const rspfDpt& dxdy)
{
   theDxDy.x = dxdy.x;
   theDxDy.y = dxdy.y;
}

const rspf2dTo2dTransform& rspf2dTo2dTransform::operator=(const rspf2dTo2dTransform&  rhs )
{
   if (this != &rhs)
   {
      rspfObject::operator = (rhs);
      
      theConvergenceThreshold = rhs.theConvergenceThreshold;
      theMaxIterations        = rhs.theMaxIterations;
      theDxDy                 = rhs.theDxDy;
   }
   return *this;
}

std::ostream& rspf2dTo2dTransform::print(std::ostream& out) const
{
   out << "convergenceThreshold: " << theConvergenceThreshold << "\n"
   << "maxIterations:        " << theMaxIterations << "\n"
   << "dxdy:                 " << theDxDy << "\n";
   return out;
}

