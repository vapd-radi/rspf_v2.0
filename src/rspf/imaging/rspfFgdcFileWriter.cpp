//----------------------------------------------------------------------------
// Copyright (c) 2005, David Burken, all rights reserved.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Class to write out meta data in a Federal Geographic Data
// Committe (FGDC) format.
//
//----------------------------------------------------------------------------
// $Id: rspfFgdcFileWriter.cpp 21631 2012-09-06 18:10:55Z dburken $


#include <rspf/imaging/rspfFgdcFileWriter.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfXmlNode.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfMapProjectionInfo.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <fstream>
using namespace std;

RTTI_DEF1(rspfFgdcFileWriter, "rspfFgdcFileWriter", rspfMetadataFileWriter)

static rspfTrace traceDebug("rspfFgdcFileWriter:debug");

   
rspfFgdcFileWriter::rspfFgdcFileWriter()
   :
   theIndentionLevel(0),
   theTabString("\t"),
   theTemplate(),
   theSourceImageFilename()
{
}

rspfFgdcFileWriter::~rspfFgdcFileWriter()
{
}

bool rspfFgdcFileWriter::writeFile()
{
   if(theFilename == rspfFilename::NIL)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfFgdcFileWriter::writeFile no filename set!" << endl;
      return false;
   }
   
   if( !theInputConnection )
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfFgdcFileWriter::writeFile no input connection!" << endl;
      return false;
   }

   ofstream os(theFilename.c_str());
   if (!os)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfFgdcFileWriter::writeFile no input connection!" << endl;
      return false;
   }

   // Get the geometry from the input.
   rspfMapProjection* mapProj = 0;
   rspfRefPtr<rspfImageGeometry> inputGeom = theInputConnection->getImageGeometry();
   if ( inputGeom.valid() ) mapProj = PTR_CAST(rspfMapProjection, inputGeom->getProjection());
   if (!mapProj)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfFgdcFileWriter::writeFile Not map projected."
         << endl;
      return false;
   }

   rspfMapProjectionInfo mpi(mapProj, theInputConnection->getBoundingRect());

   os << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n"
      << "<?xml-stylesheet type=\"text/xsl\" href=\"/common/fgdc_classic.xsl\" ?>\n";
   openTag(os, rspfString("metadata"),  true);
   openTag(os, rspfString("idinfo"),    true);
   openTag(os, rspfString("citation"),  true);
   openTag(os, rspfString("citeinfo"),  true);
   
   openTag(os, rspfString("origin"),    false);
   os << find(rspfString("/metadata/idinfo/citation/citeinfo/origin"));
   closeTag(os, rspfString("origin"),   false);
   
   openTag(os, rspfString("pubdate"),    false);
   os << find(rspfString("/metadata/idinfo/citation/citeinfo/pubdate"));
   closeTag(os, rspfString("pubdate"),   false);

   openTag(os, rspfString("title"),    false);
   // os << find(rspfString("/metadata/idinfo/citation/citeinfo/title"));
   os << theSourceImageFilename.c_str();
   closeTag(os, rspfString("title"),   false);

   openTag(os, rspfString("edition"),    false);
   os << find(rspfString("/metadata/idinfo/citation/citeinfo/edition"));
   closeTag(os, rspfString("edition"),   false);

   openTag(os, rspfString("geoform"),    false);
   os << find(rspfString("/metadata/idinfo/citation/citeinfo/geoform"));
   closeTag(os, rspfString("geoform"),   false);

   openTag(os, rspfString("serinfo"),  true);
   openTag(os, rspfString("sername"),    false);
   os << find(rspfString("/metadata/idinfo/citation/citeinfo/serinfo/sername"));
   closeTag(os, rspfString("sername"),   false);

   openTag(os, rspfString("issue"),    false);
   os << theSourceImageFilename.c_str();
   closeTag(os, rspfString("issue"),   false);

   closeTag(os, rspfString("serinfo"),  true);

   openTag(os, rspfString("pubinfo"),  true);

   openTag(os, rspfString("pubplace"),    false);
   os << find(rspfString("/metadata/idinfo/citation/citeinfo/pubinfo/pubplace"));
   closeTag(os, rspfString("pubplace"),   false);

   openTag(os, rspfString("publish"),    false);
   os << find(rspfString("/metadata/idinfo/citation/citeinfo/pubinfo/publish"));
   closeTag(os, rspfString("publish"),   false);

   closeTag(os, rspfString("pubinfo"),  true);
   closeTag(os, rspfString("citeinfo"),  true);
   closeTag(os, rspfString("citation"),  true);

   openTag(os, rspfString("descript"),  true);

   openTag(os, rspfString("abstract"),    false);
   os << find(rspfString("/metadata/idinfo/descript/abstract"));
   closeTag(os, rspfString("abstract"),   false);

   openTag(os, rspfString("purpose"),    false);
   os << find(rspfString("/metadata/idinfo/descript/purpose"));
   closeTag(os, rspfString("purpose"),   false);

   openTag(os, rspfString("supplinf"),    false);
   os << find(rspfString("/metadata/idinfo/descript/supplinf"));
   closeTag(os, rspfString("supplinf"),   false);

   closeTag(os, rspfString("descript"),  true);

   openTag(os, rspfString("timeperd"),  true);
   openTag(os, rspfString("timeinfo"),  true);
   openTag(os, rspfString("sngdate"),  true);

   openTag(os, rspfString("caldate"),    false);
   os << find(rspfString("/metadata/idinfo/timeperd/timeinfo/sngdate/caldate"));
   closeTag(os, rspfString("caldate"),   false);
   
   closeTag(os, rspfString("sngdate"),  true);
   closeTag(os, rspfString("timeinfo"),  true);

   
   openTag(os, rspfString("current"),    false);
   os << find(rspfString("/metadata/idinfo/timeperd/current"));
   closeTag(os, rspfString("current"),   false);

   closeTag(os, rspfString("timeperd"),  true);

   openTag(os, rspfString("status"),  true);

   openTag(os, rspfString("progress"),    false);
   os << find(rspfString("/metadata/idinfo/status/progress"));
   closeTag(os, rspfString("progress"),   false);

   openTag(os, rspfString("update"),    false);
   os << find(rspfString("/metadata/idinfo/status/update"));
   closeTag(os, rspfString("update"),   false);

   closeTag(os, rspfString("status"),  true);

   openTag(os, rspfString("spdom"),  true);
   openTag(os, rspfString("bounding"),  true);

   if (mapProj->isGeographic())
   {
      openTag(os, rspfString("westbc"),    false);
      os << rspfString::toString(mpi.ulGroundPt().lon);
      closeTag(os, rspfString("westbc"),   false);
      
      openTag(os, rspfString("eastbc"),    false);
      os << rspfString::toString(mpi.lrGroundPt().lon);
      closeTag(os, rspfString("eastbc"),   false);
      
      openTag(os, rspfString("northbc"),    false);
      os << rspfString::toString(mpi.ulGroundPt().lat);
      closeTag(os, rspfString("northbc"),   false);
      
      openTag(os, rspfString("southbc"),    false);
      os << rspfString::toString(mpi.lrGroundPt().lat);
      closeTag(os, rspfString("southbc"),   false);
   }
   else
   {
      openTag(os, rspfString("westbc"),    false);
      os << rspfString::toString(mpi.ulEastingNorthingPt().x);
      closeTag(os, rspfString("westbc"),   false);
      
      openTag(os, rspfString("eastbc"),    false);
      os << rspfString::toString(mpi.lrEastingNorthingPt().x);
      closeTag(os, rspfString("eastbc"),   false);
      
      openTag(os, rspfString("northbc"),    false);
      os << rspfString::toString(mpi.ulEastingNorthingPt().y);
      closeTag(os, rspfString("northbc"),   false);
      
      openTag(os, rspfString("southbc"),    false);
      os << rspfString::toString(mpi.ulEastingNorthingPt().y);
      closeTag(os, rspfString("southbc"),   false);
   }
   
   closeTag(os, rspfString("bounding"),  true);
   closeTag(os, rspfString("spdom"),    true);

   openTag(os, rspfString("keywords"), true);

   openTag(os, rspfString("theme"), true);

   openTag(os, rspfString("themekt"), false);
   os << find(rspfString("/metadata/idinfo/keywords/theme/themekt"));
   closeTag(os, rspfString("themekt"), false);

   openTag(os, rspfString("themekey"), false);
   os << find(rspfString("/metadata/idinfo/keywords/theme/themekey"));
   closeTag(os, rspfString("themekey"), false);
   
   closeTag(os, rspfString("theme"), true);

   closeTag(os, rspfString("keywords"), true);

   openTag(os, rspfString("accconst"), false);
   os << find(rspfString("/metadata/idinfo/accconst"));
   closeTag(os, rspfString("accconst"), false);

   openTag(os, rspfString("useconst"), false);
   os << find(rspfString("/metadata/idinfo/useconst"));
   closeTag(os, rspfString("useconst"), false);

   openTag(os, rspfString("ptcontac"), true);
   
   openTag(os, rspfString("cntinfo"), true);
   
   openTag(os, rspfString("cntperp"), true);
   openTag(os, rspfString("cntper"), false);
   os << find(rspfString("/metadata/idinfo/ptcontac/cntinfo/cntperp/cntper"));
   closeTag(os, rspfString("cntper"), false);
   closeTag(os, rspfString("cntperp"), true);

   openTag(os, rspfString("cntaddr"), true);

   openTag(os, rspfString("addrtype"), false);
   os << find(rspfString("/metadata/idinfo/ptcontac/cntinfo/cntaddr/addrtype"));
   closeTag(os, rspfString("addrtype"), false);

   openTag(os, rspfString("address"), false);
   os << find(rspfString("/metadata/idinfo/ptcontac/cntinfo/cntaddr/address"));
   closeTag(os, rspfString("address"), false);

   openTag(os, rspfString("city"), false);
   os << find(rspfString("/metadata/idinfo/ptcontac/cntinfo/cntaddr/city"));
   closeTag(os, rspfString("city"), false);

   openTag(os, rspfString("state"), false);
   os << find(rspfString("/metadata/idinfo/ptcontac/cntinfo/cntaddr/state"));
   closeTag(os, rspfString("state"), false);

   openTag(os, rspfString("postal"), false);
   os << find(rspfString("/metadata/idinfo/ptcontac/cntinfo/cntaddr/postal"));
   closeTag(os, rspfString("postal"), false);

   openTag(os, rspfString("country"), false);
   os << find(rspfString("/metadata/idinfo/ptcontac/cntinfo/cntaddr/country"));
   closeTag(os, rspfString("country"), false);

   closeTag(os, rspfString("cntaddr"), true);

   openTag(os, rspfString("cntvoice"), false);
   os << find(rspfString("/metadata/idinfo/ptcontac/cntinfo/cntvoice"));
   closeTag(os, rspfString("cntvoice"), false);
  
   openTag(os, rspfString("cntfax"), false);

   os << find(rspfString("/metadata/idinfo/ptcontac/cntinfo/cntfax"));
   closeTag(os, rspfString("cntfax"), false);
  
   openTag(os, rspfString("cntemail"), false);
   os << find(rspfString("/metadata/idinfo/ptcontac/cntinfo/cntemail"));
   closeTag(os, rspfString("cntemail"), false);
   
   closeTag(os, rspfString("cntinfo"), true);
   closeTag(os, rspfString("ptcontac"), true);

   openTag(os, rspfString("secinfo"), true);

   openTag(os, rspfString("secclass"), false);
   os << find(rspfString("/metadata/idinfo/secinfo/secclass"));
   closeTag(os, rspfString("secclass"), false);

   openTag(os, rspfString("secsys"), false);
   os << find(rspfString("/metadata/idinfo/secinfo/secsys"));
   closeTag(os, rspfString("secsys"), false);

   openTag(os, rspfString("sechandl"), false);
   os << find(rspfString("/metadata/idinfo/secinfo/sechandl"));
   closeTag(os, rspfString("sechandl"), false);

   closeTag(os, rspfString("secinfo"), true);
   closeTag(os, rspfString("idinfo"), true);
   
   openTag(os, rspfString("distinfo"), true);
   openTag(os, rspfString("distrib"), true);
   openTag(os, rspfString("cntinfo"), true);
   openTag(os, rspfString("cntperp"), true);
   openTag(os, rspfString("cntper"), false);
   os << find(rspfString("/metadata/distinfo/distrib/cntinfo/cntperp/cntper"));

   closeTag(os, rspfString("cntper"), false);
   closeTag(os, rspfString("cntperp"), true);

   openTag(os, rspfString("cntaddr"), true);

   openTag(os, rspfString("addrtype"), false);
   os << find(rspfString("/metadata/distinfo/distrib/cntinfo/cntaddr/addrtype"));
   closeTag(os, rspfString("addrtype"), false);

   openTag(os, rspfString("address"), false);
   os << find(rspfString("/metadata/distinfo/distrib/cntinfo/cntaddr/address"));
   closeTag(os, rspfString("address"), false);

   openTag(os, rspfString("city"), false);
   os << find(rspfString("/metadata/distinfo/distrib/cntinfo/cntaddr/city"));
   closeTag(os, rspfString("city"), false);

   openTag(os, rspfString("state"), false);
   os << find(rspfString("/metadata/distinfo/distrib/cntinfo/cntaddr/state"));
   closeTag(os, rspfString("state"), false);

   openTag(os, rspfString("postal"), false);
   os << find(rspfString("/metadata/distinfo/distrib/cntinfo/cntaddr/postal"));
   closeTag(os, rspfString("postal"), false);

   openTag(os, rspfString("country"), false);
   os << find(rspfString("/metadata/distinfo/distrib/cntinfo/cntaddr/country"));
   closeTag(os, rspfString("country"), false);

   closeTag(os, rspfString("cntaddr"), true);

   openTag(os, rspfString("cntvoice"), false);
   os << find(rspfString("/metadata/distinfo/distrib/cntinfo/cntvoice"));
   closeTag(os, rspfString("cntvoice"), false);
  
   openTag(os, rspfString("cntfax"), false);
   os << find(rspfString("/metadata/distinfo/distrib/cntinfo/cntfax"));
   closeTag(os, rspfString("cntfax"), false);
  
   openTag(os, rspfString("cntemail"), false);
   os << find(rspfString("/metadata/distinfo/distrib/cntinfo/cntemail"));
   closeTag(os, rspfString("cntemail"), false);
   
   closeTag(os, rspfString("cntinfo"), true);
   closeTag(os, rspfString("distrib"), true);

   openTag(os, rspfString("resdesc"), false);
   os << find(rspfString("/metadata/distinfo/resdesc"));
   closeTag(os, rspfString("resdesc"), false);

   openTag(os, rspfString("distliab"), false);
   os << find(rspfString("/metadata/distinfo/distliab"));
   closeTag(os, rspfString("distliab"), false);

   openTag(os, rspfString("stdorder"), true);
   openTag(os, rspfString("digform"), true);
   openTag(os, rspfString("digtinfo"), true);
   
   openTag(os, rspfString("formname"), false);
   os << find(rspfString("/metadata/distinfo/stdorder/digform/digtinfo/formname"));
   closeTag(os, rspfString("formname"), false);

   closeTag(os, rspfString("digtinfo"), true);

   openTag(os, rspfString("digtopt"), true);
   openTag(os, rspfString("onlinopt"), true);   
   openTag(os, rspfString("computer"), true);
   openTag(os, rspfString("networka"), true);

   openTag(os, rspfString("networkr"), false);
   os << find(rspfString("/metadata/distinfo/stdorder/digform/digtopt/onlinopt/computer/networka/networkr"));
   closeTag(os, rspfString("networkr"), false);

   closeTag(os, rspfString("networka"), true);
   closeTag(os, rspfString("computer"), true);
   closeTag(os, rspfString("onlinopt"), true);
   closeTag(os, rspfString("digtopt"), true);
   closeTag(os, rspfString("digform"), true);

   openTag(os, rspfString("fees"), false);
   os << find(rspfString("/metadata/distinfo/stdorder/fees"));
   closeTag(os, rspfString("fees"), false);

   closeTag(os, rspfString("stdorder"), true);
   closeTag(os, rspfString("distinfo"), true);

   openTag(os, rspfString("metainfo"), true);

   openTag(os, rspfString("metd"), false);
   os << find(rspfString("/metadata/metainfo/metd"));
   closeTag(os, rspfString("metd"), false);

   openTag(os, rspfString("metc"), true);
   openTag(os, rspfString("cntinfo"), true);
   openTag(os, rspfString("cntperp"), true);
   
   openTag(os, rspfString("cntper"), false);
   os << find(rspfString("/metadata/metainfo/metc/cntinfo/cntperp/cntper"));
   closeTag(os, rspfString("cntper"), false);

   closeTag(os, rspfString("cntperp"), true);
   
   openTag(os, rspfString("cntaddr"), true);

   openTag(os, rspfString("addrtype"), false);
   os << find(rspfString("/metadata/metainfo/metc/cntinfo/cntaddr/addrtype"));
   closeTag(os, rspfString("addrtype"), false);

   openTag(os, rspfString("address"), false);
   os << find(rspfString("/metadata/metainfo/metc/cntinfo/cntaddr/address"));
   closeTag(os, rspfString("address"), false);

   openTag(os, rspfString("city"), false);
   os << find(rspfString("/metadata/metainfo/metc/cntinfo/cntaddr/city"));
   closeTag(os, rspfString("city"), false);

   openTag(os, rspfString("state"), false);
   os << find(rspfString("/metadata/metainfo/metc/cntinfo/cntaddr/state"));
   closeTag(os, rspfString("state"), false);

   openTag(os, rspfString("postal"), false);
   os << find(rspfString("/metadata/metainfo/metc/cntinfo/cntaddr/postal"));
   closeTag(os, rspfString("postal"), false);

   openTag(os, rspfString("country"), false);
   os << find(rspfString("/metadata/metainfo/metc/cntinfo/cntaddr/country"));
   closeTag(os, rspfString("country"), false);

   closeTag(os, rspfString("cntaddr"), true);

   openTag(os, rspfString("cntvoice"), false);
   os << find(rspfString("/metadata/metainfo/metc/cntinfo/cntvoice"));
   closeTag(os, rspfString("cntvoice"), false);
  
   openTag(os, rspfString("cntfax"), false);
   os << find(rspfString("/metadata/metainfo/metc/cntinfo/cntfax"));
   closeTag(os, rspfString("cntfax"), false);
  
   openTag(os, rspfString("cntemail"), false);
   os << find(rspfString("/metadata/metainfo/metc/cntinfo/cntemail"));
   closeTag(os, rspfString("cntemail"), false);
   
   closeTag(os, rspfString("cntinfo"), true);
   closeTag(os, rspfString("metc"), true);

   openTag(os, rspfString("metstdn"), false);
   os << find(rspfString("/metadata/metainfo/metstdn"));
   closeTag(os, rspfString("metstdn"), false);

   openTag(os, rspfString("metstdv"), false);
   os << find(rspfString("/metadata/metainfo/metstdv"));
   closeTag(os, rspfString("metstdv"), false);

   closeTag(os, rspfString("metainfo"), true);
   closeTag(os, rspfString("metadata"), true);

   os.close();

   return true;
}

bool rspfFgdcFileWriter::writeTemplate(const rspfFilename& file) const
{
   ofstream os(file.c_str());
   if (!os)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfFgdcFileWriter::writeTemplate Could not open: " << file
         << endl;
      return false;
   }

   os << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n"
      << "<?xml-stylesheet type=\"text/xsl\" href=\"/common/fgdc_classic.xsl\" ?>\n";
   openTag(os, rspfString("metadata"),  true);
   openTag(os, rspfString("idinfo"),    true);
   openTag(os, rspfString("citation"),  true);
   openTag(os, rspfString("citeinfo"),  true);
   
   openTag(os, rspfString("origin"),    false);
   closeTag(os, rspfString("origin"),   false);
   
   openTag(os, rspfString("pubdate"),    false);
   closeTag(os, rspfString("pubdate"),   false);

   openTag(os, rspfString("title"),    false);
   closeTag(os, rspfString("title"),   false);

   openTag(os, rspfString("edition"),    false);
   closeTag(os, rspfString("edition"),   false);

   openTag(os, rspfString("geoform"),    false);
   closeTag(os, rspfString("geoform"),   false);

   openTag(os, rspfString("serinfo"),  true);
   openTag(os, rspfString("sername"),    false);

   closeTag(os, rspfString("sername"),   false);

   openTag(os, rspfString("issue"),    false);
   closeTag(os, rspfString("issue"),   false);

   closeTag(os, rspfString("serinfo"),  true);

   openTag(os, rspfString("pubinfo"),  true);

   openTag(os, rspfString("pubplace"),    false);

   closeTag(os, rspfString("pubplace"),   false);

   openTag(os, rspfString("publish"),    false);
   closeTag(os, rspfString("publish"),   false);

   closeTag(os, rspfString("pubinfo"),  true);
   closeTag(os, rspfString("citeinfo"),  true);
   closeTag(os, rspfString("citation"),  true);

   openTag(os, rspfString("descript"),  true);

   openTag(os, rspfString("abstract"),    false);
   closeTag(os, rspfString("abstract"),   false);

   openTag(os, rspfString("purpose"),    false);
   closeTag(os, rspfString("purpose"),   false);

   openTag(os, rspfString("supplinf"),    false);
   closeTag(os, rspfString("supplinf"),   false);

   closeTag(os, rspfString("descript"),  true);

   openTag(os, rspfString("timeperd"),  true);
   openTag(os, rspfString("timeinfo"),  true);
   openTag(os, rspfString("sngdate"),  true);

   openTag(os, rspfString("caldate"),    false);
   closeTag(os, rspfString("caldate"),   false);
   
   closeTag(os, rspfString("sngdate"),  true);
   closeTag(os, rspfString("timeinfo"),  true);

   
   openTag(os, rspfString("current"),    false);
   closeTag(os, rspfString("current"),   false);

   closeTag(os, rspfString("timeperd"),  true);

   openTag(os, rspfString("status"),  true);

   openTag(os, rspfString("progress"),    false);
   closeTag(os, rspfString("progress"),   false);

   openTag(os, rspfString("update"),    false);
   closeTag(os, rspfString("update"),   false);

   closeTag(os, rspfString("status"),  true);

   openTag(os, rspfString("spdom"),  true);
   openTag(os, rspfString("bounding"),  true);

   openTag(os, rspfString("westbc"),    false);
   closeTag(os, rspfString("westbc"),   false);
   openTag(os, rspfString("eastbc"),    false);

   closeTag(os, rspfString("eastbc"),   false);
   openTag(os, rspfString("northbc"),    false);

   closeTag(os, rspfString("northbc"),   false);
   openTag(os, rspfString("southbc"),    false);

   closeTag(os, rspfString("southbc"),   false);

   closeTag(os, rspfString("bounding"),  true);
   closeTag(os, rspfString("spdom"),    true);

   openTag(os, rspfString("keywords"), true);

   openTag(os, rspfString("theme"), true);

   openTag(os, rspfString("themekt"), false);
   closeTag(os, rspfString("themekt"), false);

   openTag(os, rspfString("themekey"), false);
   closeTag(os, rspfString("themekey"), false);
   
   closeTag(os, rspfString("theme"), true);

   closeTag(os, rspfString("keywords"), true);

   openTag(os, rspfString("accconst"), false);
   closeTag(os, rspfString("accconst"), false);

   openTag(os, rspfString("useconst"), false);
   closeTag(os, rspfString("useconst"), false);

   openTag(os, rspfString("ptcontac"), true);
   
   openTag(os, rspfString("cntinfo"), true);
   
   openTag(os, rspfString("cntperp"), true);
   openTag(os, rspfString("cntper"), false);
   closeTag(os, rspfString("cntper"), false);
   closeTag(os, rspfString("cntperp"), true);

   openTag(os, rspfString("cntaddr"), true);

   openTag(os, rspfString("addrtype"), false);
   closeTag(os, rspfString("addrtype"), false);

   openTag(os, rspfString("address"), false);
   closeTag(os, rspfString("address"), false);

   openTag(os, rspfString("city"), false);
   closeTag(os, rspfString("city"), false);

   openTag(os, rspfString("state"), false);
   closeTag(os, rspfString("state"), false);

   openTag(os, rspfString("postal"), false);
   closeTag(os, rspfString("postal"), false);

   openTag(os, rspfString("country"), false);
   closeTag(os, rspfString("country"), false);

   closeTag(os, rspfString("cntaddr"), true);

   openTag(os, rspfString("cntvoice"), false);
   closeTag(os, rspfString("cntvoice"), false);
  
   openTag(os, rspfString("cntfax"), false);
   closeTag(os, rspfString("cntfax"), false);
  
   openTag(os, rspfString("cntemail"), false);
   closeTag(os, rspfString("cntemail"), false);
   
   closeTag(os, rspfString("cntinfo"), true);
   
   closeTag(os, rspfString("ptcontac"), true);

   openTag(os, rspfString("secinfo"), true);
   openTag(os, rspfString("secclass"), false);
   closeTag(os, rspfString("secclass"), false);
   openTag(os, rspfString("secsys"), false);
   closeTag(os, rspfString("secsys"), false);
   openTag(os, rspfString("sechandl"), false);
   closeTag(os, rspfString("sechandl"), false);
   closeTag(os, rspfString("secinfo"), true);

   closeTag(os, rspfString("idinfo"), true);

   openTag(os, rspfString("distinfo"), true);
   openTag(os, rspfString("distrib"), true);
   openTag(os, rspfString("cntinfo"), true);
   openTag(os, rspfString("cntperp"), true);
   openTag(os, rspfString("cntper"), false);
   closeTag(os, rspfString("cntper"), false);
   closeTag(os, rspfString("cntperp"), true);

   openTag(os, rspfString("cntaddr"), true);

   openTag(os, rspfString("addrtype"), false);
   closeTag(os, rspfString("addrtype"), false);

   openTag(os, rspfString("address"), false);
   closeTag(os, rspfString("address"), false);

   openTag(os, rspfString("city"), false);
   closeTag(os, rspfString("city"), false);

   openTag(os, rspfString("state"), false);
   closeTag(os, rspfString("state"), false);

   openTag(os, rspfString("postal"), false);
   closeTag(os, rspfString("postal"), false);

   openTag(os, rspfString("country"), false);
   closeTag(os, rspfString("country"), false);

   closeTag(os, rspfString("cntaddr"), true);

   openTag(os, rspfString("cntvoice"), false);
   closeTag(os, rspfString("cntvoice"), false);
  
   openTag(os, rspfString("cntfax"), false);
   closeTag(os, rspfString("cntfax"), false);
  
   openTag(os, rspfString("cntemail"), false);
   closeTag(os, rspfString("cntemail"), false);
   
   closeTag(os, rspfString("cntinfo"), true);
   closeTag(os, rspfString("distrib"), true);

   openTag(os, rspfString("resdesc"), false);
   closeTag(os, rspfString("resdesc"), false);

   openTag(os, rspfString("distliab"), false);
   closeTag(os, rspfString("distliab"), false);

   openTag(os, rspfString("stdorder"), true);
   openTag(os, rspfString("digform"), true);
   openTag(os, rspfString("digtinfo"), true);
   
   openTag(os, rspfString("formname"), false);
   closeTag(os, rspfString("formname"), false);

   closeTag(os, rspfString("digtinfo"), true);

   openTag(os, rspfString("digtopt"), true);
   openTag(os, rspfString("onlinopt"), true);   
   openTag(os, rspfString("computer"), true);
   openTag(os, rspfString("networka"), true);

   openTag(os, rspfString("networkr"), false);
   closeTag(os, rspfString("networkr"), false);

   closeTag(os, rspfString("networka"), true);
   closeTag(os, rspfString("computer"), true);
   closeTag(os, rspfString("onlinopt"), true);
   closeTag(os, rspfString("digtopt"), true);
   closeTag(os, rspfString("digform"), true);

   openTag(os, rspfString("fees"), false);
   closeTag(os, rspfString("fees"), false);

   closeTag(os, rspfString("stdorder"), true);
   closeTag(os, rspfString("distinfo"), true);

   openTag(os, rspfString("metainfo"), true);

   openTag(os, rspfString("metd"), false);
   closeTag(os, rspfString("metd"), false);

   openTag(os, rspfString("metc"), true);
   openTag(os, rspfString("cntinfo"), true);
   openTag(os, rspfString("cntperp"), true);
   
   openTag(os, rspfString("cntper"), false);
   closeTag(os, rspfString("cntper"), false);

   closeTag(os, rspfString("cntperp"), true);
   
   openTag(os, rspfString("cntaddr"), true);

   openTag(os, rspfString("addrtype"), false);
   closeTag(os, rspfString("addrtype"), false);

   openTag(os, rspfString("address"), false);
   closeTag(os, rspfString("address"), false);

   openTag(os, rspfString("city"), false);
   closeTag(os, rspfString("city"), false);

   openTag(os, rspfString("state"), false);
   closeTag(os, rspfString("state"), false);

   openTag(os, rspfString("postal"), false);
   closeTag(os, rspfString("postal"), false);

   openTag(os, rspfString("country"), false);
   closeTag(os, rspfString("country"), false);

   closeTag(os, rspfString("cntaddr"), true);

   openTag(os, rspfString("cntvoice"), false);
   closeTag(os, rspfString("cntvoice"), false);
  
   openTag(os, rspfString("cntfax"), false);
   closeTag(os, rspfString("cntfax"), false);
  
   openTag(os, rspfString("cntemail"), false);
   closeTag(os, rspfString("cntemail"), false);
   
   closeTag(os, rspfString("cntinfo"), true);
   closeTag(os, rspfString("metc"), true);

   openTag(os, rspfString("metstdn"), false);
   closeTag(os, rspfString("metstdn"), false);

   openTag(os, rspfString("metstdv"), false);
   closeTag(os, rspfString("metstdv"), false);

   closeTag(os, rspfString("metainfo"), true);
   closeTag(os, rspfString("metadata"), true);

   os.close();

	if(traceDebug())
	{
		rspfNotify(rspfNotifyLevel_NOTICE)
      << "Wrote file:  " << file.c_str() << endl;
	}
   
   return true;
}
   
bool rspfFgdcFileWriter::loadState (const rspfKeywordlist& kwl,
                                     const char* prefix)
{
   const char* lookup = kwl.find(prefix, "template");
   if (lookup)
   {
      setTemplate(rspfFilename(lookup));
   }
   lookup = kwl.find(prefix, "source_image_filename");
   if (lookup)
   {
      theSourceImageFilename = lookup;
   }
   lookup = kwl.find(prefix, "tab_string");
   if (lookup)
   {
      theTabString = lookup;
   }
   lookup = kwl.find(prefix, "fgdc_template_doc");
   if (lookup)
   {
      setTemplate(rspfFilename(lookup));
   }
   
   return rspfMetadataFileWriter::loadState(kwl, prefix);
}

rspf_uint32 rspfFgdcFileWriter::getIndentionLevel() const
{
   return theIndentionLevel;
}

void rspfFgdcFileWriter::setIndentionLevel(rspf_uint32 level)
{
   theIndentionLevel = level;
}

rspfString rspfFgdcFileWriter::getTabString() const
{
   return theTabString;
}

void rspfFgdcFileWriter::setTabString(const rspfString& tabString)
{
   theTabString = tabString;
}

void rspfFgdcFileWriter::openTag(std::ostream& os,
                                  const rspfString& tag,
                                  bool newLine) const
{
   if ( theIndentionLevel && theTabString.size() )
   {
      rspfString tab;
      for (rspf_uint32 i = 0; i < theIndentionLevel; ++i)
      {
         tab += theTabString;
      }

      os << tab.c_str();
   }
   
   os << "<" << tag.c_str() << ">";

   if (newLine)
   {
      os << "\n";
      ++theIndentionLevel;
   }
}

void rspfFgdcFileWriter::closeTag(std::ostream& os,
                                   const rspfString& tag,
                                   bool decrementLevel) const
{
   if ( decrementLevel && (theIndentionLevel > 1) && theTabString.size() )
   {
      rspfString tab;
      for (rspf_uint32 i = 0; i < theIndentionLevel-1; ++i)
      {
         tab += theTabString;
      }

      os << tab.c_str();
   }
   
   os << "</" << tag.c_str() << ">\n";

   if (theIndentionLevel && decrementLevel)
   {
      --theIndentionLevel;
   }
}

void rspfFgdcFileWriter::setTemplate(const rspfFilename& xmlTemplateFile)
{
   theTemplate.openFile(xmlTemplateFile);
}

rspfString rspfFgdcFileWriter::find(const rspfString& xpath) const
{
   rspfString result;

   if (theTemplate.getErrorStatus())
   {
      return result;
   }
   
   vector< rspfRefPtr<rspfXmlNode> > xml_nodes;
   theTemplate.findNodes(xpath, xml_nodes);

   if (xml_nodes.size())
   {
      if (xml_nodes[0].valid())
      {
         result = xml_nodes[0]->getText();
      }
   }
   return result;
}

void rspfFgdcFileWriter::getMetadatatypeList(
   std::vector<rspfString>& metadatatypeList) const
{
   metadatatypeList.push_back(rspfString("rspf_fgdc")); 
}

bool rspfFgdcFileWriter::hasMetadataType(
   const rspfString& metadataType)const
{
   return (metadataType == "rspf_fgdc");
}
