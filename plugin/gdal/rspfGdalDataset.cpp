#include <fstream>
#include <rspfGdalDataset.h>
#include <rspfGdalType.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>
static GDALDriver *poOssimGdalDriver = 0;
static const rspfTrace traceDebug(rspfString("rspfGdalDataset:debug"));
CPL_C_START
void	GDALRegister_rspfGdalDataset(void);
CPL_C_END
#ifdef RSPF_ID_ENABLED
static const char RSPF_ID[] = "$Id: rspfGdalDataset.cpp 15766 2009-10-20 12:37:09Z gpotts $";
#endif
rspfGdalDataset::rspfGdalDataset()
   : theImageHandler(0)
{
   if (!poOssimGdalDriver)
   {
      GDALRegister_rspfGdalDataset();
      poDriver     = poOssimGdalDriver;
   }
      
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfGdalDataset::rspfGdalDataset  entered..."
         << std::endl;
#ifdef RSPF_ID_ENABLED
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "RSPF_ID:  "
         << RSPF_ID
         << std::endl;
#endif       
   }
}
rspfGdalDataset::~rspfGdalDataset()
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfGdalDataset::~rspfGdalDataset "
         << "\n"
         << std::endl;
   }
   if (theImageHandler.valid())
      theImageHandler = 0;
}
bool rspfGdalDataset::open(const rspfFilename& file)
{
   theImageHandler = rspfImageHandlerRegistry::instance()->open(file);
   
   if ( !theImageHandler.valid() )
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfGdalDataset::open DEBUG:"
            << "\nCould not open:  " << file.c_str() << std::endl;
      }
      return false;
   }
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfGdalDataset::open DEBUG:"
         << "\nOpened:  " << file.c_str() << std::endl;
   }
   init();
   return true;
}
GDALDataset* rspfGdalDataset::Open( GDALOpenInfo * poOpenInfo )
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfGdalDataset::Open  entered..."
         << std::endl;
   }
   rspfFilename f = poOpenInfo->pszFilename;
   GDALDataset* ds = new rspfGdalDataset;
   if ( ((rspfGdalDataset*)ds)->open(f) == true)
   {
      return ds;
   }
   return 0;
}
void rspfGdalDataset::setImageHandler(rspfImageHandler* ih)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfGdalDataset::setImageHandler entered..."
         << std::endl;
   }
   theImageHandler = ih;
   init();
}
const rspfImageHandler* rspfGdalDataset::getImageHandler() const
{
   return theImageHandler.get();
}
rspfImageHandler* rspfGdalDataset::getImageHandler()
{
   return theImageHandler.get();
}
void rspfGdalDataset::initGdalOverviewManager()
{
   rspfFilename f = theImageHandler->getFilename();
   sDescription = f.c_str();
   
   if (theImageHandler.valid())
   {
      oOvManager.Initialize( this, f.c_str() );
   }
}
void rspfGdalDataset::init()
{
   nRasterXSize = theImageHandler->getImageRectangle(0).width();
   nRasterYSize = theImageHandler->getImageRectangle(0).height();
   nBands       = theImageHandler->getNumberOfOutputBands();
   eAccess      = GA_ReadOnly;
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfGdalDataset::init DEBUG:"
            << "\nWidth:  " << nRasterXSize
            << "\nHeight: " << nRasterYSize
            << "\nBands:  " << nBands << std::endl;
   }
   
   for( int iBand = 0; iBand < nBands; ++iBand )
   {
      rspfGdalDatasetRasterBand* rb =
         new rspfGdalDatasetRasterBand( this,
                                         iBand+1,
                                         theImageHandler.get());
      SetBand( iBand+1, rb );
   }
}
void rspfGdalDataset::setGdalAcces(GDALAccess access)
{
   eAccess = access;
}
rspfGdalDatasetRasterBand::rspfGdalDatasetRasterBand(rspfGdalDataset* ds,
                                                       int band,
                                                       rspfImageHandler* ih)
   : GDALPamRasterBand(),
     theImageHandler(ih)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfGdalDatasetRasterBand::rspfGdalDatasetRasterBand entered..."
         << "band: " << band
         << std::endl;
   }
   if (!ih)
   {
      return;
   }
   
   poDS     = ds;
   nBand    = band;
   nRasterXSize     = theImageHandler->getImageRectangle(0).width();
   nRasterYSize     = theImageHandler->getImageRectangle(0).height();
   
   eAccess  = GA_Update;
   
   rspfGdalType gt;
   eDataType = gt.toGdal(theImageHandler->getOutputScalarType());
   
   nBlockXSize      = theImageHandler->getTileWidth();
   nBlockYSize      = theImageHandler->getTileHeight();
   nBlockXSize      = (nBlockXSize==0) ? 1 : nBlockXSize;
   nBlockYSize      = (nBlockYSize==0) ? 1 : nBlockYSize;
   
   nBlocksPerRow    = nRasterXSize / nBlockXSize;
   nBlocksPerColumn = nRasterYSize / nBlockYSize;
   if (nRasterXSize % nBlockXSize) ++nBlocksPerRow;
   if (nRasterYSize % nBlockYSize) ++nBlocksPerColumn;
   nSubBlocksPerRow = 0;
   nSubBlocksPerColumn = 0;
   bSubBlockingActive = FALSE;
   papoBlocks = 0;
   nBlockReads = 0;
   bForceCachedIO = false;
}
rspfGdalDatasetRasterBand::~rspfGdalDatasetRasterBand()
{
}
CPLErr rspfGdalDatasetRasterBand::IReadBlock(int nBlockXOff,
                                              int nBlockYOff,
                                              void * pImage )
{
   if ( !theImageHandler.valid() || !pImage)
   {
      return CE_Failure;
   }
   rspfIpt startPt(nBlockXOff*nBlockXSize, nBlockYOff*nBlockYSize);
   rspfIpt endPt( startPt.x+nBlockXSize-1,
                   startPt.y+nBlockYSize-1);
   rspfIrect rect( startPt, endPt);
   rspfRefPtr<rspfImageData> id = theImageHandler->getTile(rect, 0);
   
   if (id.valid())
   {
      if( (id->getDataObjectStatus() == RSPF_FULL) ||
          (id->getDataObjectStatus() == RSPF_PARTIAL) )
      {
         id->unloadBand(pImage, rect, nBand-1);
         return CE_None;
      }
   }
   memset(pImage, 0, nBlockXSize * nBlockYSize);      
   return CE_None;
}
double rspfGdalDatasetRasterBand::GetNoDataValue( int *pbSuccess )
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfGdalDatasetRasterBand::GetNoDataValue entered..."
         << "\n"
         << std::endl;
   }
   return 0.0;
}
void GDALRegister_rspfGdalDataset()
{
   GDALDriver	*poDriver;
   
   if( poOssimGdalDriver == 0 )
   {
      poDriver = new GDALDriver();
      poOssimGdalDriver = poDriver;
      
      poDriver->SetDescription( "rspfGdalDataset" );
      poDriver->SetMetadataItem( GDAL_DMD_LONGNAME, 
                                 "rspf data set" );
      poDriver->pfnOpen = rspfGdalDataset::Open;
      GetGDALDriverManager()->RegisterDriver( poDriver );
   }
}
