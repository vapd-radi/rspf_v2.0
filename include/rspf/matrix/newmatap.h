#ifndef NEWMATAP_LIB
#define NEWMATAP_LIB 0
#include <rspf/matrix/newmat.h>
#ifdef use_namespace
namespace NEWMAT {
#endif
void QRZT(Matrix&, LowerTriangularMatrix&);
void QRZT(const Matrix&, Matrix&, Matrix&);
void QRZ(Matrix&, UpperTriangularMatrix&);
void QRZ(const Matrix&, Matrix&, Matrix&);
inline void HHDecompose(Matrix& X, LowerTriangularMatrix& L)
{ QRZT(X,L); }
inline void HHDecompose(const Matrix& X, Matrix& Y, Matrix& M)
{ QRZT(X, Y, M); }
void UpdateQRZT(Matrix& X, LowerTriangularMatrix& L);
void UpdateQRZ(Matrix& X, UpperTriangularMatrix& U);
ReturnMatrix Cholesky(const SymmetricMatrix&);
ReturnMatrix Cholesky(const SymmetricBandMatrix&);
void UpdateCholesky(UpperTriangularMatrix &chol, RowVector r1Modification);
void DowndateCholesky(UpperTriangularMatrix &chol, RowVector x);
void RightCircularUpdateCholesky(UpperTriangularMatrix &chol, int k, int l);
void LeftCircularUpdateCholesky(UpperTriangularMatrix &chol, int k, int l); 
void SVD(const Matrix&, DiagonalMatrix&, Matrix&, Matrix&,
    bool=true, bool=true);
void SVD(const Matrix&, DiagonalMatrix&);
inline void SVD(const Matrix& A, DiagonalMatrix& D, Matrix& U,
   bool withU = true) { SVD(A, D, U, U, withU, false); }
void SortSV(DiagonalMatrix& D, Matrix& U, bool ascending = false);
void SortSV(DiagonalMatrix& D, Matrix& U, Matrix& V, bool ascending = false);
void Jacobi(const SymmetricMatrix&, DiagonalMatrix&);
void Jacobi(const SymmetricMatrix&, DiagonalMatrix&, SymmetricMatrix&);
void Jacobi(const SymmetricMatrix&, DiagonalMatrix&, Matrix&);
void Jacobi(const SymmetricMatrix&, DiagonalMatrix&, SymmetricMatrix&,
   Matrix&, bool=true);
void EigenValues(const SymmetricMatrix&, DiagonalMatrix&);
void EigenValues(const SymmetricMatrix&, DiagonalMatrix&, SymmetricMatrix&);
void EigenValues(const SymmetricMatrix&, DiagonalMatrix&, Matrix&);
class SymmetricEigenAnalysis
{
public:
   SymmetricEigenAnalysis(const SymmetricMatrix&);
private:
   DiagonalMatrix diag;
   DiagonalMatrix offdiag;
   SymmetricMatrix backtransform;
   FREE_CHECK(SymmetricEigenAnalysis)
};
void SortAscending(GeneralMatrix&);
void SortDescending(GeneralMatrix&);
class FFT_Controller
{
public:
   static bool OnlyOldFFT;
   static bool ar_1d_ft (int PTS, Real* X, Real *Y);
   static bool CanFactor(int PTS);
};
void FFT(const ColumnVector&, const ColumnVector&,
   ColumnVector&, ColumnVector&);
void FFTI(const ColumnVector&, const ColumnVector&,
   ColumnVector&, ColumnVector&);
void RealFFT(const ColumnVector&, ColumnVector&, ColumnVector&);
void RealFFTI(const ColumnVector&, const ColumnVector&, ColumnVector&);
void DCT_II(const ColumnVector&, ColumnVector&);
void DCT_II_inverse(const ColumnVector&, ColumnVector&);
void DST_II(const ColumnVector&, ColumnVector&);
void DST_II_inverse(const ColumnVector&, ColumnVector&);
void DCT(const ColumnVector&, ColumnVector&);
void DCT_inverse(const ColumnVector&, ColumnVector&);
void DST(const ColumnVector&, ColumnVector&);
void DST_inverse(const ColumnVector&, ColumnVector&);
void FFT2(const Matrix& U, const Matrix& V, Matrix& X, Matrix& Y);
void FFT2I(const Matrix& U, const Matrix& V, Matrix& X, Matrix& Y);
class MultiRadixCounter
{
   const SimpleIntArray& Radix;
   SimpleIntArray& Value;     // value of each digit
   const int n;               // number of digits
   int reverse;               // value when order of digits is reversed
   int product;               // product of radices
   int counter;               // counter
   bool finish;               // true when we have gone over whole range
public:
   MultiRadixCounter(int nx, const SimpleIntArray& rx,
      SimpleIntArray& vx);
   void operator++();         // increment the multi-radix counter
   bool Swap() const { return reverse < counter; }
   bool Finish() const { return finish; }
   int Reverse() const { return reverse; }
   int Counter() const { return counter; }
};
#ifdef use_namespace
}
#endif
#endif
