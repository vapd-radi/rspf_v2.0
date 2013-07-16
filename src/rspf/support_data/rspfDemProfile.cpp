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
// Description: This class defines a DEM profile.
//
//********************************************************************
// $Id: rspfDemProfile.cpp 17206 2010-04-25 23:20:40Z dburken $

#include <cstdlib>
#include <iostream>
#include <sstream>

#include <rspf/support_data/rspfDemProfile.h>
#include <rspf/support_data/rspfDemUtil.h>

rspfDemProfile::rspfDemProfile()
   : _row(0),
     _column(0),
     _numberElevations(0),
     _locDatumElev(0.0),
     _minElev(0.0),
     _maxElev(0.0)
{
}

rspfDemProfile::rspfDemProfile(rspfDemProfile const& right)
{
   operator=(right);
}

rspfDemProfile const&
rspfDemProfile::operator=(rspfDemProfile const& right)
{
   if (this == &right)
      return *this;

   _row = right._row;
   _column = right._column;
   _numberElevations = right._numberElevations;
   _profileLocation = right._profileLocation;
   _locDatumElev = right._locDatumElev;
   _minElev = right._minElev;
   _maxElev = right._maxElev;
   _elevations = right._elevations;

   return *this;
}

rspf_int32
rspfDemProfile::getRowNumber() const
{
   return _row;
}

rspf_int32
rspfDemProfile::getColumnNumber() const
{
   return _column;
}

rspf_int32
rspfDemProfile::getNumberOfElevations() const
{
   return _numberElevations;
}

rspfDemPoint const&
rspfDemProfile::getProfileLocation() const
{
   return _profileLocation;
}

double
rspfDemProfile::getLocalDatumElevation() const
{
   return _locDatumElev;
}

double
rspfDemProfile::getMinimumElev() const
{
   return _minElev;
}

double
rspfDemProfile::getMaximumElev() const
{
   return _maxElev;
}

rspfDemElevationVector const&
rspfDemProfile::getElevations() const
{
   return _elevations;
}

std::istream&
operator>>(std::istream& s, rspfDemProfile& demp)
{
   //   string bufstr;
   char bufstr[1024];

   rspfDemUtil::getRecord(s,bufstr);

   demp._row = rspfDemUtil::getLong(bufstr, 0, 6);
   demp._column = rspfDemUtil::getLong(bufstr, 6, 6);
   demp._numberElevations = rspfDemUtil::getLong(bufstr, 12, 6);

   double x,y;
   rspfDemUtil::getDouble(bufstr, 24, 24, x);
   rspfDemUtil::getDouble(bufstr, 48, 24, y);
   demp._profileLocation.setXY(x,y);

   rspfDemUtil::getDouble(bufstr, 72, 24, demp._locDatumElev);
   rspfDemUtil::getDouble(bufstr, 96, 24, demp._minElev);
   rspfDemUtil::getDouble(bufstr, 120, 24, demp._maxElev);

   demp._elevations.erase(demp._elevations.begin(), demp._elevations.end());   
   demp._elevations.reserve(demp._numberElevations);

   // Extract elevations in this record.
   rspf_int32 count = 0;
   while ((count < 146) && (count < demp._numberElevations))
   {
      demp._elevations.push_back(rspfDemUtil::getLong(bufstr, 144+(count*6), 6));
      count++;
   }
   
   rspf_int32 t;
   while (count < demp._numberElevations)
   {
      t = (count - 146) % 170;
      if (t == 0)
         rspfDemUtil::getRecord(s,bufstr);
      demp._elevations.push_back(rspfDemUtil::getLong(bufstr, t*6, 6));
      count++;
   }

   return s;
}


std::ostream& operator<<(std::ostream& s, rspfDemProfile& /* demp */)
{
   return s;
}


