//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: Class declaration for image generator.
//
//*************************************************************************
// $Id: rspfIgen.h 20303 2011-11-29 16:11:46Z oscarkramer $
#ifndef rspfIgen_HEADER
#define rspfIgen_HEADER

#include <iostream>
#include <list>
#include <fstream>

#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfConnectableContainer.h>
#include <rspf/imaging/rspfTiling.h>

class rspfImageChain;
class rspfImageFileWriter;
class rspfMapProjection;
class rspfImageViewTransform;
class rspfImageSource;
class rspfImageMpiMWriterSequenceConnection;
class rspfImageMpiSWriterSequenceConnection;
class rspfObject;
class rspfArgumentParser;

class RSPF_DLL rspfIgen : public rspfReferenced
{
public:
   rspfIgen();
   virtual ~rspfIgen();

   virtual void initialize(const rspfKeywordlist& kwl);
   virtual void outputProduct();
   
protected:
   void initializeAttributes();
   void slaveSetup();
   bool loadProductSpec();
   void setView();
   void initThumbnailProjection();
   void initializeChain();
   bool writeToFile(rspfImageFileWriter* writer);

   rspfRefPtr<rspfConnectableContainer> theContainer;
   rspfRefPtr<rspfMapProjection>  theProductProjection;
   rspfRefPtr<rspfImageChain>  theProductChain;
   rspfRefPtr<rspfTiling>      theTiling;
   rspfDrect        theOutputRect;
   bool              theBuildThumbnailFlag;
   rspfIpt          theThumbnailSize;
   long              theNumberOfTilesToBuffer;
   rspfKeywordlist  theKwl;
   bool              theTilingEnabled;
   bool              theProgressFlag;
   bool              theStdoutFlag;
   rspf_uint32      theThreadCount;

};

#endif
