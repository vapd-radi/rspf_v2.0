/*! \file kbool/include/kbool/node.h
    \brief Holds a GDSII node structure (Header)
    \author Probably Klaas Holwerda
    Copyright: 2001-2004 (C) Probably Klaas Holwerda
    Licence: wxWidgets Licence
    RCS-ID: $Id: node.h 12629 2008-04-07 17:19:06Z dburken $
*/
#ifndef NODE_H
#define NODE_H
#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface
#endif
#include <rspf/kbool/booleng.h>
#include <rspf/kbool/lpoint.h>
#include <rspf/kbool/link.h>
#include <rspf/kbool/_lnk_itr.h>
enum NodePosition { N_LEFT, N_ON, N_RIGHT};
class A2DKBOOLDLLEXP Node : public LPoint
{
	protected:
      Bool_Engine* _GC;
	public:
		// friend must be deleted in the final version!
		friend class Debug_driver;
		// constructors and destructors
		Node(Bool_Engine* GC);
		Node(const B_INT, const B_INT, Bool_Engine* GC);
	
		Node(LPoint* const a_point, Bool_Engine* GC);
		Node(Node * const, Bool_Engine* GC);
		Node& operator=(const Node &other_node);
		~Node();
		//public member functions
		void AddLink(KBoolLink*);
		DL_List<void*>* GetLinklist();
      bool SameSides( KBoolLink* const prev , KBoolLink* const link, BOOL_OP operation );
      /*! flags the same on the sides of the new link.
      */
		KBoolLink*  GetMost( KBoolLink* const prev ,LinkStatus whatside, BOOL_OP operation );
      KBoolLink* GetMostHole( KBoolLink* const prev ,LinkStatus whatside, BOOL_OP operation );
      KBoolLink* 	GetNotFlat();
      KBoolLink* GetHoleLink( KBoolLink* const prev, bool checkbin, BOOL_OP operation );
		int Merge(Node*);
		void RemoveLink(KBoolLink*);
		bool Simplify(Node* First, Node* Second, B_INT Marge );
		//  memberfunctions for maximum performance
		void        RoundInt(B_INT grid);
		KBoolLink*	GetIncomingLink();
		int		  GetNumberOfLinks();
		KBoolLink* GetNextLink();
		KBoolLink* GetOtherLink(KBoolLink*);
		KBoolLink* GetOutgoingLink();
		KBoolLink* GetPrevLink();
		KBoolLink* Follow(KBoolLink* const prev );
		KBoolLink* GetBinHighest(bool binset);
	protected:
		DL_List<void*>*		_linklist;
};
#endif
