//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file
//
// Author:  Mingjie Su
//
// Description: Interface class for ESRI Shape file.
//
//----------------------------------------------------------------------------
// $Id: rspfEsriShapeFileInterface.h 1916 2010-12-16 16:08:33Z ming.su $

#ifndef rspfEsriShapeFileInterface_HEADER
#define rspfEsriShapeFileInterface_HEADER

//Std Includes
#include <map>

//Ossim Includes
#include <rspf/base/rspfConstants.h> /* for RSPF_DLL macro */
#include <rspf/base/rspfRtti.h>

class rspfAnnotationObject;
class rspfString;

class RSPFDLLEXPORT rspfEsriShapeFileInterface
{
public:
   
   /**
    * Pure virtual getFeatureTable method.
    *
    */
   virtual std::multimap<long, rspfAnnotationObject*> getFeatureTable() = 0;

   /**
    * Pure virtual setQuery method.
    *
    */
   virtual void setQuery(const rspfString& query) = 0;

   /**
    * Pure virtual setGeometryBuffer method.
    *
    */
   virtual void setGeometryBuffer(rspf_float64 distance, rspfUnitType type) = 0;
  
   TYPE_DATA
};

#endif /* #ifndef rspfEsriShapeFileInterface_HEADER */
