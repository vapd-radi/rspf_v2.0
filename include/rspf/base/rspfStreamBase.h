//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
// Description:
// Base class for all rspf streams...  Derives from rspfReferenced.
//
//*******************************************************************
//  $Id: rspfStreamBase.h 11176 2007-06-07 19:45:56Z dburken $
//
#ifndef rspfStreamBase_HEADER
#define rspfStreamBase_HEADER

#include <rspf/base/rspfConstants.h>  /* for RSPF_DLL */
#include <rspf/base/rspfReferenced.h> /* for base class */

class RSPF_DLL rspfStreamBase : public rspfReferenced
{
public:
   /** @brief default constructor */
   rspfStreamBase();

   /** @brief virtual destructor */
   virtual ~rspfStreamBase();

   /**
    * @brief isCompressed method.
    * @return true if compressed false if not compressed.
    * @note This default implementation returns false.  Derived classes should
    * override if needed.
    */
   virtual bool isCompressed() const;
};
#endif /* end of "#ifndef rspfStreamBase_HEADER" */
