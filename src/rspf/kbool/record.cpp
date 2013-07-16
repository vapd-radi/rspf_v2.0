/*! \file kbool/src/record.cpp
    \author Probably Klaas Holwerda or Julian Smart
    Copyright: 2001-2004 (C) Probably Klaas Holwerda
    Licence: wxWidgets Licence
    RCS-ID: $Id: record.cpp 9094 2006-06-13 19:12:40Z dburken $
*/
#ifdef __GNUG__
#pragma implementation 
#endif
#include <rspf/kbool/booleng.h>
#include <rspf/kbool/record.h>
#include <rspf/kbool/node.h>
#include <stdlib.h>
#include <math.h>
#define LNK _line.GetLink()
Record::~Record()
{
}
Record::Record(KBoolLink* link,Bool_Engine* GC)
   :_line(GC)
{
   _GC=GC;
	_dir=GO_RIGHT;
   _a=0;
   _b=0;
	_line.Set(link);
	_line.CalculateLineParameters();
}
void Record::SetNewLink(KBoolLink* link)
{
	_line.Set(link);
   _line.CalculateLineParameters();
}
void Record::Calc_Ysp(Node* low)
{
	if ((LNK->GetEndNode() == low) || (LNK->GetBeginNode() == low))
   {
      _ysp=low->GetY();
      return;
   }
   if	(LNK->GetEndNode()->GetX() == LNK->GetBeginNode()->GetX())
         _ysp=low->GetY(); //flatlink only in flatbeams
   else if (LNK->GetEndNode()->GetX() == low->GetX())
         _ysp=LNK->GetEndNode()->GetY();
   else if (LNK->GetBeginNode()->GetX() == low->GetX())
         _ysp=LNK->GetBeginNode()->GetY();
   else
      	_ysp=_line.Calculate_Y_from_X(low->GetX());
}
void Record::Set_Flags()
{
	if (LNK->GetEndNode()->GetX()==LNK->GetBeginNode()->GetX()) //flatlink ?
	{  //only happens in flat beams
		if (LNK->GetEndNode()->GetY() < LNK->GetBeginNode()->GetY())
			_dir=GO_RIGHT;
		else
			_dir=GO_LEFT;
	}
	else
	{
		if (LNK->GetEndNode()->GetX() > LNK->GetBeginNode()->GetX())
			_dir=GO_RIGHT;
		else
			_dir=GO_LEFT;
	}
}
KBoolLink* Record::GetLink()
{
	return LNK;
}
B_INT Record::Ysp()
{
	return _ysp;
}
void Record::SetYsp(B_INT ysp)
{
	_ysp=ysp;
}
DIRECTION Record::Direction()
{
	return DIRECTION(_dir);
}
bool Record::Calc_Left_Right(Record* record_above_me)
{
   bool par=false;
   if (!record_above_me)   //null if no record above
   { 	_a=0;_b=0;  }
   else
   {
   	_a=record_above_me->_a;
   	_b=record_above_me->_b;
   }
   switch (_dir&1)
   {
      case GO_LEFT	:	if (LNK->Group() == GROUP_A)
                        {
                           LNK->SetRightA((bool)(_a>0));
                           if (_GC->GetWindingRule())
                              LNK->GetInc() ? _a++ : _a--;
                           else
                           {  //ALTERNATE
                              if (_a)
                                 _a=0;
                              else
                                 _a=1;
                           }
                           LNK->SetLeftA((bool)(_a>0));
                           LNK->SetLeftB((bool)(_b>0));
                           LNK->SetRightB((bool)(_b>0));
                        }
                        else
                        {
                           LNK->SetRightA((bool)(_a > 0));
                           LNK->SetLeftA((bool)(_a>0));
                           LNK->SetRightB((bool)(_b>0));
                           if (_GC->GetWindingRule())
                              LNK->GetInc() ? _b++ : _b--;
                           else //ALTERNATE
                           {
                               if (_b)
                                    _b=0;
                               else
                                    _b=1;
                           }
                           LNK->SetLeftB((bool)(_b>0));
                        }
                           break;
      case	GO_RIGHT	:	if (LNK->Group() == GROUP_A)
                        {
                           LNK->SetLeftA((bool)(_a>0));
                           if (_GC->GetWindingRule())
                              LNK->GetInc() ? _a++ : _a--;
                           else
                           {  //ALTERNATE
                              if (_a)
                                 _a=0;
                              else
                                 _a=1;
                           }
                           LNK->SetRightA((bool)(_a>0));
                           LNK->SetLeftB((bool)(_b>0));
                           LNK->SetRightB((bool)(_b>0));
                        }
                        else
                        {
                           LNK->SetRightA((bool)(_a>0));
                           LNK->SetLeftA((bool)(_a>0));
                           LNK->SetLeftB((bool)(_b>0));
                           if (_GC->GetWindingRule())
                              LNK->GetInc() ? _b++ : _b--;
                           else
                           {  //ALTERNATE
                              if (_b)
                                 _b=0;
                              else
                                 _b=1;
                           }
                           LNK->SetRightB((bool)(_b>0));
                        }
                        break;
      default			:  _GC->error("Undefined Direction of link","function IScanBeam::Calc_Set_Left_Right()");
                        break;
   }
/*
   switch (_dir&1)
   {
      case GO_LEFT	:	if (LNK->Group() == GROUP_A)
                        {
                           LNK->SetRightA((bool)(_a>0));
                           if (booleng->Get_WindingRule())
                              LNK->GetInc() ? _a++ : _a--;
                           else
                              _a--;
                           LNK->SetLeftA((bool)(_a>0));
                           LNK->SetLeftB((bool)(_b>0));
                           LNK->SetRightB((bool)(_b>0));
                        }
                        else
                        {
                           LNK->SetRightA((bool)(_a > 0));
                           LNK->SetLeftA((bool)(_a>0));
                           LNK->SetRightB((bool)(_b>0));
                           if (booleng->Get_WindingRule())
                              LNK->GetInc() ? _b++ : _b--;
                           else
                              _b--;
                           LNK->SetLeftB((bool)(_b>0));
                        }
                           break;
      case	GO_RIGHT	:	if (LNK->Group() == GROUP_A)
                           {
                              LNK->SetLeftA((bool)(_a>0));
                              if (booleng->Get_WindingRule())
                                 LNK->GetInc() ? _a++ : _a--;
                              else
                                 _a++;
                              LNK->SetRightA((bool)(_a>0));
                              LNK->SetLeftB((bool)(_b>0));
                              LNK->SetRightB((bool)(_b>0));
                           }
                        else
                           {
                              LNK->SetRightA((bool)(_a>0));
                              LNK->SetLeftA((bool)(_a>0));
                              LNK->SetLeftB((bool)(_b>0));
                              if (booleng->Get_WindingRule())
                                 LNK->GetInc() ? _b++ : _b--;
                              else
                                 _b++;
                              LNK->SetRightB((bool)(_b>0));
                           }
                        break;
      default			:  _messagehandler->error("Undefined Direction of link","function IScanBeam::Calc_Set_Left_Right()");
                        break;
   }
*/
   if (record_above_me && Equal(record_above_me))
   {
      par=true;
		LNK->Mark();
   	record_above_me->_a=_a;
   	record_above_me->_b=_b;
		if (Direction()== GO_LEFT)
		{
         if (record_above_me->Direction()== GO_LEFT)
         {
            record_above_me->LNK->SetLeftA(LNK->GetLeftA());
            record_above_me->LNK->SetLeftB(LNK->GetLeftB());
         }
         else
         {
            record_above_me->LNK->SetRightA(LNK->GetLeftA());
            record_above_me->LNK->SetRightB(LNK->GetLeftB());
         }
		}
		else
		{
         if (record_above_me->Direction()== GO_LEFT)
         {
            record_above_me->LNK->SetLeftA(LNK->GetRightA());
            record_above_me->LNK->SetLeftB(LNK->GetRightB());
         }
         else
         {
            record_above_me->LNK->SetRightA(LNK->GetRightA());
            record_above_me->LNK->SetRightB(LNK->GetRightB());
         }
		}
   }
   return par;
}
bool Record::Equal(Record *a)
{
	return((bool)( ( LNK->GetOther(a->LNK->GetBeginNode()) == a->LNK->GetEndNode())  &&
						  ( LNK->GetOther(a->LNK->GetEndNode())   == a->LNK->GetBeginNode())  ));
}
KBoolLine* Record::GetLine()
{
	return &_line;
}
