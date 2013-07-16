//*******************************************************************
// Copyright (C) 2003 Storage Area Networks, Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Kenneth Melero (kmelero@sanz.com)
//
//*******************************************************************
//  $Id: rspfMetadataFileWriter.h 15766 2009-10-20 12:37:09Z gpotts $

#ifndef rspfMetadataFileWriter_H
#define rspfMetadataFileWriter_H

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfConnectableObject.h>
#include <rspf/base/rspfProcessInterface.h>
#include <rspf/base/rspfConnectableObjectListener.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfObjectEvents.h>
#include <rspf/base/rspfProcessProgressEvent.h>

class rspfImageSource;

/**
 * rspfMetadataFileWriter
 *
 * Typical usage something like this:
 *
 * rspfObject* obj = rspfImageMetaDataWriterRegistry::instance()->
 *     createObject(rspfString("rspfReadmeFileWriter"));
 * if (!obj)
 * {
 *    return;
 * }
 * rspfMetadataFileWriter* mw = PTR_CAST(rspfMetadataFileWriter, obj);
 * if (!mw)
 * {
 *    return;
 * }
 * 
 * mw->setFilename(xmlFile);
 * mw->loadState(kwl);
 * mw->connectMyInputTo(ih.get());
 * mw->execute();
 * delete mw;
 */
class RSPFDLLEXPORT rspfMetadataFileWriter :
   public rspfConnectableObject,
   public rspfProcessInterface,
   public rspfConnectableObjectListener
{   
public:
   rspfMetadataFileWriter();
   

   virtual rspfObject*       getObject();
   virtual const rspfObject* getObject() const;

   virtual void initialize();
   
   virtual bool execute();
   
   virtual void setPercentComplete(double percentComplete);
   
   virtual void setFilename(const rspfFilename& file);
   
   const rspfFilename& getFilename()const;

   /**
    * Load state method:
    *
    * This method call base class rspfConnectableObject::loadState then
    * looks for its keywords.
    *
    * @param kwl Keyword list to initialize from.
    *
    * @param prefix Usually something like: "object1."
    *
    * @return This method will alway return true as it is intended to be
    * used in conjuction with the set methods.
    *
    * Keywords picked up by loadState:
    *
    * filename: foo.tfw
    *
    * (pixel_type should be area or point)
    * 
    * pixel_type: area
    */
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix = 0);
   
   bool canConnectMyInputTo(rspf_int32 inputIndex,
                            const rspfConnectableObject* object) const;
   
   virtual void disconnectInputEvent(rspfConnectionEvent& event);
   virtual void connectInputEvent(rspfConnectionEvent& event);
   virtual void propertyEvent(rspfPropertyEvent& event);

   /**
    * Ossim uses a concept of "pixel is point" internally.
    * 
    * This means that if you say a tie point is 30.0N -81.0W, the center of
    * the pixel at the tie point is 30.0N -81.0W.
    *
    */ 
   virtual void setPixelType(rspfPixelType pixelType);
   virtual rspfPixelType getPixelType() const;

   /**
    * Sets the area of interest to write the meta data for.
    * 
    * @param areaOfInterest Sets theAreaOfInterest.
    *
    * @note By default the writers will use
    * "theInputConnection->getBoundingRect()" if theAreaOfInterest has nans.
    */
   virtual void setAreaOfInterest(const rspfIrect& areaOfInterest);

   /**
    * @returns theAreaOfInterest.
    */
   virtual rspfIrect getAreaOfInterest() const;

   /**
    * void getMetadatatypeList(std::vector<rspfString>& metadatatypeList)const
    *
    * pure virtual
    * 
    * Appends the writers metadata types to the "metadatatypeList".
    * 
    * This is the actual image type name.  So for
    * example, rspfTiffWorldFileWriter has tiff_world_file type.
    *
    * @param metadatatypeList stl::vector<rspfString> list to append to.
    *
    * @note All writers should append to the list, not, clear it and then add
    * their types.
    */
   virtual void getMetadatatypeList(
      std::vector<rspfString>& metadatatypeList)const=0;

   /**
    * bool hasMetadataType(const rspfString& metadataType) const
    *
    * @param imageType string representing image type.
    *
    * @return true if "metadataType" is supported by writer.
    */
   virtual bool hasMetadataType(const rspfString& metadataType)const=0;
   
   /*!
    * property interface
    */
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
protected:
   virtual ~rspfMetadataFileWriter();
   /**
    * Write out the file.
    * @return true on success, false on error.
    */
   virtual bool writeFile() = 0;

   rspfImageSource* theInputConnection;
   rspfFilename              theFilename;
   rspfPixelType             thePixelType;
   rspfIrect                 theAreaOfInterest;
   
TYPE_DATA
};

#endif /* End of #ifndef rspfMetadataFileWriter_H */
