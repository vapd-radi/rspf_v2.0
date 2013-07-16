//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts (gpotts@imagelinks.com)
//
//*******************************************************************
//  $Id: rspfImageHandlerFactoryBase.cpp 22228 2013-04-12 14:11:45Z dburken $
#include <rspf/imaging/rspfImageHandlerFactoryBase.h>

RTTI_DEF1(rspfImageHandlerFactoryBase, "rspfImageHandlerFactoryBase", rspfObjectFactory);

void rspfImageHandlerFactoryBase::getImageHandlersBySuffix(ImageHandlerList& /*result*/,
                                                            const rspfString& /*ext*/)const
{
}

void rspfImageHandlerFactoryBase::getImageHandlersByMimeType(ImageHandlerList& /*result*/,
                                                              const rspfString& /*mimeType*/)const
{
}

rspfRefPtr<rspfImageHandler> rspfImageHandlerFactoryBase::openOverview(
   const rspfFilename& file ) const
{
   return rspfRefPtr<rspfImageHandler>(0);
}
