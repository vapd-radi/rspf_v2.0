//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id$
#ifndef rspfDuration_HEADER
#define rspfDuration_HEADER
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfReferenced.h>
#include <iostream>

class RSPF_DLL rspfDuration
{
public:
   /**
    * This will take an iso8601 encoded duration string and parse out
    * the individual values
    */
   rspfDuration(const rspfString& iso8601Duration = rspfString(""));
   
   
   /**
    * zero out all fields
    */
   void clearFields();
   
   /**
    * This will take an iso8601 encoded duration stream and parse out
    * the individual values it will stop when a blank character or whitespace is found
    */
   bool readIso8601Encoding(std::istream& in);
   
   /**
    * This will take an iso8601 encoded duration stream and parse out
    * the individual values it will stop when a blank character or whitespace is found
    */
   bool setByIso8601DurationString(const rspfString& iso8601Duration);
   
   /**
    * Will take the field values and encode into a iso8601 string format.
    * Note, anything that is 0 will not be output.
    *
    */
   void toIso8601DurationString(rspfString& result);
   
   /**
    * this will not use the months field or the years field but will use all other
    * fields to calculate a total value in seconds.  We can not determine leap years
    * and how many days are in a month so those are omitted and so this serves as
    * a utility method to just calculate the total seconds if you give a duration string
    * that contains only one or all or any of the following: weeks, days, minutes, hours,
    * and/or seconds.
    *
    * If the sign is set to negative it will return a negative value.
    */
   rspf_float64 toSeconds()const;
   
   void setSign(rspf_int32 value)
   {
      theSign = ((value < 0)?-1:1);
   }
   /**
    * returns the sign.  Should be either -1 or 1 for the return
    */
   rspf_int32 sign()const
   {
      return theSign;
   }
   void setYears(rspf_uint64 value)
   {
      theYears = value;
   }
   rspf_int64 years()const
   {
      return theYears;
   }
   void setMonths(rspf_uint64 value)
   {
      theMonths = value;
   }
   rspf_int64 months()const
   {
      return theMonths;
   }
   void setWeeks(rspf_uint64 value)
   {
      theWeeks = value;
   }
   rspf_int64 weeks()const
   {
      return theWeeks;
   }
   void setDays(rspf_uint64 value)
   {
      theDays = value;
   }
   rspf_int64 days()const
   {
      return theDays;
   }
   void setHours(rspf_uint64 value)
   {
      theHours = value;
   }
   rspf_int64 hours()const
   {
      return theHours;
   }
   void setMinutes(rspf_uint64 value)
   {
      theMinutes = value;
   }
   rspf_int64 minutes()const
   {
      return theMinutes;
   }
   void setSeconds(rspf_float64 value)
   {
      theSeconds = value;
   }
   rspf_float64 seconds()const
   {
      return theSeconds;
   }
   
   /**
    * Sets all values in one call
    */
   void setAll(rspf_int32 signValue,
               rspf_uint64 yearsValue,
               rspf_uint64 monthsValue,
               rspf_uint64 weeksValue,
               rspf_uint64 daysValue,
               rspf_uint64 hoursValue,
               rspf_uint64 minutesValue,
               rspf_float64 secondsValue)
   {
      theSign    = ((signValue < 0)?-1:1);
      theYears   = yearsValue;
      theMonths  = monthsValue;
      theWeeks   = weeksValue;
      theDays    = daysValue;
      theHours   = hoursValue;
      theMinutes = minutesValue;
      theSeconds = secondsValue;
   }
               
   
protected:
   rspf_int32 theSign; // indicates -1 for negative and anything else is positive
   rspf_int64 theYears; // number of years
   rspf_int64 theMonths; // number of months
   rspf_int64 theWeeks; // number of months
   rspf_int64 theDays;   // number of days
   rspf_int64 theHours;  // number of hours
   rspf_int64 theMinutes; // number of minutes
   rspf_float64 theSeconds; // umber of Seconds
};

#endif
