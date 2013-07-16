//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Hicks
//
// Description: Helper interface class for rspfAdjustmentExecutive
//              and rspfWLSBundleSolution.
//----------------------------------------------------------------------------

#include <rspf/base/rspfAdjSolutionAttributes.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNotify.h>

static rspfTrace traceDebug(rspfString("rspfAdjSolutionAttributes:debug"));
static rspfTrace traceExec(rspfString("rspfAdjSolutionAttributes:exec"));


//*****************************************************************************
// METHOD:      rspfAdjSolutionAttributes::rspfAdjSolutionAttributes
//
// DESCRIPTION: Default constructor
//
// PARAMETERS:  N/A
//
// RETURN:      N/A
//*****************************************************************************
rspfAdjSolutionAttributes::rspfAdjSolutionAttributes(
	const int& numObjObs, const int& numImages, const int& numMeas, const int& rankN)
	:
	theNumObjObs(numObjObs),
	theNumImages(numImages),
	theFullRank(rankN),
   theNumMeasurements(numMeas)
{
   theTotalCorrections.ReSize(rankN,1);
   theLastCorrections.ReSize(rankN,1);
   theTotalCorrections = 0.0;
   theLastCorrections = 0.0;
}


//*****************************************************************************
// METHOD:      rspfAdjSolutionAttributes::~rspfAdjSolutionAttributes
//
// DESCRIPTION: Destructor
//
// PARAMETERS:  N/A
//
// RETURN:      N/A
//*****************************************************************************
rspfAdjSolutionAttributes::~rspfAdjSolutionAttributes()
{
	if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG)
		<< "DEBUG: ~rspfAdjSolutionAttributes(): returning..." << std::endl;
}


//*****************************************************************************
// METHOD:      operator <<
//
// DESCRIPTION: Output point parameters.
//
// PARAMETERS:  N/A
//
// RETURN:      N/A
//*****************************************************************************
std::ostream & operator << (std::ostream &output, rspfAdjSolutionAttributes& /* data */)
{

	 output << "rspfAdjSolutionAttributes..." << std::endl;

	 return output;
}
