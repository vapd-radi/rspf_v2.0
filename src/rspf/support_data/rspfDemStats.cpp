//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Ken Melero
//         Orginally written by Jamie Moyers (jmoyers@geeks.com)
//         Adapted from the package KDEM.
// Description: This class provides some statistics for DEMs.
//
//********************************************************************
// $Id: rspfDemStats.cpp 17206 2010-04-25 23:20:40Z dburken $

#include <stdlib.h>

#include <rspf/support_data/rspfDemStats.h>
#include <rspf/support_data/rspfDemUtil.h>
using namespace std;

rspfDemStats::rspfDemStats()
   : _availCodeDE2(0),
     _rsmeDE2x(0),
     _rsmeDE2y(0),
     _rsmeDE2z(0),
     _sampleSizeDE2(0),
     _availCodeDE5(0),
     _rsmeDE5x(0),
     _rsmeDE5y(0),
     _rsmeDE5z(0),
     _sampleSizeDE5(0)

{
}

rspfDemStats::~rspfDemStats()
{
}

long
rspfDemStats::getAvailCodeDE2() const
{
   return _availCodeDE2;
}

long
rspfDemStats::getRSMEDE2x() const
{
   return _rsmeDE2x;
}

long
rspfDemStats::getRSMEDE2y() const
{
   return _rsmeDE2y;
}

long
rspfDemStats::getRSMEDE2z() const
{
   return _rsmeDE2z;
}

long
rspfDemStats::getSampleSizeDE2() const
{
   return _sampleSizeDE2;
}

long
rspfDemStats::getAvailCodeDE5() const
{
   return _availCodeDE5;
}

long
rspfDemStats::getRSMEDE5x() const
{
   return _rsmeDE5x;
}

long
rspfDemStats::getRSMEDE5y() const
{
   return _rsmeDE5y;
}

long
rspfDemStats::getRSMEDE5z() const
{
   return _rsmeDE5z;
}

long
rspfDemStats::getSampleSizeDE5() const
{
   return _sampleSizeDE5;
}

ostream&
operator<<(ostream& s,  rspfDemStats const& /* stats */)
{
   return s;
}

istream&
operator>>(istream& s, rspfDemStats& stats)
{
   char bufstr[1024];

   rspfDemUtil::getRecord(s, bufstr);

   stats._availCodeDE2 = rspfDemUtil::getLong(bufstr, 0, 6);
   stats._rsmeDE2x = rspfDemUtil::getLong(bufstr, 6, 6);
   stats._rsmeDE2y = rspfDemUtil::getLong(bufstr, 12, 6);
   stats._rsmeDE2z = rspfDemUtil::getLong(bufstr, 18, 6);
   stats._sampleSizeDE2 = rspfDemUtil::getLong(bufstr, 24, 6);
   stats._availCodeDE5 = rspfDemUtil::getLong(bufstr, 30, 6);
   stats._rsmeDE5x = rspfDemUtil::getLong(bufstr, 36, 6);
   stats._rsmeDE5y = rspfDemUtil::getLong(bufstr, 42, 6);
   stats._rsmeDE5z = rspfDemUtil::getLong(bufstr, 48, 6);
   stats._sampleSizeDE5 = rspfDemUtil::getLong(bufstr, 54, 6);

   return s;
}
