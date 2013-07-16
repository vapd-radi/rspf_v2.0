//*******************************************************************
// Copyright (C) 2005 Garrett Potts
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//
//*******************************************************************
//  $Id: rspfStreamFactoryBase.h 11176 2007-06-07 19:45:56Z dburken $
//
#ifndef rspfStreamFactoryBase_HEADER
#define rspfStreamFactoryBase_HEADER

#include <iosfwd>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfIoStream.h>

class rspfFilename;
class rspfIStream;

class RSPF_DLL rspfStreamFactoryBase
{
public:
   virtual ~rspfStreamFactoryBase(){}
   
   virtual rspfRefPtr<rspfIFStream> createNewIFStream(
      const rspfFilename& file,
      std::ios_base::openmode openMode)const=0;
};

#endif
