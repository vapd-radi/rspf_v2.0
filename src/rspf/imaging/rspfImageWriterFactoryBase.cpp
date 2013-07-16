//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts (gpotts@imagelinks.com)
//
//*******************************************************************
//  $Id: rspfImageWriterFactoryBase.cpp 18011 2010-08-31 12:48:56Z dburken $
#include <rspf/imaging/rspfImageWriterFactoryBase.h>

RTTI_DEF1(rspfImageWriterFactoryBase, "rspfImageWriterFactoryBase", rspfObjectFactory);

void rspfImageWriterFactoryBase::getImageFileWritersBySuffix(
   rspfImageWriterFactoryBase::ImageFileWriterList& /*result*/,
   const rspfString& /*ext*/)const
{
}

void rspfImageWriterFactoryBase::getImageFileWritersByMimeType(
   rspfImageWriterFactoryBase::ImageFileWriterList& /*result*/,
   const rspfString& /*mimeType*/)const
{
}
