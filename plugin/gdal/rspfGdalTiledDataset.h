#ifndef rspfGdalTiledDataset_HEADER
#define rspfGdalTiledDataset_HEADER
#include <gdal_priv.h>
#include <memdataset.h>
#include <rspf/vpfutil/set.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfListenerManager.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfIrect.h>
class MEMTiledRasterBand;
class rspfImageSourceSequencer;
class rspfImageData;
class CPL_DLL MEMTiledDataset : public MEMDataset,
   public rspfListenerManager
{
private:
   friend class MEMTiledRasterBand;
   rspfRefPtr<rspfImageData> theData;
   rspfImageSourceSequencer*  theInterface;
   rspfIpt                    theTileSize;
   rspfIrect                  theAreaOfInterest;
   bool                        theJustCreatedFlag;
   /**
    * DRB - 20081020
    * If true (default) the no data value will be set to null pixel
    * value.
    */
   bool theSetNoDataValueFlag;
   
   void create(rspfImageSourceSequencer* iface);
 
public:
   MEMTiledDataset();
   MEMTiledDataset(rspfImageSourceSequencer* iface);
   ~MEMTiledDataset();
   static GDALDataset *Create( const char * pszFilename,
                               int nXSize, int nYSize, int nBands,
                               GDALDataType eType, char ** papszParmList );
   /**
    * DRB - 20081020
    * If true (default) the no data value will be set to null pixel
    * value.
    *
    * The call to rasterBand->SetNoDataValue causes snow with some viewer
    * with some data types.  Like J2K.  This can disable it.
    *
    * @param flag If true the call to rasterBand->SetNoDataValue will be
    * performed.  If false it will be bypassed.
    */
   void setNoDataValueFlag(bool flag);
};
/************************************************************************/
/*                            MEMRasterBand                             */
/************************************************************************/
class CPL_DLL MEMTiledRasterBand : public MEMRasterBand
{
protected:
   
   friend class MEMTiledDataset;
   MEMTiledDataset* theDataset;
   rspfImageSourceSequencer* theInterface;
   
public:
   MEMTiledRasterBand( GDALDataset *poDS,
                       int nBand,
                       GByte *pabyData,
                       GDALDataType eType,
                       int nPixelOffset,
                       int nLineOffset,
                       int bAssumeOwnership );
   
    virtual        ~MEMTiledRasterBand();
    
    virtual CPLErr IReadBlock( int, int, void * );
    virtual CPLErr IWriteBlock( int, int, void * );
private:
    /**
     * Copies null values to pImage.
     * @param pImage Buffer to copy to.
     * @param count pixels to null out.
     */
    void copyNulls(void* pImage, int count) const;
    
    /**
     * Copies null values to pImage.
     * @param pImage Buffer to copy to.
     * @param count pixels to null out.
     * @param dummyTemplate Dummy for scalar type.
     */
    template <class T> void copyNulls(void* pImage,
                                      int count,
                                      T dummyTemplate) const;
    
};
#endif
