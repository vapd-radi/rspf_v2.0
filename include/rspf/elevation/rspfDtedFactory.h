//*****************************************************************************
// FILE: rspfDtedFactory.cc
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
// 
// Author:  David Burken
//
// Description:
//
// Class declaration for rspfDtedFactory.
// This is the base class interface for elevation cell factories.
//*****************************************************************************
// $Id: rspfDtedFactory.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfDtedFactory_HEADER
#define rspfDtedFactory_HEADER

#include <rspf/elevation/rspfElevSourceFactory.h>

/** CLASS:  rspfDtedFactory */
class RSPF_DLL rspfDtedFactory : public rspfElevSourceFactory
{
public:

   rspfDtedFactory();
   rspfDtedFactory(const rspfFilename& dir);
      

   virtual rspfElevSource* getNewElevSource(const rspfGpt& gpt) const;
   virtual void createIndex();
protected:
   virtual ~rspfDtedFactory();

TYPE_DATA
};

#endif /* End of "#ifndef rspfDtedFactory_HEADER" */
