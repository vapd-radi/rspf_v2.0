//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
// Description:
//
// Contains class definition for the class "rspfLandsatTileSource".
// rspfLandsatTileSource is derived from ImageHandler which is derived from
// TileSource.
//
//*******************************************************************
//  $Id: rspfLandsatTileSource.h 17932 2010-08-19 20:34:35Z dburken $

#ifndef rspfLandsatTileSource_HEADER
#define rspfLandsatTileSource_HEADER

#include <rspf/imaging/rspfGeneralRasterTileSource.h>
#include <rspf/base/rspfDate.h>
#include <rspf/support_data/rspfFfL7.h>

class RSPF_DLL rspfLandsatTileSource : public rspfGeneralRasterTileSource
{
public:
   rspfLandsatTileSource();

   rspfLandsatTileSource(const rspfKeywordlist& kwl,
                           const char* prefix=0);
   

   virtual rspfString getShortName() const;
   
   virtual rspfString getLongName() const;

   virtual bool open();
   
   virtual double getNullPixelValue(rspf_uint32 band=0)const;

   virtual double getMinPixelValue(rspf_uint32 band=0)const;

   virtual double getMaxPixelValue(rspf_uint32 band=0)const;

   virtual rspfScalarType getOutputScalarType() const;

   bool getAcquisitionDate(rspfDate& date)const;
   rspfString getSatelliteName()const;

   rspfFilename getBandFilename(rspf_uint32 idx)const;
   
   bool isPan()const;
   bool isVir()const;
   bool isTm()const;
   
   //! Returns the image geometry object associated with this tile source or NULL if non defined.
   //! The geometry contains full-to-local image transform as well as projection (image-to-world)
   virtual rspfRefPtr<rspfImageGeometry> getImageGeometry();

   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix = NULL);

   /**
    * @brief Gets a property for matching name.
    * @param name The name of the property to get.
    * @return Returns property matching "name".
    */
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   
   /**
    * @brief Gets a list of property names available.
    * @param propertyNames The list to push back names to.
    */
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   
private:
   virtual ~rspfLandsatTileSource();

   void openHeader(const rspfFilename& file);
   
   rspfRefPtr<rspfFfL7> theFfHdr;

   TYPE_DATA
};

#endif
