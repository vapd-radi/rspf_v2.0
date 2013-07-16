/*! \file kbool/src/scanbeam.cpp
    \author Probably Klaas Holwerda or Julian Smart
    Copyright: 2001-2004 (C) Probably Klaas Holwerda 
    Licence: wxWidgets Licence
    RCS-ID: $Id: scanbeam.cpp 9094 2006-06-13 19:12:40Z dburken $
*/
#ifdef __GNUG__
#pragma implementation
#endif
#include <rspf/kbool/scanbeam.h>
#include <math.h>
#include <assert.h>
#include <rspf/kbool/booleng.h>
#include <rspf/kbool/graph.h>
#include <rspf/kbool/node.h>
int recordsorter(Record* , Record* );
int recordsorter_ysp_angle(Record* , Record* );
int recordsorter_ysp_angle_back(Record* rec1, Record* rec2);
ScanBeam::ScanBeam(Bool_Engine* GC):DL_List<Record*>()
{
   _GC = GC;
	_type=NORMAL;
   _BI.Attach(this);
}
ScanBeam::~ScanBeam()
{
   _BI.Detach();
   remove_all( true );
}
void ScanBeam::SetType(Node* low,Node* high)
{
   if (low->GetX() < high->GetX())
   	_type=NORMAL;
   else
   	_type=FLAT;
}
/*
int ScanBeam::FindCloseLinksAndCross(TDLI<KBoolLink>* _I,Node* _lowf)
{
	int merges = 0;
	Record* record;
   TDLI<Record> _BBI=TDLI<Record>(this);
	if (_BI.count() > 1)
	{
		//first search a link towards this node
		for(_BI.tohead(); !_BI.hitroot(); _BI++)
		{
			record=_BI.item();
			if( (record->GetLink()->GetBeginNode()==_lowf) ||
				 (record->GetLink()->GetEndNode()  ==_lowf)
			  )
			  break;
		}
		//NOTICE if the node "a_node" is not inside a record
		//for instance to connected flat links (flatlinks not in beam)
		//then IL will be at end    (those will be catched at 90 degrees rotation)
		if (_BI.hitroot())
      {
			return(merges);
      }
      _BBI.toiter(&_BI);
      _BBI--;
      while(!_BBI.hitroot())
      {
         record=_BBI.item();
         if (record->Ysp() != _lowf->GetY())
            break;
         if( (record->GetLink()->GetBeginNode()!=_lowf) &&
             (record->GetLink()->GetEndNode()  !=_lowf)
           )
         {  // the link is not towards the low node
            record->GetLink()->AddCrossing(_lowf);
            record->SetNewLink(record->GetLink()->ProcessCrossingsSmart(_I));
            merges++;
         }
         _BBI--;
      }
      _BBI.toiter(&_BI);
      _BBI++;
      while(!_BBI.hitroot())
      {
         record=_BBI.item();
         if (record->Ysp() != _lowf->GetY())
            break;
         if( (record->GetLink()->GetBeginNode()!=_lowf) &&
             (record->GetLink()->GetEndNode()  !=_lowf)
           )
         {  // the link is not towards the low node
            record->GetLink()->AddCrossing(_lowf);
            record->SetNewLink(record->GetLink()->ProcessCrossingsSmart(_I));
            merges++;
         }
         _BBI++;
      }
   }
	return merges;
}
*/
/*
bool  ScanBeam::Update(TDLI<KBoolLink>* _I,Node* _lowf)
{
	bool found=false;
	KBoolLink* link;
	_BI.tohead();
	while (!_BI.hitroot())
	{
		Record* record=_BI.item();
		record->Calc_Ysp(_type,_low);
		_BI++;
	}
   FindCloseLinksAndCross(_I,_lowf);
   _BI.tohead();
   while (!_BI.hitroot())
   {
      Record* record=_BI.item();
      if ((record->GetLink()->GetEndNode() == _lowf) ||
          (record->GetLink()->GetBeginNode() == _lowf)
         )
      {
         delete _BI.item();
         _BI.remove();
         _BI--;
         if (!_BI.hitroot() && (_BI.count() > 1))
         {
            Record* prev=_BI.item();
            _BI++;
            if (!_BI.hitroot())
            {
               if (!_BI.item()->Equal(prev)) // records NOT parallel
                  if (_BI.item()->GetLine()->Intersect(prev->GetLine(),MARGE))
                  {
                     _BI.item()->SetNewLink(_BI.item()->GetLink()->ProcessCrossingsSmart(_I));
                     prev->SetNewLink(prev->GetLink()->ProcessCrossingsSmart(_I));
                  }
            }
         }
         else
          	_BI++;
      }
      else
         _BI++;
   }
	//writebeam();
	//the bin flag will be set if it fits in the beam
	//so for following beams it will not be checked again
	while ( bool(link=_lowf->GetBinHighest(false)) )
	{
      Record* record=new Record(link);
      record->SetYsp(_lowf->GetY());
      record->Set_Flags(_type);
      if (empty())
         insend(record);
      else
      {
         _BI.tohead();
         while(!_BI.hitroot())
         {
            if (recordsorter_ysp_angle(record,_BI.item())==1)
               break;
            _BI++;
         }
         _BI.insbefore(record);
         _BI--;_BI--; //just before the new record inserted
         if (!_BI.hitroot())
         {
            Record* prev=_BI.item();
            _BI++; //goto the new record inserted
            if (!_BI.item()->Equal(prev)) // records NOT parallel
            {
               if (_BI.item()->GetLine()->Intersect(prev->GetLine(),MARGE))
               {
                  _BI.item()->SetNewLink(_BI.item()->GetLink()->ProcessCrossingsSmart(_I));
                  prev->SetNewLink(prev->GetLink()->ProcessCrossingsSmart(_I));
               }
            }
         }
         else
           _BI++;
         Record* prev=_BI.item(); //the new record
         _BI++;
         if (!_BI.hitroot() && !_BI.item()->Equal(prev)) // records NOT parallel
         {
	         Record* cur=_BI.item();
            if (cur->GetLine()->Intersect(prev->GetLine(),MARGE))
            {
               cur->SetNewLink(cur->GetLink()->ProcessCrossingsSmart(_I));
               prev->SetNewLink(prev->GetLink()->ProcessCrossingsSmart(_I));
            }
         }
      }
      GNI->insend(record->GetLink()->GetGraphNum());
      found=true;
	   record->GetLink()->SetBeenHere();
   }
   FindCloseLinksAndCross(_I,_lowf);
	//writebeam();
	return(found);
}
*/
bool ScanBeam::FindNew(SCANTYPE scantype,TDLI<KBoolLink>* _I, bool& holes )
{
   bool foundnew = false;
   _low = _I->item()->GetBeginNode();
	KBoolLink* link;
	//if (!checksort())
	//	SortTheBeam();
   lastinserted=0;
	//the bin flag will be set if it fits in the beam
	//so for following beams it will not be checked again
	while ( (link = _low->GetBinHighest(false)) != NULL )
   {
     	if ( (link->GetEndNode()->GetX() == link->GetBeginNode()->GetX()) //flatlink in flatbeam
           && ((scantype == NODELINK) || (scantype == LINKLINK) || (scantype == LINKHOLES))
         )
      {
            switch(scantype)
            {
               case NODELINK:
               {
                  Record* record=new Record(link,_GC);
                  record->SetYsp(_low->GetY());
                  record->Set_Flags();
                  link->SetRecordNode( _BI.insbefore(record) );
                  _BI--;
	               foundnew = Process_PointToLink_Crossings() !=0 || foundnew;
                  delete record;
                  _BI.remove();
                  break;
               }
               case LINKLINK:
               {
                  KBoolLine flatline = KBoolLine(link, _GC);
                  foundnew = Process_LinkToLink_Flat(&flatline) || foundnew;
	   				flatline.ProcessCrossings(_I);
		            break;
               }
               case LINKHOLES : //holes are never to flatlinks
                   assert( true );
      	      default: 
      		       break;             
            }
      }
      else
      {
         Record* record = new Record(link,_GC);
         record->SetYsp(_low->GetY());
         record->Set_Flags();
         link->SetRecordNode( _BI.insbefore(record) );         
         lastinserted++;
         writebeam();
         switch(scantype)
         {
          case NODELINK:
               _BI--;
               foundnew = Process_PointToLink_Crossings() !=0  || foundnew;
               _BI++;
               break;
          case INOUT:
            {
               _BI--;
					Generate_INOUT(record->GetLink()->GetGraphNum());
               _BI++;
            }
            break;
          case GENLR:
            {
               _BI--;
               _BI--;
               Record* above=0;
               if (!_BI.hitroot())
                  above=_BI.item();
               _BI++;
               if (record->Calc_Left_Right(above))
               {
                  delete record;
                  _BI.remove();
                  lastinserted--;
               }
               else
                  _BI++;
            }
            break;
          case LINKHOLES:
            _BI--;
            holes = ProcessHoles(true,_I) || holes;
            _BI++;
            break;
	       default:
	         break;
         }
      }
      link->SetBeenHere();
	}
   writebeam();
	return foundnew;
}
bool ScanBeam::RemoveOld(SCANTYPE scantype,TDLI<KBoolLink>* _I, bool& holes )
{
	bool found = false;
	bool foundnew = false;
   DL_Iter<Record*>  _BBI=DL_Iter<Record*>();
	bool attached=false;
   _low = _I->item()->GetBeginNode();
   switch(scantype)
   {
      case INOUT:
      case GENLR:
      case LINKHOLES:
      if (_type==NORMAL )
      {
         if (_low->GetBinHighest(true)) //is there something to remove
         {
            if ( scantype == LINKHOLES )
            {
               _BI.tohead();
               while (!_BI.hitroot())
               {
                  Record* record=_BI.item();
                  if ((record->GetLink()->GetEndNode() == _low) ||
                      (record->GetLink()->GetBeginNode() == _low)
                     )
                  {
                     holes = ProcessHoles(false,_I) || holes;
                  }
                  _BI++;
               }
            }
            _BI.tohead();
            while (!_BI.hitroot())
            {
               Record* record=_BI.item();
               if ((record->GetLink()->GetEndNode() == _low) ||
                   (record->GetLink()->GetBeginNode() == _low)
                  )
               {
                  if (attached)   //there is a bug
                  {
                     _BBI.Detach();
                     if (!checksort())
                        SortTheBeam( true );
                     _BI.tohead();
                     attached=false;
                  }
                  delete _BI.item();
                  _BI.remove();
                  found=true;
               }
               else if (found) //only once in here
               {
                  attached=true;
                  found=false;
                  _BBI.Attach(this);
                  _BBI.toiter(&_BI); //this is the position new records will be inserted
                  record->Calc_Ysp(_low);
                  _BI++;
               }
               else
               {
                  record->Calc_Ysp(_low);
                  _BI++;
               }
            }
            if (attached)
            {
               _BI.toiter(&_BBI);
               _BBI.Detach();
            }
         }
         else
         {
            _BBI.Attach(this);
            _BBI.toroot();
            _BI.tohead();
            while (!_BI.hitroot())
            {
                  Record* record=_BI.item();
                  record->Calc_Ysp(_low);
                  if (!found && (record->Ysp() < _low->GetY()))
                  {
                     found=true;
                     _BBI.toiter(&_BI);
                  }
                  _BI++;
            }
            _BI.toiter(&_BBI);
            _BBI.Detach();
         }
      }
      else
      {  //because the previous beam was flat the links to remove are
         if (_low->GetBinHighest(true)) //is there something to remove
         {
            if ( scantype == LINKHOLES )
            {
               _BI.tohead();
               while (!_BI.hitroot())
               {
                  Record* record=_BI.item();
                  if ((record->GetLink()->GetEndNode() == _low) ||
                      (record->GetLink()->GetBeginNode() == _low)
                     )
                  {
                     holes = ProcessHoles(false,_I) || holes;
                  }
                  _BI++;
               }
            }
            _BI.tohead();
            while (!_BI.hitroot())
            {
               Record* record=_BI.item();
               if ((record->GetLink()->GetEndNode() == _low) ||
                   (record->GetLink()->GetBeginNode() == _low)
                  )
               {
                  delete _BI.item();
                  _BI.remove();
                  found=true;
               }
               else if (found) //only once in here
                  break;
               else if (record->Ysp() < _low->GetY())
                     break;
               else
                  _BI++;
            }
         }
         else
         {
               _BI.tohead();
            while (!_BI.hitroot())
            {
                  Record* record=_BI.item();
                  if (record->Ysp() < _low->GetY())
                     break;
                  _BI++;
            }
         }
      }
      break;
      case NODELINK:
      case LINKLINK:
      {
         if (_type == NORMAL)
         {
            Calc_Ysp();
            if (scantype==LINKLINK)
               foundnew = Process_LinkToLink_Crossings() !=0 || foundnew;
            else
               SortTheBeam( false );
         }
         if (_low->GetBinHighest(true)) //is there something to remove
         {
            _BI.tohead();
            while (!_BI.hitroot())
            {
               Record* record=_BI.item();
               if ((record->GetLink()->GetEndNode() == _low) ||
                   (record->GetLink()->GetBeginNode() == _low)
                  )
               {
                  KBoolLine* line=record->GetLine();
                  if (scantype==NODELINK)
   	               foundnew = Process_PointToLink_Crossings() !=0 || foundnew;
                  line->ProcessCrossings(_I);
                  delete _BI.item();
                  _BI.remove();
                  found=true;
               }
               else if ((record->Ysp() < _low->GetY()))
                  break;
               else
                  _BI++;
            }
         }
         else
         {
            _BI.tohead();
            while (!_BI.hitroot())
            {
                  Record* record=_BI.item();
                  if ((record->Ysp() < _low->GetY()))
                     break;
                  _BI++;
            }
         }
      }
      break;
      default: 
         break; 
   }
	return foundnew;
}
/*
bool ScanBeam::RemoveOld(SCANTYPE scantype,TDLI<KBoolLink>* _I, bool& holes )
{
	bool found = false;
	bool foundnew = false;
   DL_Iter<Record*>  _BBI=DL_Iter<Record*>();
	bool attached=false;
   _low = _I->item()->GetBeginNode();
   switch(scantype)
   {
      case INOUT:
      case GENLR:
      case LINKHOLES:
      if (_type==NORMAL )
      {
         KBoolLink* link = _low->GetBinHighest(true);
         if ( link ) //is there something to remove
         {
            link->SetRecordNode( NULL );
            if ( scantype == LINKHOLES )
            {
               _BI.tohead();
               while (!_BI.hitroot())
               {
                  Record* record = _BI.item();
                  if ((record->GetLink()->GetEndNode() == _low) ||
                      (record->GetLink()->GetBeginNode() == _low)
                     )
                  {
                     holes = ProcessHoles(false,_I) || holes;
                  }
                  _BI++;
               }
            }
            _BI.tohead();
            while (!_BI.hitroot())
            {
               Record* record=_BI.item();
               if ((record->GetLink()->GetEndNode() == _low) ||
                   (record->GetLink()->GetBeginNode() == _low)
                  )
               {
                  delete _BI.item();
                  _BI.remove();
                  found=true;
               }
               else if (found) //only once in here
               {
                  attached=true;
                  found=false;
                  record->Calc_Ysp(_low);
                  _BI++;
               }
               else
               {
                  record->Calc_Ysp(_low);
                  _BI++;
               }
            }
         }
         else
         {
            _BI.tohead();
            while (!_BI.hitroot())
            {
                  Record* record=_BI.item();
                  record->Calc_Ysp(_low);
                  _BI++;
            }
         }
      }
      else
      {  //because the previous beam was flat the links to remove are
         KBoolLink* link;
         link = _low->GetBinHighest(true);
         if( link  )//is there something to remove
         {
            link->SetRecordNode( NULL );
            bool linkf = false;
            _BI.tohead();
            while (!_BI.hitroot())
            {
               Record* record = _BI.item();
               if (record->GetLink() == link) 
                  linkf = true;
               _BI++;
            }
            if ( !linkf )
               _BI.tohead();
            if ( scantype == LINKHOLES )
            {
               _BI.tohead();
               while (!_BI.hitroot())
               {
                  Record* record=_BI.item();
                  if ((record->GetLink()->GetEndNode() == _low) ||
                      (record->GetLink()->GetBeginNode() == _low)
                     )
                  {
                     holes = ProcessHoles(false,_I) || holes;
                  }
                  _BI++;
               }
            }
            _BI.tohead();
            while (!_BI.hitroot())
            {
               Record* record=_BI.item();
               if ((record->GetLink()->GetEndNode() == _low) ||
                   (record->GetLink()->GetBeginNode() == _low)
                  )
               {
                  if ( link != record->GetLink() )
                  {
                     break;
                  }
                  if ( link->GetRecordNode() != _BI.node() )
                  {
                     delete _BI.item();
                     _BI.remove();
                  }
                  else
                  {
                     delete _BI.item();
                     _BI.remove();
                  }
                  found=true;
               }
               else if (found) //only once in here
                  break;
               else if (record->Ysp() < _low->GetY())
                     break;
               else
                  _BI++;
            }
         }
         else
         {
               _BI.tohead();
            while (!_BI.hitroot())
            {
                  Record* record=_BI.item();
                  if (record->Ysp() < _low->GetY())
                     break;
                  _BI++;
            }
         }
      }
      break;
      case NODELINK:
      case LINKLINK:
      {
         if (_type == NORMAL)
         {
            Calc_Ysp();
            if (scantype==LINKLINK)
               foundnew = Process_LinkToLink_Crossings() !=0 || foundnew;
            else
               SortTheBeam( false );
         }
         if (_low->GetBinHighest(true)) //is there something to remove
         {
            _BI.tohead();
            while (!_BI.hitroot())
            {
               Record* record=_BI.item();
               if ((record->GetLink()->GetEndNode() == _low) ||
                   (record->GetLink()->GetBeginNode() == _low)
                  )
               {
                  KBoolLine* line=record->GetLine();
                  if (scantype==NODELINK)
   	               foundnew = Process_PointToLink_Crossings() !=0 || foundnew;
                  line->ProcessCrossings(_I);
                  delete _BI.item();
                  _BI.remove();
                  found=true;
               }
               else if ((record->Ysp() < _low->GetY()))
                  break;
               else
                  _BI++;
            }
         }
         else
         {
            _BI.tohead();
            while (!_BI.hitroot())
            {
                  Record* record=_BI.item();
                  if ((record->Ysp() < _low->GetY()))
                     break;
                  _BI++;
            }
         }
      }
      break;
      default: 
         break; 
   }
	return foundnew;
}
*/
void ScanBeam::SortTheBeam( bool backangle )
{
   if ( backangle )
      _BI.mergesort( recordsorter_ysp_angle_back );
   else
	   _BI.mergesort( recordsorter_ysp_angle );
}
void	ScanBeam::Calc_Ysp()
{
	_BI.tohead();
	while (!_BI.hitroot())
	{
		Record* record=_BI.item();
      record->Calc_Ysp(_low);
		_BI++;
	}
}
void ScanBeam::Generate_INOUT(int graphnumber)
{
	DIRECTION first_dir = GO_LEFT;
	int diepte          = 0;
   DL_Iter<Record*> _BBI = DL_Iter<Record*>();
   _BBI.Attach(this);
	for( _BBI.tohead(); !_BBI.hitroot(); _BBI++ )
	{
   	// recalculate _inc again
		if ( _BBI.item()->GetLink()->GetGraphNum()==graphnumber)
		{	//found a link that belongs to the graph
			if (diepte==0)
			{	// first link found or at depth zero again
				first_dir=_BBI.item()->Direction();
				_BBI.item()->GetLink()->SetInc(true);
				diepte=1;
			}
			else
			{	// according to depth=1 links set depth
				// verhoog of verlaag diepte
				if (_BBI.item()->Direction() == first_dir)
				{
					diepte++;
					_BBI.item()->GetLink()->SetInc(true);
				}
				else
				{
					diepte--;
					_BBI.item()->GetLink()->SetInc(false);
				}
			}
		}
		if ( _BBI.item() == _BI.item()) break; //not need to do the rest, will come in a later beam
	}
   _BBI.Detach();
}
bool ScanBeam::ProcessHoles( bool atinsert, TDLI<KBoolLink>* _LI )
{
	// The scanbeam must already be sorted at this moment
   Node *topnode;
   bool foundholes = false;
   Record* record = _BI.item();
   KBoolLink* link = record->GetLink();
   if (!record->GetLine()->CrossListEmpty())
   {
      SortTheBeam( atinsert );
      TDLI<Node> I(record->GetLine()->GetCrossList());
      I.tohead();
      while(!I.hitroot())
      {
         topnode = I.item();
         I.remove();
         KBoolLine line(_GC);
         line.Set(link);
         B_INT Y = line.Calculate_Y(topnode->GetX());
         Node * leftnode; //left node of clossest link
         (link->GetBeginNode()->GetX() < link->GetEndNode()->GetX()) ?
               leftnode = link->GetBeginNode():
               leftnode = link->GetEndNode();
         Node *node_A = new Node(topnode->GetX(),Y, _GC);
         KBoolLink *link_A = new KBoolLink(0, leftnode, node_A, _GC);
         KBoolLink *link_B = new KBoolLink(0, node_A, topnode, _GC);
         KBoolLink *link_BB = new KBoolLink(0, topnode, node_A, _GC);
         KBoolLink *link_D = _BI.item()->GetLink();
         link_D->Replace(leftnode,node_A);
         _LI->insbegin(link_A);
         _LI->insbegin(link_B);
         _LI->insbegin(link_BB);
         link_B->SetHoleLink(true);
         link_BB->SetHoleLink(true);
         bool closest_is_hole = link->GetHole();
         link_A->SetHole(closest_is_hole);
         link_B->SetHole(closest_is_hole);
         link_BB->SetHole(closest_is_hole);
         link_A->TakeOverOperationFlags( link );
         link_B->TakeOverOperationFlags( link );
         link_BB->TakeOverOperationFlags( link );
      }
   }
   if (link->IsTopHole() )
   {
      SortTheBeam( atinsert );
      writebeam();
   }      
      
   if (link->IsTopHole() && !_BI.athead() )
   {
     	 // now we check if this hole should now be linked, or later
   	 // we always link on the node with the maximum y value, Why ? because i like it !
   	 // to link we put the node of the hole into the crosslist of the closest link !
       assert( record->Direction() == GO_LEFT );
       if (atinsert)
       {
          if ( link->GetBeginNode()->GetY() <= link->GetEndNode()->GetY() )
          {
            topnode = link->GetEndNode();
            _BI--;
            _BI.item()->GetLine()->AddCrossing(topnode);
            _BI++;
            link->SetTopHole(false);
            foundholes = true;
          }
       }
       else  //remove stage of links from te beam
       {
         topnode = _BI.item()->GetLink()->GetBeginNode();
         _BI--;
         _BI.item()->GetLine()->AddCrossing(topnode);
         _BI++;
         link->SetTopHole(false);
         foundholes = true;
       }
   }
   return foundholes;
}
int recordsorter_ysp_angle(Record* rec1, Record* rec2)
{
	if (rec1->Ysp() > rec2->Ysp() )
		return(1);
	if (rec1->Ysp() < rec2->Ysp() )
		return(-1);
	//it seems they are equal
   B_INT rightY1;
   if (rec1->Direction()==GO_LEFT)
      rightY1 = rec1->GetLink()->GetBeginNode()->GetY();
   else
      rightY1 = rec1->GetLink()->GetEndNode()->GetY();
   B_INT rightY2;
   if (rec2->Direction()==GO_LEFT)
      rightY2 = rec2->GetLink()->GetBeginNode()->GetY();
   else
      rightY2 = rec2->GetLink()->GetEndNode()->GetY();
   if ( rightY1 > rightY2 )
		return(1);
   if ( rightY1 < rightY2 )
     return(-1);
   return(0);
}
int recordsorter_ysp_angle_back(Record* rec1, Record* rec2)
{
	if (rec1->Ysp() > rec2->Ysp() )
		return(1);
	if (rec1->Ysp() < rec2->Ysp() )
		return(-1);
	//it seems they are equal
   B_INT leftY1;
   if ( rec1->Direction() == GO_RIGHT )
      leftY1 = rec1->GetLink()->GetBeginNode()->GetY();
   else
      leftY1 = rec1->GetLink()->GetEndNode()->GetY();
   B_INT leftY2;
   if ( rec2->Direction() == GO_RIGHT )
      leftY2 = rec2->GetLink()->GetBeginNode()->GetY();
   else
      leftY2 = rec2->GetLink()->GetEndNode()->GetY();
   if ( leftY1 > leftY2 )
		return(1);
   if ( leftY1 < leftY2 )
     return(-1);
   return(0);
}
bool swap_crossing_normal(Record *a, Record *b)
{
	if (!a->Equal(b)) // records NOT parallel
   {
		a->GetLine()->Intersect_simple( b->GetLine() );
      return true;
   }
   return false;
}
int ScanBeam::Process_LinkToLink_Crossings()
{
	// sort on y value of next intersection; and find the intersections
	return _BI.cocktailsort( recordsorter_ysp_angle_back, swap_crossing_normal );
}
int ScanBeam::Process_PointToLink_Crossings()
{
	int merges = 0;
	Record* record;
	if (_BI.count() > 1)
	{
	   DL_Iter<Record*> IL = DL_Iter<Record*>(this);
	   IL.toiter(&_BI);
		//from IL search back for close links
		IL--;
		while(!IL.hitroot())
		{
			record=IL.item();
			if (record->Ysp() > _low->GetY()+ _GC->GetInternalMarge())
				break;
			// the distance to the lo/hi node is smaller then the _GC->GetInternalMarge()
			if( (record->GetLink()->GetBeginNode()!= _low) &&
				 (record->GetLink()->GetEndNode()  != _low)
			  )
			{  // the link is not towards the lohi node
				record->GetLine()->AddCrossing(_low);
				merges++;
			}
			IL--;
		}
		//from IL search forward for close links
		IL.toiter(&_BI);
		IL++;
		while(!IL.hitroot())
		{
			record=IL.item();
			if (record->Ysp() < _low->GetY()- _GC->GetInternalMarge())
				break;
			// the distance to the lohi node is smaller then the booleng->Get_Marge()
			if( (record->GetLink()->GetBeginNode()!=_low) &&
				 (record->GetLink()->GetEndNode()  !=_low)
			  )
			{  // the link is not towards the low node
				record->GetLine()->AddCrossing(_low);
				merges++;
			}
			IL++;
		}
	}
	return merges;
}
int ScanBeam::Process_LinkToLink_Flat(KBoolLine* flatline)
{
	int crossfound = 0;
	Record* record;
   DL_Iter<Record*> _BBI = DL_Iter<Record*>();
   _BBI.Attach(this);
   _BBI.toiter(&_BI);
		for(_BI.tohead(); !_BI.hitroot(); _BI++)
		{
			record=_BI.item();
			if (record->Ysp() < (flatline->GetLink()->GetLowNode()->GetY() - _GC->GetInternalMarge()))
				break;//they are sorted so no other can be there
			if ((record->Ysp() > (flatline->GetLink()->GetLowNode()->GetY() - _GC->GetInternalMarge()))
				  &&
				 (record->Ysp() < (flatline->GetLink()->GetHighNode()->GetY() + _GC->GetInternalMarge()))
				)
			{ //it is in between the flat link region
			  //create a new node at ysp and insert it in both the flatlink and the crossing link
				if (
						(record->GetLink()->GetEndNode()  != flatline->GetLink()->GetHighNode()) &&
						(record->GetLink()->GetEndNode()  != flatline->GetLink()->GetLowNode() ) &&
						(record->GetLink()->GetBeginNode()!= flatline->GetLink()->GetHighNode()) &&
						(record->GetLink()->GetBeginNode()!= flatline->GetLink()->GetLowNode() )
					)
				{
				  Node *newnode = new Node(_low->GetX(),_BI.item()->Ysp(), _GC);
				  flatline->AddCrossing(newnode);
				  record->GetLine()->AddCrossing(newnode);
				  crossfound++;
				}
			}
		}
   _BI.toiter(&_BBI);
   _BBI.Detach();
	return crossfound;
}
bool ScanBeam::checksort()
{
	// if empty then just insert
	if (empty())
		return true;
   _BI.tohead();
   Record* prev=_BI.item();
   _BI++;
   while(!_BI.hitroot())
   {
	   Record* curr=_BI.item();
      if (recordsorter_ysp_angle(prev,curr)==-1)
      {
         recordsorter_ysp_angle(prev,curr);
         return false;
      }
      prev=_BI.item();
      _BI++;
   }
   return true;
}
bool ScanBeam::writebeam()
{
#if KBOOL_DEBUG == 1
   FILE* file = _GC->GetLogFile();
   if (file == NULL)
       return true;
	fprintf( file, "# beam %d \n", count() );
   fprintf( file, " low %I64d %I64d \n", _low->GetX() , _low->GetY() );
   fprintf( file, " type %d \n", _type );
	if (empty())
   {  
      fprintf( file, "             empty \n" );
		return true;
   }
   DL_Iter<Record*> _BI( this );
   _BI.tohead();
   while(!_BI.hitroot())
   {
	   Record* cur=_BI.item();
      fprintf( file, " ysp %I64d \n", cur->Ysp() );
      KBoolLink* curl=cur->GetLink();
	   fprintf( file, "             linkbegin %I64d %I64d \n", curl->GetBeginNode()->GetX(), curl->GetBeginNode()->GetY() );
	   fprintf( file, "             linkend %I64d %I64d \n", curl->GetEndNode()->GetX(), curl->GetEndNode()->GetY() );
      _BI++;
   }
#endif
   return true;
}
