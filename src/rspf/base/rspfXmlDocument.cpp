//*******************************************************************
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Oscar Kramer (rspf port by D. Burken)
//
// Description:  
//
// Contains definition of class rspfXmlDocument. This class provides read-only
// parsing and accessing of an XML document file.
//*****************************************************************************
// $Id: rspfXmlDocument.cpp 21579 2012-08-31 14:24:11Z dburken $


#include <rspf/base/rspfXmlDocument.h>
#include <rspf/base/rspfXmlAttribute.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfRegExp.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfKeywordNames.h>
#include <stack>
#include <iostream>
#include <fstream>


// Static trace for debugging
#include <rspf/base/rspfTrace.h>
static rspfTrace traceDebug("rspfXmlDocument:debug");

static std::istream& xmlskipws(std::istream& in)
{
   int c = in.peek();
   while((!in.fail())&&
         (( (c == ' ') || (c == '\t') || (c == '\n')|| (c == '\r') || (c<0x20) || (c>=0x7f) )))//|| (c<0x20) || (c >=0x2f) )))
   {
      in.ignore(1);
      c = in.peek();
   }
   
   return in;
}

static const int BUFFER_MAX_LEN = 1000;
static const rspfString XPATH_DELIM ("/");

RTTI_DEF1(rspfXmlDocument, "rspfXmlDocument", rspfObject)
rspfXmlDocument::rspfXmlDocument(const rspfFilename& xmlFileName)
   :
   theRootNode  (0),
   theXmlHeader("<?xml version='1.0'?>"),
   theStrictCheckFlag(false)
{

   if(xmlFileName != "")
   {
      openFile(xmlFileName);
   }
}
rspfXmlDocument::rspfXmlDocument(const rspfXmlDocument& src)
:rspfObject(src),
theRootNode(src.theRootNode.valid()?(rspfXmlNode*)src.theRootNode->dup():(rspfXmlNode*)0),
theXmlHeader(src.theXmlHeader),
theFilename(src.theFilename),
theStrictCheckFlag(src.theStrictCheckFlag)
{
   
}

rspfXmlDocument::~rspfXmlDocument()
{
}

bool rspfXmlDocument::write(const rspfFilename& file)
{
   std::ofstream out(file.c_str());

   if(out)
   {
      out << *this << std::endl;
   }
   else
   {
      return false;
   }

   return true;
}

bool rspfXmlDocument::openFile(const rspfFilename& filename)
{
   
   theFilename = filename;

   if(theFilename == "")
   {
      return false;
   }

   //
   // Open XML File:
   // Note: Opening text document binary to overcome an apparent windows bug.
   //
   ifstream xml_stream (filename.c_str(), ios::binary);
   if (!xml_stream)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "DEBUG: rspfXmlDocument::rspfXmlDocument\n"
            << "encountered opening file <" << filename << "> for "
            << "reading. Aborting..." << endl;
      }
      return false;
   }

   return read(xml_stream);
}

bool rspfXmlDocument::read(std::istream& in)
{
//   char buffer[BUFFER_MAX_LEN];
//   streampos file_pos;
//   bool readingHeader = true;
   bool startTagCharacterFound = false;
   char c = in.peek();


   // initially we will do our own skipping to make sure we ar not binary
   //
   while(!in.bad()&&
         (c != '<')&&
         (c >= 0x20)&&(c <= 0x7e))
   {
      in.ignore(1);
      c = in.peek();
   }

   if (in.bad()||
       (c!='<'))
   {
      setErrorStatus();
      return false;
   }
   startTagCharacterFound = true;

   if(readHeader(in))
   {
      if(theXmlHeader=="")
      {
         if(startTagCharacterFound)
         {
            theXmlHeader = "<?xml version='1.0'?>";
         }
         else
         {
            setErrorStatus();
            return false;
         }
      }
   }
   if((!theXmlHeader.contains("xml version"))&&
      (!startTagCharacterFound)&&theStrictCheckFlag)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "FATAL: rspfXmlDocument::rspfXmlDocument"
            << "encountered parsing XML file <" << theFilename
            << ">. The file does not appear to be XML v1.0. \n"
            << "Header = \n" << theXmlHeader <<"\n"
            << endl;
      }
      setErrorStatus();
      return false;
   }
   theRootNode = new rspfXmlNode(in, 0);
   setErrorStatus(theRootNode->getErrorStatus());
   return (getErrorStatus()==rspfErrorCodes::RSPF_OK);
}

void rspfXmlDocument::findNodes(const rspfString& arg_xpath,
                            vector<rspfRefPtr<rspfXmlNode> >& result) const
{
   //
   // First verify the root node exists:
   //
   if (!theRootNode.valid())
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "WARNING: rspfXmlDocument::findNodes,\n"
            << "No root node has been instantiated. Returning null "
            << "node list..." << endl;
      }
      return;
   }

   //
   // Make a copy to manipulate:
   //
   rspfString xpath (arg_xpath);
   if (xpath.empty())
      return;
   
   //
   // Check if absolute path:
   //
   if (xpath[static_cast<std::string::size_type>(0)] !=
       XPATH_DELIM[static_cast<std::string::size_type>(0)])
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "WARNING: rspfXmlDocument::findNodes\n"
            << "Only absolute XPaths are supported. Returning null "
            << "node list..." << endl;
      }
      return;
   }

   //
   // Check that root tag matches path root:
   //
   rspfString rel_xpath (xpath.after(XPATH_DELIM));
   rspfString root_tag (rel_xpath);
   if (root_tag.contains(XPATH_DELIM))
       root_tag = rel_xpath.before(XPATH_DELIM);
   
   if (root_tag != theRootNode->getTag())
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "WARNING: rspfXmlDocument::findNodes\n"
            << "XPath's root node <"<<root_tag<<"> does not match the "
            << "stored root node's tag <" << theRootNode->getTag() << ">. "
            << "Returning null node list..." << endl;
      }
      return;
   }

   //
   // If the root node was the requested node, return it alone:
   //
   rel_xpath = rel_xpath.after(XPATH_DELIM);
   if (rel_xpath.empty())
   {
      result.push_back(theRootNode);
      return;
   }
   
   //
   // Pass the node request on to the root node with the relative path:
   //
   theRootNode->findChildNodes(rel_xpath, result);
}

ostream& operator << (ostream& os, const rspfXmlDocument& xml_doc) 
{
   os << xml_doc.theXmlHeader << endl;
   if (xml_doc.theRootNode.valid())
   {
      os << (xml_doc.theRootNode.get()) << endl;
   }
//    else
//       os << "-- no root node assigned -- " << endl;
   
   return os;
}

void rspfXmlDocument::initRoot(rspfRefPtr<rspfXmlNode> node)
{
   theRootNode = node;
}

rspfRefPtr<rspfXmlNode> rspfXmlDocument::getRoot()
{
   return theRootNode;
}

const rspfRefPtr<rspfXmlNode> rspfXmlDocument::getRoot()const
{
   return theRootNode;
}

rspfRefPtr<rspfXmlNode> rspfXmlDocument::removeRoot()
{
   rspfRefPtr<rspfXmlNode> root = theRootNode;

   theRootNode = 0;

   return root;
}


void rspfXmlDocument::fromKwl(const rspfKeywordlist& kwlToConvert)
{

   rspfKeywordlist kwl = kwlToConvert;
   theRootNode = 0;
   theRootNode = new rspfXmlNode;

   
   rspfString prefix = "";
   rspfKeywordlist::KeywordMap& map              = kwl.getMap();
   rspfKeywordlist::KeywordMap::iterator mapIter = map.begin();

   while(mapIter != map.end())
   {
      rspfString key = mapIter->first;
      key = key.substitute(".", "/", true);
      theRootNode->addNode(key, mapIter->second);
      ++mapIter;
   }

   // now collapse all keywordlist styles to the XML style
   //
   std::stack<rspfRefPtr<rspfXmlNode> > tempStack;
   tempStack.push(theRootNode);
   while(!tempStack.empty())
   {
      
      rspfRefPtr<rspfXmlNode> node = tempStack.top();
      tempStack.pop();
      vector<rspfRefPtr<rspfXmlNode> >& childNodes = node->getChildNodes();
      rspf_uint32 idx = 0;
      for(idx = 0; idx < childNodes.size(); ++idx)
      {
         tempStack.push(childNodes[idx]);
      }
      if(node->getChildNodes().size() > 0)
      {
         if(node->getTag()!="")
         {
            node->addAttribute("name", node->getTag());
         }
         node->setTag("object");
      }
      
      if(node->getTag() == "type")
      {
         if(node->getParentNode())
         {
            node->getParentNode()->removeChild("type");
            node->getParentNode()->addAttribute("type", node->getText());
         }
      }
      else if(node->getChildNodes().size() < 1)
      {
         if(node->getTag()!="")
         {
            node->addAttribute("name", node->getTag());
         }
         if(!node->getText().contains("\n"))
         {
            if(node->getText()!="")
            {
               node->addAttribute("value", node->getText());
               node->setText("");
            }
         }
         node->setTag("property");
      }
   }
}

void rspfXmlDocument::toKwl(rspfKeywordlist& kwl ,
                             const rspfString& prefix)const
{
   if(theRootNode.valid())
   {
      theRootNode->toKwl(kwl, prefix);
   }
//    const std::vector<rspfRefPtr<rspfXmlNode> >& children = theRootNode->getChildNodes();
   
//    rspf_uint32 idx = 0;
   
//    for(idx = 0; idx < children.size(); ++idx)
//    {
//       children[idx]->toKwl(kwl, prefix);
//    }
}

bool rspfXmlDocument::readHeader(std::istream& in)
{
   char c;
   in>>xmlskipws;

   while(in.peek() == '<')
   {
      std::stack<char> theLessThanStack;
      theLessThanStack.push('<');
      in.ignore(1);
      c = in.peek();
      // we will for now skip things like !DOCTYPE and any other things in the header of the document that start with <? or <!
      if((c == '?')||
         (c == '!'))
      {
         theXmlHeader += "<";
         theXmlHeader += (char)in.get();
         
         while(!theLessThanStack.empty()&&
               (!in.bad()))
         {
            if(in.peek() == '<')
            {
               theLessThanStack.push('<');
            }
            else if(in.peek() == '>')
            {
               theLessThanStack.pop();
            }
            theXmlHeader += (char)in.get();
         }
         if(!in.bad())
         {
            if(in.peek()=='\n'||
               in.peek()=='\r')
            {
               theXmlHeader += (char)in.get();
            }
         }
//          if(!in.bad())
//          {
//             theXmlHeader += (char)in.get();
//          }
         in>>xmlskipws;
      }
   }

   return (!in.bad());
}
