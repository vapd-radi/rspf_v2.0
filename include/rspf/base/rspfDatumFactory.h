//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
// Description:
//
// This is the DatumFactory class.  You give it a code and it will
// construct a Datum class.  It returns NULL if no code was found.
//*******************************************************************
//  $Id: rspfDatumFactory.h 19682 2011-05-31 14:21:20Z dburken $

#ifndef rspfDatumFactory_HEADER
#define rspfDatumFactory_HEADER 1

#include <rspf/base/rspfDatumFactoryInterface.h>
#include <rspf/base/rspfEllipsoidFactory.h>
#include <map>
#include <string>
#include <vector>

class rspfFilename;
class rspfDatum;
class rspfWgs84Datum;
class rspfWgs72Datum;
////////
class rspfBj54Datum;
class rspfXiAn80Datum;
///////////wwadd
class rspfString;

class RSPF_DLL rspfDatumFactory : public rspfDatumFactoryInterface
{
public:

   /** virtual destructor */
   virtual ~rspfDatumFactory();

   /**
    * create method
    *
    * Implements pure virtual rspfDatumFactoryInterface::create.
    *
    * @return const pointer to a datum.
    */
   virtual const rspfDatum* create(const rspfString &code)const;
   virtual const rspfDatum* create(const rspfKeywordlist& kwl, const char *prefix=0)const;

   const rspfDatum* create(const rspfDatum* copy) const;

   static rspfDatumFactory* instance();
   const rspfDatum* wgs84()const{return theWgs84Datum;}
   const rspfDatum* wgs72()const{return theWgs72Datum;}
   const rspfDatum* bj54()const{return theBj54Datum;}
   const rspfDatum* xa80()const{return theXA80Datum;}
   std::vector<rspfString> getList()const;
   virtual void getList(std::vector<rspfString>& list)const;
   void writeCStructure(const rspfFilename& file);
protected:
   static rspfDatumFactory*       theInstance;
   const rspfDatum*               theWgs84Datum;
   const rspfDatum*               theWgs72Datum;
   const rspfDatum*               theBj54Datum;//wwadd
   const rspfDatum*               theXA80Datum;
   std::map<std::string, const rspfDatum*> theDatumTable;
   

   rspfDatumFactory()
      :theWgs84Datum(0),
       theWgs72Datum(0)
       {}

   void deleteAll();
   void initializeDefaults();
};
#endif
