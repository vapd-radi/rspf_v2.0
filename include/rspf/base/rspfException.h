//----------------------------------------------------------------------------
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:  Generic RSPF Exception that is a std::exception with a
// what() method implemented.
//----------------------------------------------------------------------------
// $Id: rspfException.h 10029 2006-12-04 23:23:11Z dburken $
#ifndef rspfException_HEADER
#define rspfException_HEADER

#include <exception>
#include <string>
#include <rspf/base/rspfConstants.h>

class RSPF_DLL rspfException : public std::exception
{
public:

   /** @brief default construction */
   rspfException() throw();

   /**
    * @brief construction that takes an error string.
    * @param errorMessage The error message.
    */
   rspfException(const std::string& errorMessage) throw();

   /** @brief virtual destructor. */
   virtual ~rspfException() throw();

   /**
    * @brief Returns the error message.
    * @return The error message as a C-style character string.
    */
   virtual const char* what() const throw();

private:

   /** This is the error message returned by what(). */
   std::string theErrorMessage;
};

#endif /* End of #ifndef rspfException_HEADER */
