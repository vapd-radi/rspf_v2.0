#ifndef rspfNitfRpcModel_HEADER
#define rspfNitfRpcModel_HEADER
#include <rspf/base/rspfConstants.h>
#include <rspf/projection/rspfRpcModel.h>
class rspfFilename;
class rspfNitfImageHeader;
/*!****************************************************************************
 *
 * CLASS:  rspfNitfRpcModel
 *
 *****************************************************************************/
class RSPF_DLL rspfNitfRpcModel : public rspfRpcModel
{
public:
   rspfNitfRpcModel();
   rspfNitfRpcModel(const rspfNitfRpcModel& rhs);
   rspfNitfRpcModel(const rspfFilename& nitfFile);
   virtual rspfObject* dup() const;
   /**
    * @brief worldToLineSample()
    * Calls rspfRpcModel::worldToLineSample(), then applies (if needed)
    * decimation.
    * This is a temp work around for decimation RPC NITFs.
    */
   virtual void  worldToLineSample(const rspfGpt& world_point,
                                   rspfDpt&       image_point) const;
   /**
    * @brief lineSampleHeightToWorld()
    * Backs out decimation of image_point (if needed) then calls:
    * rspfRpcModel::lineSampleHeightToWorld
    * This is a temp work around for decimation RPC NITFs.
    */
   virtual void lineSampleHeightToWorld(const rspfDpt& image_point,
                                        const double&   heightEllipsoid,
                                        rspfGpt&       worldPoint) const;
   /**
    * @brief Saves "decimation".  Then calls rspfRpcModel::saveState.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0) const;
   /**
    * @brief Looks for decimation. Then calls rspfRpcModel::loadState.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   /**
    * @brief Method to parse an nitf file and initialize model.
    * @param entryIndex The entry to get model for.  Note the nitf's can have
    * multiple entries; each with a different projection.
    * @return true on success, false on error.
    */
   virtual bool parseFile(const rspfFilename& nitfFile,
                          rspf_uint32 entryIndex=0);
   
   virtual bool parseImageHeader(const rspfNitfImageHeader* ih);
private:
   /**
    * Attempts to get gsd from nitf tags and initialize
    * rspfSensorModel::theGsd.
    * 
    * If unsuccessful  theGsd will be initialized to RSPF_DBL_NAN.
    */
   void getGsd(const rspfNitfImageHeader* ih);
   /**
    * Attempts to get sensor ID from nitf tags and initialize
    * rspfSensorModel::theSensorID.
    * 
    * If unsuccessful  theGsd will be initialized to UNKNOWN.
    */
   void getSensorID(const rspfNitfImageHeader* ih);
   /**
    * Get model information from either the RPC00A or RPC00B tag.
    * @return true on success, false on error.
    */
   bool getRpcData(const rspfNitfImageHeader* ih);
   /**
    * Stored from header field "IMAG".  This is a temp work around to handle
    * decimated rpc nitf's.
    */
   rspf_float64 theDecimation;
   TYPE_DATA
};
#endif
