//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
//
// Contains class declaration for rspfScalarTypeLut.
// 
//*******************************************************************
//  $Id: rspfScalarTypeLut.h 11914 2007-10-28 18:28:18Z dburken $

#ifndef rspfScalarTypeLut_HEADER
#define rspfScalarTypeLut_HEADER

#include <rspf/base/rspfLookUpTable.h>
#include <rspf/base/rspfKeywordNames.h>

/**
 * Used to convert from scalar type enumeration to strings and vice versa.
 * See rspfConstants.h for a complete list of enumerations.  This class is
 * a singleton, only one of them so all callers must go through the
 * instance method like:
 * rspfScalarTypeLut::instance()->getEntryString(scalar);
 */
class RSPFDLLEXPORT rspfScalarTypeLut : public rspfLookUpTable
{
public:

   /**
    * Returns the static instance of an rspfScalarTypeLut object.
    */
   static rspfScalarTypeLut* instance();

   virtual ~rspfScalarTypeLut();

   rspfScalarType getScalarTypeFromString(const rspfString& s) const;
   
   virtual rspfKeyword getKeyword() const;

protected:
   rspfScalarTypeLut();

private:

   static rspfScalarTypeLut* theInstance;
};

#endif
