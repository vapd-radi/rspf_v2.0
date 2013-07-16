//*************************************************************************
// Copyright (C) 2004 Intelligence Data Systems, Inc.  All rights reserved.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
//**************************************************************************
// $Id: rspfVpfAnnotationCoverageInfo.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfVpfAnnotationCoverageInfo_HEADER
#define rspfVpfAnnotationCoverageInfo_HEADER
#include <rspf/imaging/rspfVpfAnnotationFeatureInfo.h>
#include <rspf/vec/rspfVpfCoverage.h>
class rspfVpfLibrary;

class rspfRgbImage;

class rspfVpfAnnotationCoverageInfo
{
public:
  rspfVpfAnnotationCoverageInfo(const rspfString& name="")
    :theName(name)
  {
    
  }
  virtual ~rspfVpfAnnotationCoverageInfo()
  {
    deleteAllFeatures();
  }
  bool hasRenderableFeature()const;
  void getEnabledFeatures(std::vector<rspfVpfAnnotationFeatureInfo*>& result);
  void getAllFeatures(std::vector<rspfVpfAnnotationFeatureInfo*>& result);

  void transform(rspfImageGeometry* geom);
  rspfIrect getBoundingProjectedRect()const;
  void buildCoverage();
  void buildCoverage(const rspfString& feature);
  void setName(const rspfString& name)
  {
    theName = name;
  }
  rspfString getName()const
     {
        return theName;
     }
  void setLibrary(rspfVpfLibrary* library)
  {
    theLibrary = library;
  }
  rspfVpfLibrary* getLibrary()
  {
    return theLibrary;
  }
  const rspfVpfLibrary* getLibrary()const
  {
    return theLibrary;
  }
  void drawAnnotations(rspfRgbImage* tile);
   void deleteAllFeatures();

  virtual bool saveState(rspfKeywordlist& kwl,
			 const char* prefix=0)const;
  virtual bool loadState(const rspfKeywordlist& kwl,
			 const char* prefix=0);
protected:
  rspfString theName;

  /*!
   * Not owned.
   */
  rspfVpfLibrary* theLibrary;
  std::vector<rspfVpfAnnotationFeatureInfo*>           theFeatureInfoArray;
};

#endif
