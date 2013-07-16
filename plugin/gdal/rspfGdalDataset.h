#ifndef rspfGdalDataset_HEADER
#define rspfGdalDataset_HEADER
#include <gdal_pam.h>
#include <cpl_string.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/imaging/rspfImageHandler.h>
class rspfGdalDatasetRasterBand;
class rspfFilename;
/**
 * @brief rspfGdalDataset This is a gdal data set that wraps an rspf image
 * handler.
 *
 * @note
 * Currently no write implemented.
 */
class rspfGdalDataset : public GDALPamDataset
{
public:
   /** @brief default constructor */
   rspfGdalDataset();
   /** @brief virtual destructor */
   virtual ~rspfGdalDataset();
   /**
    * @brief open method.
    * @param file The file to open.
    * @return true on success, false on error.
    */
   bool open(const rspfFilename& file);
   /**
    * @brief Open for static gdal driver.
    */
   static GDALDataset *Open( GDALOpenInfo * );
   /**
    * @brief Sets theImageHandler.
    * @param ih Pointer to image handler.
    */
   void setImageHandler(rspfImageHandler* ih);
   /** @return Pointer(const) to the image handler or 0 if not set. */
   const rspfImageHandler* getImageHandler() const;
   /** @return Pointer(not const) to the image handler or 0 if not set. */
   rspfImageHandler*       getImageHandler();
   /**
    * @brief Calls gdal's oOvManager.Initialize.  This must be called if
    * you are building overviews or the code will fail.
    */
   void initGdalOverviewManager();
   /**
    * @brief Set the access data member.
    * Either GA_ReadOnly = 0, or GA_Update = 1
    */
   void setGdalAcces(GDALAccess access);
private:
   
   /**
    * Initializes this object from the image handler.
    */
   void init();
   
   friend class rspfGdalDatasetRasterBand;
   rspfRefPtr<rspfImageHandler>  theImageHandler;
};
/**
 * @brief rspfGdalDatasetRasterBand Represents a single band within the
 * image.
 */
class rspfGdalDatasetRasterBand : public GDALPamRasterBand
{
   friend class rspfGdalDataset;
   
public:
   /**
    * @brief Constructor that takes a rspfGdalDataset, band and image handler.
    * @param ds The parent data set.
    * @param band The "ONE" based band.
    * @param ih The pointer to the image handler.
    */
   rspfGdalDatasetRasterBand( rspfGdalDataset* ds,
                               int band,
                               rspfImageHandler* ih);
   /** virtual destructor */
   virtual ~rspfGdalDatasetRasterBand();
   /**
    * This returns 0 right now and should probably be implemented if anything
    * serious is to be done with this data set with the gdal library.
    */
   virtual double GetNoDataValue( int *pbSuccess = 0 );
   
protected:
   /**
    * @brief Read block method.
    * 
    * @param nBlockXOff X Block offset, "0" being upper left sample of image,
    * 1 being sample at nBlockXOff * nBlockXSize and so on.
    * 
    * @param nBlockYOff YBlock offset, "0" being upper left sample of image,
    * 1 being sample at nBlockYOff * nBlockYSize and so on.
    *
    * @param pImage Buffer to put image data in.  Must be at least:
    * pixel_size_in_bytes * nBlockXSize * nBlockYSize
    *
    * @return CE_None on success, CE_Failure on failure.
    */
   virtual CPLErr IReadBlock(int nBlockXOff,
                             int nBlockYOff,
                             void* pImage);
private:
   rspfRefPtr<rspfImageHandler> theImageHandler;
};
#endif /* End of "#ifndef rspfGdalDataset_HEADER" */
