//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
// Description:
//
// Thie file contains the rspfEllipsoidFactory.
//*******************************************************************
//  $Id: rspfEllipsoidFactory.h 19682 2011-05-31 14:21:20Z dburken $

#ifndef rspfEllipsoidFactory_HEADER
#define rspfEllipsoidFactory_HEADER

#include <rspf/base/rspfConstants.h>
#include <map>
#include <string>

class rspfEllipsoid;
class rspfString;
class rspfWgs84Ellipsoid;
class rspfWgs72Ellipsoid;
class rspfBJ54Ellipsoid;
class rspfXiAn80Ellipsoid;

class RSPFDLLEXPORT rspfEllipsoidFactory
{
public:
   virtual ~rspfEllipsoidFactory();
   const rspfEllipsoid* create(const rspfString &code)const;
   const rspfEllipsoid* wgs84()const{return theWgs84Ellipsoid;}
   const rspfEllipsoid* wgs72()const{return theWgs72Ellipsoid;}
   const rspfEllipsoid* bj54()const{return theBj54Ellipsoid;}
   const rspfEllipsoid* xian80()const{return theXiAn80Ellipsoid;}
   //! Given an alpha code (for example "WE" for WGS84), returns the corresponding EPSG code or 0
   //! if not found.
   rspf_uint32 findEpsgCode(const rspfString &alpha_code) const;

   static rspfEllipsoidFactory* instance();
   
private:
   typedef std::map<std::string, rspfEllipsoid*> TableType;
   typedef std::map<rspf_uint32, std::string> EpsgTableType;

   static rspfEllipsoidFactory*     theInstance;
   rspfEllipsoid*       theWgs84Ellipsoid;
   rspfEllipsoid*       theWgs72Ellipsoid;
   rspfEllipsoid*       theBj54Ellipsoid;
   rspfEllipsoid*       theXiAn80Ellipsoid;
   TableType             theEllipsoidTable;
   EpsgTableType         theEpsgTable;
   rspfEllipsoidFactory();
   void initializeTable();
   void deleteAll();
};

#endif
