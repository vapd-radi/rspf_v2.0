#ifndef NEWMATIO_LIB
#define NEWMATIO_LIB 0
#ifndef WANT_STREAM
#define WANT_STREAM
#endif
#include <rspf/matrix/newmat.h>
#include <iostream>
#ifdef use_namespace
namespace NEWMAT {
#endif
/**************************** input/output *****************************/
	RSPF_DLL std::ostream& operator<<(std::ostream&, const BaseMatrix&);
	RSPF_DLL std::ostream& operator<<(std::ostream&, const GeneralMatrix&);
/*  Use in some old versions of G++ without complete iomanipulators
class Omanip_precision
{
   int x;
public:
   Omanip_precision(int i) : x(i) {}
   friend ostream& operator<<(ostream& os, Omanip_precision i);
};
Omanip_precision setprecision(int i);
class Omanip_width
{
   int x;
public:
   Omanip_width(int i) : x(i) {}
   friend ostream& operator<<(ostream& os, Omanip_width i);
};
Omanip_width setw(int i);
*/
#ifdef use_namespace
}
#endif
#endif
