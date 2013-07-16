#include <algorithm>
#include <rspf/font/rspfFontFactoryRegistry.h>
#include <rspf/font/rspfGdBitmapFont.h>
#if RSPF_HAS_FREETYPE
#  include <rspf/font/rspfFreeTypeFontFactory.h>
#endif
#include <rspf/font/rspfGdSansBold.inc>
rspfFontFactoryRegistry::rspfFontFactoryRegistry()
:theDefaultFont(NULL)
{
#if RSPF_HAS_FREETYPE
   registerFactory(rspfFreeTypeFontFactory::instance());
#endif
}
rspfFontFactoryRegistry* rspfFontFactoryRegistry::instance()
{
   static rspfFontFactoryRegistry sharedInstance;
   
   return &sharedInstance;
}
bool rspfFontFactoryRegistry::registerFactory(rspfFontFactoryBase* factory)
{
   bool result = false;
   if(factory&&!findFactory(factory))
   {
      theFactoryList.push_back(factory);
      result = true;
   }
   
   return result;
}
void rspfFontFactoryRegistry::unregisterFactory(rspfFontFactoryBase* factory)
{
   std::vector<rspfFontFactoryBase*>::iterator iter =  std::find(theFactoryList.begin(),
                                                                  theFactoryList.end(),
                                                                  factory);
   if(iter != theFactoryList.end())
   {
      theFactoryList.erase(iter);
   }
}
bool rspfFontFactoryRegistry::findFactory(rspfFontFactoryBase* factory)const
{
   return (std::find(theFactoryList.begin(),
                     theFactoryList.end(),
                     factory)!=theFactoryList.end());
}
rspfFont* rspfFontFactoryRegistry::createFont(const rspfFontInformation& information)const
{
   rspfFont* result = (rspfFont*)NULL;
   int i = 0;
   
   for(i= 0; ( (i < (int)theFactoryList.size()) &&(!result)); ++i)
   {
      result = theFactoryList[i]->createFont(information);
   }
   
   return result;
}
rspfFont* rspfFontFactoryRegistry::createFont(const rspfFilename& file)const
{
   rspfFont* result = (rspfFont*)NULL;
   int i = 0;
   
   for(i= 0; ( (i < (int)theFactoryList.size()) &&(!result)); ++i)
   {
      result = theFactoryList[i]->createFont(file);
   }
   
   return result;
}
void rspfFontFactoryRegistry::getFontInformation(std::vector<rspfFontInformation>& informationList)const
{
   int i = 0;
   for(i= 0; i < (int)theFactoryList.size(); ++i)
   {
      theFactoryList[i]->getFontInformation(informationList);
   }
}
void rspfFontFactoryRegistry::getFontInformationFamilyName(std::vector<rspfFontInformation>& informationList,
                                                            const rspfString& familyName)const
{
   vector<rspfFontInformation> info;
   getFontInformation(info);
   
   rspfString right = familyName;
   right = right.upcase().trim();
   int i = 0;
   for(i = 0; i < (int)info.size(); ++i)
   {
      rspfString left  = info[i].theFamilyName;
      left  = left.upcase().trim();
      
      if(left == right)
      {
         informationList.push_back(info[i]);
      }
   }
}
void rspfFontFactoryRegistry::getFontInformationContainingFamilyName(std::vector<rspfFontInformation>& informationList,
                                                                      const rspfString& familyName)const
{
   vector<rspfFontInformation> info;
   getFontInformation(info);
   
   rspfString right = familyName;
   right = right.upcase().trim();
   int i = 0;
   for(i = 0; i < (int)info.size(); ++i)
   {
      rspfString left  = info[i].theFamilyName;
      
      left  = left.upcase().trim();
      if(left.contains(right))
      {
         informationList.push_back(info[i]);
      }
   }
}
rspfFont* rspfFontFactoryRegistry::getDefaultFont()const
{
   if(!theDefaultFont)
   {
      std::vector<rspfFontInformation> infoList;
      getFontInformationContainingFamilyName(infoList,
                                             "Times");
      if(infoList.size() < 1)
      {
         getFontInformationContainingFamilyName(infoList,
                                                "Roman");
      }
      if(infoList.size() < 1)
      {
         getFontInformationContainingFamilyName(infoList,
                                                "Sanz");
      }
      if(infoList.size() < 1)
      {
         getFontInformation(infoList);
      }
      if(infoList.size())
      {
         theDefaultFont = createFont(infoList[0]);
         theDefaultFont->setPixelSize(12, 12);
      }
      else
      {
         theDefaultFont = new  rspfGdBitmapFont("gd sans",
                                                 "bold",
                                                 rspfGdSansBold);
      }
   }
   return theDefaultFont.get();
}
rspfFontFactoryRegistry::rspfFontFactoryRegistry(const rspfFontFactoryRegistry& /* rhs */ )
{
}
void rspfFontFactoryRegistry::operator=(const rspfFontFactoryRegistry& /* rhs */ )
{
}
