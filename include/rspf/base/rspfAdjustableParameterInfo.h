//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfAdjustableParameterInfo.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfAdjustableParameterInfo_HEADER
#define rspfAdjustableParameterInfo_HEADER
#include <iostream>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfKeywordlist.h>

class RSPF_DLL rspfAdjustableParameterInfo
{
public:
   RSPF_DLL friend std::ostream& operator <<(std::ostream& out, const rspfAdjustableParameterInfo& data);
   
   rspfAdjustableParameterInfo()
      : theParameter(0.0),
        theSigma(0.0),
	theCenter(0.0),
        theUnit(RSPF_UNIT_UNKNOWN),
      theDescription(""),
      theLockFlag(false)
      {
      }
   rspfAdjustableParameterInfo(const rspfAdjustableParameterInfo& rhs)
      :theParameter(rhs.theParameter),
      theSigma(rhs.theSigma),
      theCenter(rhs.theCenter),
      theUnit(rhs.theUnit),
      theDescription(rhs.theDescription),
      theLockFlag(rhs.theLockFlag)
      {
      }
   double getParameter()const
      {
         return theParameter;
      }
   void setParameter(double parameter)
      {
         if(!theLockFlag)
         {
            theParameter = parameter;
         }
      }
   double getSigma()const
      {
         return theSigma;
      }
   void setSigma(double sigma)
      {
         if(!theLockFlag)
         {
            theSigma = sigma;
         }
      }
   const rspfString& getDescription()const
      {
         return theDescription;
      }
   
   void setDescription(const rspfString& description)
      {
         if(!theLockFlag)
         {
            theDescription = description;
         }
      }
   
   rspfUnitType getUnit()const
      {
         return theUnit;
      }
   void setUnit(rspfUnitType unit)
      {
         theUnit = unit;
      }
   rspfString getUnitAsString()const;

   void setCenter(double center);
   double getCenter()const;

   void setOffset(rspf_float64 value);
   
  /*!
   * will return theCenter + theSigma*theParameter
   */
   double computeOffset()const;

   void setLockFlag(bool flag)
   {
      theLockFlag = flag;
   }
   bool getLockFlag()const
   {
      return theLockFlag;
   }
   
   bool loadState(const rspfKeywordlist& kwl,
                  const rspfString& prefix=rspfString(""));
   
   bool saveState(rspfKeywordlist& kwl,
                  const rspfString& prefix=rspfString(""))const;
protected:
   double        theParameter;
   double        theSigma;
   double        theCenter;
   rspfUnitType theUnit;
   rspfString   theDescription;
   bool          theLockFlag;
};

#endif
