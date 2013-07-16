//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: JP2 Info object.
// 
//----------------------------------------------------------------------------
// $Id: rspfJp2Info.h 20125 2011-10-11 19:47:19Z dburken $
#ifndef rspfJp2Info_HEADER
#define rspfJp2Info_HEADER 1

#include <rspf/base/rspfConstants.h>
#include <rspf/support_data/rspfInfoBase.h>
#include <rspf/base/rspfFilename.h>

/**
 * @brief JP2 info class.
 */
class RSPF_DLL rspfJp2Info : public rspfInfoBase
{
public:

   /** Anonymous enumerations: */
   enum
   {
      SIGNATURE_BOX_SIZE = 12,
      GEOTIFF_UUID_SIZE  = 16
   };
   
   /** default constructor */
   rspfJp2Info();

   /** virtual destructor */
   virtual ~rspfJp2Info();

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

   rspfFilename m_file;
};

#endif /* End of "#ifndef rspfJp2Info_HEADER" */
