/*! \file kbool/src/graphlst.cpp
    \brief Implements a list of graphs
    \author Probably Klaas Holwerda 
    Copyright: 2001-2004 (C) Probably Klaas Holwerda 
    Licence: wxWidgets Licence
    RCS-ID: $Id: graphlst.cpp 9094 2006-06-13 19:12:40Z dburken $
*/
#ifdef __GNUG__
#pragma implementation 
#endif
#include <rspf/kbool/booleng.h>
#include <rspf/kbool/graphlst.h>
int 	graphsorterX( Graph *, Graph * );
int 	graphsorterY( Graph *, Graph * );
GraphList::GraphList(Bool_Engine* GC) 
{
   _GC=GC;
}
GraphList::GraphList( GraphList* other ) 
{
   _GC = other->_GC;
   TDLI<Graph> _LI = TDLI<Graph>( other );
	_LI.tohead();
	while (!_LI.hitroot())
	{
      insend( new Graph( _LI.item() ) );
		_LI++;
	}
}
GraphList::~GraphList()
{
   clearListAndBoolEngine();
}
void GraphList::clearList()
{
   if(!empty())
   {
      TDLI<Graph> _LI=TDLI<Graph>(this);
      _LI.delete_all();
   }
}
void GraphList::clearListAndBoolEngine()
{
   if(!empty())
   {
      TDLI<Graph> _LI=TDLI<Graph>(this);
      _LI.delete_all();
   }
   _GC = 0;
}
void GraphList::Prepare(Graph* total)
{
   if (empty())
      return;
	_GC->SetState("Simplify");
	// Simplify all graphs in the list
	Simplify( (double) _GC->GetGrid() );
   if ( ! _GC->GetOrientationEntryMode() )
   {
      TDLI<Graph> _LI=TDLI<Graph>(this);
      _LI.tohead();
      while (!_LI.hitroot())
      {
			_LI.item()->MakeClockWise();
         _LI++;
      }
   }
	Renumber();
	//the graplist contents will be transferred to one graph
	MakeOneGraph(total);
}
void GraphList::MakeOneGraph(Graph* total)
{
   TDLI<Graph> _LI=TDLI<Graph>(this);
	_LI.tohead();
	while(!_LI.hitroot())
	{
		total->TakeOver(_LI.item());
		delete _LI.item();
		_LI.remove();
	}
}
void GraphList::Renumber()
{
   if ( _GC->GetOrientationEntryMode() )
   {
      TDLI<Graph> _LI=TDLI<Graph>(this);
	   _LI.tohead();
	   while (!_LI.hitroot())
	   {
         if ( _LI.item()->GetFirstLink()->Group() == GROUP_A )
		      _LI.item()->SetNumber(1);
         else
		      _LI.item()->SetNumber(2);
		   _LI++;
	   }
   }
   else
   {
	   unsigned int Number = 1;
      TDLI<Graph> _LI=TDLI<Graph>(this);
	   _LI.tohead();
	   while (!_LI.hitroot())
	   {
		   _LI.item()->SetNumber(Number++);
		   _LI++;
	   }
   }
}
void GraphList::Simplify(double marge)
{
   TDLI<Graph> _LI=TDLI<Graph>(this);
	_LI.foreach_mf(&Graph::Reset_Mark_and_Bin);
	_LI.tohead();
	while (!_LI.hitroot())
	{
		if (_LI.item()->Simplify( (B_INT) marge))
      { 
			if (_LI.item()->GetNumberOfLinks() < 3)
				// delete this graph from the graphlist
         {
            delete _LI.item();
            _LI.remove();
         }
      }
		else
			_LI++;
	}
}
void GraphList::Smoothen(double marge)
{
   TDLI<Graph> _LI=TDLI<Graph>(this);
	_LI.foreach_mf(&Graph::Reset_Mark_and_Bin);
	_LI.tohead();
	while (!_LI.hitroot())
	{
		if (_LI.item()->Smoothen( (B_INT) marge))
      {
			if (_LI.item()->GetNumberOfLinks() < 3)
			// delete this graph from the graphlist
         {
            delete _LI.item();
            _LI.remove();
         }
      }
		else
			_LI++;
	}
}
void GraphList::UnMarkAll()
{
   TDLI<Graph> _LI=TDLI<Graph>(this);
	_LI.foreach_mf(&Graph::Reset_Mark_and_Bin);
}
void GraphList::Correction()
{
   TDLI<Graph> _LI=TDLI<Graph>(this);
	int todo=_LI.count();
   if ( _GC->GetInternalCorrectionFactor()) //not zero
   {
      _LI.tohead();
      for(int i=0; i<todo ; i++)
      {
         GraphList *_correct=new GraphList(_GC);
         {
            _LI.item()->MakeClockWise();
            _LI.item()->Correction(_correct,_GC->GetInternalCorrectionFactor());
            delete _LI.item();
            _LI.remove();
            while (!_correct->empty())
            {
               _LI.insend((Graph*)_correct->headitem());
               _correct->removehead();
            }
         }
         delete _correct;
      }
   }  
   }
void GraphList::MakeRings()
{
   TDLI<Graph> _LI=TDLI<Graph>(this);
	int todo=_LI.count();
	_LI.tohead();
	for(int i=0; i<todo ; i++)
	{
		//the input graph will be empty in the end
		GraphList *_ring=new GraphList(_GC);
		{
			_LI.item()->MakeClockWise();
			_LI.item()->MakeRing(_ring,_GC->GetInternalCorrectionFactor());
         delete _LI.item();
			_LI.remove();
			//move created rings graphs to this
			while (!_ring->empty())
			{
				//add to end
				((Graph*)_ring->headitem())->MakeClockWise();
				_LI.insend((Graph*)_ring->headitem());
				_ring->removehead();
			}
		}
		delete _ring;
	}
}
void GraphList::Merge()
{
   if (count()<=1)
      return;
   {
      TDLI<Graph> _LI=TDLI<Graph>(this);
      _LI.tohead();
      while (!_LI.hitroot())
      {
         _LI.item()->SetGroup(GROUP_A);
         _LI++;
      }
   }
   Graph* _tomerge=new Graph(_GC);
   Renumber();
   MakeOneGraph(_tomerge);
   _tomerge->Prepare(1);
   _tomerge->Boolean(BOOL_OR,this);
	delete _tomerge;
}
#define TRIALS 30  
#define SAVEME 1
void GraphList::Boolean(BOOL_OP operation, int intersectionRunsMax )
{
	_GC->SetState("Performing Boolean Operation");
   if (count()==0)
         return;
	Graph* _prepared = new Graph(_GC);
   if (empty())
      return;
   
	_GC->SetState("Simplify");
   int intersectionruns = 1;
   while ( intersectionruns <= intersectionRunsMax )
   {
      try
	   {
         Prepare( _prepared );
         if (_prepared->GetNumberOfLinks())
         {
            _GC->SetState("prepare");
            _prepared->Prepare( intersectionruns );
            _prepared->Boolean(operation,this);
         }
         intersectionruns = intersectionRunsMax +1;
      }
	   catch (Bool_Engine_Error& error)
	   {
#if BOOL_DEBUG
         _prepared->WriteGraphKEY(_GC);
#endif
         intersectionruns++;
         if ( intersectionruns == intersectionRunsMax )
         {
            _prepared->WriteGraphKEY(_GC);
            _GC->info(error.GetErrorMessage(), "error");
		      throw error;
         }
      }
      catch(...)
      {
#if BOOL_DEBUG
         _prepared->WriteGraphKEY(_GC);
#endif
         intersectionruns++;
         if ( intersectionruns == intersectionRunsMax )
         {
            _prepared->WriteGraphKEY(_GC);
            _GC->info("Unknown exception", "error");
		      throw;
         }
      }
   }
	delete _prepared;
}
void GraphList::WriteGraphs()
{
   TDLI<Graph> _LI=TDLI<Graph>(this);
	_LI.tohead();
	while(!_LI.hitroot())
	{
        _LI.item()->writegraph( false );
        _LI++;
	}
}
void GraphList::WriteGraphsKEY( Bool_Engine* GC )
{
   FILE* file = fopen("graphkeyfile.key", "w");
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
   TDLI<Graph> _LI=TDLI<Graph>(this);
	_LI.tohead();
	while(!_LI.hitroot())
	{
        _LI.item()->WriteKEY( GC, file );
        _LI++;
	}
   fprintf(file,"\
      ENDSTR top; \
      ENDLIB; \
   ");
   fclose (file);
}
