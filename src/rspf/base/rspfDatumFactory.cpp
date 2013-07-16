//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
// Description:
//
// This holds the class definition of DatumFactory.
//*******************************************************************
//  $Id: rspfDatumFactory.cpp 19682 2011-05-31 14:21:20Z dburken $

#include <rspf/base/rspfDatumFactory.h>
#include <rspf/base/rspfEllipsoidFactory.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfThreeParamDatum.h>
#include <rspf/base/rspfSevenParamDatum.h>
#include <rspf/base/rspfWgs84Datum.h>
#include <rspf/base/rspfWgs72Datum.h>
////////////
#include <rspf/base/rspfbj54Datum.h>
#include <rspf/base/rspfXiAn80Datum.h>
/////////////wwadd
#include <rspf/base/rspfNadconNarDatum.h>
#include <rspf/base/rspfNadconNasDatum.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfPreferences.h>
#include "rspfDatumFactory.inc"

#include <cstring> /* for strlen */
#include <utility> /* for std::make_pair */

rspfDatumFactory* rspfDatumFactory::theInstance = 0;

static rspfString WGE = "WGE";
static rspfString WGD = "WGD";
static rspfString BJ54 = "BJ54";
static rspfString XiAn80 = "XiAn80";

rspfDatumFactory::~rspfDatumFactory()
{
   deleteAll();
   theInstance = 0;
}

rspfDatumFactory* rspfDatumFactory::instance()
{
   if(!theInstance) 
   {
      theInstance = new rspfDatumFactory;

      // datum depends on the creation of an ellipsoid factory
      // so make sure an instance exists
      rspfEllipsoidFactory::instance();
      theInstance->initializeDefaults();
      theInstance->theWgs84Datum = theInstance->create(WGE);
      theInstance->theWgs72Datum = theInstance->create(WGD);
	  theInstance->theXA80Datum=theInstance->create(XiAn80);
	  theInstance->theBj54Datum=theInstance->create(BJ54);
   }  
   return theInstance; 
} 

const rspfDatum* rspfDatumFactory::create(const rspfString &code)const
{
   if ( code.size() )
   {
      std::map<std::string, const rspfDatum*>::const_iterator datum = theDatumTable.find(code);
      
      if(datum != theDatumTable.end())
      {
         return (*datum).second;
      }

      if(code == "NAR") // User did not set "datum_grids" so map to NAR-C.
      {
         datum = theDatumTable.find("NAR-C");
         if(datum != theDatumTable.end())
         {
            return (*datum).second;
         }
      }
      else if(code == "NAS") // User did not set "datum_grids" so map to NAS-C."
      {
         datum = theDatumTable.find("NAS-C");
         if(datum != theDatumTable.end())
         {
            return (*datum).second;
         }
      }
   }
   
   return 0;
}

const rspfDatum* rspfDatumFactory::create(const rspfKeywordlist& kwl, const char *prefix) const
{
   rspfString alpha_code = kwl.find(prefix, rspfKeywordNames::DATUM_KW);
   if(!alpha_code.empty())
      return create(alpha_code);
   return 0;
}

const rspfDatum* rspfDatumFactory::create(const rspfDatum* aDatum)const
{
   if (aDatum)
      return create (aDatum->code());
   else
      return 0;
}

std::vector<rspfString> rspfDatumFactory::getList()const
{
   std::map<std::string, const rspfDatum*>::const_iterator datum = theDatumTable.begin();
   std::vector<rspfString> result;

   while(datum != theDatumTable.end())
   {
      result.push_back(rspfString((*datum).first) );
      ++datum;
   }
   return result;
}

void rspfDatumFactory::getList(std::vector<rspfString>& list) const
{
   std::map<std::string, const rspfDatum*>::const_iterator datum =
      theDatumTable.begin();

   while(datum != theDatumTable.end())
   {
      list.push_back( rspfString((*datum).first) );
      ++datum;
   }
}

void rspfDatumFactory::deleteAll()
{   
   std::map<std::string, const rspfDatum*>::iterator datum;

   datum = theDatumTable.begin();
   while(datum != theDatumTable.end())
   {
      delete ((*datum).second);
      ++datum;
   }

   theDatumTable.clear();

}

void rspfDatumFactory::initializeDefaults()
{
   //make the standards
   theDatumTable.insert(std::make_pair(WGE.string(), new rspfWgs84Datum));
   theDatumTable.insert(std::make_pair(WGD.string(), new rspfWgs72Datum));
   ////////////
   theDatumTable.insert(std::make_pair(BJ54.string(), new rspfBj54Datum));
   theDatumTable.insert(std::make_pair(XiAn80.string(), new rspfXiAn80Datum));
   //////////////////wwadd
   rspf_uint32 idx = 0;     
   while( std::strlen(threeParamDatum[idx].theCode) )
   {
      if( (threeParamDatum[idx].theCode != WGE) &&
          (threeParamDatum[idx].theCode != WGD) &&  (threeParamDatum[idx].theCode != BJ54) &&  (threeParamDatum[idx].theCode != XiAn80) )
      {
         theDatumTable.insert(std::make_pair(std::string(threeParamDatum[idx].theCode), 
            new rspfThreeParamDatum(threeParamDatum[idx].theCode, 
            threeParamDatum[idx].theName,
            rspfEllipsoidFactory::instance()->create(rspfString(threeParamDatum[idx].theEllipsoidCode)),
            threeParamDatum[idx].theSigmaX, 
            threeParamDatum[idx].theSigmaY, 
            threeParamDatum[idx].theSigmaZ, 
            threeParamDatum[idx].theWestLongitude, 
            threeParamDatum[idx].theEastLongitude, 
            threeParamDatum[idx].theSouthLatitude, 
            threeParamDatum[idx].theNorthLatitude, 
            threeParamDatum[idx].theParam1, 
            threeParamDatum[idx].theParam2, 
            threeParamDatum[idx].theParam3)));
      }

      ++idx;
   }
   idx = 0;
   while( std::strlen(sevenParamDatum[idx].theCode) )
   {
      theDatumTable.insert(std::make_pair(std::string(sevenParamDatum[idx].theCode), 
         new rspfSevenParamDatum(sevenParamDatum[idx].theCode, 
         sevenParamDatum[idx].theName,
         rspfEllipsoidFactory::instance()->create(rspfString(sevenParamDatum[idx].theEllipsoidCode)),
         sevenParamDatum[idx].theSigmaX, 
         sevenParamDatum[idx].theSigmaY, 
         sevenParamDatum[idx].theSigmaZ, 
         sevenParamDatum[idx].theWestLongitude, 
         sevenParamDatum[idx].theEastLongitude, 
         sevenParamDatum[idx].theSouthLatitude, 
         sevenParamDatum[idx].theNorthLatitude,
         sevenParamDatum[idx].theParam1,
         sevenParamDatum[idx].theParam2, 
         sevenParamDatum[idx].theParam3,
         sevenParamDatum[idx].theParam4, 
         sevenParamDatum[idx].theParam5,
         sevenParamDatum[idx].theParam6, 
         sevenParamDatum[idx].theParam7)));
      ++idx;
   }

   // Fetch the HARN grid filenames and add these datums to the table:
   rspfFilename file = rspfPreferences::instance()->findPreference("datum_grids");

   if(file != "")
   {
      if(!file.isDir())
      {
         file = file.path();
      }

      rspfFilename fileTest1 = file.dirCat("conus.las");
      rspfFilename fileTest2 = file.dirCat("conus.los");

      if (fileTest1.exists() && fileTest2.exists())
      {
         theDatumTable.insert(std::make_pair(std::string("NAS"), new rspfNadconNasDatum(file)));
         theDatumTable.insert(std::make_pair(std::string("NAR"), new rspfNadconNarDatum(file)));
      }
   }
}

void rspfDatumFactory::writeCStructure(const rspfFilename& /*file*/)
{
#if 0
   std::ofstream out(file.c_str());

   if(!out) return;

   out << "typedef struct rspfSevenParamDatumType" << std::endl
      << "{" << std::endl
      << "   rspfString theCode;\n"
      << "   rspfString theName;\n"
      << "   rspfString theEllipsoidCode;\n"
      << "   rspf_float64 theSigmaX;\n"
      << "   rspf_float64 theSigmaY;\n"
      << "   rspf_float64 theSigmaZ;\n"
      << "   rspf_float64 theWestLongitude;\n"
      << "   rspf_float64 theEastLongitude;\n"
      << "   rspf_float64 theSouthLatitude;\n"
      << "   rspf_float64 theNorthLatitude;\n"
      << "   rspf_float64 theParam1;\n"
      << "   rspf_float64 theParam2;\n"
      << "   rspf_float64 theParam3;\n"
      << "   rspf_float64 theParam4;\n"
      << "   rspf_float64 theParam5;\n"
      << "   rspf_float64 theParam6;\n"
      << "   rspf_float64 theParam7;\n"
      << "};\n";

   out << "typedef struct rspfThreeParamDatumType" << std::endl
      << "{" << std::endl
      << "   rspfString theCode;\n"
      << "   rspfString theName;\n"
      << "   rspfString theEllipsoidCode;\n"
      << "   rspf_float64 theSigmaX;\n"
      << "   rspf_float64 theSigmaY;\n"
      << "   rspf_float64 theSigmaZ;\n"
      << "   rspf_float64 theWestLongitude;\n"
      << "   rspf_float64 theEastLongitude;\n"
      << "   rspf_float64 theSouthLatitude;\n"
      << "   rspf_float64 theNorthLatitude;\n"
      << "   rspf_float64 theParam1;\n"
      << "   rspf_float64 theParam2;\n"
      << "   rspf_float64 theParam3;\n"
      << "};\n";
   out << "#define NUMBER_OF_SEVEN_PARAM_DATUMS " << 2 << std::endl;
   if(out)
   {
      std::map<std::string, const rspfDatum*>::iterator datum;
      out << "static rspfThreeParamDatumType threeParamDatum[] = {\n";
      datum = theDatumTable.begin();
      rspf_uint32 datumCount = 0;
      while(datum != theDatumTable.end())
      {
         bool written = false;
         if( ((*datum).first != "NAS") &&
            ((*datum).first != "NAR")&&
            ((*datum).first != "EUR-7")&&
            ((*datum).first != "OGB-7"))
         {
            written = true;
            const rspfDatum* d = (*datum).second;

            out << "{\"" << d->code() << "\", "
               << "\"" << d->name() << "\", "
               << "\"" << d->ellipsoid()->code() << "\", "
               << d->sigmaX() << ", "
               << d->sigmaY() << ", "
               << d->sigmaZ() << ", "
               << d->westLongitude()*DEG_PER_RAD << ", "
               << d->eastLongitude()*DEG_PER_RAD << ", "
               << d->southLatitude()*DEG_PER_RAD << ", "
               << d->northLatitude()*DEG_PER_RAD << ", "
               << d->param1() << ", " 
               << d->param2() << ", " 
               << d->param3() << "}";
            ++datumCount;
         }
         ++datum;
         if(datum != theDatumTable.end()&&written)
         {

            out << "," << std::endl;
         }
         else if(datum == theDatumTable.end())
         {
            out << std::endl;
         }
      }
      out << "};" << std::endl;

      out << "#define NUMBER_OF_THREE_PARAM_DATUMS " << datumCount << std::endl;
      out << "static rspfSevenParamDatumType sevenParamDatum[] = {\n";
      const rspfDatum* d = create("EUR-7");
      if(d)
      {
         out << "{\"" << d->code() << "\", "
            << "\"" << d->name() << "\", "
            << "\"" << d->ellipsoid()->code() << "\", "
            << d->sigmaX() << ", "
            << d->sigmaY() << ", "
            << d->sigmaZ() << ", "
            << d->westLongitude()*DEG_PER_RAD << ", "
            << d->eastLongitude()*DEG_PER_RAD << ", "
            << d->southLatitude()*DEG_PER_RAD << ", "
            << d->northLatitude()*DEG_PER_RAD << ", "
            << d->param1() << ", " 
            << d->param2() << ", " 
            << d->param3() << ", "
            << d->param4() << ", "
            << d->param5() << ", "
            << d->param6() << ", "
            << d->param7() << "},\n";
      }
      d = create("OGB-7");
      if(d)
      {
         out << "{\"" << d->code() << "\", "
            << "\"" << d->name() << "\", "
            << "\"" << d->ellipsoid()->code() << "\", "
            << d->sigmaX() << ", "
            << d->sigmaY() << ", "
            << d->sigmaZ() << ", "
            << d->westLongitude()*DEG_PER_RAD << ", "
            << d->eastLongitude()*DEG_PER_RAD << ", "
            << d->southLatitude()*DEG_PER_RAD << ", "
            << d->northLatitude()*DEG_PER_RAD << ", "
            << d->param1() << ", " 
            << d->param2() << ", " 
            << d->param3() << ", "
            << d->param4() << ", "
            << d->param5() << ", "
            << d->param6() << ", "
            << d->param7() << "}\n";
      }
      out << "};" << std::endl;
   }
#endif
}

