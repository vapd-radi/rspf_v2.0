//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Base class for Info object.
// 
//----------------------------------------------------------------------------
// $Id$
#ifndef rspfInfoBase_HEADER
#define rspfInfoBase_HEADER

#include <iosfwd>

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfReferenced.h>

class rspfFilename;
class rspfKeywordlist;

/**
 * @brief Info Base.
 *
 * This is the base class for all info objects.  The purpose of an Info object
 * is to dump whatever info is available for a given file name to user.
 */
class RSPF_DLL rspfInfoBase : public rspfReferenced
{
public:
   
   /** default constructor */
   rspfInfoBase();


   /**
    * @brief open method.
    *
    * @param file File name to open.
    *
    * @return true on success false on error.
    */
   virtual bool open(const rspfFilename& file) = 0;

   /**
    * Print method.
    *
    * @param out Stream to print to.
    * 
    * @return std::ostream&
    */
   virtual std::ostream& print(std::ostream& out) const = 0;

   /**
    * @brief Sets the overview flag.
    * @param flag If true overview info is processed also.
    */
   void setProcessOverviewFlag(bool flag);
   
   /**
    * @brief Method to get the overview flag.
    * @return The overview flag.
    */
   bool getProcessOverviewFlag() const;

   /**
    * @brief Method to dump info to a keyword list.
    * @param kwl The keyword list to initialize.
    * @return true on success, false on error.
    */
   virtual bool getKeywordlist(rspfKeywordlist& kwl)const;

protected:
   /** virtual destructor */
   virtual ~rspfInfoBase();
   
   bool theOverviewFlag; // If true overview information should be processed.
   
};

#endif /* End of "#ifndef rspfInfoBase_HEADER" */
