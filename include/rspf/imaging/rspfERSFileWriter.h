//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  Andrew Huang
//
// Description:
// 
// Class declaration for rspfERSFileWriter.  Meta data class for
// writing an ERS header file.
//
//----------------------------------------------------------------------------
// $Id: rspfERSFileWriter.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfERSFileWriter_H
#define rspfERSFileWriter_H

#include <fstream>
#include <rspf/imaging/rspfMetadataFileWriter.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/support_data/rspfERS.h>


/**
 * Class for writing a "ERS" style header.
 */
class RSPF_DLL rspfERSFileWriter : public rspfMetadataFileWriter
{
public:

   /** default constructor */
   rspfERSFileWriter();


   /**
    * Initializes the state of the writer.  Users should use this to set
    * the ERS header fields that are not taken from theInputConnection prior
    * to calling execute.
    *
    * @return true on success, false on failure.
    */
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix = 0);

   /**
    * Satisfies pure virtual from rspfMetadataWriter base.
    *
    * Appends the writers image types to the "metadatatypeList".
    * 
    * @param metadatatypeList stl::vector<rspfString> list to append to.
    *
    * @note Appends to the list, does not clear it first.
    */
   virtual void getMetadatatypeList(
      std::vector<rspfString>& metadatatypeList) const;

   /**
    * Satisfies pure virtual from rspfMetadataWriter base.
    *
    * @param imageType string representing image type.
    *
    * @return true if "metadataType" is supported by writer.
    */
   virtual bool hasMetadataType(const rspfString& metadataType)const;   
   
protected:
   /** virtual destructor */
   virtual ~rspfERSFileWriter();
   
   virtual bool writeFile();

   rspfERS theHdr;
   
TYPE_DATA
};

#endif /* End of #ifndef rspfERSFileWriter_H */
