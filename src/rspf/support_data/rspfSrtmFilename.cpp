#include <rspf/support_data/rspfSrtmFilename.h>
#include <rspf/base/rspfRegExp.h>

rspfSrtmFilename::rspfSrtmFilename(const rspfFilename& srtmFilename)
{
   setFilename(srtmFilename);
}

rspfGpt rspfSrtmFilename::ul()const
{
   rspfGpt result;
   
   if((rspf::isnan(theSouthwestLongitude)) ||
      (rspf::isnan(theSouthwestLatitude)))
   {
      result.makeNan();
   }
   else
   {
      result.latd(theSouthwestLatitude+1.0);
      result.lond(theSouthwestLongitude);
   }
   
   return result;
}

rspfGpt rspfSrtmFilename::ur()const
{
   rspfGpt result;
   
   if((rspf::isnan(theSouthwestLongitude)) ||
      (rspf::isnan(theSouthwestLatitude)) )
   {
      result.makeNan();
   }
   else
   {
      result.latd(theSouthwestLatitude+1.0);
      result.lond(theSouthwestLongitude+1.0);
   }
   
   return result;
}

rspfGpt rspfSrtmFilename::lr()const
{
   rspfGpt result;
   
   if((rspf::isnan(theSouthwestLongitude)) ||
      (rspf::isnan(theSouthwestLatitude)) )
   {
      result.makeNan();
   }
   else
   {
      result.latd(theSouthwestLatitude);
      result.lond(theSouthwestLongitude+1.0);
   }
   
   return result;
}

rspfGpt rspfSrtmFilename::ll()const
{
   rspfGpt result;
   
   if((rspf::isnan(theSouthwestLongitude)) ||
      (rspf::isnan(theSouthwestLatitude)) )
   {
      result.makeNan();
   }
   else
   {
      result.latd(theSouthwestLatitude);
      result.lond(theSouthwestLongitude);
   }
   
   return result;
}


bool rspfSrtmFilename::setFilename(const rspfFilename& srtmFilename)
{
   theFilename = srtmFilename;
   theSouthwestLongitude = rspf::nan();
   theSouthwestLatitude  = rspf::nan();

   if(srtmFilename == "") return false;

   rspfFilename f = theFilename.fileNoExtension();
   rspfString regularExp1 = "[N|S][0-9][0-9][E|W][0-9][0-9][0-9]";
   rspfString regularExp2 = "[E|W][0-9][0-9][0-9][N|S][0-9][0-9]";
   rspfRegExp regEx;
   bool latLonOrderFlag = true;
   bool foundFlag = false;
   f = f.upcase();

   regEx.compile(regularExp1.c_str());
   foundFlag = regEx.find(f.c_str());
   if(!foundFlag)
   {
      regEx.compile(regularExp2.c_str());
      foundFlag = regEx.find(f.c_str());
      if(foundFlag)
      {
         latLonOrderFlag = false;
         f = rspfFilename(rspfString(f.begin()+regEx.start(),
                                       f.begin()+regEx.end()));
      }
   }
   if(foundFlag)
   {
      f = rspfFilename(rspfString(f.begin()+regEx.start(),
                                    f.begin()+regEx.end()));
   }
   if (f.size() != 7)
   {
      return false;
   }
//    rspfString s;
   if(latLonOrderFlag)
   {
      
//       s.push_back(f[1]);
//       s.push_back(f[2]);
      theSouthwestLatitude = rspfString(f.begin()+1,
                                         f.begin()+3).toDouble();//s.toDouble();
      // Get the latitude.
      if (f[static_cast<std::string::size_type>(0)] == 'S')
      {
         theSouthwestLatitude *= -1;
      }
      else if (f[static_cast<std::string::size_type>(0)] != 'N')
      {
         return false; // Must be either 's' or 'n'.
      }
      // Get the longitude.
//       s.clear();
//       s.push_back(f[4]);
//       s.push_back(f[5]);
//       s.push_back(f[6]);
      theSouthwestLongitude = rspfString(f.begin()+4,
                                          f.begin()+7).toDouble();//s.toDouble();
      if (f[static_cast<std::string::size_type>(3)] == 'W')
      {
      theSouthwestLongitude *= -1;
      }
      else if (f[static_cast<std::string::size_type>(3)] != 'E')
      {
         return false; // Must be either 'e' or 'w'.
      }
   }
   else
   {
      // Get the longitude.
//       s.clear();
//       s.push_back(f[1]);
//       s.push_back(f[2]);
//       s.push_back(f[3]);
      theSouthwestLongitude =  rspfString(f.begin()+1,
                                           f.begin()+4).toDouble();//s.toDouble();
      if (f[static_cast<std::string::size_type>(0)] == 'W')
      {
      theSouthwestLongitude *= -1;
      }
      else if (f[static_cast<std::string::size_type>(0)] != 'E')
      {
         return false; // Must be either 'e' or 'w'.
      }
//       s.clear();
      
//       s.push_back(f[5]);
//       s.push_back(f[6]);
      theSouthwestLatitude = rspfString(f.begin()+5,
                                         f.begin()+7).toDouble();//s.toDouble();
      // Get the latitude.
      if (f[static_cast<std::string::size_type>(4)] == 'S')
      {
         theSouthwestLatitude *= -1;
      }
      else if (f[static_cast<std::string::size_type>(4)] != 'N')
      {
         return false; // Must be either 's' or 'n'.
      }
   }

//    if(latLonOrderFlag)
//    {
//       s.push_back(f[1]);
//       s.push_back(f[2]);
//       theSouthwestLatitude = s.toDouble();
//       // Get the latitude.
//       if (f[0] == 'S')
//       {
//          theSouthwestLatitude *= -1;
//       }
//       else if (f[0] != 'N')
//       {
//          return false; // Must be either 's' or 'n'.
//       }
//       // Get the longitude.
//       s.clear();
//       s.push_back(f[4]);
//       s.push_back(f[5]);
//       s.push_back(f[6]);
//       theSouthwestLongitude = s.toDouble();
//       if (f[3] == 'W')
//       {
//       theSouthwestLongitude *= -1;
//       }
//       else if (f[3] != 'E')
//       {
//          return false; // Must be either 'e' or 'w'.
//       }
//    }
//    else
//    {
//       // Get the longitude.
//       s.clear();
//       s.push_back(f[1]);
//       s.push_back(f[2]);
//       s.push_back(f[3]);
//       theSouthwestLongitude = s.toDouble();
//       if (f[0] == 'W')
//       {
//       theSouthwestLongitude *= -1;
//       }
//       else if (f[0] != 'E')
//       {
//          return false; // Must be either 'e' or 'w'.
//       }
//       s.clear();
      
//       s.push_back(f[5]);
//       s.push_back(f[6]);
//       theSouthwestLatitude = s.toDouble();
//       // Get the latitude.
//       if (f[4] == 'S')
//       {
//          theSouthwestLatitude *= -1;
//       }
//       else if (f[4] != 'N')
//       {
//          return false; // Must be either 's' or 'n'.
//       }
//    }
   
   return true;
}
