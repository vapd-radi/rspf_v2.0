//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description: NITF Info object.
// 
//----------------------------------------------------------------------------
// $Id$
#ifndef rspfNitfInfo_HEADER
#define rspfNitfInfo_HEADER

#include <iosfwd>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/support_data/rspfInfoBase.h>
#include <rspf/support_data/rspfNitfFile.h>

/**
 * @brief NITF info class.
 *
 * Encapsulates the dumpnitf functionality.
 */
class RSPF_DLL rspfNitfInfo : public rspfInfoBase
{
public:
   
   /** default constructor */
   rspfNitfInfo();

   /** virtual destructor */
   virtual ~rspfNitfInfo();

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

   virtual bool getKeywordlist(rspfKeywordlist& kwl)const;
   
private:
   rspfRefPtr<rspfNitfFile> m_nitfFile;
};

#endif /* End of "#ifndef rspfNitfInfo_HEADER" */
