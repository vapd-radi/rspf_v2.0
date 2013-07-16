//*******************************************************************
// Copyright (C) 2001 ImageLinks Inc.  All rights reserved.
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Oscar Kramer (rspf port by D. Burken)
//
// Description:  
//
// Contains definition of class rspfXmlAttribute.
// 
//*****************************************************************************
// $Id: rspfXmlAttribute.cpp 19682 2011-05-31 14:21:20Z dburken $

#include <iostream>
#include <sstream>

#include <rspf/base/rspfXmlAttribute.h>
#include <rspf/base/rspfNotifyContext.h>

RTTI_DEF2(rspfXmlAttribute, "rspfXmlAttribute", rspfObject, rspfErrorStatusInterface)

static std::istream& xmlskipws(std::istream& in)
{
   int c = in.peek();
   while((!in.fail())&&
         ((c == ' ') ||
          (c == '\t') ||
          (c == '\n')||
          (c == '\r')))
   {
      in.ignore(1);
      c = in.peek();
   }
   
   return in;
}

rspfXmlAttribute::rspfXmlAttribute(rspfString& spec)
{
   std::stringstream in(spec);

   read(in);
}

rspfXmlAttribute::rspfXmlAttribute(const rspfXmlAttribute& src)
   :theName(src.theName),
    theValue(src.theValue)
{
}

bool rspfXmlAttribute::read(std::istream& in)
{
   xmlskipws(in);
   if(in.fail()) return false;
   if(readName(in))
   {
      xmlskipws(in);
      if((in.peek() != '=')||
         (in.fail()))
      {
         setErrorStatus();
         return false;
      }
      in.ignore(1);
      if(readValue(in))
      {
         return true;
      }
      else
      {
         setErrorStatus();
         return false;
      }
   }
   return false;
   
#if 0   
   //
   // Pull out name:
   //
   theName = spec.before('=');
   theName = theName.trim();
   if (theName.empty())
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "rspfXmlAttribute::rspfXmlAttribute \n"
                                           << "Bad attribute format encountered near:\n\""<< spec<<"\"\n"
                                           << "Parsing aborted...\n";
      setErrorStatus();
      
      return;
   }
   spec = spec.after('=');

   //***
   // Read value:
   //***
   char quote_char = spec[0];
   spec = spec.after(quote_char);  // after first quote
   theValue = spec.before(quote_char); // before second quote

   //
   // Reposition attribute specification to the start of next attribute or end
   // of tag:
   //
   spec = spec.after(quote_char);  // after second quote
   rspfString next_entry ("-?[+0-9A-Za-z<>]+");
   spec = spec.fromRegExp(next_entry.c_str());
#endif
}

rspfXmlAttribute::~rspfXmlAttribute()
{
}

rspfXmlAttribute::rspfXmlAttribute()
{
}

rspfXmlAttribute::rspfXmlAttribute(const rspfString& name,
                                     const rspfString& value)
{
   setNameValue(name, value);
}

const rspfString& rspfXmlAttribute::getName()  const
{
   return theName;
}

const rspfString& rspfXmlAttribute::getValue() const
{
   return theValue;
}

void rspfXmlAttribute::setNameValue(const rspfString& name,
                                     const rspfString& value)
{
   theName  = name;
   theValue = value;
}

void rspfXmlAttribute::setName(const rspfString& name)
{
   theName = name;
}

void rspfXmlAttribute::setValue(const rspfString& value)
{
   theValue = value;
}

std::ostream& operator << (std::ostream& os, const rspfXmlAttribute* xml_attr) 
{
   os << " " << xml_attr->theName << "=\"" << xml_attr->theValue << "\"";

   return os;
}


bool rspfXmlAttribute::readName(std::istream& in)
{
   xmlskipws(in);
   theName = "";
   char c = in.peek();
   while((c != ' ')&&
         (c != '\n')&&
	 (c != '\r')&&
         (c != '\t')&&
         (c != '=')&&
         (c != '<')&&
         (c != '/')&&
         (c != '>')&&
         (!in.fail()))
   {
      theName += (char)in.get();
      c = in.peek();
   }

   return ((!in.fail())&&
           (theName != ""));
}

bool rspfXmlAttribute::readValue(std::istream& in)
{
   xmlskipws(in);
   if(in.fail()) return false;
   theValue = "";
   char c = in.peek();
   bool done = false;
	char startQuote = '\0';
   if((c == '\'')||
      (c == '"'))
   {
		startQuote = c;
      theValue += in.get();
      while(!done&&!in.fail())
      {
         c = in.peek();
         if(c==startQuote)
         {
            theValue += c;
            done = true;
            in.ignore(1);
            
         }
         else if(c == '\n')
         {
            done = true;
         }
         else
         {
            theValue += in.get();
         }
      }
   }

   if(theValue != "")
   {
      std::string::iterator startIter = theValue.begin();
      std::string::iterator endIter   = theValue.end();
      --endIter;
      if(*startIter == startQuote)
      {
         ++startIter;
      }
      else
      {
         return false;
         setErrorStatus();
      }
      if(*endIter != startQuote)
      {
         return false;
         setErrorStatus();
      }
      theValue = rspfString(startIter, endIter);
   }
   return ((!in.bad())&&
           (theValue !=""));
}
