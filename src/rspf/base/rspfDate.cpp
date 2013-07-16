//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
//----------------------------------------------------------------------------
// $Id: rspfDate.cpp 18270 2010-10-17 16:11:54Z dburken $

#include <rspf/base/rspfDate.h>
#include <cctype> /* for isdigit */
#include <iomanip>
#include <sstream>
#include <iostream>

std::ostream& operator<< (std::ostream& out, const rspfDate& src)
{
   return src.print(out);
}

std::ostream& operator<< (std::ostream & os, rspfLocalTm const & t)
{
   return t.print(os);
}

int operator== (rspfLocalTm const & t1, rspfLocalTm const & t2)
{
   return int(t1.compare(t2) == 0);
}

int operator!= (rspfLocalTm const & t1, rspfLocalTm const & t2)
{
   return int(t1.compare(t2) != 0);
}

int operator<  (rspfLocalTm const & t1, rspfLocalTm const & t2)
{
   return int(t1.compare(t2) < 0);
}

int operator<= (rspfLocalTm const & t1, rspfLocalTm const & t2)
{
   return int(t1.compare(t2) <= 0);
}

int operator>  (rspfLocalTm const & t1, rspfLocalTm const & t2)
{
   return int(t1.compare(t2) > 0);
}

int operator>=  (rspfLocalTm const & t1, rspfLocalTm const & t2)
{
   return int(t1.compare(t2) >= 0);
}

char rspfLocalTm::timech = ':';
char rspfLocalTm::datech = '/';

int rspfLocalTm::datefmt = rspfLocalTm::rspfLocalTmFormatFull;
int rspfLocalTm::timefmt = rspfLocalTm::rspfTimeFormatInternational;


rspfLocalTm::rspfLocalTm (time_t t)
   :theFractionalSecond(0.0)
{
    if (t == 0)
        t = time(0);
    *(tm *)this = *localtime(&t);
}

rspfLocalTm::rspfLocalTm (tm const & t)
   :theFractionalSecond(0.0)
{
    *((tm *)this) = t;
}

rspfLocalTm& rspfLocalTm::operator= (tm const & t)
{
    *((tm *)this) = t;
    return *this;
}

int rspfLocalTm::compare (rspfLocalTm const & t) const
{
    return compare ((time_t)t);
}

int rspfLocalTm::compare (time_t const tt) const
{
    time_t tx = (time_t)*this;
    return (tx == tt) ? 0 : (tx > tt) ? 1 : -1;
}

rspfLocalTm::operator time_t (void) const
{
    return mktime ((tm *)this);
}

int rspfLocalTm::isValid (void) const
{
    static int maxd[] =
    {
        31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };

    return ((tm_year > 0) &&
            (tm_mon >= 0) && (tm_mon < 12) &&
            (tm_mday > 0) && (tm_mday <= maxd[tm_mon]) &&
            (tm_wday < 7) && (tm_yday < 367) &&
            (tm_sec < 60) && (tm_min < 60) && (tm_hour < 24));
}
void rspfLocalTm::now()
{
   time_t t = time(0);
   *this = *localtime(&t);
}

void rspfLocalTm::dSfx (std::ostream & os, int fmt) const
{
    if (fmt & rspfLocalTmFormatSepChar)
        os << datech;
    if (fmt & rspfLocalTmFormatSepSpace)
        os << ' ';
}

void rspfLocalTm::pYear (std::ostream & os, int fmt) const
{
    if (fmt & rspfLocalTmFormatYear)
    {
        int year = tm_year;
        int dig;
        if (fmt & rspfLocalTmFormatYearShort)
        {
            dig = 2;
            year %= 100;
        }
        else
        {
            dig = 4;
            if (year < 200)
                year += 1900;
        }
        os << std::setw((fmt & rspfLocalTmFormatPadYear) ? dig : 0);
        os << std::setfill((fmt & rspfLocalTmFormatZeroYear) ? '0' : ' ');
        os << year;
        if ((fmt & rspfLocalTmFormatYearFirst))
        {
            fmt &= (rspfLocalTmFormatSepChar|rspfLocalTmFormatSepSpace);
            dSfx (os, fmt);
        }
    }
}

void rspfLocalTm::pMonth (std::ostream & os, int fmt) const
{

    static const char * _months[] =
    {
        "January", "February", "March", "April",
        "May", "June", "July", "August", "September",
        "October", "November", "December"
    };

    if (fmt & rspfLocalTmFormatMonth)
    {
        int mon = (tm_mon % 12);
        if (fmt & rspfLocalTmFormatMonText)
        {
            char const * tmon = _months[mon];
            if (!(fmt & rspfLocalTmFormatPadMon))
                os << tmon;
            else
                for (int x = 0; x < 3; ++x)
                    os << tmon[x];
        }
        else
        {
            ++mon;
            os << std::setw((fmt & rspfLocalTmFormatPadMon) ? 2 : 0);
            os << std::setfill((fmt & rspfLocalTmFormatZeroMon) ? '0' : ' ');
            os << mon;
        }
        if (((fmt & rspfLocalTmFormatYear) && !(fmt & rspfLocalTmFormatYearFirst)) ||
            ((fmt & rspfLocalTmFormatDay) && (fmt & rspfLocalTmFormatMonFirst)))
        {
            fmt &= (rspfLocalTmFormatSepChar|rspfLocalTmFormatSepSpace);
            dSfx (os, fmt);
        }
    }
}


void rspfLocalTm::pDate (std::ostream & os, int fmt) const
{
    if (fmt & rspfLocalTmFormatDay)
    {
        int day = tm_mday;
        os << std::setw((fmt & rspfLocalTmFormatPadDay) ? 2 : 0);
        os << std::setfill((fmt & rspfLocalTmFormatZeroDay) ? '0' : ' ');
        os << day;
        if (!(fmt & rspfLocalTmFormatYearFirst) || !(fmt & rspfLocalTmFormatMonFirst))
        {
            fmt &= (rspfLocalTmFormatSepChar|rspfLocalTmFormatSepSpace);
            dSfx (os, fmt);
        }
    }
}

std::ostream& rspfLocalTm::print(std::ostream & os,
                                    int df,
                                    int tf) const
{
    std::ostringstream pTmp;
    printDate (pTmp, df);
    pTmp << ' ';
    printTime (pTmp, tf);

    return os << pTmp.str();
}



std::ostream& rspfLocalTm::printDate (std::ostream & os, int fmt) const
{
    std::ostringstream pTmp;

    static const char * _days[] =
    {
        "Sunday", "Monday", "Tuesday", "Wednesday",
        "Thursday", "Friday", "Saturday"
    };

    if (fmt & rspfLocalTmFormatDayOfWeek)
    {
        int day = tm_wday % 7;
        char const * p = _days[day];
        if (fmt & rspfLocalTmFormatPadDay)
            for (int x = 0; x < 3; ++x)
                pTmp << p[x];
        else
        {
            pTmp << p;
            if (fmt & rspfLocalTmFormatDMY)
                pTmp << ',';
        }
        if ((fmt & rspfLocalTmFormatDMY) && fmt & rspfLocalTmFormatSepSpace)
            pTmp << ' ';
    }
    if (fmt & rspfLocalTmFormatYearFirst)
        pYear (pTmp, fmt);
    if (fmt & rspfLocalTmFormatMonFirst)
        pMonth (pTmp, fmt);
    pDate (pTmp, fmt);
    if (!(fmt & rspfLocalTmFormatMonFirst))
        pMonth (pTmp, fmt);
    if (!(fmt & rspfLocalTmFormatYearFirst))
        pYear (pTmp, fmt);

    return os << pTmp.str();
}

std::ostream& rspfLocalTm::dump(std::ostream& os) const
{
   // Capture the original flags.
   std::ios_base::fmtflags f = os.flags();

   os << setiosflags(ios::fixed) << setprecision(8) << setiosflags(ios::left);

   os << "fr_sec:   " << setw(12) << theFractionalSecond
      << "fractional second\n"
      << "tm_sec:   " << setw(12) << tm_sec
      << "seconds [0-60] (1 leap second)\n"
      << "tm_min:   " << setw(12) << tm_min   << "minutes [0-59]\n"
      << "tm_hour:  " << setw(12) << tm_hour  << "hours [0-23]\n"
      << "tm_mday:  " << setw(12) << tm_mday  << "day [1-31]\n"
      << "tm_mon:   " << setw(12) << tm_mon   << "month [0-11]\n"
      << "tm_year:  " << setw(12) << tm_year  << "year - 1900\n"
      << "tm_wday:  " << setw(12) << tm_wday  << "day of week [0-6]\n"
      << "tm_yday:  " << setw(12) << tm_yday  << "days in year[0-365]\n"
      << "tm_isdst: " << setw(12) << tm_isdst << "DST.[-1/0/1]\n"
      << std::endl;

   // Reset flags.
   os.setf(f);
   
   return os;
}

void rspfLocalTm::tSfx (std::ostream & os, int fmt, char ch) const
{
    if (fmt & rspfTimeFormatSepAbbrev)
        os << ch;
    if (fmt & rspfTimeFormatSepChar)
        os << timech;
    if (fmt & rspfTimeFormatSepSpace)
        os << ' ';
}

void rspfLocalTm::pHour (std::ostream & os, int fmt) const
{
    if (fmt & rspfTimeFormatHour)
    {
        int hour = tm_hour;
        if (!(fmt & rspfTimeFormat24hour))
        {
            if (hour > 12)
                hour -= 12;
            else if (!hour && (fmt & rspfTimeFormatAmPm))
                hour += 12;
        }
        os << std::setw((fmt & rspfTimeFormatPadHour) ? 2 : 0);
        os << std::setfill((fmt & rspfTimeFormatZeroHour) ? '0' : ' ');
        os << hour;
        if (!(fmt & rspfTimeFormatMins))
            fmt &= rspfTimeFormatSepAbbrev;
        tSfx (os, fmt, 'h');
    }
}

void rspfLocalTm::pMins (std::ostream & os, int fmt) const
{
    if (fmt & rspfTimeFormatMins)
    {
        int min = tm_min;
        int dig = 2;
        if (!(fmt & rspfTimeFormatHour))
        {
            min += (tm_hour * 60);
            dig += 2;
        }
        os << std::setw((fmt & rspfTimeFormatPadMins) ? dig : 0);
        os << std::setfill((fmt & rspfTimeFormatZeroMins) ? '0' : ' ');
        os << min;
        if (!(fmt & rspfTimeFormatSecs))
            fmt &= rspfTimeFormatSepAbbrev;
        tSfx (os, fmt, 'm');
    }
}

void rspfLocalTm::pSecs (std::ostream & os, int fmt) const
{
    if (fmt & rspfTimeFormatSecs)
    {
        int sec = tm_sec;
        int dig = 2;
        if (!(fmt & (rspfTimeFormatHour|rspfTimeFormatMins)))
        {
            sec += ((tm_hour * 60) + tm_min) + 60;
            dig += 3;
        }
        os << std::setw((fmt & rspfTimeFormatPadSecs) ? dig : 0);
        os << std::setfill((fmt & rspfTimeFormatZeroSecs) ? '0' : ' ');
        os << sec;
        if (fmt & rspfTimeFormatAmPm)
            fmt &= ~rspfTimeFormatSepChar;
        else
            fmt &= (rspfTimeFormatSepAbbrev|rspfTimeFormatSepSpace);
        tSfx (os, fmt, 's');
    }
}

std::ostream &rspfLocalTm::printTime (std::ostream & os, int fmt) const
{
    std::ostringstream pTmp;
    pHour (pTmp, fmt);
    pMins (pTmp, fmt);
    pSecs (pTmp, fmt);
    if (fmt & rspfTimeFormatAmPm)
    {
        pTmp << (tm_hour > 11 ? "pm" : "am");
    }

    return os << pTmp.str();
}


int rspfLocalTm::getYear()const
{
   int result = tm_year;
//   if (result < 200)
   {
      result += 1900;
   }
   return result;
}

int rspfLocalTm::getShortYear()const
{
   return (getYear()%100);
}

int rspfLocalTm::getMonth()const
{
   return ((tm_mon % 12)+1); 
}

int rspfLocalTm::getDay()const
{
   return tm_mday;
}

double rspfLocalTm::getJulian()const
{
   int J = getMonth();
   int K = getDay();
   int I = getYear();

   return (K-32075+1461*(I+4800+(J-14)/12)/4+367*(J-2-(J-14)/12*12)
           /12-3*((I+4900+(J-14)/12)/100)/4+
           (getHour()/24.0)+
           (getMin()/1440.0)+
           ((getSec()+theFractionalSecond)/86400.0));
}

double rspfLocalTm::getModifiedJulian()const
{
   return getJulian() - 2400000.5;
}

rspfLocalTm& rspfLocalTm::setDay(int day)
{
   tm_mday = day;
   
   return *this;
}

rspfLocalTm& rspfLocalTm::setMonth(int month)
{
   tm_mon = month - 1;

   return *this;
}

rspfLocalTm& rspfLocalTm::setYear(int year)
{
   if(year < 2099)
   {
      tm_year = year - 1900;
   }
   else
   {
      tm_year = year;
   }

   return *this;
}

rspfLocalTm& rspfLocalTm::setDateFromJulian(double jd)
{
   double fractional = jd - (long)jd;
   long l;
   long n;
   long i;
   long j;
   long k;


   l= (long)(jd+68569);
   n= 4*l/146097;
   l= l-(146097*n+3)/4;
   i= 4000*(l+1)/1461001;
   l= l-1461*i/4+31l;
   j= 80*l/2447;
   k= l-2447*j/80;
   l= j/11;
   j= j+2-12*l;
   i= 100*(n-49)+i+l;
   setDay(k);
   setMonth(j);
   setYear(i);

   setFractionalDay(fractional);
   
   return *this;
}

rspfLocalTm& rspfLocalTm::setDateFromModifiedJulian(double mjd)
{
   setDateFromJulian(mjd + 2400000.5);

   return *this;
}

void rspfLocalTm::setFractionalDay(double fractionalDay)
{
   int h, m, s;
   double fractionalSecond;
   extractHmsFromFractionalDay(fractionalDay, h, m, s, fractionalSecond);

   setHour(h);
   setMin(m);
   setSec(s);
   setFractionalSecond(fractionalSecond);
}

void rspfLocalTm::extractHmsFromFractionalDay(double fractionalDay,
                                               int &h,
                                               int &m,
                                               int &s,
                                               double& fractionalSecond)
{
   fractionalDay *=24;
   h = (int)fractionalDay;
   fractionalDay = fractionalDay-h;
   fractionalDay*=60;
   m = (int)fractionalDay;
   fractionalDay = fractionalDay-m;
   fractionalDay*=60;
   s = (int)fractionalDay;
   fractionalDay = fractionalDay-s;
   fractionalSecond = fractionalDay;
}

int rspfLocalTm::getHour()const
{
   return tm_hour;
}

int rspfLocalTm::getMin()const
{
   return tm_min;
}

int rspfLocalTm::getSec()const
{
   return tm_sec;
}

double rspfLocalTm::getFractionalSecond()const
{
   return theFractionalSecond;
}

rspfLocalTm& rspfLocalTm::setHour(int h)
{
   tm_hour = h;

   return *this;
}

rspfLocalTm& rspfLocalTm::setMin(int m)
{
   tm_min = m;

   return *this;
}

rspfLocalTm& rspfLocalTm::setSec(int s)
{
   tm_sec = s;

   return *this;
}

rspfLocalTm& rspfLocalTm::setFloatSec(double s)
{
   tm_sec = (int)s;
   return setFractionalSecond(s-tm_sec);
}

rspfLocalTm& rspfLocalTm::setFractionalSecond(double fractionalSecond)
{
   theFractionalSecond = fractionalSecond;

   return *this;
}

time_t rspfLocalTm::getTicks()const
{
   return getEpoc();
}

time_t rspfLocalTm::getEpoc()const
{
   std::tm temp = *this;

   return mktime(&temp);
}

void rspfLocalTm::addSeconds(rspf_float64 n)
{
   // use julian to help in this addition.  Julian is in days
   setDateFromJulian(getJulian() + (n/86400.0));
}

void rspfLocalTm::addMinutes(rspf_float64 n)
{
   setDateFromJulian(getJulian() + (n/1440.0));
}

void rspfLocalTm::addHours(rspf_float64 n)
{
   setDateFromJulian(getJulian() + (n/24.0));
}

void rspfLocalTm::addDays(rspf_float64 n)
{
   setDateFromJulian(getJulian() + n);
}

rspf_float64 rspfLocalTm::deltaInSeconds(const rspfLocalTm& d)const
{
   return (getJulian()-d.getJulian())*86400.0;
}

rspf_float64 rspfLocalTm::deltaInMinutes(const rspfLocalTm& d)const
{
   return (getJulian()-d.getJulian())*1440.0;
}

rspf_float64 rspfLocalTm::delatInHours(const rspfLocalTm& d)const
{
   return (getJulian()-d.getJulian())*24;
}

rspf_float64 rspfLocalTm::deltaInDays(const rspfLocalTm& d)const
{
   return (getJulian()-d.getJulian());
}

rspfLocalTm rspfLocalTm::convertToGmt()const
{
   struct tm gmt = *this;
#if !defined(_MSC_VER) 
   tzset();
#else
   _tzset();
#endif

#if ( defined(__APPLE__) || defined(__FreeBSD__)  || defined(__OpenBSD__) )
   gmt.tm_sec -= tm_gmtoff; // Seconds east of UTC
#else
   gmt.tm_sec += timezone; // Seconds west of UTC
   if ( tm_isdst )
   {
      gmt.tm_sec -= 3600; // Subtract an hour.
   }
#endif
   
   time_t t = mktime(&gmt);
   rspfLocalTm result(*localtime(&t));
   
   return result;
}

void rspfLocalTm::setTimeNoAdjustmentGivenEpoc(time_t ticks)
{
   *this = *gmtime(&ticks);
}

void rspfLocalTm::setTimeGivenEpoc(time_t ticks)
{
   *this = *localtime(&ticks);
}


static bool readIntegerFromString(rspf_int32& result,
                                 const std::string& input,
                                 std::string::size_type& currentPos,
                                 int digits)
{
   rspfString number;
   while((digits>0)&&
         (currentPos < input.size()))
   {
      if(isdigit(input[currentPos]))
      {
         number += input[currentPos];
         ++currentPos;
         --digits;
      }
      else
      {
         return false;
      }
   }
   result = number.toInt32();
   return (digits <= 0);
}

static bool readTimeZoneOffset(rspf_int32& result,
                               const std::string& input,
                               std::string::size_type& currentPos)
{
   bool returnValue = false;
   result = 0;
   if(input[currentPos] == '+'||
      input[currentPos] == '-')
   {
      returnValue = true;
      rspf_int32 signMult = ((input[0] == '+')?1:-1);
      rspf_int32 zoneMin = 0;
      rspf_int32 zoneHour = 0;
      ++currentPos;
      if(readIntegerFromString(zoneHour,
                               input,
                               currentPos,
                               2))
      {
         if(!isdigit(input[currentPos]))
         {
            ++currentPos; // skip :
         }
         if(readIntegerFromString(zoneMin,
                                  input,
                                  currentPos,
                                  2))
         {
            result = signMult*(zoneMin*60 + zoneHour*3600);
         }
      }
   }
   
   return returnValue;
}

bool rspfLocalTm::setIso8601(const std::string& timeString, bool shiftToGmtOffsetZero)
{
   rspfDate now;
   std::string::size_type pos = 0;
   rspf_int32 year  = 0;
   rspf_int32 month = 0;
   rspf_int32 day   = 0;
   rspf_int32 timeZoneOffset = 0;
   
   if(timeString[0] != 'T') // make sure it's not time only
   {
      // look for year
      //
      if(readIntegerFromString(year,
                               timeString,
                               pos,
                               4))
      {
         // retrieved the year portion
         // now check for separator not digit
         //
         
         // we at least have a year
         // now check for others
         setYear(year);
         if(!isdigit(timeString[pos]))
         {
            // skip separator
            ++pos;
         }
         if(readIntegerFromString(month,
                                  timeString,
                                  pos,
                                  2))
         
         {
            setMonth(month);
            if(!isdigit(timeString[pos]))
            {
               // skip separator
               ++pos;
            }
            if(readIntegerFromString(day,
                                     timeString,
                                     pos,
                                     2))
            {
               setDay(day);
            }
         }
      }
      else
      {
         return false;
      }
   }
   else // set year month day to current
   {
      setYear(now.getYear());
      setMonth(now.getMonth());
      setDay(now.getDay());
   }
   // check to see if we need to read time portion
   if(timeString[pos] == 'T')
   {
      ++pos; // skip T character
      rspf_int32 hours=0, minutes=0;
      
      if(readIntegerFromString(hours,
                               timeString,
                               pos,
                               2))
      {
         setHour(hours);
         
         // now check for separator
         if(!std::isdigit(timeString[pos]))
         {
            ++pos; // skip separator if present
         }
         if(readIntegerFromString(minutes,
                                  timeString,
                                  pos,
                                  2))
         {
            setMin(minutes);
            // now check for time zone if only a hour minute time
            //
            if(timeString[pos] == 'Z')
            {
               // no adjustment needed
            }
            else if(!readTimeZoneOffset(timeZoneOffset,
                                       timeString,
                                       pos))
            {
               double fractionalSeconds = 0.0;
               if(!std::isdigit(timeString[pos]))
               {
                  ++pos;
               }
               std::string::size_type endPos = timeString.find_first_not_of("0123456789.", pos);
               if(endPos == std::string::npos)
               {
                  fractionalSeconds = rspfString(timeString.begin()+pos,
                                                  timeString.end()).toDouble();
               }
               else
               {
                  fractionalSeconds = rspfString(timeString.begin()+pos,
                                                  timeString.begin()+endPos).toDouble();
               }
               setFloatSec(fractionalSeconds);
               pos = endPos;
               if(pos == std::string::npos)
               {
                  // we will not be too strict so if at the end then just return we got enough
                  return true;
               }
               if(timeString[pos] == 'Z')
               {
                  // no adjustment needed
               }
               else
               {
                  readTimeZoneOffset(timeZoneOffset,
                                          timeString,
                                     pos);
               }
            }
         }
      }
      else
      {
         // need at least hours 
         return false;
      }
   }
   else if(std::isdigit(timeString[pos]))
   {
      rspf_int32 hours=0, minutes=0;
      
      if(readIntegerFromString(hours,
                               timeString,
                               pos,
                               2))
      {
         setHour(hours);
         
         // now check for separator
         if(!std::isdigit(timeString[pos]))
         {
            ++pos; // skip separator if present
         }
         if(readIntegerFromString(minutes,
                                  timeString,
                                  pos,
                                  2))
         {
            setMin(minutes);
            
            if(!readTimeZoneOffset(timeZoneOffset,
                                  timeString,
                                  pos))
            {
               double fractionalSeconds = 0.0;
               if(!std::isdigit(timeString[pos]))
               {
                  ++pos;
               }
               std::string::size_type endPos = timeString.find_first_not_of("0123456789.", pos);
               if(endPos == std::string::npos)
               {
                  fractionalSeconds = rspfString(timeString.begin()+pos,
                                                  timeString.end()).toDouble();
               }
               else
               {
                  fractionalSeconds = rspfString(timeString.begin()+pos,
                                                  timeString.begin()+endPos).toDouble();
               }
               setFloatSec(fractionalSeconds);
               pos = endPos;
               if(pos == std::string::npos)
               {
                  // we will not be too strict so if at the end then just return we got enough
                  return true;
               }
               if(timeString[pos] == 'Z')
               {
                  // no adjustment needed
               }
               else
               {
                  readTimeZoneOffset(timeZoneOffset,
                                     timeString,
                                     pos);
               }
            }
         }
      }  
   }
   else
   {
      // need at least hours 
      return false;
   }
   
   if(shiftToGmtOffsetZero && (timeZoneOffset!=0))
   {
      addSeconds(-timeZoneOffset);
   }
   return true;
}

rspfRefPtr<rspfXmlNode> rspfLocalTm::saveXml()const
{
   rspfRefPtr<rspfXmlNode> result = new rspfXmlNode;

   result->setTag("rspfDate");
   result->addAttribute("version", "1");
   result->addChildNode("month", rspfString::toString(getMonth()));
   result->addChildNode("day", rspfString::toString(getDay()));
   result->addChildNode("year", rspfString::toString(getYear()));
   result->addChildNode("hour", rspfString::toString(getHour()));
   result->addChildNode("minutes", rspfString::toString(getMin()));
   result->addChildNode("seconds", rspfString::toString(getSec()));
   result->addChildNode("fractionalSecond", rspfString::toString(getFractionalSecond()));
   
   return result.get();
}

bool rspfLocalTm::loadXml(rspfRefPtr<rspfXmlNode> dateNode)
{
   bool result = true;
   rspfRefPtr<rspfXmlNode> month = dateNode->findFirstNode("month");
   rspfRefPtr<rspfXmlNode> day = dateNode->findFirstNode("day");
   rspfRefPtr<rspfXmlNode> year = dateNode->findFirstNode("year");
   rspfRefPtr<rspfXmlNode> hour = dateNode->findFirstNode("hour");
   rspfRefPtr<rspfXmlNode> minutes = dateNode->findFirstNode("minutes");
   rspfRefPtr<rspfXmlNode> seconds = dateNode->findFirstNode("seconds");
   rspfRefPtr<rspfXmlNode> fractionalSecond = dateNode->findFirstNode("fractionalSecond");
   rspfRefPtr<rspfXmlNode> julian = dateNode->findFirstNode("julian");
   rspfRefPtr<rspfXmlNode> modifiedJulian = dateNode->findFirstNode("modifiedJulian");

   if(month.valid()&&
      day.valid()&&
      year.valid()&&
      hour.valid()&&
      minutes.valid()&&
      seconds.valid())
   {
      setMonth(month->getText().toInt32());
      setDay(day->getText().toInt32());
      setYear(year->getText().toInt32());
      setHour(hour->getText().toInt32());
      setMin(minutes->getText().toInt32());
      setSec(seconds->getText().toInt32());
      if(fractionalSecond.valid())
      {
         setFractionalSecond(fractionalSecond->getText().toDouble());
      }
      else
      {
         setFractionalSecond(0.0);
      }
   }
   else if(modifiedJulian.valid())
   {
      setDateFromModifiedJulian(modifiedJulian->getText().toDouble());
   }
   else if(julian.valid())
   {
      setDateFromJulian(julian->getText().toDouble());
   }
   else
   {
      result = false;
   }

   return result;
}

rspfDate::rspfDate(int datefmt)
   :rspfLocalTm(0), _fmt(datefmt)
{}

rspfDate::rspfDate (rspfLocalTm const & t,
                      int dtfmt)
   : rspfLocalTm (t), _fmt(dtfmt)
{}

rspfDate::rspfDate (time_t t, int dtfmt)
   : rspfLocalTm (t), _fmt(dtfmt)
{}

rspfDate::rspfDate(int month,
                     int day,
                     int year,
                     int dtfmt)
   :rspfLocalTm (0), _fmt(dtfmt)
{
   setMonth(month);
   setDay(day);
   setYear(year);
   setHour(0);
   setMin(0);
   setSec(0);
   setFractionalSecond(0.0);
}

int rspfDate::fmt(int f)
{
   return _fmt = f;
}

int rspfDate::fmt(void) const
{
   return _fmt;
}

std::ostream& rspfDate::print (std::ostream & os) const
{
   return printDate (os, _fmt);
}

std::ostream& operator <<(std::ostream& out, const rspfTime& src)
{
   return src.print(out);
}
rspfTime::rspfTime(int tmfmt)
      : rspfLocalTm(0), _fmt(tmfmt)
{
}
rspfTime::rspfTime(rspfTime const & t,
                      int tmfmt)
      : rspfLocalTm (t), _fmt(tmfmt)
{
}

rspfTime::rspfTime(time_t t, int tmfmt)
      : rspfLocalTm (t), _fmt(tmfmt)
{
}
      
int rspfTime::fmt(int f)
{
   return _fmt = f;
}

int rspfTime::fmt(void) const
{
   return _fmt;
}

std::ostream& rspfTime::print (std::ostream & os) const
{
   return printTime(os, _fmt);
}
