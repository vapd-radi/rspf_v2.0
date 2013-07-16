/*! \file kbool/include/kbool/record.h
    \author Probably Klaas Holwerda
    Copyright: 2001-2004 (C) Probably Klaas Holwerda
    Licence: wxWidgets Licence
    RCS-ID: $Id: record.h 14798 2009-06-29 19:19:22Z dburken $
*/
#ifndef RECORD_H
#define RECORD_H
#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface
#endif
class Node;
#include <rspf/kbool/booleng.h>
#include <rspf/kbool/link.h>
#include <rspf/kbool/line.h>
enum BEAM_TYPE { NORMAL,FLAT};
enum DIRECTION  {GO_LEFT,GO_RIGHT};
class Bool_Engine;
class A2DKBOOLDLLEXP Record
{
        protected:
                                        Bool_Engine* _GC;
	public:
					Record(KBoolLink* link,Bool_Engine* GC);
					~Record();
					void SetNewLink(KBoolLink* link);
					void Set_Flags();
					void Calc_Ysp(Node* low);
					KBoolLink* GetLink();
					KBoolLine* GetLine();
					B_INT	Ysp();
					void SetYsp(B_INT ysp);
					DIRECTION Direction();
					bool Calc_Left_Right(Record* record_above_me);
					bool Equal(Record*);
	private:
					KBoolLine   _line;
					B_INT 		_ysp;
					DIRECTION	_dir;    
               int         _a;               
               int         _b;               
};
#endif 
