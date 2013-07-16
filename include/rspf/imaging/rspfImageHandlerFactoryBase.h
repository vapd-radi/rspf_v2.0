//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfImageHandlerFactoryBase.h 22228 2013-04-12 14:11:45Z dburken $

#ifndef rspfImageHandlerFactoryBase_HEADER
#define rspfImageHandlerFactoryBase_HEADER 1

#include <algorithm>

#include <rspf/base/rspfString.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/base/rspfObjectFactory.h>

class rspfFilename;
class rspfKeywordlist;

class RSPF_DLL rspfImageHandlerFactoryBase : public rspfObjectFactory
{
public:
   template <class T> class UniqueList
   {
   public:
      
      void push_back(const T& value)
      {
         if(std::find(theList.begin(),theList.end(), value) == theList.end())
         {
            theList.push_back(value);
         }
      }

      const std::vector<T>& getList()const
      {
         return theList;
      }
      rspf_uint32 size()const
      {
         return theList.size();
      }
      const T& operator[](rspf_uint32 idx)const
      {
         return theList[idx];
      }
      T& operator[](rspf_uint32 idx)
      {
         return theList[idx];
      }
   protected:
      std::vector<T> theList;
   };
   
   typedef UniqueList<rspfString> UniqueStringList;
   typedef std::vector<rspfRefPtr<rspfImageHandler> > ImageHandlerList;
   
   virtual rspfImageHandler* open(const rspfFilename& fileName,
                                   bool openOverview=true)const = 0;
   virtual rspfImageHandler* open(const rspfKeywordlist& kwl,
                                   const char* prefix=0)const = 0;

   /**
    * @brief Open overview that takes a file name.
    *
    * This default implementation returns a null ref pointer.
    * Derived factories that have overview readers should override.
    * 
    * @param file File to open.
    *
    * @return This default implementation returns a null ref pointer.
    */
   virtual rspfRefPtr<rspfImageHandler> openOverview(
      const rspfFilename& file ) const;

   virtual void getImageHandlersBySuffix(ImageHandlerList& result,
                                         const rspfString& ext)const;

   virtual void getImageHandlersByMimeType(ImageHandlerList& result,
                                           const rspfString& mimeType)const;

   virtual void getSupportedExtensions(rspfImageHandlerFactoryBase::UniqueStringList& extensionList)const=0;
   
TYPE_DATA
};

#endif
