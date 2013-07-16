//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
// 
// Image handler class for a raster files with an ENVI header file.
//
//----------------------------------------------------------------------------
// $Id$

#ifndef rspfEnviTileSource_HEADER
#define rspfEnviTileSource_HEADER 1

#include <rspf/imaging/rspfGeneralRasterTileSource.h>
#include <rspf/support_data/rspfEnviHeader.h>


class RSPFDLLEXPORT rspfEnviTileSource : public rspfGeneralRasterTileSource
{
public:

   /** @brief default constructor */
   rspfEnviTileSource();

   /** @return "ENVI" */
   virtual rspfString getShortName() const;

   /** @return "ENVI reader" */
   virtual rspfString getLongName()  const;

   /** @brief open method. */
   virtual bool open();

   /**
    * @return The image geometry object associated with this tile source or
    * NULL if not defined.  The geometry contains full-to-local image transform
    * as well as projection (image-to-world).
    */
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
   
   /** @brief Hidden from use destructor.  This is a ref ptr. */
   virtual ~rspfEnviTileSource();

private:

   /**
    * @brief Sets default output band list if we are a band selector and
    * "default bands" is found in the header file.
    */
   void setDefaultBandList();

   /**
    * @brief Creates geometry from known support data, i.e. envi header,
    * and so on if found.
    */
   rspfRefPtr<rspfImageGeometry> getInternalImageGeometry() const;
   
   rspfEnviHeader m_enviHdr;
   
TYPE_DATA
   
}; // End: class rspfEnviTileSource

#endif /* #ifndef rspfEnviTileSource_HEADER */
