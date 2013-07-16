#include <iostream>
#include <iomanip>

#include <rspf/base/rspfTDpt.h>
#include <rspf/base/rspfString.h>

//*******************************************************************

std::ostream& rspfTDpt::print(std::ostream& os) const
{
   os << "( ";
   os << dynamic_cast<const rspfDpt&>(*this);
   os << ", ";
   os << tie;
   os << ", ";
   if (rspf::isnan(score) == false)
   {
      os << std::setiosflags(std::ios::fixed) << std::setprecision(15);
      os << score;
   }
   else
   {
      os << "nan";
   }
   os << " )";

   return os;
}

std::ostream& rspfTDpt::printTab(std::ostream& os) const
{
   os << std::setiosflags(std::ios::fixed) << std::setprecision(15);
   os<< x ;
   os<<"\t"<< y;
   os<<"\t"<< tie.x;
   os<<"\t"<< tie.y;
   os<<"\t"<< score;

   return os;
}

std::ostream& operator<<(std::ostream& os, const rspfTDpt& pt)
{
   return pt.print(os);
}

std::istream& operator>>(std::istream& is, rspfTDpt& pt)
{
   //---
   // Expected input format:
   // ( (rspfDPt), (rspfDpt), score )
   // score is real or nan
   //---

   // Start with a nan point.
   pt.makeNan();
   // Check the stream.
   if (!is) return is;

   rspfString tempString;

   // Gobble the "(".
   is >> tempString;

   //get the first point
   is>>dynamic_cast<rspfDpt&>(pt);

   // Eat the ",".
   char c;
   is.get(c);

   //get the second point
   is>>pt.tie;

   // Eat the second ",".
   is.get(c);

   // Get the score
   const int SZ = 64; // real number buffer size
   char tempChars[SZ];
   is.get(tempChars, SZ, ',');
   if (!is) return is;
   tempChars[SZ-1] = '\0';
   tempString = tempChars;
   tempString.trim();
   if (tempString == "nan")
   {
      pt.score = rspf::nan();
   }
   else
   {
      pt.score = tempString.toDouble();
   }

   // Gobble the trailing ")".
   is >> tempString;
   
   // Finished
   return is;
}
