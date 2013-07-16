/*! \file kbool/src/graph.cpp
    \brief Used to Intercect and other process functions
    \author Probably Klaas Holwerda 
    Copyright: 2001-2004 (C) Probably Klaas Holwerda
    Licence: wxWidgets Licence
    RCS-ID: $Id: graph.cpp 17606 2010-06-21 20:46:16Z dburken $
*/
#ifdef __GNUG__
#pragma implementation 
#endif
#include <rspf/base/rspfErrorContext.h>
#include <math.h>
#include <assert.h>
#include <rspf/kbool/booleng.h>
#include <rspf/kbool/graph.h>
#include <rspf/kbool/graphlst.h>
#include <rspf/kbool/node.h>
int linkXYsorter(KBoolLink *, KBoolLink *);
int linkYXsorter(KBoolLink *, KBoolLink *);
int linkLsorter(KBoolLink *, KBoolLink *);
int linkYXtopsorter(KBoolLink *a, KBoolLink *b);
int linkGraphNumsorter(KBoolLink *_l1, KBoolLink* _l2);
Graph::Graph(KBoolLink *a_link, Bool_Engine* GC )
{
   _GC = GC;
	_linklist=new DL_List<void*>();
	_linklist->insbegin(a_link);
	_bin = false;
}
Graph::Graph(Bool_Engine* GC)
{
   _GC = GC;
	_linklist=new DL_List<void*>();
	_bin = false;
}
Graph::Graph( Graph* other )
{
   _GC = other->_GC;
	_linklist = new DL_List<void*>();
	_bin = false;
	int _nr_of_points = other->_linklist->count();
	KBoolLink* _current = other->GetFirstLink();
	Node* _last = _current->GetBeginNode();
	Node* node = new Node( _current->GetBeginNode()->GetX(), _current->GetBeginNode()->GetY(), _GC );
	Node* nodefirst = node;
	for (int i = 0; i < _nr_of_points; i++)
	{
		// get the other node on the link
		_last = _current->GetOther(_last);
		// get the other link on the _last node
		_current = _current->Forth(_last);
   	Node* node2 = new Node( _current->GetBeginNode()->GetX(), _current->GetBeginNode()->GetY(), _GC );
      _linklist->insend( new KBoolLink( node,  node2, _GC ) );
      node = node2;
	}
   _linklist->insend( new KBoolLink( node,  nodefirst, _GC ) );
}
Graph::~Graph()
{
   if(_linklist)
   {
      {
         TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
         
         _LI.delete_all();
      }
      delete _linklist;
      _linklist = 0;
   }
   _GC = 0;
}
KBoolLink*	Graph::GetFirstLink()
{
   if(!_linklist) return 0;
   return (KBoolLink*) _linklist->headitem();
};
void Graph::Prepare( int intersectionruns )
{
   if(!_GC) return;
   _GC->SetState("Intersection");
   bool found = true;
   int run = 0;
   while( run < intersectionruns && found )
   {    
	   found = CalculateCrossings(_GC->GetInternalMarge());
      run++;
   }
   {
   	TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
		_LI.foreach_mf(&KBoolLink::UnMark);// Reset Bin and Mark flag
   }
   _GC->SetState("Set group A/B Flags");
   bool dummy = false;
   if (_GC->GetWindingRule())
	   ScanGraph2( INOUT, dummy );
   ScanGraph2( GENLR, dummy );
	_GC->SetState("Set operation Flags");
	Set_Operation_Flags();
	_GC->SetState("Remove doubles");
	// Remove the marked links
   {
   	TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
      _LI.tohead();
      while(!_LI.hitroot())
      {
         if (_LI.item()->IsMarked())
         {
            delete _LI.item();
            _LI.remove();
         }
         else
            _LI++;
      }
   }
	_GC->SetState("Remove inlinks");
	Remove_IN_Links();
	_GC->SetState("Finished prepare graph");
}
void Graph::RoundInt(B_INT grid)
{
   TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	_LI.tohead();
	while (!_LI.hitroot())
	{
			_LI.item()->GetBeginNode()->RoundInt(grid);
			_LI.item()->GetEndNode()->RoundInt(grid);
			_LI++;
	}
}
void Graph::Rotate(bool plus90)
{
	B_INT swap;
	Node* last=NULL;
   B_INT neg=-1;
   if (plus90)
      neg=1;
   TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	_LI.mergesort(linkXYsorter);
	_LI.tohead();
	while (!_LI.hitroot())
	{
			if (_LI.item()->GetBeginNode() != last)
			{
				swap=_LI.item()->GetBeginNode()->GetX();
				_LI.item()->GetBeginNode()->SetX(-neg*(_LI.item()->GetBeginNode()->GetY()));
				_LI.item()->GetBeginNode()->SetY(neg*swap);
				last=_LI.item()->GetBeginNode();
			}
			_LI++;
	}
}
bool Graph::RemoveNullLinks()
{
   bool graph_is_modified = false;
   
   TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
   _LI.tohead();
   while (!_LI.hitroot())
   {
      if (_LI.item()->GetBeginNode() == _LI.item()->GetEndNode())
      {
         _LI.item()->MergeNodes(_LI.item()->GetBeginNode());
         delete _LI.item();
         _LI.remove();
         graph_is_modified = true;
      }
      else
         _LI++;
   }
   return (graph_is_modified);
}
void Graph::AddLink(KBoolLink *a_link)
{
   if(!_linklist) return;
   
   assert(a_link);
   
   _linklist->insend(a_link);
}
void Graph::AddLink(Node *begin, Node *end)
{
   if(!(begin&&end))
   {
      rspfNotify(rspfNotifyLevel_WARN) << "Graph::AddLink WARING: line " << __LINE__ << " begin or end are NULL" << std::endl;
      return;
   }
   if(begin==end)
   {
      rspfNotify(rspfNotifyLevel_WARN) << "Graph::AddLink WARING: line " << __LINE__ << " begin and end are equal" << std::endl;
      return;
   }
   AddLink(new KBoolLink(0, begin, end, _GC));
}
bool Graph::AreZeroLines(B_INT Marge)
{
	bool Found_it = false;
   TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	_LI.tohead();
	while (!_LI.hitroot())
	{
		if (_LI.item()->IsZero(Marge))
		{
			Found_it = true;
			break;
		}
		_LI++;
	}
	return Found_it;
}
void Graph::DeleteNonCond(BOOL_OP operation)
{
   TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	_LI.tohead();
	while(!_LI.hitroot())
	{
		if ( !_LI.item()->IsMarked(operation))
		{
			delete _LI.item();
			_LI.remove();
		}
		else
			_LI++;
	}
}
void Graph::HandleNonCond(BOOL_OP operation)
{
   TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	_LI.tohead();
	while(!_LI.hitroot())
	{
		if ( !_LI.item()->IsMarked(operation))
      {
         _LI.item()->SetBeenHere();
         _LI.item()->SetGraphNum( -1 );
      }   
   	_LI++;
	}
}
bool Graph::DeleteZeroLines(B_INT Marge)
{
	// Is the graph modified ?
	bool Is_Modified = false;
   TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	int Processed = _LI.count();
	_LI.tohead();
	while (Processed > 0)
	{
		Processed--;
		if (_LI.item()->IsZero(Marge))
		{
			// the current link is zero, so make from both nodes one node
			// and delete the current link from this graph
			_LI.item()->MergeNodes(_LI.item()->GetBeginNode());
			// if an item is deleted the cursor of the list is set to the next
			// so no explicit forth is needed
			delete _LI.item();
			_LI.remove();
			// we have to set Processed, because if a zero line is deleted
			// another can be made zero by this deletion
			Processed = _LI.count();
			Is_Modified = true;
			if (_LI.hitroot())
				_LI.tohead();
		}
		else
			_LI++;
			if (_LI.hitroot())
				_LI.tohead();
	}
	return Is_Modified;
}
void Graph::CollectGraph(Node *current_node,BOOL_OP operation, bool detecthole,int graphnumber, bool& foundholes )
{
	KBoolLink *currentlink;
	KBoolLink *nextlink;
	Node *next_node;
	Node *MyFirst;
	Node *Unlinked;
	KBoolLink *MyFirstlink;
	bool Hole;
	LinkStatus whatside;
	currentlink=current_node->GetNotFlat();
	if (!currentlink)
   {
      char buf[100];
      if (detecthole)
	      sprintf(buf,"no NON flat link Collectgraph for operation at %15.3lf , %15.3lf",
         				double(current_node->GetX()),double(current_node->GetY()));
      else
	      sprintf(buf,"no NON flat link Collectgraph at %15.3lf , %15.3lf",
         				double(current_node->GetX()),double(current_node->GetY()));
		throw Bool_Engine_Error(buf, "Error", 9, 0);
	}
	currentlink->SetBeenHere();
   if (detecthole)
		Hole = currentlink->IsHole(operation);
   else
      Hole = currentlink->GetHole(); //simple extract do not detect holes, but use hole flag.
   currentlink->Redirect(current_node);
   foundholes = Hole || foundholes;
	//depending if we have a hole or not
	//we take the left node or right node from the selected link (currentlink)
	//this MEANS for holes go left around and for non holes go right around
	if (Hole)
	{
      whatside = IS_LEFT;
	   if ( currentlink->GetEndNode()->GetX() > current_node->GetX())
		   current_node=currentlink->GetEndNode();
	}
	else
	{
      whatside = IS_RIGHT;
	   if ( currentlink->GetEndNode()->GetX() < current_node->GetX())
			current_node=currentlink->GetEndNode();
	}
   currentlink->Redirect(current_node);
   MyFirst=current_node; //remember this as the start
   MyFirstlink=currentlink;
	next_node = currentlink->GetEndNode();
	// If this is a hole, Set as special link, this is the top link of this hole !
	// from this link we have to make links to the link above later on.
	if (Hole)
		currentlink->SetTopHole(true);
   if (detecthole)
		currentlink->SetHole(Hole);
	currentlink->SetGraphNum(graphnumber);
	// Walk over links and redirect them. taking most right links around
	while (currentlink != NULL)
	{
      if ( Hole )
      {
         nextlink = next_node->GetMost(currentlink, IS_RIGHT, operation);
      }
      else
      {
         nextlink = next_node->GetMost(currentlink, IS_LEFT, operation);
      }
		if (nextlink == NULL)
		{	//END POINT MUST BE EQAUL TO BEGIN POINT
			if (!next_node->Equal(MyFirst, 1))
			{
            throw Bool_Engine_Error("no next (endpoint != beginpoint)","graph", 9, 0);
				  //nextlink = next_node->GetMost(currentlink, whatside ,operation);
			}
		}
		current_node = next_node;
		if (nextlink!=NULL)
		{
		   nextlink->Redirect(current_node);
			nextlink->SetBeenHere();
			next_node = nextlink->GetEndNode();
         if ( current_node->GetNumberOfLinks() > 2)
         {  // replace endnode of currentlink and beginnode of nextlink with new node
            Unlinked = new Node(current_node, _GC);
				currentlink->Replace(current_node,Unlinked);
				nextlink->Replace(current_node,Unlinked);
         }
		   if (detecthole)
				nextlink->SetHole(Hole);
			nextlink->SetGraphNum(graphnumber);
		}
      else
      {
         if ( current_node->GetNumberOfLinks() > 2)
         {  // replace endnode of currentlink and beginnode of nextlink with new node
            Unlinked = new Node(current_node, _GC);
            currentlink->Replace(current_node,Unlinked);
            MyFirstlink->Replace(current_node,Unlinked);
         }
      }
		currentlink = nextlink;
	}
	//END POINT MUST BE EQAUL TO BEGIN POINT
	if (!current_node->Equal(MyFirst, 1))
   {
      throw Bool_Engine_Error("in collect graph endpoint != beginpoint", "Error", 9, 0);
   }
}
void Graph::CollectGraphLast(Node *current_node,BOOL_OP operation, bool detecthole,int graphnumber, bool& foundholes )
{
	KBoolLink *currentlink;
	KBoolLink *nextlink;
	Node *next_node;
	Node *MyFirst;
	Node *Unlinked;
	KBoolLink *MyFirstlink;
	bool Hole;
	LinkStatus whatside;
	currentlink=current_node->GetNotFlat();
	if (!currentlink)
   {
      char buf[100];
      if (detecthole)
	      sprintf(buf,"no NON flat link Collectgraph for operation at %15.3lf , %15.3lf",
         				double(current_node->GetX()),double(current_node->GetY()));
      else
	      sprintf(buf,"no NON flat link Collectgraph at %15.3lf , %15.3lf",
         				double(current_node->GetX()),double(current_node->GetY()));
		throw Bool_Engine_Error(buf, "Error", 9, 0);
	}
	currentlink->SetBeenHere();
   if (detecthole)
		Hole = currentlink->IsHole(operation);
   else
      Hole = currentlink->GetHole(); //simple extract do not detect holes, but use hole flag.
   currentlink->Redirect(current_node);
   foundholes = Hole || foundholes;
	//depending if we have a hole or not
	//we take the left node or right node from the selected link (currentlink)
	//this MEANS for holes go left around and for non holes go right around
	if (Hole)
	{
      whatside = IS_LEFT;
	   if ( currentlink->GetEndNode()->GetX() > current_node->GetX())
		   current_node=currentlink->GetEndNode();
	}
	else
	{
      whatside = IS_RIGHT;
	   if ( currentlink->GetEndNode()->GetX() < current_node->GetX())
			current_node=currentlink->GetEndNode();
	}
   currentlink->Redirect(current_node);
   MyFirst=current_node; //remember this as the start
   MyFirstlink=currentlink;
	next_node = currentlink->GetEndNode();
	// If this is a hole, Set as special link, this is the top link of this hole !
	// from this link we have to make links to the link above later on.
	if (Hole)
		currentlink->SetTopHole(true);
	currentlink->SetGraphNum(graphnumber);
	// Walk over links and redirect them. taking most right links around
	while (currentlink != NULL)
	{
      if ( Hole )
      {
         if ( currentlink->GetHoleLink() )
         {
            nextlink = next_node->GetMostHole(currentlink, IS_RIGHT ,operation );
            if ( !nextlink ) // hole done?
   		      nextlink = next_node->GetHoleLink(currentlink, true, operation );
            if ( !nextlink )
            {
               nextlink = next_node->GetMost(currentlink, IS_RIGHT, operation);
            }
         }
         else
         {
 		      nextlink = next_node->GetHoleLink(currentlink, true, operation ); // other linked holes first
            if ( !nextlink ) 
               nextlink = next_node->GetMostHole(currentlink, IS_RIGHT, operation ); // other holes first
            if ( !nextlink ) 
            {
               nextlink = next_node->GetMost(currentlink, IS_RIGHT, operation);
            }
         }
      }
      else
      {
         nextlink = next_node->GetHoleLink(currentlink, true, operation );
         if ( !nextlink )
            nextlink = next_node->GetMostHole(currentlink, IS_RIGHT, operation);
         if ( !nextlink )
            nextlink = next_node->GetMost(currentlink, IS_LEFT, operation);
      }
		if (nextlink == NULL)
		{	//END POINT MUST BE EQAUL TO BEGIN POINT
			if (!next_node->Equal(MyFirst, 1))
			{
            throw Bool_Engine_Error("no next (endpoint != beginpoint)","graph", 9, 0);
				    //nextlink = next_node->GetMost(currentlink, whatside, operation);
			}
		}
      else
      {
         Hole = nextlink->GetHole() || nextlink->GetHoleLink();
      }
		current_node = next_node;
		if (nextlink!=NULL)
		{
		   nextlink->Redirect(current_node);
			nextlink->SetBeenHere();
			next_node = nextlink->GetEndNode();
         if ( current_node->GetNumberOfLinks() > 2)
         {  // replace endnode of currentlink and beginnode of nextlink with new node
            Unlinked = new Node(current_node, _GC);
				currentlink->Replace(current_node,Unlinked);
				nextlink->Replace(current_node,Unlinked);
         }
			nextlink->SetGraphNum(graphnumber);
		}
      else
      {
         if ( current_node->GetNumberOfLinks() > 2)
         {  // replace endnode of currentlink and beginnode of nextlink with new node
            Unlinked = new Node(current_node, _GC);
            currentlink->Replace(current_node,Unlinked);
            MyFirstlink->Replace(current_node,Unlinked);
         }
      }
		currentlink = nextlink;
	}
	//END POINT MUST BE EQAUL TO BEGIN POINT
	if (!current_node->Equal(MyFirst, 1))
   {
      throw Bool_Engine_Error("in collect graph endpoint != beginpoint", "Error", 9, 0);
   }
}
void Graph::Extract_Simples(BOOL_OP operation, bool detecthole, bool& foundholes )
{
	TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	if (_LI.empty()) return;
	Node *begin;
   int graphnumber=1;
	_LI.mergesort(linkYXtopsorter);
   _LI.tohead();
	while (true)
	{
		begin = GetMostTopLeft(&_LI); // from all the most Top nodes,
												// take the most left one
												// to most or not to most, that is the question
		if (!begin)
			break;
		try // collect the graph
		{
         if ( detecthole )
				CollectGraph( begin,operation,detecthole,graphnumber++, foundholes );
         else 
				//CollectGraph( begin,operation,detecthole,graphnumber++, foundholes );
				CollectGraphLast( begin,operation,detecthole,graphnumber++, foundholes );
		}
		catch (Bool_Engine_Error& error)
		{
			_GC->info(error.GetErrorMessage(), "error");
			throw error;
		}
	}
}
void Graph::Split(GraphList* partlist)
{
  TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
  if (_LI.empty()) return;
  Graph *part = NULL;
  int graphnumber=0;
  _LI.mergesort(linkGraphNumsorter);
  _LI.tohead();
  while (!_LI.hitroot())
  {
    if ( _LI.item()->GetGraphNum() > 0 && graphnumber != _LI.item()->GetGraphNum())
    {
      graphnumber=_LI.item()->GetGraphNum();
      part = new Graph(_GC);
      partlist->insend(part);
    }
    KBoolLink* tmp=_LI.item();
    if ( _LI.item()->GetGraphNum() > 0 )
    {
      part->AddLink(tmp);
    }
    else
    {
      delete tmp; 
    }
    _LI.remove();
	}
}
bool Graph::GetBeenHere()
{
	return _bin;
}
int Graph::GetNumberOfLinks()
{
   TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	return _LI.count();
}
void Graph::Set_Operation_Flags()
{
   TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	_LI.tohead();
	while(!_LI.hitroot())
	{
	  _LI.item()->SetLineTypes();
	  _LI++;
	}
}
void Graph::Remove_IN_Links()
{
   TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	_LI.tohead();
	for (int t = _LI.count() ; t > 0; t--)
	{
		// Is this link not used for any operation?
		if (_LI.item()->IsUnused())
		{
			delete _LI.item();
			_LI.remove();
		}
		else
			_LI++;
	}
}
void Graph::ResetBinMark()
{
	TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	if (_LI.empty()) return;
	_LI.foreach_mf(&KBoolLink::UnMark);//reset bin and mark flag of each link
}
void Graph::ReverseAllLinks()
{
	Node*dummy;
   TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	_LI.tohead();
	while (!_LI.hitroot())
	{
		// swap the begin- and endnode of the current link
		dummy = _LI.item()->GetBeginNode();
		_LI.item()->SetBeginNode(_LI.item()->GetEndNode());
		_LI.item()->SetEndNode(dummy);
		_LI++;
	}
}
void Graph::SetBeenHere(bool value)
{
	_bin=value;
}
void Graph::Reset_flags()
{
   TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	_LI.foreach_mf(&KBoolLink::Reset_flags);
}
void Graph::Reset_Mark_and_Bin()
{
   TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	_LI.foreach_mf(&KBoolLink::UnMark);//reset bin and mark flag of each link
}
void Graph::SetGroup(GroupType newgroup)
{
   TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	_LI.tohead();
	while (!_LI.hitroot())
	{
		_LI.item()->SetGroup(newgroup);
		_LI++;
	}
}
void Graph::SetNumber(int newnr)
{
   TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	_LI.tohead();
	while (!_LI.hitroot())
	{
		_LI.item()->SetGraphNum(newnr);
		_LI++;
	}
}
bool Graph::Simplify( B_INT Marge )
{
	bool graph_is_modified = false;
   TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	int Processed = _LI.count();
	_LI.foreach_mf(&KBoolLink::UnMark);//reset bin and mark flag of each link
	_LI.tohead();
	GroupType mygroup=_LI.item()->Group();
	// All items must be processed
	while (Processed > 0)
	{
		// Gives the number of items to process
		Processed--;
		// Check if line is marked
		// Links will be marked during the process
		if (_LI.item()->IsMarked())
		{
			delete _LI.item();
			_LI.remove();
			graph_is_modified = true;
			Processed = _LI.count();
			if (_LI.hitroot())
				_LI.tohead();
			continue;
		}
		// Line is not marked, check if line is zero
		if (_LI.item()->IsZero(Marge))
		{
			_LI.item()->MergeNodes(_LI.item()->GetBeginNode());
			delete _LI.item();
			_LI.remove();
			graph_is_modified = true;
			Processed = _LI.count();
			if (_LI.hitroot())
				_LI.tohead();
			continue;
		}
		// begin with trying to simplify the link
		{
			// Line is not marked, not zero, so maybe it can be simplified
			bool virtual_link_is_modified;
			Node*new_begin, *new_end, *a_node;
			KBoolLink *a_link;
			_LI.item()->Mark();
			new_begin = _LI.item()->GetBeginNode();
			new_end   = _LI.item()->GetEndNode();
			// while virtual link is modified
			do
			{
				virtual_link_is_modified = false;
				// look in the previous direction
				if ((a_link = new_begin->GetPrevLink()) != NULL)
				{
					a_node = a_link->GetBeginNode();
					if (a_node->Simplify(new_begin,new_end,Marge))
					{
						new_begin->GetPrevLink()->Mark();
						new_begin = a_node;
						virtual_link_is_modified = true;
					}
				}
				// look in the next direction
				if ((a_link = new_end->GetNextLink()) != NULL)
				{
					a_node = a_link->GetEndNode();
					if (a_node->Simplify(new_begin,new_end,Marge))
					{
						new_end->GetNextLink()->Mark();
						new_end = a_node;
						virtual_link_is_modified = true;
					}
				}
				graph_is_modified = (bool) (graph_is_modified || virtual_link_is_modified);
			} while (virtual_link_is_modified);
			// was the link simplified
			if ((_LI.item()->GetBeginNode() != new_begin) ||
				(_LI.item()->GetEndNode() != new_end))
			{
				// YES !!!!!
				int number = _LI.item()->GetGraphNum();
				delete _LI.item();
				_LI.remove();
            if (_LI.hitroot())
               _LI.tohead();
				KBoolLink *newlink = new KBoolLink(number, new_begin, new_end, _GC);
				newlink->SetGroup(mygroup);
				_LI.insend(newlink);
				Processed = _LI.count();
				graph_is_modified = true;
				continue;
			}
			_LI.item()->UnMark();
		}	// end of try to simplify
		_LI++;
		if (_LI.hitroot())
			_LI.tohead();
	}//end while all processed
	return graph_is_modified;
}
/*
*/
bool Graph::Smoothen( B_INT Marge )
{
   TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	if (_LI.count()<=3)	// Don't modify it
		return false;
	Node*Z, *A, *B, *C, *cross_node = new Node(_GC);
	KBoolLink *prevlink, *nextlink, *thislink;
	KBoolLine	prevline(_GC),  nextline(_GC),  thisline(_GC);
	KBoolLine	prevhelp(_GC),  nexthelp(_GC);
	KBoolLink  LZB(new Node(_GC), new Node(_GC), _GC),
			LAC(new Node(_GC), new Node(_GC), _GC);
	double distance=0;
   double prevdist,nextdist;
	bool doprev, donext;
	bool graph_is_modified = false;
	bool kill = false;	// for first instance
	_LI.tohead();
	int todo = _LI.count();
	thislink=_LI.item();
	B = thislink->GetEndNode();
	nextlink = thislink->Forth(B);
	// Type 1
	while (todo>0)
	{
		if (kill==true)
		{
			// remove link from graph
			_LI.toitem(thislink);
			graph_is_modified = true;
			delete _LI.item();
			_LI.remove();
			kill=false;
			thislink=nextlink;
			todo--;
			if (_LI.count()<3)
				break;
		}
		A = thislink->GetBeginNode();
		B = thislink->GetEndNode();
		if (A->ShorterThan(B,1))
		{
			A->Merge(B);
			kill = true;
			continue;
		}
		Z = thislink->Forth(A)->GetBeginNode();
		C = thislink->Forth(B)->GetEndNode();
		thisline.Set(thislink);
		thisline.CalculateLineParameters();
		nextlink = thislink->Forth(B);
		if (thisline.PointInLine(Z,distance,0.0) == ON_AREA)
		{	// Z--A--B	=>		Z--B							Merge this to previous
			thislink->MergeNodes(B);	// remove A
			kill = true;
			continue;
		}
		else if (thisline.PointInLine(C,distance,0.0) == ON_AREA)
		{	// A--B--C	=>		A--C							Merge this to next
			thislink->MergeNodes(A);	//	remove B
			kill = true;
			continue;
		}
		thislink=nextlink;
		todo--;
	}
	kill=false;
	todo = _LI.count();
	_LI.mergesort(linkLsorter);
	_LI.tohead();
	while (todo>0)
	{
		if (kill==true)
		{
			delete _LI.item();
			_LI.remove();
			graph_is_modified = true;
			kill = false;
			//mergesort(linkLsorter);
			_LI.mergesort(linkLsorter);
			_LI.tohead();
			todo = _LI.count();
			if (todo<3)		// A polygon, at least, has 3 sides
				break;
		}
		// Keep this order!
		thislink = _LI.item();
		A = thislink->GetBeginNode();
		B = thislink->GetEndNode();
		prevlink = thislink->Forth(A);
		nextlink = thislink->Forth(B);
		Z = prevlink->GetBeginNode();
		C = nextlink->GetEndNode();
		if (A->ShorterThan(B,1))
		{
			A->Merge(B);
			kill = true;
			continue;
		}
		prevline.Set(prevlink);
		prevline.CalculateLineParameters();
		nextline.Set(nextlink);
		nextline.CalculateLineParameters();
		// Type 2
		if (B->ShorterThan(Z,Marge))
		{	// Z--A--B	=>		Z--B							Merge this to previous
			thislink->MergeNodes(B);	// remove A
			kill = true;
			continue;
		}
		else if (A->ShorterThan(C,Marge))
		{	// A--B--C	=>		A--C							Merge this to next
			thislink->MergeNodes(A);	//	remove B
			kill = true;
			continue;
		}
		*LZB.GetBeginNode()=*Z;
		*LZB.GetEndNode()=*B;
		*LAC.GetBeginNode()=*A;
		*LAC.GetEndNode()=*C;
		prevhelp.Set(&LZB);
		nexthelp.Set(&LAC);
		prevhelp.CalculateLineParameters();
		nexthelp.CalculateLineParameters();
		//	Type 4
		doprev = bool(prevhelp.PointInLine(A,prevdist,(double)Marge) == IN_AREA);
		donext = bool(nexthelp.PointInLine(B,nextdist,(double)Marge) == IN_AREA);
		doprev = bool(B->ShorterThan(Z,_GC->GetInternalMaxlinemerge()) && doprev);
		donext = bool(A->ShorterThan(C,_GC->GetInternalMaxlinemerge()) && donext);
		if (doprev && donext)
		{
			if (fabs(prevdist)<=fabs(nextdist))
				thislink->MergeNodes(B);	// remove A
			else
				thislink->MergeNodes(A);	// remove B
			kill = true;
			continue;
		}
		else if (doprev)
		{
			thislink->MergeNodes(B);	// remove A
			kill = true;
			continue;
		}
		else if (donext)
		{
			thislink->MergeNodes(A);	// remove B
			kill = true;
			continue;
		}
		thisline.Set(thislink);
		thisline.CalculateLineParameters();
		// Type 1
		if (thisline.PointInLine(Z,distance,0.0) == ON_AREA)
		{	// Z--A--B	=>		Z--B							Merge this to previous
			thislink->MergeNodes(B);	// remove A
			kill = true;
			continue;
		}
		else if (thisline.PointInLine(C,distance,0.0) == ON_AREA)
		{	// A--B--C	=>		A--C							Merge this to next
			thislink->MergeNodes(A);	//	remove B
			kill = true;
			continue;
		}
		// Type 3
		if (nextline.PointInLine(A,distance, (double) Marge)==IN_AREA)
		{
			if (nextline.Intersect2(cross_node,&prevline))
			{
				if (nextline.PointInLine(cross_node,distance,0.0)==IN_AREA)
				{
					B->Set(cross_node);
					thislink->MergeNodes(B);	// remove A
					kill=true;
					continue;
				}
				else
				{
					thislink->MergeNodes(A);	// remove B
					kill=true;
					continue;
				}
			}
			else
			{
				thislink->MergeNodes(A);	// remove B
				kill=true;
				continue;
			}
		}
		// Type 3
		if (prevline.PointInLine(B,distance,(double)Marge)==IN_AREA)
		{
			if (prevline.Intersect2(cross_node,&nextline))
			{
				if (prevline.PointInLine(cross_node,distance,0.0)==IN_AREA)
				{
					A->Set(cross_node);
					thislink->MergeNodes(A);	// remove B
					kill=true;
					continue;
				}
				else
				{
					thislink->MergeNodes(B);	// remove A
					kill=true;
					continue;
				}
			}
			else
			{
				thislink->MergeNodes(B);	// remove A
				kill=true;
				continue;
			}
		}
		todo--;
		_LI++;
	}	// end: while all processed
	delete cross_node;
	return graph_is_modified;
}
int Graph::GetGraphNum()
{
	return ((KBoolLink*)_linklist->headitem())->GetGraphNum();
}
Node* Graph::GetTopNode()
{
	B_INT max_Y = MAXB_INT;
	Node* result;
   TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	_LI.tohead();
	while (!_LI.hitroot())
	{
		if (!(_LI.item()->GetBeginNode()->GetY() < max_Y))
			 break;
		_LI++;
	}
	result = _LI.item()->GetBeginNode();
	return result;
}
Node*	Graph::GetMostTopLeft(TDLI<KBoolLink>* _LI)
{
	while (!_LI->hitroot())
	{
		if (!_LI->item()->BeenHere())
      {
         KBoolLink* a=_LI->item();
         if (a->GetBeginNode()->GetY() > a->GetEndNode()->GetY())
				return(a->GetBeginNode());
         if (a->GetBeginNode()->GetY() < a->GetEndNode()->GetY())
   			return(a->GetEndNode());
         else
   			return(a->GetBeginNode());
      }
		(*_LI)++;
	}
	return NULL;
}
void Graph::TakeOver(Graph *other)
{
   TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	_LI.takeover( other->_linklist);
}
bool Graph::CalculateCrossings(B_INT Marge)
{
	// POINT <==> POINT
	_GC->SetState("Node to Node");
   bool found = false;
   bool dummy = false;
	found = Merge_NodeToNode(Marge) != 0;
	if (_linklist->count() < 3)
		  return found;
	// POINT <==> LINK
	_GC->SetState("Node to KBoolLink 0");
   found = ScanGraph2(NODELINK, dummy) != 0 || found;
	_GC->SetState("Rotate -90");
	Rotate(false);
	_GC->SetState("Node to KBoolLink -90");
   found = ScanGraph2(NODELINK, dummy) != 0 || found;
	_GC->SetState("Rotate +90");
	Rotate(true);
	// LINK <==> LINK
	_GC->SetState("intersect");
   found = ScanGraph2(LINKLINK, dummy) != 0 || found;
/*
   if (!checksort())
   { {
	   TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
		_LI.mergesort(linkXYsorter);
     }
	  writeintersections();
	  writegraph(true);
   }
*/
   writegraph(true);
   _GC->Write_Log("Node to Node");
	_GC->SetState("Node to Node");
	found = Merge_NodeToNode(Marge) != 0 || found;
   writegraph(true);
   return found;
}
int Graph::Merge_NodeToNode(B_INT Marge)
{
	//aantal punten dat verplaatst is
	int merges = 0;
   {
      TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
      _LI.foreach_mf(&KBoolLink::UnMark);
      _LI.mergesort(linkXYsorter);
      {
      TDLI<KBoolLink> 	links=TDLI<KBoolLink>(_linklist);
      Node*nodeOne, *nodeTwo;
      for(_LI.tohead(); !_LI.hitroot(); _LI++)
      {
         nodeOne = _LI.item()->GetBeginNode();
         if(!_LI.item()->IsMarked())
         {
            _LI.item()->Mark();
            links.toiter(&_LI);links++;
            while (!links.hitroot())
            {
               nodeTwo = links.item()->GetBeginNode();
               if(!links.item()->IsMarked())
               {
                  if(babs(nodeOne->GetX()-nodeTwo->GetX()) <= Marge )
                  {
                     if(babs(nodeOne->GetY()-nodeTwo->GetY()) <= Marge &&
                        (!(nodeOne == nodeTwo))
                       )
                     {
                        links.item()->Mark();
                        nodeOne->Merge(nodeTwo);
                        merges++;
                     }//y binnen marge en niet zelfde node
                  }//x binnen marge?
                  else
                  {
                     links.totail();
                  }
               }//marked?
               links++;
            }//current -> ongeldig
         }//verwerkt?
      }//all links
      }//om de extra iterator te verwijderen
   }
	RemoveNullLinks();
	return merges;
}
int Graph::ScanGraph2(SCANTYPE scantype, bool& holes )
{
   TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
   int found=0;
	//sort links on x and y value of beginnode
	_LI.mergesort(linkXYsorter);
	writegraph( false );
	//bin flag is used in scanbeam so reset
   _LI.foreach_mf(&KBoolLink::SetNotBeenHere);
   ScanBeam* scanbeam = new ScanBeam(_GC);
   Node*  _low;
   Node*  _high;
   _LI.tohead();
   while (!_LI.attail())
   {
         _low = _LI.item()->GetBeginNode();
         if ( scanbeam->FindNew(scantype,&_LI, holes ) )
            found++;
         do
         {  _LI++;}
         while (!_LI.hitroot() && (_low == _LI.item()->GetBeginNode()));
         if (_LI.hitroot())
            break;
         else
            _high=_LI.item()->GetBeginNode();
         scanbeam->SetType(_low,_high);
         if (scanbeam->RemoveOld(scantype,&_LI, holes ) )
            found++;
   }
	delete scanbeam;
	return found;
}
/*
      if (j%100 ==0)
      {
        long x;
        long y;
        char buf[80];
		   x=(long)_lowlink->GetBeginNode()->GetX();
		   y=(long)_lowlink->GetBeginNode()->GetY();
        sprintf(buf," x=%I64d , y=%I64d here %I64d",x,y,scanbeam->count());
			_GC->SetState(buf);
      	scanbeam->writebeam();
      }
         writegraph(false);
            if (!checksort())
            {
               double x=_lowlink->GetBeginNode()->GetX();
               checksort();
            }
         _LI++;
      }
   }
	delete scanbeam;
	return 0;
}
         if (!checksort())
         {
            x=_lowlink->GetBeginNode()->GetX();
            checksort();
         }
         if (x >= -112200)
         {
         }
*/
int linkXYsorter(KBoolLink *a, KBoolLink* b)
{
	if ( a->GetBeginNode()->GetX() < b->GetBeginNode()->GetX())
		return(1);
	if ( a->GetBeginNode()->GetX() > b->GetBeginNode()->GetX())
		return(-1);
	if ( a->GetBeginNode()->GetY() < b->GetBeginNode()->GetY())
		return(-1);
	if ( a->GetBeginNode()->GetY() > b->GetBeginNode()->GetY())
		return(1);
	return(0);
}
int linkYXsorter(KBoolLink *a, KBoolLink* b)
{
	if ( a->GetBeginNode()->GetY() > b->GetBeginNode()->GetY())
		return(1);
	if ( a->GetBeginNode()->GetY() < b->GetBeginNode()->GetY())
		return(-1);
	if ( a->GetBeginNode()->GetX() > b->GetBeginNode()->GetX())
		return(-1);
	if ( a->GetBeginNode()->GetX() < b->GetBeginNode()->GetX())
		return(1);
	return(0);
}
int linkLsorter(KBoolLink *_l1, KBoolLink* _l2)
{
	B_INT dx1,dx2,dy1,dy2;
	dx1 = (_l1->GetEndNode()->GetX() - _l1->GetBeginNode()->GetX());
	dx1*=dx1;
	dy1 = (_l1->GetEndNode()->GetY() - _l1->GetBeginNode()->GetY());
	dy1*=dy1;
	dx2 = (_l2->GetEndNode()->GetX() - _l2->GetBeginNode()->GetX());
	dx2*=dx2;
	dy2 = (_l2->GetEndNode()->GetY() - _l2->GetBeginNode()->GetY());
	dy2*=dy2;
	dx1+=dy1;	dx2+=dy2;
	if ( dx1 > dx2 )
		return(-1);
	if ( dx1 < dx2 )
		return(1);
	return(0);
}
int linkYXtopsorter(KBoolLink *a, KBoolLink *b)
{
   if (bmax(a->GetBeginNode()->GetY(),a->GetEndNode()->GetY()) < bmax(b->GetBeginNode()->GetY(),b->GetEndNode()->GetY()))
      return -1;
   
   if (bmax(a->GetBeginNode()->GetY(),a->GetEndNode()->GetY()) > bmax(b->GetBeginNode()->GetY(),b->GetEndNode()->GetY()))
      return 1;
   
   if (bmin(a->GetBeginNode()->GetX(),a->GetEndNode()->GetX()) < bmin(b->GetBeginNode()->GetX(),b->GetEndNode()->GetX()))
      return -1;
   
   if (bmin(a->GetBeginNode()->GetX(),a->GetEndNode()->GetX()) > bmin(b->GetBeginNode()->GetX(),b->GetEndNode()->GetX()))
      return 1;
   
   return 0;
}
int linkGraphNumsorter(KBoolLink *_l1, KBoolLink* _l2)
{
	if ( _l1->GetGraphNum() > _l2->GetGraphNum())
		return(-1);
	if ( _l1->GetGraphNum() < _l2->GetGraphNum())
		return(1);
	return(0);
}
void Graph::Boolean(BOOL_OP operation,GraphList* Result)
{
	_GC->SetState("Performing Operation");
	// At this moment we have one graph
	// step one, split it up in single graphs, and mark the holes
	// step two, make one graph again and link the holes
	// step three, split up again and dump the result in Result
	_GC->SetState("Extract simples first ");
   ResetBinMark();
   DeleteNonCond(operation);
   HandleNonCond(operation);
   bool foundholes = false;
   try
	{
      writegraph(true);
		Extract_Simples(operation,true, foundholes);
	}
	catch (Bool_Engine_Error& error)
	{
      throw error;
	}
	// now we will link all the holes in de graphlist
	// By the scanbeam method we will search all the links that are marked
	//	as topleft link of a the hole polygon, when we find them we will get the
	//	closest link, being the one higher in the beam.
	//	Next we will create a link and nodes toceoonect the hole into it outside contour 
	_GC->SetState("Linking Holes");
   if (_linklist->count() == 0)
      return;
   if ( foundholes && _GC->GetLinkHoles() )
   {
      Merge_NodeToNode(0);
      _GC->Write_Log("LINKHOLES\n");
      writegraph( false );
      bool holes = false; 
      ScanGraph2(LINKHOLES, holes );
      writegraph(true);
      if ( holes )
      {
		   //to delete extra points
		   //those extra points are caused by link holes
		   DeleteZeroLines(1);
		   _GC->SetState("extract simples last");
         ResetBinMark();
         HandleNonCond(operation);
         DeleteNonCond(operation);
		   Extract_Simples(operation,false, foundholes);
      }
   }
   writegraph( true );
   Split(Result);
}
void Graph::Correction( GraphList* Result, double factor )
{
	// At this moment we have one graph
	// step one, split it up in single graphs, and mark the holes
	// step two, make one graph again and link the holes
	// step three, split up again and dump the result in Result
	_GC->SetState("Extract simple graphs");
	//extract the (MERGE or OR) result from the graph
	if (Simplify(_GC->GetGrid()))
		if (GetNumberOfLinks() < 3)
				return;
	Graph* original=new Graph(_GC);
   {
      if (_linklist->empty()) return;
      KBoolLink* _current = GetFirstLink();
		Node*_first = new Node(_current->GetBeginNode(), _GC);
      Node*_last	 = _current->GetBeginNode();
      Node*_begin = _first;
      Node*_end   = _first;
		int _nr_of_points = GetNumberOfLinks();
		for (int i = 1; i < _nr_of_points; i++)
      {
 			// get the other node on the link
			_last = _current->GetOther(_last);
         _end = new Node(_last, _GC);
         original->AddLink(_begin, _end);
         _begin=_end;
 			_current = _current->Forth(_last);
      }
      original->AddLink(_begin, _first);
   }
	SetNumber(1);
	SetGroup(GROUP_A);
	Prepare(1);
   ResetBinMark();
   HandleNonCond(BOOL_OR);
   bool foundholes = false;
	Extract_Simples( BOOL_OR, true, foundholes );
   Split(Result);
	//Result contains the separate boundaries and holes
   bool rule = _GC->GetWindingRule();
   _GC->SetWindingRule( true );
	_GC->SetState("Create rings");
	//first create a ring around all simple graphs
   {
     	TDLI<Graph> IResult=TDLI<Graph>(Result);
      GraphList *_ring = new GraphList(_GC);
      {
         IResult.tohead();
         int i;
         int n=IResult.count();
         for (i=0;i<n;i++)
         {
           {
				  IResult.item()->MakeClockWise();
              IResult.item()->CreateRing_fast(_ring,fabs(factor));
           }
           delete(IResult.item());
           IResult.remove();
            while (!_ring->empty())
            {
					((Graph*)_ring->headitem())->MakeClockWise();
               IResult.insend((Graph*)_ring->headitem());
               _ring->removehead();
            }
         }
      }
      delete _ring;
      int i=2;
      IResult.tohead();
      while (!IResult.hitroot())
      {
        IResult.item()->Reset_flags();
        IResult.item()->SetGroup(GROUP_B);
        IResult.item()->SetNumber(i);
        i++;
        IResult++;
      }
   }
   if ( !rule ) //alternate rule? 
   {
      Prepare(1);
      Boolean(BOOL_OR,Result);
     	TDLI<Graph> IResult=TDLI<Graph>(Result);
      int i=2;
      IResult.tohead();
      while (!IResult.hitroot())
      {
        IResult.item()->Reset_flags();
        IResult.item()->SetGroup(GROUP_B);
        IResult.item()->SetNumber(i);
        i++;
        IResult++;
      }
   }
   _GC->SetWindingRule( rule );
	TakeOver(original);
   Reset_flags();
   SetNumber(1);
   SetGroup(GROUP_A);
   Result->MakeOneGraph(this); // adds all graph its links to original
										  // Result will be empty afterwords
	//merge ring with original shapes for positive correction else subtract ring
	//calculate intersections etc.
	//SINCE correction will calculate intersections between
	//ring and original _GC->Get_Marge() must be set a lot smaller then factor
	//during the rest of this routine
	//else wierd effects will be the result.
	double Backup_Marge = _GC->GetMarge();
	if (_GC->GetInternalMarge() > fabs(factor/100))
	{
      _GC->SetInternalMarge( (B_INT) fabs(factor/100));
	   //less then 1 is usless since all coordinates are integers
	   if (_GC->GetInternalMarge() < 1)
		   _GC->SetInternalMarge(1);
	}
	Prepare(1);
	_GC->SetState("Add/Substract rings");
	if (factor > 0)
		Boolean(BOOL_OR,Result);
	else
		Boolean(BOOL_A_SUB_B,Result);
	_GC->SetMarge( Backup_Marge);
	//the result of the graph correction is in Result
   delete original;
}
void Graph::MakeRing( GraphList* Result, double factor )
{
   bool rule = _GC->GetWindingRule();
   _GC->SetWindingRule( true );
	// At this moment we have one graph
	// step one, split it up in single graphs, and mark the holes
	// step two, make one graph again and link the holes
	// step three, split up again and dump the result in Result
	_GC->SetState("Extract simple graphs");
	//extract the (MERGE or OR) result from the graph
	SetNumber(1);
	Prepare(1);
   ResetBinMark();
   HandleNonCond(BOOL_OR);
   bool foundholes = false;
	Extract_Simples( BOOL_OR, true, foundholes );
   Split(Result);
	//Iresult contains the separate boundaries and holes
	//make a correction on the boundaries factor
	//make a correction on the holes -factor
	_GC->SetState("Create rings");
   TDLI<Graph> IResult=TDLI<Graph>(Result);
   GraphList *_ring = new GraphList(_GC);
   {
      IResult.tohead();
      int i;
      int n=IResult.count();
      for (i=0;i<n;i++)
      {
        {
  			   IResult.item()->MakeClockWise();
            IResult.item()->CreateRing_fast(_ring,fabs(factor));
        }
        delete(IResult.item());
        IResult.remove();
         while (!_ring->empty())
         {
				((Graph*)_ring->headitem())->MakeClockWise();
            IResult.insend((Graph*)_ring->headitem());
            _ring->removehead();
         }
      }
   }
	delete _ring;
   _GC->SetWindingRule( rule );
}
void Graph::CreateRing( GraphList *ring, double factor )
{
  TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
  _LI.tohead();
  while( !_LI.hitroot())
  {
	 Graph *shape=new Graph(_GC);
	 //generate shape around link
	 shape->Make_Rounded_Shape(_LI.item(),factor);
	 ring->insbegin(shape);
	 _LI++;
  }
}
void Graph::CreateRing_fast( GraphList *ring, double factor )
{
	Node* begin;
	KBoolLink* currentlink;
	KBoolLine  currentline(_GC);
	KBoolLine  firstline(_GC);
	KBoolLink* nextlink;
	KBoolLine nextline(_GC);
   {
   	TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
		_LI.foreach_mf(&KBoolLink::UnMark);//reset bin and mark flag of each link
		_LI.mergesort(linkYXsorter);
	   _LI.tohead();
		begin = GetMostTopLeft(&_LI); // from all the most Top nodes,
											   // take the most left one
   }
	if (!begin)
		return;
	currentlink=begin->GetIncomingLink();
	currentline.Set(currentlink);
	currentline.CalculateLineParameters();
	nextlink=begin->GetOutgoingLink();
	nextline.Set(nextlink);
	nextline.CalculateLineParameters();
	firstline.Set(nextlink);
	firstline.CalculateLineParameters();
	while (nextlink)
	{
		Graph *shape=new Graph(_GC);
		{
			Node* _last_ins_left  =0;
			Node* _last_ins_right =0;
			currentline.Create_Begin_Shape(&nextline,&_last_ins_left,&_last_ins_right,factor,shape);
			while(true)
			{
				currentline=nextline;
				currentlink=nextlink;
				currentlink->SetBeenHere();
				nextlink=currentlink->GetEndNode()->Follow(currentlink);
				if (nextlink)
				{
					nextline.Set(nextlink);
					nextline.CalculateLineParameters();
					if (!currentline.Create_Ring_Shape(&nextline,&_last_ins_left,&_last_ins_right,factor,shape))
						break;
				}
				else
					break;
			}
			//finish this part
			if (nextlink)
				currentline.Create_End_Shape(&nextline,_last_ins_left,_last_ins_right,factor,shape);
			else
				currentline.Create_End_Shape(&firstline,_last_ins_left,_last_ins_right,factor,shape);
			shape->MakeOneDirection();
  			shape->MakeClockWise();
		}
		//if the shape is very small first merge it with the previous shape
		if (!ring->empty() && shape->Small( (B_INT) fabs(factor*3)))
		{
		   TDLI<Graph> Iring = TDLI<Graph>(ring);
			Iring.totail();
			GraphList *_twoshapes=new GraphList(_GC);
			_twoshapes->insbegin(shape);
			_twoshapes->insbegin(Iring.item());
			Iring.remove();
			_twoshapes->Merge();
			//move merged graphlist to ring
			Iring.takeover(_twoshapes);
			delete _twoshapes;
		}
		else
			ring->insend(shape);
		currentlink->SetBeenHere();
   }
}
void Graph::CreateArc(Node* center, Node* begin, Node* end,double radius,bool clock,double aber)
{
	double phi,dphi,dx,dy;
	int Segments;
	int i;
	double ang1,ang2,phit;
	Node* _last_ins;
	Node* _current;
	_last_ins=begin;
	dx = (double) _last_ins->GetX() - center->GetX();
	dy = (double) _last_ins->GetY() - center->GetY();
	ang1=atan2(dy,dx);
	if (ang1<0) ang1+=2.0*M_PI;
	dx = (double) end->GetX() - center->GetX();
	dy = (double) end->GetY() - center->GetY();
	ang2=atan2(dy,dx);
	if (ang2<0) ang2+=2.0*M_PI;
	if (clock)
	{ //clockwise
		if (ang2 > ang1)
			phit=2.0*M_PI-ang2+ ang1;
		else
			phit=ang1-ang2;
	}
	else
	{ //counter_clockwise
		if (ang1 > ang2)
			phit=-(2.0*M_PI-ang1+ ang2);
		else
			phit=-(ang2-ang1);
	}
	//what is the delta phi to get an accurancy of aber
	dphi=2*acos((radius-aber)/radius);
	//set the number of segments
	if (phit > 0)
		Segments=(int)ceil(phit/dphi);
	else
		Segments=(int)ceil(-phit/dphi);
	if (Segments <= 1)
	  Segments=1;
	if (Segments > 6)
	  Segments=6;
	dphi=phit/(Segments);
	for (i=1; i<Segments; i++)
	{
	  dx = (double) _last_ins->GetX() - center->GetX();
	  dy = (double) _last_ins->GetY() - center->GetY();
	  phi=atan2(dy,dx);
	  _current = new Node((B_INT) (center->GetX() + radius * cos(phi-dphi)),
								 (B_INT) (center->GetY() + radius * sin(phi-dphi)), _GC);
	  AddLink(_last_ins, _current);
	  _last_ins=_current;
	}
	// make a node from the endnode of link
	AddLink(_last_ins, end);
}
void Graph::CreateArc(Node* center, KBoolLine* incoming, Node* end,double radius,double aber)
{
	double distance=0;
	if (incoming->PointOnLine(center, distance, _GC->GetAccur()) == RIGHT_SIDE)
		CreateArc(center,incoming->GetEndNode(),end,radius,true,aber);
	else
		CreateArc(center,incoming->GetEndNode(),end,radius,false,aber);
}
void Graph::MakeOneDirection()
{
	int _nr_of_points = _linklist->count();
	KBoolLink* _current = (KBoolLink*)_linklist->headitem();
	Node* _last = _current->GetBeginNode();
	Node* dummy;
	for (int i = 0; i < _nr_of_points; i++)
	{
		// get the other node on the link
		_last = _current->GetOther(_last);
		// get the other link on the node
		_current = _current->Forth(_last);
		if (_current->GetBeginNode() != _last)
		{
			// swap the begin- and endnode of the current link
			dummy = _current->GetBeginNode();
			_current->SetBeginNode(_current->GetEndNode());
			_current->SetEndNode(dummy);
		}
	}
}
bool Graph::Small(B_INT howsmall)
{
   TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	_LI.tohead();
   Node* bg=_LI.item()->GetBeginNode();
   B_INT xmin=bg->GetX();
   B_INT xmax=bg->GetX();
   B_INT ymin=bg->GetY();
   B_INT ymax=bg ->GetY();
	while (!_LI.hitroot())
	{
     bg=_LI.item()->GetBeginNode();
	  // Checking if point is in bounding-box with marge
     xmin=bmin(xmin,bg->GetX());
     xmax=bmax(xmax,bg->GetX());
     ymin=bmin(ymin,bg->GetY());
     ymax=bmax(ymax,bg->GetY());
	  _LI++;
	}
	B_INT dx=(xmax-xmin);
	B_INT dy=(ymax-ymin);
	if ((dx < howsmall) && (dy < howsmall) )
      return true;
	return false;
}
void	Graph::Make_Rounded_Shape( KBoolLink* a_link, double factor)
{
	double phi,dphi,dx,dy;
	int Segments=6;
	int i;
	KBoolLine theline(a_link, _GC);
	theline.CalculateLineParameters();
	Node* _current;
	Node*_first = new Node(a_link->GetBeginNode(), _GC);
	Node*_last_ins = _first;
	theline.Virtual_Point(_first,factor);
	// make a node from this point
	_current = new Node(a_link->GetEndNode(), _GC);
	theline.Virtual_Point(_current,factor);
	// make a link between the current and the previous and add this to graph
	AddLink(_last_ins, _current);
	_last_ins=_current;
	// make a half circle around the clock with the opposite point as
	// the middle point of the circle
	dphi=M_PI/(Segments);
	for (i=1; i<Segments; i++)
	{
	  dx = (double) _last_ins->GetX() - a_link->GetEndNode()->GetX();
	  dy = (double) _last_ins->GetY() - a_link->GetEndNode()->GetY();
	  phi=atan2(dy,dx);
	  _current = new Node((B_INT) (a_link->GetEndNode()->GetX() + factor * cos(phi-dphi)),
								 (B_INT) (a_link->GetEndNode()->GetY() + factor * sin(phi-dphi)), _GC);
	  AddLink(_last_ins, _current);
	  _last_ins=_current;
	}
	// make a node from the endnode of link
	_current = new Node(a_link->GetEndNode(), _GC);
	theline.Virtual_Point(_current,-factor);
	AddLink(_last_ins, _current);
	_last_ins=_current;
	// make a node from this beginnode of link
	_current = new Node(a_link->GetBeginNode(), _GC);
	theline.Virtual_Point(_current,-factor);
	AddLink(_last_ins, _current);
	_last_ins=_current;
	for (i=1; i<Segments; i++)
	{
	  dx = (double) _last_ins->GetX() - a_link->GetBeginNode()->GetX();
	  dy = (double) _last_ins->GetY() - a_link->GetBeginNode()->GetY();
	  phi=atan2(dy,dx);
	  _current = new Node((B_INT)(a_link->GetBeginNode()->GetX() + factor * cos(phi-dphi)),
								 (B_INT)(a_link->GetBeginNode()->GetY() + factor * sin(phi-dphi)), _GC);
	  AddLink(_last_ins, _current);
	  _last_ins=_current;
	}
	// make a link between the last and the first to close the graph
	AddLink(_last_ins, _first);
};
bool Graph::MakeClockWise()
{
   if ( _GC->GetOrientationEntryMode() )
      return false;
	TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	if (_LI.empty()) return false;
	KBoolLink *currentlink;
	Node* begin;
	_LI.foreach_mf(&KBoolLink::UnMark);//reset bin and mark flag of each link
	_LI.mergesort(linkYXtopsorter);
   _LI.tohead();
	begin = GetMostTopLeft(&_LI); // from all the most Top nodes,
   								  		// take the most left one
	currentlink=begin->GetNotFlat();
	if (!currentlink)
   {
      char buf[100];
	   sprintf(buf,"no NON flat link MakeClockWise at %15.3lf , %15.3lf",
         				double(begin->GetX()),double(begin->GetY()));
		throw Bool_Engine_Error(buf, "Error", 9, 0);
	}
   if (currentlink->GetBeginNode() == begin)
   {
      if ( currentlink->GetEndNode()->GetX() < begin->GetX())
      {
	      //going left
         ReverseAllLinks();
         return true;
      }
   }
   else
   {
      if ( currentlink->GetBeginNode()->GetX() > begin->GetX())
      {  //going left
			//it needs redirection
         ReverseAllLinks();
         return true;
      }
   }
   return false;
}
bool Graph::writegraph(bool linked)
{
#if DEBUG == 1
   FILE* file = _GC->GetLogFile();
   if (file == NULL)
       return true;
	fprintf( file, "# graph\n" );
	TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	if (_LI.empty())
   {  
		return true;
   }
   _LI.tohead();
   while(!_LI.hitroot())
   {
	   KBoolLink* curl = _LI.item();
      fprintf( file, " linkbegin %I64d %I64d \n", curl->GetBeginNode()->GetX() , curl->GetBeginNode()->GetY() );
		if (linked)
      {
			TDLI<KBoolLink> Inode(curl->GetBeginNode()->GetLinklist());
         Inode.tohead();
		   while(!Inode.hitroot())
   		{
            fprintf( file, " b %I64d %I64d \n", Inode.item()->GetBeginNode()->GetX() , Inode.item()->GetBeginNode()->GetY() );
            fprintf( file, " e %I64d %I64d \n", Inode.item()->GetEndNode()->GetX() , Inode.item()->GetEndNode()->GetY() );
            Inode++;
         }
      }
      fprintf( file, " linkend %I64d %I64d \n", curl->GetEndNode()->GetX() , curl->GetEndNode()->GetY() );
      if (linked)
      {
			TDLI<KBoolLink> Inode(curl->GetEndNode()->GetLinklist());
         Inode.tohead();
		   while(!Inode.hitroot())
   		{
            fprintf( file, " b %I64d %I64d \n", Inode.item()->GetBeginNode()->GetX() , Inode.item()->GetBeginNode()->GetY() );
            fprintf( file, " e %I64d %I64d \n", Inode.item()->GetEndNode()->GetX() , Inode.item()->GetEndNode()->GetY() );
            Inode++;
         }
      }
      if ( curl->GetBeginNode() == curl->GetEndNode() )
		   fprintf( file, "     null_link \n" );
	   fprintf( file, "    group %d ", curl->Group() );
	   fprintf( file, "    bin   %d ", curl->BeenHere() );
	   fprintf( file, "    mark  %d ", curl->IsMarked() );
	   fprintf( file, "    leftA %d ", curl->GetLeftA() );
	   fprintf( file, "    rightA %d ", curl->GetRightA() );
	   fprintf( file, "    leftB %d ", curl->GetLeftB() );
	   fprintf( file, "    rightB %d \n", curl->GetRightB() );
	   fprintf( file, "    or %d ", curl->IsMarked(BOOL_OR) );
	   fprintf( file, "    and %d " , curl->IsMarked(BOOL_AND) );
	   fprintf( file, "    exor %d " , curl->IsMarked(BOOL_EXOR) );
	   fprintf( file, "    a_sub_b %d " , curl->IsMarked(BOOL_A_SUB_B) );
	   fprintf( file, "    b_sub_a %d " , curl->IsMarked(BOOL_B_SUB_A) );
	   fprintf( file, "    hole %d " , curl->GetHole() );
	   fprintf( file, "    top_hole %d \n" , curl->IsTopHole() );
      
      _LI++;
   }
#endif
   return true;
}
bool Graph::writeintersections()
{
#if DEBUG == 1
   FILE* file = _GC->GetLogFile();
   if (file == NULL)
       return true;
   fprintf( file, "# graph\n" );
	TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	if (_LI.empty())
   {  
		return true;
   }
   _LI.tohead();
   while(!_LI.hitroot())
   {
	   KBoolLink* curl=_LI.item();
      TDLI<KBoolLink> Inode(curl->GetBeginNode()->GetLinklist());
      Inode.tohead();
      if (Inode.count() > 2)
      {
         fprintf( file, " count %I64d", Inode.count() );
         fprintf( file, " b %I64d %I64d \n\n", curl->GetBeginNode()->GetX() , curl->GetBeginNode()->GetY() );
      }
      _LI++;
   }
#endif
   return true;
}
bool Graph::checksort()
{
	// if empty then just insert
	if (_linklist->empty())
		return true;
	TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
   _LI.tohead();
   KBoolLink* prev=_LI.item();
   KBoolLink* cur=_LI.item();
   _LI++;
   while(!_LI.hitroot())
   {
      KBoolLink* aap=_LI.item();
      if (linkXYsorter(prev,_LI.item())==-1)
      {
         cur=aap;
         return false;
      }
      prev=_LI.item();
      _LI++;
   }
   return true;
}
void Graph::WriteKEY( Bool_Engine* GC, FILE* file )
{
   double scale = 1.0/GC->GetGrid()/GC->GetGrid();
   bool ownfile = false;
   if ( !file )
   {
      file = fopen("keyfile.key", "w");
      ownfile = true;
      fprintf(file,"\
         HEADER 5; \
         BGNLIB; \
         LASTMOD {2-11-15  15:39:21}; \
         LASTACC {2-11-15  15:39:21}; \
         LIBNAME trial; \
         UNITS; \
         USERUNITS 0.0001; PHYSUNITS 1e-009; \
      \
         BGNSTR;  \
         CREATION {2-11-15  15:39:21}; \
         LASTMOD  {2-11-15  15:39:21}; \
         STRNAME top; \
      ");
   }
	TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	if (_LI.empty())
   {  
      if ( ownfile )
      {
         fprintf(file,"\
            ENDSTR top; \
            ENDLIB; \
         ");
         fclose (file);
      }
		return;
   }
   _LI.tohead();
   KBoolLink* curl = _LI.item();
	if ( _LI.item()->Group() == GROUP_A )
      fprintf(file,"BOUNDARY; LAYER 0;  DATATYPE 0;\n");
   else
      fprintf(file,"BOUNDARY; LAYER 1;  DATATYPE 0;\n");
   fprintf(file," XY %d; \n", _LI.count()+1 );
   double firstx = curl->GetBeginNode()->GetX()*scale;
   double firsty = curl->GetBeginNode()->GetY()*scale;
   fprintf(file,"X %f;\t", firstx);
   fprintf(file,"Y %f; \n", firsty); 
   _LI++;
   while(!_LI.hitroot())
   {
	   KBoolLink* curl = _LI.item();
      fprintf(file,"X %f;\t", curl->GetBeginNode()->GetX()*scale);
      fprintf(file,"Y %f; \n", curl->GetBeginNode()->GetY()*scale); 
      
      _LI++;
   }
   fprintf(file,"X %f;\t", firstx);
   fprintf(file,"Y %f; \n", firsty); 
   fprintf(file,"ENDEL;\n");
   if ( ownfile )
   {
      fprintf(file,"\
         ENDSTR top; \
         ENDLIB; \
      ");
      fclose (file);
   }
}
 
void Graph::WriteGraphKEY(Bool_Engine* GC)
{
   double scale = 1.0/GC->GetGrid()/GC->GetGrid();
   FILE* file = fopen("keygraphfile.key", "w");
   fprintf(file,"\
      HEADER 5; \
      BGNLIB; \
      LASTMOD {2-11-15  15:39:21}; \
      LASTACC {2-11-15  15:39:21}; \
      LIBNAME trial; \
      UNITS; \
      USERUNITS 1; PHYSUNITS 1e-006; \
   \
      BGNSTR;  \
      CREATION {2-11-15  15:39:21}; \
      LASTMOD  {2-11-15  15:39:21}; \
      STRNAME top; \
   ");
	TDLI<KBoolLink> _LI=TDLI<KBoolLink>(_linklist);
	if (_LI.empty())
   {  
      fprintf(file,"\
         ENDSTR top; \
         ENDLIB; \
      ");
      fclose (file);
		return;
   }
   _LI.tohead();
   KBoolLink* curl;
	int _nr_of_points = _linklist->count();
	for (int i = 0; i < _nr_of_points; i++)
	{
	   curl = _LI.item();
	   if ( curl->Group() == GROUP_A )
         fprintf(file,"PATH; LAYER 0;\n");
      else
         fprintf(file,"PATH; LAYER 1;\n");
      fprintf(file," XY %d; \n", 2 );
      fprintf(file,"X %f;\t", curl->GetBeginNode()->GetX()*scale);
      fprintf(file,"Y %f; \n", curl->GetBeginNode()->GetY()*scale); 
      fprintf(file,"X %f;\t", curl->GetEndNode()->GetX()*scale);
      fprintf(file,"Y %f; \n", curl->GetEndNode()->GetY()*scale); 
      _LI++;
      fprintf(file,"ENDEL;\n");      
	}
   fprintf(file,"\
      ENDSTR top; \
      ENDLIB; \
   ");
   fclose (file);
}
