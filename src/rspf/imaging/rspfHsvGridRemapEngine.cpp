//*****************************************************************************
// FILE: rspfHsvGridRemapEngine.cc
//
// Copyright (C) 2001 ImageLinks, Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// AUTHOR: Oscar Kramer
//
// DESCRIPTION: Contains implementation of class 
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfHsvGridRemapEngine.cpp 15833 2009-10-29 01:41:53Z eshirschorn $

#include <rspf/imaging/rspfHsvGridRemapEngine.h>

RTTI_DEF1(rspfHsvGridRemapEngine, "rspfHsvGridRemapEngine",
          rspfGridRemapEngine);

#include <rspf/imaging/rspfGridRemapSource.h>
#include <rspf/imaging/rspfAtbPointSource.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfDblGrid.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/base/rspfRgbVector.h>
#include <rspf/base/rspfHsvVector.h>

//***
// Define Trace flags for use within this file:
//***
#include <rspf/base/rspfTrace.h>
static rspfTrace traceExec  ("rspfHsvGridRemapEngine:exec");
static rspfTrace traceDebug ("rspfHsvGridRemapEngine:debug");

//*****************************************************************************
//  METHOD: rspfHsvGridRemapEngine::remapTile
//  
//*****************************************************************************
rspfObject* rspfHsvGridRemapEngine::dup() const
{
   return new rspfHsvGridRemapEngine;
}

//*****************************************************************************
//  METHOD: rspfHsvGridRemapEngine::remapTile
//  
//*****************************************************************************
void rspfHsvGridRemapEngine::remapTile(const rspfDpt&       origin,
                                        rspfGridRemapSource* remapper,
                                        rspfRefPtr<rspfImageData>& tile)
{
   static const char MODULE[] = "rspfHsvGridRemapEngine::remapTile";
   if (traceExec())  CLOG << "entering..." << endl;

   //***
   // Fetch tile size and NULL pixel value:
   //***
   int    width         = tile->getWidth();
   int    height        = tile->getHeight();
   int    offset        = 0;
   
   void* red_buf = tile->getBuf(0);
   void* grn_buf = tile->getBuf(1);
   void* blu_buf = tile->getBuf(2);

   rspfDblGrid& gridH = *(remapper->getGrid(0));
   rspfDblGrid& gridS = *(remapper->getGrid(1));
   rspfDblGrid& gridV = *(remapper->getGrid(2));
      
   //---
   // Remap according to pixel type:
   //---
   switch(tile->getScalarType())
   {
      case RSPF_UINT8:
      {
         for (double line=origin.line; line<origin.line+height; line+=1.0)
         {
            for (double samp=origin.samp; samp<origin.samp+width; samp+=1.0)
            {
               //---
               // Fetch pixel from the input tile band buffers and convert
               // to HSV:
               //---
               rspfRgbVector rgb_pixel (((rspf_uint8*)red_buf)[offset],
                                         ((rspf_uint8*)grn_buf)[offset],
                                         ((rspf_uint8*)blu_buf)[offset]);
               rspfHsvVector hsv_pixel (rgb_pixel);
               
               //---
               // Remap pixel HSV  with spatially variant bias value:
               //---
               hsv_pixel.setH(hsv_pixel.getH() + gridH(samp,line));
               hsv_pixel.setS(hsv_pixel.getS() + gridS(samp,line));
               hsv_pixel.setV(hsv_pixel.getV() + gridV(samp,line));
               
               //---
               // Convert back to RGB and write to the tile:
               //---
               rgb_pixel = hsv_pixel;  // auto-clamped
               ((rspf_uint8*)red_buf)[offset] = rgb_pixel.getR();
               ((rspf_uint8*)grn_buf)[offset] = rgb_pixel.getG();
               ((rspf_uint8*)blu_buf)[offset] = rgb_pixel.getB();
               
               offset++;
            }
         }
         break;
      }
      
      case RSPF_USHORT11:
         break;
         
      case RSPF_UINT16:
         break;
         
      case RSPF_SINT16:
         break;	

      case RSPF_FLOAT64:
         break;	

      case RSPF_NORMALIZED_DOUBLE:
         break;	

      case RSPF_FLOAT32:
         break;	

      case RSPF_NORMALIZED_FLOAT:
         break;	

      case RSPF_SCALAR_UNKNOWN:
      default:
         break;

   }   // end switch statement

   if (traceExec())  CLOG << "returning..." << endl;
   return;
};

//*****************************************************************************
//  METHOD: rspfHsvGridRemapEngine::assignRemapValues
//
//  This engine defines the target value as an HSV vector of doubles, computed
//  as the mean of all contributor HSV values.
//  
//*****************************************************************************
void rspfHsvGridRemapEngine::assignRemapValues (
   vector<rspfAtbPointSource*>& sources_list)
{
   static const char MODULE[] = "rspfHsvGridRemapEngine::assignRemapValues";
   if (traceExec())  CLOG << "entering..." << endl;

   int i; // index to individual sources

   //***
   // Declare a 2D array that will contain all of the contributing sources'
   // HSV mean values. Also declare the accumulator target vector.
   //***
   int num_contributors = (int)sources_list.size();
   double** contributor_pixel = new double* [num_contributors];
   for (i=0; i<num_contributors; i++)
      contributor_pixel[i] = new double[3];
   double target_pixel[3] = {0.0, 0.0, 0.0};

   //***
   // Now loop over each remaining contributor and sum in its contribution:
   //***
   vector<rspfAtbPointSource*>::iterator source;
   i = 0;
   for(source  = sources_list.begin();
       source != sources_list.end();
       source++)
   {
      (*source)->getSourceValue(contributor_pixel[i]);

      target_pixel[0] += contributor_pixel[i][0]/(double)num_contributors;
      target_pixel[1] += contributor_pixel[i][1]/(double)num_contributors;
      target_pixel[2] += contributor_pixel[i][2]/(double)num_contributors;

      i++;
   }

   //***
   // The target pixel has been established. Now need to compute the actual
   // remap quantities that will be written to the appropriate remap grids:
   //***
   i = 0;
   for(source  = sources_list.begin();
       source != sources_list.end();
       source++)
   {
      computeRemapNode(*source, contributor_pixel[i], target_pixel);
      i++;
   }

   //***
   // Delete locally allocated memory:
   //***
   for (i=0; i<num_contributors; i++)
      delete [] contributor_pixel[i];
   delete [] contributor_pixel;
   
   if (traceExec())  CLOG << "returning..." << endl;
   return;
}

//*****************************************************************************
//  METHOD: rspfHsvGridRemapEngine::computeSourceValue
//  
//*****************************************************************************
void rspfHsvGridRemapEngine::computeSourceValue(
   rspfRefPtr<rspfImageData>& source, void* result)
{
   static const char MODULE[]="rspfHsvGridRemapEngine::computeSourceValue";
   if (traceExec())  CLOG << "entering..." << endl;

   //***
   // This engine defines "value" as the HSV vector corresponding to the mean
   // RGB pixel value of the source data:
   //***
   rspfRgbVector rgb_vector;
   rgb_vector.setR((unsigned char) source->computeAverageBandValue(0));
   rgb_vector.setG((unsigned char) source->computeAverageBandValue(1));
   rgb_vector.setB((unsigned char) source->computeAverageBandValue(2));

   //***
   // Assign the HSV components to the result vector:
   //***
   rspfHsvVector hsv_vector (rgb_vector);
   ((double*)result)[0] = (double) hsv_vector.getH();
   ((double*)result)[1] = (double) hsv_vector.getS();
   ((double*)result)[2] = (double) hsv_vector.getV();
   
   if (traceExec())  CLOG << "returning..." << endl;
   return;
}

//*****************************************************************************
//  METHOD: rspfHsvGridRemapEngine::computeRemapNode
//
//  This engine defines the remap value as the difference between the target
//  HSV vector and the individual point source's value vector.
//
//*****************************************************************************
void rspfHsvGridRemapEngine::computeRemapNode(rspfAtbPointSource* ps,
                                              void* source_value,
                                              void* target_value)
{
   static const char MODULE[] = "rspfHsvGridRemapEngine::computeRemapNode";
   if (traceExec())  CLOG << "entering..." << endl;

   //***
   // Compute the remap grid node value specific to this HSV implementation:
   //***
   double node[3];
   node[0] = ((double*)target_value)[0] - ((double*)source_value)[0];
   node[1] = ((double*)target_value)[1] - ((double*)source_value)[1];
   node[2] = ((double*)target_value)[2] - ((double*)source_value)[2];

   //***
   // Fetch a pointer to the remapper feeding this point source in order to
   // pass it the node value:
   //***
   rspfGridRemapSource* remapper = ps->getRemapSource();
   remapper->setGridNode(ps->getViewPoint(), node);
                         
   if (traceExec())  CLOG << "returning..." << endl;
   return;
}

