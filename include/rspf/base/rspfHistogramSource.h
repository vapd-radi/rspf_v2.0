//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfHistogramSource.h 11721 2007-09-13 13:19:34Z gpotts $
#ifndef rspfHistogramSource_HEADER
#define rspfHistogramSource_HEADER

#include <rspf/base/rspfSource.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfMultiResLevelHistogram.h>

class RSPF_DLL rspfHistogramSource : public rspfSource
{
public:
   
   virtual ~rspfHistogramSource();
   
   virtual rspfRefPtr<rspfMultiResLevelHistogram> getHistogram();
   
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
protected:
   rspfHistogramSource(rspfObject* owner=NULL,
                        rspf_uint32 numberOfInputs=0,
                        rspf_uint32 numberOfOutputs=0,
                        bool inputListFixedFlag=true,
                        bool outputListFixedFlag=true);
   
   rspfRefPtr<rspfMultiResLevelHistogram> theHistogram;
   
   /*!
    * if the filename is not "" then it will use
    * this to store the histogram outside the keywordlist.
    *
    * it will set the filename keyword to the filename and
    * during a save state will save it out to the file
    * instead of inline to the keyword list.
    */
   rspfFilename                theFilename;
   
private:
   /** Hidden from use copy constructor. */
   rspfHistogramSource(const rspfHistogramSource&);

TYPE_DATA
};

#endif /* End of "#ifndef rspfHistogramSource_HEADER" */
