//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfRectilinearDataObject.h 19931 2011-08-10 11:53:25Z gpotts $
#ifndef rspfRectilinearDataObject_HEADER
#define rspfRectilinearDataObject_HEADER
#include <rspf/base/rspfDataObject.h>

class RSPFDLLEXPORT rspfRectilinearDataObject : public rspfDataObject
{
public:

   /** default constructor */
   rspfRectilinearDataObject();

   rspfRectilinearDataObject(const rspfRectilinearDataObject&rhs);
      
   rspfRectilinearDataObject(rspf_uint32 numberOfSpatialComponents,
                              rspfSource* owner,
                              rspf_uint32 numberOfDataComponents,
                              rspfScalarType scalarType=RSPF_SCALAR_UNKNOWN,
                              rspfDataObjectStatus status=RSPF_NULL);
   
   /**
    * This is a helper constructor that allows one to instantiate a one
    * dimensional Spatial component with N number of data components.
    * It will internally allocate theSpatialExtent to 1 and set the
    * contents equal to the value passed in to length.
    */
   rspfRectilinearDataObject(rspfSource* owner,
                              rspf_uint32 numberOfDataComponents,
                              rspf_uint32 length,
                              rspfScalarType scalarType=RSPF_SCALAR_UNKNOWN,
                              rspfDataObjectStatus status=RSPF_NULL);

   /**
    * This is a helper constructor that allows one to instantiate a two
    * dimensional Spatial component (WidthxHeight) with N number of
    * data components.  It will internally allocate theSpatialExtent
    * to 2 and set the contents equal to the value passed in to width, and
    * height.
    */
   rspfRectilinearDataObject(rspfSource* owner,
                              rspf_uint32 numberOfDataComponents,
                              rspf_uint32 width,
                              rspf_uint32 height,
                              rspfScalarType scalarType=RSPF_SCALAR_UNKNOWN,
                              rspfDataObjectStatus status=RSPF_NULL);

   /**
    * This is a helper constructor that allows one to instantiate a two
    * dimensional Spatial component (WidthxHeightxDepth) with N number of
    * data components.  It will internally allocate theSpatialExtent
    * to 3 and set the contents equal to the value passed in to width,
    * height, and depth.
    */
   rspfRectilinearDataObject(rspfSource* owner,
                              rspf_uint32 numberOfDataComponents,
                              rspf_uint32 width,
                              rspf_uint32 height,
                              rspf_uint32 depth,
                              rspfScalarType   scalarType=RSPF_SCALAR_UNKNOWN,
                              rspfDataObjectStatus status=RSPF_NULL);
   

   virtual ~rspfRectilinearDataObject();
   /**
    * How many components make up this data object.  For
    * example:  If this were an RGB image data object then
    * the number of components would be set to 3 and the
    * RGB would be seen as a single data object being
    * passed through.
    */
   virtual void setNumberOfDataComponents(rspf_uint32 n);
   
   virtual void setSpatialExtents(rspf_uint32* extents, rspf_uint32  size);

   /**
    *  See rspfScalarType in rspfConstants for a full list
    *
    *  RSPF_SCALAR_UNKNOWN
    *  RSPF_UCHAR          Unsigned char
    *  RSPF_USHORT16       16bit unsigned short
    *  RSPF_SSHORT16       16bit signed short
    *  RSPF_USHORT11       11bit unsigned short
    *  RSPF_FLOAT          float
    *  RSPF_NORMALIZED_DOUBLE normalized 0 to 1 data
    */
   virtual void setScalarType(rspfScalarType type);

   virtual rspf_uint32 getNumberOfDataComponents() const;

   virtual rspf_uint32 getNumberOfSpatialComponents() const;

   virtual const rspf_uint32* getSpatialExtents() const;

   virtual rspfScalarType getScalarType() const;

   virtual rspf_uint32 getScalarSizeInBytes()const;

   virtual void*   getBuf();

   virtual const void* getBuf() const;

   virtual void assign(const rspfRectilinearDataObject* data);

   /**
    *  Calls deInitialize() which deletes theDataBuffer and sets object status
    *  to RSPF_NULL, then initializes theDataBuffer.
    *
    *  @see deInitialize
    *  
    *  @note This method is destructive in that if the buffer existed of
    *  the correct size it will delete it, then reallocate.
    */
   virtual void initialize();

   virtual rspf_uint32 computeSpatialProduct()const;

   virtual rspf_uint32 getDataSizeInBytes()const;

   virtual std::ostream& print(std::ostream& out) const;

   /**
    * @brief assignment operator=
    * @param rhs The data to assign from.
    * @param A reference to this object.
    */
   virtual const rspfRectilinearDataObject& operator=(
      const rspfRectilinearDataObject& rhs);

   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=0)const;
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);

protected:
   rspf_uint32              m_numberOfDataComponents;
   rspfScalarType           m_scalarType;
   std::vector<rspf_uint8>  m_dataBuffer;
   std::vector<rspf_uint32> m_spatialExtents;
   
TYPE_DATA
};

#endif
