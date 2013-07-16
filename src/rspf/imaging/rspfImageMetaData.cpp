//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts
//
// Description:
//
// Contains class definition for rspfImageMetaData.
// 
//*******************************************************************
//  $Id: rspfImageMetaData.cpp 21745 2012-09-16 15:21:53Z dburken $

#include <rspf/imaging/rspfImageMetaData.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfScalarTypeLut.h>
#include <rspf/base/rspfTrace.h>
#include <algorithm>
#include <iostream>
#include <vector>

static rspfTrace traceDebug("rspfImageMetaData:debug");

rspfImageMetaData::rspfImageMetaData()
   :theNullPixelArray(0),
    theMinPixelArray(0),
    theMaxPixelArray(0),
    theMinValuesValidFlag(false),
    theMaxValuesValidFlag(false),
    theNullValuesValidFlag(false),
    theScalarType(RSPF_SCALAR_UNKNOWN),
    theBytesPerPixel(0),
    theNumberOfBands(0)
{
}

rspfImageMetaData::rspfImageMetaData(rspfScalarType aType,
				       rspf_uint32 numberOfBands)
   :theNullPixelArray(0),
    theMinPixelArray(0),
    theMaxPixelArray(0),
    theMinValuesValidFlag(false),
    theMaxValuesValidFlag(false),
    theNullValuesValidFlag(false),
    theScalarType(aType),
    theBytesPerPixel(0),
    theNumberOfBands(numberOfBands)
{
   if(theNumberOfBands)
   {
      theNullPixelArray = new double[theNumberOfBands];
      theMinPixelArray  = new double[theNumberOfBands];
      theMaxPixelArray  = new double[theNumberOfBands];
      
      setDefaultsForArrays();
   }
   theBytesPerPixel = rspf::scalarSizeInBytes( aType );
}

rspfImageMetaData::rspfImageMetaData(const rspfImageMetaData& rhs)
   :
   theNullPixelArray(0),
   theMinPixelArray(0),
   theMaxPixelArray(0),
   theMinValuesValidFlag(rhs.theMinValuesValidFlag),
   theMaxValuesValidFlag(rhs.theMaxValuesValidFlag),
   theNullValuesValidFlag(rhs.theNullValuesValidFlag),
   theScalarType(rhs.theScalarType),
   theBytesPerPixel(rhs.theBytesPerPixel),
   theNumberOfBands(rhs.theNumberOfBands)
{
   if(theNumberOfBands)
   {
      theNullPixelArray = new double[theNumberOfBands];
      theMinPixelArray  = new double[theNumberOfBands];
      theMaxPixelArray  = new double[theNumberOfBands];
      
      std::copy(rhs.theNullPixelArray,
                rhs.theNullPixelArray+theNumberOfBands,
                theNullPixelArray);
      std::copy(rhs.theMinPixelArray,
                rhs.theMinPixelArray+theNumberOfBands,
                theMinPixelArray);
      std::copy(rhs.theMaxPixelArray,
                rhs.theMaxPixelArray+theNumberOfBands,
                theMaxPixelArray);
   }
}

const rspfImageMetaData& rspfImageMetaData::operator=( const rspfImageMetaData& rhs )
{
   if( this != &rhs )
   {
      clear();
      theMinValuesValidFlag  = rhs.theMinValuesValidFlag;
      theMaxValuesValidFlag  = rhs.theMaxValuesValidFlag;
      theNullValuesValidFlag = rhs.theNullValuesValidFlag;
      theNumberOfBands       = rhs.theNumberOfBands;
      theScalarType          = rhs.theScalarType;
      theBytesPerPixel       = rhs.theBytesPerPixel;
      if(theNumberOfBands)
      {
         theNullPixelArray = new double[theNumberOfBands];
         theMinPixelArray  = new double[theNumberOfBands];
         theMaxPixelArray  = new double[theNumberOfBands];
         
         std::copy(rhs.theNullPixelArray,
                   rhs.theNullPixelArray+theNumberOfBands,
                   theNullPixelArray);
         std::copy(rhs.theMinPixelArray,
                   rhs.theMinPixelArray+theNumberOfBands,
                   theMinPixelArray);
         std::copy(rhs.theMaxPixelArray,
                   rhs.theMaxPixelArray+theNumberOfBands,
                   theMaxPixelArray);
      }
   }
   return *this;
}

rspfImageMetaData::~rspfImageMetaData()
{
  clear();
}

void rspfImageMetaData::clear()
{
   if(theNullPixelArray)
   {
      delete [] theNullPixelArray;
      theNullPixelArray = 0;
   }
   if(theMinPixelArray)
   {
      delete [] theMinPixelArray;
      theMinPixelArray = 0;
   }
   if(theMaxPixelArray)
   {
      delete [] theMaxPixelArray;
      theMaxPixelArray = 0;
   }
   theScalarType    = RSPF_SCALAR_UNKNOWN;
   theBytesPerPixel = 0;
   theNumberOfBands = 0;
}

void rspfImageMetaData::setDefaultsForArrays()
{
   rspf_uint32 i=0;
   
   for(i = 0; i < theNumberOfBands; ++i)
   {
      theNullPixelArray[i] = rspf::defaultNull(theScalarType);
      theMinPixelArray[i]  = rspf::defaultMin(theScalarType);
      theMaxPixelArray[i]  = rspf::defaultMax(theScalarType);
   }
}

void rspfImageMetaData::setNumberOfBands(rspf_uint32 numberOfBands)
{
   if(theNullPixelArray)
   {
      delete [] theNullPixelArray;
      theNullPixelArray = 0;
   }
   if(theMinPixelArray)
   {
      delete [] theMinPixelArray;
      theMinPixelArray = 0;
   }
   if(theMaxPixelArray)
   {
      delete [] theMaxPixelArray;
      theMaxPixelArray = 0;
   }
   
   theNumberOfBands = numberOfBands;

   if(theNumberOfBands)
   {
      theNullPixelArray = new double[theNumberOfBands];
      theMinPixelArray  = new double[theNumberOfBands];
      theMaxPixelArray  = new double[theNumberOfBands];
   
      setDefaultsForArrays();
   }
}

bool rspfImageMetaData::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   bool result = false; // return status
   
   // Clear the object:
   clear();

   std::string pfx = ( prefix ? prefix : "" );
   
   rspf_uint32 bands = getBandCount( kwl, pfx );
   if ( bands )
   {
      result = true;

      setNumberOfBands( bands );
      
      theMinValuesValidFlag  = true;
      theMaxValuesValidFlag  = true;
      theNullValuesValidFlag = true;

      updateMetaData( kwl, pfx ); // Initializes the rest of object.
   }
   else
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << " ERROR:\n"
            << "Required keyword not found:  " << rspfKeywordNames::NUMBER_BANDS_KW << std::endl;
      }
   }
   
   if (traceDebug())
   {
      print( rspfNotify(rspfNotifyLevel_DEBUG) );
   }

   return result;
}

bool rspfImageMetaData::saveState(rspfKeywordlist& kwl,
				   const char* prefix)const
{
   if ( theNumberOfBands )
   {
      rspf_uint32 band = 0;
      
      for (band=0; band < theNumberOfBands; ++band)
      {
         rspfString kwMin = (rspfKeywordNames::BAND_KW +
                              rspfString::toString(band+1) + "." +
                              rspfKeywordNames::MIN_VALUE_KW);
         rspfString kwMax = (rspfKeywordNames::BAND_KW +
                              rspfString::toString(band+1) + "." +
                              rspfKeywordNames::MAX_VALUE_KW);
         rspfString kwNull = (rspfKeywordNames::BAND_KW +
                               rspfString::toString(band+1) + "." +
                               rspfKeywordNames::NULL_VALUE_KW);
         
         kwl.add(prefix,
                 kwMin.c_str(),
                 theMinPixelArray[band],
                 true);
         kwl.add(prefix,
                 kwMax.c_str(),
                 theMaxPixelArray[band],
                 true);
         kwl.add(prefix,
                 kwNull.c_str(),
                 theNullPixelArray[band],
                 true);
      }
      // std::cout << "added bands ******************" << std::endl;
      kwl.add(prefix,
	      rspfKeywordNames::NUMBER_BANDS_KW,
	      theNumberOfBands,
	      true);
      kwl.add(prefix,
	      rspfKeywordNames::SCALAR_TYPE_KW,
	      (rspfScalarTypeLut::instance()->getEntryString(theScalarType)),
	      true);

      kwl.add( prefix, "bytes_per_pixel", theBytesPerPixel, true );
   }
   
   return true;
}

rspf_uint32 rspfImageMetaData::getNumberOfBands()const
{
   return theNumberOfBands;
}

void rspfImageMetaData::setScalarType(rspfScalarType aType)
{
   theScalarType    = aType;
   theBytesPerPixel = rspf::scalarSizeInBytes( aType );
}

rspfScalarType rspfImageMetaData::getScalarType()const
{
   return theScalarType;
}

rspf_uint32 rspfImageMetaData::getBytesPerPixel() const
{
   return theBytesPerPixel;
}

void rspfImageMetaData::setMinPix(rspf_uint32 band, double pix)
{
   if((band < theNumberOfBands)&&(theMinPixelArray))
   {
      theMinPixelArray[band] = pix;
   }
}

void rspfImageMetaData::setMaxPix(rspf_uint32 band, double pix)
{
   if((band < theNumberOfBands) && theMaxPixelArray)
   {
      theMaxPixelArray[band] = pix;
   }
}

void rspfImageMetaData::setNullPix(rspf_uint32 band, double pix)
{
   if((band < theNumberOfBands)&&(theNullPixelArray))
   {
      theNullPixelArray[band] = pix;
   }
}

double rspfImageMetaData::getMinPix(rspf_uint32 band)const
{
   if(theNumberOfBands&&theMinPixelArray)
   {
      rspf_uint32 i = rspf::min(band, (rspf_uint32)(theNumberOfBands-1));
      
      return theMinPixelArray[i];
   }
   return rspf::defaultMin(theScalarType);
}

double rspfImageMetaData::getMaxPix(rspf_uint32 band)const
{
   if(theNumberOfBands&&theMaxPixelArray)
   {
      rspf_uint32 i = rspf::min(band, (rspf_uint32)(theNumberOfBands-1));
      
      return theMaxPixelArray[i];
   }
   return rspf::defaultMax(theScalarType);
}

double rspfImageMetaData::getNullPix(rspf_uint32 band)const
{
   if(theNumberOfBands&&theNullPixelArray)
   {
      rspf_uint32 i = rspf::min(band, (rspf_uint32)(theNumberOfBands-1));
      
      return theNullPixelArray[i];
   }
   return rspf::defaultNull(theScalarType);
}

const double* rspfImageMetaData::getMinPixelArray()const
{
   return theMinPixelArray;
}
const double* rspfImageMetaData::getMaxPixelArray()const
{
   return theMaxPixelArray;
}

const double* rspfImageMetaData::getNullPixelArray()const
{
   return theNullPixelArray;
}

void rspfImageMetaData::setMinValuesValid(bool flag)
{
   theMinValuesValidFlag = flag;
}

void rspfImageMetaData::setMaxValuesValid(bool flag)
{
   theMaxValuesValidFlag = flag;
}

void rspfImageMetaData::setNullValuesValid(bool flag)
{
   theNullValuesValidFlag = flag;
}

bool rspfImageMetaData::getMinValuesValidFlag()const
{
   return theMinValuesValidFlag;
}

bool rspfImageMetaData::getMaxValuesValidFlag()const
{
   return theMaxValuesValidFlag;
}

bool rspfImageMetaData::getNullValuesValidFlag()const
{
   return theNullValuesValidFlag;
}

bool rspfImageMetaData::isValid()const
{
   return (theNumberOfBands > 0);
//    return ((theNumberOfBands>0)&&
//            (theScalarType != RSPF_SCALAR_UNKNOWN));
}

void rspfImageMetaData::updateMetaData(
   const rspfKeywordlist& kwl, const std::string& prefix )
{
   rspfString value;
   std::string key;
   
   // Get the bands:
   if ( !theNumberOfBands )
   {
      rspf_uint32 bands = getBandCount( kwl, prefix );
      if ( bands )
      {
         setNumberOfBands( bands );
      }
   }

   if ( theNumberOfBands )
   {
      //---
      // See if bands are zero or one based.  Bands are ONE based in the "omd"
      // file! If band0 is present assume zero based.
      //---

      rspfString regExpression = std::string("^(") + prefix + std::string("band0+.)");
      std::vector<rspfString> keys = kwl.getSubstringKeyList( regExpression );

      rspf_uint32 startBand = (keys.size() ? 0 : 1);

      rspf_uint32 limit = theNumberOfBands+startBand; // For for loop:
      
      std::string bs = "band"; // band sting
      std::string base;
      
      for ( rspf_uint32 band = startBand; band < limit; ++band )
      {
         base = bs + rspfString::toString( band ).string() + std::string(".") ;

         // Min:
         key = base + std::string(rspfKeywordNames::MIN_VALUE_KW);
         value =  kwl.findKey( prefix, key );
         
         if ( value.size() )
         {
            theMinPixelArray[ band - startBand  ] = value.toFloat64();
         }

         // Max:
         key = base + std::string(rspfKeywordNames::MAX_VALUE_KW);
         value =  kwl.findKey( prefix, key );
         if ( value.size() )
         {
            theMaxPixelArray[ band - startBand ] = value.toFloat64();
         }

         // Null:
         key = base + std::string(rspfKeywordNames::NULL_VALUE_KW);
         value =  kwl.findKey( prefix, key );
         if ( value.size() )
         {
            theNullPixelArray[ band - startBand ] = value.toFloat64();
         }
      }  
   }

   // Scalar, only look for if not set.
   if ( theScalarType == RSPF_SCALAR_UNKNOWN )
   {
      std::string key = rspfKeywordNames::SCALAR_TYPE_KW; // "scalar_type"
      value.string() = kwl.findKey( prefix, key );
      if ( value.empty() )
      {
         key = "radiometry";
         value.string() = kwl.findKey( prefix, key );
      }
      if ( value.size() )
      {
         theScalarType = rspfScalarTypeLut::instance()->getScalarTypeFromString( value );
      }
      
      if ( theScalarType != RSPF_SCALAR_UNKNOWN )
      {
         theBytesPerPixel = rspf::scalarSizeInBytes( theScalarType );
      }
   }

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfImageMetaData::updateMetaData DEBUG"
         << *this << std::endl;
   }
}

std::ostream& rspfImageMetaData::print(std::ostream& out) const
{
   rspfKeywordlist kwl;
   saveState( kwl, 0 );
   out << kwl << std::endl;
   return out;
}

std::ostream& operator<<(std::ostream& out, const rspfImageMetaData& obj)
{
   return obj.print( out );
}

rspf_uint32 rspfImageMetaData::getBandCount(const rspfKeywordlist& kwl,
                                              const std::string& prefix) const
{
   rspf_uint32 result = 0;

   rspfString value;
   value.string() = kwl.findKey( prefix, std::string(rspfKeywordNames::NUMBER_BANDS_KW) );
   if ( value.size() )
   {
      result = value.toUInt32();
   }
   else
   {
      value.string() = std::string("^(") + prefix + std::string("band[0-9]+.)");
      std::vector<rspfString> keys = kwl.getSubstringKeyList( value );
      result = static_cast<rspf_uint32>( keys.size() );
   }
   return result;   
}
