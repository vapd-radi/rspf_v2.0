//**************************************************************************************************
//                          RSPF -- Open Source Software Image Map
//
// LICENSE: See top level LICENSE.txt file.
//
// AUTHOR: Oscar Kramer, GeoEye Inc.
//
// DESCRIPTION: Contains Lagrange Interpolator class
// 
//  $Id$
//**************************************************************************************************
#ifndef LagrangeInterp_HEADER
#define LagrangeInterp_HEADER

#include <iostream>
#include <vector>
#include <rspf/matrix/newmat.h>
#include <rspf/base/rspfReferenced.h>

//******************************************************************************
// CLASS:  rspfLagrangeInterpolator
//******************************************************************************
class rspfLagrangeInterpolator : public rspfReferenced
{
   friend std::ostream& operator<<(std::ostream&, const rspfLagrangeInterpolator&);
   friend std::istream& operator>>(std::istream&, rspfLagrangeInterpolator&);
   
public:
   rspfLagrangeInterpolator() : theNumElements(0) {}
   rspfLagrangeInterpolator(std::istream&);
   rspfLagrangeInterpolator(const std::vector<double>& t_array, 
                             const std::vector<NEWMAT::ColumnVector>&  data_array);
   ~rspfLagrangeInterpolator();
   
   void addData(const double& t, const NEWMAT::ColumnVector& data);

   bool interpolate(const double& t, NEWMAT::ColumnVector& result) const;
   
private:
   std::vector<double> theTeeArray;
   std::vector<NEWMAT::ColumnVector> theDataArray;
   std::vector<double> theNormalizer;
   rspf_uint32 theNumElements;
};

#endif
