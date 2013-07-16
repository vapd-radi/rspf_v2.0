/*! \file kbool/src/link.cpp
    \author Probably Klaas Holwerda
    Copyright: 2001-2004 (C) Probably Klaas Holwerda
    Licence: wxWidgets Licence
    RCS-ID: $Id: link.cpp 11347 2007-07-23 13:01:59Z gpotts $
*/
#ifdef __GNUG__
#pragma implementation 
#endif
#include <rspf/kbool/booleng.h>
#include <rspf/kbool/link.h>
#include <rspf/kbool/line.h>
#include <math.h>
#include <assert.h>
#include <rspf/kbool/node.h>
#include <rspf/kbool/graph.h>
#include <rspf/kbool/graphlst.h>
int linkXYsorter(KBoolLink *, KBoolLink *);
KBoolLink::KBoolLink(Bool_Engine* GC)
{
   _GC=GC;
	Reset();
}
KBoolLink::KBoolLink(int graphnr, Node *begin, Node *end, Bool_Engine* GC)
{
   _GC=GC;
	Reset();
	// Set the references of the node and of this link correct
	begin->AddLink(this);
	end->AddLink(this);
	m_beginnode = begin;
	m_endnode = end;
	m_graphnum = graphnr;
}
KBoolLink::KBoolLink(Node *begin, Node *end, Bool_Engine* GC)
{
	_GC=GC;
	Reset();
	// Set the references of the node and of this link correct
	begin->AddLink(this);
	end->AddLink(this);
	m_beginnode=begin;
	m_endnode=end;
	m_graphnum=0;
}
KBoolLink::~KBoolLink()
{
	UnLink();
}
bool KBoolLink::BeenHere()
{
   if (m_bin) return true;
	return false;
}
void KBoolLink::TakeOverOperationFlags( KBoolLink* link )
{
   m_merge_L = link->m_merge_L;
   m_a_substract_b_L = link->m_a_substract_b_L;
   m_b_substract_a_L = link->m_b_substract_a_L;
   m_intersect_L = link->m_intersect_L;
   m_exor_L = link->m_exor_L;
   m_merge_R = link->m_merge_R;
   m_a_substract_b_R = link->m_a_substract_b_R;
   m_b_substract_a_R = link->m_b_substract_a_R;
   m_intersect_R = link->m_intersect_R;
   m_exor_R = link->m_exor_R;
}
KBoolLink*	KBoolLink::Forth(Node *node)
{
	assert(node==m_beginnode || node==m_endnode);
	return node->GetOtherLink(this);
}
Node *KBoolLink::GetBeginNode()
{
	return m_beginnode;
}
Node*	KBoolLink::GetEndNode()
{
	return m_endnode;
}
Node*	KBoolLink::GetLowNode()
{
	return ( ( m_beginnode->GetY() < m_endnode->GetY() ) ? m_beginnode : m_endnode );
}
Node*	KBoolLink::GetHighNode()
{
	return ( ( m_beginnode->GetY() > m_endnode->GetY() ) ? m_beginnode : m_endnode );
}
int KBoolLink::GetGraphNum()
{
	return m_graphnum;
}
bool KBoolLink::GetInc()
{
   return m_Inc;
}
void KBoolLink::SetInc(bool inc)
{
     m_Inc = inc;
}
bool KBoolLink::GetLeftA()
{
	return m_LeftA;
}
void KBoolLink::SetLeftA(bool la)
{
	m_LeftA = la;
}
bool KBoolLink::GetLeftB()
{
	return m_LeftB;
}
void KBoolLink::SetLeftB(bool lb)
{
	m_LeftB = lb;
}
bool KBoolLink::GetRightA()
{
	return m_RightA;
}
void KBoolLink::SetRightA(bool ra)
{
	m_RightA = ra;
}
bool KBoolLink::GetRightB()
{
	return m_RightB;
}
void KBoolLink::SetRightB(bool rb)
{
	m_RightB = rb;
}
Node* KBoolLink::GetOther(const Node *const a)
{
	return ( (a != m_beginnode) ? m_beginnode : m_endnode);
}
bool KBoolLink::IsMarked(BOOL_OP operation)
{
   switch (operation)
   {
      case(BOOL_OR): 	   return m_merge_L || m_merge_R;
      case(BOOL_AND):     return m_intersect_L || m_intersect_R;
      case(BOOL_A_SUB_B): return m_a_substract_b_L || m_a_substract_b_R;
      case(BOOL_B_SUB_A): return m_b_substract_a_L || m_b_substract_a_R;
      case(BOOL_EXOR):    return m_exor_L || m_exor_R;
      default:             return false;
   }
}
bool KBoolLink::IsMarkedLeft(BOOL_OP operation)
{
   switch (operation)
   {
      case(BOOL_OR):      return m_merge_L;
      case(BOOL_AND):     return m_intersect_L;
      case(BOOL_A_SUB_B): return m_a_substract_b_L;
      case(BOOL_B_SUB_A): return m_b_substract_a_L;
      case(BOOL_EXOR):    return m_exor_L;
      default:             return false;
   }
}
bool KBoolLink::IsMarkedRight(BOOL_OP operation)
{
   switch (operation)
   {
      case(BOOL_OR):      return m_merge_R;
      case(BOOL_AND):     return m_intersect_R;
      case(BOOL_A_SUB_B): return m_a_substract_b_R;
      case(BOOL_B_SUB_A): return m_b_substract_a_R;
      case(BOOL_EXOR):    return m_exor_R;
      default:             return false;
   }
}
bool KBoolLink::IsHole(BOOL_OP operation)
{
   bool topsideA,topsideB;
   
   if (m_beginnode->GetX() < m_endnode->GetX()) //going to the right?
   { 	topsideA = m_RightA; topsideB = m_RightB;  }
   else
   {  topsideA = m_LeftA; topsideB = m_LeftB; }
   
   switch (operation)
   {
      case(BOOL_OR):      return ( !topsideB && !topsideA );
      case(BOOL_AND):     return ( !topsideB || !topsideA );
      case(BOOL_A_SUB_B): return ( topsideB || !topsideA );
      case(BOOL_B_SUB_A): return ( topsideA || !topsideB );
      case(BOOL_EXOR):    return !( (topsideB && !topsideA) || (!topsideB && topsideA) );
      default:             return false;
   }
}
bool KBoolLink::GetHole()
{
	return (m_hole);
}
void KBoolLink::SetHole(bool h)
{
	m_hole = h;
}
bool KBoolLink::IsUnused()
{
	return 
            !(m_merge_L || m_merge_R ||
              m_a_substract_b_L || m_a_substract_b_R ||
              m_b_substract_a_L || m_b_substract_a_R ||
              m_intersect_L || m_intersect_R ||
              m_exor_L || m_exor_R );
}
bool KBoolLink::IsZero(B_INT marge)
{
	return (m_beginnode->Equal(m_endnode,marge)) ;
}
bool KBoolLink::ShorterThan(B_INT marge)
{
	return (m_beginnode->ShorterThan(m_endnode,marge)) ;
}
void KBoolLink::Mark()
{
	m_mark = true;
}
void KBoolLink::MergeNodes(Node *const begin_or_end_node)
{
	m_beginnode->RemoveLink(this);
	m_endnode->RemoveLink(this);
	if (m_endnode != m_beginnode)
	{	// only if beginnode and endnode are different nodes
		begin_or_end_node->Merge(GetOther(begin_or_end_node));
	}
	m_endnode = NULL;
	m_beginnode=NULL;
}
LinkStatus KBoolLink::OutProduct(KBoolLink* const two,double accur)
{
	Node*	center;
	double distance;
	if (two->GetBeginNode()->Equal(two->GetEndNode(), 1))
		assert(!two);
	if (GetBeginNode()->Equal(GetEndNode(), 1))
		assert(!this);
	KBoolLine* temp_line = new KBoolLine(this, _GC);
	//the this link should connect to the other two link at at least one node
	if (m_endnode == two->m_endnode || m_endnode == two->m_beginnode)
		center = m_endnode;
	else
	{	center = m_beginnode;
	}
	//here something tricky
	// the factor 10000.0 is needed to asure that the pointonline
	// is more accurate in this case compared to the intersection for graphs
	int uitp = temp_line->PointOnLine(two->GetOther(center), distance, accur);
	delete temp_line;
	/*double uitp=	(_x - first._x) * (third._y - _y) -
					(_y - first._y) * (third._x - _x);
	if (uitp>0) return IS_LEFT;
	if (uitp<0) return IS_RIGHT;
	return IS_ON;*/
	//depending on direction of this link (going to or coming from centre)
	if (center == m_endnode)
	{
	  if (uitp==LEFT_SIDE)
			return IS_LEFT;
	  if (uitp==RIGHT_SIDE)
			return IS_RIGHT;
	}
	else  //center=beginnode
	{
	  if (uitp==LEFT_SIDE)
			return IS_RIGHT;
	  if (uitp==RIGHT_SIDE)
			return IS_LEFT;
	}
	return IS_ON;
}
LinkStatus KBoolLink::PointOnCorner(KBoolLink* const two, KBoolLink* const third)
{
	LinkStatus
	TwoToOne,		// Position of two to this line
	ThirdToOne,    // Position of third to this line
	ThirdToTwo,		// Position of third to two
	Result;
	// Calculate the position of the links compared to eachother
	TwoToOne	 = OutProduct(two,_GC->GetAccur());
	ThirdToOne= OutProduct(third,_GC->GetAccur());
	//center is used in outproduct to give de direction of two
	// this is why the result should be swapped
	ThirdToTwo=	two->OutProduct(third,_GC->GetAccur());
	if (ThirdToTwo==IS_RIGHT)
	  ThirdToTwo=IS_LEFT;
	else if (ThirdToTwo==IS_LEFT)
	  ThirdToTwo=IS_RIGHT;
	// Select the result
	switch(TwoToOne)
	{
		// Line 2 lies on  leftside of this line
		case IS_LEFT : if ((ThirdToOne==IS_RIGHT) || (ThirdToTwo==IS_RIGHT))	return IS_RIGHT;
							else if ((ThirdToOne==IS_LEFT) && (ThirdToTwo==IS_LEFT))	return IS_LEFT;
							else	Result=IS_ON;	break;
		// Line 2 lies on this line
		case IS_ON	 :	if ((ThirdToOne==IS_RIGHT) && (ThirdToTwo==IS_RIGHT))		  return IS_RIGHT;
							else if ((ThirdToOne==IS_LEFT) && (ThirdToTwo==IS_LEFT))   return IS_LEFT;
					  //		else if ((ThirdToOne==IS_RIGHT) && (ThirdToTwo==IS_LEFT))   return IS_RIGHT;
					  //		else if ((ThirdToOne==IS_LEFT) && (ThirdToTwo==IS_RIGHT))   return IS_LEFT;
							else	Result=IS_ON;	break;
		// Line 2 lies on right side of this line
		case IS_RIGHT :if ((ThirdToOne==IS_RIGHT) && (ThirdToTwo==IS_RIGHT))	return IS_RIGHT;
							else if ((ThirdToOne==IS_LEFT) || (ThirdToTwo==IS_LEFT)) return IS_LEFT;
							else	Result=IS_ON;	break;
    default: Result = IS_ON; assert( false );
	}
	return Result;
}
void KBoolLink::Remove(Node *a_node)
{
	(m_beginnode == a_node) ? m_beginnode = NULL : m_endnode = NULL;
}
void KBoolLink::Replace(Node *oldnode, Node *newnode)
{	
   if (m_beginnode == oldnode)
	{	m_beginnode->RemoveLink(this);	// remove the reference to this
		newnode->AddLink(this);       // let newnode refer to this
		m_beginnode = newnode;				// let this refer to newnode
	}
	else
	{	//assert(endnode==oldnode);
		m_endnode->RemoveLink(this);
		newnode->AddLink(this);
		m_endnode = newnode;
	}
}
void KBoolLink::Reset()
{
	m_beginnode = 0;
	m_endnode = 0;
	Reset_flags();
}
void KBoolLink::Reset_flags()
{
	m_bin	= false; 			// Marker for walking over the graph
	m_hole  = false;			// Is this a part of hole ?
	m_hole_top = false;	   // link that is toplink of hole?
	m_group = GROUP_A;		// Does this belong to group A or B ( o.a. for boolean operations between graphs)
	m_LeftA = false;     	// Is left in polygongroup A
	m_RightA= false;     	// Is right in polygon group A
	m_LeftB = false;     	// Is left in polygon group B
	m_RightB= false;     	// Is right in polygongroup B
	m_mark	= false;     	// General purose marker, internally unused
   m_holelink=false;
	m_merge_L = m_merge_R = false;			// Marker for Merge
	m_a_substract_b_L = m_a_substract_b_R = false;	// Marker for substract
	m_b_substract_a_L = m_b_substract_a_R = false;	// Marker for substract
	m_intersect_L = m_intersect_R = false;		// Marker for intersect
	m_exor_L	= m_exor_R= false;          // Marker for Exor
}
void KBoolLink::Reset(Node *begin, Node *end,int graphnr)
{
	// Remove all the previous references
	UnLink();
	Reset();
	// Set the references of the node and of this link correct
	begin->AddLink(this);
	end->AddLink(this);
	m_beginnode = begin;
	m_endnode = end;
	if (graphnr!=0)
		 m_graphnum = graphnr;
}
void KBoolLink::Set(Node *begin, Node *end)
{
	m_beginnode = begin;
	m_endnode = end;
}
void KBoolLink::SetBeenHere()
{
	m_bin = true;
}
void KBoolLink::SetNotBeenHere()
{
	m_bin = false;
}
void KBoolLink::SetBeginNode(Node* new_node)
{
	m_beginnode = new_node;
}
void KBoolLink::SetEndNode(Node* new_node)
{
	m_endnode = new_node;
}
void KBoolLink::SetGraphNum( int num )
{
	m_graphnum=num;
}
GroupType KBoolLink::Group()
{
	return m_group;
}
void KBoolLink::SetGroup(GroupType groep)
{
	m_group= groep;
}
void KBoolLink::UnLink()
{
   if (m_beginnode)
   {
      m_beginnode->RemoveLink(this);
      if (!m_beginnode->GetNumberOfLinks())
      {
         delete m_beginnode;
      }
   }
   m_beginnode = 0;
   if (m_endnode)
   {
      m_endnode->RemoveLink(this);
      if (!m_endnode->GetNumberOfLinks())
      {
         delete m_endnode;
      }
   }
   m_endnode = 0;
}
void KBoolLink::UnMark()
{
	m_mark = false;
	m_bin = false;
}
void KBoolLink::SetMark(bool value)
{
	m_mark = value;
}
bool KBoolLink::IsMarked() { return m_mark; }
void  KBoolLink::SetTopHole(bool value) {	m_hole_top = value; }
bool KBoolLink::IsTopHole() {	return m_hole_top; }
void	KBoolLink::SetLineTypes()
{
	m_merge_R     = 
	m_a_substract_b_R = 
	m_b_substract_a_R = 
	m_intersect_R = 
	m_exor_R      = 
	m_merge_L     = 
	m_a_substract_b_L = 
	m_b_substract_a_L = 
	m_intersect_L = 
	m_exor_L      = false;
	m_merge_L   = m_LeftA || m_LeftB;  											
	m_merge_R   = m_RightA || m_RightB;
   if (m_merge_L && m_merge_R)
      m_merge_L = m_merge_R = false;
   
   m_a_substract_b_L = m_LeftA && !m_LeftB;  
   m_a_substract_b_R = m_RightA && !m_RightB;
   if (m_a_substract_b_L && m_a_substract_b_R)
      m_a_substract_b_L = m_a_substract_b_R = false;
   m_b_substract_a_L = m_LeftB && !m_LeftA;  
   m_b_substract_a_R = m_RightB && !m_RightA;
   if (m_b_substract_a_L && m_b_substract_a_R)
      m_b_substract_a_L = m_b_substract_a_R = false;
	m_intersect_L = m_LeftB && m_LeftA;
	m_intersect_R = m_RightB && m_RightA;
   if (m_intersect_L && m_intersect_R)
      m_intersect_L = m_intersect_R = false;
	m_exor_L = !( (m_LeftB && m_LeftA) || (!m_LeftB && !m_LeftA) );
	m_exor_R = !( (m_RightB && m_RightA) || (!m_RightB && !m_RightA) );
   if (m_exor_L && m_exor_R)
      m_exor_L = m_exor_R = false;
}
void  KBoolLink::Redirect(Node* a_node)
{
   if (a_node != m_beginnode)
   {
		// swap the begin- and endnode of the current link
		Node* dummy = m_beginnode;
		m_beginnode = m_endnode;
		m_endnode = dummy;
      bool swap = m_LeftA;
	   m_LeftA = m_RightA;     	
	   m_RightA= swap;
      swap = m_LeftB;
	   m_LeftB = m_RightB;     	
	   m_RightB= swap;
      
      swap = m_merge_L ;
	   m_merge_L = m_merge_R; 
	   m_merge_R = swap; 
      swap = m_a_substract_b_L;
	   m_a_substract_b_L = m_a_substract_b_R; 
	   m_a_substract_b_R = swap; 
      swap = m_b_substract_a_L;
	   m_b_substract_a_L = m_b_substract_a_R; 
	   m_b_substract_a_R = swap; 
      swap = m_intersect_L;
	   m_intersect_L = m_intersect_R; 
	   m_intersect_R = swap; 
      swap = m_exor_L;
	   m_exor_L = m_exor_R; 
	   m_exor_R = swap; 
   }
}
