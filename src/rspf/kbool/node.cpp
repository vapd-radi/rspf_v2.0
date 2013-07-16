/*! \file kbool/src/node.cpp
    \brief Holds a GDSII node structure
    \author Probably Klaas Holwerda
    Copyright: 2001-2004 (C) Probably Klaas Holwerda 
    Licence: wxWidgets Licence
    RCS-ID: $Id: node.cpp 19473 2011-05-03 15:04:19Z gpotts $
*/
#ifdef __GNUG__
#pragma implementation 
#endif
#include <rspf/kbool/node.h>
#include <rspf/kbool/link.h>
#include <rspf/kbool/line.h>
#include <cmath>
#include <iostream>
Node::Node(Bool_Engine* /*GC*/) : LPoint(0,0)
{
	_linklist=new DL_List<void*>();
}
Node::Node(B_INT const X, B_INT const Y, Bool_Engine* /*GC*/) : LPoint(X,Y)
{
	_linklist=new DL_List<void*>();
}
Node::Node(LPoint* const a_point, Bool_Engine* /*GC*/) : LPoint(a_point)
{
	_linklist=new DL_List<void*>();
}
Node::Node(Node * const other, Bool_Engine* /*GC*/)
{
	_x = other->_x;
	_y = other->_y;
	_linklist=new DL_List<void*>();
}
Node& Node::operator=(const Node &other_node)
{
	_x = other_node._x;
	_y = other_node._y;
	return *this;
}
void Node::RoundInt(B_INT grid)
{
   _x=(B_INT) std::floor((_x + grid * 0.5) / grid) * grid;
   _y=(B_INT) std::floor((_y + grid * 0.5) / grid) * grid;
}
Node::~Node()
{
   if(_linklist)
   {
      delete _linklist;
      _linklist = 0;
   }
}
DL_List<void*>* Node::GetLinklist()
{
	return _linklist;
}
void Node::AddLink(KBoolLink *a_link)
{
   if(!_linklist) return;
	_linklist->insbegin(a_link);
}
KBoolLink* Node::GetIncomingLink()
{
   if(!_linklist) return 0;
	if (((KBoolLink*)_linklist->headitem())->GetEndNode() == this)
		return (KBoolLink*)_linklist->headitem();
   else
		return (KBoolLink*)_linklist->tailitem();
}
KBoolLink* Node::GetOutgoingLink()
{
   if(!_linklist) return 0;
	if (((KBoolLink*)_linklist->headitem())->GetBeginNode() == this)
		return (KBoolLink*)_linklist->headitem();
   else
		return (KBoolLink*)_linklist->tailitem();
}
int	Node::GetNumberOfLinks()
{
   if(!_linklist) return 0;
   return _linklist->count();
}
KBoolLink* Node::GetOtherLink(KBoolLink* prev)
{
   if(!_linklist) return 0;
   if (prev==(KBoolLink*)_linklist->headitem())
      return (KBoolLink*)_linklist->tailitem();
   if (prev==(KBoolLink*)_linklist->tailitem())
      return (KBoolLink*)_linklist->headitem();
   
   return NULL;
}
int Node::Merge(Node *other)
{
   if (this==other) //they are already merged dummy
      return 0;
   TDLI<KBoolLink> 	linkiter(_linklist);
   {
   }
   int Counter;
	// used to delete Iterator on other->_linklist
	// otherwise there can't be a takeover, because for takeover there can't
	// be an iterator on other->_linklist;
   {
      TDLI<KBoolLink> Iother(other->_linklist);
      KBoolLink* temp;
      Counter = Iother.count();
      Iother.tohead();
      while (!Iother.hitroot())
      {
         temp=Iother.item();
         if (temp->GetEndNode()==other)
            temp->SetEndNode(this);
         if (temp->GetBeginNode()==other)
            temp->SetBeginNode(this);
         Iother++;
      }
      linkiter.takeover(&Iother);
   }
   
   delete other;
   return Counter;
}
void Node::RemoveLink(KBoolLink *a_link)
{
   TDLI<KBoolLink> 	linkiter(_linklist);
   
   if (linkiter.toitem(a_link))	// find the link
      linkiter.remove();
}
bool Node::Simplify(Node *First, Node *Second, B_INT Marge)
{
	double distance=0;
	// The first and second point are a zero line, if so we can
	// make a line between the first and third point
	if (First->Equal(Second,Marge))
		return true;
	// Are the first and third point equal, if so
	// we can delete the second point
	if (First->Equal(this, Marge))
		return true;
	// Used tmp_link.set here, because the link may not be linked in the graph,
	// because the point of the graphs are used, after use of the line we have
	//to set the link to zero so the nodes will not be destructed by exit of the function
	KBoolLink tmp_link(_GC);
	tmp_link.Set(First,Second);
	KBoolLine tmp_line(_GC);
	tmp_line.Set(&tmp_link);
	//	If third point is on the same line which is made from the first
	// and second point then we can delete the second point
	if (tmp_line.PointOnLine(this,distance, (double) Marge) == ON_AREA)
	{
		tmp_link.Set(NULL,NULL);
		return true;
	}
	//
	//
	tmp_link.Set(Second,this);
	tmp_line.Set(&tmp_link);
	if (tmp_line.PointOnLine(First,distance, (double) Marge) == ON_AREA)
	{
		tmp_link.Set(NULL,NULL);
		return true;
	}
	tmp_link.Set(NULL,NULL);
	return false;
}
KBoolLink* Node::GetNextLink()
{
   if(!_linklist) return 0;
   int Aantal = _linklist->count();
   
   
   if (Aantal == 1)
      return NULL;
   int Marked_Counter = 0;
   KBoolLink *the_link = NULL;
   TDLI<KBoolLink> 	linkiter(_linklist);
   
   linkiter.tohead();
   while (!linkiter.hitroot())
   {
      if (linkiter.item()->IsMarked())
         Marked_Counter++;
      else
      {
         if (!the_link)
            the_link = linkiter.item();
      }
      (++linkiter);
   }
   if (Aantal - Marked_Counter != 1)
      return NULL;
   else
   {
      if (the_link->GetBeginNode() == this)
         return the_link;
      else
         return NULL;
   }
}
KBoolLink* Node::GetPrevLink()
{
   int Aantal;
   if (!_linklist)
      return 0;
   TDLI<KBoolLink> 	linkiter(_linklist);
   
   Aantal = _linklist->count();
   
   
   if (Aantal == 1)
      return NULL;
   
   int Marked_Counter = 0;
   KBoolLink *the_link = NULL;
   
   linkiter.tohead();
   while (!linkiter.hitroot())
   {
      if (linkiter.item()->IsMarked())
         Marked_Counter++;
      else
      {
         if (!the_link)
            the_link = linkiter.item();
      }
      (++linkiter);
   }
   if (Aantal - Marked_Counter != 1)
      return NULL;
   else
   {
      if (the_link->GetEndNode() == this)
         return the_link;
      else
         return NULL;
   }
}
bool Node::SameSides( KBoolLink* const prev , KBoolLink* const link, BOOL_OP operation )
{
   bool directedLeft;
   bool directedRight;
   if ( prev->GetEndNode() == this ) //forward direction
   {
      directedLeft = prev->IsMarkedLeft( operation );
      directedRight = prev->IsMarkedRight( operation );
      if ( link->GetBeginNode() == this ) //forward direction
      {
         return directedLeft == link->IsMarkedLeft( operation ) && 
                directedRight == link->IsMarkedRight( operation );
      }
      return directedLeft == link->IsMarkedRight( operation ) && 
             directedRight == link->IsMarkedLeft( operation );
   }
   directedLeft = prev->IsMarkedRight( operation );
   directedRight = prev->IsMarkedLeft( operation );
   if ( link->GetBeginNode() == this ) //forward direction
   {
      return directedLeft == link->IsMarkedLeft( operation ) && 
             directedRight == link->IsMarkedRight( operation );
   }
   return directedLeft == link->IsMarkedRight( operation ) && 
          directedRight == link->IsMarkedLeft( operation );
}
KBoolLink* Node::GetMost( KBoolLink* const prev ,LinkStatus whatside, BOOL_OP operation )
{
   KBoolLink *reserve=0;
   KBoolLink *Result = NULL,*link;
   Node* prevbegin = prev->GetOther(this);
   
   if(!_linklist) return 0;
   if (_linklist->count() == 2) // only two links to this node take the one != prev
   {
      if ( (link = (KBoolLink*)_linklist->headitem()) == prev )      //this is NOT the one to go on
         link = (KBoolLink*)_linklist->tailitem();
      if (!link->BeenHere() && SameSides( prev, link, operation ) )
         return link;
      return(0);
   }
   
   TDLI<KBoolLink> 	linkiter(_linklist);
   linkiter.tohead();
   while(!linkiter.hitroot())
   {
      link = linkiter.item();
      if ( !link->BeenHere() &&
           SameSides( prev, link, operation ) && 
           link != prev   //should be set to bin already
           )
      {
         if (prevbegin == link->GetOther(this) )//pointers equal
            reserve = link;
         else
         {  //this link is in a different direction
            if (!Result)
               Result = link; //first one found sofar
            else
            {
               if	(prev->PointOnCorner(Result, link) == whatside )
                  Result = link;
            }
         }
      }
      (++linkiter);
   }
   
   return ((Result) ? Result : reserve);
}
KBoolLink* Node::GetMostHole( KBoolLink* const prev, LinkStatus whatside, BOOL_OP operation )
{
   if(!_linklist) return 0;
   KBoolLink *reserve=0;
   KBoolLink *Result=NULL,*link;
   Node* prevbegin = prev->GetOther(this);
   
   if (_linklist->count() == 2) // only two links to this node take the one != prev
   {
      if ( (link = (KBoolLink*)_linklist->headitem()) == prev )      //this is NOT the one to go on
         link = (KBoolLink*)_linklist->tailitem();
      if ( link->GetHole() && !link->GetHoleLink() && !link->BeenHere() && SameSides( prev, link, operation ) )
         return link;
      return(0);
   }
   
   TDLI<KBoolLink> 	linkiter(_linklist);
   linkiter.tohead();
   while(!linkiter.hitroot())
   {
      link = linkiter.item();
      if ( !link->BeenHere() &&
           link->GetHole() &&
           !link->GetHoleLink() && 
           SameSides( prev, link, operation ) &&
           link != prev   //should be set to bin already
           )
      {
         if (prevbegin == link->GetOther(this) )//pointers equal
            reserve = link;
         else
         {  //this link is in a different direction
            if (!Result)
               Result = link; //first one found sofar
            else
            {
               if	(prev->PointOnCorner(Result, link) == whatside )
                  Result = link;
            }
         }
      }
      (++linkiter);
   }
   
   return ((Result) ? Result : reserve);
}
KBoolLink* Node::GetHoleLink( KBoolLink* const prev, bool checkbin, BOOL_OP operation )
{
   if(!_linklist) return 0;
	KBoolLink *Result=NULL,*link;
        TDLI<KBoolLink> 	linkiter(_linklist);
	for(linkiter.tohead();!linkiter.hitroot();++linkiter)
	{
           link = linkiter.item();
           if ( link->GetHoleLink() && 
                ( !checkbin || ( checkbin && !link->BeenHere()) ) &&
                SameSides( prev, link, operation ) 
                )
           {
              Result=link; 
              break;
           }
	}
        
	return (Result);
}
KBoolLink* Node::GetNotFlat()
{
   if(!_linklist) return 0;
   KBoolLink *Result=NULL,*link;
   TDLI<KBoolLink> 	linkiter(_linklist);
   
  double tangold = 0.0;
  double tangnew = 0.0;
  for(linkiter.tohead();!linkiter.hitroot();++linkiter)
  {
     link=linkiter.item();
     if (!linkiter.item()->BeenHere())
     {
        B_INT dx=link->GetOther(this)->GetX()-_x;
        B_INT dy=link->GetOther(this)->GetY()-_y;
        if (dx!=0)
        {
           tangnew=fabs( (double) dy / (double) dx );
        }
        else
        {
           tangnew=MAXDOUBLE;
        }
        
        if (!Result)
        {
           Result=link; //first one found sofar
           tangold=tangnew;
        }
        else
        {
           if(tangnew < tangold)
           {
              Result=link;
              tangold=tangnew;
           }
        }
     }
  }
  
  return (Result);
}
KBoolLink *Node::Follow(KBoolLink* const prev )
{
   if(!_linklist) return 0;
   KBoolLink *temp;
   TDLI<KBoolLink> 	linkiter(_linklist);
   
   linkiter.tohead();
   while(!linkiter.hitroot())
   {
      if (( linkiter.item() != prev ) &&
          ( !linkiter.item()->BeenHere()) &&
          ( linkiter.item()->GetGraphNum() == prev->GetGraphNum()) &&
          (
             ( (prev->GetEndNode()   == this) &&
               (linkiter.item()->GetEndNode()  !=this)
               )
             ||
             ( (prev->GetBeginNode() == this) &&
               (linkiter.item()->GetBeginNode() !=this)
               )
             )
          )
      {
         temp=linkiter.item();
         linkiter.Detach();
         return(temp);
      }
      (++linkiter);
   }
   
   return (0);
}
KBoolLink* Node::GetBinHighest(bool binset)
{
   if(!_linklist) return 0;
   TDLI<KBoolLink> 	linkiter(_linklist);
   KBoolLink *Result=NULL,*link;
   
   double tangold = 0.0;
   double tangnew = 0.0;
   
   for(linkiter.tohead();!linkiter.hitroot();(++linkiter))
   {
      link=linkiter.item();
      if (linkiter.item()->BeenHere() == binset)
      {
         B_INT dx=link->GetOther(this)->GetX()-_x;
         B_INT dy=link->GetOther(this)->GetY()-_y;
         if (dx!=0)
         {
            tangnew = (double) dy / (double) dx;
         }
         else if (dy > 0)
         {
            tangnew = MAXDOUBLE;
         }
         else
         {
            tangnew = -MAXDOUBLE;
         }
         
         if (!Result)
         {
            Result = link; //first one found sofar
            tangold = tangnew;
         }
         else
         {
            if(tangnew > tangold)
            {
               Result = link;
               tangold = tangnew;
            }
         }
      }
   }
   
   return (Result);
}
