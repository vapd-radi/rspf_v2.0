#include <rspf/base/rspfCsvFile.h>
#include <iostream>
#include <iterator>
static const rspf_uint32 DEFAULT_BUFFER_SIZE = 1024;

rspf_int32 rspfCsvFile::INVALID_INDEX = -1;
static std::istream& csvSkipWhiteSpace(std::istream& in)
{
   int c = in.peek();
   while(!in.fail()&& ( (c == ' ')||(c == '\t')||(c == '\n')||(c == '\r') ) )
   {
      in.ignore(1);
      c = in.peek();
   }
   
   return in;
}

rspfCsvFile::Record::Record(rspfCsvFile* csvFile)
:theCsvFile(csvFile)
{
}

bool rspfCsvFile::Record::valueAt(const rspfString& fieldName,
                                   rspfString& value)const
{
   bool result = false;
   if(theCsvFile)
   {
      rspf_int32 idx = theCsvFile->indexOfField(fieldName);
      if((idx > 0)&&(idx < (rspf_int32)theValues.size()))
      {
         value = theValues[idx];
         result = true;
      }
   }
   
   return result;
}

bool rspfCsvFile::Record::valueAt(rspf_uint32 idx,
                                   rspfString& value)const
{
   bool result = false;
   if(idx < theValues.size())
   {
      value = theValues[idx];
      result = true;
   }
   
   return result;
}

rspfString& rspfCsvFile::Record::operator [](const rspfString& fieldName)
{
   if(theCsvFile)
   {
      rspf_int32 idx = theCsvFile->indexOfField(fieldName);
      if((idx >= 0)&&(idx < (rspf_int32)theValues.size()))
      {
         return theValues[idx];
      }
   }      
   
   return theDummyValue;
}

const rspfString& rspfCsvFile::Record::operator [](const rspfString& fieldName)const
{
   if(theCsvFile)
   {
      rspf_int32 idx = theCsvFile->indexOfField(fieldName);
      if((idx >= 0)&&(idx < (rspf_int32)theValues.size()))
      {
         return theValues[idx];
      }
   }      
   
   return theDummyValue;
}

rspfString& rspfCsvFile::Record::operator [](rspf_uint32 idx)
{
   if(idx < theValues.size())
   {
      return theValues[idx];
   }
   
   return theDummyValue;
}

const rspfString& rspfCsvFile::Record::operator [](rspf_uint32 idx)const
{
   if(idx < theValues.size())
   {
      return theValues[idx];
   }
   
   return theDummyValue;
}

rspfCsvFile::rspfCsvFile(const rspfString& separatorList)
:theInputStream(0),
theSeparatorList(separatorList),
 theOpenFlag(false)
{
}

rspfCsvFile::~rspfCsvFile()
{
   close();
}

bool rspfCsvFile::readCsvLine(std::istream& inStream,
                               rspfCsvFile::StringListType& tokens)const
{
   tokens.clear();
   bool done = false;
   char c;
   const char quote = '\"';
   bool inQuotedString = false;
   bool inDoubleQuote    = false;
   rspfString currentToken;
   inStream >> csvSkipWhiteSpace;
   while(!done&&inStream.get(c)&&inStream.good())
   {
      if(c > 0x7e )
      {
         return false;
      }
      if((c!='\n')&&
         (c!='\r'))
      {
         // check to see if we are quoted and check to see if double quoted
         if(c == quote)
         {
            // check if at a double quote
            if(inStream.peek() == quote)
            {
               currentToken += quote;
               inStream.ignore(1);
               if(!inDoubleQuote)
               {
                  inDoubleQuote = true;
               }
               else
               {
                  inDoubleQuote = false;
               }
            }
            else
            {
               if(!inQuotedString)
               {
                  inQuotedString = true;
               }
               else
               {
                  inQuotedString = false;
               }
            }
         }
         // if we are at a separator then check to see if we are inside a quoted string
         else if(theSeparatorList.contains(c)) 
         {
            // ignore token separator if quoted
            if(inQuotedString||inDoubleQuote)
            {
               currentToken += c;
            }
            else
            {
               // push the current token.
               currentToken = currentToken.trim();
               tokens.push_back(currentToken);
               currentToken = "";
               inStream >> csvSkipWhiteSpace;
            }
         }
         else
         {
            currentToken += c;
         }
      }
      else if(!inQuotedString||inDoubleQuote)
      {
         currentToken = currentToken.trim();
         tokens.push_back(currentToken);
         done = true;
      }
      else
      {
         currentToken += c;
      }
   }
   
   return (inStream.good()&&(tokens.size()>0)); 
}

bool rspfCsvFile::open(const rspfFilename& file, const rspfString& flags)
{
   close();
   
   if((*flags.begin()) == 'r')
   {
      theInputStream = new std::ifstream(file.c_str(), std::ios::in|std::ios::binary);
      theOpenFlag = true;
      theRecordBuffer = new rspfCsvFile::Record(this);
   }
   else
   {
      return theOpenFlag;
   }
   
   return theOpenFlag;
}

bool rspfCsvFile::readHeader()
{
   if(theOpenFlag)
   {
      theFieldHeaderList.clear();
      return readCsvLine(*theInputStream, theFieldHeaderList);
   }
   
   return false;
}

void rspfCsvFile::close()
{
   if(theOpenFlag)
   {
      theFieldHeaderList.clear();
      if(theInputStream)
      {
         delete theInputStream;
         theInputStream = 0;
      }
      theOpenFlag = false;
      theRecordBuffer = 0;
   }
}
rspfRefPtr<rspfCsvFile::Record> rspfCsvFile::nextRecord()
{
   if(!theOpenFlag) return rspfRefPtr<rspfCsvFile::Record>(0);
   
   if(theFieldHeaderList.empty())
   {
      if(!readHeader())
      {
         return rspfRefPtr<rspfCsvFile::Record>();
      }
   }
   if(!readCsvLine(*theInputStream,  theRecordBuffer->values()))
   {
      return rspfRefPtr<rspfCsvFile::Record>();
   }
   return theRecordBuffer;
}

const rspfCsvFile::StringListType& rspfCsvFile::fieldHeaderList()const
{
   return theFieldHeaderList;
}

rspf_int32 rspfCsvFile::indexOfField(const rspfString& fieldName)const
{
   rspf_int32 result = rspfCsvFile::INVALID_INDEX;
   rspf_uint32 idx = 0;
   for(;idx < theFieldHeaderList.size(); ++idx)
   {
      if(theFieldHeaderList[idx] == fieldName)
      {
         result = (rspf_int32)idx;
         break;
      }
   }
   
   return result;
}
