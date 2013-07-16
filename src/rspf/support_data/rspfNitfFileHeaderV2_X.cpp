//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
// 
//----------------------------------------------------------------------------
// $Id: rspfNitfFileHeaderV2_X.cpp 20123 2011-10-11 17:55:44Z dburken $

#include <rspf/support_data/rspfNitfFileHeaderV2_X.h>
#include <rspf/support_data/rspfNitfCommon.h>

#include <iomanip>
#include <sstream>
#include <rspf/base/rspfDate.h> /* for rspfLocalTm */
#include <rspf/base/rspfDateProperty.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfNumericProperty.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/support_data/rspfNitfCommon.h>

RTTI_DEF1(rspfNitfFileHeaderV2_X, "rspfNitfFileHeaderV2_X", rspfNitfFileHeader);
static rspfString monthConversionTable[] = {"   ", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

const rspfString rspfNitfFileHeaderV2_X::FILE_TYPE_KW = "FILE_TYPE";
const rspfString rspfNitfFileHeaderV2_X::VERSION_KW   = "VERSION";
const rspfString rspfNitfFileHeaderV2_X::FHDR_KW      = "FHDR";
const rspfString rspfNitfFileHeaderV2_X::CLEVEL_KW    = "CLEVEL";
const rspfString rspfNitfFileHeaderV2_X::STYPE_KW     = "STYPE";
const rspfString rspfNitfFileHeaderV2_X::OSTAID_KW    = "OSTAID";
const rspfString rspfNitfFileHeaderV2_X::FDT_KW       = "FDT";  
const rspfString rspfNitfFileHeaderV2_X::FTITLE_KW    = "FTITLE";
const rspfString rspfNitfFileHeaderV2_X::FSCLAS_KW    = "FSCLAS";
const rspfString rspfNitfFileHeaderV2_X::FSCODE_KW    = "FSCODE";
const rspfString rspfNitfFileHeaderV2_X::FSCTLH_KW    = "FSCTLH";
const rspfString rspfNitfFileHeaderV2_X::FSREL_KW     = "FSREL";
const rspfString rspfNitfFileHeaderV2_X::FSCAUT_KW    = "FSCAUT";
const rspfString rspfNitfFileHeaderV2_X::FSCTLN_KW    = "FSCTLN";
const rspfString rspfNitfFileHeaderV2_X::FSCOP_KW     = "FSCOP";
const rspfString rspfNitfFileHeaderV2_X::FSCPYS_KW    = "FSCPYS";
const rspfString rspfNitfFileHeaderV2_X::ENCRYP_KW    = "ENCRYP";
const rspfString rspfNitfFileHeaderV2_X::ONAME_KW     = "ONAME";
const rspfString rspfNitfFileHeaderV2_X::OPHONE_KW    = "OPHONE";

rspfNitfFileHeaderV2_X::rspfNitfFileHeaderV2_X()
{
}

void rspfNitfFileHeaderV2_X::setComplexityLevel(const rspfString& level)
{
   rspfNitfCommon::setField(theComplexityLevel, level, 2, std::ios::right, '0');
}

void rspfNitfFileHeaderV2_X::setSystemType(const rspfString& systemType)
{
   rspfNitfCommon::setField(theSystemType, systemType, 4);
}

void rspfNitfFileHeaderV2_X::setOriginatingStationId(const rspfString& originationId)
{
   rspfNitfCommon::setField(theOriginatingStationId, originationId, 10);
}

rspfString rspfNitfFileHeaderV2_X::formatDate(const rspfString& version,
                                                const rspfLocalTm& d)
{
   // Convert to ZULU as per spec for both versions.
   rspfLocalTm d1 = d.convertToGmt();
   
   std::ostringstream out;

   if(version.contains("2.1"))
   {
      out << std::setw(4)
      << std::setfill('0')
      << d1.getYear()
      << std::setw(2)
      << std::setfill('0')
      << d1.getMonth()
      << std::setw(2)
      << std::setfill('0')
      << d1.getDay()
      << std::setw(2)
      << std::setfill('0')
      << d1.getHour()
      << std::setw(2)
      << std::setfill('0')
      << d1.getMin()
      << std::setw(2)
      << std::setfill('0')
      << d1.getSec();
   }
   else
   {
      out  << std::setw(2)
      << std::setfill('0')
      << d1.getDay()
      << std::setw(2)
      << std::setfill('0')
      << d1.getHour()
      << std::setw(2)
      << std::setfill('0')
      << d1.getMin()
      << std::setw(2)
      << std::setfill('0')
      << d1.getSec()
      << "Z"
      <<monthConversionTable[d1.getMonth()]
      << std::setw(2)
      << std::setfill('0')
      <<d1.getShortYear();
      
   }
      
   return out.str();
}

void rspfNitfFileHeaderV2_X::setDate(const rspfLocalTm& d)
{
   memcpy(theDateTime, formatDate(getVersion(), d).c_str(), 14);
}

void rspfNitfFileHeaderV2_X::setDate(const rspfString& d)
{
   if(d.size()==14)
   {
      memcpy(theDateTime, d.c_str(), 14);
   }
}

void rspfNitfFileHeaderV2_X::setDate()
{
   setDate(rspfLocalTm(0));
}  

void rspfNitfFileHeaderV2_X::setTitle(const rspfString& title)
{
   rspfNitfCommon::setField(theFileTitle, title, 80);
}

void rspfNitfFileHeaderV2_X::setFileSecurityClassification(const rspfString& securityClassification)
{
   rspfNitfCommon::setField(theSecurityClassification, securityClassification, 1);
}

void rspfNitfFileHeaderV2_X::setCopyNumber(const rspfString& copyNumber)
{
   if(copyNumber.trim() == "")
   {
      memset(theCopyNumber, '0', 5);
   }
   else
   {
      rspfNitfCommon::setField(theCopyNumber, copyNumber, 5, std::ios::right, '0');
   }
}

void rspfNitfFileHeaderV2_X::setNumberOfCopies(const rspfString& numberOfCopies)
{
   if(numberOfCopies.trim() == "")
   {
      memset(theNumberOfCopies, '0', 5);
   }
   else
   {
      rspfNitfCommon::setField(theNumberOfCopies, numberOfCopies, 5, std::ios::right, '0');
   }
}

void rspfNitfFileHeaderV2_X::setEncryption(const rspfString& encryption)
{
   rspfNitfCommon::setField(theEncryption, encryption, 1);
}

rspfString rspfNitfFileHeaderV2_X::getComplexityLevel()const
{
   return theComplexityLevel;
}

rspfString rspfNitfFileHeaderV2_X::getSystemType()const
{
   return theSystemType;
}

rspfString rspfNitfFileHeaderV2_X::getOriginatingStationId()const
{
   return theOriginatingStationId;
}

rspfString rspfNitfFileHeaderV2_X::getDate()const
{
   return theDateTime;
}

rspfString rspfNitfFileHeaderV2_X::getTitle()const
{
   return theFileTitle;
}

rspfString rspfNitfFileHeaderV2_X::getSecurityClassification()const
{
   return theSecurityClassification;
}

rspfString rspfNitfFileHeaderV2_X::getCopyNumber()const
{
   return theCopyNumber;
}

rspfString rspfNitfFileHeaderV2_X::getNumberOfCopies()const
{
   return theNumberOfCopies;
}

rspfString rspfNitfFileHeaderV2_X::getEncryption()const
{
   return theEncryption;
}

void rspfNitfFileHeaderV2_X::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property.valid()) return;

   rspfString name = property->getName();

   // Make case insensitive:
   name.upcase();

   if(name == FHDR_KW)
   {
      
   }
   else if(name == STYPE_KW)
   {
      setSystemType(property->valueToString());
   }
   else if(name == OSTAID_KW)
   {
      setOriginatingStationId(property->valueToString());
   }
   else if(name == FDT_KW)
   {
      setDate(property->valueToString());
   }
   else if(name == FTITLE_KW)
   {
      setTitle(property->valueToString());
   }
   else if(name == FSCLAS_KW)
   {
      setFileSecurityClassification(property->valueToString());
   }
   else if(name == FSCODE_KW)
   {
      setCodeWords(property->valueToString()); 
   }
   else if(name == FSCTLH_KW)
   {
      setControlAndHandling(property->valueToString());
   }
   else if(name == FSREL_KW)
   {
      setReleasingInstructions(property->valueToString());
   }
   else if(name == FSCAUT_KW)
   {
      setClassificationAuthority(property->valueToString()); 
   }
   else if(name == FSCTLN_KW)
   {
      setSecurityControlNumber(property->valueToString());
   }
   else if(name == FSCOP_KW)
   {
      setCopyNumber(property->valueToString());
   }
   else if(name == FSCPYS_KW)
   {
      setNumberOfCopies(property->valueToString());
   }
   else if(name == ENCRYP_KW)
   {
      setEncryption(property->valueToString());
   }
   else if(name == ONAME_KW)
   {
      setOriginatorsName(property->valueToString());
   }
   else if(name == OPHONE_KW)
   {
      setOriginatorsPhone(property->valueToString());
   }
   else
   {
      rspfNitfFileHeader::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfNitfFileHeaderV2_X::getProperty(const rspfString& name)const
{
   rspfRefPtr<rspfProperty> property = 0;
   
   if(name == FHDR_KW)
   {
      property = new rspfStringProperty(name, rspfString(theFileTypeVersion));
   }
   else if(name == VERSION_KW)
   {
      property = new rspfStringProperty(name, rspfString(getVersion()));
   }
   else if(name == FILE_TYPE_KW)
   {
      property = new rspfStringProperty(name, "NITF");
   }
   else if(name == CLEVEL_KW)
   {
      rspfNumericProperty* numericProperty =
         new rspfNumericProperty(name,
                                  getComplexityLevel(),
                                  1,
                                  99);
      numericProperty->setNumericType(rspfNumericProperty::rspfNumericPropertyType_INT);
      property = numericProperty;
      
   }
   else if(name == STYPE_KW)
   {
      property = new rspfStringProperty(name, getSystemType().trim());
   }
   else if(name == OSTAID_KW)
   {
      property = new rspfStringProperty(name, getOriginatingStationId().trim());
   }
   else if(name == FDT_KW)
   {
      property = new rspfStringProperty(name, getDate());
   }
   else if(name == FTITLE_KW)
   {
      property = new rspfStringProperty(name, getTitle().trim());
   }
   else if(name == FSCLAS_KW)
   {
      rspfStringProperty* stringProperty =
         new rspfStringProperty(name,
                                 getSecurityClassification().trim(),
                                 false);
      
      stringProperty->addConstraint("");
      stringProperty->addConstraint("T");
      stringProperty->addConstraint("S");
      stringProperty->addConstraint("C");
      stringProperty->addConstraint("R");
      stringProperty->addConstraint("U");
      
      property = stringProperty;
   }
   else if(name == FSCODE_KW)
   {
      property = new rspfStringProperty(name,
                                         getCodeWords().trim());
   }
   else if(name == FSCTLH_KW)
   {
      property = new rspfStringProperty(name,
                                         getControlAndHandling().trim());
   }
   else if(name == FSREL_KW)
   {
      property = new rspfStringProperty(name,
                                         getReleasingInstructions().trim());
   }
   else if(name == FSCAUT_KW)
   {
      property = new rspfStringProperty(name,
                                         getClassificationAuthority().trim());
   }
   else if(name == FSCTLN_KW)
   {
      property = new rspfStringProperty(name,
                                         getSecurityControlNumber().trim());
      
   }
   else if(name == FSCOP_KW)
   {
      property = new rspfStringProperty(name,
                                         getCopyNumber().trim());
   }
   else if(name == FSCPYS_KW)
   {
      property = new rspfStringProperty(name,
                                         getNumberOfCopies().trim());
   }
   else if(name == ENCRYP_KW)
   {
      property = new rspfStringProperty(name,
                                         getEncryption().trim(),
                                         false);
   }
   else
   {
      property = rspfNitfFileHeader::getProperty(name).get();
   }
   
   return property;
}

bool rspfNitfFileHeaderV2_X::saveState(rspfKeywordlist& kwl, const rspfString& prefix)const
{
   bool result = rspfNitfFileHeader::saveState(kwl, prefix);
   
   if(result)
   {
      kwl.add(prefix, FHDR_KW.c_str(),   theFileTypeVersion);
      kwl.add(prefix, CLEVEL_KW.c_str(), theComplexityLevel);
      kwl.add(prefix, STYPE_KW.c_str(),  theSystemType);
      kwl.add(prefix, OSTAID_KW.c_str(), theOriginatingStationId);
      kwl.add(prefix, FDT_KW.c_str(),    theDateTime);
      kwl.add(prefix, FTITLE_KW.c_str(), theFileTitle);
      kwl.add(prefix, FSCLAS_KW.c_str(), theSecurityClassification);
      kwl.add(prefix, FSCOP_KW.c_str(),  theCopyNumber);
      kwl.add(prefix, FSCPYS_KW.c_str(), theNumberOfCopies);
      kwl.add(prefix, ENCRYP_KW.c_str(), theEncryption);
   }
   
   return result;
}

bool rspfNitfFileHeaderV2_X::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   // Note: Currently not looking up all fieds only ones that make sense.
   
   const char* lookup;
   
   lookup = kwl.find( prefix, OSTAID_KW);
   if ( lookup )
   {
      setOriginatingStationId( rspfString(lookup) );
   }
   lookup = kwl.find( prefix, FDT_KW);
   if ( lookup )
   {
      setDate( rspfString(lookup) );
   }
   lookup = kwl.find( prefix, FTITLE_KW);
   if ( lookup )
   {
      setTitle( rspfString(lookup) );
   }
   lookup = kwl.find( prefix, FSCLAS_KW);
   if ( lookup )
   {
      setFileSecurityClassification( rspfString(lookup) );
   }
   lookup = kwl.find( prefix, FSCODE_KW);
   if ( lookup )
   {
      setCodeWords( rspfString(lookup) ); 
   }
   lookup = kwl.find( prefix, FSCTLH_KW);
   if ( lookup )
   {
      setControlAndHandling( rspfString(lookup) );
   }
   lookup = kwl.find( prefix, FSREL_KW);
   if ( lookup )
   {
      setReleasingInstructions( rspfString(lookup) );
   }
   lookup = kwl.find( prefix, FSCAUT_KW);
   if ( lookup )
   {
      setClassificationAuthority( rspfString(lookup) );
   }
   lookup = kwl.find( prefix, FSCTLN_KW);
   if ( lookup )
   {
      setSecurityControlNumber( rspfString(lookup) );
   }
   lookup = kwl.find( prefix, FSCOP_KW);
   if ( lookup )
   {
      setCopyNumber( rspfString(lookup) );
   }
   lookup = kwl.find( prefix, FSCPYS_KW);
   if ( lookup )
   {
      setNumberOfCopies( rspfString(lookup) );
   }
   lookup = kwl.find( prefix, ENCRYP_KW);
   if ( lookup )
   {
      setEncryption( rspfString(lookup) );
   }
   lookup = kwl.find( prefix, ONAME_KW);
   if ( lookup )
   {
      setOriginatorsName( rspfString(lookup) );
   }
   lookup = kwl.find( prefix, OPHONE_KW);
   if ( lookup )
   {
      setOriginatorsPhone( rspfString(lookup) );
   }

   return true;
}

void rspfNitfFileHeaderV2_X::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfNitfFileHeader::getPropertyNames(propertyNames);
   propertyNames.push_back(FILE_TYPE_KW);
   propertyNames.push_back(VERSION_KW);
   propertyNames.push_back(FHDR_KW);
   propertyNames.push_back(CLEVEL_KW);
   propertyNames.push_back(STYPE_KW);
   propertyNames.push_back(OSTAID_KW);
   propertyNames.push_back(FDT_KW);
   propertyNames.push_back(FTITLE_KW);
   propertyNames.push_back(FSCLAS_KW);
   propertyNames.push_back(FSCODE_KW);
   propertyNames.push_back(FSCTLH_KW);
   propertyNames.push_back(FSREL_KW);
   propertyNames.push_back(FSCAUT_KW);
   propertyNames.push_back(FSCTLN_KW);
   propertyNames.push_back(FSCOP_KW);
   propertyNames.push_back(FSCPYS_KW);
   propertyNames.push_back(ENCRYP_KW);
   propertyNames.push_back(ONAME_KW);
   propertyNames.push_back(OPHONE_KW);
}
