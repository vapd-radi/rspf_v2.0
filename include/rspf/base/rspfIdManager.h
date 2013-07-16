#ifndef rspfIdManager_HEADER
#define rspfIdManager_HEADER

#include <rspf/base/rspfId.h>

class RSPFDLLEXPORT rspfIdManager
{
public:
   static rspfIdManager* instance();
   rspfId generateId();
   rspfId generateId(rspf_int64 customId);
   void setCurrentId(rspf_int64 customId);
   
protected:
   rspfIdManager(); // make sure this can't be constructed outside
   rspfIdManager(const rspfIdManager& rhs);
   ~rspfIdManager();
   void operator =(const rspfIdManager& rhs);

   static rspfIdManager* theInstance;
   static rspf_int64            theCurrentId;
};

#endif
