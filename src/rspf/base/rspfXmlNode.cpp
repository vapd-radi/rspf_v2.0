//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Oscar Kramer <okramer@imagelinks.com> (rspf port by D. Burken)
//
// Description:  
//
// Contains definition of class rspfXmlNode.
// 
//*****************************************************************************
// $Id: rspfXmlNode.cpp 20747 2012-04-18 15:24:12Z gpotts $

#include <iostream>
#include <stack>
#include <rspf/base/rspfXmlNode.h>
#include <rspf/base/rspfXmlAttribute.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfTrace.h>

static rspfTrace traceDebug("rspfXmlNode:debug");
static const rspfString XPATH_DELIM ("/");

RTTI_DEF2(rspfXmlNode, "rspfXmlNode", rspfObject, rspfErrorStatusInterface);

static std::istream& xmlskipws(std::istream& in)
{
   int c = in.peek();
   while( !in.fail() &&
         (( (c== ' ') || (c == '\t') || (c == '\n')|| (c == '\r') || (c<0x20) || (c>=0x7f) ))
         )
   {
      in.ignore(1);
      c = in.peek();
   }
   
   return in;
}

rspfXmlNode::rspfXmlNode(istream& xml_stream, rspfXmlNode* parent)
:
theParentNode (parent),
theCDataFlag(false)
{
   read(xml_stream);
}

rspfXmlNode::rspfXmlNode()
:theParentNode(0),
theCDataFlag(false)
{
}

rspfXmlNode::rspfXmlNode(const rspfXmlNode& src)
:theTag(src.theTag),
theParentNode(0),
theText(src.theText),
theCDataFlag(src.theCDataFlag)
{
   rspf_uint32 idx = 0;
   for(idx = 0; idx < src.theChildNodes.size();++idx)
   {
      theChildNodes.push_back((rspfXmlNode*)(src.theChildNodes[idx]->dup()));
   }
   for(idx = 0; idx < src.theAttributes.size();++idx)
   {
      theAttributes.push_back((rspfXmlAttribute*)(src.theAttributes[idx]->dup()));
   }
}

rspfXmlNode::~rspfXmlNode()
{
}

void rspfXmlNode::duplicateAttributes(rspfXmlNode::AttributeListType result)const
{
   rspf_uint32 idx = 0;
   for(idx = 0; idx<theAttributes.size();++idx)
   {
      result.push_back((rspfXmlAttribute*)theAttributes[idx]->dup());
   }
	
}

void rspfXmlNode::duplicateChildren(rspfXmlNode::ChildListType& result)const
{
   rspf_uint32 idx = 0;
   for(idx = 0; idx<theChildNodes.size();++idx)
   {
      result.push_back((rspfXmlNode*)theChildNodes[idx]->dup());
   }
}

void rspfXmlNode::setParent(rspfXmlNode* parent)
{
   theParentNode = parent;
}

void rspfXmlNode::skipCommentTag(std::istream& in)
{
   char c;
   while(!in.fail())
   {
      c = in.get();
      if(c == '-')
      {
         if(in.peek() == '-')
         {
            in.ignore();
            if(in.peek() == '>')
            {
               in.ignore();
               return;
            }
         }
      }
   }
}

bool rspfXmlNode::read(std::istream& in)
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfXmlNode::read: entered ......" << std::endl;
   }
   char c;
   xmlskipws(in);
   if(in.fail())
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfXmlNode::read: leaving ......\n"<<__LINE__ << "\n";
      }
      return false;
   }
   if(in.peek() == '<')
   {
      in.ignore(1);
   }
   if(in.fail())
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfXmlNode::read: leaving ......\n"<<__LINE__ << "\n";
      }
      return false;
   }

   rspfString endTag;
   
   if(!readTag(in, theTag))
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfXmlNode::read: leaving ......\n"<<__LINE__ << "\n";
      }
      return false;
   }
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "theTag = " << theTag << "\n";
   }
   
   if((!in.fail())&&readEndTag(in, endTag))
   {
      if((endTag == "")||
         (endTag == theTag))
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "rspfXmlNode::read: leaving ......\n"<<__LINE__ << "\n";
         }
         return true;
      }
      else
      {
         setErrorStatus();
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "rspfXmlNode::read: leaving ......\n"<<__LINE__ << "\n";
         }
         return false;
      }
   }
   // now parse attributes
   rspfRefPtr<rspfXmlAttribute> attribute = new rspfXmlAttribute;
   while(attribute->read(in))
   {
      theAttributes.push_back(new rspfXmlAttribute(*attribute));
   }
   // skip white space characters
   //
   xmlskipws(in);
   
   if(!in.fail()&&readEndTag(in, endTag))
   {
      if((endTag == "")||
         (endTag == theTag))
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "rspfXmlNode::read: leaving ......\n"<<__LINE__ << "\n";
         }
         return true;
      }
      else
      {
         setErrorStatus();
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "rspfXmlNode::read: leaving ......\n"<<__LINE__ << "\n";
         }
         return false;
      }
   }
   c = in.peek();
   // make sure the attribute is closed
   //
   if(c != '>')
   {
      setErrorStatus();
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfXmlNode::read: leaving ......\n"<<__LINE__ << "\n";
      }
      return false;
   }
   
   in.ignore(1);
   c = in.peek();
   
   // now do the text portion
   if(!readTextContent(in))
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfXmlNode::read: leaving ......\n"<<__LINE__ << "\n";
      }
      return false;
   }
   xmlskipws(in);
   c = in.peek();
   
   if(c != '<')
   {
      setErrorStatus();
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfXmlNode::read: leaving ......\n"<<__LINE__ << "\n";
      }
      return false;
   }
   in.ignore(1);
   if(readEndTag(in, endTag))
   {
      if((endTag == "")||
         (endTag == theTag))
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "rspfXmlNode::read: leaving ......\n"<<__LINE__ << "\n";
         }
         return true;
      }
      else
      {
         setErrorStatus();
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "rspfXmlNode::read: leaving ......\n"<<__LINE__ << "\n";
         }
         return false;
      }
   }
   c = in.peek();

   //---
   // now if it's not an endtag then it must be a tag starting the new child
   // node
   //---
   rspfRefPtr<rspfXmlNode> childNode;
   do
   {
      childNode = new rspfXmlNode;
      childNode->setParent(this);
      if(childNode->read(in))
      {
         theChildNodes.push_back(childNode);
      }
      else
      {
         setErrorStatus();
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "rspfXmlNode::read: leaving ......\n"<<__LINE__ << "\n";
         }
         return false;
      }
      xmlskipws(in);
      
      c = in.peek();
      if(c != '<')
      {
         setErrorStatus();
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "rspfXmlNode::read: leaving ......\n"<<__LINE__ << "\n";
         }
         return false;
      }
      in.ignore(1);
      if(readEndTag(in, endTag))
      {
         if((endTag == "")||
            (endTag == theTag))
         {
            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << "rspfXmlNode::read: leaving ......\n"<<__LINE__ << "\n";
            }
            return true;
         }
         else
         {
            setErrorStatus();
            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << "rspfXmlNode::read: leaving ......\n"<<__LINE__ << "\n";
            }
            return false;
         }
      }
   }while( !in.fail() );
   
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfXmlNode::read: leaving ......\n";
   }
   return true;
}


void rspfXmlNode::findChildNodes(const rspfString& xpath,
                                  rspfXmlNode::ChildListType& result)const
{
   //***
   // Scan for trivial result (no children owned):
   //***
   if (theChildNodes.size() == 0)
      return;
   
   //***
   // Make a copy to manipulate:
   //***
   rspfString rel_xpath (xpath);
   if (rel_xpath.empty())
      return;
   
   //---
   // First verify that this is not an absolute path:
   //---
   if (rel_xpath[static_cast<std::string::size_type>(0)] ==
       XPATH_DELIM[static_cast<std::string::size_type>(0)])
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "WARNING: rspfXmlNode::findChildNodes\n"
            << "Only relative XPaths can be searched from a node. "
             << "Returning null list...\n";
         }
      return;
   }
   
   //***
   // Read the desired tag from the relative xpath
   //***
   rspfString desired_tag (rel_xpath);
   if (rel_xpath.contains(XPATH_DELIM))
      desired_tag = rel_xpath.before(XPATH_DELIM);
   rspfString sub_xpath (rel_xpath.after(XPATH_DELIM));
   
   //***
   // Loop over all child nodes for match:
   //***
   rspfXmlNode::ChildListType::const_iterator child_iter = theChildNodes.begin();
   while(child_iter != theChildNodes.end())
   {
      if ((*child_iter)->getTag() == desired_tag)
      {
         if (sub_xpath.empty())
         {
            //***
            // This was the final target node, simply append to the result:
            //***
            result.push_back(*child_iter);
         }
         else
         {
            //***
            // This match identifies a possible tree to search given the
            // remaining xpath (sub_xpath). Query this child node to search
            // its tree for the remaining xpath:
            //***
            (*child_iter)->findChildNodes(sub_xpath, result);
         }
      }
      
      //***
      // Proceed to next child:
      //***
      child_iter++;
   }
}

const rspfRefPtr<rspfXmlNode> rspfXmlNode::findFirstNode(const rspfString& xpath)const
{
   if(theChildNodes.size() < 1) return 0;
   //
   // Make a copy to manipulate:
   //
   rspfString rel_xpath (xpath);
   if (rel_xpath.empty())
      return 0;
   
   //
   // First verify that this is not an absolute path:
   //
   if (rel_xpath[static_cast<std::string::size_type>(0)] ==
       XPATH_DELIM[static_cast<std::string::size_type>(0)])
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
         << "WARNING: rspfXmlNode::findChildNodes\n"
         << "Only relative XPaths can be searched from a node. "
         << "Returning null list...\n";

      }
      return 0;
   }
   
   //
   // Read the desired tag from the relative xpath
   //
   rspfString desired_tag (rel_xpath);
   if (rel_xpath.contains(XPATH_DELIM))
      desired_tag = rel_xpath.before(XPATH_DELIM);
   rspfString sub_xpath (rel_xpath.after(XPATH_DELIM));
   
   rspfRefPtr<rspfXmlNode> result = 0;
   
   //
   // Loop over all child nodes for match:
   //
   rspfXmlNode::ChildListType::const_iterator child_iter = theChildNodes.begin();
   while((child_iter != theChildNodes.end())&&
         (!result.valid()))
   {
      if ((*child_iter)->getTag() == desired_tag)
      {
         if (sub_xpath.empty())
         {
            return *child_iter;
         }
         else
         {
            //
            // This match identifies a possible tree to search given the
            // remaining xpath (sub_xpath). Query this child node to search
            // its tree for the remaining xpath:
            //
            result = (*child_iter)->findFirstNode(sub_xpath);
         }
      }
      
      //
      // Proceed to next child:
      //
      ++child_iter;
   }
   
   return result;
}

rspfRefPtr<rspfXmlNode> rspfXmlNode::findFirstNode(const rspfString& xpath)
{
   if(theChildNodes.size() < 1) return 0;
   //
   // Make a copy to manipulate:
   //
   rspfString rel_xpath (xpath);
   if (rel_xpath.empty())
      return 0;
   
   //
   // First verify that this is not an absolute path:
   //
   if (rel_xpath[static_cast<std::string::size_type>(0)] ==
       XPATH_DELIM[static_cast<std::string::size_type>(0)])
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
         << "WARNING: rspfXmlNode::findChildNodes\n"
         << "Only relative XPaths can be searched from a node. "
         << "Returning null list...\n";
      }
      return 0;
   }
   
   //
   // Read the desired tag from the relative xpath
   //
   rspfString desired_tag (rel_xpath);
   if (rel_xpath.contains(XPATH_DELIM))
      desired_tag = rel_xpath.before(XPATH_DELIM);
   rspfString sub_xpath (rel_xpath.after(XPATH_DELIM));
   
   rspfRefPtr<rspfXmlNode> result = 0;
   
   //
   // Loop over all child nodes for match:
   //
   rspfXmlNode::ChildListType::iterator child_iter = theChildNodes.begin();
   while((child_iter != theChildNodes.end())&&
         (!result.valid()))
   {
      if ((*child_iter)->getTag() == desired_tag)
      {
         if (sub_xpath.empty())
         {
            return *child_iter;
         }
         else
         {
            //
            // This match identifies a possible tree to search given the
            // remaining xpath (sub_xpath). Query this child node to search
            // its tree for the remaining xpath:
            //
            result = (*child_iter)->findFirstNode(sub_xpath);
         }
      }
      
      //
      // Proceed to next child:
      //
      ++child_iter;
   }
   
   return result;
}

rspfRefPtr<rspfXmlAttribute> rspfXmlNode::findAttribute(const rspfString& name)
{
   rspf_uint32 idx = 0;
   
   for(idx = 0; idx < theAttributes.size();++idx)
   {
      if(theAttributes[idx]->getName() == name)
      {
         return theAttributes[idx];
      }
   }
   
   return 0;
}

const rspfRefPtr<rspfXmlAttribute> rspfXmlNode::findAttribute(const rspfString& name)const
{
   rspf_uint32 idx = 0;
   
   for(idx = 0; idx < theAttributes.size();++idx)
   {
      if(theAttributes[idx]->getName() == name)
      {
         return theAttributes[idx];
      }
   }
   
   return 0;
}

void rspfXmlNode::setTag(const rspfString& tag)
{
   theTag = tag;
}

rspfString rspfXmlNode::getTag() const
{
   return theTag;
}

const rspfXmlNode* rspfXmlNode::getParentNode() const
{
   return theParentNode;
}

rspfXmlNode* rspfXmlNode::getParentNode()
{
   return theParentNode;
}

const rspfXmlNode::ChildListType& rspfXmlNode::getChildNodes() const
{
   return theChildNodes;
}

rspfXmlNode::ChildListType& rspfXmlNode::getChildNodes()
{
   return theChildNodes;
}

const rspfXmlNode::AttributeListType& rspfXmlNode::getAttributes() const
{
   return theAttributes;
}

bool rspfXmlNode::getAttributeValue(rspfString& value, const rspfString& name)const
{
   rspfRefPtr<rspfXmlAttribute> attribute = findAttribute(name);
   
   if(attribute.valid())
   {
      value = attribute->getValue();
   }
   
   return attribute.valid();
}

bool rspfXmlNode::getChildTextValue(rspfString& value,
                                     const rspfString& relPath)const
{
   rspfRefPtr<rspfXmlNode> node = findFirstNode(relPath);
   if(node.valid())
   {
      value = node->getText();
   }
   
   return node.valid();
}


void rspfXmlNode::setText(const rspfString& text)
{
   theText = text;
}

const rspfString& rspfXmlNode::getText() const
{
   return theText;
}

bool rspfXmlNode::cdataFlag()const
{
   return theCDataFlag;
}

void rspfXmlNode::setCDataFlag(bool value)
{
   theCDataFlag = value;
}

ostream& operator << (ostream& os, const rspfXmlNode& xml_node)
{
   return operator <<(os, &xml_node);
}

//**************************************************************************
//  FRIEND OPERATOR
//**************************************************************************
ostream& operator << (ostream& os, const rspfXmlNode* xml_node) 
{
   //
   // Determine the indentation level:
   //
   rspfString indent ("");
   const rspfXmlNode* parent = xml_node->theParentNode;
   while (parent)
   {
      indent += "   ";
      parent = parent->theParentNode;
   }
   
   //
   // Dump the tag opening:
   //
   os << "\n" << indent << "<" << xml_node->theTag;
   
   //
   // Dump any attributes:
   //
   if (xml_node->theAttributes.size())
   {
      rspfXmlNode::AttributeListType::const_iterator attr =
      xml_node->theAttributes.begin();
      while (attr != xml_node->theAttributes.end())
      {
         os << attr->get();
         attr++;
      }
   }
   
   if((xml_node->theChildNodes.size() == 0)&&
      (xml_node->theText == ""))
   {
      os << "/>";
   }
   else
   {
      os << ">";
      
      if(xml_node->cdataFlag())
      {
         os << "<![CDATA[" <<xml_node->theText <<  "]]>";
      }
      else
      {
         //
         // Dump any text:
         //
         os << xml_node->theText;
      }
      //
      // Dump any child nodes:
      //
      if (xml_node->theChildNodes.size())
      {
         rspfXmlNode::ChildListType::const_iterator nodes = xml_node->theChildNodes.begin();
         while (nodes != xml_node->theChildNodes.end())
         {
            os << (*nodes).get();
            nodes++;
         }
         os << "\n" << indent;
      }
      
      //
      // Dump the tag closing:
      // 
      os << "</" << xml_node->theTag << ">";
   }
   
   return os;
}

void rspfXmlNode::addAttribute(rspfRefPtr<rspfXmlAttribute> attribute)
{
   theAttributes.push_back(attribute.get());
}

void rspfXmlNode::addAttribute(const rspfString& name,
                                const rspfString& value)
{
   theAttributes.push_back(new rspfXmlAttribute(name, value));
}

bool  rspfXmlNode::setAttribute(const rspfString& name,
                                 const rspfString& value,
                                 bool addIfNotPresentFlag)
{
   bool result = false;
   rspfRefPtr<rspfXmlAttribute> attribute = findAttribute(name);
   if(attribute.valid())
   {
      attribute->setValue(value);
      result = true;
   }
   else
   {
      if(addIfNotPresentFlag)
      {
         addAttribute(name, value);
         result = true;
      }
   }
   
   return result;
}

rspfRefPtr<rspfXmlNode> rspfXmlNode::addNode(const rspfString& relPath,
                                                const rspfString& text)
{
   //
   // Make a copy to manipulate:
   //
   rspfString relXpath (relPath);
   if (relXpath.empty())
      return 0;
   
   //
   // First verify that this is not an absolute path:
   //
   if (relXpath[static_cast<std::string::size_type>(0)] ==
       XPATH_DELIM[static_cast<std::string::size_type>(0)])
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN) << "WARNING: rspfXmlNode::findChildNodes\n"
         << "Only relative XPaths can be searched from a node. "
         << "Returning null list...\n";
         
      }
      return 0;
   }
   
   //
   // Read the desired tag from the relative xpath
   //
   rspfString desiredTag (relXpath);
   if (relXpath.contains(XPATH_DELIM))
   {
      desiredTag = relXpath.before(XPATH_DELIM);
   }
   rspfString subPath (relXpath.after(XPATH_DELIM));
   
   rspfRefPtr<rspfXmlNode> node = findFirstNode(desiredTag);
   
   if(!node.valid())
   {
      if(subPath.empty())
      {
         node = addChildNode(desiredTag, text);
      }
      else
      {
         node = addChildNode(desiredTag, "");
      }
   }
   if(!subPath.empty())
   {
      return node->addNode(subPath, text);
   }
   
   return node;
}

rspfRefPtr<rspfXmlNode> rspfXmlNode::addOrSetNode(const rspfString& relPath,
                                                     const rspfString& text)
{
   
   rspfRefPtr<rspfXmlNode> result = addNode(relPath, text);
   
   result->setText(text);
   
   return result;
}

void rspfXmlNode::addChildNode(rspfRefPtr<rspfXmlNode> node)
{
   if(node->theParentNode)
   {
      node->theParentNode->removeChild(node);
   }
   node->theParentNode = this;
   theChildNodes.push_back(node.get());
}

rspfRefPtr<rspfXmlNode> rspfXmlNode::addChildNode(const rspfString& tagName,
                                                     const rspfString& text)
{
   rspfRefPtr<rspfXmlNode> node = new rspfXmlNode();
   node->setParent(this);
   node->theTag = tagName;
   node->theText = text;
   theChildNodes.push_back(node);
   
   return node;
}

rspfRefPtr<rspfXmlNode> rspfXmlNode::removeChild(rspfRefPtr<rspfXmlNode> node)
{
   rspfXmlNode::ChildListType::iterator iter = theChildNodes.begin();
   while(iter != theChildNodes.end())
   {
      
      if(node == iter->get())
      {
         rspfRefPtr<rspfXmlNode> temp = *iter;
         
         theChildNodes.erase(iter);
         
         return temp;
      }
      ++iter;
   }
   
   return 0;
}

rspfRefPtr<rspfXmlNode> rspfXmlNode::removeChild(const rspfString& tag)
{
   rspfXmlNode::ChildListType::iterator iter = theChildNodes.begin();
   while(iter != theChildNodes.end())
   {
      if(tag == iter->get()->theTag)
      {
         rspfRefPtr<rspfXmlNode> temp = *iter;
         
         theChildNodes.erase(iter);
         
         return temp;
      }
      ++iter;
   }
   
   return 0;
}
void rspfXmlNode::addChildren(rspfXmlNode::ChildListType& children)
{
   rspf_uint32 idx;
   for(idx = 0; idx < children.size(); ++idx)
   {
      addChildNode(children[idx].get());
   }
}

void rspfXmlNode::setChildren(rspfXmlNode::ChildListType& children)
{
   clearChildren();
   addChildren(children);
}

void rspfXmlNode::addAttributes(rspfXmlNode::AttributeListType& children)
{
   rspf_uint32 idx;
   
   for(idx = 0; idx < children.size(); ++idx)
   {
      addAttribute(children[idx].get());
   }
}

void rspfXmlNode::setAttributes(rspfXmlNode::AttributeListType& children)
{
   clearAttributes();
   addAttributes(children);
}

void rspfXmlNode::clear()
{
   theChildNodes.clear();
   theAttributes.clear();
   theTag="";
   theText="";
   theCDataFlag=false;
}

void rspfXmlNode::clearChildren()
{
   theChildNodes.clear();
}

void rspfXmlNode::clearAttributes()
{
   theAttributes.clear();
}

void rspfXmlNode::toKwl(rspfKeywordlist& kwl,
                         const rspfString& prefix)const
{
   rspfString name = getTag();
   rspfString value = getText();
   
   rspfString copyPrefix = prefix;
   
   if(name != "")
   {
      copyPrefix += (name+".");
   }
   if(theChildNodes.size() < 1)
   {
      kwl.add(prefix+name,
              value,
              true);
   }

   rspfString attributePrefix = copyPrefix + "@";
   rspf_uint32 attributeIdx = 0;
   for(attributeIdx = 0; attributeIdx < theAttributes.size(); ++attributeIdx)
   {
      kwl.add(attributePrefix+theAttributes[attributeIdx]->getName(),
              theAttributes[attributeIdx]->getValue(), 
              true);
   }

   rspf_uint32 idx = 0;
   for(idx = 0; idx < theChildNodes.size();++idx)
   {
      theChildNodes[idx]->toKwl(kwl,
                                copyPrefix);
   }
   
}

bool rspfXmlNode::readTag(std::istream& in,
                           rspfString& tag)
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfXmlNode::readTag: entered ......\n";
   }
   xmlskipws(in);
   
   tag = "";
   int c = in.peek();
   
   // bool validTag = false;
   //    while(!validTag)
   {
      while( (c != ' ')&&
            (c != '\n')&&
            (c != '\t')&&
            (c != '\r')&&
            (c != '<')&&
            (c != '>')&&
            (c != '/')&&
            (!in.fail()))
      {
         tag += (char)c;
         in.ignore(1);
         c = in.peek();
         if(tag == "!--") // ignore comment tags
         {
            tag = "--";
            break;
         }
      }
   }
   
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfXmlNode::readTag: leaving ......\n";
   }
  
   return ((tag != "")&&(!in.fail()));
}

bool rspfXmlNode::readCDataContent(std::istream& in)
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfXmlNode::readCDataContent: entered ..." << std::endl;
   }   
   
   // Ignore up to "]]>"
   
   bool result = false;

   char c;

   while(!in.fail())
   {
      c = in.get();
      if ( c != ']' )
      {
         theText += c;
      }
      else // at "]"
      {
         c = in.get();
         if( c == ']' ) // at "]]"
         {
            c = in.get();
            if( c == '>' )
            {
               //in >> xmlskipws;
               result = true;
               break;
            }
         }
      }
   }

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "theText: " << theText
         << "\nexit status: " << (result?"true":"false")
         << "\nrspfXmlNode::readCDataContent: leaving ..." << std::endl;
   }
   
   return result;
}

#if 0
bool rspfXmlNode::readTextContent(std::istream& in)
{
   xmlskipws(in);
   
   theText = "";
   theCDataFlag = false;
   
   char buf[9];
   buf[8]='\0';
   
   std::streampos initialPos = in.tellg();
   
   in.read(buf,9);
   rspfString ostrBuf(buf);
   
   if(ostrBuf == "<![CDATA[")
   {
	   if(readCDataContent(in))
      {
         theCDataFlag = true;
         return true;
      }
	   else
	   {
         return false;
	   }
   }
   else if(ostrBuf.substr(0,4) == "<!--")
   {
	   in.seekg(initialPos);
	   char c = in.get();
	   // Strip comment
	   while(!in.fail()) // continue until we see a --> pattern
	   {
         c = in.get();
         if(c == '-')
         {
            c = in.get();
            if(c == '-')
            {
               c = in.get();
               if(c == '>')
               {
                  break;
               }
            }
         }
	   }
   }
   else if(ostrBuf.substr(0,1) ==  "<")
   {
	   in.seekg(initialPos);
   }
   else
   {
      in.seekg(initialPos);
	   char c = in.peek();
	   while(!in.fail() && c != '<')
	   {
         theText += (char)in.get();
         c = in.peek();
	   }
   }
   return !in.fail();
}
#endif
#if 0
bool rspfXmlNode::readTextContent(std::istream& in)
{
   xmlskipws(in);
   
   theText = "";
   theCDataFlag = false;
   char c = in.peek();
   
   do
   {
      if(c == '<')
      {
         in.ignore();
         
         // we will check for comments or CDATA
         if(in.peek()=='!')
         {
            char buf1[4];
            buf1[3] = '\0';
            in.read(buf1, 3);
            if(rspfString(buf1) == "!--")
            {
               // special text read
               theText += buf1;
               bool done = false;
               do
               {
                  if(in.peek() != '-')
                  {
                     in.ignore();
                  }
                  else
                  {
                     in.ignore();
                     if(in.peek() == '-')
                     {
                        in.ignore();
                        if(in.peek() == '>')
                        {
                           in.ignore();
                           done = true;
                           c = in.peek();
                        }
                     }
                  }
               }while(!done&&!in.fail());
            }
            else
            {
               
               char buf2[6];
               buf2[5] = '\0';
               in.read(buf2, 5);
               if(in.fail())
               {
                  return false;
               }
               if(rspfString(buf1)+rspfString(buf2) == "![CDATA[")
               {
                  if(readCDataContent(in))
                  {
                     theCDataFlag = true;
                     return true;
                  }
               }
            }
         }
         else
         {
            in.putback(c);
            return true;
         }
      }
      else
      {
         theText += (char)in.get();
         c = in.peek();
      }
   }while(!in.fail());
   
   return !in.fail();
}
#endif

bool rspfXmlNode::readTextContent(std::istream& in)
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfXmlNode::readTextContent: entered ..." << std::endl;
   }
   
   //---
   // Parse the text string.  Do it with no peeks, ignores, or putbacks as
   // those seem to have issues on Windows (vs9).
   //---
   bool result = false;

   theText = "";
   theCDataFlag = false;
   
   xmlskipws(in);

   if ( !in.fail() )
   {
      std::streampos initialPos = in.tellg();

      char c = in.get();
      
      if ( c != '<' )
      {
         do // Get the text up to the next '<'.
         {
            theText += c;
            c = in.get();
         } while ( (c != '<') && !in.fail() );
         
         in.unget(); // Put '<' back.
         result = !in.fail();
      }
      else // At "<" see if comment
      {
         c = in.get();

         if ( c != '!' )
         {
            in.seekg(initialPos);
            result = !in.fail();
         }
         else // at "<!"
         {
            c = in.get();
            if ( c == '-' )
            {
               // Comment section: <!-- some comment -->
               c = in.get();
               if ( c == '-' ) // at "<!--"
               {
                  // Strip comment
                  while( !in.fail() ) // continue until we see a --> pattern
                  {
                     c = in.get();
                     if(c == '-')
                     {
                        c = in.get();
                        if(c == '-')
                        {
                           c = in.get();
                           if(c == '>')
                           {
                              result = !in.fail();
                              break;
                           }
                        }
                     }
                  }
               }
            }
            else if ( c == '[' ) // at "<!["
            {
               // CDATA section: <![CDATA[something-here]]>
               c = in.get();
               if ( c == 'C') // at "<![C:"
               {
                  c = in.get();
                  if ( c == 'D' )// at "<![CD"
                  {
                     c = in.get();
                     if ( c == 'A' ) // at "<![CDA"
                     {
                        c = in.get();
                        if ( c == 'T' ) // at "<![CDAT"
                        {
                           c = in.get();
                           if ( c == 'A' ) // at "<![CDATA"
                           {
                              c = in.get();
                              if ( c == '[' ) // at "<!CDATA["
                              {
                                 if (readCDataContent(in))
                                 {
                                    theCDataFlag = true;
                                    result = true;
                                 }
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "theText: " << theText
         << "\ntheCDataFlag: " << (theCDataFlag?"true":"false")
         << "\nexit status: " << (result?"true":"false")
         << "\nrspfXmlNode::readTextContent: leaving ..." << std::endl;
   }
   
   return result;
}

bool rspfXmlNode::readEndTag(std::istream& in,
                              rspfString& endTag)
{
   bool result = false;
   char c = in.peek();
   endTag = "";
   
   if(theTag == "--")// this is a comment tag
   {
      skipCommentTag(in);
      endTag = "--";
      return (!in.fail());
   }
   // check end tag
   //
   if(c == '/')
   {
      in.ignore();
      readTag(in, endTag);
      if(in.fail()) return false;
      xmlskipws(in);
      c = in.peek();
      result = true;
   }
   else
   {
      return false;
   }
   if(c != '>')
   {
      setErrorStatus();
      return false;
   }
   else
   {
      in.ignore(1);
   }
   if(in.fail()) result = false;
   
   return result;
}
