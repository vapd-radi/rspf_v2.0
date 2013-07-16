//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// AUTHOR: Oscar Kramer (okramer@imagelinks.com)
//
// DESCRIPTION: Contains declaration of class rspfGridRemapEngineFactory
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfGridRemapEngineFactory.h 9968 2006-11-29 14:01:53Z gpotts $

#ifndef rspfGridRemapEngineFactory_HEADER
#define rspfGridRemapEngineFactory_HEADER

class rspfGridRemapEngine;

/*!****************************************************************************
 *
 * CLASS:  rspfGridRemapEngineFactory (SIMPLETON)
 *
 *****************************************************************************/
class rspfGridRemapEngineFactory
{
public:
   /*!
    * Attempts to create an instance of the Product given a Specifier. Returns
    * successfully constructed product or NULL.
    */
   static rspfGridRemapEngine* create(const char* spec);
   
protected:
   rspfGridRemapEngineFactory() {}
};

#endif
