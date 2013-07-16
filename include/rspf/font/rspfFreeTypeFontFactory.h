#ifndef rspfFreeTypeFontFactory_HEADER
#define rspfFreeTypeFontFactory_HEADER
#include <rspf/font/rspfFontFactoryBase.h>
class rspfFreeTypeFontInformation
{
public:
   rspfFreeTypeFontInformation(const rspfFilename& file,
                                const rspfFontInformation& info)
      :theFilename(file),
       theFontInformation(info)
      {}
   rspfFilename        theFilename;
   rspfFontInformation theFontInformation;
};
class rspfFreeTypeFontFactory : public rspfFontFactoryBase
{
public:
   virtual ~rspfFreeTypeFontFactory();
   static rspfFreeTypeFontFactory* instance();
   virtual rspfFont* createFont(const rspfFontInformation& information)const;
   virtual rspfFont* createFont(const rspfFilename& file)const;
   virtual void getFontInformation(std::vector<rspfFontInformation>& informationList)const;
  
   bool addFile(const rspfFilename& file);
   
protected:
   rspfFreeTypeFontFactory();
   static rspfFreeTypeFontFactory* theInstance;
   std::vector<rspfFreeTypeFontInformation> theFontInformationList;
   void initializeDefaults();
private:
   rspfFreeTypeFontFactory(const rspfFreeTypeFontFactory& /*rhs*/){}
   void operator =(const rspfFreeTypeFontFactory& /*rhs*/){}
   
};
#endif
