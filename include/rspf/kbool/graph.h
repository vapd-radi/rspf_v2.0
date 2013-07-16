/*! \file kbool/include/kbool/graph.h
    \author Probably Klaas Holwerda
    Copyright: 2001-2004 (C) Probably Klaas Holwerda
    Licence: wxWidgets Licence
    RCS-ID: $Id: graph.h 9094 2006-06-13 19:12:40Z dburken $
*/
/* @@(#) $Source$ $Revision: 9094 $ $Date: 2006-06-14 03:12:40 +0800 (星期三, 14 六月 2006) $ */
/*
Program	GRAPH.H
Purpose	Used to Intercect and other process functions
Last Update	03-04-1996
*/
#ifndef GRAPH_H
#define GRAPH_H
#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface
#endif
#include <rspf/kbool/booleng.h>
#include <rspf/kbool/_lnk_itr.h>
#include <rspf/kbool/link.h>
#include <rspf/kbool/line.h>
#include <rspf/kbool/scanbeam.h>
class Node;
class GraphList;
class A2DKBOOLDLLEXP Graph
{
   protected:
      Bool_Engine* _GC;
	public:
      Graph(Bool_Engine* GC);
      Graph(KBoolLink*,Bool_Engine* GC);
      Graph( Graph* other );
      ~Graph();
      bool		 GetBin() 							{ return _bin; };
      void		 SetBin(bool b) 					{ _bin = b; };
		void 		 Prepare( int intersectionruns );
		void      RoundInt(B_INT grid);
		void 		 Rotate(bool plus90);
		void		 AddLink(Node *begin,Node *end);
		void		 AddLink(KBoolLink *a_link);  
      
		bool		 AreZeroLines(B_INT Marge);	
		void		 DeleteDoubles();             
		bool		 DeleteZeroLines(B_INT Marge);
		bool 		 RemoveNullLinks();
		void		 ProcessCrossings();        					
		void      Set_Operation_Flags();                   
		//! Left Right values																		
		void		 Remove_IN_Links();						
      void ResetBinMark();
		// Remove unused links
		void		 ReverseAllLinks();
		bool		 Simplify( B_INT Marge );			 		
		bool 		 Smoothen( B_INT Marge);
		void		 TakeOver(Graph*);                        
		//! function for maximum performance
		KBoolLink*	GetFirstLink();
		Node*			GetTopNode();
		void			SetBeenHere(bool);
	   void 			Reset_flags();
		void			SetGroup(GroupType);					
		void			SetNumber(int);				
		void 			Reset_Mark_and_Bin();
		bool		   GetBeenHere();
		int			GetGraphNum();
		int			GetNumberOfLinks();
		void        Boolean(BOOL_OP operation,GraphList* Result);
		void        Correction(GraphList* Result,double factor);
		void        MakeRing(GraphList* Result,double factor);
		void        CreateRing(GraphList *ring,double factor);
		void        CreateRing_fast(GraphList *ring,double factor);
		void        CreateArc(Node* center, KBoolLine* incoming, Node* end,double radius,double aber);
		void        CreateArc(Node* center, Node* begin, Node* end,double radius,bool clock,double aber);
		void        MakeOneDirection();
		void        Make_Rounded_Shape(KBoolLink* a_link, double factor);
		bool 		   MakeClockWise();
		bool 		   writegraph(bool linked);
		bool 		   writeintersections();
      void        WriteKEY( Bool_Engine* GC,  FILE* file = NULL );
      void        WriteGraphKEY( Bool_Engine* GC );
   
	protected:
      /*     
         Links are sorted in XY at beginpoint. Bin and mark flag are reset.
         Next start to collect subparts from the graph, setting the links bin for all found parts.
         The parts are searched starting at a topleft corner NON set bin flag link.
         Found parts are numbered, to be easily split into to real sperate graphs by Split()
      
         \param operation operation to collect for.
         \param detecthole if you want holes detected, influences also way of extraction.
         \param foundholes when holes are found this flag is set true, but only if detecthole is set true.
      */
		void 	Extract_Simples(BOOL_OP operation, bool detecthole, bool& foundholes );
		void 	Split(GraphList* partlist);
   	//! Collect a graph by starting at argument link
      /*
         Called from Extract_Simples, and assumes sorted links with bin flag unset for non extarted piece
         Collect graphs pieces from a total graph, by following links set to a given boolean operation.
         \param current_node start node to collect
         \param operation operation to collect for.
         \param detecthole if you want holes detected, influences also way of extraction.
         \param graphnumber number to be given to links in the extracted graph piece
         \param foundholes when holes are found this flag is set true.
      */
		void 	CollectGraph(Node *current_node, BOOL_OP operation, bool detecthole,int graphnumber, bool& foundholes );
		void 	CollectGraphLast(Node *current_node, BOOL_OP operation, bool detecthole,int graphnumber, bool& foundholes );
      /*!
         Last found position is used to find it quickly.
         Used in ExtractSimples()
      */
		Node*	GetMostTopLeft(TDLI<KBoolLink>* _LI);
      /*
         It is not just crossings calculation, snapping close nodes is part of it.
         This is not done at maximum stability in economic time.
         There are faster ways, but hardly ever they solve the problems, and they do not snap.
         Here it is on purpose split into separate steps, to get a better result in snapping, and
         to reach a better stability.
         \param Marge nodes and lines closer to eachother then this, are merged.
      */
		bool CalculateCrossings(B_INT Marge);
		int	Merge_NodeToNode(B_INT Marge);
      /*!
          \param scantype a different face in the algorithm.
          \param holes to detect hole when needed.
      */
		int ScanGraph2( SCANTYPE scantype, bool& holes );
		void 	DeleteNonCond(BOOL_OP operation);
      void HandleNonCond(BOOL_OP operation);
	   bool 	checksort();
		bool 	Small(B_INT howsmall);
		bool	_bin;
		DL_List<void*>* _linklist;
};
#endif
