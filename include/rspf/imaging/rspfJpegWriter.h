//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
//
// Contains class definition for JpegWriter.
//
//*******************************************************************
//  $Id: rspfJpegWriter.h 16597 2010-02-12 15:10:53Z dburken $
#ifndef rspfJpegWriter_HEADER
#define rspfJpegWriter_HEADER

#include <rspf/imaging/rspfImageFileWriter.h>
#include <rspf/base/rspfKeywordlist.h>

//*******************************************************************
// CLASS:  rspfJpegWriterFactory
//*******************************************************************

class RSPFDLLEXPORT rspfJpegWriter : public rspfImageFileWriter
{
public:

   /** default constructor */
   rspfJpegWriter();

   /**
    * void getImageTypeList(std::vector<rspfString>& imageTypeList)const
    *
    * Appends this writer image types to list "imageTypeList".
    *
    * This writer only has one type "jpeg".
    *
    * @param imageTypeList stl::vector<rspfString> list to append to.
    */
   virtual void getImageTypeList(std::vector<rspfString>& imageTypeList)const;
   
   /**
    * Sets the jpeg compression quality.
    *
    * @param quality Compression quality.  (valid range is 1 to 100)
    */
   virtual void  setQuality(rspf_int32 quality);
   
   virtual bool isOpen()const;   
   
   virtual bool open();

   virtual void close();
   
   /**
    * saves the state of the object.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   
   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

   /**
    * Will set the property whose name matches the argument
    * "property->getName()".
    *
    * @param property Object containing property to set.
    */
   virtual void setProperty(rspfRefPtr<rspfProperty> property);

   /**
    * @param name Name of property to return.
    * 
    * @returns A pointer to a property object which matches "name".
    */
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;

   /**
    * Pushes this's names onto the list of property names.
    *
    * @param propertyNames array to add this's property names to.
    */
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;

   /**
    * Returns a 3-letter extension from the image type descriptor 
    * (theOutputImageType) that can be used for image file extensions.
    *
    * @param imageType string representing image type.
    *
    * @return the 3-letter string extension.
    */
   virtual rspfString getExtension() const;

   bool hasImageType(const rspfString& imageType) const;

protected:
   /** virtual destructor */
   virtual ~rspfJpegWriter();
   
   
private:
   virtual bool writeFile();
   
   rspf_int32 theQuality;
   FILE*       theOutputFilePtr;
   
TYPE_DATA
};

#endif /* #ifndef rspfJpegWriter_HEADER */
