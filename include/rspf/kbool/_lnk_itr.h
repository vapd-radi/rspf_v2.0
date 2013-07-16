/*! \file kbool/include/kbool/_lnk_itr.h
    \author Probably Klaas Holwerda
    Copyright: 2001-2004 (C) Probably Klaas Holwerda
    Licence: wxWidgets Licence
    RCS-ID: $Id: _lnk_itr.h 9094 2006-06-13 19:12:40Z dburken $
*/
/*
 * Definitions of classes, for list implementation
 * template list and iterator for any list node type
*/
#ifndef _LinkBaseIter_H
#define _LinkBaseIter_H
#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface
#endif
#include <stdlib.h>
#include <rspf/kbool/booleng.h>
#define SWAP(x,y,t)((t)=(x),(x)=(y),(y)=(t))
#include <rspf/kbool/_dl_itr.h>
/*!
 class for iterator on DL_List<void*> that is type casted version of DL_Iter
 \sa DL_Iter for further documentation
*/
template<class Type> class TDLI : public DL_Iter<void*>
{
	public:
		//!constructor
      /*!
      \param list to iterate on.
      */
		TDLI(DL_List<void*>* list);
 		//!constructor
	   TDLI(DL_Iter<void*>* otheriter);
		//! nolist constructor
		TDLI();
		//! destructor
		~TDLI();
		//!call fp for each item
      void 	  foreach_f(void (*fp) (Type* item) );
      void 	  foreach_mf(void (Type::*fp) () );
		/* list mutations */
		//! delete all items
		void    delete_all		();
		void    insend		      (Type* n);
		void    insbegin		   (Type* n);
		void    insbefore       (Type* n);
		void    insafter        (Type* n);
		void    insend_unsave	 (Type* n);
		void    insbegin_unsave	 (Type* n);
		void    insbefore_unsave (Type* n);
		void    insafter_unsave  (Type* n);
		void    takeover        (DL_List<void*>* otherlist);
		void    takeover        (TDLI* otheriter);
		void    takeover        (TDLI* otheriter, int maxcount);
		bool  has             (Type*);
		bool  toitem          (Type*);
      Type*   item            ();
		void    mergesort             (int (*f)(Type* a,Type* b));
		int  cocktailsort( int (*) (Type* a,Type* b), bool (*) (Type* c,Type* d) = NULL);
};
/*!
*/
template<class Type> class TDLISort : public DL_SortIter<void*>
{
	public:
		TDLISort(DL_List<void*>* list, int (*newfunc)(void*,void*));
		~TDLISort();
		void    		 delete_all();
		bool       has     (Type*);
		bool       toitem  (Type*);
		Type* item    ();
};
/*!
 class for iterator on DL_List<void*> that is type casted version of DL_StackIter
 see also inhereted class DL_StackIter for further documentation
*/
template<class Type> class TDLIStack : public DL_StackIter<void*>
{
   public:
		TDLIStack(DL_List<void*>* list);
		~TDLIStack();
		void            push(Type*);
		Type*           pop();
};
#include <rspf/kbool/_lnk_itr.inc>
#endif
