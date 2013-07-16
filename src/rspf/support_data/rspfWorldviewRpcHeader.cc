#include <rspf/support_data/rspfWorldviewRpcHeader.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
std::ostream& operator << (std::ostream& out,
		      const rspfWorldviewRpcHeader& data)
{
   out << "theSatId      = " << data.theSatId << std::endl
       << "theBandId     = " << data.theBandId << std::endl
       << "theSpecId     = " << data.theSpecId << std::endl
       << "theErrBias    = " << data.theErrBias << std::endl
       << "theLineOffset = " << data.theLineOffset << std::endl
       << "theSampOffset = " << data.theSampOffset << std::endl
       << "theLatOffset  = " << data.theLatOffset << std::endl
       << "theLonOffset  = " << data.theLonOffset << std::endl
       << "theHeightOffset  = " << data.theHeightOffset << std::endl
       << "theLineScale  = " << data.theLineScale << std::endl
       << "theSampScale  = " << data.theSampScale << std::endl
       << "theLatScale  = " << data.theLatScale << std::endl
       << "theLonScale  = " << data.theLonScale << std::endl
       << "theHeightScale  = " << data.theHeightScale << std::endl;
   
   out << "lineNumCoef = " << std::endl;
   std::copy(data.theLineNumCoeff.begin(),
             data.theLineNumCoeff.end(),
             std::ostream_iterator<double>(out, "\n"));
   out << "lineDenCoef = " << std::endl;
   std::copy(data.theLineDenCoeff.begin(),
             data.theLineDenCoeff.end(),
             std::ostream_iterator<double>(out, "\n"));
   out << "sampNumCoef = " << std::endl;
   std::copy(data.theSampNumCoeff.begin(),
             data.theSampNumCoeff.end(),
             std::ostream_iterator<double>(out, "\n"));
   out << "sampDenCoef = " << std::endl;
   std::copy(data.theSampDenCoeff.begin(),
             data.theSampDenCoeff.end(),
             std::ostream_iterator<double>(out, "\n"));
   
   return out;
}
rspfWorldviewRpcHeader::rspfWorldviewRpcHeader()
{
}
bool rspfWorldviewRpcHeader::open(const rspfFilename& file)
{
   theFilename = file;
   std::ifstream in(file.c_str(), std::ios::in|std::ios::binary);
   
   char test[64];
   
   in.read((char*)test, 63);
   test[63] = '\0';
   in.seekg(0);
   rspfString line = test;
   line = line.upcase();
   
   if(parseNameValue(line))
   {
      theErrorStatus = rspfErrorCodes::RSPF_OK;
      getline(in,
              line);
      while((in)&&(theErrorStatus == rspfErrorCodes::RSPF_OK))
      {
         line = line.upcase();
         if(line.contains("LINENUMCOEF"))
         {
            if(!readCoeff(in, theLineNumCoeff))
            {
               setErrorStatus();
               break;
            }
         }
         else if(line.contains("LINEDENCOEF"))
         {
            if(!readCoeff(in, theLineDenCoeff))
            {
               setErrorStatus();
               break;
            }
         }
         else if(line.contains("SAMPNUMCOEF"))
         {
            if(!readCoeff(in, theSampNumCoeff))
            {
               setErrorStatus();
               break;
            }
         }
         else if(line.contains("SAMPDENCOEF"))
         {
            if(!readCoeff(in, theSampDenCoeff))
            {
               setErrorStatus();
               break;
            }
         }
         else if(!parseNameValue(line))
         {
            setErrorStatus();
            break;
         }
         getline(in,
                 line);
      }
   }
   else
   {
      setErrorStatus();
   }
   return (theErrorStatus == rspfErrorCodes::RSPF_OK);
}
bool rspfWorldviewRpcHeader::readCoeff(std::istream& in,
					std::vector<double>& coeff)
{
   coeff.clear();
   bool done = false;
   rspfString line;
   while(!in.eof()&&!in.bad()&&!done)
   {
      getline(in,
	      line);
      line.trim();
      line.trim(',');
      if(line.contains(");"))
      {
         done = true;
         line.trim(';');
         line.trim(')');
      }
      coeff.push_back(line.toDouble());
   }
   return done;
}
bool rspfWorldviewRpcHeader::parseNameValue(const rspfString& line)
{
   bool result = true;
   rspfString lineCopy = line;
   
   if(lineCopy.contains("SATID"))
   {
      theSatId = lineCopy.after("\"");
      theSatId = theSatId.before("\"");
   }
   else if(lineCopy.contains("BANDID"))
   {
      theBandId = lineCopy.after("\"");
      theBandId = theBandId.before("\"");
   }
   else if(lineCopy.contains("SPECID"))
   {
      theSpecId = lineCopy.after("\"");
      theSpecId = theSpecId.before("\"");
   }
   else if(lineCopy.contains("BEGIN_GROUP"))
   {
   }
   else if(lineCopy.contains("ERRBIAS"))
   {
      lineCopy = lineCopy.after("=");
      theErrBias = lineCopy.before(";").toDouble();
   }
   else if(lineCopy.contains("ERRRAND"))
   {
      lineCopy = lineCopy.after("=");
      theErrRand = lineCopy.before(";").toDouble();
   }
   else if(lineCopy.contains("LINEOFFSET"))
   {
      lineCopy = lineCopy.after("=");
      theLineOffset = lineCopy.before(";").toInt();
   }
   else if(lineCopy.contains("SAMPOFFSET"))
   {
      lineCopy = lineCopy.after("=");
      theSampOffset = lineCopy.before(";").toInt();
   }
   else if(lineCopy.contains("LATOFFSET"))
   {
      lineCopy = lineCopy.after("=");
      theLatOffset = lineCopy.before(";").toDouble();
   }
   else if(lineCopy.contains("LONGOFFSET"))
   {
      lineCopy = lineCopy.after("=");
      theLonOffset = lineCopy.before(";").toDouble();
   }
   else if(lineCopy.contains("HEIGHTOFFSET"))
   {
      lineCopy = lineCopy.after("=");
      theHeightOffset = lineCopy.before(";").toDouble();
   }
   else if(lineCopy.contains("LINESCALE"))
   {
      lineCopy = lineCopy.after("=");
      theLineScale = lineCopy.before(";").toDouble();
   }
   else if(lineCopy.contains("SAMPSCALE"))
   {
      lineCopy = lineCopy.after("=");
      theSampScale = lineCopy.before(";").toDouble();
   }
   else if(lineCopy.contains("LATSCALE"))
   {
      lineCopy = lineCopy.after("=");
      theLatScale = lineCopy.before(";").toDouble();
   }
   else if(lineCopy.contains("LONGSCALE"))
   {
      lineCopy = lineCopy.after("=");
      theLonScale = lineCopy.before(";").toDouble();
   }
   else if(lineCopy.contains("HEIGHTSCALE"))
   {
      lineCopy = lineCopy.after("=");
      theHeightScale = lineCopy.before(";").toDouble();
   }
   else if(lineCopy.contains("END_GROUP"))
   {
   }
   else if(lineCopy.contains("END"))
   {
   }
   else
   {
      result = false;
   }
   return result;
}
