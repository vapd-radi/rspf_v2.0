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
//  $Id: rspfStreamFactory.h 11176 2007-06-07 19:45:56Z dburken $
//
#ifndef rspfStreamFactory_HEADER
#define rspfStreamFactory_HEADER
#include <rspf/base/rspfStreamFactoryBase.h>
#include <rspf/base/rspfIoStream.h>

class RSPF_DLL rspfStreamFactory : public rspfStreamFactoryBase
{
public:
   static rspfStreamFactory* instance();
   virtual ~rspfStreamFactory();
 
   virtual rspfRefPtr<rspfIFStream>
      createNewIFStream(const rspfFilename& file,
                        std::ios_base::openmode openMode) const;

   
protected:
   rspfStreamFactory();
   rspfStreamFactory(const rspfStreamFactory&);
   static rspfStreamFactory* theInstance;
   
};

#endif
