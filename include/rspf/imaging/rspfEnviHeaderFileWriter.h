//----------------------------------------------------------------------------
// Copyright (c) 2005, David Burken, all rights reserved.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
// 
// Class declaration for rspfEnviHeaderFileWriter.  Meta data class for
// writing an ENVI (The Environment for Visualizing Images) header file.
//
//----------------------------------------------------------------------------
// $Id: rspfEnviHeaderFileWriter.h 15766 2009-10-20 12:37:09Z gpotts $

#ifndef rspfEnviHeaderFileWriter_H
#define rspfEnviHeaderFileWriter_H

#include <fstream>
#include <rspf/imaging/rspfMetadataFileWriter.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/support_data/rspfEnviHeader.h>

/**
 * Class for writing a "ENVI" style header.
 */
class RSPFDLLEXPORT rspfEnviHeaderFileWriter : public rspfMetadataFileWriter
{
public:

   /** default constructor */
   rspfEnviHeaderFileWriter();


   /**
    * Initializes the state of the writer.  Users should use this to set
    * the envi header fields that are not taken from theInputConnection prior
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
   virtual ~rspfEnviHeaderFileWriter();

private:
   
   virtual bool writeFile();

   rspfEnviHeader theHdr;
   
TYPE_DATA
};
#endif /* End of #ifndef rspfEnviHeaderFileWriter_H */
