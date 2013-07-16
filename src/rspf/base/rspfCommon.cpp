//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: David Burken
//
// Description: Common file for global functions.
//
//*************************************************************************
// $Id: rspfCommon.cpp 20610 2012-02-27 12:19:25Z gpotts $

#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfPreferences.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfDpt3d.h>
#include <rspf/matrix/newmat.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfGpt.h>
#include <OpenThreads/Thread>
#include <sstream>

static rspfTrace traceDebug("rspfCommon:debug");

// stores a floating point nan value
const rspf::IntFloatBitCoercion rspf::nanValue(~rspf_int64(0));

std::istream& rspf::skipws(std::istream& in)
{
   int c = in.peek();
   while( !in.bad() && rspf::isWhiteSpace(c))
   {
      in.ignore();
      c = in.peek();
   }
   
   return in;
}
bool rspf::isWhiteSpace(int c)
{
   return ( (c == ' ') || (c == '\t') || (c == '\n')|| (c == '\r') ) ;
}

rspfByteOrder rspf::byteOrder()
{
   union
   {
      short s;
      char  c[sizeof(short)];
   } un;

   un.s = 0x0102;
   if (un.c[0] ==  2 && un.c[1] == 1)
   {
      return RSPF_LITTLE_ENDIAN;
   }
   else
   {
      return RSPF_BIG_ENDIAN;
   }
}

double rspf::defaultMin(rspfScalarType scalarType)
{
   switch(scalarType)
   {
      case RSPF_UINT8:
      {
	return RSPF_DEFAULT_MIN_PIX_UINT8;
      }
      case RSPF_SINT8:
      {
	return RSPF_DEFAULT_MIN_PIX_SINT8;
      }
      case RSPF_UINT16:
      {
         return RSPF_DEFAULT_MIN_PIX_UINT16;
      }
      case RSPF_SINT16:
      {
         return RSPF_DEFAULT_MIN_PIX_SINT16;
      }
      case RSPF_USHORT11:
      {
         return RSPF_DEFAULT_MIN_PIX_UINT11;
      }
      case RSPF_UINT32:
      {
         return RSPF_DEFAULT_MIN_PIX_UINT32;
      }
      case RSPF_SINT32:
      {
         return RSPF_DEFAULT_MIN_PIX_SINT32;
      }
      case RSPF_FLOAT32:
      {
         return RSPF_DEFAULT_MIN_PIX_FLOAT;
      }
      case RSPF_NORMALIZED_FLOAT:
      {
         return RSPF_DEFAULT_MIN_PIX_NORM_FLOAT;
      }
      case RSPF_FLOAT64:
      {
         return RSPF_DEFAULT_MIN_PIX_DOUBLE;
      }
      case RSPF_NORMALIZED_DOUBLE:
      {
         return RSPF_DEFAULT_MIN_PIX_NORM_DOUBLE;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << __FILE__ << ":" << __LINE__
               << "\nUnhandled scalar type:  " << scalarType << std::endl;
         }
         break;
      }
   }
   return 0.0; // Should never happen...
}

double rspf::defaultMax(rspfScalarType scalarType)
{
   switch(scalarType)
   {
      case RSPF_UINT8:
      {
         return RSPF_DEFAULT_MAX_PIX_UINT8;
      }
      case RSPF_SINT8:
      {
         return RSPF_DEFAULT_MAX_PIX_SINT8;
      }
      case RSPF_UINT16:
      {
	return RSPF_DEFAULT_MAX_PIX_UINT16;
      }
      case RSPF_SINT16:
      {
         return RSPF_DEFAULT_MAX_PIX_SINT16;
      }
      case RSPF_USHORT11:
      {
         return RSPF_DEFAULT_MAX_PIX_UINT11;
      }
      case RSPF_UINT32:
      {
	return RSPF_DEFAULT_MAX_PIX_UINT32;
      }
      case RSPF_SINT32:
      {
         return RSPF_DEFAULT_MAX_PIX_SINT32;
      }
      case RSPF_FLOAT32:
      {
         return RSPF_DEFAULT_MAX_PIX_FLOAT;
      }
      case RSPF_NORMALIZED_FLOAT:
      {
         return RSPF_DEFAULT_MAX_PIX_NORM_FLOAT;
      }
      case RSPF_FLOAT64:
      {
         return RSPF_DEFAULT_MAX_PIX_DOUBLE;
      }
      case RSPF_NORMALIZED_DOUBLE:
      {
         return RSPF_DEFAULT_MAX_PIX_NORM_DOUBLE;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << __FILE__ << ":" << __LINE__
               << "\nUnhandled scalar type:  " << scalarType << std::endl;
         }
         break;
      }
   }
   return 0.0; // Should never happen...
}

double rspf::defaultNull(rspfScalarType scalarType)
{
   switch(scalarType)
   {
      case RSPF_UINT8:
      {
         return RSPF_DEFAULT_NULL_PIX_UINT8;
      }
      case RSPF_SINT8:
      {
         return RSPF_DEFAULT_NULL_PIX_SINT8;
      }
      case RSPF_UINT16:
      {
         return RSPF_DEFAULT_NULL_PIX_UINT16;
      }
      case RSPF_SINT16:
      {
         return RSPF_DEFAULT_NULL_PIX_SINT16;
      }
      case RSPF_USHORT11:
      {
         return RSPF_DEFAULT_NULL_PIX_UINT11;
      }
      case RSPF_UINT32:
      {
         return RSPF_DEFAULT_NULL_PIX_UINT32;
      }
      case RSPF_SINT32:
      {
         return RSPF_DEFAULT_NULL_PIX_SINT32;
      }
      case RSPF_FLOAT32:
      {
         return RSPF_DEFAULT_NULL_PIX_FLOAT;
      }
      case RSPF_NORMALIZED_FLOAT:
      {
         return RSPF_DEFAULT_NULL_PIX_NORM_FLOAT;
      }
      case RSPF_FLOAT64:
      {
         return RSPF_DEFAULT_NULL_PIX_DOUBLE;
      }
      case RSPF_NORMALIZED_DOUBLE:
      {
         return RSPF_DEFAULT_NULL_PIX_NORM_DOUBLE;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << __FILE__ << ":" << __LINE__
               << "\nUnhandled scalar type:  " << scalarType << std::endl;
         }
         break;
      }
    }

   return 0.0; // Should never happen...
}

rspf_uint32 rspf::scalarSizeInBytes(rspfScalarType scalarType)
{
   switch(scalarType)
   {
      case RSPF_UINT8:
      {
         return sizeof(rspf_uint8);
      }
      case RSPF_SINT8:
      {
         return sizeof(rspf_sint8);
      }
      case RSPF_UINT16:
      case RSPF_USHORT11:
      {
         return sizeof(rspf_uint16);
      }
      case RSPF_SINT16:
      {
         return sizeof(rspf_sint16);
      }
      case RSPF_UINT32:
      {
         return sizeof(rspf_uint32);
      }
      case RSPF_SINT32:
      {
         return sizeof(rspf_sint32);
      }
      case RSPF_FLOAT32:
      case RSPF_NORMALIZED_FLOAT:
      {
         return sizeof(rspf_float32);
      }
      case RSPF_FLOAT64:
      case RSPF_NORMALIZED_DOUBLE:
      {
         return sizeof(rspf_float64);
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << __FILE__ << ":" << __LINE__
               << "\nUnhandled scalar type:  " << scalarType << std::endl;
         }
         break;
      }
  }
  
  return 1;
}

bool rspf::isSigned(rspfScalarType scalarType)
{
   bool result = false;
   switch(scalarType)
   {
      case RSPF_SINT8:
      case RSPF_SINT16:
      case RSPF_SINT32:
      case RSPF_FLOAT32:
      case RSPF_FLOAT64:
      {
         result = true;
         break;
      }
      default:
      {
         break;
      }
  }
  return result;
}

rspf_uint32 rspf::getActualBitsPerPixel(rspfScalarType scalarType)
{
   rspf_uint32 actualBitsPerPixel = 0;
   switch(scalarType)
   {
      case RSPF_UINT8:
      case RSPF_SINT8:
      {
         actualBitsPerPixel = 8;
         break;
      }
      case RSPF_USHORT11:
      {
         actualBitsPerPixel = 11;
         break;
      }
      case RSPF_UINT16:
      case RSPF_SINT16:
      {
         actualBitsPerPixel = 16;
         break;
      }
      case RSPF_UINT32:
      case RSPF_SINT32:
      case RSPF_FLOAT32:
      case RSPF_CINT16:
      case RSPF_NORMALIZED_FLOAT:
      {
         actualBitsPerPixel = 32;
         break;
      }
      case RSPF_FLOAT64:
      case RSPF_CFLOAT32:
      case RSPF_CINT32:
      case RSPF_NORMALIZED_DOUBLE:
      {
         actualBitsPerPixel = 64;
         break;
      }
      case RSPF_CFLOAT64:
      {
         actualBitsPerPixel = 128;
         
         break;
      }
      default:
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << __FILE__ << ":" << __LINE__
               << "\nUnhandled scalar type:  " << scalarType << std::endl;
         }
         break;
      }
   }
   return actualBitsPerPixel;
}

rspf_uint32 rspf::getBitsPerPixel(rspfScalarType scalarType)
{
   rspf_uint32 bitsPerPixel = 0;
   switch(scalarType)
   {
      case RSPF_UINT8:
      case RSPF_SINT8:
      {
         bitsPerPixel = 8;
         break;
      }
      case RSPF_USHORT11:
      {
         bitsPerPixel = 16;
         break;
      }
      case RSPF_UINT16:
      case RSPF_SINT16:
      {
         bitsPerPixel = 16;
         break;
      }
      case RSPF_UINT32:
      case RSPF_SINT32:
      case RSPF_FLOAT32:
      case RSPF_NORMALIZED_FLOAT:
      {
         bitsPerPixel = 32;
         break;
      }
      case RSPF_FLOAT64:
      case RSPF_NORMALIZED_DOUBLE:
      {
         bitsPerPixel = 64;
         break;
      }
      default:
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << __FILE__ << ":" << __LINE__
               << "\nUnhandled scalar type:  " << scalarType << std::endl;
         }
         break;
      }
   }
   return bitsPerPixel;
}

void rspf::defaultTileSize(rspfIpt& tileSize)
{
   const char* tileSizeKw = rspfPreferences::instance()->
      findPreference("tile_size");

   if(tileSizeKw)
   {
      std::vector<rspfString> splitArray;
      rspfString tempString(tileSizeKw);
      tempString.split(splitArray, " ");
      bool hasX = true;
    if(splitArray.size() == 2)
      {
         tileSize.x = splitArray[0].toInt32();
         tileSize.y = splitArray[1].toInt32();
      }
      else if(splitArray.size() == 1)
      {
         tileSize.x = splitArray[0].toInt32();
         tileSize.y = splitArray[0].toInt32();
      }
      else
      {
         tileSize = rspfIpt(0,0);
      }
      if(tileSize.x < 1)
      {
         tileSize.x = RSPF_DEFAULT_TILE_WIDTH;
         hasX = false;
      }
      if(tileSize.y < 1)
      {
         if(!hasX)
         {
            tileSize.y = RSPF_DEFAULT_TILE_HEIGHT;
         }
         else
         {
            tileSize.y = tileSize.x;
         }
      }
   }
   else
   {
      tileSize.x = RSPF_DEFAULT_TILE_WIDTH;
      tileSize.y = RSPF_DEFAULT_TILE_HEIGHT;
   }
}

std::string rspf::convertHtmlSpecialCharactersToNormalCharacter(const std::string& src)
{
   rspfString result = src;
   std::string::size_type pos = 0;
   pos = result.find("&");
   
   while(pos != std::string::npos)
   {
      std::string::size_type size = result.size();
      std::string test1(&result[pos], rspf::min(6, (int)(size-pos)));
      std::string test2(&result[pos], rspf::min(5, (int)(size-pos)));
      std::string test3(&result[pos], rspf::min(4, (int)(size-pos)));
      
      if(test1 == "&apos;")
      {
         result = result.substitute(test1, "'");
      }
      else if(test1 == "&quot;")
      {
         result = result.substitute(test1, "\"");
      }
      else if(test2 == "&amp;")
      {
         result = result.substitute(test2, "&");
      }
      else if(test3 == "&gt;")
      {
         result = result.substitute(test3, ">");
      }
      else if(test3 == "&lt;")
      {
         result = result.substitute(test3, "<");
      }
      pos = result.find("&", pos+1);
   }
   
   return result;
}

bool rspf::matrixToHpr( rspf_float64 hpr[3], const NEWMAT::Matrix& rotation )
{
    //implementation converted from plib's sg.cxx
    //PLIB - A Suite of Portable Game Libraries
    //Copyright (C) 1998,2002  Steve Baker
    //For further information visit http://plib.sourceforge.net
    
   NEWMAT::Matrix mat(rotation);
    
   rspfDpt3d col1(rotation[0][0], rotation[1][0], rotation[2][0]);
   double s = col1.length();
   
   hpr[0] = 0.0;
   hpr[1] = 0.0;
   hpr[2] = 0.0;

   if ( s <= 0.00001 )
   {
      return true;
   }
   
   
   double oneOverS = 1.0f / s;
   for( int i = 0; i < 3; i++ )
      for( int j = 0; j < 3; j++ )
         mat[i][j] = rotation[j][i] * oneOverS;
   
   
   hpr[1] = rspf::asind(rspf::clamp(mat[1][2], -1.0, 1.0));
   
   double cp = rspf::cosd(hpr[1]);
   
   if ( cp > -0.00001 && cp < 0.00001 )
   {
      double cr = rspf::clamp(mat[0][1], -1.0, 1.0);
      double sr = rspf::clamp(-mat[2][1], -1.0, 1.0);
      
      hpr[0] = 0.0f;
      hpr[2] = rspf::atan2d(sr,cr);
   }
   else
   {
      cp = 1.0 / cp;
      double sr = rspf::clamp((-mat[0][2] * cp), -1.0,1.0);
      double cr = rspf::clamp((mat[2][2] * cp), -1.0, 1.0);
      double sh = rspf::clamp((-mat[1][0] * cp), -1.0, 1.0);
      double ch = rspf::clamp((mat[1][1] * cp), -1.0, 1.0);
      
      if ( (sh == 0.0f && ch == 0.0f) || (sr == 0.0f && cr == 0.0f) )
      {
         cr = rspf::clamp(mat[0][1], -1.0, 1.0);
         sr = rspf::clamp(-mat[2][1], -1.0, 1.0);
         
         hpr[0] = 0.0f;
      }
      else
      {
         hpr[0] = rspf::atan2d(sh, ch);
      }
      
      hpr[2] = rspf::atan2d(sr, cr);
   }
   
   hpr[0] *= -1.0;
   return true;
}

bool rspf::matrixToHpr( rspf_float64 hpr[3],
                  const NEWMAT::Matrix& lsrMatrix,
                  const NEWMAT::Matrix& rotationalMatrix)
{
    bool result = false;
    NEWMAT::Matrix invertLsr(lsrMatrix.i());
   
    hpr[0] = 0.0;
    hpr[1] = 0.0;
    hpr[2] = 0.0;
    result = matrixToHpr(hpr, invertLsr*rotationalMatrix);
    if(std::abs(hpr[0]) < FLT_EPSILON)
    {
       hpr[0] = 0.0;
    }
    if(std::abs(hpr[1]) < FLT_EPSILON)
    {
       hpr[1] = 0.0;
    }
    if(std::abs(hpr[2]) < FLT_EPSILON)
    {
       hpr[2] = 0.0;
    }
    
    return result;
}


void rspf::lexQuotedTokens(const std::string& str,
                            rspf_uint32 start,
                            const char* whitespace,
                            const char* quotes,
                            std::vector<std::string>& tokens,
                            bool& unbalancedQuotes)
{
   rspfREQUIRE(whitespace != NULL);
   rspfREQUIRE(quotes != NULL);
   rspfREQUIRE(tokens != NULL);
   
   const char openQuote(quotes[0]), closeQuote(quotes[1]);
   
   tokens.clear();
   unbalancedQuotes = false;
   
   int end=0;
   while (start < str.length())
   {
      if (str[start] == openQuote)
      {
         int openBraceCount = 1;
         
         if (start+1 < str.length())
         {
            start++;
            if (str[start] != closeQuote)
            {
               //               end = start+1;
               end = start;
               while (static_cast<rspf_uint32>(end) < str.length() &&
                      openBraceCount > 0)
               {
                  if (str[end] == openQuote)
                     openBraceCount++;
                  else if (str[end] == closeQuote)
                     openBraceCount--;
                  end++;
               }
            }
            else
            {
               openBraceCount = 0;
               start++;
               end = start+1;
            }
         }
         if (openBraceCount == 0)
         {
            tokens.push_back(str.substr(start, end-1-start));
         }
         else
         {
            unbalancedQuotes = true;
            end = (int)str.length();
         }
      }
      else if (str[start] == closeQuote)
      {
         unbalancedQuotes = true;
         end = (int)str.length();
	 
      }
      else
      {
         end = (int)str.find_first_of(whitespace, start);
         tokens.push_back(str.substr(start, end-start));
      }
      
      start = (rspf_uint32)str.find_first_not_of(whitespace, end);
   }
}

void rspf::toStringList(rspfString& resultStringOfPoints,
                         const std::vector<rspfDpt>& pointList, 
                         char separator)
{
   rspf_uint32 idx = 0;
   for(;idx < pointList.size();++idx)
   {
      rspfString pt = pointList[idx].toString();
      if(resultStringOfPoints.empty())
      {
         resultStringOfPoints = pt;
      }
      else
      {
         resultStringOfPoints += (separator + pt);
      }
   }
}

void rspf::toStringList(rspfString& resultStringOfPoints,
                         const std::vector<rspfIpt>& pointList, 
                         char separator)
{
   rspf_uint32 idx = 0;
   for(;idx < pointList.size();++idx)
   {
      rspfString pt = pointList[idx].toString();
      if(resultStringOfPoints.empty())
      {
         resultStringOfPoints = pt;
      }
      else
      {
         resultStringOfPoints += (separator + pt);
      }
   }
}

void rspf::toStringList(rspfString& resultStringOfPoints,
                         const std::vector<rspfGpt>& pointList, 
                         char separator)
{
   rspf_uint32 idx = 0;
   for(;idx < pointList.size();++idx)
   {
      rspfString pt = pointList[idx].toString();
      if(resultStringOfPoints.empty())
      {
         resultStringOfPoints = pt;
      }
      else
      {
         resultStringOfPoints += (separator + pt);
      }
   }
}

void rspf::toVector(std::vector<rspfDpt>& result,
                     const rspfString& stringOfPoints)
{
   std::vector<rspfString> splitResult;
  
   // let grab each point string that is surrounded by ()
   std::istringstream in(stringOfPoints.trim());
   rspfString currentPoint = "";
   rspfDpt tempPoint;
   while(!in.bad()&&!in.eof())
   {
      skipws(in);
      if(in.peek() == '(')
      {
         currentPoint += (char)in.get();
         skipws(in);
         if(in.peek() == '(') in.ignore();
         
         while(!in.bad()&&!in.eof()&&(in.peek() != ')'))
         {
            currentPoint += (char)in.get();
         }
         if(in.good())
         {
            currentPoint += (char)in.get();
            tempPoint.toPoint(currentPoint);
            result.push_back(tempPoint);
         }
         currentPoint = "";
      }
      else 
      {
         in.ignore();
      }
   }   
}

void rspf::toVector(std::vector<rspfIpt>& result,
                     const rspfString& stringOfPoints)
{
   std::vector<rspfString> splitResult;
   
   // let grab each point string that is surrounded by ()
   std::istringstream in(stringOfPoints.trim());
   rspfString currentPoint = "";
   rspfIpt tempPoint;
   while(!in.bad()&&!in.eof())
   {
      skipws(in);
      if(in.peek() == '(')
      {
         currentPoint += (char)in.get();
         skipws(in);
         if(in.peek() == '(') in.ignore();
         
         while(!in.bad()&&!in.eof()&&(in.peek() != ')'))
         {
            currentPoint += (char)in.get();
         }
         if(in.good())
         {
            currentPoint += (char)in.get();
            tempPoint.toPoint(currentPoint);
            result.push_back(tempPoint);
         }
         currentPoint = "";
      }
      else 
      {
         in.ignore();
      }
   }   
}
void rspf::toVector(std::vector<rspfGpt>& result,
                     const rspfString& stringOfPoints)
{
   std::vector<rspfString> splitResult;
   
   // let grab each point string that is surrounded by ()
   std::istringstream in(stringOfPoints.trim());
   rspfString currentPoint = "";
   rspfGpt tempPoint;
   while(!in.bad()&&!in.eof())
   {
      skipws(in);
      if(in.peek() == '(')
      {
         currentPoint += (char)in.get();
         skipws(in);
         if(in.peek() == '(') in.ignore();
         
         while(!in.bad()&&!in.eof()&&(in.peek() != ')'))
         {
            currentPoint += (char)in.get();
         }
         if(in.good())
         {
            currentPoint += (char)in.get();
            tempPoint.toPoint(currentPoint);
            result.push_back(tempPoint);
         }
         currentPoint = "";
      }
      else 
      {
         in.ignore();
      }
   }   
}

bool rspf::extractSimpleValues(std::vector<rspfString>& values,
                                const rspfString& stringOfPoints)
{
   std::istringstream in(stringOfPoints);
   rspf::skipws(in);
   bool result = true;
   if(stringOfPoints.empty()) return result;
   char c = in.get();
   rspfString value = "";
   if(c == '(')
   {
      c = (char)in.get();
      while((c!=')')&&
            (c!= '\n')&&
            (in.good()))
      {
         if(c!= ',')
         {
            value += rspfString(c);
         }
         else
         {
            values.push_back(value);
            value = "";
         }
         c = in.get();
      }
   }
   if(c!= ')')
   {
      result = false;
   }
   else
   {
      if(!value.empty())
      {
         values.push_back(value);
      }
   }
   
   return result;
}

bool rspf::toSimpleVector(std::vector<rspf_uint32>& result,
                           const rspfString& stringOfPoints)
{
   std::vector<rspfString> extractedValues;
   bool resultFlag = extractSimpleValues(extractedValues, stringOfPoints);
   if(resultFlag)
   {
      rspf_uint32 idx = 0;
      rspf_uint32 size = (rspf_uint32) extractedValues.size();
      for(idx = 0; idx < size; ++idx)
      {
         result.push_back(extractedValues[idx].toUInt32());
      }
   }
   return resultFlag;
}

template <>
void rspf::toSimpleStringList(rspfString& result,
                               const std::vector<rspf_uint8>& valuesList)

{
   std::ostringstream out;
   
   if(!valuesList.empty())
   {
      rspf_uint32 idx = 0;
      rspf_uint32 size = (rspf_uint32)(valuesList.size()-1);
      for(idx = 0; idx < size; ++idx)
      {
         out << ((rspf_uint32)valuesList[idx]) << ",";
      }
      out << static_cast<rspf_uint32>(valuesList[size]);
   }
   
   result = "("+out.str()+")";
}

bool rspf::toSimpleVector(std::vector<rspf_int32>& result,
                           const rspfString& stringOfPoints)
{
   std::vector<rspfString> extractedValues;
   bool resultFlag = extractSimpleValues(extractedValues, stringOfPoints);
   if(resultFlag)
   {
      rspf_uint32 idx = 0;
      rspf_uint32 size = (rspf_int32) extractedValues.size();
      for(idx = 0; idx < size; ++idx)
      {
         result.push_back(extractedValues[idx].toInt32());
      }
   }
   return resultFlag;
}

bool rspf::toSimpleVector(std::vector<rspf_uint16>& result,
                           const rspfString& stringOfPoints)
{
   std::vector<rspfString> extractedValues;
   bool resultFlag = extractSimpleValues(extractedValues, stringOfPoints);
   if(resultFlag)
   {
      rspf_uint32 idx = 0;
      rspf_uint32 size = (rspf_int32) extractedValues.size();
      for(idx = 0; idx < size; ++idx)
      {
         result.push_back(extractedValues[idx].toUInt32());
      }
   }
   return resultFlag;
}

bool rspf::toSimpleVector(std::vector<rspf_int16>& result,
                           const rspfString& stringOfPoints)
{
   std::vector<rspfString> extractedValues;
   bool resultFlag = extractSimpleValues(extractedValues, stringOfPoints);
   if(resultFlag)
   {
      rspf_uint32 idx = 0;
      rspf_uint32 size = (rspf_uint32) extractedValues.size();
      for(idx = 0; idx < size; ++idx)
      {
         result.push_back(extractedValues[idx].toInt32());
      }
   }
   return resultFlag;
}

bool rspf::toSimpleVector(std::vector<rspf_uint8>& result,
                    const rspfString& stringOfPoints)
{
   std::vector<rspfString> extractedValues;
   bool resultFlag = extractSimpleValues(extractedValues, stringOfPoints);
   if(resultFlag)
   {
      rspf_uint32 idx = 0;
      rspf_uint32 size = (rspf_uint32) extractedValues.size();
      for(idx = 0; idx < size; ++idx)
      {
         result.push_back(extractedValues[idx].toUInt8());
      }
   }
   return resultFlag;
}

bool rspf::toSimpleVector(std::vector<rspf_int8>& result,
                    const rspfString& stringOfPoints)
{
   std::vector<rspfString> extractedValues;
   bool resultFlag = extractSimpleValues(extractedValues, stringOfPoints);
   if(resultFlag)
   {
      rspf_uint32 idx = 0;
      rspf_uint32 size = (rspf_uint32) extractedValues.size();
      for(idx = 0; idx < size; ++idx)
      {
         result.push_back(extractedValues[idx].toUInt8());
      }
   }
   return resultFlag;
}

rspf_uint32 rspf::getNumberOfThreads()
{
   rspf_uint32 result;
   const char* str = rspfPreferences::instance()->findPreference("rspf_threads");
   if ( str )
   {
      result = rspfString(str).toUInt32();
   }
   else
   {
      result = static_cast<rspf_uint32>( OpenThreads::GetNumberOfProcessors() );
   }
   if ( !result )
   {
      result = 1;
   }
   return result;
}
