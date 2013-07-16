#ifndef rspfPluginLibrary_HEADER
#define rspfPluginLibrary_HEADER
#include <rspf/plugin/rspfSharedObjectBridge.h>
#include <rspf/plugin/rspfDynamicLibrary.h>
class RSPFDLLEXPORT rspfPluginLibrary : public rspfDynamicLibrary
{
public:
   rspfPluginLibrary();
   rspfPluginLibrary(const rspfString& name, const rspfString& options="");
   virtual ~rspfPluginLibrary();
   void initialize();
   void finalize();
   rspfString getDescription()const;
   void getClassNames(std::vector<rspfString>& classNames)const;
   void setOptions(const rspfString& options);
protected:
   rspfString m_options;
   rspfSharedObjectInfo* m_info;
TYPE_DATA
};
#endif 
