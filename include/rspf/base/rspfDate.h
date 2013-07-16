//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfDate.h 19704 2011-05-31 22:20:40Z dburken $
#ifndef rspfDate_HEADER
#define rspfDate_HEADER
#include <ctime>
#include <iosfwd>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfXmlNode.h>

class RSPF_DLL rspfLocalTm : public std::tm
{
public:
   friend RSPF_DLL std::ostream& operator<<(std::ostream& out, const rspfLocalTm& src);
   friend RSPF_DLL int operator== (const rspfLocalTm& t1,
                                    const rspfLocalTm& src);
   friend RSPF_DLL int operator!= (const rspfLocalTm& t1,
                                    const rspfLocalTm& src);
   friend RSPF_DLL int operator<  (const rspfLocalTm& t1,
                                    const rspfLocalTm& src);
   friend RSPF_DLL int operator<= (const rspfLocalTm& t1,
                                    const rspfLocalTm& src);
   friend RSPF_DLL int operator>  (const rspfLocalTm& t1,
                                    const rspfLocalTm& src);
   friend RSPF_DLL int operator>= (const rspfLocalTm& t1,
                                    const rspfLocalTm& src);
   
   rspfLocalTm (time_t t=0);     // Set to time, 0 is magic for 'now'
   rspfLocalTm (tm const & t); // Copy constructor
   
   rspfLocalTm & operator= (tm const & t);            // Assignment
   operator time_t (void) const;        // Conversion operator
   
   int isValid (void) const;             // Test for validity
   
   void now();
   int compare (rspfLocalTm const & t) const;      // Compare times
   int compare (std::time_t const tt) const;       // Compare times
   
   enum rspfLocalTmFormat     // Date format flags
   {
      rspfLocalTmFormatYear          = (int)0x0001,            // Print year
      rspfLocalTmFormatYearShort     = (int)0x0002, // Print last two digits
      rspfLocalTmFormatPadYear       = (int)0x0004,    // Pad year to 2 or 4
      rspfLocalTmFormatZeroYear      = (int)0x0008,        // Zero fill year
      rspfLocalTmFormatMonth         = (int)0x0010,           // Print month
      rspfLocalTmFormatMonText       = (int)0x0020,   // Print month in text
      rspfLocalTmFormatPadMon        = (int)0x0040, // Pad to 2 (trunc to 3)
      rspfLocalTmFormatZeroMon       = (int)0x0080,       // Zero fill month
      rspfLocalTmFormatDay           = (int)0x0100,            // Print date
      rspfLocalTmFormatDayOfWeek     = (int)0x0200,     // Print day of week
      rspfLocalTmFormatPadDay        = (int)0x0400,         // Pad date to 2
      rspfLocalTmFormatZeroDay       = (int)0x0800,         // Zero fill day
      rspfLocalTmFormatDMY           = (int)0x0111, // Print date, mth, year
      rspfLocalTmFormatPadDMY        = (int)0x0444,         // Pad all three
      rspfLocalTmFormatZeroDMY       = (int)0x0888,   // Zero fill all three
      rspfLocalTmFormatYearFirst     = (int)0x1000,      // Print year first
      rspfLocalTmFormatMonFirst      = (int)0x2000,     // Print month first
      rspfLocalTmFormatSepChar       = (int)0x4000,// Separate fields datech
      rspfLocalTmFormatSepSpace      = (int)0x8000,  // Separate fields space
      rspfLocalTmFormatInternational = (int)(rspfLocalTmFormatDMY|rspfLocalTmFormatMonText|rspfLocalTmFormatPadMon|
                                              rspfLocalTmFormatSepSpace),
      rspfLocalTmFormatIntlShort     = (int)(rspfLocalTmFormatDMY|rspfLocalTmFormatMonText|rspfLocalTmFormatPadMon|
                                              rspfLocalTmFormatSepSpace|rspfLocalTmFormatYearShort),
      rspfLocalTmFormatUsa           = (int)(rspfLocalTmFormatDMY|rspfLocalTmFormatMonFirst|rspfLocalTmFormatPadDMY|
                                              rspfLocalTmFormatZeroDMY|rspfLocalTmFormatYearShort|rspfLocalTmFormatSepChar),
      rspfLocalTmFormatEnglish       = (int)(rspfLocalTmFormatDMY|rspfLocalTmFormatYearShort|rspfLocalTmFormatPadDMY|
                                              rspfLocalTmFormatZeroDMY|rspfLocalTmFormatSepChar),
      rspfLocalTmFormatJapanese      = (int)(rspfLocalTmFormatDMY|rspfLocalTmFormatYearFirst|rspfLocalTmFormatPadDMY|
                                              rspfLocalTmFormatZeroDMY|rspfLocalTmFormatYearShort|rspfLocalTmFormatMonFirst|
                                              rspfLocalTmFormatSepChar),
      rspfLocalTmFormatFull          = (int)(rspfLocalTmFormatDMY|rspfLocalTmFormatDayOfWeek|rspfLocalTmFormatMonText|
                                              rspfLocalTmFormatSepSpace)
   };
   
   enum rspfTimeFormat  // Time format (nb: time zones not implemented)
   {
      rspfTimeFormatSecs          = (int)0x0001,         // Print seconds
      rspfTimeFormatZeroSecs      = (int)0x0002,     // Zero fill seconds
      rspfTimeFormatPadSecs       = (int)0x0004,      // Pad seconds to 2
      rspfTimeFormatSecsAll       = (int)0x0007,
      rspfTimeFormatTimeZone      = (int)0x0008,        // Print timezone
      rspfTimeFormatMins          = (int)0x0010,         // Print minutes
      rspfTimeFormatZeroMins      = (int)0x0020,     // Zero fill minutes
      rspfTimeFormatPadMins       = (int)0x0040,      // Pad minutes to 2
      rspfTimeFormatMinsAll       = (int)0x0070,
      rspfTimeFormatTZNumeric     = (int)0x0080,      // Print numeric TZ
      rspfTimeFormatHour          = (int)0x0100,            // Print hour
      rspfTimeFormatZeroHour      = (int)0x0200,        // Zero fill hour
      rspfTimeFormatPadHour       = (int)0x0400,  // Pad hour to 2 digits
      rspfTimeFormatHourAll       = (int)0x0700,
      rspfTimeFormat24hour        = (int)0x0800,          // 24hour clock
      rspfTimeFormatSepChar       = (int)0x1000, // Separate field timech
      rspfTimeFormatSepSpace      = (int)0x2000, // Separate fields space
      rspfTimeFormatSepAbbrev     = (int)0x4000,     // Add abbreviations
      rspfTimeFormatAmPm          = (int)0x8000,       // Add 'am' or 'pm'
      rspfTimeFormatInternational = (int)(rspfTimeFormatHourAll|rspfTimeFormatMinsAll|rspfTimeFormatSecsAll|
                                           rspfTimeFormat24hour|rspfTimeFormatSepChar),
      rspfTimeFormatShortTime     = (int)(rspfTimeFormatHourAll|rspfTimeFormatMinsAll|rspfTimeFormat24hour|
                                           rspfTimeFormatSepChar),
      rspfTimeFormatClockTime     = (int)(rspfTimeFormatHour|rspfTimeFormatPadHour|rspfTimeFormatMinsAll|
                                           rspfTimeFormatAmPm|rspfTimeFormatSepChar),
      rspfTimeFormatLongTime      = (int)(rspfTimeFormatHour|rspfTimeFormatPadHour|rspfTimeFormatMinsAll|
                                           rspfTimeFormatSecsAll|rspfTimeFormatSepAbbrev|rspfTimeFormatSepSpace),
      rspfTimeFormatMillitary     = (int)(rspfTimeFormatHourAll|rspfTimeFormatMinsAll|rspfTimeFormat24hour)
   };
   
   
   static char timech;    // Character used for time separator
   static char datech;    // Character used for date separator
   static int datefmt;                  // Default date format
   static int timefmt;                  // Default time format
   
   // Output methods
   std::ostream& print(std::ostream & os,
                       int df =datefmt,
                       int tf =timefmt) const;
   std::ostream& printTime(std::ostream & os, int f =timefmt) const;
   std::ostream& printDate(std::ostream & os, int f =datefmt) const;
   
   /**
    * @brief method to dump all the data members.  This is the tm struct +
    * the theFractionalSecond from this class.
    *
    * @note: tm_gmtoff and  tm_zone from tm struct are not printed as they
    * are not standard.
    *
    * @param os Stream to dump to.
    *
    * @return stream.
    */
   std::ostream& dump(std::ostream& os) const;
   
   
   int getYear()const;
   
   int getShortYear()const;
   
   /*!
    * This is one based i.e. values are returned between 1-12
    */
   int getMonth()const;
   
   /*!
    * returns the day 1-31
    */
   int getDay()const;
   double getJulian()const;
   double getModifiedJulian()const;
   
   rspfLocalTm& setDay(int day);
   /*!
    * The month is a number bewteen 1 and 12.  We will shift it to the internal
    * representation
    */
   rspfLocalTm& setMonth(int month);
   rspfLocalTm& setYear(int year);
   rspfLocalTm& setDateFromJulian(double jd);
   rspfLocalTm& setDateFromModifiedJulian(double mjd);
   
   void setFractionalDay(double fractionalDay);
   static void extractHmsFromFractionalDay(double fractionalDay,
                                           int &h,
                                           int &m,
                                           int &s,
                                           double& fractionalSecond);
   int getHour()const;
   int getMin()const;
   int getSec()const;
   double getFractionalSecond()const;
   rspfLocalTm& setHour(int h);
   rspfLocalTm& setMin(int m);
   rspfLocalTm& setSec(int s);
   rspfLocalTm& setFloatSec(double s);
   rspfLocalTm& setFractionalSecond(double fractS);
   
   /**
    * getTicks() will call getEpoc.  This is te number of microseconds passed
    * since (00:00:00 UTC, January 1, 1970)
    */ 
   time_t getTicks()const;
   time_t getEpoc()const;
   
   void addSeconds(rspf_float64 n);
   void addMinutes(rspf_float64 n);
   void addHours(rspf_float64 n);
   void addDays(rspf_float64 n);
   
   rspf_float64 deltaInSeconds(const rspfLocalTm& d)const;
   rspf_float64 deltaInMinutes(const rspfLocalTm& d)const;
   rspf_float64 delatInHours(const rspfLocalTm& d)const;
   rspf_float64 deltaInDays(const rspfLocalTm& d)const;
   
   rspfLocalTm convertToGmt()const;
   /**
    * Will not adjust for timezone.  The passed in value is based on seconds.
    */ 
   void setTimeNoAdjustmentGivenEpoc(time_t ticks);
   
   /**
    * Will adjust for timezone. The passed in value is based on seconds.
    */ 
   void setTimeGivenEpoc(time_t ticks);
   
   bool setIso8601(const std::string& timeString, bool shiftToGmtOffsetZero=false);
   
   rspfRefPtr<rspfXmlNode> saveXml()const;
   bool loadXml(rspfRefPtr<rspfXmlNode> dateNode);
   
protected:
   // Time suffix
   void tSfx(std::ostream & os, int fmt, char ch) const;
   // Time field formatters
   void pHour(std::ostream & os, int fmt) const;
   void pMins(std::ostream & os, int fmt) const;
   void pSecs(std::ostream & os, int fmt) const;
   // Date suffix
   void dSfx(std::ostream & os, int fmt) const;
   // Date field formatters
   void pDate(std::ostream & os, int fmt) const;
   void pMonth(std::ostream & os, int fmt) const;
   void pYear(std::ostream & os, int fmt) const;
   
   double theFractionalSecond;
};


class RSPF_DLL rspfDate : public rspfLocalTm
{
public:
   friend RSPF_DLL std::ostream& operator<<(std::ostream& out, const rspfDate& src);
   
   rspfDate(int datefmt =rspfLocalTm::datefmt);
   rspfDate (rspfLocalTm const & t,
              int dtfmt =rspfLocalTm::datefmt);
   rspfDate (time_t t, int dtfmt =rspfLocalTm::datefmt);
   rspfDate(int month, int day, int year,int dtfmt=rspfLocalTm::datefmt);
   
   int fmt(int f);
   int fmt(void) const;
   
   std::ostream & print (std::ostream & os) const;
   
private:
   int _fmt;
};

class RSPF_DLL rspfTime :public rspfLocalTm
{
public:
   friend RSPF_DLL std::ostream& operator<<(std::ostream& out, const rspfTime& src);
   rspfTime(int tmfmt =rspfLocalTm::timefmt);
   rspfTime (rspfTime const & t,
              int tmfmt =rspfLocalTm::timefmt);
   rspfTime (time_t t, int tmfmt =rspfLocalTm::timefmt);
   
   int fmt (int f);
   int fmt (void) const;
   
   std::ostream& print (std::ostream & os) const;
   
private:
   int _fmt;   
};
#endif
