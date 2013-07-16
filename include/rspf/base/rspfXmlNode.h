//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Oscar Kramer <okramer@imagelinks.com> (rspf port by D. Burken)
//
// Description:  
//
// Contains declaration of class rspfXmlNode.
// 
//*****************************************************************************
// $Id: rspfXmlNode.h 12527 2008-03-04 17:02:55Z gpotts $
#ifndef rspfXmlNode_HEADER
#define rspfXmlNode_HEADER

#include <vector>
using namespace std;

#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfErrorStatusInterface.h>
#include <rspf/base/rspfXmlAttribute.h>
#include <rspf/base/rspfString.h>


class RSPFDLLEXPORT rspfXmlNode : public rspfObject,
                                    public rspfErrorStatusInterface
{
public:
   typedef std::vector<rspfRefPtr<rspfXmlNode> > ChildListType;
   typedef std::vector<rspfRefPtr<rspfXmlAttribute> > AttributeListType;
   
   rspfXmlNode(istream& xml_stream, rspfXmlNode* parent=0);
   rspfXmlNode();
   rspfXmlNode(const rspfXmlNode& src);
   virtual rspfObject* dup() const
   {
      return new rspfXmlNode(*this);
   }
	void duplicateAttributes(rspfXmlNode::AttributeListType result)const;
   void duplicateChildren(rspfXmlNode::ChildListType& result)const;
   bool read(std::istream& in);
   // Appends any matching nodes to the list supplied
   void findChildNodes(const rspfString& rel_xpath,
                       rspfXmlNode::ChildListType& nodelist)const;
   const rspfRefPtr<rspfXmlNode> findFirstNode(const rspfString& rel_xpath)const;
   rspfRefPtr<rspfXmlNode> findFirstNode(const rspfString& rel_xpath);

   rspfRefPtr<rspfXmlAttribute> findAttribute(const rspfString& name);
   const rspfRefPtr<rspfXmlAttribute> findAttribute(const rspfString& name)const;
   void setTag(const rspfString& tag);
   rspfString                      getTag()        const;
   const rspfXmlNode*              getParentNode() const;
   rspfXmlNode*              getParentNode();
   void setParent(rspfXmlNode* parent);
   const rspfXmlNode::ChildListType&      getChildNodes() const;
   rspfXmlNode::ChildListType&      getChildNodes();
   const  rspfXmlNode::AttributeListType& getAttributes() const;
   bool getAttributeValue(rspfString& value, const rspfString& name)const;
   rspfString getAttributeValue(const rspfString& name)const
   {
      rspfString value;
      getAttributeValue(value, name);
      return value;
   }
   bool getChildTextValue(rspfString& value,
                          const rspfString& relPath)const;
   rspfString getChildTextValue(const rspfString& relPath)const
   {
      rspfString value;
      getChildTextValue(value, relPath);
      return value;
   }
   void addAttribute(rspfRefPtr<rspfXmlAttribute> attribute);
   void addAttribute(const rspfString& name, const rspfString& value);
   bool setAttribute(const rspfString& name,
                     const rspfString& value,
                     bool addIfNotPresentFlag=false);
   rspfRefPtr<rspfXmlNode> addNode(const rspfString& relPath,
                                     const rspfString& text = "");
   rspfRefPtr<rspfXmlNode> addOrSetNode(const rspfString& relPath,
                                          const rspfString& text = "");
   void addChildNode(rspfRefPtr<rspfXmlNode> node);
   rspfRefPtr<rspfXmlNode> addChildNode(const rspfString& tagName,
                                          const rspfString& text="");
   void addChildren(rspfXmlNode::ChildListType& children);
   void setChildren(rspfXmlNode::ChildListType& children);
   void addAttributes(rspfXmlNode::AttributeListType& children);
   void setAttributes(rspfXmlNode::AttributeListType& children);
   void setText(const rspfString& text);
   const rspfString&                      getText()       const;
   bool cdataFlag()const;
   void setCDataFlag(bool value);
   RSPFDLLEXPORT friend ostream& operator << (ostream& os, const rspfXmlNode* xml_node);
   RSPFDLLEXPORT friend ostream& operator << (ostream& os, const rspfXmlNode& xml_node);

   rspfRefPtr<rspfXmlNode> removeChild(rspfRefPtr<rspfXmlNode> node);
   rspfRefPtr<rspfXmlNode> removeChild(const rspfString& tag);
   void clear();
   void clearChildren();
   void clearAttributes();
   
   void toKwl(rspfKeywordlist& kwl,
              const rspfString& prefix="")const;
   void fromKwl(const rspfKeywordlist& kwlToConvert);
  
protected:
   ~rspfXmlNode();
   bool readTag(std::istream& in,
                rspfString& tag);
   bool readTextContent(std::istream& in);
   bool readEndTag(std::istream& in,
                   rspfString& endTag);

   void skipCommentTag(std::istream& in);
   bool readCDataContent(std::istream& in);
   rspfString                 theTag;
   rspfXmlNode*         theParentNode;
   vector<rspfRefPtr<rspfXmlNode> >      theChildNodes;
   vector<rspfRefPtr<rspfXmlAttribute> >  theAttributes;
   rspfString                 theText;
   bool                        theCDataFlag;
/*    rspfString                 theCData; */

TYPE_DATA
};

#endif /* #ifndef rspfXmlNode_HEADER */
