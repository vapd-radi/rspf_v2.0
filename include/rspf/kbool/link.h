/*! \file kbool/include/kbool/link.h
    \brief Part of a graph, connection between nodes (Header)
    \author Probably Klaas Holwerda or Julian Smart
    Copyright: 2001-2004 (C) Probably Klaas Holwerda
    Licence: wxWidgets Licence
    RCS-ID: $Id: link.h 9094 2006-06-13 19:12:40Z dburken $
*/
#ifndef LINK_H
#define LINK_H
#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface
#endif
#include <rspf/kbool/booleng.h>
#include <rspf/kbool/_lnk_itr.h>
enum LinkStatus {IS_LEFT,IS_ON,IS_RIGHT};
class LPoint;
class Node;
class Record;
/*
   A Graph contains a list of KBoolLink, the KBoolLink or connected by Node's. 
   Several KBoolLink can be connected to one Node. 
   A KBoolLink has a direction defined by its begin and end node.
   Node do have a list of connected KBoolLink's.
   So one can walk trough a graph in two ways:
   1- via its KBoolLink list 
   2- via the node connected to the KBoolLink's 
*/
class A2DKBOOLDLLEXP KBoolLink
{
   protected:
      Bool_Engine* _GC;
	public:
		//! contructors 
		KBoolLink(Bool_Engine* GC);
		//! contructors 
		KBoolLink(int graphnr, Node* begin, Node* end, Bool_Engine* GC);
		//! contructors 
		KBoolLink(Node *begin, Node *end, Bool_Engine* GC);
		//! destructors
		~KBoolLink();
		void MergeNodes(Node* const);      				
      LinkStatus OutProduct(KBoolLink* const two,double accur);              
		
      LinkStatus PointOnCorner(KBoolLink* const, KBoolLink* const);	
		void Remove(Node*);		
      
		void Replace(Node* oldnode, Node* newnode);		
		void SetTopHole(bool value);
		bool IsTopHole();
		void UnMark();
		void Mark();
		void SetMark(bool);
		bool IsMarked();
      void SetHoleLink(bool val){ m_holelink = val;};
      bool GetHoleLink(){ return m_holelink;};
		void SetNotBeenHere();
		void SetBeenHere();
		bool BeenHere();										
		void	UnLink();												
		//! functions for maximum performance
		Node*	GetBeginNode();
		Node*	GetEndNode();
		Node*	GetLowNode();
		Node*	GetHighNode();
		KBoolLink* Forth(Node*);
      
		int GetGraphNum();
		bool GetInc();
      bool GetLeftA();
		bool GetLeftB();
		bool GetRightA();
		bool GetRightB();
		void GetLRO(LPoint*, int&, int&, double);
		Node*	GetOther(const Node* const);
		bool IsUnused();									
		bool IsMarked(BOOL_OP operation);            
		bool IsMarkedLeft(BOOL_OP operation);            
		bool IsMarkedRight(BOOL_OP operation);            
		bool IsHole(BOOL_OP operation);				
      
      void SetHole(bool);
		bool GetHole();
		bool IsZero(B_INT marge );		
		bool ShorterThan(B_INT marge );
		void Reset(Node* begin, Node* end, int graphnr = 0);	
		void Set(Node* begin, Node* end);
		void SetBeginNode(Node*);
		void SetEndNode(Node*);
		void SetGraphNum(int);
		void SetInc(bool);
      void SetLeftA(bool);
		void SetLeftB(bool);
		void SetRightA(bool);
		void SetRightB(bool);
		void SetGroup(GroupType);
		GroupType Group();
		void SetLineTypes();
		void Reset();
		void Reset_flags();
      void Redirect(Node* a_node); 
      void TakeOverOperationFlags( KBoolLink* link );
      void SetRecordNode( DL_Node<Record*>* recordNode ) { m_record = recordNode; }
      DL_Node<Record*>* GetRecordNode() { return m_record; }
	protected:
      Node 	*m_beginnode, *m_endnode;
      bool	m_bin    	: 1;				
      bool	m_hole     : 1;				
      bool	m_hole_top : 1;				
      bool 	m_Inc   	: 1;    			
      bool 	m_LeftA 	: 1;    			
      bool 	m_RightA	: 1;    			
      bool 	m_LeftB 	: 1;    			
      bool	m_RightB	: 1;    			
      bool	m_mark		: 1;    			
      bool	m_holelink : 1; 
      
      bool m_merge_L 	: 1;				
      bool m_a_substract_b_L: 1;	  		
      bool m_b_substract_a_L: 1;	  		
      bool m_intersect_L: 1;				
      bool m_exor_L: 1;				
      
      bool m_merge_R 	: 1;				
      bool m_a_substract_b_R: 1;	  		
      bool m_b_substract_a_R: 1;	  		
      bool m_intersect_R: 1;				
      bool m_exor_R: 1;				
      
      GroupType m_group : 1;      
      
      int m_graphnum;
      
      DL_Node<Record*>* m_record; 
};
#endif
