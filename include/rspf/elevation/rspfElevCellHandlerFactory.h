//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// DESCRIPTION:
//   Contains declaration of class rspfElevCellHandlerFactory. This factory
//   class instantiates the proper rspfElevCellHandler given a file name.
//
// SOFTWARE HISTORY:
//>
//   26Apr2001  Oscar Kramer (okramer@imagelinks.com)
//              Initial coding.
//<
//*****************************************************************************

#ifndef rspfElevCellHandlerFactory_HEADER
#define rspfElevCellHandlerFactory_HEADER

#include <list>
#include <rspf/base/rspfFactoryBaseTemplate.h>

class rspfElevCellHandler;
class rspfGpt;

/*!****************************************************************************
 *
 * CLASS:  rspfElevCellHandlerFactory
 *
 *****************************************************************************/
class rspfElevCellHandlerFactory :
   public rspfFactoryBase <rspfElevCellHandler>
{
 public:
   static rspfElevCellHandlerFactory* instance();
   
   virtual rspfElevCellHandler* create(const rspfString&) const;
   virtual rspfElevCellHandler* create(const rspfKeywordlist& kwl,
                                        const char* prefix) const;
   
   virtual std::list<rspfString> getList() const;
   
 protected:
   rspfElevCellHandlerFactory();

   static rspfElevCellHandlerFactory*  theInstance;
};

#endif
