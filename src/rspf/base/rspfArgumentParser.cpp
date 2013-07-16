//-------------------------------------------------------------------------
//
// This code was taken from Open Scene Graph and incorporated from into
// RSPF.
//
//-------------------------------------------------------------------------
// $Id: rspfArgumentParser.cpp 19900 2011-08-04 14:19:57Z dburken $

#include <cstring>
#include <set>
#include <iostream>

#include <rspf/base/rspfArgumentParser.h>
#include <rspf/base/rspfApplicationUsage.h>
#include <rspf/base/rspfString.h>

bool rspfArgumentParser::isOption(const char* str)
{
    return str && str[0]=='-';
}

bool rspfArgumentParser::isString(const char* str)
{
    if (!str) return false;

    return true;
//    return !isOption(str);
}

bool rspfArgumentParser::isNumber(const char* str)
{
   if (!str) return false;
   
   bool hadPlusMinus = false;
    bool hadDecimalPlace = false;
    bool hadExponent = false;
    bool couldBeInt = true;
    bool couldBeFloat = true;
    int noZeroToNine = 0;

    const char* ptr = str;

    // check if could be a hex number.
    if (std::strncmp(ptr,"0x",2)==0)
    {
        // skip over leading 0x, and then go through rest of string
        // checking to make sure all values are 0...9 or a..f.
        ptr+=2;
        while (
               *ptr!=0 &&
               ((*ptr>='0' && *ptr<='9') ||
                (*ptr>='a' && *ptr<='f') ||
                (*ptr>='A' && *ptr<='F'))
              )
        {
            ++ptr;
        }

        // got to end of string without failure, therefore must be a hex integer.
        if (*ptr==0) return true;
    }

    ptr = str;

    // check if a float or an int.
    while (*ptr!=0 && couldBeFloat)
    {
       if (*ptr=='+' || *ptr=='-')
       {
          if (hadPlusMinus)
          {
             couldBeInt = false;
             couldBeFloat = false;
          }
          else
          {
             hadPlusMinus = true;
          }
       }
       else if (*ptr>='0' && *ptr<='9')
       {
          noZeroToNine++;
       }
       else if (*ptr=='.')
       {
          if (hadDecimalPlace)
          {
             couldBeInt = false;
             couldBeFloat = false;
          }
          else
          {
             hadDecimalPlace = true;
             couldBeInt = false;
          }
       }
       else if (*ptr=='e' || *ptr=='E')
       {
          if (hadExponent || noZeroToNine==0)
          {
             couldBeInt = false;
             couldBeFloat = false;
          }
          else
          {
             hadExponent = true;
             couldBeInt = false;
             hadDecimalPlace = false;
             hadPlusMinus = false;
             noZeroToNine=0;
          }
       }
       else
       {
          couldBeInt = false;
          couldBeFloat = false;
       }
       ++ptr;
    }

    if (couldBeInt && noZeroToNine>0) return true;
    if (couldBeFloat && noZeroToNine>0) return true;

    return false;

}

bool rspfArgumentParser::rspfParameter::valid(const char* str) const
{
    switch(theType)
    {
       case rspfParameter::RSPF_FLOAT_PARAMETER:        return isNumber(str);
       case rspfParameter::RSPF_DOUBLE_PARAMETER:       return isNumber(str);
       case rspfParameter::RSPF_INT_PARAMETER:          return isNumber(str);
       case rspfParameter::RSPF_UNSIGNED_INT_PARAMETER: return isNumber(str);
       case rspfParameter::RSPF_STRING_PARAMETER:       return isString(str);
    }
    return false;
}

bool rspfArgumentParser::rspfParameter::assign(const char* str)
{
    if (valid(str))
    {
        switch(theType)
        {
           case rspfParameter::RSPF_FLOAT_PARAMETER:        *theValue.theFloat = (float)rspfString(str).toDouble(); break;
           case rspfParameter::RSPF_DOUBLE_PARAMETER:       *theValue.theDouble = rspfString(str).toDouble(); break;
           case rspfParameter::RSPF_INT_PARAMETER:          *theValue.theInt = rspfString(str).toInt(); break;
           case rspfParameter::RSPF_UNSIGNED_INT_PARAMETER: *theValue.theUint = rspfString(str).toUInt32(); break;
           case rspfParameter::RSPF_STRING_PARAMETER:       *theValue.theString = str; break;
        }
        return true;
    }
    else
    {
        return false;
    }
}



rspfArgumentParser::rspfArgumentParser(int* argc,char **argv):
    theArgc(argc),
    theArgv(argv),
    theUsage(rspfApplicationUsage::instance())
{
}

rspfArgumentParser::~rspfArgumentParser()
{
}

void rspfArgumentParser::initialize(int* argc, const char **argv)
{
   if(argc > 0) delete (theArgv);
   theArgc = argc;
   theArgv = new char*[*argc];
   for(int i=0;i<*argc;i++)
   {
      theArgv[i] = new char[strlen(argv[i])];
      strcpy(theArgv[i], argv[i]);
   }
}

std::string rspfArgumentParser::getApplicationName() const
{
    if (theArgc && *theArgc>0 ) return std::string(theArgv[0]);
    return "";
}


bool rspfArgumentParser::isOption(int pos) const
{
    return pos<*theArgc && isOption(theArgv[pos]);
}

bool rspfArgumentParser::isString(int pos) const
{
    return pos < *theArgc && isString(theArgv[pos]);
}

bool rspfArgumentParser::isNumber(int pos) const
{
    return pos < *theArgc && isNumber(theArgv[pos]);
}


int rspfArgumentParser::find(const std::string& str) const
{
    for(int pos=1;pos<*theArgc;++pos)
    {
        if (str==theArgv[pos])
        {
            return pos;
        }
    }
    return 0;
}

bool rspfArgumentParser::match(int pos, const std::string& str) const
{
    return pos<*theArgc && str==theArgv[pos];
}


bool rspfArgumentParser::containsOptions() const
{
   for(int pos=1;pos<*theArgc;++pos)
   {
      if (isOption(pos)) return true;
   }
   return false;
}


int rspfArgumentParser::numberOfParams(const std::string& str, rspfParameter param) const
{
   int pos=find(str);
   if (pos<=0) 
      return -1;

   ++pos;
   int num_params = 0;
   while (param.valid(theArgv[pos+num_params]))
      ++num_params;
   return num_params;
}


void rspfArgumentParser::remove(int pos,int num)
{
    if (num==0) return;

    for(;pos+num<*theArgc;++pos)
    {
        theArgv[pos]=theArgv[pos+num];
    }
    for(;pos<*theArgc;++pos)
    {
        theArgv[pos]=0;
    }
    *theArgc-=num;
}

bool rspfArgumentParser::read(const std::string& str)
{
    int pos=find(str);
    if (pos<=0) return false;
    remove(pos);
    return true;
}

bool rspfArgumentParser::read(const std::string& str, rspfParameter value1)
{
    int pos=find(str);
    if (pos<=0) return false;
    if (!value1.valid(theArgv[pos+1]))
    {
        reportError("argument to `"+str+"` is missing");
        return false;
    }
    value1.assign(theArgv[pos+1]);
    remove(pos,2);
    return true;
}

bool rspfArgumentParser::read(const std::string& str, rspfParameter value1, rspfParameter value2)
{
    int pos=find(str);
    if (pos<=0) return false;
    if (!value1.valid(theArgv[pos+1]) ||
        !value2.valid(theArgv[pos+2]))
    {
        reportError("argument to `"+str+"` is missing");
        return false;
    }
    value1.assign(theArgv[pos+1]);
    value2.assign(theArgv[pos+2]);
    remove(pos,3);
    return true;
}

bool rspfArgumentParser::read(const std::string& str, rspfParameter value1, rspfParameter value2, rspfParameter value3)
{
    int pos=find(str);
    if (pos<=0) return false;
    if (!value1.valid(theArgv[pos+1]) ||
        !value2.valid(theArgv[pos+2]) ||
        !value2.valid(theArgv[pos+3]))
    {
        reportError("argument to `"+str+"` is missing");
        return false;
    }
    value1.assign(theArgv[pos+1]);
    value2.assign(theArgv[pos+2]);
    value3.assign(theArgv[pos+3]);
    remove(pos,4);
    return true;
}

bool rspfArgumentParser::read(const std::string& str, rspfParameter value1, rspfParameter value2, rspfParameter value3, rspfParameter value4)
{
    int pos=find(str);
    if (pos<=0) return false;
    if (!value1.valid(theArgv[pos+1]) ||
        !value2.valid(theArgv[pos+2]) ||
        !value2.valid(theArgv[pos+3]) ||
        !value3.valid(theArgv[pos+4]))
    {
        reportError("argument to `"+str+"` is missing");
        return false;
    }
    value1.assign(theArgv[pos+1]);
    value2.assign(theArgv[pos+2]);
    value3.assign(theArgv[pos+3]);
    value4.assign(theArgv[pos+4]);
    remove(pos,5);
    return true;
}


/** if the argument value at the posotion pos matches specified string, and subsequent
  * parameters are also matched then set the paramter values and remove the from the list of arguments.*/
bool rspfArgumentParser::read(int pos, const std::string& str)
{
    if (match(pos,str))
    {
        remove(pos,1);
        return true;
    }
    else
    {
        return false;
    }
}

bool rspfArgumentParser::read(int pos, const std::string& str, rspfParameter value1)
{
    if (match(pos,str) &&
        value1.valid(theArgv[pos+1]))
    {
        value1.assign(theArgv[pos+1]);
        remove(pos,2);
        return true;
    }
    else
    {
        return false;
    }
}

bool rspfArgumentParser::read(int pos, const std::string& str, rspfParameter value1, rspfParameter value2)
{
    if (match(pos,str) &&
        value1.valid(theArgv[pos+1]) &&
        value2.valid(theArgv[pos+2]))
    {
        value1.assign(theArgv[pos+1]);
        value2.assign(theArgv[pos+2]);
        remove(pos,3);
        return true;
    }
    else
    {
        return false;
    }
}

bool rspfArgumentParser::read(int pos, const std::string& str, rspfParameter value1, rspfParameter value2, rspfParameter value3)
{
    if (match(pos,str) &&
        value1.valid(theArgv[pos+1]) &&
        value2.valid(theArgv[pos+2]) &&
        value3.valid(theArgv[pos+3]))
    {
        value1.assign(theArgv[pos+1]);
        value2.assign(theArgv[pos+2]);
        value3.assign(theArgv[pos+3]);
        remove(pos,4);
        return true;
    }
    else
    {
        return false;
    }
}

bool rspfArgumentParser::read(int pos, const std::string& str, rspfParameter value1, rspfParameter value2, rspfParameter value3, rspfParameter value4)
{
    if (match(pos,str) &&
        value1.valid(theArgv[pos+1]) &&
        value2.valid(theArgv[pos+2]) &&
        value3.valid(theArgv[pos+3]) &&
        value4.valid(theArgv[pos+4]))
    {
        value1.assign(theArgv[pos+1]);
        value2.assign(theArgv[pos+2]);
        value3.assign(theArgv[pos+3]);
        value4.assign(theArgv[pos+4]);
        remove(pos,5);
        return true;
    }
    else
    {
        return false;
    }
}



bool rspfArgumentParser::errors(rspfErrorSeverity severity) const
{
    for(rspfErrorMessageMap::const_iterator itr=theErrorMessageMap.begin();
        itr!=theErrorMessageMap.end();
        ++itr)
    {
        if (itr->second>=severity) return true;
    }
    return false;
}

void rspfArgumentParser::reportError(const std::string& message, rspfErrorSeverity severity)
{
    theErrorMessageMap[message]=severity;
}

void rspfArgumentParser::reportRemainingOptionsAsUnrecognized(rspfErrorSeverity severity)
{
    std::set<std::string> options;
    if (theUsage)
    {
        // parse the usage options to get all the option that the application can potential handle.
        for(rspfApplicationUsage::UsageMap::const_iterator itr=theUsage->getCommandLineOptions().begin();
            itr!=theUsage->getCommandLineOptions().end();
            ++itr)
        {
            const std::string& option = itr->first;
            std::string::size_type prevpos = 0, pos = 0;
            while ((pos=option.find(' ',prevpos))!=std::string::npos)
            {
                if (option[prevpos]=='-')
                {
                    options.insert(std::string(option,prevpos,pos-prevpos));
                }
                prevpos=pos+1;
            }
            if (option[prevpos]=='-')
            {

                options.insert(std::string(option,prevpos,std::string::npos));
            }
        }

    }

    for(int pos=1;pos<argc();++pos)
    {
        // if an option and havn't been previous querried for report as unrecognized.
        if (isOption(pos) && options.find(theArgv[pos])==options.end())
        {
            reportError(getApplicationName() +": unrecognized option "+theArgv[pos],severity);
        }
    }
}
void rspfArgumentParser::writeErrorMessages(std::ostream& output, rspfErrorSeverity severity)
{
    for(rspfErrorMessageMap::iterator itr=theErrorMessageMap.begin();
        itr!=theErrorMessageMap.end();
        ++itr)
    {
        if (itr->second>=severity)
        {
            output<< getApplicationName() << ": " << itr->first << std::endl;
        }
    }
}
