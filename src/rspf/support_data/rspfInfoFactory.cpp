//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Factory for info objects.
// 
//----------------------------------------------------------------------------
// $Id: rspfInfoFactory.cpp 22118 2013-01-18 21:05:14Z dburken $

#include <rspf/support_data/rspfInfoFactory.h>
#include <rspf/support_data/rspfInfoBase.h>
#include <rspf/support_data/rspfCcfInfo.h>
#include <rspf/support_data/rspfDemInfo.h>
#include <rspf/support_data/rspfDoqq.h>
#include <rspf/support_data/rspfDtedInfo.h>
#include <rspf/support_data/rspfEnviInfo.h>
#include <rspf/support_data/rspfJ2kInfo.h>
#include <rspf/support_data/rspfJp2Info.h>
#include <rspf/support_data/rspfLasInfo.h>
#include <rspf/support_data/rspfNitfInfo.h>
#include <rspf/support_data/rspfRpfInfo.h>
#include <rspf/support_data/rspfTiffInfo.h>
#include <rspf/support_data/rspfXmpInfo.h>

#include <rspf/base/rspfFilename.h>

rspfInfoFactory* rspfInfoFactory::theInstance = 0;

rspfInfoFactory::~rspfInfoFactory()
{}

rspfInfoFactory* rspfInfoFactory::instance()
{
   if ( !theInstance )
   {
      theInstance = new rspfInfoFactory;
   }
   return theInstance;
}

rspfInfoBase* rspfInfoFactory::create(const rspfFilename& file) const
{
   rspfRefPtr<rspfInfoBase> result = 0;

   result = new rspfTiffInfo();
   if ( result->open(file) )
   {
      return result.release();
   }

   result = new rspfJp2Info();
   if ( result->open(file) )
   {
      return result.release();
   }

   result = new rspfNitfInfo();
   if ( result->open(file) )
   {
      return result.release();
   }

   result = new rspfDemInfo();
   if ( result->open(file) )
   {
      return result.release();
   }

   result = new rspfDtedInfo();
   if ( result->open(file) )
   {
      return result.release();
   }

   result = new rspfEnviInfo();
   if ( result->open(file) )
   {
      return result.release();
   }

   result = new rspfRpfInfo();
   if ( result->open(file) )
   {
      return result.release();
   }
   
   result = new rspfXmpInfo();
   if ( result->open(file) )
   {
      return result.release();
   }
   
   result = new rspfLasInfo();
   if ( result->open(file) )
   {
      return result.release();
   }

   result = new rspfDoqq();
   if ( result->open(file) )
   {
      return result.release();
   }

   result = new rspfJ2kInfo();
   if ( result->open(file) )
   {
      return result.release();
   }
   
   result = new rspfCcfInfo();
   if ( result->open(file) )
   {
      return result.release();
   }
   
   return 0;
}

rspfInfoFactory::rspfInfoFactory()
{}

rspfInfoFactory::rspfInfoFactory(const rspfInfoFactory& /* obj */ )
{}

const rspfInfoFactory& rspfInfoFactory::operator=(
   const rspfInfoFactory& /* rhs */)
{
   return *this;
}
