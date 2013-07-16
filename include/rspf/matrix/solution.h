#include <rspf/matrix/myexcept.h>
#ifdef use_namespace
namespace RBD_COMMON {
#endif
class R1_R1
{
protected:
   Real x;                             // Current x value
   bool xSet;                          // true if a value assigned to x
public:
   virtual ~R1_R1();
   
   Real minX, maxX;                    // range of value x
   bool minXinf, maxXinf;              // true if these are infinite
   R1_R1() : xSet(false), minXinf(true), maxXinf(true) {}
   virtual Real operator()() = 0;      // function value at current x
   virtual void Set(Real X);           // set x, check OK
   Real operator()(Real X) { Set(X); return operator()(); }
   virtual bool IsValid(Real X);
   operator Real();                    // implicit conversion
};
class SolutionException : public Exception
{
public:
   static unsigned long Select;
   SolutionException(const char* a_what = 0);
};
class OneDimSolve
{
   R1_R1& function;                     // reference to the function
   Real accX;                           // accuracy in X direction
   Real accY;                           // accuracy in Y direction
   int lim;                             // maximum number of iterations
public:
   OneDimSolve(R1_R1& f, Real AccY = 0.0001, Real AccX = 0.0)
      : function(f), accX(AccX), accY(AccY) {}
   Real Solve(Real Y, Real X, Real Dev, int Lim=100);
private:
   Real x[3], y[3];                         // Trial values of X and Y
   int L,C,U,Last;                          // Locations of trial values
   int vpol, hpol;                          // polarities
   Real YY;                                 // target value
   int i;
   void LookAt(int);                        // get new value of function
   bool Finish;                             // true if LookAt finds conv.
   bool Captured;                           // true when target surrounded
   void VFlip();
   void HFlip();
   void Flip();
   void State(int I, int J, int K);
   void Linear(int, int, int);
   void Quadratic(int, int, int);
};
#ifdef use_namespace
}
#endif
