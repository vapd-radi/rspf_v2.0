#ifndef rspfHdfInfo_HEADER
#define rspfHdfInfo_HEADER 1
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/support_data/rspfInfoBase.h>
#include <rspf/base/rspfFilename.h>
#include <rspfHdfReader.h>
/**
 * @brief HDF info class.
 *
 * Encapsulates the HDF functionality.
 */
class rspfHdfInfo : public rspfInfoBase
{
public:
   
   /** default constructor */
   rspfHdfInfo();
   
   /** virtual destructor */
   virtual ~rspfHdfInfo();
   
   /**
    * @brief open method.
    *
    * @param file File name to open.
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
   
private: 
   
   rspfFilename                       theFile;
   rspfRefPtr<rspfHdfReader>         m_hdfReader;
   rspfString                         m_driverName;
   std::map<rspfString, rspfString>  m_globalMeta;
   std::vector<rspfString>            m_globalMetaVector;
};
#endif /* End of "#ifndef rspfHdfInfo_HEADER" */
