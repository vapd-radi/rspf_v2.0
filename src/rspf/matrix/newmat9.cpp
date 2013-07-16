#define WANT_STREAM
#include <rspf/matrix/include.h>
#include <rspf/matrix/newmat.h>
#include <rspf/matrix/newmatio.h>
#include <rspf/matrix/newmatrc.h>
#ifdef use_namespace
namespace NEWMAT {
#endif
using namespace std;
#ifdef DO_REPORT
#define REPORT { static ExeCounter ExeCount(__LINE__,9); ++ExeCount; }
#else
#define REPORT {}
#endif
#ifndef ios_format_flags
#define ios_format_flags long
#endif
ostream& operator<<(ostream& s, const BaseMatrix& X)
{
   GeneralMatrix* gm = ((BaseMatrix&)X).Evaluate(); operator<<(s, *gm);
   gm->tDelete(); return s;
}
ostream& operator<<(ostream& s, const GeneralMatrix& X)
{
   MatrixRow mr((GeneralMatrix*)&X, LoadOnEntry);
   int w = s.width();  int nr = X.Nrows();
   std::ios_base::fmtflags f = s.flags();
   s.setf(ios::fixed, ios::floatfield);
   for (int i=1; i<=nr; i++)
   {
      int skip = mr.skip;  int storage = mr.storage;
      Real* store = mr.data;  skip *= w+1;
      while (skip--) s << " ";
      while (storage--) { s.width(w); s << *store++ << " "; }
      mr.Next();  s << "\n";
   }
   s << flush;  s.flags(f);
   return s;
}
/*
ostream& operator<<(ostream& os, Omanip_precision i)
   { os.precision(i.x); return os; }
Omanip_precision setprecision(int i) { return Omanip_precision(i); }
ostream& operator<<(ostream& os, Omanip_width i)
   { os.width(i.x); return os; }
Omanip_width setw(int i) { return Omanip_width(i); }
*/
#ifdef use_namespace
}
#endif
