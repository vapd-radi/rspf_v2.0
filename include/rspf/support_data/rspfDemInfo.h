//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: USGS DEM Info object.
// 
//----------------------------------------------------------------------------
// $Id$
#ifndef rspfDemInfo_HEADER
#define rspfDemInfo_HEADER

#include <iosfwd>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/support_data/rspfInfoBase.h>

/**
 * @brief USGS DEM info class.
 *
 * Encapsulates the dumpnitf functionality.
 */
class RSPF_DLL rspfDemInfo : public rspfInfoBase
{
public:
   
   /** default constructor */
   rspfDemInfo();

   /** virtual destructor */
   virtual ~rspfDemInfo();

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

#endif /* End of "#ifndef rspfDemInfo_HEADER" */
