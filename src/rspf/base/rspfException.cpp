//----------------------------------------------------------------------------
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:  Generic RSPF Exception that is a std::exception with a
// what() method implemented.
//----------------------------------------------------------------------------
// $Id: rspfException.cpp 10029 2006-12-04 23:23:11Z dburken $

#include <rspf/base/rspfException.h>

rspfException::rspfException() throw()
   : theErrorMessage()
{
}

rspfException::rspfException(const std::string& errorMessage) throw()
   : theErrorMessage(errorMessage)
{
}

rspfException::~rspfException() throw()
{
}

const char* rspfException::what() const throw()
{
   return theErrorMessage.c_str();
}
