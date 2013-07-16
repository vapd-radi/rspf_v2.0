#ifndef rspfFontFactoryRegistry_HEADER
#define rspfFontFactoryRegistry_HEADER
#include <vector>
#include <rspf/base/rspfFontInformation.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/font/rspfFontFactoryBase.h>
#include <rspf/font/rspfFont.h>
#include <rspf/base/rspfRefPtr.h>
class RSPFDLLEXPORT rspfFontFactoryRegistry
{
public:
   static rspfFontFactoryRegistry* instance();
   bool registerFactory(rspfFontFactoryBase* factory);
   void unregisterFactory(rspfFontFactoryBase* factory);
   bool findFactory(rspfFontFactoryBase* factory)const;
   
   rspfFont* createFont(const rspfFontInformation& information)const;
   rspfFont* createFont(const rspfFilename& file)const;
   /*!
    * Returns all font information.
    */
   void getFontInformation(std::vector<rspfFontInformation>& informationList)const;
   /*!
    * Matches the entire string.  Does a case insensitive match.
    */ 
   void getFontInformationFamilyName(std::vector<rspfFontInformation>& informationList,
                                     const rspfString& familyName)const;
   /*!
    * Matches the sub-string.  Does a case insensitive match.
    */ 
   void getFontInformationContainingFamilyName(std::vector<rspfFontInformation>& informationList,
                                               const rspfString& familyName)const;
   /*!
    * This font should not be deleted.  It returns the default font.  If you
    * want to make a copy then just call the dup method on
    * font.
    */
   rspfFont* getDefaultFont()const;
   
protected:
   mutable rspfRefPtr<rspfFont> theDefaultFont;
   std::vector<rspfFontFactoryBase*> theFactoryList;
   rspfFontFactoryRegistry();
   
private:
   rspfFontFactoryRegistry(const rspfFontFactoryRegistry& rhs);
   void operator =(const rspfFontFactoryRegistry& rhs);
};
#endif
