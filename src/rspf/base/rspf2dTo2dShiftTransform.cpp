#include <rspf/base/rspf2dTo2dShiftTransform.h>
#include <rspf/base/rspfKeywordlist.h>
RTTI_DEF1(rspf2dTo2dShiftTransform, "rspf2dTo2dShiftTransform", rspf2dTo2dTransform);
bool rspf2dTo2dShiftTransform::saveState(rspfKeywordlist& kwl,
                                          const char* prefix)const
{
   bool result = rspf2dTo2dTransform::saveState(kwl, prefix);
   kwl.add(prefix, "shift", m_shift.toString(), true);
   
   return result;
}

bool rspf2dTo2dShiftTransform::loadState(const rspfKeywordlist& kwl,
                                          const char* prefix)
{
   bool result = rspf2dTo2dTransform::loadState(kwl, prefix);
   
   rspfString shift = kwl.find(prefix, "shift");
   if(!shift.empty())
   {
      m_shift.toPoint(shift);
   }
   
   return result;
}
