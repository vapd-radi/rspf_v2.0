//*******************************************************************
// Copyright (C) 2003 Storage Area Networks, Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Kenneth Melero (kmelero@sanz.com)
//
//*******************************************************************
//  $Id: rspfReadmeFileWriter.h 15766 2009-10-20 12:37:09Z gpotts $

#ifndef rspfReadmeFileWriter_H
#define rspfReadmeFileWriter_H

#include <fstream>
#include <rspf/imaging/rspfMetadataFileWriter.h>

/** rspfReadmeFileWriter */
class RSPFDLLEXPORT rspfReadmeFileWriter : public rspfMetadataFileWriter
{
public:

   /** default constructor */
   rspfReadmeFileWriter();


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
   virtual ~rspfReadmeFileWriter();
   virtual bool writeFile();
   
TYPE_DATA
};

#endif /* End of #ifndef rspfReadmeFileWriter_H */
