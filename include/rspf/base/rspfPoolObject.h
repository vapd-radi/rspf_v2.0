//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  David Burken
//
//**********************************************************************
// $Id: rspfPoolObject.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfPoolObject_HEADER
#define rspfPoolObject_HEADER


/**
 * Templated container for an object pointer and a flag.
 */
template <class T>
class rspfPoolObject
{
 public:
   ~rspfPoolObject()
   {
      if(theObjectPtr)
      {
         cout << "deleting obj:  " << (hex) << theObjectPtr << endl;
         delete theObjectPtr;
         theObjectPtr = NULL;
      }
      theAvailableFlag = false;  
   }


   bool theAvailableFlag;
   T*   theObjectPtr;
};
#endif /* #ifndef rspfPoolObject_HEADER */
