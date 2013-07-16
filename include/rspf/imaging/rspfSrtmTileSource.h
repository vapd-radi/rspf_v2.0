//----------------------------------------------------------------------------
// Copyright (c) 2005, David Burken, all rights reserved.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
// 
// Image handler class for a Shuttle Radar Topography Mission (SRTM) file.
//
//----------------------------------------------------------------------------
// $Id: rspfSrtmTileSource.h 17932 2010-08-19 20:34:35Z dburken $
#ifndef rspfSrtmTileSource_HEADER
#define rspfSrtmTileSource_HEADER

#include <rspf/imaging/rspfGeneralRasterTileSource.h>
#include <rspf/support_data/rspfSrtmSupportData.h>


class RSPFDLLEXPORT rspfSrtmTileSource : public rspfGeneralRasterTileSource
{
public:
   rspfSrtmTileSource();

   virtual rspfString getShortName() const;
   virtual rspfString getLongName()  const;

   virtual bool open();

   //! Returns the image geometry object associated with this tile source or NULL if not defined.
   //! The geometry contains full-to-local image transform as well as projection (image-to-world)
   virtual rspfRefPtr<rspfImageGeometry> getImageGeometry();

   /**
    * Method to save the state of an object to a keyword list.
    * @return true on success, false on error.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;

   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.
    *
    * @return true on success, false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix = NULL);

   /**
    * @brief Gets a property.
    * @param The name of the property to get.
    * @return The property if found.
    */
   virtual rspfRefPtr<rspfProperty> getProperty(
      const rspfString& name)const;

   /**
    * @brief Adds property names to array.
    * @param propertyNames Array to populate with property names.
    */
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;

protected:
   virtual ~rspfSrtmTileSource();
   rspfSrtmSupportData m_SrtmSupportData;
   
TYPE_DATA
};

#endif
