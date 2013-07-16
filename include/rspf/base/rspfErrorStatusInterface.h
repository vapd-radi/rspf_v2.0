//*******************************************************************
// Copyright (C) 2002 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: David Burken
//
// Description:
// 
// Contains class declaration for common error interfaces.
//
//*************************************************************************
// $Id: rspfErrorStatusInterface.h 22149 2013-02-11 21:36:10Z dburken $

#ifndef rspfErrorStatusInterface_HEADER
#define rspfErrorStatusInterface_HEADER

#include <iosfwd>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfRtti.h>
#include <rspf/base/rspfErrorCodes.h>

/*!
 *  class rspfErrorStatusInterface
 *  Provides common interfaces for error handling.
 *
 *  Note: For error codes and associated strings use the rspfErrorCodes class.
 */
class RSPFDLLEXPORT rspfErrorStatusInterface
{
public:
   rspfErrorStatusInterface();
   
   virtual ~rspfErrorStatusInterface();

   /*!  return theErrorStatus */
   virtual rspfErrorCode getErrorStatus() const;

   /*!  return theErrorStatus as an rspfString */
   virtual rspfString getErrorStatusString() const;

   /*! Sets theErrorStatus to error_status. */
   virtual void  setErrorStatus(rspfErrorCode error_status) const;

   /*! Set theErrorStatus to rspfErrorCodes::RSPF_ERROR. */
   virtual void  setErrorStatus() const;

   /*! Clears theErrorStatus by setting to rspfErrorCodes::RSPF_OK or 0. */
   virtual void  clearErrorStatus() const;

   /*! @return true if theErrorStatus != rspfErrorCodes::RSPF_OK. */
   bool hasError() const;

   /**
    * Outputs theErrorStatus as an rspfErrorCode and an rspfString.
    * 
    * @return std::ostream&
    *
    * @note  Derived classes should only have to implement a virtual print,
    *        not an operator<< funtion as it's implemented here.
    */
   virtual std::ostream& print(std::ostream& out) const;

   friend RSPFDLLEXPORT std::ostream& operator<<(std::ostream& out,
                                                  const rspfErrorStatusInterface& obj);
   
protected:
   mutable rspfErrorCode theErrorStatus;
   
TYPE_DATA
};

#endif
