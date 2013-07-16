//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// Contributor: David A. Horner (DAH) - http://dave.thehorners.com
//
//*************************************************************************
// $Id: rspfRectilinearDataObject.cpp 20045 2011-09-06 15:03:11Z oscarkramer $

#include <rspf/base/rspfRectilinearDataObject.h>
#include <rspf/base/rspfScalarTypeLut.h>
#include <rspf/base/rspfKeywordlist.h>

RTTI_DEF1(rspfRectilinearDataObject, "rspfRectilinearDataObject", rspfDataObject);

rspfRectilinearDataObject::rspfRectilinearDataObject()
   : rspfDataObject(),
     m_numberOfDataComponents(0),
     m_scalarType(),
     m_dataBuffer(),
     m_spatialExtents()
{
}

rspfRectilinearDataObject::rspfRectilinearDataObject(
   const rspfRectilinearDataObject& rhs)
   : rspfDataObject(rhs),
     m_numberOfDataComponents(rhs.m_numberOfDataComponents),
     m_scalarType(rhs.m_scalarType),
     m_dataBuffer(rhs.m_dataBuffer),
     m_spatialExtents(rhs.m_spatialExtents)
{
}

rspfRectilinearDataObject::rspfRectilinearDataObject(
   rspf_uint32 numberOfSpatialComponents,
   rspfSource* owner,
   rspf_uint32 numberOfDataComponents,
   rspfScalarType   scalarType,
   rspfDataObjectStatus /* status */)
   :rspfDataObject(owner, RSPF_NULL),
    m_numberOfDataComponents(numberOfDataComponents),
    m_scalarType(scalarType),
    m_dataBuffer(0),
    m_spatialExtents(numberOfSpatialComponents)
{
}

rspfRectilinearDataObject::rspfRectilinearDataObject(
   rspfSource* owner,
   rspf_uint32 numberOfDataComponents,
   rspf_uint32 length,
   rspfScalarType   scalarType,
   rspfDataObjectStatus /* status */ )
   :rspfDataObject(owner, RSPF_NULL),
    m_numberOfDataComponents(numberOfDataComponents),
    m_scalarType(scalarType),
    m_dataBuffer(0),
    m_spatialExtents(1)
{
   m_spatialExtents[0] = length;
}

rspfRectilinearDataObject::rspfRectilinearDataObject(
   rspfSource* owner,
   rspf_uint32 numberOfDataComponents,
   rspf_uint32 width,
   rspf_uint32 height,
   rspfScalarType   scalarType,
   rspfDataObjectStatus /* status */)
   :rspfDataObject(owner, RSPF_NULL),
    m_numberOfDataComponents(numberOfDataComponents),
    m_scalarType(scalarType),
    m_dataBuffer(0),
    m_spatialExtents(2)
{
   m_spatialExtents[0] = width;
   m_spatialExtents[1] = height;
}

rspfRectilinearDataObject::rspfRectilinearDataObject(
   rspfSource* owner,
   rspf_uint32 numberOfDataComponents,
   rspf_uint32 width,
   rspf_uint32 height,
   rspf_uint32 depth,
   rspfScalarType   scalarType,
   rspfDataObjectStatus /* status */)
   :rspfDataObject(owner, RSPF_NULL),
    m_numberOfDataComponents(numberOfDataComponents),
    m_scalarType(scalarType),
    m_dataBuffer(0),
    m_spatialExtents(3)
{
   m_spatialExtents[0] = width;
   m_spatialExtents[1] = height;
   m_spatialExtents[2] = depth;
}

rspfRectilinearDataObject::~rspfRectilinearDataObject()
{
}

rspf_uint32 rspfRectilinearDataObject::computeSpatialProduct()const
{
   rspf_uint32 spatialProduct = 0;
   for(rspf_uint32 index = 0; index < m_spatialExtents.size(); ++index)
   {
      spatialProduct *= m_spatialExtents[index];
   }
   return spatialProduct;
}

void rspfRectilinearDataObject::setNumberOfDataComponents(rspf_uint32 n)
{
   m_numberOfDataComponents = n;
}

void rspfRectilinearDataObject::setSpatialExtents(rspf_uint32* extents,
                                                   rspf_uint32 size)
{
   if (extents)
   {
      m_spatialExtents.resize(size);
      for(rspf_uint32 i =0; i < size; ++i)
      {
         m_spatialExtents[i] = extents[i];
      }
   }
}

void rspfRectilinearDataObject::setScalarType(rspfScalarType type)
{
   m_scalarType = type;
}

rspf_uint32 rspfRectilinearDataObject::getNumberOfDataComponents() const
{
   return m_numberOfDataComponents;
}

rspf_uint32 rspfRectilinearDataObject::getNumberOfSpatialComponents() const
{
   return (rspf_uint32)m_spatialExtents.size();
}

const rspf_uint32* rspfRectilinearDataObject::getSpatialExtents()const
{
   return &(m_spatialExtents.front());
}

rspfScalarType rspfRectilinearDataObject::getScalarType() const
{
   return m_scalarType;
}

rspf_uint32 rspfRectilinearDataObject::getScalarSizeInBytes() const
{
   return rspf::scalarSizeInBytes(getScalarType());
}

void* rspfRectilinearDataObject::getBuf()
{
   if (m_dataBuffer.size() > 0)
   {
      return static_cast<void*>(&m_dataBuffer.front());
   }
   return NULL;
}

const void* rspfRectilinearDataObject::getBuf()const
{
   if (m_dataBuffer.size() > 0)
   {
      return static_cast<const void*>(&m_dataBuffer.front());
   }
   return NULL;
}

void rspfRectilinearDataObject::assign(const rspfRectilinearDataObject* data)
{
   if(data)
   {
      if (this != data)
      {
         rspfDataObject::assign(data);
         
         m_numberOfDataComponents    = data->m_numberOfDataComponents;
         m_scalarType                = data->m_scalarType;
         m_dataBuffer                = data->m_dataBuffer;
         m_spatialExtents            = data->m_spatialExtents;
      }
   }
}

void rspfRectilinearDataObject::initialize()
{
   if (m_dataBuffer.size() != getDataSizeInBytes())
   {
      m_dataBuffer.resize(getDataSizeInBytes());
      setDataObjectStatus(RSPF_STATUS_UNKNOWN);
   }
}

rspf_uint32 rspfRectilinearDataObject::getDataSizeInBytes()const
{
   return (rspf_uint32)(getScalarSizeInBytes()*
                         computeSpatialProduct()*
                         m_numberOfDataComponents);
}

std::ostream& rspfRectilinearDataObject::print(std::ostream& out) const
{
   out << "rspfRectilinearDataObject::print:"
       << "\nm_numberOfDataComponents:     " << m_numberOfDataComponents
       << "\ntheNumberOfSpatialComponents:  " << m_spatialExtents.size()
       << "\nm_scalarType:                 "
       << (rspfScalarTypeLut::instance()->getEntryString(m_scalarType))
       << endl;
   
   return rspfDataObject::print(out);
}

const rspfRectilinearDataObject& rspfRectilinearDataObject::operator=(
   const rspfRectilinearDataObject& rhs)
{
   if (this != &rhs)
   {
      // rspfDataObject initialization:
      rspfDataObject::operator=(rhs);

      // rspfRectilinearDataObject (this) initialization:
      m_numberOfDataComponents    = rhs.m_numberOfDataComponents;
      m_scalarType                = rhs.m_scalarType;
      m_dataBuffer                = rhs.m_dataBuffer;
      m_spatialExtents            = rhs.m_spatialExtents;
   }
   return *this;
}

bool rspfRectilinearDataObject::saveState(rspfKeywordlist& kwl, const char* prefix)const
{
   rspfString byteEncoded;
   rspf::toSimpleStringList(byteEncoded, m_dataBuffer);
   kwl.add(prefix, "data_buffer", byteEncoded, true);
   rspf::toSimpleStringList(byteEncoded, m_spatialExtents);
   kwl.add(prefix, "spatial_extents", byteEncoded, true);
   kwl.add(prefix, rspfKeywordNames::SCALAR_TYPE_KW, rspfScalarTypeLut::instance()->getEntryString(m_scalarType));
   
   return rspfDataObject::saveState(kwl, prefix);
}

bool rspfRectilinearDataObject::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   if(!rspfDataObject::loadState(kwl, prefix)) return false;
   
   const char* spatial_extents = kwl.find(prefix, "spatial_extents");
   const char* data_buffer = kwl.find(prefix, "data_buffer");
   const char* scalar_type = kwl.find(prefix, rspfKeywordNames::SCALAR_TYPE_KW);
   m_spatialExtents.clear();
   m_dataBuffer.clear();
                                 
   if(spatial_extents)
   {
      if(!rspf::toSimpleVector(m_spatialExtents, rspfString(spatial_extents)))
      {
         return false;
      }
   }
   if(data_buffer)
   {
      if(!rspf::toSimpleVector(m_dataBuffer, rspfString(kwl.find(prefix, "data_buffer"))))
      {
         return false;
      }
   }
   if(scalar_type)
   {
      rspfScalarTypeLut::instance()->getScalarTypeFromString(scalar_type);
   }
   else 
   {
      m_scalarType = RSPF_SCALAR_UNKNOWN;
   }

   m_numberOfDataComponents = (rspf_uint32) m_spatialExtents.size();
   
   return true;
   
}                     
