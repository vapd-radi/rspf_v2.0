#ifndef rspfMeanRadialLensDistortion_HEADER
#define rspfMeanRadialLensDistortion_HEADER
#include <vector>
#include <rspf/matrix/newmat.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/projection/rspfLensDistortion.h>
class rspfMeanRadialLensDistortion : public rspfLensDistortion
{
public:
   rspfMeanRadialLensDistortion()
   {
      theCoefficients.resize(4);
      theCoefficients[0] = 0.0;
      theCoefficients[1] = 0.0;
      theCoefficients[2] = 0.0;
      theCoefficients[3] = 0.0;
   }
  /*!
   * Instantiate a rspfMeanRadialLens with the calibrated data.
   * The data consists of the Principal point,
   * focal distance, radialDistance measures for the distortion
   * deltas, the radial distortion values.
   * 
   * example of construction:
   *
   * lets say we have the following:
   *
   * principal point = (.008 , -.001) mm
   *
   * and
   *
   * field angle       radial distance       deltaR (distortion)
   *_____________________________________________________________
   *  7.5 deg.             .004 mm             .0202 mm
   *  15 deg.              .007 mm             .0411 mm
   *  22.5 deg.            .007 mm             .0635 mm
   *  :
   *  :
   *  :
   *
   * then construct the rspfMeanRadialLensDistortion class to construct the
   * given focal length and principal point and the middle column as
   * the radial distance column vector and the third column as the
   * distortion values.
   */
/*    rspfMeanRadialLensDistortion(rspfDpt    calibratedPrincipalPoint, */
/*                                  const NEWMAT::ColumnVector &radialDistance, */
/*                                  const NEWMAT::ColumnVector &radialDistortion) */
   rspfMeanRadialLensDistortion(rspfDpt    calibratedPrincipalPoint,
                                 const NEWMAT::ColumnVector &radialDistance,
                                 const NEWMAT::ColumnVector &radialDistortion)
      :rspfLensDistortion(calibratedPrincipalPoint)
  {
     theCoefficients.resize(4);
     solveCoefficients(radialDistance, radialDistortion);
  }
      
  /*!
   * Returns the coefficients (k1, k2, k3, k4) for the deltaR polynomial:
   *
   *    deltaR = k1*r + k2*r^3 + k3*r^5 + k4*r^7
   */
  const std::vector<double>& getCoefficients()const{return theCoefficients;}
  std::vector<double>& getCoefficients(){return theCoefficients;}
  virtual std::ostream& print(std::ostream& out) const
  {return out;}
  /*!
   * dletaR computes how much we should adjust r.  The
   * return value is the adjustment amount.  This method is used
   * by adjustPoint.
   */
  double deltaR(double r)const;
  virtual void undistort(const rspfDpt& input, rspfDpt& output)const;
  
  /*!
   * solves the coefficients given the radial distances and distortion
   * for each distance.
   */
  void solveCoefficients(const NEWMAT::ColumnVector &radialDistance,
			 const NEWMAT::ColumnVector &radialDistortion);
  /*!
   * solves the coefficients given the radial distances and distortion
   * for each distance.
   */
  void solveCoefficients(const std::vector<double>& radialDistance,
			 const std::vector<double>& radialDistortion);
  
  virtual bool saveState(rspfKeywordlist& kwl,
                         const char* prefix = 0)const;
  
  virtual bool loadState(const rspfKeywordlist& kwl,
                         const char* prefix = 0);
  
protected:
  std::vector<double> theCoefficients;
  
TYPE_DATA  
};
#endif
