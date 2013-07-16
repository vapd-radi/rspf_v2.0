#ifndef rspfOgcWktTranslator_HEADER
#define rspfOgcWktTranslator_HEADER
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfString.h>
#include <map>
#include <string>
class rspfKeywordlist;
class rspfOgcWktTranslator
{
public:
   rspfOgcWktTranslator();
   
   bool toOssimKwl(const rspfString& wktString,
                   rspfKeywordlist& kwl,
                   const char* prefix=NULL)const;
   
   rspfString fromOssimKwl(const rspfKeywordlist& kwl,
                            const char* prefix=NULL)const;
   /*!
    * Returns the empty string if the datum is not found
    *
    */
   rspfString wktToOssimDatum(const rspfString& datum)const;
   rspfString rspfToWktDatum(const rspfString& datum)const;
   rspfString wktToOssimProjection(const rspfString& projection)const;
   rspfString rspfToWktProjection(const rspfString& projection)const;
   
protected:
   std::map<std::string, std::string> theWktToOssimDatumTranslation;
   std::map<std::string, std::string> theWktToOssimProjectionTranslation;
   std::map<std::string, std::string> theOssimToWktDatumTranslation;
   std::map<std::string, std::string> theOssimToWktProjectionTranslation;
   void initializeDatumTable();
   void initializeProjectionTable();
   
};
#endif
