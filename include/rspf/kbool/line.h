/*! \file kbool/include/kbool/line.h
    \brief Mainy used for calculating crossings
    \author Probably Klaas Holwerda
    Copyright: 2001-2004 (C) Probably Klaas Holwerda
    Licence: wxWidgets Licence
    RCS-ID: $Id: line.h 14798 2009-06-29 19:19:22Z dburken $
*/
#ifndef LINE_H
#define LINE_H
#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface
#endif
#include <rspf/kbool/booleng.h>
#include <rspf/kbool/link.h>
class Bool_Engine;
enum PointStatus {LEFT_SIDE, RIGHT_SIDE, ON_AREA, IN_AREA};
class A2DKBOOLDLLEXP Graph;
class A2DKBOOLDLLEXP KBoolLine
{
   protected:
      Bool_Engine* m_GC;
	public:
		// constructors and destructor
		KBoolLine(Bool_Engine* GC);
		KBoolLine(KBoolLink*,Bool_Engine* GC);
		~KBoolLine();
		void			Set(KBoolLink *);
		KBoolLink*		GetLink();
		Node*		GetBeginNode();		
      
		Node*		GetEndNode();   										  	
		int					CheckIntersect(KBoolLine*, double Marge);	
		int					Intersect(KBoolLine*, double Marge);    	 
		int               Intersect_simple(KBoolLine * lijn);
		bool				   Intersect2(Node* crossing,KBoolLine * lijn);
		PointStatus			PointOnLine(Node* a_node, double& Distance, double Marge ); 
		PointStatus			PointInLine(Node* a_node, double& Distance, double Marge ); 
      B_INT					Calculate_Y(B_INT X); 								
		B_INT         		Calculate_Y_from_X(B_INT X);
		void              Virtual_Point( LPoint *a_point, double distance);
		KBoolLine& 			operator=(const KBoolLine&); 					
		Node* 				OffsetContour(KBoolLine* const nextline,Node* last_ins, double factor,Graph *shape);
		Node* 				OffsetContour_rounded(KBoolLine* const nextline,Node* _last_ins, double factor,Graph *shape);
		bool 				   OkeForContour(KBoolLine* const nextline,double factor,Node* LastLeft,Node* LastRight, LinkStatus& _outproduct);
		bool				   Create_Ring_Shape(KBoolLine* nextline,Node** _last_ins_left,Node** _last_ins_right,double factor,Graph *shape);
		void 					Create_Begin_Shape(KBoolLine* nextline,Node** _last_ins_left,Node** _last_ins_right,double factor,Graph *shape);
		void 					Create_End_Shape(KBoolLine* nextline,Node* _last_ins_left,Node* _last_ins_right,double factor,Graph *shape);
		void  CalculateLineParameters(); 								
		void  AddLineCrossing(B_INT , B_INT , KBoolLine *); 		
		void  AddCrossing(Node *a_node);
		Node* AddCrossing(B_INT X, B_INT Y);
		bool  ProcessCrossings(TDLI<KBoolLink>* _LI);
		void	SortLineCrossings();
		bool	CrossListEmpty();
		DL_List<void*>*		GetCrossList();
	private:
		int   ActionOnTable1(PointStatus,PointStatus); 							
		int   ActionOnTable2(PointStatus,PointStatus); 						
		double 		m_AA;
		double		m_BB;
		double		m_CC;
		KBoolLink*	m_link;
		bool		m_valid_parameters;
		DL_List<void*>	 *linecrosslist;
};
#endif
