//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfAdjustmentInfo.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfAdjustmentInfo_HEADER
#define rspfAdjustmentInfo_HEADER
#include <vector>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfAdjustableParameterInfo.h>

class rspfAdjustmentInfo
{
public:
   friend std::ostream& operator <<(std::ostream& out, const rspfAdjustmentInfo& data);
   
   
   rspfAdjustmentInfo(int numberOfAdjustableParameters=0);
   rspfAdjustmentInfo(const rspfAdjustmentInfo& rhs);
   
   void setNumberOfAdjustableParameters(rspf_uint32 numberOfAdjustableParameters);
   rspf_uint32 getNumberOfAdjustableParameters()const;
   rspfString getDescription()const;
   void setDescription(const rspfString& description);
   bool isDirty()const;
   void setDirtyFlag(bool flag=true);
   void setLockParameterFlag(bool flag,
                             rspf_uint32 idx);
   void keep();
   
   std::vector<rspfAdjustableParameterInfo>& getParameterList();
   const std::vector<rspfAdjustableParameterInfo>& getParameterList()const;
   void setLockFlag(bool flag,rspf_uint32 idx);
   
   bool saveState(rspfKeywordlist& kwl,
                  const rspfString& prefix=rspfString(""))const;
   bool loadState(const rspfKeywordlist& kwl,
                  const rspfString& prefix=rspfString(""));
   
private:
   std::vector<rspfAdjustableParameterInfo> theParameterList;
   rspfString                               theDescription;
   mutable bool                              theDirtyFlag;
};

#endif
