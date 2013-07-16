//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: DTED Info object.
// 
//----------------------------------------------------------------------------
// $Id$
#ifndef rspfDtedInfo_HEADER
#define rspfDtedInfo_HEADER

#include <iosfwd>
#include <vector>
#include <rspf/base/rspfConstants.h>
#include <rspf/support_data/rspfInfoBase.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfRefPtr.h>

class rspfNitfFile;
class rspfProperty;

/**
 * @brief DTED Info class.
 *
 * Encapsulates the dtedInfo functionality.
 */
class RSPF_DLL rspfDtedInfo : public rspfInfoBase
{
public:
   
   /** default constructor */
   rspfDtedInfo();

   /** virtual destructor */
   virtual ~rspfDtedInfo();

   /**
    * @brief open method.
    *
    * @param file File name to open.
    *
    * @return true on success false on error.
    */
   virtual bool open(const rspfFilename& file);
   
   /**
    * Print method.
    *
    * @param out Stream to print to.
    * 
    * @return std::ostream&
    */
   virtual std::ostream& print(std::ostream& out) const;

   /**
    * @brief Gets a property for name.
    * @param name Property name to get.
    * @return rspfRefPtr<rspfProperty> Note that this can be empty if
    * property for name was not found.
    */
   rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   
   /**
    * @brief Adds this class's properties to list.
    * @param propertyNames list to append to.
    */
   void getPropertyNames(std::vector<rspfString>& propertyNames)const;

private:
   rspfFilename theFile;
};

#endif /* End of "#ifndef rspfDtedInfo_HEADER" */
