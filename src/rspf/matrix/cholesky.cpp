#define WANT_MATH
#include <cmath>
#include <rspf/matrix/include.h>
#include <rspf/matrix/newmat.h>
#include <rspf/matrix/newmatrm.h>
#ifdef use_namespace
namespace NEWMAT {
#endif
#ifdef DO_REPORT
#define REPORT { static ExeCounter ExeCount(__LINE__,14); ++ExeCount; }
#else
#define REPORT {}
#endif
/********* Cholesky decomposition of a positive definite matrix *************/
ReturnMatrix Cholesky(const SymmetricMatrix& S)
{
   REPORT
   Tracer trace("Cholesky");
   int nr = S.Nrows();
   LowerTriangularMatrix T(nr);
   Real* s = S.Store(); Real* t = T.Store(); Real* ti = t;
   for (int i=0; i<nr; i++)
   {
      Real* tj = t; Real sum; int k;
      for (int j=0; j<i; j++)
      {
         Real* tk = ti; sum = 0.0; k = j;
         while (k--) { sum += *tj++ * *tk++; }
         *tk = (*s++ - sum) / *tj++;
      }
      sum = 0.0; k = i;
      while (k--) { sum += square(*ti++); }
      Real d = *s++ - sum;
      if (d<=0.0)  Throw(NPDException(S));
      *ti++ = std::sqrt(d);
   }
   T.Release(); return T.ForReturn();
}
ReturnMatrix Cholesky(const SymmetricBandMatrix& S)
{
   REPORT
   Tracer trace("Band-Cholesky");
   int nr = S.Nrows(); int m = S.lower;
   LowerBandMatrix T(nr,m);
   Real* s = S.Store(); Real* t = T.Store(); Real* ti = t;
   for (int i=0; i<nr; i++)
   {
      Real* tj = t; Real sum; int l;
      if (i<m) { REPORT l = m-i; s += l; ti += l; l = i; }
      else { REPORT t += (m+1); l = m; }
      for (int j=0; j<l; j++)
      {
         Real* tk = ti; sum = 0.0; int k = j; tj += (m-j);
         while (k--) { sum += *tj++ * *tk++; }
         *tk = (*s++ - sum) / *tj++;
      }
      sum = 0.0;
      while (l--) { sum += square(*ti++); }
      Real d = *s++ - sum;
      if (d<=0.0)  Throw(NPDException(S));
      *ti++ = std::sqrt(d);
   }
   T.Release(); return T.ForReturn();
}
void UpdateCholesky(UpperTriangularMatrix &chol, RowVector r1Modification)
{
   int ncols = chol.Nrows();
   ColumnVector cGivens(ncols); cGivens = 0.0;
   ColumnVector sGivens(ncols); sGivens = 0.0;
	
   for(int j = 1; j <= ncols; ++j) // process the jth column of chol
   {
      for(int k = 1; k < j; ++k)
         GivensRotation(cGivens(k), sGivens(k), chol(k,j), r1Modification(j));
      pythag(chol(j,j), r1Modification(j), cGivens(j), sGivens(j));
      {
         Real tmp0 = cGivens(j) * chol(j,j) + sGivens(j) * r1Modification(j);
         chol(j,j) = tmp0; r1Modification(j) = 0.0;
      }
   }
}
void DowndateCholesky(UpperTriangularMatrix &chol, RowVector x)
{
   int nRC = chol.Nrows();
	
   LowerTriangularMatrix L = chol.t();
   ColumnVector a(nRC); a = 0.0;
   int i, j;
	
   for (i = 1; i <= nRC; ++i)
   {
      Real subtrsum = 0.0;
      for(int k = 1; k < i; ++k) subtrsum += a(k) * L(i,k);
      a(i) = (x(i) - subtrsum) / L(i,i);
   }
   Real squareNormA = a.SumSquare();
   if (squareNormA >= 1.0)
      Throw(ProgramException("DowndateCholesky() fails", chol));
   Real alpha = std::sqrt(1.0 - squareNormA);
   ColumnVector cGivens(nRC);  cGivens = 0.0;
   ColumnVector sGivens(nRC);  sGivens = 0.0;
   for(i = nRC; i >= 1; i--)
      alpha = pythag(alpha, a(i), cGivens(i), sGivens(i));
   ColumnVector xtilde(nRC); xtilde = 0.0;
   for(j = nRC; j >= 1; j--)
   {
      for(int k = j; k >= 1; k--)
         GivensRotation(cGivens(k), -sGivens(k), chol(k,j), xtilde(j));
   }
}
void RightCircularUpdateCholesky(UpperTriangularMatrix &chol, int k, int l)
{
   int nRC = chol.Nrows();
   int i, j;
	
   Matrix cholCopy = chol;
   ColumnVector columnL = cholCopy.Column(l);
   for(j = l-1; j >= k; --j)
      cholCopy.Column(j+1) = cholCopy.Column(j);
   cholCopy.Column(k) = 0.0;
   for(i = 1; i < k; ++i) cholCopy(i,k) = columnL(i);
   int nGivens = l-k;
   ColumnVector cGivens(nGivens); cGivens = 0.0;
   ColumnVector sGivens(nGivens); sGivens = 0.0;
   for(i = l; i > k; i--)
   {
      int givensIndex = l-i+1;
      columnL(i-1) = pythag(columnL(i-1), columnL(i),
         cGivens(givensIndex), sGivens(givensIndex));
      columnL(i) = 0.0;
   }
   cholCopy(k,k) = columnL(k);
	
   for(j = k+1; j <= nRC; ++j)
   {
      ColumnVector columnJ = cholCopy.Column(j);
      int imin = nGivens - (j-k) + 1; if (imin < 1) imin = 1;
      for(int gIndex = imin; gIndex <= nGivens; ++gIndex)
      {
         int topRowIndex = k + nGivens - gIndex;
         GivensRotationR(cGivens(gIndex), sGivens(gIndex),
            columnJ(topRowIndex), columnJ(topRowIndex+1));
      }
      cholCopy.Column(j) = columnJ;
   }
   chol << cholCopy;
}
void LeftCircularUpdateCholesky(UpperTriangularMatrix &chol, int k, int l)
{
   int nRC = chol.Nrows();
   int i, j;
   Matrix cholCopy = chol;
   ColumnVector columnK = cholCopy.Column(k);
   for(j = k+1; j <= l; ++j)
      cholCopy.Column(j-1) = cholCopy.Column(j);
   cholCopy.Column(l) = 0.0;
   for(i = 1; i <= k; ++i)
      cholCopy(i,l) = columnK(i);
   int nGivens = l-k;
   ColumnVector cGivens(nGivens); cGivens = 0.0;
   ColumnVector sGivens(nGivens); sGivens = 0.0;
   for(j = k; j <= nRC; ++j)
   {
      ColumnVector columnJ = cholCopy.Column(j);
      int imax = j - k; if (imax > nGivens) imax = nGivens;
      for(int i = 1; i <= imax; ++i)
      {
         int gIndex = i;
         int topRowIndex = k + i - 1;
         GivensRotationR(cGivens(gIndex), sGivens(gIndex),
            columnJ(topRowIndex), columnJ(topRowIndex+1));
      }
      if(j < l)
      {
         int gIndex = j-k+1;
         columnJ(j) = pythag(columnJ(j), columnJ(j+1), cGivens(gIndex), sGivens(gIndex));
         columnJ(j+1) = 0.0;
      }
      cholCopy.Column(j) = columnJ;
   }
   chol << cholCopy;
	
}
#ifdef use_namespace
}
#endif
