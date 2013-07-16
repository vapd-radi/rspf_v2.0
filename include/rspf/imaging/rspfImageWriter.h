//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  See top level LICENSE.txt file.
//
// Author:  Frank Warmerdam (warmerdam@pobox.com)
//
// Description:
//
// Contains class declaration for rspfImageWriter
//*******************************************************************
//  $Id: rspfImageWriter.h 15798 2009-10-23 19:15:20Z gpotts $

#ifndef rspfImageWriter_HEADER
#define rspfImageWriter_HEADER

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfOutputSource.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfIrect.h>

class rspfImage;
class rspfImageSource;
class rspfImageWriter;
class rspfImageWriterFactory;
class rspfKeywordlist;
class rspfFilename;

#include <rspf/imaging/rspfImageSourceSequencer.h>


class RSPF_DLL rspfImageWriter :  public rspfOutputSource
{   
public:
   rspfImageWriter(rspfObject* owner=NULL);
   
   rspfImageWriter(rspfObject* owner,
                    rspf_uint32 numberOfInputs,
                    rspf_uint32 numberOfOutputs,
                    bool inputListIsFixed,
                    bool outputListIsFixed);
   
   virtual ~rspfImageWriter();

   /**
    * @param aRect the rectangle to write.
    * 
    * @return true on success, false if the rectangle is invalid.
    */
   virtual bool setViewingRect(const rspfIrect &aRect);

   /**
    * @return Returns theAreaOfInterest.
    */
   virtual rspfIrect getAreaOfInterest() const;
 
   virtual void setAreaOfInterest(const rspfIrect& inputAreaOfInterest);

   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;

   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
      
protected:

   rspfIrect theAreaOfInterest;
TYPE_DATA
};

#endif
