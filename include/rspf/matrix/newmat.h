#ifndef NEWMAT_LIB
#define NEWMAT_LIB 0
#include <rspf/matrix/include.h>
#include <rspf/matrix/myexcept.h>
#ifdef use_namespace
namespace NEWMAT { using namespace RBD_COMMON; }
namespace RBD_LIBRARIES { using namespace NEWMAT; }
namespace NEWMAT {
#endif
#ifdef NO_LONG_NAMES
#define UpperTriangularMatrix UTMatrix
#define LowerTriangularMatrix LTMatrix
#define SymmetricMatrix SMatrix
#define DiagonalMatrix DMatrix
#define BandMatrix BMatrix
#define UpperBandMatrix UBMatrix
#define LowerBandMatrix LBMatrix
#define SymmetricBandMatrix SBMatrix
#define BandLUMatrix BLUMatrix
#endif
class GeneralMatrix;
void MatrixErrorNoSpace(const void*);                 // no space handler
class LogAndSign
{
   Real log_value;
   int sign;
public:
   LogAndSign() { log_value=0.0; sign=1; }
   LogAndSign(Real);
   void operator*=(Real);
   void PowEq(int k);  // raise to power of k
   void ChangeSign() { sign = -sign; }
   Real LogValue() const { return log_value; }
   int Sign() const { return sign; }
   Real Value() const;
   FREE_CHECK(LogAndSign)
};
#ifdef DO_REPORT
class ExeCounter
{
   int line;                                    // code line number
   int fileid;                                  // file identifier
   long nexe;                                   // number of executions
   static int nreports;                         // number of reports
public:
   ExeCounter(int,int);
   void operator++() { nexe++; }
   ~ExeCounter();                               // prints out reports
};
#endif
class GeneralMatrix;                            // defined later
class BaseMatrix;                               // defined later
class MatrixInput;                              // defined later
class RSPF_DLL MatrixType
{
public:
   enum Attribute {  Valid     = 1,
                     Diagonal  = 2,             // order of these is important
                     Symmetric = 4,
                     Band      = 8,
                     Lower     = 16,
                     Upper     = 32,
                     Square    = 64,
                     Skew      = 128,
                     LUDeco    = 256,
                     Ones      = 512 };
   enum            { US = 0,
                     UT = Valid + Upper + Square,
                     LT = Valid + Lower + Square,
                     Rt = Valid,
                     Sq = Valid + Square,
                     Sm = Valid + Symmetric + Square,
                     Sk = Valid + Skew + Square,
                     Dg = Valid + Diagonal + Band + Lower + Upper + Symmetric
                        + Square,
                     Id = Valid + Diagonal + Band + Lower + Upper + Symmetric
                        + Square + Ones,
                     RV = Valid,     //   do not separate out
                     CV = Valid,     //   vectors
                     BM = Valid + Band + Square,
                     UB = Valid + Band + Upper + Square,
                     LB = Valid + Band + Lower + Square,
                     SB = Valid + Band + Symmetric + Square,
                     Ct = Valid + LUDeco + Square,
                     BC = Valid + Band + LUDeco + Square,
                     Mask = ~Square
                   };
   static int nTypes() { return 12; }          // number of different types
					       // exclude Ct, US, BC
public:
   int attribute;
   bool DataLossOK;                            // true if data loss is OK when
public:
   MatrixType () : DataLossOK(false) {}
   MatrixType (int i) : attribute(i), DataLossOK(false) {}
   MatrixType (int i, bool dlok) : attribute(i), DataLossOK(dlok) {}
   MatrixType (const MatrixType& mt)
      : attribute(mt.attribute), DataLossOK(mt.DataLossOK) {}
   void operator=(const MatrixType& mt)
      { attribute = mt.attribute; DataLossOK = mt.DataLossOK; }
   void SetDataLossOK() { DataLossOK = true; }
   int operator+() const { return attribute; }
   MatrixType operator+(MatrixType mt) const
      { return MatrixType(attribute & mt.attribute); }
   MatrixType operator*(const MatrixType&) const;
   MatrixType SP(const MatrixType&) const;
   MatrixType KP(const MatrixType&) const;
   MatrixType operator|(const MatrixType& mt) const
      { return MatrixType(attribute & mt.attribute & Valid); }
   MatrixType operator&(const MatrixType& mt) const
      { return MatrixType(attribute & mt.attribute & Valid); }
   bool operator>=(MatrixType mt) const
      { return ( attribute & ~mt.attribute & Mask ) == 0; }
   bool operator<(MatrixType mt) const         // for MS Visual C++ 4
      { return ( attribute & ~mt.attribute & Mask ) != 0; }
   bool operator==(MatrixType t) const
      { return (attribute == t.attribute); }
   bool operator!=(MatrixType t) const
      { return (attribute != t.attribute); }
   bool operator!() const { return (attribute & Valid) == 0; }
   MatrixType i() const;                       // type of inverse
   MatrixType t() const;                       // type of transpose
   MatrixType AddEqualEl() const               // Add constant to matrix
      { return MatrixType(attribute & (Valid + Symmetric + Square)); }
   MatrixType MultRHS() const;                 // type for rhs of multiply
   MatrixType sub() const                      // type of submatrix
      { return MatrixType(attribute & Valid); }
   MatrixType ssub() const                     // type of sym submatrix
      { return MatrixType(attribute); }        // not for selection matrix
   GeneralMatrix* New() const;                 // new matrix of given type
   GeneralMatrix* New(int,int,BaseMatrix*) const;
   const char* Value() const;                  // to print type
   friend bool Rectangular(MatrixType a, MatrixType b, MatrixType c);
   friend bool Compare(const MatrixType&, MatrixType&);
   bool IsBand() const { return (attribute & Band) != 0; }
   bool IsDiagonal() const { return (attribute & Diagonal) != 0; }
   bool IsSymmetric() const { return (attribute & Symmetric) != 0; }
   bool CannotConvert() const { return (attribute & LUDeco) != 0; }
   FREE_CHECK(MatrixType)
};
class RSPF_DLL MatrixBandWidth
{
public:
   int lower;
   int upper;
   MatrixBandWidth(const int l, const int u) : lower(l), upper (u) {}
   MatrixBandWidth(const int i) : lower(i), upper(i) {}
   MatrixBandWidth operator+(const MatrixBandWidth&) const;
   MatrixBandWidth operator*(const MatrixBandWidth&) const;
   MatrixBandWidth minimum(const MatrixBandWidth&) const;
   MatrixBandWidth t() const { return MatrixBandWidth(upper,lower); }
   bool operator==(const MatrixBandWidth& bw) const
      { return (lower == bw.lower) && (upper == bw.upper); }
   bool operator!=(const MatrixBandWidth& bw) const { return !operator==(bw); }
   int Upper() const { return upper; }
   int Lower() const { return lower; }
   FREE_CHECK(MatrixBandWidth)
};
class RSPF_DLL ArrayLengthSpecifier
{
   int value;
public:
   int Value() const { return value; }
   ArrayLengthSpecifier(int l) : value(l) {}
};
class MatrixRowCol;                             // defined later
class MatrixRow;
class MatrixCol;
class MatrixColX;
class GeneralMatrix;                            // defined later
class AddedMatrix;
class MultipliedMatrix;
class SubtractedMatrix;
class SPMatrix;
class KPMatrix;
class ConcatenatedMatrix;
class StackedMatrix;
class SolvedMatrix;
class ShiftedMatrix;
class NegShiftedMatrix;
class ScaledMatrix;
class TransposedMatrix;
class ReversedMatrix;
class NegatedMatrix;
class InvertedMatrix;
class RowedMatrix;
class ColedMatrix;
class DiagedMatrix;
class MatedMatrix;
class GetSubMatrix;
class ReturnMatrix;
class Matrix;
class SquareMatrix;
class nricMatrix;
class RowVector;
class ColumnVector;
class SymmetricMatrix;
class UpperTriangularMatrix;
class LowerTriangularMatrix;
class DiagonalMatrix;
class CroutMatrix;
class BandMatrix;
class LowerBandMatrix;
class UpperBandMatrix;
class SymmetricBandMatrix;
class LinearEquationSolver;
class GenericMatrix;
#define MatrixTypeUnSp 0
class RSPF_DLL BaseMatrix : public RBD_COMMON::Janitor               // base of all matrix classes
{
protected:
   virtual int search(const BaseMatrix*) const = 0;
						// count number of times matrix
   						// is referred to
public:
   virtual GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp) = 0;
						// evaluate temporary
   AddedMatrix operator+(const BaseMatrix&) const;    // results of operations
   MultipliedMatrix operator*(const BaseMatrix&) const;
   SubtractedMatrix operator-(const BaseMatrix&) const;
   ConcatenatedMatrix operator|(const BaseMatrix&) const;
   StackedMatrix operator&(const BaseMatrix&) const;
   ShiftedMatrix operator+(Real) const;
   ScaledMatrix operator*(Real) const;
   ScaledMatrix operator/(Real) const;
   ShiftedMatrix operator-(Real) const;
   TransposedMatrix t() const;
   NegatedMatrix operator-() const;                   // change sign of elements
   ReversedMatrix Reverse() const;
   InvertedMatrix i() const;
   RowedMatrix AsRow() const;
   ColedMatrix AsColumn() const;
   DiagedMatrix AsDiagonal() const;
   MatedMatrix AsMatrix(int,int) const;
   GetSubMatrix SubMatrix(int,int,int,int) const;
   GetSubMatrix SymSubMatrix(int,int) const;
   GetSubMatrix Row(int) const;
   GetSubMatrix Rows(int,int) const;
   GetSubMatrix Column(int) const;
   GetSubMatrix Columns(int,int) const;
   Real AsScalar() const;                      // conversion of 1 x 1 matrix
   virtual LogAndSign LogDeterminant() const;
   Real Determinant() const;
   virtual Real SumSquare() const;
   Real NormFrobenius() const;
   virtual Real SumAbsoluteValue() const;
   virtual Real Sum() const;
   virtual Real MaximumAbsoluteValue() const;
   virtual Real MaximumAbsoluteValue1(int& i) const;
   virtual Real MaximumAbsoluteValue2(int& i, int& j) const;
   virtual Real MinimumAbsoluteValue() const;
   virtual Real MinimumAbsoluteValue1(int& i) const;
   virtual Real MinimumAbsoluteValue2(int& i, int& j) const;
   virtual Real Maximum() const;
   virtual Real Maximum1(int& i) const;
   virtual Real Maximum2(int& i, int& j) const;
   virtual Real Minimum() const;
   virtual Real Minimum1(int& i) const;
   virtual Real Minimum2(int& i, int& j) const;
   virtual Real Trace() const;
   Real Norm1() const;
   Real NormInfinity() const;
   virtual MatrixBandWidth BandWidth() const;  // bandwidths of band matrix
   virtual void CleanUp() {}                   // to clear store
   void IEQND() const;                         // called by ineq. ops
   friend class GeneralMatrix;
   friend class Matrix;
   friend class SquareMatrix;
   friend class nricMatrix;
   friend class RowVector;
   friend class ColumnVector;
   friend class SymmetricMatrix;
   friend class UpperTriangularMatrix;
   friend class LowerTriangularMatrix;
   friend class DiagonalMatrix;
   friend class CroutMatrix;
   friend class BandMatrix;
   friend class LowerBandMatrix;
   friend class UpperBandMatrix;
   friend class SymmetricBandMatrix;
   friend class AddedMatrix;
   friend class MultipliedMatrix;
   friend class SubtractedMatrix;
   friend class SPMatrix;
   friend class KPMatrix;
   friend class ConcatenatedMatrix;
   friend class StackedMatrix;
   friend class SolvedMatrix;
   friend class ShiftedMatrix;
   friend class NegShiftedMatrix;
   friend class ScaledMatrix;
   friend class TransposedMatrix;
   friend class ReversedMatrix;
   friend class NegatedMatrix;
   friend class InvertedMatrix;
   friend class RowedMatrix;
   friend class ColedMatrix;
   friend class DiagedMatrix;
   friend class MatedMatrix;
   friend class GetSubMatrix;
   friend class ReturnMatrix;
   friend class LinearEquationSolver;
   friend class GenericMatrix;
   NEW_DELETE(BaseMatrix)
};
class RSPF_DLL GeneralMatrix : public BaseMatrix         // declarable matrix types
{
   virtual GeneralMatrix* Image() const;        // copy of matrix
protected:
   int tag;                                     // shows whether can reuse
   int nrows, ncols;                            // dimensions
   int storage;                                 // total store required
   Real* store;                                 // point to store (0=not set)
   GeneralMatrix();                             // initialise with no store
   GeneralMatrix(ArrayLengthSpecifier);         // constructor getting store
   void Add(GeneralMatrix*, Real);              // sum of GM and Real
   void Add(Real);                              // add Real to this
   void NegAdd(GeneralMatrix*, Real);           // Real - GM
   void NegAdd(Real);                           // this = this - Real
   void Multiply(GeneralMatrix*, Real);         // product of GM and Real
   void Multiply(Real);                         // multiply this by Real
   void Negate(GeneralMatrix*);                 // change sign
   void Negate();                               // change sign
   void ReverseElements();                      // internal reverse of elements
   void ReverseElements(GeneralMatrix*);        // reverse order of elements
   void operator=(Real);                        // set matrix to constant
   Real* GetStore();                            // get store or copy
   GeneralMatrix* BorrowStore(GeneralMatrix*, MatrixType);
   void GetMatrix(const GeneralMatrix*);        // used by = and initialise
   void Eq(const BaseMatrix&, MatrixType);      // used by =
   void Eq(const GeneralMatrix&);               // version with no conversion
   void Eq(const BaseMatrix&, MatrixType, bool);// used by <<
   void Eq2(const BaseMatrix&, MatrixType);     // cut down version of Eq
   int search(const BaseMatrix*) const;
   virtual GeneralMatrix* Transpose(TransposedMatrix*, MatrixType);
   void CheckConversion(const BaseMatrix&);     // check conversion OK
   void ReSize(int, int, int);                  // change dimensions
   virtual short SimpleAddOK(const GeneralMatrix* /* gm */ ) { return 0; }
   virtual void MiniCleanUp() { store = 0; storage = 0; nrows = 0; ncols = 0; tag = -1;}
   void PlusEqual(const GeneralMatrix& gm);
   void MinusEqual(const GeneralMatrix& gm);
   void PlusEqual(Real f);
   void MinusEqual(Real f);
public:
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   virtual MatrixType Type() const = 0;         // type of a matrix
   int Nrows() const { return nrows; }          // get dimensions
   int Ncols() const { return ncols; }
   int Storage() const { return storage; }
   Real* Store() const { return store; }
   virtual ~GeneralMatrix();                    // delete store if set
   void tDelete();                              // delete if tag permits
   bool reuse();                                // true if tag allows reuse
   void Protect() { tag=-1; }                   // cannot delete or reuse
   int Tag() const { return tag; }
   bool IsZero() const;                         // test matrix has all zeros
   void Release() { tag=1; }                    // del store after next use
   void Release(int t) { tag=t; }               // del store after t accesses
   void ReleaseAndDelete() { tag=0; }           // delete matrix after use
   void operator<<(const Real*);                // assignment from an array
   void operator<<(const int*);                // assignment from an array
   void operator<<(const BaseMatrix& X)
      { Eq(X,this->Type(),true); }              // = without checking type
   void Inject(const GeneralMatrix&);           // copy stored els only
   void operator+=(const BaseMatrix&);
   void operator-=(const BaseMatrix&);
   void operator*=(const BaseMatrix&);
   void operator|=(const BaseMatrix&);
   void operator&=(const BaseMatrix&);
   void operator+=(Real);
   void operator-=(Real r) { operator+=(-r); }
   void operator*=(Real);
   void operator/=(Real r) { operator*=(1.0/r); }
   virtual GeneralMatrix* MakeSolver();         // for solving
   virtual void Solver(MatrixColX&, const MatrixColX&) {}
   virtual void GetRow(MatrixRowCol&) = 0;      // Get matrix row
   virtual void RestoreRow(MatrixRowCol&) {}    // Restore matrix row
   virtual void NextRow(MatrixRowCol&);         // Go to next row
   virtual void GetCol(MatrixRowCol&) = 0;      // Get matrix col
   virtual void GetCol(MatrixColX&) = 0;        // Get matrix col
   virtual void RestoreCol(MatrixRowCol&) {}    // Restore matrix col
   virtual void RestoreCol(MatrixColX&) {}      // Restore matrix col
   virtual void NextCol(MatrixRowCol&);         // Go to next col
   virtual void NextCol(MatrixColX&);           // Go to next col
   Real SumSquare() const;
   Real SumAbsoluteValue() const;
   Real Sum() const;
   Real MaximumAbsoluteValue1(int& i) const;
   Real MinimumAbsoluteValue1(int& i) const;
   Real Maximum1(int& i) const;
   Real Minimum1(int& i) const;
   Real MaximumAbsoluteValue() const;
   Real MaximumAbsoluteValue2(int& i, int& j) const;
   Real MinimumAbsoluteValue() const;
   Real MinimumAbsoluteValue2(int& i, int& j) const;
   Real Maximum() const;
   Real Maximum2(int& i, int& j) const;
   Real Minimum() const;
   Real Minimum2(int& i, int& j) const;
   LogAndSign LogDeterminant() const;
   virtual bool IsEqual(const GeneralMatrix&) const;
   void CheckStore() const;                     // check store is non-zero
   virtual void SetParameters(const GeneralMatrix*) {}
   operator ReturnMatrix() const;               // for building a ReturnMatrix
   ReturnMatrix ForReturn() const;
   virtual bool SameStorageType(const GeneralMatrix& A) const;
   virtual void ReSizeForAdd(const GeneralMatrix& A, const GeneralMatrix& B);
   virtual void ReSizeForSP(const GeneralMatrix& A, const GeneralMatrix& B);
   virtual void ReSize(const GeneralMatrix& A);
   MatrixInput operator<<(Real);                // for loading a list
   MatrixInput operator<<(int f);
   void CleanUp();                              // to clear store
   friend class Matrix;
   friend class SquareMatrix;
   friend class nricMatrix;
   friend class SymmetricMatrix;
   friend class UpperTriangularMatrix;
   friend class LowerTriangularMatrix;
   friend class DiagonalMatrix;
   friend class CroutMatrix;
   friend class RowVector;
   friend class ColumnVector;
   friend class BandMatrix;
   friend class LowerBandMatrix;
   friend class UpperBandMatrix;
   friend class SymmetricBandMatrix;
   friend class BaseMatrix;
   friend class AddedMatrix;
   friend class MultipliedMatrix;
   friend class SubtractedMatrix;
   friend class SPMatrix;
   friend class KPMatrix;
   friend class ConcatenatedMatrix;
   friend class StackedMatrix;
   friend class SolvedMatrix;
   friend class ShiftedMatrix;
   friend class NegShiftedMatrix;
   friend class ScaledMatrix;
   friend class TransposedMatrix;
   friend class ReversedMatrix;
   friend class NegatedMatrix;
   friend class InvertedMatrix;
   friend class RowedMatrix;
   friend class ColedMatrix;
   friend class DiagedMatrix;
   friend class MatedMatrix;
   friend class GetSubMatrix;
   friend class ReturnMatrix;
   friend class LinearEquationSolver;
   friend class GenericMatrix;
   NEW_DELETE(GeneralMatrix)
};
class RSPF_DLL Matrix : public GeneralMatrix             // usual rectangular matrix
{
   GeneralMatrix* Image() const;                // copy of matrix
public:
   Matrix():GeneralMatrix() {}
   ~Matrix() {}
   Matrix(int, int);                            // standard declaration
   Matrix(const BaseMatrix&);                   // evaluate BaseMatrix
   void operator=(const BaseMatrix&);
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   void operator=(const Matrix& m) { Eq(m); }
   MatrixType Type() const;
   Real& operator()(int, int);                  // access element
   Real& element(int, int);                     // access element
   Real operator()(int, int) const;            // access element
   Real element(int, int) const;               // access element
#ifdef SETUP_C_SUBSCRIPTS
   Real* operator[](int m) { return store+m*ncols; }
   const Real* operator[](int m) const { return store+m*ncols; }
#endif
   Matrix(const Matrix& gm):GeneralMatrix(gm) { GetMatrix(&gm); }
   GeneralMatrix* MakeSolver();
   Real Trace() const;
   void GetRow(MatrixRowCol&);
   void GetCol(MatrixRowCol&);
   void GetCol(MatrixColX&);
   void RestoreCol(MatrixRowCol&);
   void RestoreCol(MatrixColX&);
   void NextRow(MatrixRowCol&);
   void NextCol(MatrixRowCol&);
   void NextCol(MatrixColX&);
   virtual void ReSize(int,int);           // change dimensions
   void ReSize(const GeneralMatrix& A);
   Real MaximumAbsoluteValue2(int& i, int& j) const;
   Real MinimumAbsoluteValue2(int& i, int& j) const;
   Real Maximum2(int& i, int& j) const;
   Real Minimum2(int& i, int& j) const;
   void operator+=(const Matrix& M) { PlusEqual(M); }
   void operator-=(const Matrix& M) { MinusEqual(M); }
   void operator+=(Real f) { GeneralMatrix::Add(f); }
   void operator-=(Real f) { GeneralMatrix::Add(-f); }
   friend Real DotProduct(const Matrix& A, const Matrix& B);
   NEW_DELETE(Matrix)
};
class RSPF_DLL SquareMatrix : public Matrix              // square matrix
{
   GeneralMatrix* Image() const;                // copy of matrix
public:
   SquareMatrix() {}
   ~SquareMatrix() {}
   SquareMatrix(ArrayLengthSpecifier);          // standard declaration
   SquareMatrix(const BaseMatrix&);             // evaluate BaseMatrix
   void operator=(const BaseMatrix&);
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   void operator=(const SquareMatrix& m) { Eq(m); }
   void operator=(const Matrix& m);
   MatrixType Type() const;
   SquareMatrix(const SquareMatrix& gm):Matrix(gm) { GetMatrix(&gm); }
   SquareMatrix(const Matrix& gm);
   void ReSize(int);                            // change dimensions
   virtual void ReSize(int,int);                // change dimensions
   void ReSize(const GeneralMatrix& A);
   void operator+=(const Matrix& M) { PlusEqual(M); }
   void operator-=(const Matrix& M) { MinusEqual(M); }
   void operator+=(Real f) { GeneralMatrix::Add(f); }
   void operator-=(Real f) { GeneralMatrix::Add(-f); }
   NEW_DELETE(SquareMatrix)
};
class RSPF_DLL nricMatrix : public Matrix                // for use with Numerical
{
   GeneralMatrix* Image() const;                // copy of matrix
   Real** row_pointer;                          // points to rows
   void MakeRowPointer();                       // build rowpointer
   void DeleteRowPointer();
public:
   nricMatrix():Matrix() {}
   nricMatrix(int m, int n)                     // standard declaration
      :  Matrix(m,n) { MakeRowPointer(); }
   nricMatrix(const BaseMatrix& bm)             // evaluate BaseMatrix
      :  Matrix(bm) { MakeRowPointer(); }
   void operator=(const BaseMatrix& bm)
      { DeleteRowPointer(); Matrix::operator=(bm); MakeRowPointer(); }
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   void operator=(const nricMatrix& m)
      { DeleteRowPointer(); Eq(m); MakeRowPointer(); }
   void operator<<(const BaseMatrix& X)
      { DeleteRowPointer(); Eq(X,this->Type(),true); MakeRowPointer(); }
   nricMatrix(const nricMatrix& gm):Matrix(gm) { GetMatrix(&gm); MakeRowPointer(); }
   void ReSize(int m, int n)               // change dimensions
      { DeleteRowPointer(); Matrix::ReSize(m,n); MakeRowPointer(); }
   void ReSize(const GeneralMatrix& A);
   ~nricMatrix() { DeleteRowPointer(); }
   Real** nric() const { CheckStore(); return row_pointer-1; }
   void CleanUp();                                // to clear store
   void MiniCleanUp();
   void operator+=(const Matrix& M) { PlusEqual(M); }
   void operator-=(const Matrix& M) { MinusEqual(M); }
   void operator+=(Real f) { GeneralMatrix::Add(f); }
   void operator-=(Real f) { GeneralMatrix::Add(-f); }
   NEW_DELETE(nricMatrix)
};
class RSPF_DLL SymmetricMatrix : public GeneralMatrix
{
   GeneralMatrix* Image() const;                // copy of matrix
public:
   SymmetricMatrix():GeneralMatrix() {}
   ~SymmetricMatrix() {}
   SymmetricMatrix(ArrayLengthSpecifier);
   SymmetricMatrix(const BaseMatrix&);
   void operator=(const BaseMatrix&);
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   void operator=(const SymmetricMatrix& m) { Eq(m); }
   Real& operator()(int, int);                  // access element
   Real& element(int, int);                     // access element
   Real operator()(int, int) const;             // access element
   Real element(int, int) const;                // access element
#ifdef SETUP_C_SUBSCRIPTS
   Real* operator[](int m) { return store+(m*(m+1))/2; }
   const Real* operator[](int m) const { return store+(m*(m+1))/2; }
#endif
   MatrixType Type() const;
   SymmetricMatrix(const SymmetricMatrix& gm):GeneralMatrix(gm) { GetMatrix(&gm); }
   Real SumSquare() const;
   Real SumAbsoluteValue() const;
   Real Sum() const;
   Real Trace() const;
   void GetRow(MatrixRowCol&);
   void GetCol(MatrixRowCol&);
   void GetCol(MatrixColX&);
   void RestoreCol(MatrixRowCol&) {}
   void RestoreCol(MatrixColX&);
	GeneralMatrix* Transpose(TransposedMatrix*, MatrixType);
   void ReSize(int);                       // change dimensions
   void ReSize(const GeneralMatrix& A);
   void operator+=(const SymmetricMatrix& M) { PlusEqual(M); }
   void operator-=(const SymmetricMatrix& M) { MinusEqual(M); }
   void operator+=(Real f) { GeneralMatrix::Add(f); }
   void operator-=(Real f) { GeneralMatrix::Add(-f); }
   NEW_DELETE(SymmetricMatrix)
};
class RSPF_DLL UpperTriangularMatrix : public GeneralMatrix
{
   GeneralMatrix* Image() const;                // copy of matrix
public:
   UpperTriangularMatrix():GeneralMatrix() {}
   ~UpperTriangularMatrix() {}
   UpperTriangularMatrix(ArrayLengthSpecifier);
   void operator=(const BaseMatrix&);
   void operator=(const UpperTriangularMatrix& m) { Eq(m); }
   UpperTriangularMatrix(const BaseMatrix&);
   UpperTriangularMatrix(const UpperTriangularMatrix& gm):GeneralMatrix(gm) { GetMatrix(&gm); }
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   Real& operator()(int, int);                  // access element
   Real& element(int, int);                     // access element
   Real operator()(int, int) const;             // access element
   Real element(int, int) const;                // access element
#ifdef SETUP_C_SUBSCRIPTS
   Real* operator[](int m) { return store+m*ncols-(m*(m+1))/2; }
   const Real* operator[](int m) const { return store+m*ncols-(m*(m+1))/2; }
#endif
   MatrixType Type() const;
   GeneralMatrix* MakeSolver() { return this; } // for solving
   void Solver(MatrixColX&, const MatrixColX&);
   LogAndSign LogDeterminant() const;
   Real Trace() const;
   void GetRow(MatrixRowCol&);
   void GetCol(MatrixRowCol&);
   void GetCol(MatrixColX&);
   void RestoreCol(MatrixRowCol&);
   void RestoreCol(MatrixColX& c) { RestoreCol((MatrixRowCol&)c); }
   void NextRow(MatrixRowCol&);
   void ReSize(int);                       // change dimensions
   void ReSize(const GeneralMatrix& A);
   MatrixBandWidth BandWidth() const;
   void operator+=(const UpperTriangularMatrix& M) { PlusEqual(M); }
   void operator-=(const UpperTriangularMatrix& M) { MinusEqual(M); }
   void operator+=(Real f) { GeneralMatrix::operator+=(f); }
   void operator-=(Real f) { GeneralMatrix::operator-=(f); }
   NEW_DELETE(UpperTriangularMatrix)
};
class RSPF_DLL LowerTriangularMatrix : public GeneralMatrix
{
   GeneralMatrix* Image() const;                // copy of matrix
public:
   LowerTriangularMatrix():GeneralMatrix() {}
   ~LowerTriangularMatrix() {}
   LowerTriangularMatrix(ArrayLengthSpecifier);
   LowerTriangularMatrix(const LowerTriangularMatrix& gm):GeneralMatrix(gm) { GetMatrix(&gm); }
   LowerTriangularMatrix(const BaseMatrix& M);
   void operator=(const BaseMatrix&);
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   void operator=(const LowerTriangularMatrix& m) { Eq(m); }
   Real& operator()(int, int);                  // access element
   Real& element(int, int);                     // access element
   Real operator()(int, int) const;             // access element
   Real element(int, int) const;                // access element
#ifdef SETUP_C_SUBSCRIPTS
   Real* operator[](int m) { return store+(m*(m+1))/2; }
   const Real* operator[](int m) const { return store+(m*(m+1))/2; }
#endif
   MatrixType Type() const;
   GeneralMatrix* MakeSolver() { return this; } // for solving
   void Solver(MatrixColX&, const MatrixColX&);
   LogAndSign LogDeterminant() const;
   Real Trace() const;
   void GetRow(MatrixRowCol&);
   void GetCol(MatrixRowCol&);
   void GetCol(MatrixColX&);
   void RestoreCol(MatrixRowCol&);
   void RestoreCol(MatrixColX& c) { RestoreCol((MatrixRowCol&)c); }
   void NextRow(MatrixRowCol&);
   void ReSize(int);                       // change dimensions
   void ReSize(const GeneralMatrix& A);
   MatrixBandWidth BandWidth() const;
   void operator+=(const LowerTriangularMatrix& M) { PlusEqual(M); }
   void operator-=(const LowerTriangularMatrix& M) { MinusEqual(M); }
   void operator+=(Real f) { GeneralMatrix::operator+=(f); }
   void operator-=(Real f) { GeneralMatrix::operator-=(f); }
   NEW_DELETE(LowerTriangularMatrix)
};
class RSPF_DLL DiagonalMatrix : public GeneralMatrix
{
   GeneralMatrix* Image() const;                // copy of matrix
public:
   DiagonalMatrix():GeneralMatrix() {}
   ~DiagonalMatrix() {}
   DiagonalMatrix(ArrayLengthSpecifier);
   DiagonalMatrix(const BaseMatrix&);
   DiagonalMatrix(const DiagonalMatrix& gm):GeneralMatrix(gm) { GetMatrix(&gm); }
   void operator=(const BaseMatrix&);
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   void operator=(const DiagonalMatrix& m) { Eq(m); }
   Real& operator()(int, int);                  // access element
   Real& operator()(int);                       // access element
   Real operator()(int, int) const;             // access element
   Real operator()(int) const;
   Real& element(int, int);                     // access element
   Real& element(int);                          // access element
   Real element(int, int) const;                // access element
   Real element(int) const;                     // access element
#ifdef SETUP_C_SUBSCRIPTS
   Real& operator[](int m) { return store[m]; }
   const Real& operator[](int m) const { return store[m]; }
#endif
   MatrixType Type() const;
   LogAndSign LogDeterminant() const;
   Real Trace() const;
   void GetRow(MatrixRowCol&);
   void GetCol(MatrixRowCol&);
   void GetCol(MatrixColX&);
   void NextRow(MatrixRowCol&);
   void NextCol(MatrixRowCol&);
   void NextCol(MatrixColX&);
   GeneralMatrix* MakeSolver() { return this; } // for solving
   void Solver(MatrixColX&, const MatrixColX&);
   GeneralMatrix* Transpose(TransposedMatrix*, MatrixType);
   void ReSize(int);                       // change dimensions
   void ReSize(const GeneralMatrix& A);
   Real* nric() const
      { CheckStore(); return store-1; }         // for use by NRIC
   MatrixBandWidth BandWidth() const;
   void operator+=(const DiagonalMatrix& M) { PlusEqual(M); }
   void operator-=(const DiagonalMatrix& M) { MinusEqual(M); }
   void operator+=(Real f) { GeneralMatrix::operator+=(f); }
   void operator-=(Real f) { GeneralMatrix::operator-=(f); }
   NEW_DELETE(DiagonalMatrix)
};
class RSPF_DLL RowVector : public Matrix
{
   GeneralMatrix* Image() const;                // copy of matrix
public:
   RowVector():Matrix() { nrows = 1; }
   ~RowVector() {}
   RowVector(ArrayLengthSpecifier n) : Matrix(1,n.Value()) {}
   RowVector(const BaseMatrix&);
   RowVector(const RowVector& gm):Matrix(gm) { GetMatrix(&gm); }
   void operator=(const BaseMatrix&);
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   void operator=(const RowVector& m) { Eq(m); }
   Real& operator()(int);                       // access element
   Real& element(int);                          // access element
   Real operator()(int) const;                  // access element
   Real element(int) const;                     // access element
#ifdef SETUP_C_SUBSCRIPTS
   Real& operator[](int m) { return store[m]; }
   const Real& operator[](int m) const { return store[m]; }
#endif
   MatrixType Type() const;
   void GetCol(MatrixRowCol&);
   void GetCol(MatrixColX&);
   void NextCol(MatrixRowCol&);
   void NextCol(MatrixColX&);
   void RestoreCol(MatrixRowCol&) {}
   void RestoreCol(MatrixColX& c);
   GeneralMatrix* Transpose(TransposedMatrix*, MatrixType);
   void ReSize(int);                       // change dimensions
   void ReSize(int,int);                   // in case access is matrix
   void ReSize(const GeneralMatrix& A);
   Real* nric() const
      { CheckStore(); return store-1; }         // for use by NRIC
   void CleanUp();                              // to clear store
   void MiniCleanUp() { store = 0; storage = 0; nrows = 1; ncols = 0; tag = -1; }
   void operator+=(const Matrix& M) { PlusEqual(M); }
   void operator-=(const Matrix& M) { MinusEqual(M); }
   void operator+=(Real f) { GeneralMatrix::Add(f); }
   void operator-=(Real f) { GeneralMatrix::Add(-f); }
   NEW_DELETE(RowVector)
};
class RSPF_DLL ColumnVector : public Matrix
{
   GeneralMatrix* Image() const;                // copy of matrix
public:
   ColumnVector():Matrix() { ncols = 1; }
   ~ColumnVector() {}
   ColumnVector(ArrayLengthSpecifier n) : Matrix(n.Value(),1) {}
   ColumnVector(const BaseMatrix&);
   ColumnVector(const ColumnVector& gm):Matrix(gm) { GetMatrix(&gm); }
   void operator=(const BaseMatrix&);
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   void operator=(const ColumnVector& m) { Eq(m); }
   Real& operator()(int);                       // access element
   Real& element(int);                          // access element
   Real operator()(int) const;                  // access element
   Real element(int) const;                     // access element
#ifdef SETUP_C_SUBSCRIPTS
   Real& operator[](int m) { return store[m]; }
   const Real& operator[](int m) const { return store[m]; }
#endif
   MatrixType Type() const;
   GeneralMatrix* Transpose(TransposedMatrix*, MatrixType);
   void ReSize(int);                       // change dimensions
   void ReSize(int,int);                   // in case access is matrix
   void ReSize(const GeneralMatrix& A);
   Real* nric() const
      { CheckStore(); return store-1; }         // for use by NRIC
   void CleanUp();                              // to clear store
   void MiniCleanUp() { store = 0; storage = 0; nrows = 0; ncols = 1; tag = -1; }
   void operator+=(const Matrix& M) { PlusEqual(M); }
   void operator-=(const Matrix& M) { MinusEqual(M); }
   void operator+=(Real f) { GeneralMatrix::Add(f); }
   void operator-=(Real f) { GeneralMatrix::Add(-f); }
   NEW_DELETE(ColumnVector)
};
class RSPF_DLL CroutMatrix : public GeneralMatrix        // for LU decomposition
{
   int* indx;
   bool d;
   bool sing;
   void ludcmp();
   void operator=(const CroutMatrix& /* m */) {}     // not allowed
public:
   CroutMatrix(const BaseMatrix&);
   MatrixType Type() const;
   void lubksb(Real*, int=0);
   ~CroutMatrix();
   GeneralMatrix* MakeSolver() { return this; } // for solving
   LogAndSign LogDeterminant() const;
   void Solver(MatrixColX&, const MatrixColX&);
   void GetRow(MatrixRowCol&);
   void GetCol(MatrixRowCol&);
   void GetCol(MatrixColX& c) { GetCol((MatrixRowCol&)c); }
   void CleanUp();                                // to clear store
   void MiniCleanUp();
   bool IsEqual(const GeneralMatrix&) const;
   bool IsSingular() const { return sing; }
   NEW_DELETE(CroutMatrix)
};
class RSPF_DLL BandMatrix : public GeneralMatrix         // band matrix
{
   GeneralMatrix* Image() const;                // copy of matrix
protected:
   void CornerClear() const;                    // set unused elements to zero
   short SimpleAddOK(const GeneralMatrix* gm);
public:
   int lower, upper;                            // band widths
   BandMatrix():GeneralMatrix() { lower=0; upper=0; CornerClear(); }
   ~BandMatrix() {}
   BandMatrix(int n,int lb,int ub) { ReSize(n,lb,ub); CornerClear(); }
   BandMatrix(const BaseMatrix&);               // evaluate BaseMatrix
   void operator=(const BaseMatrix&);
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   void operator=(const BandMatrix& m) { Eq(m); }
   MatrixType Type() const;
   Real& operator()(int, int);                  // access element
   Real& element(int, int);                     // access element
   Real operator()(int, int) const;             // access element
   Real element(int, int) const;                // access element
#ifdef SETUP_C_SUBSCRIPTS
   Real* operator[](int m) { return store+(upper+lower)*m+lower; }
   const Real* operator[](int m) const { return store+(upper+lower)*m+lower; }
#endif
   BandMatrix(const BandMatrix& gm):GeneralMatrix(gm) { GetMatrix(&gm); }
   LogAndSign LogDeterminant() const;
   GeneralMatrix* MakeSolver();
   Real Trace() const;
   Real SumSquare() const { CornerClear(); return GeneralMatrix::SumSquare(); }
   Real SumAbsoluteValue() const
      { CornerClear(); return GeneralMatrix::SumAbsoluteValue(); }
   Real Sum() const
      { CornerClear(); return GeneralMatrix::Sum(); }
   Real MaximumAbsoluteValue() const
      { CornerClear(); return GeneralMatrix::MaximumAbsoluteValue(); }
   Real MinimumAbsoluteValue() const
      { int i, j; return GeneralMatrix::MinimumAbsoluteValue2(i, j); }
   Real Maximum() const { int i, j; return GeneralMatrix::Maximum2(i, j); }
   Real Minimum() const { int i, j; return GeneralMatrix::Minimum2(i, j); }
   void GetRow(MatrixRowCol&);
   void GetCol(MatrixRowCol&);
   void GetCol(MatrixColX&);
   void RestoreCol(MatrixRowCol&);
   void RestoreCol(MatrixColX& c) { RestoreCol((MatrixRowCol&)c); }
   void NextRow(MatrixRowCol&);
   virtual void ReSize(int, int, int);             // change dimensions
   void ReSize(const GeneralMatrix& A);
   bool SameStorageType(const GeneralMatrix& A) const;
   void ReSizeForAdd(const GeneralMatrix& A, const GeneralMatrix& B);
   void ReSizeForSP(const GeneralMatrix& A, const GeneralMatrix& B);
   MatrixBandWidth BandWidth() const;
   void SetParameters(const GeneralMatrix*);
   MatrixInput operator<<(Real);                // will give error
   MatrixInput operator<<(int f);
   void operator<<(const Real* r);              // will give error
   void operator<<(const int* r);               // will give error
   void operator<<(const BaseMatrix& X) { GeneralMatrix::operator<<(X); }
   NEW_DELETE(BandMatrix)
};
class RSPF_DLL UpperBandMatrix : public BandMatrix       // upper band matrix
{
   GeneralMatrix* Image() const;                // copy of matrix
public:
   UpperBandMatrix():BandMatrix() {}
   ~UpperBandMatrix() {}
   UpperBandMatrix(int n, int ubw)              // standard declaration
      : BandMatrix(n, 0, ubw) {}
   UpperBandMatrix(const BaseMatrix&);          // evaluate BaseMatrix
   void operator=(const BaseMatrix&);
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   void operator=(const UpperBandMatrix& m) { Eq(m); }
   MatrixType Type() const;
   UpperBandMatrix(const UpperBandMatrix& gm):BandMatrix(gm) { GetMatrix(&gm); }
   GeneralMatrix* MakeSolver() { return this; }
   void Solver(MatrixColX&, const MatrixColX&);
   LogAndSign LogDeterminant() const;
   void ReSize(int, int, int);             // change dimensions
   void ReSize(int n,int ubw)              // change dimensions
      { BandMatrix::ReSize(n,0,ubw); }
   void ReSize(const GeneralMatrix& A) { BandMatrix::ReSize(A); }
   Real& operator()(int, int);
   Real operator()(int, int) const;
   Real& element(int, int);
   Real element(int, int) const;
#ifdef SETUP_C_SUBSCRIPTS
   Real* operator[](int m) { return store+upper*m; }
   const Real* operator[](int m) const { return store+upper*m; }
#endif
   NEW_DELETE(UpperBandMatrix)
};
class RSPF_DLL LowerBandMatrix : public BandMatrix       // upper band matrix
{
   GeneralMatrix* Image() const;                // copy of matrix
public:
   LowerBandMatrix():BandMatrix() {}
   ~LowerBandMatrix() {}
   LowerBandMatrix(int n, int lbw)              // standard declaration
      : BandMatrix(n, lbw, 0) {}
   LowerBandMatrix(const BaseMatrix&);          // evaluate BaseMatrix
   void operator=(const BaseMatrix&);
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   void operator=(const LowerBandMatrix& m) { Eq(m); }
   MatrixType Type() const;
   LowerBandMatrix(const LowerBandMatrix& gm):BandMatrix(gm) { GetMatrix(&gm); }
   GeneralMatrix* MakeSolver() { return this; }
   void Solver(MatrixColX&, const MatrixColX&);
   LogAndSign LogDeterminant() const;
   void ReSize(int, int, int);             // change dimensions
   void ReSize(int n,int lbw)             // change dimensions
      { BandMatrix::ReSize(n,lbw,0); }
   void ReSize(const GeneralMatrix& A) { BandMatrix::ReSize(A); }
   Real& operator()(int, int);
   Real operator()(int, int) const;
   Real& element(int, int);
   Real element(int, int) const;
#ifdef SETUP_C_SUBSCRIPTS
   Real* operator[](int m) { return store+lower*(m+1); }
   const Real* operator[](int m) const { return store+lower*(m+1); }
#endif
   NEW_DELETE(LowerBandMatrix)
};
class RSPF_DLL SymmetricBandMatrix : public GeneralMatrix
{
   GeneralMatrix* Image() const;                // copy of matrix
   void CornerClear() const;                    // set unused elements to zero
   short SimpleAddOK(const GeneralMatrix* gm);
public:
   int lower;                                   // lower band width
   SymmetricBandMatrix():GeneralMatrix() { lower=0; CornerClear(); }
   ~SymmetricBandMatrix() {}
   SymmetricBandMatrix(int n, int lb) { ReSize(n,lb); CornerClear(); }
   SymmetricBandMatrix(const BaseMatrix&);
   void operator=(const BaseMatrix&);
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   void operator=(const SymmetricBandMatrix& m) { Eq(m); }
   Real& operator()(int, int);                  // access element
   Real& element(int, int);                     // access element
   Real operator()(int, int) const;             // access element
   Real element(int, int) const;                // access element
#ifdef SETUP_C_SUBSCRIPTS
   Real* operator[](int m) { return store+lower*(m+1); }
   const Real* operator[](int m) const { return store+lower*(m+1); }
#endif
   MatrixType Type() const;
   SymmetricBandMatrix(const SymmetricBandMatrix& gm):GeneralMatrix(gm) { GetMatrix(&gm); }
   GeneralMatrix* MakeSolver();
   Real SumSquare() const;
   Real SumAbsoluteValue() const;
   Real Sum() const;
   Real MaximumAbsoluteValue() const
      { CornerClear(); return GeneralMatrix::MaximumAbsoluteValue(); }
   Real MinimumAbsoluteValue() const
      { int i, j; return GeneralMatrix::MinimumAbsoluteValue2(i, j); }
   Real Maximum() const { int i, j; return GeneralMatrix::Maximum2(i, j); }
   Real Minimum() const { int i, j; return GeneralMatrix::Minimum2(i, j); }
   Real Trace() const;
   LogAndSign LogDeterminant() const;
   void GetRow(MatrixRowCol&);
   void GetCol(MatrixRowCol&);
   void GetCol(MatrixColX&);
   void RestoreCol(MatrixRowCol&) {}
   void RestoreCol(MatrixColX&);
   GeneralMatrix* Transpose(TransposedMatrix*, MatrixType);
   void ReSize(int,int);                       // change dimensions
   void ReSize(const GeneralMatrix& A);
   bool SameStorageType(const GeneralMatrix& A) const;
   void ReSizeForAdd(const GeneralMatrix& A, const GeneralMatrix& B);
   void ReSizeForSP(const GeneralMatrix& A, const GeneralMatrix& B);
   MatrixBandWidth BandWidth() const;
   void SetParameters(const GeneralMatrix*);
   void operator<<(const Real* r);              // will give error
   void operator<<(const int* r);               // will give error
   void operator<<(const BaseMatrix& X) { GeneralMatrix::operator<<(X); }
   NEW_DELETE(SymmetricBandMatrix)
};
class RSPF_DLL BandLUMatrix : public GeneralMatrix
{
   int* indx;
   bool d;
   bool sing;                                   // true if singular
   Real* store2;
   int storage2;
   void ludcmp();
   int m1,m2;                                   // lower and upper
   void operator=(const BandLUMatrix& /* m */) {}     // no allowed
public:
   BandLUMatrix(const BaseMatrix&);
   MatrixType Type() const;
   void lubksb(Real*, int=0);
   ~BandLUMatrix();
   GeneralMatrix* MakeSolver() { return this; } // for solving
   LogAndSign LogDeterminant() const;
   void Solver(MatrixColX&, const MatrixColX&);
   void GetRow(MatrixRowCol&);
   void GetCol(MatrixRowCol&);
   void GetCol(MatrixColX& c) { GetCol((MatrixRowCol&)c); }
   void CleanUp();                                // to clear store
   void MiniCleanUp();
   bool IsEqual(const GeneralMatrix&) const;
   bool IsSingular() const { return sing; }
   NEW_DELETE(BandLUMatrix)
};
class RSPF_DLL IdentityMatrix : public GeneralMatrix
{
   GeneralMatrix* Image() const;          // copy of matrix
public:
   IdentityMatrix():GeneralMatrix() {}
   ~IdentityMatrix() {}
   IdentityMatrix(ArrayLengthSpecifier n) : GeneralMatrix(1)
      { nrows = ncols = n.Value(); *store = 1; }
   IdentityMatrix(const IdentityMatrix& gm):GeneralMatrix(gm) { GetMatrix(&gm); }
   IdentityMatrix(const BaseMatrix&);
   void operator=(const BaseMatrix&);
   void operator=(const IdentityMatrix& m) { Eq(m); }
   void operator=(Real f) { GeneralMatrix::operator=(f); }
   MatrixType Type() const;
   LogAndSign LogDeterminant() const;
   Real Trace() const;
   Real SumSquare() const;
   Real SumAbsoluteValue() const;
   Real Sum() const { return Trace(); }
   void GetRow(MatrixRowCol&);
   void GetCol(MatrixRowCol&);
   void GetCol(MatrixColX&);
   void NextRow(MatrixRowCol&);
   void NextCol(MatrixRowCol&);
   void NextCol(MatrixColX&);
   GeneralMatrix* MakeSolver() { return this; } // for solving
   void Solver(MatrixColX&, const MatrixColX&);
   GeneralMatrix* Transpose(TransposedMatrix*, MatrixType);
   void ReSize(int n);
   void ReSize(const GeneralMatrix& A);
   MatrixBandWidth BandWidth() const;
   NEW_DELETE(IdentityMatrix)
};
class RSPF_DLL GenericMatrix : public BaseMatrix
{
   GeneralMatrix* gm;
   int search(const BaseMatrix* bm) const;
   friend class BaseMatrix;
public:
   GenericMatrix() : gm(0) {}
   GenericMatrix(const BaseMatrix& bm)
      { gm = ((BaseMatrix&)bm).Evaluate(); gm = gm->Image(); }
   GenericMatrix(const GenericMatrix& bm) : BaseMatrix(bm)
      { gm = bm.gm->Image(); }
   void operator=(const GenericMatrix&);
   void operator=(const BaseMatrix&);
   void operator+=(const BaseMatrix&);
   void operator-=(const BaseMatrix&);
   void operator*=(const BaseMatrix&);
   void operator|=(const BaseMatrix&);
   void operator&=(const BaseMatrix&);
   void operator+=(Real);
   void operator-=(Real r) { operator+=(-r); }
   void operator*=(Real);
   void operator/=(Real r) { operator*=(1.0/r); }
   ~GenericMatrix() { delete gm; }
   void CleanUp() { delete gm; gm = 0; }
   void Release() { gm->Release(); }
   GeneralMatrix* Evaluate(MatrixType = MatrixTypeUnSp);
   MatrixBandWidth BandWidth() const;
   NEW_DELETE(GenericMatrix)
};
class RSPF_DLL MultipliedMatrix : public BaseMatrix
{
protected:
   union { const BaseMatrix* bm1; GeneralMatrix* gm1; };
						  // pointers to summands
   union { const BaseMatrix* bm2; GeneralMatrix* gm2; };
   MultipliedMatrix(const BaseMatrix* bm1x, const BaseMatrix* bm2x)
      : bm1(bm1x),bm2(bm2x) {}
   int search(const BaseMatrix*) const;
   friend class BaseMatrix;
   friend class GeneralMatrix;
   friend class GenericMatrix;
public:
   ~MultipliedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixBandWidth BandWidth() const;
   NEW_DELETE(MultipliedMatrix)
};
class RSPF_DLL AddedMatrix : public MultipliedMatrix
{
protected:
   AddedMatrix(const BaseMatrix* bm1x, const BaseMatrix* bm2x)
      : MultipliedMatrix(bm1x,bm2x) {}
   friend class BaseMatrix;
   friend class GeneralMatrix;
   friend class GenericMatrix;
public:
   ~AddedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixBandWidth BandWidth() const;
   NEW_DELETE(AddedMatrix)
};
class RSPF_DLL SPMatrix : public AddedMatrix
{
protected:
   SPMatrix(const BaseMatrix* bm1x, const BaseMatrix* bm2x)
      : AddedMatrix(bm1x,bm2x) {}
   friend class BaseMatrix;
   friend class GeneralMatrix;
   friend class GenericMatrix;
public:
   ~SPMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixBandWidth BandWidth() const;
   friend SPMatrix SP(const BaseMatrix&, const BaseMatrix&);
   NEW_DELETE(SPMatrix)
};
class RSPF_DLL KPMatrix : public MultipliedMatrix
{
protected:
   KPMatrix(const BaseMatrix* bm1x, const BaseMatrix* bm2x)
      : MultipliedMatrix(bm1x,bm2x) {}
   friend class BaseMatrix;
   friend class GeneralMatrix;
   friend class GenericMatrix;
public:
   ~KPMatrix() {}
   MatrixBandWidth BandWidth() const;
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   friend KPMatrix KP(const BaseMatrix&, const BaseMatrix&);
   NEW_DELETE(KPMatrix)
};
class RSPF_DLL ConcatenatedMatrix : public MultipliedMatrix
{
protected:
   ConcatenatedMatrix(const BaseMatrix* bm1x, const BaseMatrix* bm2x)
      : MultipliedMatrix(bm1x,bm2x) {}
   friend class BaseMatrix;
   friend class GeneralMatrix;
   friend class GenericMatrix;
public:
   ~ConcatenatedMatrix() {}
   MatrixBandWidth BandWidth() const;
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   NEW_DELETE(ConcatenatedMatrix)
};
class RSPF_DLL StackedMatrix : public ConcatenatedMatrix
{
protected:
   StackedMatrix(const BaseMatrix* bm1x, const BaseMatrix* bm2x)
      : ConcatenatedMatrix(bm1x,bm2x) {}
   friend class BaseMatrix;
   friend class GeneralMatrix;
   friend class GenericMatrix;
public:
   ~StackedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   NEW_DELETE(StackedMatrix)
};
class RSPF_DLL SolvedMatrix : public MultipliedMatrix
{
   SolvedMatrix(const BaseMatrix* bm1x, const BaseMatrix* bm2x)
      : MultipliedMatrix(bm1x,bm2x) {}
   friend class BaseMatrix;
   friend class InvertedMatrix;                        // for operator*
public:
   ~SolvedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixBandWidth BandWidth() const;
   NEW_DELETE(SolvedMatrix)
};
class RSPF_DLL SubtractedMatrix : public AddedMatrix
{
   SubtractedMatrix(const BaseMatrix* bm1x, const BaseMatrix* bm2x)
      : AddedMatrix(bm1x,bm2x) {}
   friend class BaseMatrix;
   friend class GeneralMatrix;
   friend class GenericMatrix;
public:
   ~SubtractedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   NEW_DELETE(SubtractedMatrix)
};
class RSPF_DLL ShiftedMatrix : public BaseMatrix
{
protected:
   union { const BaseMatrix* bm; GeneralMatrix* gm; };
   Real f;
   ShiftedMatrix(const BaseMatrix* bmx, Real fx) : bm(bmx),f(fx) {}
   int search(const BaseMatrix*) const;
   friend class BaseMatrix;
   friend class GeneralMatrix;
   friend class GenericMatrix;
public:
   ~ShiftedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   friend ShiftedMatrix operator+(Real f, const BaseMatrix& BM);
   NEW_DELETE(ShiftedMatrix)
};
class RSPF_DLL NegShiftedMatrix : public ShiftedMatrix
{
protected:
   NegShiftedMatrix(Real fx, const BaseMatrix* bmx) : ShiftedMatrix(bmx,fx) {}
   friend class BaseMatrix;
   friend class GeneralMatrix;
   friend class GenericMatrix;
public:
   ~NegShiftedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   friend NegShiftedMatrix operator-(Real, const BaseMatrix&);
   NEW_DELETE(NegShiftedMatrix)
};
class RSPF_DLL ScaledMatrix : public ShiftedMatrix
{
   ScaledMatrix(const BaseMatrix* bmx, Real fx) : ShiftedMatrix(bmx,fx) {}
   friend class BaseMatrix;
   friend class GeneralMatrix;
   friend class GenericMatrix;
public:
   ~ScaledMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixBandWidth BandWidth() const;
   friend ScaledMatrix operator*(Real f, const BaseMatrix& BM);
   NEW_DELETE(ScaledMatrix)
};
class RSPF_DLL NegatedMatrix : public BaseMatrix
{
protected:
   union { const BaseMatrix* bm; GeneralMatrix* gm; };
   NegatedMatrix(const BaseMatrix* bmx) : bm(bmx) {}
   int search(const BaseMatrix*) const;
private:
   friend class BaseMatrix;
public:
   ~NegatedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixBandWidth BandWidth() const;
   NEW_DELETE(NegatedMatrix)
};
class RSPF_DLL TransposedMatrix : public NegatedMatrix
{
   TransposedMatrix(const BaseMatrix* bmx) : NegatedMatrix(bmx) {}
   friend class BaseMatrix;
public:
   ~TransposedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixBandWidth BandWidth() const;
   NEW_DELETE(TransposedMatrix)
};
class RSPF_DLL ReversedMatrix : public NegatedMatrix
{
   ReversedMatrix(const BaseMatrix* bmx) : NegatedMatrix(bmx) {}
   friend class BaseMatrix;
public:
   ~ReversedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   NEW_DELETE(ReversedMatrix)
};
class RSPF_DLL InvertedMatrix : public NegatedMatrix
{
   InvertedMatrix(const BaseMatrix* bmx) : NegatedMatrix(bmx) {}
public:
   ~InvertedMatrix() {}
   SolvedMatrix operator*(const BaseMatrix&) const;       // inverse(A) * B
   ScaledMatrix operator*(Real t) const { return BaseMatrix::operator*(t); }
   friend class BaseMatrix;
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixBandWidth BandWidth() const;
   NEW_DELETE(InvertedMatrix)
};
class RSPF_DLL RowedMatrix : public NegatedMatrix
{
   RowedMatrix(const BaseMatrix* bmx) : NegatedMatrix(bmx) {}
   friend class BaseMatrix;
public:
   ~RowedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixBandWidth BandWidth() const;
   NEW_DELETE(RowedMatrix)
};
class RSPF_DLL ColedMatrix : public NegatedMatrix
{
   ColedMatrix(const BaseMatrix* bmx) : NegatedMatrix(bmx) {}
   friend class BaseMatrix;
public:
   ~ColedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixBandWidth BandWidth() const;
   NEW_DELETE(ColedMatrix)
};
class RSPF_DLL DiagedMatrix : public NegatedMatrix
{
   DiagedMatrix(const BaseMatrix* bmx) : NegatedMatrix(bmx) {}
   friend class BaseMatrix;
public:
   ~DiagedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixBandWidth BandWidth() const;
   NEW_DELETE(DiagedMatrix)
};
class RSPF_DLL MatedMatrix : public NegatedMatrix
{
   int nr, nc;
   MatedMatrix(const BaseMatrix* bmx, int nrx, int ncx)
      : NegatedMatrix(bmx), nr(nrx), nc(ncx) {}
   friend class BaseMatrix;
public:
   ~MatedMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   MatrixBandWidth BandWidth() const;
   NEW_DELETE(MatedMatrix)
};
class RSPF_DLL ReturnMatrix : public BaseMatrix    // for matrix return
{
   GeneralMatrix* gm;
   int search(const BaseMatrix*) const;
public:
   ~ReturnMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   friend class BaseMatrix;
   ReturnMatrix(const ReturnMatrix& tm) :BaseMatrix(tm), gm(tm.gm) {}
   ReturnMatrix(const GeneralMatrix* gmx) : gm((GeneralMatrix*&)gmx) {}
   MatrixBandWidth BandWidth() const;
   NEW_DELETE(ReturnMatrix)
};
class RSPF_DLL GetSubMatrix : public NegatedMatrix
{
   int row_skip;
   int row_number;
   int col_skip;
   int col_number;
   bool IsSym;
   GetSubMatrix
      (const BaseMatrix* bmx, int rs, int rn, int cs, int cn, bool is)
      : NegatedMatrix(bmx),
      row_skip(rs), row_number(rn), col_skip(cs), col_number(cn), IsSym(is) {}
   void SetUpLHS();
   friend class BaseMatrix;
public:
   GetSubMatrix(const GetSubMatrix& g)
      : NegatedMatrix(g.bm), row_skip(g.row_skip), row_number(g.row_number),
      col_skip(g.col_skip), col_number(g.col_number), IsSym(g.IsSym) {}
   ~GetSubMatrix() {}
   GeneralMatrix* Evaluate(MatrixType mt=MatrixTypeUnSp);
   void operator=(const BaseMatrix&);
   void operator+=(const BaseMatrix&);
   void operator-=(const BaseMatrix&);
   void operator=(const GetSubMatrix& m) { operator=((const BaseMatrix&)m); }
   void operator<<(const BaseMatrix&);
   void operator<<(const Real*);                // copy from array
   void operator<<(const int*);                 // copy from array
   MatrixInput operator<<(Real);                // for loading a list
   MatrixInput operator<<(int f);
   void operator=(Real);                        // copy from constant
   void operator+=(Real);                       // add constant
   void operator-=(Real r) { operator+=(-r); }  // subtract constant
   void operator*=(Real);                       // multiply by constant
   void operator/=(Real r) { operator*=(1.0/r); } // divide by constant
   void Inject(const GeneralMatrix&);           // copy stored els only
   MatrixBandWidth BandWidth() const;
   NEW_DELETE(GetSubMatrix)
};
class RSPF_DLL LinearEquationSolver : public BaseMatrix
{
   GeneralMatrix* gm;
   int search(const BaseMatrix*) const { return 0; }
   friend class BaseMatrix;
public:
   LinearEquationSolver(const BaseMatrix& bm);
   ~LinearEquationSolver() { delete gm; }
   void CleanUp() { delete gm; } 
   GeneralMatrix* Evaluate(MatrixType) { return gm; }
   NEW_DELETE(LinearEquationSolver)
};
class RSPF_DLL MatrixInput // for reading a list of values into a matrix
{
   int n;                  // number values still to be read
   Real* r;                // pointer to next location to be read to
public:
   MatrixInput(const MatrixInput& mi) : n(mi.n), r(mi.r) {}
   MatrixInput(int nx, Real* rx) : n(nx), r(rx) {}
   ~MatrixInput();
   MatrixInput operator<<(Real);
   MatrixInput operator<<(int f);
   friend class GeneralMatrix;
};
class RSPF_DLL SimpleIntArray : public RBD_COMMON::Janitor
{
protected:
   int* a;                    // pointer to the array
   int n;                     // length of the array
public:
   SimpleIntArray(int xn);    // build an array length xn
   ~SimpleIntArray();         // return the space to memory
   int& operator[](int i);    // access element of the array - start at 0
   int operator[](int i) const;
			      // access element of constant array
   void operator=(int ai);    // set the array equal to a constant
   void operator=(const SimpleIntArray& b);
			      // copy the elements of an array
   SimpleIntArray(const SimpleIntArray& b);
			      // make a new array equal to an existing one
   int Size() const { return n; }
			      // return the size of the array
   int* Data() { return a; }  // pointer to the data
   const int* Data() const { return a; }
			      // pointer to the data
   void ReSize(int i, bool keep = false);
   void CleanUp() { ReSize(0); }
   NEW_DELETE(SimpleIntArray)
};
class RSPF_DLL NPDException : public Runtime_error     // Not positive definite
{
public:
   static unsigned long Select;          // for identifying exception
   NPDException(const GeneralMatrix&);
};
class RSPF_DLL ConvergenceException : public Runtime_error
{
public:
   static unsigned long Select;          // for identifying exception
   ConvergenceException(const GeneralMatrix& A);
   ConvergenceException(const char* c);
};
class RSPF_DLL SingularException : public Runtime_error
{
public:
   static unsigned long Select;          // for identifying exception
   SingularException(const GeneralMatrix& A);
};
class RSPF_DLL OverflowException : public Runtime_error
{
public:
   static unsigned long Select;          // for identifying exception
   OverflowException(const char* c);
};
class RSPF_DLL ProgramException : public Logic_error
{
protected:
   ProgramException();
public:
   static unsigned long Select;          // for identifying exception
   ProgramException(const char* c);
   ProgramException(const char* c, const GeneralMatrix&);
   ProgramException(const char* c, const GeneralMatrix&, const GeneralMatrix&);
   ProgramException(const char* c, MatrixType, MatrixType);
};
class RSPF_DLL IndexException : public Logic_error
{
public:
   static unsigned long Select;          // for identifying exception
   IndexException(int i, const GeneralMatrix& A);
   IndexException(int i, int j, const GeneralMatrix& A);
   IndexException(int i, const GeneralMatrix& A, bool);
   IndexException(int i, int j, const GeneralMatrix& A, bool);
};
class RSPF_DLL VectorException : public Logic_error    // cannot convert to vector
{
public:
   static unsigned long Select;          // for identifying exception
   VectorException();
   VectorException(const GeneralMatrix& A);
};
class RSPF_DLL NotSquareException : public Logic_error
{
public:
   static unsigned long Select;          // for identifying exception
   NotSquareException(const GeneralMatrix& A);
   NotSquareException();
};
class RSPF_DLL SubMatrixDimensionException : public Logic_error
{
public:
   static unsigned long Select;          // for identifying exception
   SubMatrixDimensionException();
};
class RSPF_DLL IncompatibleDimensionsException : public Logic_error
{
public:
   static unsigned long Select;          // for identifying exception
   IncompatibleDimensionsException();
   IncompatibleDimensionsException(const GeneralMatrix&, const GeneralMatrix&);
};
class RSPF_DLL NotDefinedException : public Logic_error
{
public:
   static unsigned long Select;          // for identifying exception
   NotDefinedException(const char* op, const char* matrix);
};
class RSPF_DLL CannotBuildException : public Logic_error
{
public:
   static unsigned long Select;          // for identifying exception
   CannotBuildException(const char* matrix);
};
class RSPF_DLL InternalException : public Logic_error
{
public:
   static unsigned long Select;          // for identifying exception
   InternalException(const char* c);
};
bool operator==(const GeneralMatrix& A, const GeneralMatrix& B);
bool operator==(const BaseMatrix& A, const BaseMatrix& B);
inline bool operator!=(const GeneralMatrix& A, const GeneralMatrix& B)
   { return ! (A==B); }
inline bool operator!=(const BaseMatrix& A, const BaseMatrix& B)
   { return ! (A==B); }
inline bool operator<=(const BaseMatrix& A, const BaseMatrix&)
   { A.IEQND(); return true; }
inline bool operator>=(const BaseMatrix& A, const BaseMatrix&)
   { A.IEQND(); return true; }
inline bool operator<(const BaseMatrix& A, const BaseMatrix&)
   { A.IEQND(); return true; }
inline bool operator>(const BaseMatrix& A, const BaseMatrix&)
   { A.IEQND(); return true; }
bool IsZero(const BaseMatrix& A);
Matrix CrossProduct(const Matrix& A, const Matrix& B);
ReturnMatrix CrossProductRows(const Matrix& A, const Matrix& B);
ReturnMatrix CrossProductColumns(const Matrix& A, const Matrix& B);
inline LogAndSign LogDeterminant(const BaseMatrix& B)
   { return B.LogDeterminant(); }
inline Real Determinant(const BaseMatrix& B)
   { return B.Determinant(); }
inline Real SumSquare(const BaseMatrix& B) { return B.SumSquare(); }
inline Real NormFrobenius(const BaseMatrix& B) { return B.NormFrobenius(); }
inline Real Trace(const BaseMatrix& B) { return B.Trace(); }
inline Real SumAbsoluteValue(const BaseMatrix& B)
   { return B.SumAbsoluteValue(); }
inline Real Sum(const BaseMatrix& B)
   { return B.Sum(); }
inline Real MaximumAbsoluteValue(const BaseMatrix& B)
   { return B.MaximumAbsoluteValue(); }
inline Real MinimumAbsoluteValue(const BaseMatrix& B)
   { return B.MinimumAbsoluteValue(); }
inline Real Maximum(const BaseMatrix& B) { return B.Maximum(); }
inline Real Minimum(const BaseMatrix& B) { return B.Minimum(); }
inline Real Norm1(const BaseMatrix& B) { return B.Norm1(); }
inline Real Norm1(RowVector& RV) { return RV.MaximumAbsoluteValue(); }
inline Real NormInfinity(const BaseMatrix& B) { return B.NormInfinity(); }
inline Real NormInfinity(ColumnVector& CV)
   { return CV.MaximumAbsoluteValue(); }
inline bool IsZero(const GeneralMatrix& A) { return A.IsZero(); }
inline MatrixInput MatrixInput::operator<<(int f) { return *this << (Real)f; }
inline MatrixInput GeneralMatrix::operator<<(int f) { return *this << (Real)f; }
inline MatrixInput BandMatrix::operator<<(int f) { return *this << (Real)f; }
inline MatrixInput GetSubMatrix::operator<<(int f) { return *this << (Real)f; }
#ifdef use_namespace
}
#endif
#endif
