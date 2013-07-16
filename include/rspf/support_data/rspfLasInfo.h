//----------------------------------------------------------------------------
//
// File: rspfLibLasInfo.h
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: LAS LIDAR info object.
// 
//----------------------------------------------------------------------------
// $Id$
#ifndef rspfLasInfo_HEADER
#define rspfLasInfo_HEADER 1

#include <rspf/base/rspfConstants.h>
#include <rspf/support_data/rspfInfoBase.h>
#include <rspf/base/rspfFilename.h>

#include <iosfwd>

/** @brief LAS LIDAR info object. */
class RSPF_DLL rspfLasInfo : public rspfInfoBase
{
public:
   
   /** default constructor */
   rspfLasInfo();

   /** virtual destructor */
   virtual ~rspfLasInfo();

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

#endif /* End of "#ifndef rspfLasInfo_HEADER" */
