//*******************************************************************
// Copyright (C) 2001 ImageLinks Inc.
//
// License:  See top level LICENSE.txt file.
// 
// Author:  David Burken
//
// Description:
//
// Contains class declartion for rspfScalarRemapper.
// This class is used to remap image data from one scalar type to another.
//
//*******************************************************************
//  $Id: rspfScalarRemapper.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfScalarRemapper_HEADER
#define rspfScalarRemapper_HEADER

#include <rspf/imaging/rspfImageSourceFilter.h>

class RSPFDLLEXPORT rspfScalarRemapper : public rspfImageSourceFilter
{
public:
   rspfScalarRemapper();
   rspfScalarRemapper(rspfImageSource* inputSource,
                       rspfScalarType outputScalarType);

   virtual void initialize();

   virtual rspfString getLongName()  const;
   virtual rspfString getShortName() const;
   
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tile_rect,
                                               rspf_uint32 resLevel=0);

   /**
    *  Returns the output pixel type of the tile source.  This override the
    *  base class since it simply returns it's input scalar type which is
    *  not correct with this object.
    */
   virtual rspfScalarType getOutputScalarType() const;

   /**
    *  Returns the output pixel type of the tile source as a string.
    */
   virtual rspfString getOutputScalarTypeString() const;
   
   /**
    *  Sets the output scalar type.
    */
   virtual void setOutputScalarType(rspfScalarType scalarType);
   virtual void setOutputScalarType(rspfString scalarType);
     
   /**
    *  Sets the current resolution level.  Returns true on success, false
    *  on error.
    */
   virtual double getNullPixelValue(rspf_uint32 band)  const;
   virtual double getMinPixelValue(rspf_uint32 band=0) const;
   virtual double getMaxPixelValue(rspf_uint32 band=0) const;

   /**
    *  Method to save the state of an object to a keyword list.
    *  Return true if ok or false on error.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=NULL)const;

   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=NULL);

   /**
    *   Override base class so that a disableSource event does not
    *   reinitialize the object and enable itself.
    */
   virtual void propertyEvent(rspfPropertyEvent& event);
   virtual void refreshEvent(rspfRefreshEvent& event);   

   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   
protected:
   virtual ~rspfScalarRemapper();

   /**
    *  Called on first getTile, will initialize all data needed.
    */
   void allocate();

   /**
    *  Deletes allocated memory.  Used by both allocate and destructor.
    */
   void destroy();
   
   double*                     theNormBuf;
   rspfRefPtr<rspfImageData> theTile;
   rspfScalarType             theOutputScalarType;

   bool                        theByPassFlag;
TYPE_DATA
};

#endif /* #ifndef rspfScalarRemapper_HEADER */
