#include <rspf/matrix/include.h>
#include <rspf/matrix/newmat.h>
#include <rspf/matrix/newmatrc.h>
#ifdef use_namespace
namespace NEWMAT {
#endif
#ifdef DO_REPORT
#define REPORT { static ExeCounter ExeCount(__LINE__,3); ++ExeCount; }
#else
#define REPORT {}
#endif
#define MONITOR(what,store,storage) {}
void GeneralMatrix::NextRow(MatrixRowCol& mrc)
{
   REPORT
   if (+(mrc.cw*StoreOnExit)) { REPORT this->RestoreRow(mrc); }
   mrc.rowcol++;
   if (mrc.rowcol<nrows) { REPORT this->GetRow(mrc); }
   else { REPORT mrc.cw -= StoreOnExit; }
}
void GeneralMatrix::NextCol(MatrixRowCol& mrc)
{
   REPORT                                                // 423
   if (+(mrc.cw*StoreOnExit)) { REPORT this->RestoreCol(mrc); }
   mrc.rowcol++;
   if (mrc.rowcol<ncols) { REPORT this->GetCol(mrc); }
   else { REPORT mrc.cw -= StoreOnExit; }
}
void GeneralMatrix::NextCol(MatrixColX& mrc)
{
   REPORT                                                // 423
   if (+(mrc.cw*StoreOnExit)) { REPORT this->RestoreCol(mrc); }
   mrc.rowcol++;
   if (mrc.rowcol<ncols) { REPORT this->GetCol(mrc); }
   else { REPORT mrc.cw -= StoreOnExit; }
}
void Matrix::GetRow(MatrixRowCol& mrc)
{
   REPORT
   mrc.skip=0; mrc.storage=mrc.length=ncols; mrc.data=store+mrc.rowcol*ncols;
}
void Matrix::GetCol(MatrixRowCol& mrc)
{
   REPORT
   mrc.skip=0; mrc.storage=mrc.length=nrows;
   if ( ncols==1 && !(mrc.cw*StoreHere) )      // ColumnVector
      { REPORT mrc.data=store; }
   else
   {
      Real* ColCopy;
      if ( !(mrc.cw*(HaveStore+StoreHere)) )
      {
         REPORT
         ColCopy = new Real [nrows]; MatrixErrorNoSpace(ColCopy);
         MONITOR_REAL_NEW("Make (MatGetCol)",nrows,ColCopy)
         mrc.data = ColCopy; mrc.cw += HaveStore;
      }
      else { REPORT ColCopy = mrc.data; }
      if (+(mrc.cw*LoadOnEntry))
      {
         REPORT
         Real* Mstore = store+mrc.rowcol; int i=nrows;
         if (i) for (;;)
            { *ColCopy++ = *Mstore; if (!(--i)) break; Mstore+=ncols; }
      }
   }
}
void Matrix::GetCol(MatrixColX& mrc)
{
   REPORT
   mrc.skip=0; mrc.storage=nrows; mrc.length=nrows;
   if (+(mrc.cw*LoadOnEntry))
   {
      REPORT  Real* ColCopy = mrc.data;
      Real* Mstore = store+mrc.rowcol; int i=nrows;
      if (i) for (;;)
          { *ColCopy++ = *Mstore; if (!(--i)) break; Mstore+=ncols; }
   }
}
void Matrix::RestoreCol(MatrixRowCol& mrc)
{
   REPORT                                   // 429
   if (+(mrc.cw*HaveStore))
   {
      REPORT                                // 426
      Real* Mstore = store+mrc.rowcol; int i=nrows;
      Real* Cstore = mrc.data;
      if (i) for (;;)
          { *Mstore = *Cstore++; if (!(--i)) break; Mstore+=ncols; }
   }
}
void Matrix::RestoreCol(MatrixColX& mrc)
{
   REPORT
   Real* Mstore = store+mrc.rowcol; int i=nrows; Real* Cstore = mrc.data;
   if (i) for (;;)
      { *Mstore = *Cstore++; if (!(--i)) break; Mstore+=ncols; }
}
void Matrix::NextRow(MatrixRowCol& mrc) { REPORT mrc.IncrMat(); }  // 1808
void Matrix::NextCol(MatrixRowCol& mrc)
{
   REPORT                                        // 632
   if (+(mrc.cw*StoreOnExit)) { REPORT RestoreCol(mrc); }
   mrc.rowcol++;
   if (mrc.rowcol<ncols)
   {
      if (+(mrc.cw*LoadOnEntry))
      {
         REPORT
         Real* ColCopy = mrc.data;
         Real* Mstore = store+mrc.rowcol; int i=nrows;
         if (i) for (;;)
            { *ColCopy++ = *Mstore; if (!(--i)) break; Mstore+=ncols; }
      }
   }
   else { REPORT mrc.cw -= StoreOnExit; }
}
void Matrix::NextCol(MatrixColX& mrc)
{
   REPORT
   if (+(mrc.cw*StoreOnExit)) { REPORT RestoreCol(mrc); }
   mrc.rowcol++;
   if (mrc.rowcol<ncols)
   {
      if (+(mrc.cw*LoadOnEntry))
      {
         REPORT
         Real* ColCopy = mrc.data;
         Real* Mstore = store+mrc.rowcol; int i=nrows;
         if (i) for (;;)
            { *ColCopy++ = *Mstore; if (!(--i)) break; Mstore+=ncols; }
      }
   }
   else { REPORT mrc.cw -= StoreOnExit; }
}
void DiagonalMatrix::GetRow(MatrixRowCol& mrc)
{
   REPORT
   mrc.skip=mrc.rowcol; mrc.storage=1;
   mrc.data=store+mrc.skip; mrc.length=ncols;
}
void DiagonalMatrix::GetCol(MatrixRowCol& mrc)
{
   REPORT
   mrc.skip=mrc.rowcol; mrc.storage=1; mrc.length=nrows;
   if (+(mrc.cw*StoreHere))              // should not happen
      Throw(InternalException("DiagonalMatrix::GetCol(MatrixRowCol&)"));
   else  { REPORT mrc.data=store+mrc.skip; }
}
void DiagonalMatrix::GetCol(MatrixColX& mrc)
{
   REPORT
   mrc.skip=mrc.rowcol; mrc.storage=1; mrc.length=nrows;
   mrc.data = mrc.store+mrc.skip;
   *(mrc.data)=*(store+mrc.skip);
}
void DiagonalMatrix::NextRow(MatrixRowCol& mrc) { REPORT mrc.IncrDiag(); }
						      // 800
void DiagonalMatrix::NextCol(MatrixRowCol& mrc) { REPORT mrc.IncrDiag(); }
void DiagonalMatrix::NextCol(MatrixColX& mrc)
{
   REPORT
   if (+(mrc.cw*StoreOnExit))
      { REPORT *(store+mrc.rowcol)=*(mrc.data); }
   mrc.IncrDiag();
   int t1 = +(mrc.cw*LoadOnEntry);
   if (t1 && mrc.rowcol < ncols)
      { REPORT *(mrc.data)=*(store+mrc.rowcol); }
}
void UpperTriangularMatrix::GetRow(MatrixRowCol& mrc)
{
   REPORT
   int row = mrc.rowcol; mrc.skip=row; mrc.length=ncols;
   mrc.storage=ncols-row; mrc.data=store+(row*(2*ncols-row+1))/2;
}
void UpperTriangularMatrix::GetCol(MatrixRowCol& mrc)
{
   REPORT
   mrc.skip=0; int i=mrc.rowcol+1; mrc.storage=i;
   mrc.length=nrows; Real* ColCopy;
   if ( !(mrc.cw*(StoreHere+HaveStore)) )
   {
      REPORT                                              // not accessed
      ColCopy = new Real [nrows]; MatrixErrorNoSpace(ColCopy);
      MONITOR_REAL_NEW("Make (UT GetCol)",nrows,ColCopy)
      mrc.data = ColCopy; mrc.cw += HaveStore;
   }
   else { REPORT ColCopy = mrc.data; }
   if (+(mrc.cw*LoadOnEntry))
   {
      REPORT
      Real* Mstore = store+mrc.rowcol; int j = ncols;
      if (i) for (;;)
         { *ColCopy++ = *Mstore; if (!(--i)) break; Mstore += --j; }
   }
}
void UpperTriangularMatrix::GetCol(MatrixColX& mrc)
{
   REPORT
   mrc.skip=0; int i=mrc.rowcol+1; mrc.storage=i;
   mrc.length=nrows;
   if (+(mrc.cw*LoadOnEntry))
   {
      REPORT
      Real* ColCopy = mrc.data;
      Real* Mstore = store+mrc.rowcol; int j = ncols;
      if (i) for (;;)
         { *ColCopy++ = *Mstore; if (!(--i)) break; Mstore += --j; }
   }
}
void UpperTriangularMatrix::RestoreCol(MatrixRowCol& mrc)
{
  REPORT
  Real* Mstore = store+mrc.rowcol; int i=mrc.rowcol+1; int j = ncols;
  Real* Cstore = mrc.data;
  if (i) for (;;)
     { *Mstore = *Cstore++; if (!(--i)) break; Mstore += --j; }
}
void UpperTriangularMatrix::NextRow(MatrixRowCol& mrc) { REPORT mrc.IncrUT(); }
						      // 722
void LowerTriangularMatrix::GetRow(MatrixRowCol& mrc)
{
   REPORT
   int row=mrc.rowcol; mrc.skip=0; mrc.storage=row+1; mrc.length=ncols;
   mrc.data=store+(row*(row+1))/2;
}
void LowerTriangularMatrix::GetCol(MatrixRowCol& mrc)
{
   REPORT
   int col=mrc.rowcol; mrc.skip=col; mrc.length=nrows;
   int i=nrows-col; mrc.storage=i; Real* ColCopy;
   if ( +(mrc.cw*(StoreHere+HaveStore)) )
      { REPORT  ColCopy = mrc.data; }
   else
   {
      REPORT                                            // not accessed
      ColCopy = new Real [nrows]; MatrixErrorNoSpace(ColCopy);
      MONITOR_REAL_NEW("Make (LT GetCol)",nrows,ColCopy)
      mrc.cw += HaveStore; mrc.data = ColCopy;
   }
   if (+(mrc.cw*LoadOnEntry))
   {
      REPORT
      Real* Mstore = store+(col*(col+3))/2;
      if (i) for (;;)
         { *ColCopy++ = *Mstore; if (!(--i)) break; Mstore += ++col; }
   }
}
void LowerTriangularMatrix::GetCol(MatrixColX& mrc)
{
   REPORT
   int col=mrc.rowcol; mrc.skip=col; mrc.length=nrows;
   int i=nrows-col; mrc.storage=i; mrc.data = mrc.store + col;
   if (+(mrc.cw*LoadOnEntry))
   {
      REPORT  Real* ColCopy = mrc.data;
      Real* Mstore = store+(col*(col+3))/2;
      if (i) for (;;)
         { *ColCopy++ = *Mstore; if (!(--i)) break; Mstore += ++col; }
   }
}
void LowerTriangularMatrix::RestoreCol(MatrixRowCol& mrc)
{
   REPORT
   int col=mrc.rowcol; Real* Cstore = mrc.data;
   Real* Mstore = store+(col*(col+3))/2; int i=nrows-col;
   if (i) for (;;)
      { *Mstore = *Cstore++; if (!(--i)) break; Mstore += ++col; }
}
void LowerTriangularMatrix::NextRow(MatrixRowCol& mrc) { REPORT mrc.IncrLT(); }
					                 //712
void SymmetricMatrix::GetRow(MatrixRowCol& mrc)
{
   REPORT                                                //571
   mrc.skip=0; int row=mrc.rowcol; mrc.length=ncols;
   if (+(mrc.cw*DirectPart))
      { REPORT mrc.storage=row+1; mrc.data=store+(row*(row+1))/2; }
   else
   {
      if (+(mrc.cw*StoreOnExit))
         Throw(InternalException("SymmetricMatrix::GetRow(MatrixRowCol&)"));
      mrc.storage=ncols; Real* RowCopy;
      if (!(mrc.cw*HaveStore))
      {
         REPORT
         RowCopy = new Real [ncols]; MatrixErrorNoSpace(RowCopy);
         MONITOR_REAL_NEW("Make (SymGetRow)",ncols,RowCopy)
         mrc.cw += HaveStore; mrc.data = RowCopy;
      }
      else { REPORT RowCopy = mrc.data; }
      if (+(mrc.cw*LoadOnEntry))
      {
         REPORT                                         // 544
         Real* Mstore = store+(row*(row+1))/2; int i = row;
         while (i--) *RowCopy++ = *Mstore++;
         i = ncols-row;
         if (i) for (;;)
            { *RowCopy++ = *Mstore; if (!(--i)) break; Mstore += ++row; }
      }
   }
}
void SymmetricMatrix::GetCol(MatrixRowCol& mrc)
{
   if (+(mrc.cw*StoreHere))
      Throw(InternalException("SymmetricMatrix::GetCol(MatrixRowCol&)"));
   int col=mrc.rowcol; mrc.length=nrows;
   REPORT
   mrc.skip=0;
   if (+(mrc.cw*DirectPart))    // actually get row ??
      { REPORT mrc.storage=col+1; mrc.data=store+(col*(col+1))/2; }
   else
   {
      if (+(mrc.cw*StoreOnExit))
         Throw(InternalException("SymmetricMatrix::GetCol(MatrixRowCol&)"));
      mrc.storage=ncols; Real* ColCopy;
      if ( +(mrc.cw*HaveStore)) { REPORT ColCopy = mrc.data; }
      else
      {
         REPORT                                      // not accessed
         ColCopy = new Real [ncols]; MatrixErrorNoSpace(ColCopy);
         MONITOR_REAL_NEW("Make (SymGetCol)",ncols,ColCopy)
         mrc.cw += HaveStore; mrc.data = ColCopy;
      }
      if (+(mrc.cw*LoadOnEntry))
      {
         REPORT
         Real* Mstore = store+(col*(col+1))/2; int i = col;
         while (i--) *ColCopy++ = *Mstore++;
         i = ncols-col;
         if (i) for (;;)
            { *ColCopy++ = *Mstore; if (!(--i)) break; Mstore += ++col; }
      }
   }
}
void SymmetricMatrix::GetCol(MatrixColX& mrc)
{
   int col=mrc.rowcol; mrc.length=nrows;
   if (+(mrc.cw*DirectPart))
   {
      REPORT
      mrc.skip=col; int i=nrows-col; mrc.storage=i;
      mrc.data = mrc.store+col;
      if (+(mrc.cw*LoadOnEntry))
      {
         REPORT                           // not accessed
         Real* ColCopy = mrc.data;
         Real* Mstore = store+(col*(col+3))/2;
         if (i) for (;;)
            { *ColCopy++ = *Mstore; if (!(--i)) break; Mstore += ++col; }
      }
   }
   else
   {
      REPORT
      if (+(mrc.cw*StoreOnExit))
         Throw(InternalException("SymmetricMatrix::GetCol(MatrixColX&)"));
      mrc.skip=0; mrc.storage=ncols;
      if (+(mrc.cw*LoadOnEntry))
      {
         REPORT
         Real* ColCopy = mrc.data;
         Real* Mstore = store+(col*(col+1))/2; int i = col;
         while (i--) *ColCopy++ = *Mstore++;
         i = ncols-col;
         if (i) for (;;)
            { *ColCopy++ = *Mstore; if (!(--i)) break; Mstore += ++col; }
      }
   }
}
void SymmetricMatrix::RestoreCol(MatrixColX& mrc)
{
   REPORT
   int col=mrc.rowcol; Real* Cstore = mrc.data;
   Real* Mstore = store+(col*(col+3))/2; int i = nrows-col;
   if (i) for (;;)
      { *Mstore = *Cstore++; if (!(--i)) break; Mstore+= ++col; }
}
void RowVector::GetCol(MatrixRowCol& mrc)
{
   REPORT
   if (+(mrc.cw*StoreHere))
      Throw(InternalException("RowVector::GetCol(MatrixRowCol&)"));
   mrc.skip=0; mrc.storage=1; mrc.length=nrows; mrc.data = store+mrc.rowcol;
}
void RowVector::GetCol(MatrixColX& mrc)
{
   REPORT
   mrc.skip=0; mrc.storage=1; mrc.length=nrows;
   if (mrc.cw >= LoadOnEntry)
      { REPORT *(mrc.data) = *(store+mrc.rowcol); }
}
void RowVector::NextCol(MatrixRowCol& mrc)
{ REPORT mrc.rowcol++; mrc.data++; }
void RowVector::NextCol(MatrixColX& mrc)
{
   if (+(mrc.cw*StoreOnExit)) { REPORT *(store+mrc.rowcol)=*(mrc.data); }
   mrc.rowcol++;
   if (mrc.rowcol < ncols)
   {
      if (+(mrc.cw*LoadOnEntry)) { REPORT *(mrc.data)=*(store+mrc.rowcol); }
   }
   else { REPORT mrc.cw -= StoreOnExit; }
}
void RowVector::RestoreCol(MatrixColX& mrc)
   { REPORT *(store+mrc.rowcol)=*(mrc.data); }           // not accessed
void BandMatrix::GetRow(MatrixRowCol& mrc)
{
   REPORT
   int r = mrc.rowcol; int w = lower+1+upper; mrc.length=ncols;
   int s = r-lower;
   if (s<0) { mrc.data = store+(r*w-s); w += s; s = 0; }
   else mrc.data = store+r*w;
   mrc.skip = s; s += w-ncols; if (s>0) w -= s; mrc.storage = w;
}
void BandMatrix::NextRow(MatrixRowCol& mrc)
{
   REPORT
   int r = ++mrc.rowcol;
   if (r<=lower) { mrc.storage++; mrc.data += lower+upper; }
   else  { mrc.skip++; mrc.data += lower+upper+1; }
   if (r>=ncols-upper) mrc.storage--;
}
void BandMatrix::GetCol(MatrixRowCol& mrc)
{
   REPORT
   int c = mrc.rowcol; int n = lower+upper; int w = n+1;
   mrc.length=nrows; Real* ColCopy;
   int b; int s = c-upper;
   if (s<=0) { w += s; s = 0; b = c+lower; } else b = s*w+n;
   mrc.skip = s; s += w-nrows; if (s>0) w -= s; mrc.storage = w;
   if ( +(mrc.cw*(StoreHere+HaveStore)) )
      { REPORT ColCopy = mrc.data; }
   else
   {
      REPORT
      ColCopy = new Real [n+1]; MatrixErrorNoSpace(ColCopy);
      MONITOR_REAL_NEW("Make (BMGetCol)",n+1,ColCopy)
      mrc.cw += HaveStore; mrc.data = ColCopy;
   }
   if (+(mrc.cw*LoadOnEntry))
   {
      REPORT
      Real* Mstore = store+b;
      if (w) for (;;)
         { *ColCopy++ = *Mstore; if (!(--w)) break; Mstore+=n; }
   }
}
void BandMatrix::GetCol(MatrixColX& mrc)
{
   REPORT
   int c = mrc.rowcol; int n = lower+upper; int w = n+1;
   mrc.length=nrows; int b; int s = c-upper;
   if (s<=0) { w += s; s = 0; b = c+lower; } else b = s*w+n;
   mrc.skip = s; s += w-nrows; if (s>0) w -= s; mrc.storage = w;
   mrc.data = mrc.store+mrc.skip;
   if (+(mrc.cw*LoadOnEntry))
   {
      REPORT
      Real* ColCopy = mrc.data; Real* Mstore = store+b;
      if (w) for (;;)
         { *ColCopy++ = *Mstore; if (!(--w)) break; Mstore+=n; }
   }
}
void BandMatrix::RestoreCol(MatrixRowCol& mrc)
{
   REPORT
   int c = mrc.rowcol; int n = lower+upper; int s = c-upper;
   Real* Mstore = store + ((s<=0) ? c+lower : s*n+s+n);
   Real* Cstore = mrc.data;
   int w = mrc.storage;
   if (w) for (;;)
      { *Mstore = *Cstore++; if (!(--w)) break; Mstore += n; }
}
void SymmetricBandMatrix::GetRow(MatrixRowCol& mrc)
{
   REPORT
   int r=mrc.rowcol; int s = r-lower; int w1 = lower+1; int o = r*w1;
   mrc.length = ncols;
   if (s<0) { w1 += s; o -= s; s = 0; }
   mrc.skip = s;
   if (+(mrc.cw*DirectPart))
      { REPORT  mrc.data = store+o; mrc.storage = w1; }
   else
   {
      if (+(mrc.cw*StoreOnExit))
         Throw(InternalException("SymmetricBandMatrix::GetRow(MatrixRowCol&)"));
      int w = w1+lower; s += w-ncols; Real* RowCopy;
      if (s>0) w -= s; mrc.storage = w; int w2 = w-w1;
      if (!(mrc.cw*HaveStore))
      {
         REPORT
         RowCopy = new Real [2*lower+1]; MatrixErrorNoSpace(RowCopy);
         MONITOR_REAL_NEW("Make (SmBGetRow)",2*lower+1,RowCopy)
         mrc.cw += HaveStore; mrc.data = RowCopy;
      }
      else { REPORT  RowCopy = mrc.data; }
      if (+(mrc.cw*LoadOnEntry))
      {
	      REPORT
         Real* Mstore = store+o;
         while (w1--) *RowCopy++ = *Mstore++;
         Mstore--;
         while (w2--) { Mstore += lower; *RowCopy++ = *Mstore; }
      }
   }
}
void SymmetricBandMatrix::GetCol(MatrixRowCol& mrc)
{
   if (+(mrc.cw*StoreHere))
      Throw(InternalException("SymmetricBandMatrix::GetCol(MatrixRowCol&)"));
   int c=mrc.rowcol; int w1 = lower+1; mrc.length=nrows;
   REPORT
   int s = c-lower; int o = c*w1;
   if (s<0) { w1 += s; o -= s; s = 0; }
   mrc.skip = s;
   if (+(mrc.cw*DirectPart))
   { REPORT  mrc.data = store+o; mrc.storage = w1; }
   else
   {
      if (+(mrc.cw*StoreOnExit))
         Throw(InternalException("SymmetricBandMatrix::GetCol(MatrixRowCol&)"));
      int w = w1+lower; s += w-ncols; Real* ColCopy;
      if (s>0) w -= s; mrc.storage = w; int w2 = w-w1;
      if ( +(mrc.cw*HaveStore) ) { REPORT ColCopy = mrc.data; }
      else
      {
         REPORT ColCopy = new Real [2*lower+1]; MatrixErrorNoSpace(ColCopy);
         MONITOR_REAL_NEW("Make (SmBGetCol)",2*lower+1,ColCopy)
         mrc.cw += HaveStore; mrc.data = ColCopy;
      }
      if (+(mrc.cw*LoadOnEntry))
      {
         REPORT
         Real* Mstore = store+o;
         while (w1--) *ColCopy++ = *Mstore++;
         Mstore--;
         while (w2--) { Mstore += lower; *ColCopy++ = *Mstore; }
      }
   }
}
void SymmetricBandMatrix::GetCol(MatrixColX& mrc)
{
   int c=mrc.rowcol; int w1 = lower+1; mrc.length=nrows;
   if (+(mrc.cw*DirectPart))
   {
      REPORT
      int b = c*w1+lower;
      mrc.skip = c; c += w1-nrows; w1 -= c; mrc.storage = w1;
      Real* ColCopy = mrc.data = mrc.store+mrc.skip;
      if (+(mrc.cw*LoadOnEntry))
      {
         REPORT
         Real* Mstore = store+b;
         if (w1) for (;;)
            { *ColCopy++ = *Mstore; if (!(--w1)) break; Mstore += lower; }
      }
   }
   else
   {
      REPORT
      if (+(mrc.cw*StoreOnExit))
         Throw(InternalException("SymmetricBandMatrix::GetCol(MatrixColX&)"));
      int s = c-lower; int o = c*w1;
      if (s<0) { w1 += s; o -= s; s = 0; }
      mrc.skip = s;
      int w = w1+lower; s += w-ncols;
      if (s>0) w -= s; mrc.storage = w; int w2 = w-w1;
      Real* ColCopy = mrc.data = mrc.store+mrc.skip;
      if (+(mrc.cw*LoadOnEntry))
      {
         REPORT
         Real* Mstore = store+o;
         while (w1--) *ColCopy++ = *Mstore++;
         Mstore--;
         while (w2--) { Mstore += lower; *ColCopy++ = *Mstore; }
      }
   }
}
void SymmetricBandMatrix::RestoreCol(MatrixColX& mrc)
{
   REPORT
   int c = mrc.rowcol;
   Real* Mstore = store + c*lower+c+lower;
   Real* Cstore = mrc.data; int w = mrc.storage;
   if (w) for (;;)
      { *Mstore = *Cstore++; if (!(--w)) break; Mstore += lower; }
}
void IdentityMatrix::GetRow(MatrixRowCol& mrc)
{
   REPORT
   mrc.skip=mrc.rowcol; mrc.storage=1; mrc.data=store; mrc.length=ncols;
}
void IdentityMatrix::GetCol(MatrixRowCol& mrc)
{
   REPORT
   mrc.skip=mrc.rowcol; mrc.storage=1; mrc.length=nrows;
   if (+(mrc.cw*StoreHere))              // should not happen
      Throw(InternalException("IdentityMatrix::GetCol(MatrixRowCol&)"));
   else  { REPORT mrc.data=store; }
}
void IdentityMatrix::GetCol(MatrixColX& mrc)
{
   REPORT
   mrc.skip=mrc.rowcol; mrc.storage=1; mrc.length=nrows;
   mrc.data = mrc.store+mrc.skip; *(mrc.data)=*store;
}
void IdentityMatrix::NextRow(MatrixRowCol& mrc) { REPORT mrc.IncrId(); }
void IdentityMatrix::NextCol(MatrixRowCol& mrc) { REPORT mrc.IncrId(); }
void IdentityMatrix::NextCol(MatrixColX& mrc)
{
   REPORT
   if (+(mrc.cw*StoreOnExit)) { REPORT *store=*(mrc.data); }
   mrc.IncrDiag();            // must increase mrc.data so need IncrDiag
   int t1 = +(mrc.cw*LoadOnEntry);
   if (t1 && mrc.rowcol < ncols) { REPORT *(mrc.data)=*store; }
}
MatrixRowCol::~MatrixRowCol()
{
   if (+(cw*HaveStore))
   {
      MONITOR_REAL_DELETE("Free    (RowCol)",-1,data)  // do not know length
      delete [] data;
   }
}
MatrixRow::~MatrixRow() { if (+(cw*StoreOnExit)) gm->RestoreRow(*this); }
MatrixCol::~MatrixCol() { if (+(cw*StoreOnExit)) gm->RestoreCol(*this); }
MatrixColX::~MatrixColX() { if (+(cw*StoreOnExit)) gm->RestoreCol(*this); }
#ifdef use_namespace
}
#endif
