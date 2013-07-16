//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Frank Warmerdam (warmerda@home.com)
//
// Description:
//
// Contains class declaration for TiffWriter.
//
//*******************************************************************
//  $Id: rspfTiffWriter.h 20026 2011-09-01 16:33:18Z dburken $
#ifndef rspfTiffWriter_HEADER
#define rspfTiffWriter_HEADER

#include <rspf/imaging/rspfImageFileWriter.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/projection/rspfMapProjectionInfo.h>
#include <rspf/base/rspfNBandLutDataObject.h>
#include <rspf/imaging/rspfNBandToIndexFilter.h>

class rspfMapProjectionInfo;

class RSPFDLLEXPORT rspfTiffWriter : public rspfImageFileWriter
{
public:

   rspfTiffWriter();
      
   enum UnitType
   {
      UNDEFINED                    =     0,
      LINEAR_METER                 =  9001,
      LINEAR_FOOT                  =  9002,
      LINEAR_FOOT_US_SURVEY        =  9003,
      ANGULAR_DEGREES              =  9102
   };

   enum
   {
      USER_DEFINED = 32767
   };

   /**
    * void getImageTypeList(std::vector<rspfString>& imageTypeList)const
    *
    * Appends this writer image types to list "imageTypeList".
    *
    * This writer has the following types:
    * tiff_strip
    * tiff_strip_band_separate
    * tiff_tiled
    * tiff_tiled_band_separate
    *
    * @param imageTypeList stl::vector<rspfString> list to append to.
    */   
   virtual void getImageTypeList(std::vector<rspfString>& imageTypeList)const;
   
   virtual bool isOpen()const;
   virtual bool open();
   virtual void close();
   
   /**
    * Ranges from 1 to 100 with 100 being the best.  This value is only used
    * if the compression type is set to jpeg.  (default = 75) 
    *
    * @param quality Value to set theJpegQuality to.
    *
    * @note If quality is out of range it will be set to DEFAULT_JPEG_QUALITY.
    */
   virtual void setJpegQuality(rspf_int32 quality);

   /**
    * @return theJpegQuality
    */
   virtual rspf_int32 getJpegQuality()const;

   /**
    * compression type can be JPEG, PACKBITS, or ZIP/DEFLATE
    */
   virtual void setCompressionType(const rspfString& type);

   virtual rspfString getCompressionType()const;

   virtual bool getGeotiffFlag()const;

   virtual void setGeotiffFlag(bool flag);

   virtual void setTileSize(const rspfIpt& tileSize);

   virtual rspfIpt getOutputTileSize()const;
   
   virtual rspf_int32 setProjectionInfo(const rspfMapProjectionInfo& proj);

   void setLut(const rspfNBandLutDataObject& lut);
      
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
    * @returns A pointer to a property object which matches "name".  Returns
    * NULL if no match is found.
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
   
   //! Will take an rspfIMageData tile and write it to disk as a general raster file.
   static void dumpTileToFile(rspfRefPtr<rspfImageData> t,  const rspfFilename& f);

protected:
   virtual ~rspfTiffWriter();
   
   virtual bool writeFile();

   /**
    *  @return true on success, false on error.
    */
   bool openTiff();

   /**
    *  @return true on success, false on error.
    */
   bool closeTiff();

   /**
    *  Writes image data to a tiled tiff format.
    *  @return true on success, false on error.
    */
   bool writeToTiles();

   /**
    *  Writes image data to a tiled tiff band separate format.
    *  @return true on success, false on error.
    */
   bool writeToTilesBandSep();
   
   /**
    *  Writes image data to a strip tiff format.
    *  @return true on success, false on error.
    */
   bool writeToStrips();

   /**
    *  Writes image data to a strip tiff band separate format.
    *  @return true on success, false on error.
    */
   bool writeToStripsBandSep();
   
   /**
    *  Writes tiff tags from rspfMapProjectionInfo to tiff file.
    *  @return true on success, false on error.
    */
   bool writeTiffTags();

   /**
    *  Writes geotiff tags from rspfMapProjectionInfo to tiff file.
    *  @return true on success, false on error.
    */
   bool writeGeotiffTags(rspfRefPtr<rspfMapProjectionInfo> projectionInfo);

   /**
    *  @return true if the output type is tiled, false if not.
    */
   bool isTiled() const;

   void writeMinMaxTags(const vector<rspf_float64>& minBand,
                        const vector<rspf_float64>& maxBand);

   bool isLutEnabled()const;

   UnitType getUnitType(rspf_int32 pcsCode,
                        const rspfString& projName) const;
   UnitType getPcsUnitType(rspf_int32 pcsCode) const;

   void checkColorLut();
   
   void*                   theTif;
   rspfString             theCompressionType;
   rspfString             thePhotoMetric;
   rspf_int32             theJpegQuality;
   bool                    theOutputGeotiffTagsFlag;
   bool                    theColorLutFlag;
   rspfRefPtr<rspfMapProjectionInfo> theProjectionInfo;
   rspfIpt                theOutputTileSize;
   rspfRefPtr<rspfNBandLutDataObject> theColorLut;
   rspfFilename           theLutFilename;
   bool                    theForceBigTiffFlag;
   bool                    theBigTiffFlag;
   mutable rspfRefPtr<rspfNBandToIndexFilter> theNBandToIndexFilter;
TYPE_DATA
};

#endif /* End of "#ifndef rspfTiffWriter_HEADER" */
