//----------------------------------------------------------------------------
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//----------------------------------------------------------------------------
// $Id: rspfImageMetaDataWriterFactoryBase.cpp 9094 2006-06-13 19:12:40Z dburken $

#include <rspf/imaging/rspfImageMetaDataWriterFactoryBase.h>

RTTI_DEF1(rspfImageMetaDataWriterFactoryBase,
          "rspfImageMetaDataWriterFactoryBase",
          rspfObjectFactory);

rspfImageMetaDataWriterFactoryBase::rspfImageMetaDataWriterFactoryBase()
{
}

rspfImageMetaDataWriterFactoryBase::rspfImageMetaDataWriterFactoryBase(
   const rspfImageMetaDataWriterFactoryBase&)
{
}

const rspfImageMetaDataWriterFactoryBase&
rspfImageMetaDataWriterFactoryBase::operator=(
   const rspfImageMetaDataWriterFactoryBase&)
{
   return *this;
}
