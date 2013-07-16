//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfAdjustableParameterInterface.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfAdjustableParameterInterface_HEADER
#define rspfAdjustableParameterInterface_HEADER
#include <rspf/base/rspfRtti.h>
#include <vector>
#include <rspf/base/rspfAdjustmentInfo.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfObject.h>

class RSPFDLLEXPORT rspfAdjustableParameterInterface
{
public:
   rspfAdjustableParameterInterface();
   rspfAdjustableParameterInterface(const rspfAdjustableParameterInterface& rhs);
   virtual ~rspfAdjustableParameterInterface(){}
   void newAdjustment(rspf_uint32 numberOfParameters=0);
   void setAdjustmentDescription(const rspfString& description);
   rspfString getAdjustmentDescription()const;
   void setCurrentAdjustment(rspf_uint32 adjustmentIndex, bool notify=false);
   void eraseAdjustment(bool notify);
   void eraseAdjustment(rspf_uint32 idx, bool notify);
   virtual void initAdjustableParameters();
   void resetAdjustableParameters(bool notify=false);
   void copyAdjustment(rspf_uint32 idx, bool notify);
   void copyAdjustment(bool notify = false);

   virtual rspfObject* getBaseObject()=0;
   virtual const rspfObject* getBaseObject()const=0;
   
   /*!
    * Will copy the adjustment but will set the new center to the
    * applied current center plus the application of the adjustment
    *
    */
   void keepAdjustment(rspf_uint32 idx, bool createCopy);
   void keepAdjustment(bool createCopy=true);
   

   const rspfAdjustableParameterInterface& operator = (const rspfAdjustableParameterInterface& rhs);
   void removeAllAdjustments();
   rspf_uint32 getNumberOfAdjustableParameters()const;
   double       getAdjustableParameter(rspf_uint32 idx)const;
   void         setAdjustableParameter(rspf_uint32 idx, double value,
                                       bool notify=false);
   void         setAdjustableParameter(rspf_uint32 idx,
                                       double value,
                                       double sigma,
                                       bool notify=false);
   double       getParameterSigma(rspf_uint32 idx)const;
   void         setParameterSigma(rspf_uint32 idx,
                                  double value,
                                  bool notify=false);
   rspfUnitType getParameterUnit(rspf_uint32 idx)const;
   void          setParameterUnit(rspf_uint32 idx, rspfUnitType unit);
   void          setParameterUnit(rspf_uint32 idx, const rspfString& unit);

   void           setParameterCenter(rspf_uint32 idx,
                                     double center,
                                     bool notify = false);
   double        getParameterCenter(rspf_uint32 idx)const;
   double        computeParameterOffset(rspf_uint32 idx)const;
   void          setParameterOffset(rspf_uint32 idx,
                                    rspf_float64 value,
                                    bool notify = false);
   
   rspfString   getParameterDescription(rspf_uint32 idx)const;
   void          setParameterDescription(rspf_uint32 idx,
                                         const rspfString& descrption);

   rspf_int32 findParameterIdxGivenDescription(rspf_uint32 adjustmentIdx,
                                                const rspfString& name)const;
   rspf_int32 findParameterIdxContainingDescription(rspf_uint32 adjustmentIdx,
                                                     const rspfString& name)const;
   
   bool isParameterLocked(rspf_uint32 idx)const;

   void setParameterLockFlag(rspf_uint32 idxParam, bool flag);
   bool getParameterLockFlag(rspf_uint32 idx)const;

   void lockAllParametersCurrentAdjustment();
   void unlockAllParametersCurrentAdjustment();

   void lockAllParameters(rspf_uint32 idxAdjustment);
   void unlockAllParameters(rspf_uint32 idxAdjustment);
   
   void resizeAdjustableParameterArray(rspf_uint32 numberOfParameters);

   void setAdjustment(const rspfAdjustmentInfo& adj, bool notify=false);
   void setAdjustment(rspf_uint32 idx, const rspfAdjustmentInfo& adj, bool notify=false);
   
   void addAdjustment(const rspfAdjustmentInfo& adj, bool notify);
   void getAdjustment(rspfAdjustmentInfo& adj);
   void getAdjustment(rspf_uint32 idx, rspfAdjustmentInfo& adj);
   
   rspf_uint32 getNumberOfAdjustments()const;
   rspf_uint32 getCurrentAdjustmentIdx()const;

   
   void setDirtyFlag(bool flag=true);
   void setAllDirtyFlag(bool flag = true);
   bool hasDirtyAdjustments()const;
   
   bool saveAdjustments(rspfKeywordlist& kwl,
                        const rspfString& prefix=rspfString(""))const;
   bool loadAdjustments(const rspfKeywordlist& kwl,
                        const rspfString& prefix=rspfString(""));
private:
   std::vector<rspfAdjustmentInfo> theAdjustmentList;
   rspf_uint32                     theCurrentAdjustment;
   
public:
   virtual void adjustableParametersChanged();
   
TYPE_DATA
};

#endif
