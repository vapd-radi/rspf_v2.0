#include <rspf/rspfConfig.h> /* To pick up RSPF_HAS_FREETYPE. */
#if RSPF_HAS_FREETYPE
#  include <rspf/font/rspfFreeTypeFontFactory.h>
#  include <rspf/font/rspfFreeTypeFont.h>
#  include <rspf/base/rspfPreferences.h>
#  include <rspf/base/rspfDirectory.h>
#  include <rspf/base/rspfNotifyContext.h>
rspfFreeTypeFontFactory* rspfFreeTypeFontFactory::theInstance=0;
rspfFreeTypeFontFactory::rspfFreeTypeFontFactory()
{
   theInstance = this;
}
rspfFreeTypeFontFactory::~rspfFreeTypeFontFactory()
{
   theInstance = 0;
}
rspfFreeTypeFontFactory* rspfFreeTypeFontFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfFreeTypeFontFactory;
      theInstance->initializeDefaults();
   }
   return theInstance;
}
rspfFont* rspfFreeTypeFontFactory::createFont(const rspfFontInformation& information)const
{
   rspfRefPtr<rspfFont> result;
   
   rspfString rightFamilyName = information.theFamilyName;
   rspfString rightStyleName  = information.theStyleName;
   rightFamilyName = rightFamilyName.upcase().trim();
   rightStyleName  = rightStyleName.upcase().trim();
   int i = 0;
   
   for(i = 0; ( (i < (int)theFontInformationList.size())&&(!result));++i)
   {
      rspfString leftFamilyName = theFontInformationList[i].theFontInformation.theFamilyName;
      rspfString leftStyleName = theFontInformationList[i].theFontInformation.theStyleName;
      leftFamilyName = leftFamilyName.upcase().trim();
      leftStyleName  = leftStyleName.upcase().trim();
      
      if( (leftFamilyName == rightFamilyName)&&
          (leftStyleName == rightStyleName))
      {
         if(theFontInformationList[i].theFontInformation.theFixedFlag)
         {
            if(theFontInformationList[i].theFontInformation.thePointSize ==
               information.thePointSize)
            {
               result = new rspfFreeTypeFont(theFontInformationList[i].theFilename);
               if(result.valid())
               {
                  if(result->getErrorStatus())
                  {
                     result = 0;
                     rspfNotify(rspfNotifyLevel_WARN) <<"WARNING rspfFreeTypeFontFactory::createFont:  unable to open font file "
                                                        << theFontInformationList[i].theFilename << std::endl;
                  }
               }
            }
         }
         else
         {
            result = new rspfFreeTypeFont(theFontInformationList[i].theFilename);
            if(result.valid())
            {
               if(result->getErrorStatus())
               {
                  result = 0;
                  rspfNotify(rspfNotifyLevel_WARN) <<  "WARNING rspfFreeTypeFontFactory::createFont: unable to open font file "
                                                     << theFontInformationList[i].theFilename << std::endl;
               }
               else
               {
                  if(information.thePointSize.x&&
                     information.thePointSize.y)
                  {
                     result->setPixelSize(information.thePointSize.x,
                                          information.thePointSize.y);
                  }
               }
            }
         }
      }         
   }
   if(result.valid())
   {
      result->setScale(information.theScale.x,
                       information.theScale.y);
      result->setScale(information.theScale.x,
                       information.theScale.y);
      result->setHorizontalVerticalShear(information.theShear.x,
                                         information.theShear.y);
      result->setRotation(information.theRotation);
   }
   
   return result.release();
}
rspfFont* rspfFreeTypeFontFactory::createFont(const rspfFilename& file)const
{
   rspfRefPtr<rspfFont> result = new rspfFreeTypeFont(file);
   if(result->getErrorStatus())
   {
      result = 0;
   }
   
   return result.release();
}
void rspfFreeTypeFontFactory::getFontInformation(std::vector<rspfFontInformation>& informationList)const
{
   int i = 0;
   for(i = 0; i < (int)theFontInformationList.size();++i)
   {
      informationList.push_back(theFontInformationList[i].theFontInformation);
   }
}
bool rspfFreeTypeFontFactory::addFile(const rspfFilename& file)
{
   rspfRefPtr<rspfFreeTypeFont> font = new rspfFreeTypeFont(file);
   
   std::vector<rspfFontInformation> fontInfoList;
   
   bool result = false;
   int i = 0;
   if(font->getFontFace())
   {
      if(!font->getErrorStatus())
      {
         font->getFontInformation(fontInfoList);
	 
         for(i = 0; i < (int)fontInfoList.size();++i)
         {
            theFontInformationList.push_back(rspfFreeTypeFontInformation(file,
                                                                          fontInfoList[i]));
         }
         result = true;
      }
   }
   font = 0;
   
   return result;
}
void rspfFreeTypeFontFactory::initializeDefaults()
{
   const rspfKeywordlist& kwl = rspfPreferences::instance()->preferencesKWL();
   rspfString regExpressionDir =  rspfString("^(") + "font.dir[0-9]+)";
   rspfString regExpressionFile =  rspfString("^(") + "font.file[0-9]+)";
   vector<rspfString> dirs =
      kwl.getSubstringKeyList( regExpressionDir );
   vector<rspfString> files =
      kwl.getSubstringKeyList( regExpressionFile );
   
   int idx = 0;
   for(idx = 0; idx < (int)dirs.size();++idx)
   {
      const char* directoryLocation = kwl.find(dirs[idx]);
      if(directoryLocation)
      {
         rspfDirectory d;
	 
         if(d.open(rspfFilename(directoryLocation)))
         {
            rspfFilename file;
	    
            if(d.getFirst(file, rspfDirectory::RSPF_DIR_FILES))
            {
               do
               {
                  addFile(file);
               }while(d.getNext(file));
            }
         }
      }
   }
   for(idx = 0; idx < (int)files.size();++idx)
   {
      const char* fileName = kwl.find(files[idx]);
      
      if(fileName)
      {
         addFile(rspfFilename(fileName));
      }
   }   
#if 0
   if(!numberOfDirs&&!numberOfFiles)
   {
#ifdef __UNIX__
      rspfFilename file("/usr/X11R6/lib/X11/fonts/Type1");
      
      if(file.exists())
      {
         rspfDirectory d;
         if(d.open(file))
         {
            if(d.getFirst(file, rspfDirectory::RSPF_DIR_FILES))
            {
               do
               {
                  addFile(file);
               }while(d.getNext(file));
            }
         }
      }
#endif
   }
#endif
}
#endif /* #if RSPF_HAS_FREETYPE */
