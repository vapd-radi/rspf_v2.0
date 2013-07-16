#ifndef rspfGdalWriter_HEADER
#define rspfGdalWriter_HEADER 1
#include <gdal.h>
#include <rspf/imaging/rspfImageFileWriter.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/imaging/rspfNBandToIndexFilter.h>
class rspfXmlNode;
class rspfGdalWriter : public rspfImageFileWriter
{
public:
   enum rspfGdalOverviewType
   {
      rspfGdalOverviewType_NONE    = 0,
      rspfGdalOverviewType_NEAREST = 1,
      rspfGdalOverviewType_AVERAGE = 2
   };
   rspfGdalWriter();
   virtual              ~rspfGdalWriter();
   
   rspfGdalWriter(rspfImageSource *inputSource,
                   const rspfFilename& filename);
   /**
    * void getImageTypeList(std::vector<rspfString>& imageTypeList)const
    *
    * Appends this writer image types to list "imageTypeList".
    *
    * @param imageTypeList stl::vector<rspfString> list to append to.
    */
   virtual void getImageTypeList(std::vector<rspfString>& imageTypeList)const;
   virtual bool hasImageType(const rspfString& imageType) const;
   /**
    * Returns a 3-letter extension from the image type descriptor 
    * (theOutputImageType) that can be used for image file extensions.
    *
    * @param imageType string representing image type.
    *
    * @return the 3-letter string extension.
    */
   virtual rspfString getExtension() const;
   virtual bool isOpen()const;
   
   virtual bool open();
   virtual void close();
   /*!
    * saves the state of the object.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   
   /*!
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   static void getSupportedWriters(vector<rspfString>& writers);
   
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   virtual void  setOutputImageType(const rspfString&  type);
   void setLut(const rspfNBandLutDataObject& lut);
protected:
   virtual bool writeFile();
   virtual bool writeBlockFile();
   virtual void writeProjectionInfo(GDALDatasetH dataset);
   
   virtual void writeColorMap(int bands);
   
   virtual void setOptions(rspfKeywordlist& kwl,
                           const char* prefix=NULL);
   void deleteGdalDriverOptions();
   rspfString convertToDriverName(const rspfString& imageTypeName)const;
   
   GDALDataType getGdalDataType(rspfScalarType scalar);
   void getGdalPropertyNames(std::vector<rspfString>& propertyNames)const;
   const rspfRefPtr<rspfXmlNode> getGdalOptions()const;
   rspf_int32 findOptionIdx(const rspfString& name)const;
   void allocateGdalDriverOptions();
   void storeProperty(const rspfString& name,
                      const rspfString& value);
   bool getStoredPropertyValue(const rspfString& name,
                               rspfString& value)const;
   bool validProperty(const rspfString& name)const;
   rspfString gdalOverviewTypeToString()const;
   rspfGdalOverviewType gdalOverviewTypeFromString(const rspfString& typeString)const;
   void buildGdalOverviews();
   void appendGdalOption(const rspfString& name, const rspfString& value, bool replaceFlag = true);
   /**
    * @brief Method called after good writeFile to clean up any unwanted
    * files.
    */
   void postProcessOutput() const;
   bool isLutEnabled()const;
   void checkColorLut();
   /**
    * @brief Check input to see if it's indexed.
    * 
    * @return True if input data is indexed pallete data, false if not.
    */
   bool isInputDataIndexed();
   
   rspfString  theDriverName;
   
   GDALDriverH  theDriver;
   GDALDatasetH theDataset;
   rspfIpt theJpeg2000TileSize;
   rspfKeywordlist theDriverOptionValues;
   
   char** theGdalDriverOptions;
   rspfGdalOverviewType theGdalOverviewType;
   bool                                         theColorLutFlag;
   rspfRefPtr<rspfNBandLutDataObject>         theColorLut;
   rspfFilename                                theLutFilename;
   mutable rspfRefPtr<rspfNBandToIndexFilter> theNBandToIndexFilter;
   
TYPE_DATA
};
#endif
