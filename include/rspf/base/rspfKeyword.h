//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Ken Melero
// Description: This class defines a keyword.
//              NOTE:  The keywordlist class cleans up all allocated
//                     memory from instantiating a keyword.
// 
//********************************************************************
// $Id: rspfKeyword.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfKeyword_HEADER
#define rspfKeyword_HEADER

#include <rspf/base/rspfString.h>

class RSPFDLLEXPORT rspfKeyword
{
public:
   rspfKeyword();
   rspfKeyword(const char* key, const char* description);
   rspfKeyword(const char* key, const char* description, const char* value);
   rspfKeyword(const rspfKeyword& src);
   ~rspfKeyword();

   const rspfKeyword& operator=(const rspfKeyword& kw);
   operator const char*() const;

   const char* key()               const;
   const char* description()       const;
   const char* value()             const;
   rspfString keyString()         const;
   rspfString descriptionString() const;
   rspfString valueString()       const;
   
   void setValue(const char* val);
   void setValue(const rspfString& val);
   void setDescription(const char* desc);
   void setDescription(const rspfString& desc);
   

   friend RSPF_DLL std::ostream& operator<<(std::ostream& os, const rspfKeyword& kw);

private:
   rspfString theKey;
   rspfString theDesc;
   rspfString theValue;
};

#endif
