//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: ENVI Info object.
// 
//----------------------------------------------------------------------------
// $Id$
#ifndef rspfEnviInfo_HEADER
#define rspfEnviInfo_HEADER 1

#include <rspf/base/rspfConstants.h>
#include <rspf/support_data/rspfInfoBase.h>
#include <rspf/base/rspfFilename.h>

/** @brief ENVI info class. */
class RSPF_DLL rspfEnviInfo : public rspfInfoBase
{
public:
   
   /** default constructor */
   rspfEnviInfo();

   /** virtual destructor */
   virtual ~rspfEnviInfo();

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

#endif /* End of "#ifndef rspfEnviInfo_HEADER" */
