#ifndef rspfGdalOverviewBuilder_HEADER
#define rspfGdalOverviewBuilder_HEADER
#include <vector>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfProperty.h>
#include <rspf/imaging/rspfOverviewBuilderBase.h>
class rspfImageSource;
class rspfGdalDataset;
/**
 * @brief rspfGdalOverviewBuilder Class to build overviews from the GDAL
 * library.
 */
class rspfGdalOverviewBuilder
   :
   public rspfOverviewBuilderBase
{
public:
   /** @brief Enumerations for the type of GDAL overviews to build. */
   enum rspfGdalOverviewType
   {
      rspfGdalOverviewType_UNKNOWN = 0,
      rspfGdalOverviewTiffNearest  = 1,
      rspfGdalOverviewTiffAverage  = 2,
      rspfGdalOverviewHfaNearest   = 3,
      rspfGdalOverviewHfaAverage   = 4
   };
   /** @brief default constructor */
   rspfGdalOverviewBuilder();
   /** @brief virtual destructor */
   virtual ~rspfGdalOverviewBuilder();
   /**
    * @brief Open that takes a file name.
    * @param file The file to open.
    * @return true on success, false on error.
    */
   bool open(const rspfFilename& file);
   /**
    * @brief Builds the overviews.
    *
    * @return true on success, false on error.
    *
    * @note If setOutputFile was not called the output name will be derived
    * from the image name.  If image was "foo.tif" the overview file will
    * be "foo.rrd" or "foo.ovr".
    */
   virtual bool execute();
      /**
    * @brief Sets the input to the builder. Satisfies pure virtual from
    * rspfOverviewBuilderBase.
    * @param imageSource The input to the builder.
    * @return True on successful initializion, false on error.
    */
   virtual bool setInputSource(rspfImageHandler* imageSource);
   
   /**
    * @brief Sets the output filename.
    * Satisfies pure virtual from rspfOverviewBuilderBase.
    * @param file The output file name.
    */
   virtual void  setOutputFile(const rspfFilename& file);
   /**
    * Returns the output.  This will be derived from the input file if not
    * explicitly set.
    * 
    * @return The output filename.
    */
   virtual rspfFilename getOutputFile() const;
   /**
    * @brief Sets the overview output type.
    *
    * Satisfies pure virtual from rspfOverviewBuilderBase.
    * 
    * Currently handled types are:
    * "rspf_tiff_nearest" and "rspf_tiff_box"
    *
    * @param type This should be the string representing the type.  This method
    * will do nothing if type is not handled and return false.
    *
    * @return true if type is handled, false if not.
    */
   virtual bool setOverviewType(const rspfString& type);
   /**
    * @brief Gets the overview type.
    * Satisfies pure virtual from rspfOverviewBuilderBase.
    * @return The overview output type as a string.
    */
   virtual rspfString getOverviewType() const;
   /**
    * @brief Method to populate class supported types.
    * Satisfies pure virtual from rspfOverviewBuilderBase.
    * @param typeList List of rspfStrings to add to.
    */
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;
   /**
    * @return rspfObject* to this object. Satisfies pure virtual.
    */
   virtual rspfObject* getObject();
   /**
    * @return const rspfObject* to this object.  Satisfies pure virtual.
    */
   virtual const rspfObject* getObject() const;
   /**
    * @return true if input is an image handler.  Satisfies pure virtual.
    */
   virtual bool canConnectMyInputTo(rspf_int32 index,
                                    const rspfConnectableObject* obj) const;
   /**
    * @brief Method to set properties.
    * @param property Property to set.
    *
    * @note Currently supported property:
    * name=levels, value should be list of levels separated by a comma with
    * no spaces. Example: "2,4,8,16,32,64"
    */
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   /**
    * @brief Method to populate the list of property names.
    * @param propertyNames List to populate.  This does not clear the list
    * just adds to it.
    */
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   
   /**
    * @brief print method.
    * @return std::ostream&
    */
   virtual std::ostream& print(std::ostream& out) const;
private:
   bool generateHfaStats() const;
   /** @return The gdal resampling string from theOverviewType. */
   rspfString getGdalResamplingType() const;
   /** @return extension like "ovr" or "rrd". */
   rspfString getExtensionFromType() const;
   
   rspfGdalDataset*               theDataset;
   rspfFilename                   theOutputFile;
   rspfGdalOverviewType           theOverviewType;
   std::vector<rspf_int32>        theLevels; // like 2, 4, 8, 16, 32
   bool                            theGenerateHfaStatsFlag;
   /** for rtti stuff */
   TYPE_DATA
};
#endif /* End if "#ifndef rspfGdalOverviewBuilder_HEADER" */
