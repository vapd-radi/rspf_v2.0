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
// $Id: rspfValueAssignImageSourceFilter.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfValueAssignImageSourceFilter_HEADER
#define rspfValueAssignImageSourceFilter_HEADER

#include <rspf/imaging/rspfImageSourceFilter.h>

/**
 * This allows one to assign a value to a group of bands.  It will test
 * for equality band separate or as group. For example:
 *
 * if we are testing as a group then the assign will not happen unless
 * all values equal theInputValue array
 *
 *  inputValue = <1, 2, 3>
 *  outputValue = <100, 100, 100>
 *
 *  then it will only convert to 100, 100, 100 if the input equals all
 *  values of the output.
 *
 * input <1, 55, 3>  would become <1, 55, 3>
 * input <1, 2, 3>   would become <100, 100, 100>
 *
 *  Now for band separate it means you want to change on a per band and
 *  not as a group.  For example:
 *
 * inputValue = <1, 2, 3>
 * outputValue = <100, 100, 100>
 *
 * input <1, 55, 3>  would become <100, 55, 100>
 *
 */
class RSPFDLLEXPORT rspfValueAssignImageSourceFilter : public rspfImageSourceFilter
{
public:
   enum rspfValueAssignType
   {
      rspfValueAssignType_SEPARATE = 1,
      rspfValueAssignType_GROUP    = 2
   };
   
   rspfValueAssignImageSourceFilter(rspfObject* owner=NULL);
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                   rspf_uint32 resLevel=0);

   void setInputOutputValues(const vector<double>& inputValues,
                             const vector<double>& outputValues);

   const vector<double>& getInputValues()const;
   const vector<double>& getOutputValues()const;
   rspfValueAssignType getValueAssignType()const;
   void setValueAssignType(rspfValueAssignType type);

   virtual void initialize();

   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=NULL)const;
   
   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=NULL);
   
protected:
   virtual ~rspfValueAssignImageSourceFilter();

   /**
    * Called on first getTile, will initialize all data needed.
    */
   void allocate();
   
   vector<double>              theInputValueArray;
   vector<double>              theOutputValueArray;
   rspfValueAssignType        theAssignType;
   rspfRefPtr<rspfImageData> theTile;
   
   void validateArrays();
   
   template <class T>
      void executeAssign(T, rspfRefPtr<rspfImageData>& data);
   
   template <class T>
      void executeAssignSeparate(T, rspfRefPtr<rspfImageData>& data);
   
   template <class T>
      void executeAssignGroup(T, rspfRefPtr<rspfImageData>& data);

TYPE_DATA
};

#endif /* #ifndef rspfValueAssignImageSourceFilter_HEADER */
