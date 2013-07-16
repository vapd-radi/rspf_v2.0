//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: David Burken  (dburken@imagelinks.com)
//
// Description:
// 
// Contains class definition for common error interfaces.
//
//*************************************************************************
// $Id: rspfErrorStatusInterface.cpp 22149 2013-02-11 21:36:10Z dburken $

#include <iostream>
#include <rspf/base/rspfErrorStatusInterface.h>
#include <rspf/base/rspfString.h>

RTTI_DEF(rspfErrorStatusInterface, "rspfErrorStatusInterface");

rspfErrorStatusInterface::rspfErrorStatusInterface()
   :
   theErrorStatus(rspfErrorCodes::RSPF_OK)
{}
   
rspfErrorStatusInterface::~rspfErrorStatusInterface()
{}

rspfErrorCode rspfErrorStatusInterface::getErrorStatus() const
{
   return theErrorStatus;
}

rspfString rspfErrorStatusInterface::getErrorStatusString() const
{
   return rspfErrorCodes::instance()->getErrorString(theErrorStatus);
}

void rspfErrorStatusInterface::setErrorStatus(rspfErrorCode error_status) const
{
   theErrorStatus = error_status;
}

void  rspfErrorStatusInterface::setErrorStatus() const
{
   theErrorStatus = rspfErrorCodes::RSPF_ERROR;
}

void  rspfErrorStatusInterface::clearErrorStatus() const
{
   theErrorStatus = rspfErrorCodes::RSPF_OK;
}

bool rspfErrorStatusInterface::hasError() const
{
   return ( theErrorStatus != rspfErrorCodes::RSPF_OK );
}

std::ostream& rspfErrorStatusInterface::print(std::ostream& out) const
{
   out << "rspfErrorStatusInterface::print"
       << "\ntheErrorStatus:         " << theErrorStatus
       << "\ntheErrorStatus string:  " << getErrorStatusString()
       << std::endl;
   return out;
}

std::ostream& operator<<(std::ostream& out,
                         const rspfErrorStatusInterface& obj)
{
   return obj.print(out);
}
