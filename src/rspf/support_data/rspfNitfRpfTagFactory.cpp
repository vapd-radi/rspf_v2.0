//*******************************************************************
//
// LICENSE: LGPL  see top level LICENSE.txt
// 
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfRpfTagFactory.cpp 16997 2010-04-12 18:53:48Z dburken $
#include <rspf/support_data/rspfNitfRpfTagFactory.h>
#include <rspf/support_data/rspfRpfHeader.h>
#include <rspf/base/rspfNotify.h>

RTTI_DEF1(rspfNitfRpfTagFactory, "rspfNitfRpfTagFactory", rspfNitfTagFactory);

rspfNitfRpfTagFactory* rspfNitfRpfTagFactory::theInstance = 0;

static const rspfString REGISTERED_RPF_HEADER_TAG = "RPFHDR";
static const rspfString REGISTERED_RPF_DES_TAG    = "RPFDES";

rspfNitfRpfTagFactory::rspfNitfRpfTagFactory()
{
   theInstance = this;
}

rspfNitfRpfTagFactory::~rspfNitfRpfTagFactory()
{
   theInstance = 0;
}

rspfNitfRpfTagFactory* rspfNitfRpfTagFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfNitfRpfTagFactory;
   }
   
   return theInstance;
}

rspfRefPtr<rspfNitfRegisteredTag> rspfNitfRpfTagFactory::create(const rspfString &tagName)const
{
   rspfString temp(rspfString(tagName).trim().upcase());
   
   if(temp == REGISTERED_RPF_HEADER_TAG)
   {
      return new rspfRpfHeader;
   }
   if(temp == REGISTERED_RPF_DES_TAG)
   {
      rspfNotify(rspfNotifyLevel_WARN) << "WARNING  rspfNitfRpfTagFactory::create: Reader for REGISTERED_RPF_DES_TAG not implemented yet" << std::endl;
   }
   
   return 0;
}
