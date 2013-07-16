//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Hicks
// test
//
// Description: Weighted least squares bundle adjustment solution.
//----------------------------------------------------------------------------
#ifndef rspfWLSBundleSolution_HEADER
#define rspfWLSBundleSolution_HEADER

#include <rspf/matrix/newmat.h>
#include <rspf/matrix/newmatap.h>
#include <rspf/matrix/newmatio.h>
#include <vector>

class rspfAdjSolutionAttributes;


class RSPF_DLL rspfWLSBundleSolution
{
public:

   /**
    * @brief Constructor
    */
   rspfWLSBundleSolution();
   
   /**
    * @brief Run solution
    */
   bool run(rspfAdjSolutionAttributes* solAttributes);
   
   /**
    * @brief Destructor
    */
   ~rspfWLSBundleSolution();


protected:
   bool theSolValid;

   // Internal solution methods
   bool solveSystem(double* d, double* c, double* delta, int jb);
   bool recurFwd(double* d, double* c, std::vector<double>& rc, std::vector<int>& nz, int jb);
   bool recurBack(double* d, int jb);
   void trimv(double* pc, double* h, int pcIndex, int hIndex, int mr, std::vector<double>& sum);
   void moveAndNegate(std::vector<double>& from, double* to, int indexFrom, int indexTo, int nElements);

};

#endif // #ifndef rspfWLSBundleSolution_HEADER
