//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Extensible Metadata Platform (XMP) Info object.
// 
//----------------------------------------------------------------------------
// $Id$

#ifndef rspfXmpInfo_HEADER
#define rspfXmpInfo_HEADER 1

#include <rspf/support_data/rspfInfoBase.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfXmlNode.h>
#include <rspf/projection/rspfProjection.h>
#include <string>

// Forward class declarations:
class rspfDrect;
class rspfString;
class rspfXmlDocument;

/** @class Extensible Metadata Platform (XMP) Info object. */
class RSPF_DLL rspfXmpInfo : public rspfInfoBase
{
public:
   
   /** default constructor */
   rspfXmpInfo();

   /** virtual destructor */
   virtual ~rspfXmpInfo();

   /**
    * @brief open method.
    *
    * @param file File name to open.
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

   /** @return XMP APP1 XML block. */ 
   const std::string& getXmpApp1XmlBlock() const;

   /**
    * @brief Method to get a projection if possible.
    * @param imageRect Require image rectangle of associated meta data
    * parsed by this class.
    * @return Projection pointer wrapped in ref pointer.  Can be null if
    * projection cannot be established.
    */
   rspfRefPtr<rspfProjection> getProjection(
      const rspfDrect& imageRect ) const;
   
private:

   /**
    * @brief Gets the xml block as a string from stream.
    * @param str Input stream.
    * @param xmpApp1XmlBlock String initialized by this.
    * @return true on success, false on error.
    */
   bool getXmpApp1XmlBlock( std::ifstream& str,
                            std::string& xmpApp1XmlBlock ) const;

   /**
    * @brief Gets path from doc and initializes string.
    * @param path Xml path to look for.
    * @param xdoc Xml doc to look in.
    * @param s String to initialize.
    * @return true on success and false if path is not found or if there
    * are more than one of path.
    */
   bool getPath( const rspfString& path,
                 const rspfXmlDocument& xdoc,
                 rspfString& s ) const;

   rspfFilename  m_file;
   std::string    m_xmpApp1XmlBlock;
};

#endif /* End of "#ifndef rspfXmpInfo_HEADER" */
