//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Mingjie Su
//
// Description: Rpf Info object.
// 
//----------------------------------------------------------------------------
// $Id: rspfRpfInfo.h 1236 2010-08-05 19:45:44Z ming.su $
#ifndef rspfRpfInfo_HEADER
#define rspfRpfInfo_HEADER

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/support_data/rspfInfoBase.h>
#include <rspf/base/rspfFilename.h>

class rspfKeywordlist;
/**
 * @brief HDF info class.
 *
 * Encapsulates the HDF functionality.
 */
class rspfRpfInfo : public rspfInfoBase
{
public:

   /** default constructor */
   rspfRpfInfo();

   /** virtual destructor */
   virtual ~rspfRpfInfo();

   /**
    * @brief open method.
    *
    * @param file File name to open.
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

   bool isOpen();

   private: 
   
     rspfFilename                       theFile;
     rspfFilename                       m_infoFile;
};

#endif /* End of "#ifndef rspfRpfInfo_HEADER" */
