//----------------------------------------------------------------------------
// File:     rspfRpfComponentIdLut.h
// 
// License:  See top level LICENSE.txt file.
//
// Author:   David Burken
//
// Description: See class description.
// 
//----------------------------------------------------------------------------
// $Id: rspfRpfComponentIdLut.h 20324 2011-12-06 22:25:23Z dburken $

#ifndef rspfRpfComponentIdLut_HEADER
#define rspfRpfComponentIdLut_HEADER 1

#include <rspf/base/rspfLookUpTable.h>
#include <rspf/support_data/rspfRpfConstants.h>

/**
 * @class rspfRpfComponentIdLut
 *
 * @brief Lookup table for RPF section/component ID's.
 * 
 * Used to convert from RPF section/component ID's to strings and vice versa.
 * See MIL-STD-2411-1, section 5.1.1 for detailed information. Defines located
 * in rspfRpfConstants.h.  This class is a singleton, only one of them so all
 * callers must go through the instance method like:
 * rspfRpfComponentIdLut::instance()->getEntryString(id);
 */
class RSPFDLLEXPORT rspfRpfComponentIdLut : public rspfLookUpTable
{
public:

   /** @return The static instance of an rspfRpfComponentIdLut object. */
   static rspfRpfComponentIdLut* instance();

   /** @brief destructor */
   virtual ~rspfRpfComponentIdLut();

   /** @return Keyword: ("rpf_component_id", "") */
   virtual rspfKeyword getKeyword() const;
   
private:
   /** @brief Hidden from use constructor. */
   rspfRpfComponentIdLut();

   /** @brief The single instance of this class. */
   static rspfRpfComponentIdLut* theInstance;
};

#endif /* #ifndef rspfRpfComponentIdLut_HEADER */
