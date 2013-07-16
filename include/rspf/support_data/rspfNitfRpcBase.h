//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
// 
// Author: Garrett Potts
//
// Description: Nitf base support data class for RPC tags.
// 
// Rational Polynomial Coefficient extension.
//
// See: http://164.214.2.51/ntb/baseline/docs/stdi0002/final.pdf
//
//********************************************************************
// $Id: rspfNitfRpcBase.h 22013 2012-12-19 17:37:20Z dburken $
#ifndef rspfNitfRpcBase_HEADER
#define rspfNitfRpcBase_HEADER 1
#include <rspf/support_data/rspfNitfRegisteredTag.h>

class RSPF_DLL rspfNitfRpcBase : public rspfNitfRegisteredTag
{
 public:
  enum 
  {
    SUCCESS_SIZE                         = 1,
    ERROR_BIAS_SIZE                      = 7,
    ERR_RAND_SIZE                        = 7,
    LINE_OFFSET_SIZE                     = 6,
    SAMPLE_OFFSET_SIZE                   = 5,
    GEODETIC_LAT_OFFSET_SIZE             = 8,
    GEODETIC_LON_OFFSET_SIZE             = 9,
    GEODETIC_HEIGHT_OFFSET_SIZE          = 5,
    LINE_SCALE_SIZE                      = 6,
    SAMPLE_SCALE_SIZE                    = 5,
    GEODETIC_LAT_SCALE_SIZE              = 8,
    GEODETIC_LON_SCALE_SIZE              = 9,
    GEODETIC_HEIGHT_SCALE_SIZE           = 5,
    LINE_NUMERATOR_COEFFICIENT_SIZE      = 12,
    LINE_NUMERATOR_COEFFICIENT_COUNT     = 20,
    LINE_DENOMINATOR_COEFFICIENT_SIZE    = 12,
    LINE_DENOMINATOR_COEFFICIENT_COUNT   = 20,
    SAMPLE_NUMERATOR_COEFFICIENT_SIZE    = 12,
    SAMPLE_NUMERATOR_COEFFICIENT_COUNT   = 20,
    SAMPLE_DENOMINATOR_COEFFICIENT_SIZE  = 12,
    SAMPLE_DENOMINATOR_COEFFICIENT_COUNT = 20
    //                                  -----
    //                                   1041
  };

  /** default constructor */
  rspfNitfRpcBase();

  /**
   * Parse method.
   *
   * @param in Stream to parse.
   */
  virtual void parseStream(std::istream& in);

  /**
   * Write method.
   *
   * @param out Stream to write to.
   */
  virtual void writeStream(std::ostream& out);

  /**
   * Clears all string fields within the record to some default nothingness.
   */
  virtual void clearFields();

  /**
   * @return The success record as a boolean.
   */
  bool getSuccess()const;

  /** @param success Sets data member theSuccess to success. */
  void setSuccess(bool success);

  /** @return theErrorBias as a string. */
  rspfString getErrorBias()const;

  //---
  // The set methods below taking rspfString args will truncate and
  // pad with spaces, as necessary, to match enumed size.
  //---

  /**
   * @param errorBias Error bias to set theErrorBias to.  This will be
   * converted to a string with two digit precision.
   */
  void setErrorBias(const rspf_float64& errorBias);

  /** @param errorBias bias string to set theErrorBias to. */
  void setErrorBias(const rspfString& errorBias);

  /** @return theErrorBias as a string. */
  rspfString getErrorRand()const;

  /**
   * @param errorRand The random error to set theErrRand to.  This will be
   * converted to a string with two digit precision.
   */
  void setErrorRand(const rspf_float64& errorRand);

  
  /** @param errorRand error random  string to set theErrorRand to. */
  void setErrorRand(const rspfString& errorRand);

  /** @return The line offset as a string. */
  rspfString getLineOffset()const;

  /**
   * @param lineOffset Sets the line offset to a string truncated to an int.
   */
  void setLineOffset(rspf_uint32 lineOffset);

  /**
   * @param lineOffset Sets theLineOffset to lineOffset string.
   */
  void setLineOffset(const rspfString& lineOffset);

  /** @return The sample offset as a string. */
  rspfString getSampleOffset()const;

  /**
   * @param sampleOffset Sets the sample offset to a string truncated to an
   * int.
   */
  void setSampleOffset(rspf_uint32 sampleOffset);

  /** @param sampleOffset Sets the sample offset to a string. */
  void setSampleOffset(const rspfString& sampleOffset);

  /** @return Geodetic latitude offset as a string. */
  rspfString getGeodeticLatOffset()const;

  /**
   * @param geodeticLatOffset Value to Set the geodedic latitude offset to.
   * Converted to a string with four digit precision.
   */
  void setGeodeticLatOffset(const rspf_float64& geodeticLatOffset);

  /**
   * @param geodeticLatOffset geodedic latitude offset string to set
   * theGeodeticLatOffset to.
   */
  void setGeodeticLatOffset(const rspfString& geodeticLatOffset);

  /** @return Geodetic longitude offset as a string. */
  rspfString getGeodeticLonOffset()const;

  /**
   * @param geodeticLonOffset Value to Set the geodedic longitude offset to.
   * Converted to a string with four digit precision.
   */
  void setGeodeticLonOffset(const rspf_float64& geodeticLonOffset);

  /**
   * @param geodeticLonOffset Value to Set the geodedic longitude offset to.
   */
  void setGeodeticLonOffset(const rspfString& geodeticLonOffset);

  /** @return Geodetic height offset as a string. */
  rspfString getGeodeticHeightOffset()const;

  /**
   * @param geodeticHeightOffset value to Set the height offset to.
   * Converted to a string truncated to an int.
   */
  void setGeodeticHeightOffset(rspf_int32 geodeticHeightOffset);
  
  /** @param geodeticHeightOffset value to Set the height offset to. */
  void setGeodeticHeightOffset(const rspfString& geodeticHeightOffset);

  /** @return The line scale as a string. */
  rspfString getLineScale()const;
  
  /**
   * @param lineScale The value to set the line scale to.
   * Converted to a string.
   */
  void setLineScale(rspf_uint32 lineScale);

  /**
   * @param lineScale The value to set the line scale to.
   */
  void setLineScale(const rspfString& lineScale);

  /**
   * @return the sample scale as a string.
   */
  rspfString getSampleScale()const;

  /**
   * @param sampleScale The value to set the sample scale to.
   * Converted to a string truncated to an int.
   */
  void setSampleScale(rspf_uint32 sampleScale);

  /**
   * @param sampleScale The value to set the sample scale to.
   */
  void setSampleScale(const rspfString& sampleScale);

  /** @return The Geodetic latitude scale as a string. */
  rspfString getGeodeticLatScale()const;

  /**
   * @param geodeticLatScale The value to set the geodedic latitude scale to.
   * Converted to a string with four digit precision.
   */
  void setGeodeticLatScale(const rspf_float64& geodeticLatScale);

  /**
   * @param geodeticLatScale The value to set the geodedic latitude scale to.
   */
  void setGeodeticLatScale(const rspfString& geodeticLatScale);

  /** @return Geodetic longitude scale as a string. */
  rspfString getGeodeticLonScale()const;

  /**
   * @param geodeticLonScale The value to set the geodedic longitude scale to.
   * Converted to a string with four digit precision.
   */
  void setGeodeticLonScale(const rspf_float64& geodeticLonScale);

  /**
   * @param geodeticLonScale The value to set the geodedic longitude scale to.
   */
  void setGeodeticLonScale(const rspfString& geodeticLonScale);

  /** @return The geodetic height scale as a string. */
  rspfString getGeodeticHeightScale()const;

  /**
   * @param geodeticHeightScale The value to set the height scale to.
   * Converted to a string truncated to an int.
   */
  void setGeodeticHeightScale(rspf_int32 geodeticHeightScale);
  
  /**
   * @param geodeticHeightScale The value to set the height scale to.
   */
  void setGeodeticHeightScale(const rspfString& geodeticHeightScale);

  /**
   * @param idx Index of line numerator coefficient to get.
   * Range: 0 - 19
   *
   * @return The line numerator coefficient as a string.
   */
  rspfString getLineNumeratorCoeff(rspf_uint32 idx)const;

  /**
   * @param coeff This should be an array of 20 line numerator coefficients.
   * Coefficients will be converted to a scientific string to conform to
   * specification.
   *
   * @note if (coeff.size() != 20) no action is taken.
   */  
  void setLineNumeratorCoeff(const std::vector<rspf_float64>& coeff);
  
  /**
   * @param idx index of line numerator coefficient to set.
   * Range: 0 - 19
   *
   * @param coeff Value to set line numerator coefficient to for
   * idx or index.
   */  
  void setLineNumeratorCoeff(rspf_uint32 idx,
                             const rspfString& coeff);

  /**
   * @param idx index of line denominator coefficient to get.
   * Range: 0 - 19
   *
   * @return line  denominator coefficient as a string.
   */
  rspfString getLineDenominatorCoeff(rspf_uint32 idx)const;
  
  /**
   * @param coeff This should be an array of 20 line denominator coefficients.
   * Coefficients will be converted to a scientific string to conform to
   * specification.
   *
   * @note if (coeff.size() != 20) no action is taken.
   */  
  void setLineDenominatorCoeff(const std::vector<rspf_float64>& coeff);
  
  /**
   * @param idx index of line denominator coefficient to set.
   * Range: 0 - 19
   *
   * @param lineDenominatorCoeff Value to set line denominator coefficient
   * to for idx or index.
   */  
  void setLineDenominatorCoeff(rspf_uint32 idx,
                               const rspfString& coeff);

  /**
   * @param idx Index of sample numerator coefficient to get.
   * Range: 0 - 19
   *
   * @return The sample numerator coefficient as a string.
   */
  rspfString getSampleNumeratorCoeff(rspf_uint32 idx)const;

  /**
   * @param coeff This should be an array of 20 sample numerator coefficients.
   * Coefficients will be converted to a scientific string to conform to
   * specification.
   *
   * @note if (coeff.size() != 20) no action is taken.
   */  
  void setSampleNumeratorCoeff(const std::vector<rspf_float64>& coeff);
  
  /**
   * @param idx index of sample numerator coefficient to set.
   * Range: 0 - 19
   *
   * @param coeff Value to set sample numerator coefficient to for
   * idx or index.
   */  
  void setSampleNumeratorCoeff(rspf_uint32 idx,
                             const rspfString& coeff);

  /**
   * @param idx index of sample denominator coefficient to get.
   * Range: 0 - 19
   *
   * @return sample  denominator coefficient as a string.
   */
  rspfString getSampleDenominatorCoeff(rspf_uint32 idx)const;
  
  /**
   * @param coeff This should be an array of 20 sample denominator coefficients.
   * Coefficients will be converted to a scientific string to conform to
   * specification.
   *
   * @note if (coeff.size() != 20) no action is taken.
   */  
  void setSampleDenominatorCoeff(const std::vector<rspf_float64>& coeff);
  
  /**
   * @param idx index of sample denominator coefficient to set.
   * Range: 0 - 19
   *
   * @param sampleDenominatorCoeff Value to set sample denominator coefficient
   * to for idx or index.
   */  
  void setSampleDenominatorCoeff(rspf_uint32 idx,
                               const rspfString& coeff);

   /**
    * @brief Print method that outputs a key/value type format
    * adding prefix to keys.
    * @param out Stream to output to.
    * @param prefix Prefix added to key like "image0.";
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix=std::string()) const;

  virtual void setProperty(rspfRefPtr<rspfProperty> property);
  virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
  virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;

protected:

  /**
   * FIELD: SUCCESS
   *
   * Required 1 byte field.
   * 
   * Value is 1
   * 
   */
  char theSuccess[SUCCESS_SIZE+1];
   
  /**
   * FIELD: ERR_BIAS
   *
   * required 7 byte field
   * 
   * Error - Bias. 68% non time-varying error estimate assumes
   * correlated images.
   *
   * Values range from 0000.00 to 9999.99 meters
   */
  char theErrorBias[ERROR_BIAS_SIZE+1];

  /**
   * FIELD ERR_RAND
   *
   * Required 7 byte field
   * 
   * Error - Random. 68% time-varying error estimate assumes
   * correlated images. 0000.00 to 9999.99 meters
   */
  char theErrRand[ERR_RAND_SIZE+1];

  /**
   * FIELD: LINE_OFF
   *
   * required 6 byte field.
   * 
   * value is in pixels and ranges from 000000-999999
   */
  char theLineOffset[LINE_OFFSET_SIZE+1];

  /**
   * FIELD: SAMP_OFF
   *
   * required 5 byte field.
   *
   * value is in pixels and ranges from 00000-99999
   */
  char theSampleOffset[SAMPLE_OFFSET_SIZE+1];

  /**
   * FIELD: LAT_OFF
   *
   * required 8 byte field.
   *
   * Holds the lat offset in degrees and ranges from +-90.0000
   */
  char theGeodeticLatOffset[GEODETIC_LAT_OFFSET_SIZE+1];

  /**
   * FIELD: LONG_OFF
   *
   * required 9 byte field.
   *
   * holds the lon offset in degrees and ranges from +-180.0000
   */
  char theGeodeticLonOffset[GEODETIC_LON_OFFSET_SIZE+1];

  /**
   * FIELD: HEIGHT_OFF
   *
   * required 5 byte field.
   *
   * holds the height offset in meters and ranges from +-9999
   */
  char theGeodeticHeightOffset[GEODETIC_HEIGHT_OFFSET_SIZE+1];

  /**
   * FIELD: LINE_SCALE
   *
   * required 6 byte field.
   *
   * holds the line scale in pixels and ranges from 000001-999999
   * 
   */
  char theLineScale[LINE_SCALE_SIZE+1];

  /**
   * FIELD: SAMP_SCALE
   *
   * required 5 byte field.
   *
   * holds the line scale in pixels and ranges from 00001-99999
   * 
   */
  char theSampleScale[SAMPLE_SCALE_SIZE+1];

  /**
   * FIELD: LAT_SCALE
   *
   * required 8 byte field
   *
   * holds the lat scale in degrees and ranges from +-90.0000 degrees
   */
  char theGeodeticLatScale[GEODETIC_LAT_SCALE_SIZE+1];

  /**
   * FIELD: LON_SCALE
   *
   * required 9 byte field
   *
   * holds the lon scale in degrees and ranges from +-180.0000 degrees
   */
  char theGeodeticLonScale[GEODETIC_LON_SCALE_SIZE+1];

  /**
   * FIELD: HEIGHT_SCALE
   *
   * required 5 byte field
   *
   * holds the geodetic height scale and ranges from +-9999 meters.
   */
  char theGeodeticHeightScale[GEODETIC_HEIGHT_SCALE_SIZE+1];

  /**
   * FIELD: LINE_NUM_COEFF_1 through LINE_NUM_COEFF_20
   *
   * Line Numerator Coefficients.
   *
   * required 12 byte field ranging ±0.999999E±9 Twenty coefficients for the
   * polynomial
   *
   */     
  std::vector<rspfString> theLineNumeratorCoefficient;

  /**
   * FIELD: LINE_DEN_COEFF_1 through LINE_DEN_COEFF_20
   *
   * Line Denominato Coefficients.
   *
   * required 12 byte field ranging ±0.999999E±9 Twenty coefficients for the
   * polynomial
   *
   */     
  std::vector<rspfString> theLineDenominatorCoefficient;

  /**
   * FIELD: SAMP_NUM_COEFF_1 through SAMP_NUM_COEFF_20
   *
   * samp Numerator Coefficients.
   *
   * required 12 byte field ranging ±0.999999E±9 Twenty coefficients for the
   * polynomial
   *
   */     
  std::vector<rspfString> theSampleNumeratorCoefficient;

  /**
   * FIELD: SAMP_DEN_COEFF_1 through SAMP_DEN_COEFF_20
   *
   * samp Denominator Coefficients.
   *
   * required 12 byte field ranging ±0.999999E±9 Twenty coefficients for the
   * polynomial
   *
   */     
  std::vector<rspfString> theSampleDenominatorCoefficient;

TYPE_DATA   
};

#endif
