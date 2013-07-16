#ifndef rspfFilter_HEADER
#define rspfFilter_HEADER
#include <math.h>
#include <rspf/matrix/newmat.h>
#include <rspf/base/rspfRtti.h>


class rspfFilter
{
public:
   rspfFilter()
      {}
   virtual ~rspfFilter() {}
   
   virtual double filter (double x, double support)const = 0;
   /*!
    * Will call the filter method to create a
    * convolution matrix.  This matrix will not
    * have normalized weights.
    *
    * Will generate a matrix by creating an outer
    * product of the 1-D filter array.  This matrix
    * will be square width-by-width.
    *
    * note:  the matrix is a new matrix and needs to
    *        be destroyed by the caller
    */
   virtual NEWMAT::Matrix *newMatrix(long width=3,
                                     double middle=0.0,
                                     double scale=0.0)const;
   virtual void createMatrix(NEWMAT::Matrix& m,
                             long width=3,
                             double middle=0.0,
                             double scale=0.0)const;
   
   virtual NEWMAT::RowVector *newVector(long width,
                                        double middle=0.0,
                                        double scale = 1.0)const;
   static double Sinc(double x)
      {
         if (x != 0.0)
         {
            x*=M_PI;
            return(sin(x)/x);
         }
         
         return(1.0);
      }
   static double Sinc(double x, double /* support */ )
      {
         if (x != 0.0)
         {
            x*=M_PI;
            return(sin(x)/x);
         }
         
         return(1.0);
      }

   virtual double getSupport()const=0;
   
TYPE_DATA
};

class rspfBoxFilter : public rspfFilter
{
public:
   rspfBoxFilter (){}
   virtual ~rspfBoxFilter() {}
   
   virtual double filter (double x, double /* support */ )const
      {
         if (x < -0.5)
            return(0.0);
         if (x < 0.5)
            return(1.0);
         return(0.0);
      }
   virtual double getSupport()const
      {
         return .5;
      }
TYPE_DATA
};

class rspfSincFilter : public rspfFilter
{
public:
   rspfSincFilter(){}
   virtual ~rspfSincFilter(){}
   double filter(double x, double /* support */ )const
      {
         x*=M_PI;
         if (x != 0.0)
            return(sin(x)/x);
         return(1.0);
      }
   virtual double getSupport()const
      {
         return 1.0;
      }
TYPE_DATA
};

class rspfBellFilter : public rspfFilter
{
public:
   rspfBellFilter(){}
   virtual ~rspfBellFilter(){}
   virtual double getSupport()const
      {
         return 1.5;
      }
   virtual double filter(double value, double /* support */ )const
      {
         if(value < 0) value = -value;
         if(value < .5) return(.75 - (value * value));
         if(value < 1.5) {
            value = (value - 1.5);
            return(.5 * (value * value));
         }
         return(0.0);
      }
TYPE_DATA
};

class rspfNearestNeighborFilter: public rspfBoxFilter
{
public:
   rspfNearestNeighborFilter (){}
   virtual ~rspfNearestNeighborFilter() {}
   
   virtual double getSupport()const
      {
         return 0.0;
      }
TYPE_DATA
};

   
class rspfBesselOrderOneFilter : public rspfFilter
{
public:
   rspfBesselOrderOneFilter(){}
   virtual ~rspfBesselOrderOneFilter(){}
   virtual double filter(double value, double /* support */ )const;
   virtual double getSupport()const
      {
         return 1.0;
      }
TYPE_DATA
};
class rspfBesselFilter : public rspfFilter
{
public:
   rspfBesselFilter(){}
   virtual ~rspfBesselFilter(){}
   virtual double filter(double x, double support)const
      {
         if (x == 0.0)
         {
            return(M_PI/4.0);
         }
         
         return(rspfBesselOrderOneFilter().filter((M_PI*x)/(2.0*x), support));
      }
   virtual double getSupport()const
      {
         return 1.0;
      }
   
TYPE_DATA
};

class rspfBlackmanFilter : public rspfFilter
{
public:
    rspfBlackmanFilter (){}
    virtual ~rspfBlackmanFilter() {}

    virtual double filter (double x, double /* support */ ) const
        {
           return(0.42+0.50*cos(M_PI*x)+0.08*cos(2.0*M_PI*x));
        }
   virtual double getSupport()const
      {
         return 1.0;
      }
TYPE_DATA
};

class rspfBlackmanSincFilter : public rspfBlackmanFilter
{
public:
    rspfBlackmanSincFilter (){}
    virtual ~rspfBlackmanSincFilter() {}

    virtual double filter (double x, double support) const
        {
           return ((rspfBlackmanFilter::filter(x/support,support))*
                   (rspfSincFilter().filter(x, support)));
        }
   virtual double getSupport()const
      {
         return 4.0;
      }
TYPE_DATA
};

class rspfBlackmanBesselFilter : public rspfBlackmanFilter
{
public:
    rspfBlackmanBesselFilter(){}
    virtual ~rspfBlackmanBesselFilter() {}

    virtual double filter (double x, double support) const
        {
           return ((rspfBlackmanFilter::filter(x/support,support))*
                   (rspfBesselFilter().filter(x, support)));
        }
   virtual double getSupport()const
      {
         return 3.2383;
      }
TYPE_DATA
};


class rspfCatromFilter : public rspfFilter
{
public:
   rspfCatromFilter(){}
   virtual ~rspfCatromFilter(){}
   double filter(double x, double /* support */ )const
      {
         if (x < -2.0)
            return(0.0);
         if (x < -1.0)
            return(0.5*(4.0+x*(8.0+x*(5.0+x))));
         if (x < 0.0)
            return(0.5*(2.0+x*x*(-5.0-3.0*x)));
         if (x < 1.0)
            return(0.5*(2.0+x*x*(-5.0+3.0*x)));
         if (x < 2.0)
            return(0.5*(4.0+x*(-8.0+x*(5.0-x))));
         return(0.0);
      }
   virtual double getSupport()const
      {
         return 2.0;
      }
TYPE_DATA
};

class rspfCubicFilter : public rspfFilter
{
public:
   rspfCubicFilter(){}
   virtual ~rspfCubicFilter(){}
   double filter(double x, double /* support */ )const
      {
         if (x < -2.0)
            return(0.0);
         if (x < -1.0)
            return((2.0+x)*(2.0+x)*(2.0+x)/6.0);
         if (x < 0.0)
            return((4.0+x*x*(-6.0-3.0*x))/6.0);
         if (x < 1.0)
            return((4.0+x*x*(-6.0+3.0*x))/6.0);
         if (x < 2.0)
            return((2.0-x)*(2.0-x)*(2.0-x)/6.0);
         return(0.0);
      }
   virtual double getSupport()const
      {
         return 2.0;
      }
TYPE_DATA
};

class rspfBSplineFilter : public rspfFilter
{
public:
   rspfBSplineFilter(){}
   virtual ~rspfBSplineFilter(){}
   virtual double filter(double value, double /* support */ )const
      {
	double tt;

	if(value < 0) value = -value;
	if(value < 1) {
		tt = value * value;
		return((.5 * tt * value) - tt + (2.0 / 3.0));
	} else if(value < 2) {
		value = 2 - value;
		return((1.0 / 6.0) * (value * value * value));
	}
	return(0.0);
      }
   virtual double getSupport()const
      {
         return 2.0;
      }
TYPE_DATA
};

class rspfGaussianFilter : public rspfFilter
{
public:
    rspfGaussianFilter (){}
    virtual ~rspfGaussianFilter() {}

    virtual double filter (double x, double /* support */ ) const
        {
           return(exp(-2.0*x*x)*sqrt(2.0/M_PI));
        }
   virtual double getSupport()const
      {
         return 1.25;
      }
TYPE_DATA
};

class rspfHanningFilter : public rspfFilter
{
public:
   rspfHanningFilter(){}
   virtual ~rspfHanningFilter(){}
   double filter(double x, double /* support */ )const
      {
         return(0.5+0.5*cos(M_PI*x));
      }
   virtual double getSupport()const
      {
         return 1.0;
      }
TYPE_DATA
};

class rspfHammingFilter : public rspfFilter
{
public:
    rspfHammingFilter (){}
    virtual ~rspfHammingFilter() {}

    virtual double filter (double x, double /* support */ ) const
        {
           return(0.54+0.46*cos(M_PI*x));
        }
   virtual double getSupport()const
      {
         return 1.0;
      }
TYPE_DATA
};

class rspfHermiteFilter : public rspfFilter
{
public:
   rspfHermiteFilter(){}
   virtual ~rspfHermiteFilter(){}
   double filter(double x, double /* support */ )const
      {
         if (x < -1.0)
            return(0.0);
         if (x < 0.0)
            return((2.0*(-x)-3.0)*(-x)*(-x)+1.0);
         if (x < 1.0)
            return((2.0*x-3.0)*x*x+1.0);
         return(0.0);
      }
   virtual double getSupport()const
      {
         return 1.0;
      }
TYPE_DATA
};


class rspfLanczosFilter : public rspfFilter
{
public:
   rspfLanczosFilter(){}
   virtual ~rspfLanczosFilter(){}
   double filter(double x, double /* support */ )const
      {
         if (x < -3.0)
            return(0.0);
         if (x < 0.0)
            return(Sinc(-x,getSupport())*Sinc(-x/3.0,getSupport()));
         if (x < 3.0)
            return(Sinc(x,getSupport())*Sinc(x/3.0,getSupport()));
         return(0.0);
      }
   virtual double getSupport()const
      {
         return 3.0;
      }
TYPE_DATA
};

class rspfMitchellFilter : public rspfFilter
{
public:
   rspfMitchellFilter(){}
   virtual ~rspfMitchellFilter(){}
   double filter(double x, double /* support */ )const
      {
#define MITCHELL_B   (1.0/3.0)
#define MITCHELL_C   (1.0/3.0)
#define MITCHELL_P0  ((  6.0- 2.0*MITCHELL_B       )/6.0)
#define MITCHELL_P2  ((-18.0+12.0*MITCHELL_B+ 6.0*MITCHELL_C)/6.0)
#define MITCHELL_P3  (( 12.0- 9.0*MITCHELL_B- 6.0*MITCHELL_C)/6.0)
#define MITCHELL_Q0  ((       8.0*MITCHELL_B+24.0*MITCHELL_C)/6.0)
#define MITCHELL_Q1  ((     -12.0*MITCHELL_B-48.0*MITCHELL_C)/6.0)
#define MITCHELL_Q2  ((       6.0*MITCHELL_B+30.0*MITCHELL_C)/6.0)
#define MITCHELL_Q3  ((     - 1.0*MITCHELL_B- 6.0*MITCHELL_C)/6.0)
         
         if (x < -2.0)
            return(0.0);
         if (x < -1.0)
            return(MITCHELL_Q0-x*(MITCHELL_Q1-x*(MITCHELL_Q2-x*MITCHELL_Q3)));
         if (x < 0.0)
            return(MITCHELL_P0+x*x*(MITCHELL_P2-x*MITCHELL_P3));
         if (x < 1.0)
            return(MITCHELL_P0+x*x*(MITCHELL_P2+x*MITCHELL_P3));
         if (x < 2.0)
            return(MITCHELL_Q0+x*(MITCHELL_Q1+x*(MITCHELL_Q2+x*MITCHELL_Q3)));
         return(0.0);
      }
   virtual double getSupport()const
      {
         return 2.0;
      }
TYPE_DATA
};

class rspfQuadraticFilter : public rspfFilter
{
public:
   rspfQuadraticFilter(){}
   virtual ~rspfQuadraticFilter(){}
   double filter(double x, double /* support */ )const
      {
         if (x < -1.5)
            return(0.0);
         if (x < -0.5)
            return(0.5*(x+1.5)*(x+1.5));
         if (x < 0.5)
            return(0.75-x*x);
         if (x < 1.5)
            return(0.5*(x-1.5)*(x-1.5));
         return(0.0);
      }
   double getSupport()const
      {
         return 1.5;
      }
TYPE_DATA
};

class rspfTriangleFilter : public rspfFilter
{
public:
   rspfTriangleFilter(){}
   virtual ~rspfTriangleFilter() {}
   
   virtual double filter (double x, double /* support */ )const
      {
         if (x < -1.0)
            return(0.0);
         if (x < 0.0)
            return(1.0+x);
         if (x < 1.0)
            return(1.0-x);
         return(0.0);
      }
   virtual double getSupport()const
      {
         return 1.0;
      }
TYPE_DATA
};


#endif
