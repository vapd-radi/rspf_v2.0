//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
// 
// Author: Garrett Potts
//
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfRegisteredTagFactory.cpp 20689 2012-03-14 18:31:20Z gpotts $

#include <rspf/support_data/rspfNitfRegisteredTagFactory.h>

#include <rspf/support_data/rspfNitfAcftbTag.h>
#include <rspf/support_data/rspfNitfAimidbTag.h>
#include <rspf/support_data/rspfNitfBlockaTag.h>
#include <rspf/support_data/rspfNitfCscrnaTag.h>
#include <rspf/support_data/rspfNitfCsdidaTag.h>
#include <rspf/support_data/rspfNitfCsexraTag.h>
#include <rspf/support_data/rspfNitfCsexraTag.h>
#include <rspf/support_data/rspfNitfEngrdaTag.h>
#include <rspf/support_data/rspfNitfGeoPositioningTag.h>
#include <rspf/support_data/rspfNitfIchipbTag.h>
#include <rspf/support_data/rspfNitfJ2klraTag.h>
#include <rspf/support_data/rspfNitfLocalGeographicTag.h>
#include <rspf/support_data/rspfNitfLocalCartographicTag.h>
#include <rspf/support_data/rspfNitfMstgtaTag.h>
#include <rspf/support_data/rspfNitfPiaimcTag.h>
#include <rspf/support_data/rspfNitfProjectionParameterTag.h>
#include <rspf/support_data/rspfNitfRpcBTag.h>
#include <rspf/support_data/rspfNitfRpcATag.h>
#include <rspf/support_data/rspfNitfSensraTag.h>
#include <rspf/support_data/rspfNitfStdidcTag.h>
#include <rspf/support_data/rspfNitfUse00aTag.h>
#include <rspf/support_data/rspfNitfHistoaTag.h>
#include <rspf/support_data/rspfNitfCsccgaTag.h>
#include <rspf/support_data/rspfNitfCsproaTag.h>
#include <rspf/support_data/rspfNitfExoptaTag.h>

RTTI_DEF1(rspfNitfRegisteredTagFactory, "rspfNitfRegisteredTagFactory", rspfNitfTagFactory);

rspfNitfRegisteredTagFactory* rspfNitfRegisteredTagFactory::theInstance = NULL;

static const char ACFTB_TAG[]                = "ACFTB";
static const char AIMIDB_TAG[]               = "AIMIDB";
static const char BLOCKA_TAG[]               = "BLOCKA";
static const char CSCRNA_TAG[]               = "CSCRNA";
static const char CSDIDA_TAG[]               = "CSDIDA";
static const char CSEXRA_TAG[]               = "CSEXRA";
static const char ENGRDA_TAG[]               = "ENGRDA";
static const char GEO_POSITIONING_TAG[]      = "GEOPSB";
static const char ICHIPB_TAG[]               = "ICHIPB";
static const char J2KLRA_TAG[]               = "J2KLRA";
static const char LOCAL_GEOGRAPHIC_TAG[]     = "GEOLOB";
static const char LOCAL_CARTOGRAPHIC_TAG[]   = "MAPLOB";
static const char MSTGTA_TAG[]               = "MSTGTA";
static const char PIAIMC_TAG[]               = "PIAIMC";
static const char PROJECTION_PARAMETER_TAG[] = "PRJPSB";
static const char RPCB_TAG[]                 = "RPC00B";
static const char RPCA_TAG[]                 = "RPC00A";
static const char SENSRA_TAG[]               = "SENSRA";
static const char STDIDC_TAG[]               = "STDIDC";
static const char USE00A_TAG[]               = "USE00A";
static const char HISTOA_TAG[]               = "HISTOA";
static const char CSCCGA_TAG[]               = "CSCCGA";
static const char CSPROA_TAG[]               = "CSPROA";
static const char EXOPTA_TAG[]               = "EXOPTA";

rspfNitfRegisteredTagFactory::rspfNitfRegisteredTagFactory()
{
   theInstance = this;
}

rspfNitfRegisteredTagFactory::~rspfNitfRegisteredTagFactory()
{
   theInstance = NULL;
}

rspfNitfRegisteredTagFactory* rspfNitfRegisteredTagFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfNitfRegisteredTagFactory;
   }

   return theInstance;
}

rspfRefPtr<rspfNitfRegisteredTag> rspfNitfRegisteredTagFactory::create(
   const rspfString& tagName)const
{
   rspfString name = rspfString(tagName).trim().upcase();
   
   if(tagName == ACFTB_TAG)
   {
      return new rspfNitfAcftbTag;
   }
   else if(tagName == AIMIDB_TAG)
   {
      return new rspfNitfAimidbTag;
   }
   else if(tagName == BLOCKA_TAG)
   {
      return new rspfNitfBlockaTag;
   }
   else if(tagName == CSCRNA_TAG)
   {
      return new rspfNitfCscrnaTag;
   }
   else if(tagName == CSDIDA_TAG)
   {
      return new rspfNitfCsdidaTag;
   }
   else if(tagName == CSEXRA_TAG)
   {
      return new rspfNitfCsexraTag;
   }
   else if(tagName == ENGRDA_TAG)
   {
      return new rspfNitfEngrdaTag;
   }
   else if(tagName == GEO_POSITIONING_TAG)
   {
      return new rspfNitfGeoPositioningTag;
   }
   else if(tagName == ICHIPB_TAG)
   {
      return new rspfNitfIchipbTag;
   }
   else if(tagName == J2KLRA_TAG)
   {
      return new rspfNitfJ2klraTag;
   }
   else if(tagName == LOCAL_GEOGRAPHIC_TAG)
   {
      return new rspfNitfLocalGeographicTag;
   }
   else if(tagName == LOCAL_CARTOGRAPHIC_TAG)
   {
      return new rspfNitfLocalCartographicTag;
   }
   else if(tagName == MSTGTA_TAG)
   {
      return new rspfNitfMstgtaTag;
   }
   else if(tagName == PIAIMC_TAG)
   {
      return new rspfNitfPiaimcTag;
   }
   else if(tagName == PROJECTION_PARAMETER_TAG)
   {
      return new rspfNitfProjectionParameterTag;
   }
   else if(tagName == RPCB_TAG)
   {
      return new rspfNitfRpcBTag;
   }
   else if(tagName == RPCA_TAG)
   {
      return new rspfNitfRpcATag;
   }
   else if (tagName == SENSRA_TAG)
   {
      return new rspfNitfSensraTag;
   }
   else if (tagName == STDIDC_TAG)
   {
      return new rspfNitfStdidcTag;
   }
   else if (tagName == USE00A_TAG)
   {
      return new rspfNitfUse00aTag;
   }
   else if(tagName == HISTOA_TAG)
   {
      return new rspfNitfHistoaTag;
   }
   else if(tagName == CSCCGA_TAG)
   {
      return new rspfNitfCsccgaTag;
   }
   else if(tagName == CSPROA_TAG)
   {
      return new rspfNitfCsproaTag;
   }
   else if(tagName == EXOPTA_TAG)
   {
      return new rspfNitfExoptaTag;
   }
   return NULL;
}
