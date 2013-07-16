#ifndef rspfFontFactoryBase_HEADER
#define rspfFontFactoryBase_HEADER
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfFontInformation.h>
class rspfFont;
class rspfFontFactoryBase
{
public:
   virtual ~rspfFontFactoryBase(){}
   
   /*!
    * Will find the best font for the passed in infromation.
    * If none is found a default font should be returned.
    */
   virtual rspfFont* createFont(const rspfFontInformation& information)const=0;
   /*!
    * Usually for trutype fonts but any font stored in a file it
    * will try to create a font from it.
    */
   virtual rspfFont* createFont(const rspfFilename& file)const=0;
   virtual void getFontInformation(std::vector<rspfFontInformation>& informationList)const=0;
};
#endif
