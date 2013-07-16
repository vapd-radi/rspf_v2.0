//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: DTED Info object.
// 
//----------------------------------------------------------------------------
// $Id$

#include <iostream>
#include <rspf/support_data/rspfDtedInfo.h>
#include <rspf/support_data/rspfDtedVol.h>
#include <rspf/support_data/rspfDtedHdr.h>
#include <rspf/support_data/rspfDtedUhl.h>
#include <rspf/support_data/rspfDtedDsi.h>
#include <rspf/support_data/rspfDtedAcc.h>
#include <rspf/base/rspfErrorContext.h>
#include <rspf/base/rspfProperty.h>
#include <rspf/base/rspfContainerProperty.h>
#include <rspf/base/rspfRegExp.h>


rspfDtedInfo::rspfDtedInfo()
   : theFile()
{
}

rspfDtedInfo::~rspfDtedInfo()
{
}

bool rspfDtedInfo::open(const rspfFilename& file)
{
   bool result = false;

   // Test for extension, like dt0, dt1...
   rspfString ext = file.ext();
   rspfRegExp regExp("^[d|D][t|T][0-9]");
   
   if ( regExp.find( ext.c_str() ) )
   {
      rspfDtedVol vol(file, 0);
      rspfDtedHdr hdr(file, vol.stopOffset());
      rspfDtedUhl uhl(file, hdr.stopOffset());
      rspfDtedDsi dsi(file, uhl.stopOffset());
      rspfDtedAcc acc(file, dsi.stopOffset());
      
      //---
      // Check for errors.  Must have uhl, dsi and acc records.  vol and hdr
      // are for magnetic tape only; hence, may or may not be there.
      //---
      if ( (uhl.getErrorStatus() == rspfErrorCodes::RSPF_OK) &&
           (dsi.getErrorStatus() == rspfErrorCodes::RSPF_OK) &&
           (acc.getErrorStatus() == rspfErrorCodes::RSPF_OK) )
      {
         theFile = file;
         result = true;
      }
      else
      {
         theFile.clear();
      }
   }

   return result;
}

std::ostream& rspfDtedInfo::print(std::ostream& out) const
{
   if ( theFile.size() )
   {
      std::string prefix = "dted.";
      
      rspfDtedVol vol(theFile, 0);
      rspfDtedHdr hdr(theFile, vol.stopOffset());
      rspfDtedUhl uhl(theFile, hdr.stopOffset());
      rspfDtedDsi dsi(theFile, uhl.stopOffset());
      rspfDtedAcc acc(theFile, dsi.stopOffset());
      if( vol.getErrorStatus() == rspfErrorCodes::RSPF_OK )
      {
         vol.print(out, prefix);
      }
      if( hdr.getErrorStatus() == rspfErrorCodes::RSPF_OK )
      {
         hdr.print(out, prefix);
      }
      if( uhl.getErrorStatus() == rspfErrorCodes::RSPF_OK )
      {
         uhl.print(out, prefix);
      }
      if( dsi.getErrorStatus() == rspfErrorCodes::RSPF_OK )
      {
         dsi.print(out, prefix);
      }
      if( acc.getErrorStatus() == rspfErrorCodes::RSPF_OK )
      {
         acc.print(out, prefix);
      }
   }
   return out;
}

rspfRefPtr<rspfProperty> rspfDtedInfo::getProperty(
   const rspfString& name)const
{
   rspfRefPtr<rspfProperty> result = 0;

   //---
   // Look through dted records.
   // Must have uhl, dsi and acc records.  vol and hdr
   // are for magnetic tape only; hence, may or may not be there.
   //---
   rspfDtedVol vol(theFile, 0);
   if( vol.getErrorStatus() == rspfErrorCodes::RSPF_OK )
   {
      if (name == "dted_vol_record")
      {
         rspfContainerProperty* box = new rspfContainerProperty();
         box->setName(name);

         std::vector<rspfString> list;
         vol.getPropertyNames(list);

         std::vector< rspfRefPtr<rspfProperty> > propList;

         std::vector<rspfString>::const_iterator i = list.begin();
         while (i != list.end())
         {
            rspfRefPtr<rspfProperty> prop = vol.getProperty( (*i) );
            if (prop.valid())
            {
               propList.push_back(prop);
            }
            ++i;
         }
         box->addChildren(propList);
         result = box;
      }
   }
   if (result.valid() == false)
   {
      rspfDtedHdr hdr(theFile, vol.stopOffset());
      if( hdr.getErrorStatus() == rspfErrorCodes::RSPF_OK )
      {
         if (name == "dted_hdr_record")
         {
            rspfContainerProperty* box = new rspfContainerProperty();
            box->setName(name);
            
            std::vector<rspfString> list;
            hdr.getPropertyNames(list);
            
            std::vector< rspfRefPtr<rspfProperty> > propList;
            
            std::vector<rspfString>::const_iterator i = list.begin();
            while (i != list.end())
            {
               rspfRefPtr<rspfProperty> prop = hdr.getProperty( (*i) );
               if (prop.valid())
               {
                  propList.push_back(prop);
               }
               ++i;
            }
            box->addChildren(propList);
            result = box;
         }
      }
      if (result.valid() == false)
      {
         rspfDtedUhl uhl(theFile, hdr.stopOffset());
         if( uhl.getErrorStatus() == rspfErrorCodes::RSPF_OK )
         {
            if (name == "dted_uhl_record")
            {
               rspfContainerProperty* box = new rspfContainerProperty();
               box->setName(name);
               
               std::vector<rspfString> list;
               uhl.getPropertyNames(list);
               
               std::vector< rspfRefPtr<rspfProperty> > propList;
               
               std::vector<rspfString>::const_iterator i = list.begin();
               while (i != list.end())
               {
                  rspfRefPtr<rspfProperty> prop = uhl.getProperty( (*i) );
                  if (prop.valid())
                  {
                     propList.push_back(prop); 
                  }
                  ++i;
               }
               box->addChildren(propList);
               result = box;
            }
         }
         if (result.valid() == false)
         {
            rspfDtedDsi dsi(theFile, uhl.stopOffset());
            if( dsi.getErrorStatus() == rspfErrorCodes::RSPF_OK )
            {
               if (name == "dted_dsi_record")
               {
                  rspfContainerProperty* box =
                     new rspfContainerProperty();
                  box->setName(name);
                  
                  std::vector<rspfString> list;
                  dsi.getPropertyNames(list);
                  
                  std::vector< rspfRefPtr<rspfProperty> > propList;
                  
                  std::vector<rspfString>::const_iterator i = list.begin();
                  while (i != list.end())
                  {
                     rspfRefPtr<rspfProperty> prop =
                        dsi.getProperty( (*i) );
                     if (prop.valid())
                     {
                        propList.push_back(prop);
                     }
                     ++i;
                  }
                  box->addChildren(propList);
                  result = box;
               }
            }
            if (result.valid() == false)
            {
               rspfDtedAcc acc(theFile, dsi.stopOffset());
               if( acc.getErrorStatus() == rspfErrorCodes::RSPF_OK )
               {
                  if (name == "dted_acc_record")
                  {
                     rspfContainerProperty* box =
                        new rspfContainerProperty();
                     box->setName(name);
                     
                     std::vector<rspfString> list;
                     acc.getPropertyNames(list);
                     
                     std::vector< rspfRefPtr<rspfProperty> > propList;
                     
                     rspfRefPtr<rspfProperty> prop = 0;
                     std::vector<rspfString>::const_iterator i =
                        list.begin();
                     while (i != list.end())
                     {
                        rspfRefPtr<rspfProperty> prop =
                           acc.getProperty( (*i) );
                        if (prop.valid())
                        {
                           propList.push_back(prop);
                        }
                        ++i;
                     }
                     box->addChildren(propList);
                     result = box;
                  }
               }
            }
         }
      }
   }

   return result;
}

void rspfDtedInfo::getPropertyNames(
   std::vector<rspfString>& propertyNames)const
{
   propertyNames.push_back(rspfString("dted_vol_record"));
   propertyNames.push_back(rspfString("dted_hdr_record"));
   propertyNames.push_back(rspfString("dted_uhl_record"));
   propertyNames.push_back(rspfString("dted_dsi_record"));
   propertyNames.push_back(rspfString("dted_acc_record"));
}


