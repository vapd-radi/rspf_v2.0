#include <rspf/base/rspfOutputSource.h>

RTTI_DEF1(rspfOutputSource, "rspfOutputSource", rspfSource);

bool rspfOutputSource::saveState(rspfKeywordlist& kwl,
                                  const char* prefix)const
{
   return rspfSource::saveState(kwl, prefix);
}

bool rspfOutputSource::loadState(const rspfKeywordlist& kwl,
                                  const char* prefix)
{
   return rspfSource::loadState(kwl, prefix);
}
