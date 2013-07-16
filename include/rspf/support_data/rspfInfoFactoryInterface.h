//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Interface for info factories.
// 
//----------------------------------------------------------------------------
// $Id$
#ifndef rspfInfoFactoryInterface_HEADER
#define rspfInfoFactoryInterface_HEADER

#include <rspf/base/rspfConstants.h>
// #include <rspf/base/rspfObjectFactory.h>

class rspfFilename;
class rspfInfoBase;

//---
// Note: Deriving from rspfObjectFactory is being backed out as no
// rspfObjectFactory interfaces were ever called and it introduces warning in
// the header file so anyone who #includes this gets warnings...
//
// We can add back if there is a interface from rspfObjectFactory needed;
// else, please leave out.
// 
// drb 20120518
//---

/**
 * @brief Info factory.
 */
class RSPF_DLL rspfInfoFactoryInterface // : public rspfObjectFactory
{
public:

   /** default constructor */
   rspfInfoFactoryInterface(){}

   /** virtual destructor */
   virtual ~rspfInfoFactoryInterface(){}

   /**
    * @brief Pure virtual create method.  Derived classes must implement to
    * be concrete.
    *
    * @param file Some file you want info for.
    *
    * @return rspfInfoBase* on success 0 on failure.  Caller is responsible
    * for memory.
    */
   virtual rspfInfoBase* create(const rspfFilename& file) const = 0;

#if 0
   virtual rspfObject* createObject(const rspfString& typeName)const
   {
      return 0;
   }
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0)const
   {
      return 0;
   }
   /*!
    * This should return the type name of all objects in all factories.
    * This is the name used to construct the objects dynamially and this
    * name must be unique.
    */
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const
   {
      
   }
#endif
   
};

#endif /* End of "#ifndef rspfInfoFactoryInterface_HEADER" */
