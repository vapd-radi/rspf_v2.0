#ifndef NEWMATRC_LIB
#define NEWMATRC_LIB 0
#ifdef use_namespace
namespace NEWMAT {
#endif
#include <rspf/matrix/controlw.h>
/************** classes MatrixRowCol, MatrixRow, MatrixCol *****************/
enum LSF { LoadOnEntry=1,StoreOnExit=2,DirectPart=4,StoreHere=8,HaveStore=16 };
class LoadAndStoreFlag : public ControlWord
{
public:
   LoadAndStoreFlag() {}
   LoadAndStoreFlag(int i) : ControlWord(i) {}
   LoadAndStoreFlag(LSF lsf) : ControlWord(lsf) {}
   LoadAndStoreFlag(const ControlWord& cwx) : ControlWord(cwx) {}
};
class MatrixRowCol
{
public:                                        // these are public to avoid
   int length;                                 // row or column length
   int skip;                                   // initial number of zeros
   int storage;                                // number of stored elements
   int rowcol;                                 // row or column number
   GeneralMatrix* gm;                          // pointer to parent matrix
   Real* data;                                 // pointer to local storage
   LoadAndStoreFlag cw;                        // Load? Store? Is a Copy?
   void IncrMat() { rowcol++; data += storage; }   // used by NextRow
   void IncrDiag() { rowcol++; skip++; data++; }
   void IncrId() { rowcol++; skip++; }
   void IncrUT() { rowcol++; data += storage; storage--; skip++; }
   void IncrLT() { rowcol++; data += storage; storage++; }
public:
   void Zero();                                // set elements to zero
   void Add(const MatrixRowCol&);              // add a row/col
   void AddScaled(const MatrixRowCol&, Real);  // add a multiple of a row/col
   void Add(const MatrixRowCol&, const MatrixRowCol&);
   void Add(const MatrixRowCol&, Real);        // add a row/col
   void NegAdd(const MatrixRowCol&, Real);     // Real - a row/col
   void Sub(const MatrixRowCol&);              // subtract a row/col
   void Sub(const MatrixRowCol&, const MatrixRowCol&);
					       // sub a row/col from another
   void RevSub(const MatrixRowCol&);           // subtract from a row/col
   void ConCat(const MatrixRowCol&, const MatrixRowCol&);
   void Multiply(const MatrixRowCol&);         // multiply a row/col
   void Multiply(const MatrixRowCol&, const MatrixRowCol&);
   void KP(const MatrixRowCol&, const MatrixRowCol&);
   void Copy(const MatrixRowCol&);             // copy a row/col
   void CopyCheck(const MatrixRowCol&);        // ... check for data loss
   void Check(const MatrixRowCol&);            // just check for data loss
   void Check();                               // check full row/col present
   void Copy(const Real*&);                    // copy from an array
   void Copy(const int*&);                     // copy from an array
   void Copy(Real);                            // copy from constant
   void Add(Real);                             // add a constant
   void Multiply(Real);                        // multiply by constant
   Real SumAbsoluteValue();                    // sum of absolute values
   Real MaximumAbsoluteValue1(Real r, int& i); // maximum of absolute values
   Real MinimumAbsoluteValue1(Real r, int& i); // minimum of absolute values
   Real Maximum1(Real r, int& i);              // maximum
   Real Minimum1(Real r, int& i);              // minimum
   Real Sum();                                 // sum of values
   void Inject(const MatrixRowCol&);           // copy stored els of a row/col
   void Negate(const MatrixRowCol&);           // change sign of a row/col
   void Multiply(const MatrixRowCol&, Real);   // scale a row/col
   friend Real DotProd(const MatrixRowCol&, const MatrixRowCol&);
   Real* Data() { return data; }
   int Skip() { return skip; }                 // number of elements skipped
   int Storage() { return storage; }           // number of elements stored
   int Length() { return length; }             // length of row or column
   void Skip(int i) { skip=i; }
   void Storage(int i) { storage=i; }
   void Length(int i) { length=i; }
   void SubRowCol(MatrixRowCol&, int, int) const;
					       // get part of a row or column
   MatrixRowCol() {}                           // to stop warning messages
   ~MatrixRowCol();
   FREE_CHECK(MatrixRowCol)
};
class MatrixRow : public MatrixRowCol
{
public:
   MatrixRow(GeneralMatrix*, LoadAndStoreFlag, int=0);
   ~MatrixRow();
   void Next();                                // get next row
   FREE_CHECK(MatrixRow)
};
class MatrixCol : public MatrixRowCol
{
public:
   MatrixCol(GeneralMatrix*, LoadAndStoreFlag, int=0);
   MatrixCol(GeneralMatrix*, Real*, LoadAndStoreFlag, int=0);
   ~MatrixCol();
   void Next();                                // get next row
   FREE_CHECK(MatrixCol)
};
class MatrixColX : public MatrixRowCol
{
public:
   MatrixColX(GeneralMatrix*, Real*, LoadAndStoreFlag, int=0);
   ~MatrixColX();
   void Next();                                // get next row
   Real* store;                                // pointer to local storage
   FREE_CHECK(MatrixColX)
};
/**************************** inline bodies ****************************/
inline MatrixRow::MatrixRow(GeneralMatrix* gmx, LoadAndStoreFlag cwx, int row)
{ gm=gmx; cw=cwx; rowcol=row; gm->GetRow(*this); }
inline void MatrixRow::Next() { gm->NextRow(*this); }
inline MatrixCol::MatrixCol(GeneralMatrix* gmx, LoadAndStoreFlag cwx, int col)
{ gm=gmx; cw=cwx; rowcol=col; gm->GetCol(*this); }
inline MatrixCol::MatrixCol(GeneralMatrix* gmx, Real* r,
   LoadAndStoreFlag cwx, int col)
{ gm=gmx; data=r; cw=cwx+StoreHere; rowcol=col; gm->GetCol(*this); }
inline MatrixColX::MatrixColX(GeneralMatrix* gmx, Real* r,
   LoadAndStoreFlag cwx, int col)
{ gm=gmx; store=data=r; cw=cwx+StoreHere; rowcol=col; gm->GetCol(*this); }
inline void MatrixCol::Next() { gm->NextCol(*this); }
inline void MatrixColX::Next() { gm->NextCol(*this); }
#ifdef use_namespace
}
#endif
#endif
