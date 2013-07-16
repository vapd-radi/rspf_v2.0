//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
// 
// Author: Garrett Potts
//
// Description: Nitf support class for RPC00A -
// Rational Polynomial Coefficient extension.
//
//********************************************************************
// $Id: rspfNitfRpcBase.cpp 22013 2012-12-19 17:37:20Z dburken $

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <vector>
#include <rspf/support_data/rspfNitfRpcBase.h>
#include <rspf/support_data/rspfNitfCommon.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfStringProperty.h>

static const rspfTrace traceDebug(rspfString("rspfNitfRpcBase:debug"));
static const rspfString ERR_BIAS_KW = "ERR_BIAS";
static const rspfString ERR_RAND_KW = "ERR_RAND";
static const rspfString LINE_OFF_KW = "LINE_OFF";
static const rspfString SAMP_OFF_KW = "SAMP_OFF";
static const rspfString LAT_OFF_KW = "LAT_OFF";
static const rspfString LONG_OFF_KW = "LONG_OFF";
static const rspfString HEIGHT_OFF_KW = "HEIGHT_OFF";
static const rspfString LINE_SCALE_KW = "LINE_SCALE";
static const rspfString SAMP_SCALE_KW = "SAMP_SCALE";
static const rspfString LAT_SCALE_KW = "LAT_SCALE";
static const rspfString LONG_SCALE_KW = "LONG_SCALE";
static const rspfString HEIGHT_SCALE_KW = "HEIGHT_SCALE";
static const rspfString LINE_NUM_COEFF_KW = "LINE_NUM_COEFF_";
static const rspfString LINE_DEN_COEFF_KW = "LINE_DEN_COEFF_";
static const rspfString SAMP_NUM_COEFF_KW = "SAMP_NUM_COEFF_";
static const rspfString SAMP_DEN_COEFF_KW = "SAMP_DEN_COEFF_";

RTTI_DEF1(rspfNitfRpcBase, "rspfNitfRpcBase", rspfNitfRegisteredTag);


rspfNitfRpcBase::rspfNitfRpcBase()
   : rspfNitfRegisteredTag()
{
   // Set the tag length in base.
   setTagLength(1041);
   
   clearFields();
}

void rspfNitfRpcBase::parseStream(std::istream& in)
{
   clearFields();
   
   in.read(theSuccess, SUCCESS_SIZE);
   in.read(theErrorBias, ERROR_BIAS_SIZE);
   in.read(theErrRand, ERR_RAND_SIZE);
   in.read(theLineOffset, LINE_OFFSET_SIZE);
   in.read(theSampleOffset, SAMPLE_OFFSET_SIZE);
   in.read(theGeodeticLatOffset, GEODETIC_LAT_OFFSET_SIZE);
   in.read(theGeodeticLonOffset, GEODETIC_LON_OFFSET_SIZE);
   in.read(theGeodeticHeightOffset, GEODETIC_HEIGHT_OFFSET_SIZE);
   in.read(theLineScale, LINE_SCALE_SIZE);
   in.read(theSampleScale, SAMPLE_SCALE_SIZE);
   in.read(theGeodeticLatScale, GEODETIC_LAT_SCALE_SIZE);
   in.read(theGeodeticLonScale, GEODETIC_LON_SCALE_SIZE);
   in.read(theGeodeticHeightScale, GEODETIC_HEIGHT_SCALE_SIZE);
   
   rspf_uint32 idx = 0;
   
   char temp1[LINE_NUMERATOR_COEFFICIENT_SIZE+1];
   temp1[LINE_NUMERATOR_COEFFICIENT_SIZE] = '\0';
   for(idx = 0; idx < LINE_NUMERATOR_COEFFICIENT_COUNT; ++idx)
   {
      in.read(temp1, LINE_NUMERATOR_COEFFICIENT_SIZE);
      theLineNumeratorCoefficient[idx] = temp1;
   }
   
   char temp2[LINE_DENOMINATOR_COEFFICIENT_SIZE+1];
   temp2[LINE_DENOMINATOR_COEFFICIENT_SIZE] = '\0';
   for(idx = 0; idx < LINE_DENOMINATOR_COEFFICIENT_COUNT; ++idx)
   {
      in.read(temp2, LINE_DENOMINATOR_COEFFICIENT_SIZE);
      theLineDenominatorCoefficient[idx] = temp2;
   }
   
   char temp3[SAMPLE_NUMERATOR_COEFFICIENT_SIZE+1];
   temp3[SAMPLE_NUMERATOR_COEFFICIENT_SIZE] = '\0';
   for(idx = 0; idx < SAMPLE_NUMERATOR_COEFFICIENT_COUNT; ++idx)
   {
      in.read(temp3, SAMPLE_NUMERATOR_COEFFICIENT_SIZE);
      theSampleNumeratorCoefficient[idx] = temp3;
   }
   
   char temp4[SAMPLE_DENOMINATOR_COEFFICIENT_SIZE+1];
   temp4[SAMPLE_DENOMINATOR_COEFFICIENT_SIZE] = '\0';
   for(idx = 0; idx < SAMPLE_DENOMINATOR_COEFFICIENT_COUNT; ++idx)
   {
      in.read(temp4, SAMPLE_DENOMINATOR_COEFFICIENT_SIZE);
      theSampleDenominatorCoefficient[idx] = temp4;
   }
}

void rspfNitfRpcBase::writeStream(std::ostream& out)
{
   rspf_uint32 idx = 0;
   
   out.write(theSuccess, SUCCESS_SIZE);
   out.write(theErrorBias, ERROR_BIAS_SIZE);
   out.write(theErrRand, ERR_RAND_SIZE);
   out.write(theLineOffset, LINE_OFFSET_SIZE);
   out.write(theSampleOffset, SAMPLE_OFFSET_SIZE);
   out.write(theGeodeticLatOffset, GEODETIC_LAT_OFFSET_SIZE);
   out.write(theGeodeticLonOffset, GEODETIC_LON_OFFSET_SIZE);
   out.write(theGeodeticHeightOffset, GEODETIC_HEIGHT_OFFSET_SIZE);
   out.write(theLineScale, LINE_SCALE_SIZE);
   out.write(theSampleScale, SAMPLE_SCALE_SIZE);
   out.write(theGeodeticLatScale, GEODETIC_LAT_SCALE_SIZE);
   out.write(theGeodeticLonScale, GEODETIC_LON_SCALE_SIZE);
   out.write(theGeodeticHeightScale, GEODETIC_HEIGHT_SCALE_SIZE);
   for(idx = 0; idx < LINE_NUMERATOR_COEFFICIENT_COUNT; ++idx)
   {
      out.write(theLineNumeratorCoefficient[idx].c_str(),
                LINE_NUMERATOR_COEFFICIENT_SIZE);
   }
   for(idx = 0; idx < LINE_DENOMINATOR_COEFFICIENT_COUNT; ++idx)
   {
      out.write(theLineDenominatorCoefficient[idx].c_str(),
                LINE_DENOMINATOR_COEFFICIENT_SIZE);
   }
   for(idx = 0; idx < SAMPLE_NUMERATOR_COEFFICIENT_COUNT; ++idx)
   {
      out.write(theSampleNumeratorCoefficient[idx].c_str(),
                SAMPLE_NUMERATOR_COEFFICIENT_SIZE);
   }
   for(idx = 0; idx < SAMPLE_DENOMINATOR_COEFFICIENT_COUNT; ++idx)
   {
      out.write(theSampleDenominatorCoefficient[idx].c_str(),
                SAMPLE_DENOMINATOR_COEFFICIENT_SIZE);
   }
}

void rspfNitfRpcBase::clearFields()
{
   theSuccess[0] = '1';
   memcpy(theErrorBias, "0000.00", 7);
   memcpy(theErrRand, "0000.00", 7);
   memcpy(theLineOffset, "000000", 6);
   memcpy(theSampleOffset, "00000", 5);
   memcpy(theGeodeticLatOffset, "+00.0000", 8);
   memcpy(theGeodeticLonOffset, "+000.0000", 9);
   memcpy(theGeodeticHeightOffset, "+0000", 5);
   memcpy(theLineScale, "000001", 6);
   memcpy(theSampleScale, "00001", 5);
   memcpy(theGeodeticLatScale, "+00.0000", 8);
   memcpy(theGeodeticLonScale, "+000.0000", 9);
   memcpy(theGeodeticHeightScale, "+0000", 5);
   
   theSuccess[1] = '\0';
   theErrorBias[7] = '\0';
   theErrRand[7] = '\0';
   theLineOffset[6] = '\0';
   theSampleOffset[5] = '\0';
   theGeodeticLatOffset[8]='\0';
   theGeodeticLonOffset[9]='\0';
   theGeodeticHeightOffset[5]='\0';
   theLineScale[6]='\0';
   theSampleScale[5]='\0';
   theGeodeticLatScale[8]='\0';
   theGeodeticLonScale[9]='\0';
   theGeodeticHeightScale[5]='\0';
   
   theLineNumeratorCoefficient.resize(20);
   theLineDenominatorCoefficient.resize(20);
   theSampleNumeratorCoefficient.resize(20);
   theSampleDenominatorCoefficient.resize(20);
   rspf_uint32 idx = 0;
   const char* zeroString = "+0.000000E+0";
   
   for(idx = 0; idx < theLineNumeratorCoefficient.size();++idx)
   {
      theLineNumeratorCoefficient[idx] = zeroString; 
      theLineDenominatorCoefficient[idx] = zeroString;;
      theSampleNumeratorCoefficient[idx] = zeroString;;
      theSampleDenominatorCoefficient[idx] = zeroString;;
   }
}

bool rspfNitfRpcBase::getSuccess()const
{
   return (theSuccess[0] == '1');
}

void rspfNitfRpcBase::setSuccess(bool success)
{
   theSuccess[0] = success?'1':'0';
}

rspfString rspfNitfRpcBase::getErrorBias()const
{
   return theErrorBias;
}

void rspfNitfRpcBase::setErrorBias(const rspf_float64& errorBias)
{
   rspfString os = rspfNitfCommon::convertToDoubleString(errorBias,
                                                           2,
                                                           ERROR_BIAS_SIZE);
   setErrorBias(os);
}

void rspfNitfRpcBase::setErrorBias(const rspfString& errorBias)
{
   memset(theErrorBias, ' ', ERROR_BIAS_SIZE);
   memcpy(theErrorBias, errorBias.c_str(), std::min((size_t)ERROR_BIAS_SIZE, errorBias.length()));
}

rspfString rspfNitfRpcBase::getErrorRand()const
{
   return theErrRand;
}

void rspfNitfRpcBase::setErrorRand(const rspf_float64& errorRand)
{
   rspfString os = rspfNitfCommon::convertToDoubleString(errorRand,
                                                           2,
                                                           ERROR_BIAS_SIZE);
   setErrorRand(os);
}

void rspfNitfRpcBase::setErrorRand(const rspfString& errorRand)
{
   memset(theErrRand, ' ', ERR_RAND_SIZE);
   memcpy(theErrRand, errorRand.c_str(), std::min((size_t)ERR_RAND_SIZE, errorRand.length()));
}

rspfString rspfNitfRpcBase::getLineOffset()const
{
   return theLineOffset;
}

void rspfNitfRpcBase::setLineOffset(rspf_uint32 lineOffset)
{
   rspfString os = rspfNitfCommon::convertToUIntString(lineOffset,
                                                         LINE_OFFSET_SIZE);
   setLineOffset(os);
}

void rspfNitfRpcBase::setLineOffset(const rspfString& lineOffset)
{
   memset(theLineOffset, ' ', LINE_OFFSET_SIZE);
   memcpy(theLineOffset, lineOffset.c_str(), std::min((size_t)LINE_OFFSET_SIZE, lineOffset.length()));
}

rspfString rspfNitfRpcBase::getSampleOffset()const
{
   return theSampleOffset;
}

void rspfNitfRpcBase::setSampleOffset(rspf_uint32 sampleOffset)
{
   rspfString os = rspfNitfCommon::convertToUIntString(sampleOffset,
                                                         SAMPLE_OFFSET_SIZE);
   setSampleOffset(os);
}

void rspfNitfRpcBase::setSampleOffset(const rspfString& sampleOffset)
{
   memset(theSampleOffset, ' ', SAMPLE_OFFSET_SIZE);
   memcpy(theSampleOffset, sampleOffset.c_str(), std::min((size_t)SAMPLE_OFFSET_SIZE, sampleOffset.length()));
}

rspfString rspfNitfRpcBase::getGeodeticLatOffset()const
{
   return theGeodeticLatOffset;
}

void rspfNitfRpcBase::setGeodeticLatOffset(
   const rspf_float64& geodeticLatOffset)
{
   if ( (geodeticLatOffset >= -90.0) && (geodeticLatOffset <= 90.0) )
   {
      rspfString os =
         rspfNitfCommon::convertToDoubleString(geodeticLatOffset,
                                                4,
                                                GEODETIC_LAT_OFFSET_SIZE);
      setGeodeticLatOffset(os);
   }
}

void rspfNitfRpcBase::setGeodeticLatOffset(
   const rspfString& geodeticLatOffset)
{
   memset(theGeodeticLatOffset, ' ', GEODETIC_LAT_OFFSET_SIZE);
   memcpy(theGeodeticLatOffset, geodeticLatOffset.c_str(), std::min((size_t)GEODETIC_LAT_OFFSET_SIZE, geodeticLatOffset.length()));
}

rspfString rspfNitfRpcBase::getGeodeticLonOffset()const
{
   return theGeodeticLonOffset;
}

void rspfNitfRpcBase::setGeodeticLonOffset(
   const rspf_float64& geodeticLonOffset)
{
   if ( (geodeticLonOffset >= -180.0) && (geodeticLonOffset <= 180.0) )
   {
      rspfString os =
         rspfNitfCommon::convertToDoubleString(geodeticLonOffset,
                                                4,
                                                GEODETIC_LON_OFFSET_SIZE);
      setGeodeticLonOffset(os);
   }
}

void rspfNitfRpcBase::setGeodeticLonOffset(const rspfString& geodeticLonOffset)
{
   memset(theGeodeticLonOffset, ' ', GEODETIC_LON_OFFSET_SIZE);
   memcpy(theGeodeticLonOffset, geodeticLonOffset.c_str(), std::min((size_t)GEODETIC_LON_OFFSET_SIZE, geodeticLonOffset.length()));
}

rspfString rspfNitfRpcBase::getGeodeticHeightOffset()const
{
   return theGeodeticHeightOffset;
}

void rspfNitfRpcBase::setGeodeticHeightOffset(
   rspf_int32 geodeticHeightOffset)
{
   rspfString os =
      rspfNitfCommon::convertToIntString(geodeticHeightOffset,
                                          GEODETIC_HEIGHT_OFFSET_SIZE);
   setGeodeticHeightOffset(os);
}

void rspfNitfRpcBase::setGeodeticHeightOffset(const rspfString& geodeticHeightOffset)
{
   memset(theGeodeticHeightOffset, ' ', GEODETIC_HEIGHT_OFFSET_SIZE);
   memcpy(theGeodeticHeightOffset, geodeticHeightOffset.c_str(), std::min((size_t)GEODETIC_HEIGHT_OFFSET_SIZE, geodeticHeightOffset.length()));
}

rspfString rspfNitfRpcBase::getLineScale()const
{
   return theLineScale;
}

void rspfNitfRpcBase::setLineScale(rspf_uint32 lineScale)
{
   rspfString os = rspfNitfCommon::convertToUIntString(lineScale,
                                                         LINE_SCALE_SIZE);
   setLineScale(os);
}

void rspfNitfRpcBase::setLineScale(const rspfString& lineScale)
{
   memset(theLineScale, ' ', LINE_SCALE_SIZE);
   memcpy(theLineScale, lineScale.c_str(), std::min((size_t)LINE_SCALE_SIZE, lineScale.length()));
}

rspfString rspfNitfRpcBase::getSampleScale()const
{
   return theSampleScale;
}

void rspfNitfRpcBase::setSampleScale(rspf_uint32 sampleScale)
{
   rspfString os = rspfNitfCommon::convertToUIntString(sampleScale,
                                       SAMPLE_SCALE_SIZE);
   setSampleScale(os);
}

void rspfNitfRpcBase::setSampleScale(const rspfString& sampleScale)
{
   memset(theSampleScale, ' ', SAMPLE_SCALE_SIZE);
   memcpy(theSampleScale, sampleScale.c_str(), std::min((size_t)SAMPLE_SCALE_SIZE, sampleScale.length()));
}

rspfString rspfNitfRpcBase::getGeodeticLatScale()const
{
   return theGeodeticLatScale;
}

void rspfNitfRpcBase::setGeodeticLatScale(
   const rspf_float64& geodeticLatScale)
{
   if (geodeticLatScale == 0.0)
   {
      // Error scale cannot be zero.
      return;
   }
   
   if ( (geodeticLatScale >= -90.0) && (geodeticLatScale <= 90.0) )
   {
      rspfString os = rspfNitfCommon::convertToDoubleString(geodeticLatScale,
                                             4,
                                             GEODETIC_LAT_SCALE_SIZE);
      setGeodeticLatScale(os);
   }
}

void rspfNitfRpcBase::setGeodeticLatScale(const rspfString& geodeticLatScale)
{
   memset(theGeodeticLatScale, ' ', GEODETIC_LAT_SCALE_SIZE);
   memcpy(theGeodeticLatScale, geodeticLatScale.c_str(), std::min((size_t)GEODETIC_LAT_SCALE_SIZE, geodeticLatScale.length()));
}

rspfString rspfNitfRpcBase::getGeodeticLonScale()const
{
   return theGeodeticLonScale;
}

void rspfNitfRpcBase::setGeodeticLonScale(
   const rspf_float64& geodeticLonScale)
{
   if (geodeticLonScale == 0.0)
   {
      // Error scale cannot be zero.
      return;
   }
   
   if ( (geodeticLonScale >= -180.0) && (geodeticLonScale <= 180.0) )
   {
      rspfString os = rspfNitfCommon::convertToDoubleString(geodeticLonScale,
                                             4,
                                             GEODETIC_LON_SCALE_SIZE);
      setGeodeticLonScale(os);
   }
}

void rspfNitfRpcBase::setGeodeticLonScale(const rspfString& geodeticLonScale)
{
   memset(theGeodeticLonScale, ' ', GEODETIC_LON_SCALE_SIZE);
   memcpy(theGeodeticLonScale, geodeticLonScale.c_str(), std::min((size_t)GEODETIC_LON_SCALE_SIZE, geodeticLonScale.length()));
}

rspfString rspfNitfRpcBase::getGeodeticHeightScale()const
{
   return theGeodeticHeightScale;
}

void rspfNitfRpcBase::setGeodeticHeightScale(rspf_int32 geodeticHeightScale)
{
   rspfString os =
      rspfNitfCommon::convertToIntString(geodeticHeightScale,
                                          GEODETIC_HEIGHT_SCALE_SIZE);
   setGeodeticHeightScale(os);
}

void rspfNitfRpcBase::setGeodeticHeightScale(
   const rspfString& geodeticHeightScale)
{
   memset(theGeodeticHeightScale, ' ', GEODETIC_HEIGHT_SCALE_SIZE);
   memcpy(theGeodeticHeightScale, geodeticHeightScale.c_str(), std::min((size_t)GEODETIC_HEIGHT_SCALE_SIZE, geodeticHeightScale.length()));
}

/* Temp comment out until range error handling is decided.*/
#if 0
// The assert here will be ignored when compiled with -DNDEBUG so rspfSetError and fix the index
#define ASSERT_ARRAY_CONSTRAINT(expression, idx, correctedValue, module) \
   assert(expression);                                                  \
   rspfSetError(getClassName().c_str(), rspfErrorCodes::RSPF_ERROR,  \
                 "File %s line %d Module %s Error:\nArray index constraint error.\n", \
                 __FILE__, __LINE__, module);                           \
   if (! (expression) ) idx = correctedValue;

#endif
   
rspfString rspfNitfRpcBase::getLineNumeratorCoeff(rspf_uint32 idx)const
{
//ASSERT_ARRAY_CONSTRAINT(idx >= 0 && idx < LINE_NUMERATOR_COEFFICIENT_COUNT,idx,0,"rspfNitfRpcBase::getLineNumeratorCoeff");

   if (idx >= LINE_NUMERATOR_COEFFICIENT_COUNT)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfNitfRpcBase::getLineNumeratorCoeff range error!"
            << std::endl;
      }
      return rspfString();
   }
   
   return theLineNumeratorCoefficient[idx];
}

void rspfNitfRpcBase::setLineNumeratorCoeff(
   const std::vector<rspf_float64>& coeff)
{
   if (coeff.size() != LINE_NUMERATOR_COEFFICIENT_COUNT)
   {
      // Error...
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfNitfRpcBase::setLineNumeratorCoeff WARNING:"
            << "\nArray not of correct size!"
            << std::endl;
      }
      return;
   }

   for (rspf_uint32 i = 0; i < LINE_NUMERATOR_COEFFICIENT_COUNT; ++i)
   {
      rspfString os = rspfNitfCommon::convertToScientificString(
         coeff[i], LINE_NUMERATOR_COEFFICIENT_SIZE);
      setLineNumeratorCoeff(i, os);
   }
}

void rspfNitfRpcBase::setLineNumeratorCoeff(rspf_uint32 idx,
                                             const rspfString& coeff)
{
   // ASSERT_ARRAY_CONSTRAINT(idx >= 0 && idx < LINE_NUMERATOR_COEFFICIENT_COUNT,idx,0,"rspfNitfRpcBase::setLineNumeratorCoeff");

   if (idx >= LINE_NUMERATOR_COEFFICIENT_COUNT)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfNitfRpcBase::setLineNumeratorCoeff range error!"
         << std::endl;
      return;
   } 

   std::string s(coeff, 0, LINE_NUMERATOR_COEFFICIENT_SIZE);
   theLineNumeratorCoefficient[idx] = s;  
}

rspfString rspfNitfRpcBase::getLineDenominatorCoeff(rspf_uint32 idx)const
{
   // ASSERT_ARRAY_CONSTRAINT(idx >= 0 && idx < LINE_DENOMINATOR_COEFFICIENT_COUNT,idx,0,"rspfNitfRpcBase::getLineDenominatorCoeff");

   if (idx >= LINE_DENOMINATOR_COEFFICIENT_COUNT)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfNitfRpcBase::getLineDenominatorCoeff range error!"
         << std::endl;
      return rspfString();
   }  
   
   return theLineDenominatorCoefficient[idx];
}

void rspfNitfRpcBase::setLineDenominatorCoeff(
   const std::vector<rspf_float64>& coeff)
{
   if (coeff.size() != LINE_DENOMINATOR_COEFFICIENT_COUNT)
   {
      // Error...
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfNitfRpcBase::setLineDenominatorCoeff WARNING:"
            << "\nArray not of correct size!"
            << std::endl;
      }
      return;
   }

   for (rspf_uint32 i = 0; i < LINE_DENOMINATOR_COEFFICIENT_COUNT; ++i)
   {
      rspfString os = rspfNitfCommon::convertToScientificString(
         coeff[i], LINE_DENOMINATOR_COEFFICIENT_SIZE);
      setLineDenominatorCoeff(i, os);
   }
}

void rspfNitfRpcBase::setLineDenominatorCoeff(rspf_uint32 idx,
                                               const rspfString& coeff)
{
   // ASSERT_ARRAY_CONSTRAINT(idx >= 0 && idx < LINE_DENOMINATOR_COEFFICIENT_COUNT,idx,0,"rspfNitfRpcBase::setLineDenominatorCoeff");

   if (idx >= LINE_DENOMINATOR_COEFFICIENT_COUNT)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfNitfRpcBase::setLineDenominatorCoeff range error!"
         << std::endl;
      return;
   } 
   
   std::string s(coeff, 0, LINE_DENOMINATOR_COEFFICIENT_SIZE);
   theLineDenominatorCoefficient[idx] = s;  
}

rspfString rspfNitfRpcBase::getSampleNumeratorCoeff(rspf_uint32 idx)const
{
   // ASSERT_ARRAY_CONSTRAINT(idx >= 0 && idx < SAMPLE_NUMERATOR_COEFFICIENT_COUNT,idx,0,"rspfNitfRpcBase::getSampleNumeratorCoeff");

   if (idx >= SAMPLE_NUMERATOR_COEFFICIENT_COUNT)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfNitfRpcBase::getSampleNumeratorCoeff range error!"
         << std::endl;
      return rspfString();
   } 
   return theSampleNumeratorCoefficient[idx];
}

void rspfNitfRpcBase::setSampleNumeratorCoeff(
   const std::vector<rspf_float64>& coeff)
{
   if (coeff.size() != SAMPLE_NUMERATOR_COEFFICIENT_COUNT)
   {
      // Error...
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfNitfRpcBase::setSampleNumeratorCoeff WARNING:"
            << "\nArray not of correct size!"
            << std::endl;
      }
      return;
   }

   for (rspf_uint32 i = 0; i < SAMPLE_NUMERATOR_COEFFICIENT_COUNT; ++i)
   {
      rspfString os = rspfNitfCommon::convertToScientificString(
         coeff[i], SAMPLE_NUMERATOR_COEFFICIENT_SIZE);
      setSampleNumeratorCoeff(i, os);
   }
}

void rspfNitfRpcBase::setSampleNumeratorCoeff(rspf_uint32 idx,
                                               const rspfString& coeff)
{
   // ASSERT_ARRAY_CONSTRAINT(idx >= 0 && idx < SAMPLE_NUMERATOR_COEFFICIENT_COUNT,idx,0,"rspfNitfRpcBase::setSampleNumeratorCoeff");

   if (idx >= SAMPLE_NUMERATOR_COEFFICIENT_COUNT)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfNitfRpcBase::setSampleNumeratorCoeff range error!"
         << std::endl;
      return;
   } 
   
   std::string s(coeff, 0, SAMPLE_NUMERATOR_COEFFICIENT_SIZE);
   theSampleNumeratorCoefficient[idx] = s;  
}

rspfString rspfNitfRpcBase::getSampleDenominatorCoeff(rspf_uint32 idx)const
{
   // ASSERT_ARRAY_CONSTRAINT(idx >= 0 && idx < SAMPLE_DENOMINATOR_COEFFICIENT_COUNT,idx,0,"rspfNitfRpcBase::getSampleDenominatorCoeff");

   if (idx >= SAMPLE_DENOMINATOR_COEFFICIENT_COUNT)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfNitfRpcBase::getSampleDenominatorCoeff range error!"
         << std::endl;
      return rspfString();
   } 
   
   return theSampleDenominatorCoefficient[idx];
}

void rspfNitfRpcBase::setSampleDenominatorCoeff(
   const std::vector<rspf_float64>& coeff)
{
   if (coeff.size() != SAMPLE_DENOMINATOR_COEFFICIENT_COUNT)
   {
      // Error...
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfNitfRpcBase::setSampleDenominatorCoeff WARNING:"
            << "\nArray not of correct size!"
            << std::endl;
      }
      return;
   }

   for (rspf_uint32 i = 0; i < SAMPLE_DENOMINATOR_COEFFICIENT_COUNT; ++i)
   {
      rspfString os = rspfNitfCommon::convertToScientificString(
         coeff[i], SAMPLE_DENOMINATOR_COEFFICIENT_SIZE);
      setSampleDenominatorCoeff(i, os);
   }
}

void rspfNitfRpcBase::setSampleDenominatorCoeff(
   rspf_uint32 idx, const rspfString& coeff)
{
   // ASSERT_ARRAY_CONSTRAINT(idx >= 0 && idx < SAMPLE_DENOMINATOR_COEFFICIENT_COUNT,idx,0,"rspfNitfRpcBase::setSampleDenominatorCoeff");

   if (idx >= SAMPLE_DENOMINATOR_COEFFICIENT_COUNT)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfNitfRpcBase::setSampleDenominatorCoeff range error!"
         << std::endl;
      return;
   }
   
  std::string s(coeff, 0, SAMPLE_DENOMINATOR_COEFFICIENT_SIZE);
  s.resize(SAMPLE_DENOMINATOR_COEFFICIENT_SIZE, ' ');
  theSampleDenominatorCoefficient[idx] = s;  
}

std::ostream& rspfNitfRpcBase::print(std::ostream& out,
                                      const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += getTagName();
   pfx += ".";

   out << setiosflags(std::ios::left)
       << pfx << std::setw(24) << "CETAG:"
       << getTagName() << "\n"
       << pfx << std::setw(24) << "CEL:"
       << getSizeInBytes() << "\n"
       << pfx << std::setw(24) << "SUCCESS:"
       << theSuccess << "\n" 
       << pfx << std::setw(24) << "ERR_BIAS:"
       << theErrorBias << "\n"
       << pfx << std::setw(24) << "ERR_RAND:"
       <<theErrRand  << "\n" 
       << pfx << std::setw(24) << "LINE_OFF:"
       << theLineOffset << "\n"
       << pfx << std::setw(24) << "SAMP_OFF:"
       << theSampleOffset << "\n" 
       << pfx << std::setw(24) << "LAT_OFF:"
       << theGeodeticLatOffset << "\n"
       << pfx << std::setw(24) << "LONG_OFF:"
       << theGeodeticLonOffset << "\n" 
       << pfx << std::setw(24) << "HEIGHT_OFF:"
       << theGeodeticHeightOffset << "\n"
       << pfx << std::setw(24) << "LINE_SCALE:"
       << theLineScale  << "\n" 
       << pfx << std::setw(24)
       << "SAMP_SCALE:" << theSampleScale << "\n"
       << pfx << std::setw(24) << "LAT_SCALE:"
       << theGeodeticLatScale << "\n" 
       << pfx << std::setw(24) << "LONG_SCALE:"
       << theGeodeticLonScale << "\n"
       << pfx << std::setw(24) << "HEIGHT_SCALE:"
       << theGeodeticHeightScale << "\n";

   rspf_int32 i;
   rspfString s;
   
   for (i=0; i<LINE_NUMERATOR_COEFFICIENT_COUNT; ++i)
   {
      s = "LINE_NUM_COEFF_";
      s += rspfString::toString(i);
      s += ":";
      out << pfx << std::setw(24) << s
          << theLineNumeratorCoefficient[i] << "\n";
   }

   for (i=0; i<LINE_DENOMINATOR_COEFFICIENT_COUNT; ++i)
   {
      s = "LINE_DEN_COEFF_";
      s += rspfString::toString(i);
      s += ":";
      out << pfx << std::setw(24) << s
          << theLineDenominatorCoefficient[i] << "\n";
   }

   for (i=0; i<LINE_NUMERATOR_COEFFICIENT_COUNT; ++i)
   {
      s = "SAMP_NUM_COEFF_";
      s += rspfString::toString(i);
      s += ":";
      out << pfx << std::setw(24) << s
          << theSampleNumeratorCoefficient[i] << "\n";
   }

   for (i=0; i<LINE_DENOMINATOR_COEFFICIENT_COUNT; ++i)
   {
      s = "SAMP_DEN_COEFF_";
      s += rspfString::toString(i);
      s += ":";
      out << pfx << std::setw(24) << s
          << theSampleDenominatorCoefficient[i] << "\n";
   }

   out.flush();
   
   return out;
}

void rspfNitfRpcBase::setProperty(rspfRefPtr<rspfProperty> /* property */)
{
   
}

rspfRefPtr<rspfProperty> rspfNitfRpcBase::getProperty(const rspfString& name)const
{
   rspfProperty* result = 0;
   if(name == ERR_BIAS_KW)
   {
      result = new rspfStringProperty(name, theErrorBias);
   }
   else if(name == ERR_RAND_KW)
   {
      result = new rspfStringProperty(name, theErrRand);
   }
   else if(name == LINE_OFF_KW)
   {
      result = new rspfStringProperty(name, theLineOffset);
   }
   else if(name == SAMP_OFF_KW)
   {
      result = new rspfStringProperty(name, theSampleOffset);
   }
   else if(name == LAT_OFF_KW)
   {
      result = new rspfStringProperty(name, theGeodeticLatOffset);
   }
   else if(name == LONG_OFF_KW)
   {
      result = new rspfStringProperty(name, theGeodeticLonOffset);
   }
   else if(name == HEIGHT_OFF_KW)
   {
      result = new rspfStringProperty(name, theGeodeticHeightOffset);
   }
   else if(name == LINE_SCALE_KW)
   {
      result = new rspfStringProperty(name, theLineScale);
   }
   else if(name == SAMP_SCALE_KW)
   {
      result = new rspfStringProperty(name,theSampleScale );
   }
   else if(name == LAT_SCALE_KW)
   {
      result = new rspfStringProperty(name, theGeodeticLatScale);
   }
   else if(name == LONG_SCALE_KW)
   {
      result = new rspfStringProperty(name, theGeodeticLonScale);
   }
   else if(name == HEIGHT_SCALE_KW)
   {
      result = new rspfStringProperty(name, theGeodeticHeightScale);
   }
   else if(name.contains(LINE_NUM_COEFF_KW))
   {
      rspf_uint32 idx = 0;

         idx = rspfString(name.begin()+LINE_NUM_COEFF_KW.length(),
                           name.end()).toUInt32();
      --idx;

      if(idx < theLineNumeratorCoefficient.size())
      {
         result = new rspfStringProperty(name, theLineNumeratorCoefficient[idx]);
      }
   }
   else if(name.contains(LINE_DEN_COEFF_KW))
   {
      rspf_uint32 idx = 0;

      idx = rspfString(name.begin()+LINE_DEN_COEFF_KW.length(),
                        name.end()).toUInt32();
      --idx;

      if(idx < theLineDenominatorCoefficient.size())
      {
         result = new rspfStringProperty(name, theLineDenominatorCoefficient[idx]);
      }
   }
   else if(name.contains(SAMP_NUM_COEFF_KW))
   {
      rspf_uint32 idx = 0;

      idx = rspfString(name.begin()+SAMP_NUM_COEFF_KW.length(),
                        name.end()).toUInt32(); 
      --idx;

      if(idx < theSampleNumeratorCoefficient.size())
      {
         result = new rspfStringProperty(name, theSampleNumeratorCoefficient[idx]);
      }
   }
   else if(name.contains(SAMP_DEN_COEFF_KW))
   {
      rspf_uint32 idx = 0;

      idx = rspfString(name.begin()+SAMP_DEN_COEFF_KW.length(),
                        name.end()).toUInt32();
      --idx;

      if(idx < theSampleDenominatorCoefficient.size())
      {
         result = new rspfStringProperty(name, theSampleDenominatorCoefficient[idx]);
      }
   }

   return result;
}

void rspfNitfRpcBase::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspf_uint32 idx = 0;
   
   propertyNames.push_back(ERR_BIAS_KW);
   propertyNames.push_back(ERR_RAND_KW);
   propertyNames.push_back(LINE_OFF_KW);
   propertyNames.push_back(SAMP_OFF_KW);
   propertyNames.push_back(LAT_OFF_KW);
   propertyNames.push_back(LONG_OFF_KW);
   propertyNames.push_back(HEIGHT_OFF_KW);
   propertyNames.push_back(LINE_SCALE_KW);
   propertyNames.push_back(SAMP_SCALE_KW);
   propertyNames.push_back(LAT_SCALE_KW);
   propertyNames.push_back(LONG_SCALE_KW);
   propertyNames.push_back(HEIGHT_SCALE_KW);

   for(idx = 1; idx <= 20; ++idx)
   {
      propertyNames.push_back(LINE_NUM_COEFF_KW+rspfString::toString(idx));
   }
   for(idx = 1; idx <= 20; ++idx)
   {
      propertyNames.push_back(LINE_DEN_COEFF_KW+rspfString::toString(idx));
   }
   for(idx = 1; idx <= 20; ++idx)
   {
      propertyNames.push_back(SAMP_NUM_COEFF_KW+rspfString::toString(idx));
   }
   for(idx = 1; idx <= 20; ++idx)
   {
      propertyNames.push_back(SAMP_DEN_COEFF_KW+rspfString::toString(idx));
   }
}

