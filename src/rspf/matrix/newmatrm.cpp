#define WANT_MATH
#include <cmath>
#include <rspf/matrix/newmat.h>
#include <rspf/matrix/newmatrm.h>
#ifdef use_namespace
namespace NEWMAT {
#endif
#ifdef DO_REPORT
#define REPORT { static ExeCounter ExeCount(__LINE__,12); ++ExeCount; }
#else
#define REPORT {}
#endif
void RectMatrixRow::Reset (const Matrix& M, int row, int skip, int length)
{
   REPORT
   RectMatrixRowCol::Reset
      ( M.Store()+row*M.Ncols()+skip, length, 1, M.Ncols() );
}
void RectMatrixRow::Reset (const Matrix& M, int row)
{
   REPORT
   RectMatrixRowCol::Reset( M.Store()+row*M.Ncols(), M.Ncols(), 1, M.Ncols() );
}
void RectMatrixCol::Reset (const Matrix& M, int skip, int col, int length)
{
   REPORT
   RectMatrixRowCol::Reset
      ( M.Store()+col+skip*M.Ncols(), length, M.Ncols(), 1 );
}
void RectMatrixCol::Reset (const Matrix& M, int col)
{
   REPORT
   RectMatrixRowCol::Reset( M.Store()+col, M.Nrows(), M.Ncols(), 1 );
}
Real RectMatrixRowCol::SumSquare() const
{
   REPORT
   long_Real sum = 0.0; int i = n; Real* s = store; int d = spacing;
   if (i) for(;;)
      { sum += (long_Real)*s * *s; if (!(--i)) break; s += d; }
   return (Real)sum;
}
Real RectMatrixRowCol::operator*(const RectMatrixRowCol& rmrc) const
{
   REPORT
   long_Real sum = 0.0; int i = n;
   Real* s = store; int d = spacing;
   Real* s1 = rmrc.store; int d1 = rmrc.spacing;
   if (i!=rmrc.n)
   {
      Tracer tr("newmatrm");
      Throw(InternalException("Dimensions differ in *"));
   }
   if (i) for(;;)
      { sum += (long_Real)*s * *s1; if (!(--i)) break; s += d; s1 += d1; }
   return (Real)sum;
}
void RectMatrixRowCol::AddScaled(const RectMatrixRowCol& rmrc, Real r)
{
   REPORT
   int i = n; Real* s = store; int d = spacing;
   Real* s1 = rmrc.store; int d1 = rmrc.spacing;
   if (i!=rmrc.n)
   {
      Tracer tr("newmatrm");
      Throw(InternalException("Dimensions differ in AddScaled"));
   }
   if (i) for (;;)
      { *s += *s1 * r; if (!(--i)) break; s += d; s1 += d1; }
}
void RectMatrixRowCol::Divide(const RectMatrixRowCol& rmrc, Real r)
{
   REPORT
   int i = n; Real* s = store; int d = spacing;
   Real* s1 = rmrc.store; int d1 = rmrc.spacing;
   if (i!=rmrc.n)
   {
      Tracer tr("newmatrm");
      Throw(InternalException("Dimensions differ in Divide"));
   }
   if (i) for (;;) { *s = *s1 / r; if (!(--i)) break; s += d; s1 += d1; }
}
void RectMatrixRowCol::Divide(Real r)
{
   REPORT
   int i = n; Real* s = store; int d = spacing;
   if (i) for (;;) { *s /= r; if (!(--i)) break; s += d; }
}
void RectMatrixRowCol::Negate()
{
   REPORT
   int i = n; Real* s = store; int d = spacing;
   if (i) for (;;) { *s = - *s; if (!(--i)) break; s += d; }
}
void RectMatrixRowCol::Zero()
{
   REPORT
   int i = n; Real* s = store; int d = spacing;
   if (i) for (;;) { *s = 0.0; if (!(--i)) break; s += d; }
}
void ComplexScale(RectMatrixCol& U, RectMatrixCol& V, Real x, Real y)
{
   REPORT
   int n = U.n;
   if (n != V.n)
   {
      Tracer tr("newmatrm");
      Throw(InternalException("Dimensions differ in ComplexScale"));
   }
   Real* u = U.store; Real* v = V.store; 
   int su = U.spacing; int sv = V.spacing;
   if (n) for (;;)
   {
      Real z = *u * x - *v * y;  *v =  *u * y + *v * x;  *u = z;
      if (!(--n)) break;
      u += su;  v += sv;
   }
}
void Rotate(RectMatrixCol& U, RectMatrixCol& V, Real tau, Real s)
{
   REPORT
   int n = U.n;
   if (n != V.n)
   {
      Tracer tr("newmatrm");
      Throw(InternalException("Dimensions differ in Rotate"));
   }
   Real* u = U.store; Real* v = V.store;
   int su = U.spacing; int sv = V.spacing;
   if (n) for(;;)
   {
      Real zu = *u; Real zv = *v;
      *u -= s * (zv + zu * tau); *v += s * (zu - zv * tau);
      if (!(--n)) break;
      u += su;  v += sv;
   }
}
Real pythag(Real f, Real g, Real& c, Real& s)
{
   if (f==0 && g==0) { c=1.0; s=0.0; return 0.0; }
   Real af = f>=0 ? f : -f;
   Real ag = g>=0 ? g : -g;
   if (ag<af)
   {
      REPORT
      Real h = g/f; Real sq = std::sqrt(1.0+h*h);
      if (f<0) sq = -sq;           // make return value non-negative
      c = 1.0/sq; s = h/sq; return sq*f;
   }
   else
   {
      REPORT
      Real h = f/g; Real sq = std::sqrt(1.0+h*h);
      if (g<0) sq = -sq;
      s = 1.0/sq; c = h/sq; return sq*g;
   }
}
#ifdef use_namespace
}
#endif
