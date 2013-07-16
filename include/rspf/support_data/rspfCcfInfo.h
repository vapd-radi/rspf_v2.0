//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: CCF Info object.
// 
//----------------------------------------------------------------------------
// $Id$
#ifndef rspfCcfInfo_HEADER
#define rspfCcfInfo_HEADER

#include <iosfwd>
#include <rspf/base/rspfConstants.h>
#include <rspf/support_data/rspfInfoBase.h>
#include <rspf/base/rspfFilename.h>

class rspfNitfFile;

/**
 * @brief NITF info class.
 *
 * Encapsulates the dumpnitf functionality.
 */
class RSPF_DLL rspfCcfInfo : public rspfInfoBase
{
public:
   
   /** default constructor */
   rspfCcfInfo();

   /** virtual destructor */
   virtual ~rspfCcfInfo();

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

private:
   rspfFilename theFile;
};

#endif /* End of "#ifndef rspfCcfInfo_HEADER" */
