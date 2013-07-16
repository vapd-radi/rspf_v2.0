#ifndef rspfOgrInfo_HEADER
#define rspfOgrInfo_HEADER 1
#include <iosfwd>
#include <string>
#include <vector>
#include <rspf/base/rspfConstants.h>
#include <rspf/support_data/rspfInfoBase.h>
#include <rspf/base/rspfFilename.h>
#include <ogrsf_frmts.h>
#include <gdal.h>
class rspfKeywordlist;
/**
 * @brief Ogr info class.
 *
 * Encapsulates the Ogr functionality.
 */
class rspfOgrInfo : public rspfInfoBase
{
public:
   /** default constructor */
   rspfOgrInfo();
   /** virtual destructor */
   virtual ~rspfOgrInfo();
   /**
    * @brief open method.
    *
    * @param file File name to open.
    * The example of SDE file name: SDE:server,instance,database,username,password,layername
    * e.g rspf-info -p -d SDE:SPATCDT001,5151,SDE,SADATABASE,SAPASSWORD,SATABLENAME
    *
    * The example of VPF file name: C:/vpfdata/mpp1/vmaplv0/eurnasia/cat
    * e.g rspf-info -p -d D:/RSPF_Data/vpf_data/WVSPLUS/WVS120M/CAT
    *
    * @return true on success false on error.
    */
   virtual bool open(const rspfFilename& file);
   /**
    * Print method.
    *
    * @param out Stream to print to.
    * 
    * @return std::ostream&
    */
   virtual std::ostream& print(std::ostream& out) const;
   virtual bool getKeywordlist(rspfKeywordlist& kwl)const;
   private: 
     rspfString getDriverName(rspfString driverName)const;
     /**
    * Parse the VPF metadata
    *
    */
     void parseMetadata(rspfString metaData, rspfKeywordlist& kwl, rspfString metaPrefix)const;
     rspfFilename  theFile;
     OGRDataSource* ogrDatasource;
     OGRSFDriver*   ogrDriver;
};
#endif /* End of "#ifndef rspfOgrInfo_HEADER" */
