//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Oscar Kramer <okramer@imagelinks.com> (rspf port by D. Burken)
//
// Description:  
//
// Contains declaration of class rspfXmlDocument. This class provides read-only
// parsing and accessing of an XML document file.
// 
//*****************************************************************************
// $Id: rspfXmlDocument.h 21579 2012-08-31 14:24:11Z dburken $
#ifndef rspfXmlDocument_HEADER
#define rspfXmlDocument_HEADER 1

#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfErrorStatusInterface.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfXmlNode.h>
#include <vector>

class RSPFDLLEXPORT rspfXmlDocument :
   public rspfObject, public rspfErrorStatusInterface
{
public:
   rspfXmlDocument(const rspfFilename& xmlFileName="");
   rspfXmlDocument(const rspfXmlDocument& src);
   virtual rspfObject* dup()const
   {
      return new rspfXmlDocument(*this);
   }
   virtual ~rspfXmlDocument();

   bool write(const rspfFilename& file);
   bool openFile(const rspfFilename& filename);
   bool read(std::istream& in);
   /**
    * Appends any matching nodes to the list supplied (should be empty):
    */
   void findNodes(const rspfString& xpath,
                  std::vector<rspfRefPtr<rspfXmlNode> >& nodelist) const;
   
   RSPFDLLEXPORT friend std::ostream& operator << (std::ostream& os, const rspfXmlDocument& xml_doc);

   void initRoot(rspfRefPtr<rspfXmlNode> node);

   rspfRefPtr<rspfXmlNode> getRoot();
   const rspfRefPtr<rspfXmlNode> getRoot()const;
   rspfRefPtr<rspfXmlNode> removeRoot();
   void fromKwl(const rspfKeywordlist& kwl);
   void toKwl(rspfKeywordlist& kwl, const rspfString& prefix = "")const;
   
private:
   rspfRefPtr<rspfXmlNode>  theRootNode;
   rspfString                theXmlHeader;
   rspfFilename              theFilename;
   bool                       theStrictCheckFlag;
   bool readHeader(std::istream& in);
TYPE_DATA
};

#endif /* #ifndef rspfXmlDocument_HEADER */
