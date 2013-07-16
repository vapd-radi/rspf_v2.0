#define WANT_STREAM                  // include.h will get stream fns
#define WANT_MATH                    // include.h will get math fns
#include <cmath>
#include <rspf/matrix/include.h>
#include <rspf/matrix/myexcept.h>
#include <rspf/matrix/solution.h>
#ifdef use_namespace
namespace RBD_COMMON {
#endif
R1_R1::~R1_R1()
{
}
void R1_R1::Set(Real X)
{
   if ((!minXinf && X <= minX) || (!maxXinf && X >= maxX))
       Throw(SolutionException("X value out of range"));
   x = X; xSet = true;
}
R1_R1::operator Real()
{
   if (!xSet) Throw(SolutionException("Value of X not set"));
   Real y = operator()();
   return y;
}
unsigned long SolutionException::Select;
SolutionException::SolutionException(const char* a_what) : Exception()
{
   Select = Exception::Select;
   AddMessage("Error detected by solution package\n");
   AddMessage(a_what); AddMessage("\n");
   if (a_what) Tracer::AddTrace();
};
inline Real square(Real x) { return x*x; }
void OneDimSolve::LookAt(int V)
{
   lim--;
   if (!lim) Throw(SolutionException("Does not converge"));
   Last = V;
   Real yy = function(x[V]) - YY;
   Finish = (std::fabs(yy) <= accY) ||
      (Captured && std::fabs(x[L]-x[U]) <= accX );
   y[V] = vpol*yy;
}
void OneDimSolve::HFlip() { hpol=-hpol; State(U,C,L); }
void OneDimSolve::VFlip()
   { vpol = -vpol; y[0] = -y[0]; y[1] = -y[1]; y[2] = -y[2]; }
void OneDimSolve::Flip()
{
   hpol=-hpol; vpol=-vpol; State(U,C,L);
   y[0] = -y[0]; y[1] = -y[1]; y[2] = -y[2];
}
void OneDimSolve::State(int I, int J, int K) { L=I; C=J; U=K; }
void OneDimSolve::Linear(int I, int J, int K)
{
   x[J] = (x[I]*y[K] - x[K]*y[I])/(y[K] - y[I]);
}
void OneDimSolve::Quadratic(int I, int J, int K)
{
   Real YJK, YIK, YIJ, XKI, XKJ;
   YJK = y[J] - y[K]; YIK = y[I] - y[K]; YIJ = y[I] - y[J];
   XKI = (x[K] - x[I]);
   XKJ = (x[K]*y[J] - x[J]*y[K])/YJK;
   if ( square(YJK/YIK)>(x[K] - x[J])/XKI ||
      square(YIJ/YIK)>(x[J] - x[I])/XKI )
   {
      x[I] = XKJ;
   }
   else
   {
      XKI = (x[K]*y[I] - x[I]*y[K])/YIK;
      x[I] = (XKJ*y[I] - XKI*y[J])/YIJ;
   }
}
Real OneDimSolve::Solve(Real Y, Real X, Real Dev, int Lim)
{
   enum Loop { start, captured1, captured2, binary, finish };
   Tracer et("OneDimSolve::Solve");
   lim=Lim; Captured = false;
   if (Dev==0.0) Throw(SolutionException("Dev is zero"));
   L=0; C=1; U=2; vpol=1; hpol=1; y[C]=0.0; y[U]=0.0;
   if (Dev<0.0) { hpol=-1; Dev = -Dev; }
   YY=Y;                                // target value
   x[L] = X;                            // initial trial value
   if (!function.IsValid(X))
      Throw(SolutionException("Starting value is invalid"));
   Loop TheLoop = start;
   for (;;)
   {
      switch (TheLoop)
      {
      case start:
         LookAt(L); if (Finish) { TheLoop = finish; break; }
         if (y[L]>0.0) VFlip();               // so Y[L] < 0
         x[U] = X + Dev * hpol;
         if (!function.maxXinf && x[U] > function.maxX)
            x[U] = (function.maxX + X) / 2.0;
         if (!function.minXinf && x[U] < function.minX)
            x[U] = (function.minX + X) / 2.0;
         LookAt(U); if (Finish) { TheLoop = finish; break; }
         if (y[U] > 0.0) { TheLoop = captured1; Captured = true; break; }
         if (y[U] == y[L])
            Throw(SolutionException("Function is flat"));
         if (y[U] < y[L]) HFlip();             // Change direction
         State(L,U,C);
         for (i=0; i<20; i++)
         {
            x[U] = x[C] + Dev * hpol;
            if (!function.maxXinf && x[U] > function.maxX)
            x[U] = (function.maxX + x[C]) / 2.0;
            if (!function.minXinf && x[U] < function.minX)
            x[U] = (function.minX + x[C]) / 2.0;
            LookAt(U); if (Finish) { TheLoop = finish; break; }
            if (y[U] > 0) { TheLoop = captured2; Captured = true; break; }
            if (y[U] < y[C])
                Throw(SolutionException("Function is not monotone"));
            Dev *= 2.0;
            State(C,U,L);
         }
         if (TheLoop != start ) break;
         Throw(SolutionException("Cannot locate a crossing point"));
      case captured1:
         Linear(L,C,U);                   // linear interpolation
         LookAt(C); if (Finish) { TheLoop = finish; break; }
         if (y[C] > 0.0) Flip();            // Want y[C] < 0
         if (y[C] < 0.5*y[L]) { State(C,L,U); TheLoop = binary; break; }
      case captured2:
         Quadratic(L,C,U);                // quad interpolation
         State(C,L,U);
         if ((x[C] - x[L])*hpol <= 0.0 || (x[C] - x[U])*hpol >= 0.0)
            { TheLoop = captured1; break; }
         LookAt(C); if (Finish) { TheLoop = finish; break; }
         if (y[C] > 0.0) Flip();
         if (y[C] > 0.5*y[L]) { TheLoop = captured2; break; }
         else { State(C,L,U); TheLoop = captured1; break; }
      case binary:
         for (i=3; i; i--)
         {
            x[C] = 0.5*(x[L]+x[U]);
            LookAt(C); if (Finish) { TheLoop = finish; break; }
            if (y[C]>0.0) State(L,U,C); else State(C,L,U);
         }
         if (TheLoop != binary) break;
         TheLoop = captured1; break;
      case finish:
	 return x[Last];
      }
   }
}
bool R1_R1::IsValid(Real X)
{
   Set(X);
   return (minXinf || x > minX) && (maxXinf || x < maxX);
}
#ifdef use_namespace
}
#endif