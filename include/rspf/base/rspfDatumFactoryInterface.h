//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file
//
// Author:  David Burken
//
// Description: Interface class for datum factories.
//
//----------------------------------------------------------------------------
// $Id$

#ifndef rspfDatumFactoryInterface_HEADER
#define rspfDatumFactoryInterface_HEADER

#include <rspf/base/rspfConstants.h> /* for RSPF_DLL macro */
#include <vector>

// Forward class declarations:
class rspfString;
class rspfDatum;
class rspfKeywordlist;

class RSPF_DLL rspfDatumFactoryInterface
{
public:
   
   /** default constructor */
   rspfDatumFactoryInterface(){}
   
   /** virtual destructor */
   virtual ~rspfDatumFactoryInterface(){}

   /**
   * Pure virtual create method.
   *
   * @return const pointer to a datum.
   */
   virtual const rspfDatum* create(const rspfString& code) const = 0;
   virtual const rspfDatum* create(const rspfKeywordlist& kwl, const char *prefix=0) const = 0;

   /**
    * Pure virtual getList method.
    *
    * @param list The list to add to.
    */
   virtual void getList(std::vector<rspfString>& list) const = 0;
};

#endif /* #ifndef rspfDatumFactoryInterface_HEADER */
