#ifndef INCLUDE_LIB
#define INCLUDE_LIB
#define use_namespace                   // define name spaces
#include <rspf/base/rspfConstants.h>
#define SETUP_C_SUBSCRIPTS              // allow element access via A[i][j]
#define UseExceptions                   // use C++ exceptions
					// temporaries too quickly
					// to return from functions only
#define USING_DOUBLE                    // elements of type double
#define bool_LIB 0                      // for compatibility with my older libraries
#define TypeDefException
#ifdef use_namespace
namespace RBD_COMMON {
#endif
#ifdef USING_FLOAT                      // set precision type to float
typedef float Real;
typedef double long_Real;
#endif
#ifdef USING_DOUBLE                     // set precision type to double
typedef double Real;
typedef double long_Real;
#endif
#ifdef use_namespace
}
#endif
#ifdef use_namespace
namespace RBD_COMMON {}
namespace RBD_LIBRARIES                 // access all my libraries
{
   using namespace RBD_COMMON;
}
#endif
#endif
