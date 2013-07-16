//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Mingjie Su
//
// Description: Rpf Info object.
// 
//----------------------------------------------------------------------------
// $Id: rspfRpfInfo.cpp 1237 2010-08-05 19:50:27Z ming.su $

//rspf includes
#include <rspf/base/rspfTrace.h>
#include <rspf/support_data/rspfRpfInfo.h>
#include <rspf/support_data/rspfInfoFactory.h>

// Static trace for debugging
static rspfTrace traceDebug("rspfRpfInfo:debug");
static rspfTrace traceDump("rspfRpfInfo:dump"); // This will dump offsets.

rspfRpfInfo::rspfRpfInfo()
   : rspfInfoBase(),
     theFile(),
     m_infoFile()
{
}

rspfRpfInfo::~rspfRpfInfo()
{
}

bool rspfRpfInfo::open(const rspfFilename& file)
{
   theFile = file;
   if (isOpen())
   {
     std::ifstream in((theFile).c_str() );

     std::string line;
     int index = 0;
     while(in.good())
     {
       // Read in a line.
       std::getline(in, line);
       rspfString tmpStr = rspfString(line);
       if (index > 0)
       {
         if (!tmpStr.empty())
         {
           std::vector<rspfString> tmpVector = tmpStr.split("|");
           if (tmpVector.size() > 0)
           {
             m_infoFile = tmpVector[0];
             break;
           }
         }
       }
       index++;
     }
     in.close();

     return true;
   }
   return false;
}

bool rspfRpfInfo::isOpen()
{
  rspfString ext = theFile.ext().downcase();

  if(ext == "rpf")
  {
    return true;
  }
  else
  {
    return false;
  }
}

std::ostream& rspfRpfInfo::print(std::ostream& out) const
{
  rspfInfoBase* info = rspfInfoFactory::instance()->create(m_infoFile);
  if (info)
  {
    info->print(out);
  }

  return out;
}