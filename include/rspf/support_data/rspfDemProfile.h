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
// $Id: rspfDemProfile.h 9094 2006-06-13 19:12:40Z dburken $

#ifndef rspfDemProfile_HEADER
#define rspfDemProfile_HEADER

#include <vector>
#include <rspf/base/rspfConstants.h>
#include <rspf/support_data/rspfDemPoint.h>

typedef std::vector<rspf_int32> rspfDemElevationVector;

class rspfDemProfile
{
public:

   rspfDemProfile();
   rspfDemProfile(rspfDemProfile const& right);
   rspfDemProfile const& operator=(rspfDemProfile const& right);

   friend std::ostream& operator<<(std::ostream& s, rspfDemProfile& demp);
   friend std::istream& operator>>(std::istream& s, rspfDemProfile& demp);

   // Accessors

   /*!
    *  Row identification number. Typically set to 1.
    */
   rspf_int32 getRowNumber() const;

   /*!
    *  Column identification number. This is the profile sequence number.
    */
   rspf_int32 getColumnNumber() const;

   /*!
    *  Number of elevations in this profile.
    */
   rspf_int32 getNumberOfElevations() const;

   /*!
    *  Location (in ground planimetric coordinates) of the first
    */
   rspfDemPoint const& getProfileLocation() const;

   /*!
    *  Elevation of local datum for the profile.
    *  Value is in units of measure given by the Elevation Units field
    *  found in the DEM header.
    */
   double getLocalDatumElevation() const;

   /*!
    *  Minimum elevation for this profile.
    */
   double getMinimumElev() const;

   /*!
    *  Maximum elevation for this profile.
    */
   double getMaximumElev() const;

   /*!
   *  Elevation values.
   *  From the DEM standard:
   *  "A value in this array would be multiplied by the spatial resolution
   *  value and added to the elevation of the local elevation datum for the
   *  element profile to obtain the elevation for the point."
   */
   rspfDemElevationVector const& getElevations() const;

private:

   rspf_int32 _row;
   rspf_int32 _column;
   rspf_int32 _numberElevations;
   rspfDemPoint _profileLocation;
   double _locDatumElev;
   double _minElev;
   double _maxElev;
   rspfDemElevationVector _elevations;
};

#endif  // #ifndef rspfDemProfile_HEADER
