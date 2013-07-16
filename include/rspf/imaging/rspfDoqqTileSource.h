//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfDoqqTileSource.h 17932 2010-08-19 20:34:35Z dburken $
#ifndef rspfDoqqTileSource_HEADER
#define rspfDoqqTileSource_HEADER
#include <rspf/imaging/rspfGeneralRasterTileSource.h>
#include <rspf/support_data/rspfDoqq.h>

class RSPF_DLL rspfDoqqTileSource: public rspfGeneralRasterTileSource
{
public:
   rspfDoqqTileSource();
   
   rspfString getShortName()const;
   rspfString getLongName()const;
   
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix = NULL);
   
   //! Returns the image geometry object associated with this tile source or NULL if non defined.
   //! The geometry contains full-to-local image transform as well as projection (image-to-world)
   virtual rspfRefPtr<rspfImageGeometry> getImageGeometry();

   bool open(const rspfFilename& filename);
   bool open(const rspfGeneralRasterInfo& info);

   rspfString acqdate() const;
	
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
	
protected:
   virtual ~rspfDoqqTileSource();
  rspfRefPtr<rspfDoqq>   theHeaderInformation;
  rspfString  theAcqDate;
   
TYPE_DATA
};

#endif
