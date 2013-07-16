#ifndef rspfQuickbirdRpcHeader_HEADER
#define rspfQuickbirdRpcHeader_HEADER
#include <iostream>

#include <rspf/base/rspfErrorStatusInterface.h>

#include <rspf/base/rspfFilename.h>
class RSPF_DLL rspfQuickbirdRpcHeader : public rspfErrorStatusInterface
{
public:
  friend RSPF_DLL std::ostream& operator << (std::ostream& out,
			       const rspfQuickbirdRpcHeader& data);
  rspfQuickbirdRpcHeader();

  bool open(const rspfFilename& file);

  bool isAPolynomial()const
  {
    return theSpecId.contains("A");
  }
  bool isBPolynomial()const
  {
    return theSpecId.contains("B");
  }
  rspfString theFilename;

  rspfString theSatId;
  rspfString theBandId;
  rspfString theSpecId;
  double    theErrBias;
  double    theErrRand;
  rspf_int32 theLineOffset;
  rspf_int32 theSampOffset;
  double    theLatOffset;
  double    theLonOffset;
  double    theHeightOffset;
  double    theLineScale;
  double    theSampScale;
  double    theLatScale;
  double    theLonScale;
  double    theHeightScale;
 
  std::vector<double> theLineNumCoeff;
  std::vector<double> theLineDenCoeff;
  std::vector<double> theSampNumCoeff;
  std::vector<double> theSampDenCoeff;


  bool readCoeff(std::istream& in,
		 std::vector<double>& coeff);
  bool parseNameValue(const rspfString& line);
		     
};

#endif
