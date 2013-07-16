//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfQuickbirdTile.cpp 19682 2011-05-31 14:21:20Z dburken $

#include <rspf/support_data/rspfQuickbirdTile.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNotifyContext.h>
#include <iostream>

static const rspfTrace traceDebug(rspfString("rspfQuickbirdTile:debug"));

rspfQuickbirdTile::rspfQuickbirdTile()
{
}

bool rspfQuickbirdTile::open(const rspfFilename tileFile)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfQuickbirdTile::open entered:"
         << "\ntileFile:  " << tileFile
         << std::endl;
   }
   
   std::ifstream in(tileFile.c_str(), std::ios::in|std::ios::binary);

   if(!in)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "Could not open!"
            << std::endl;
      }
      return false;
   }

   char testBuf[512];
   in.read((char*)testBuf, 50);
   testBuf[511] = '\0';
   rspfString testString =  testBuf;
   testString = testString.upcase();
   if(testString.contains("BEGIN_GROUP")||
      testString.contains("BANDID"))
   {
      in.seekg(0);
      rspfString line;
      rspfString name;
      rspfString value;
      std::getline(in, line.string());

      while(!in.eof()&&!in.bad()&&(in.good())&&
            (theErrorStatus==rspfErrorCodes::RSPF_OK))
      {
         parseNameValue(name,
                        value,
                        line);

         name = name.upcase();
         name = name.trim();
         if(name == "BEGIN_GROUP")
         {
            parseTileGroup(in, value);
         }
         else if(name == "BANDID")
         {
            theBandId = value.after("\"").before("\"");
         }
         else if(name == "NUMTILES")
         {
            theNumberOfTiles = value.before(";").toInt32();
         }
         else if(name == "TILESIZE")
         {
            if (traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << "Key " << name << " not parsed!"
                  << std::endl;
            }
         }
         else if(name == "TILESIZEX")
         {
            if (traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << "Key " << name << " not parsed!"
                  << std::endl;
            }
         }
         else if(name == "TILESIZEY")
         {
            if (traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << "Key " << name << " not parsed!"
                  << std::endl;
            }
         }
         else if(name == "TILEOVERLAP")
         {
            if (traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << "Key " << name << " not parsed!"
                  << std::endl;
            }
         }
         else if(name == "TILEUNITS")
         {
            if (traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << "Key " << name << " not parsed!"
                  << std::endl;
            }
         }
         else if (name == "END;")
         {
            break;
         }
         else
         {
            if (traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << "key " << name << " not parsed! Exiting with error..."
                  << std::endl;
            }
            setErrorStatus();
         }
         std::getline(in, line.string());
      }
   }

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfQuickbirdTile::open return value:"
         << ((theErrorStatus == rspfErrorCodes::RSPF_OK)?"true":"false")
         << std::endl;
   }
   
   return (theErrorStatus == rspfErrorCodes::RSPF_OK);
}

void rspfQuickbirdTile::parseTileGroup(std::istream& in,
                                        const rspfString& tileName)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfQuickbirdTile::parseTileGroup entered:"
         << std::endl;
   }
   
   rspfQuickbirdTileInfo info;
   rspfString line;
   rspfString name;
   rspfString value;
   info.theTileGroup = tileName;
   while(!in.eof()&&!in.bad()&&
         (theErrorStatus==rspfErrorCodes::RSPF_OK)&&
         (name != "END_GROUP"))
   {
      line = "";
      std::getline(in, line.string());

      parseNameValue(name, value, line);
      name = name.upcase();

      if(name == "FILENAME")
      {
         rspfString filename = value.after("\"").before("\"");
         
         info.theFilename = filename.upcase();
      }
      else if(name == "COLOFFSET")
      {
         info.theUlXOffset = value.before(";").toInt32();
      }
      else if(name == "ROWOFFSET")
      {
         info.theUlYOffset = value.before(";").toInt32();
      }
      else if(name == "ULCOLOFFSET")
      {
         info.theUlXOffset = value.before(";").toInt32();
      }
      else if(name == "ULROWOFFSET")
      {
         info.theUlYOffset = value.before(";").toInt32();
      }
      else if(name == "URCOLOFFSET")
      {
         info.theUrXOffset = value.before(";").toInt32();
      }
      else if(name == "URROWOFFSET")
      {
         info.theUrYOffset = value.before(";").toInt32();
      }
      else if(name == "LRCOLOFFSET")
      {
         info.theLrXOffset = value.before(";").toInt32();
      }
      else if(name == "LRROWOFFSET")
      {
         info.theLrYOffset = value.before(";").toInt32();
      }
      else if(name == "LLCOLOFFSET")
      {
         info.theLlXOffset = value.before(";").toInt32();
      }
      else if(name == "LLROWOFFSET")
      {
         info.theLlYOffset = value.before(";").toInt32();
      }
      else if(name == "ULLON")
      {
         info.theUlLon = value.before(";").toDouble();
      }
      else if(name == "ULLAT")
      {
         info.theUlLat = value.before(";").toDouble();
      }
      else if(name == "URLON")
      {
         info.theUrLon = value.before(";").toDouble();
      }
      else if(name == "URLAT")
      {
         info.theUrLat = value.before(";").toDouble();
      }
      else if(name == "LRLON")
      {
         info.theLrLon = value.before(";").toDouble();
      }
      else if(name == "LRLAT")
      {
         info.theLrLat = value.before(";").toDouble();
      }
      else if(name == "LLLON")
      {
         info.theLlLon = value.before(";").toDouble();
      }
      else if(name == "LLLAT")
      {
         info.theLlLat = value.before(";").toDouble();
      }
      else if(name == "END_GROUP")
      {
         break;
      }
      else if (name == "ULX")
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "key " << name << " not parsed!"
               << std::endl;
         }
      }
      else if (name == "ULY")
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "key " << name << " not parsed!"
               << std::endl;
         }
      }
      else if (name == "URX")
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "key " << name << " not parsed!"
               << std::endl;
         }
      }
      else if (name == "URY")
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "key " << name << " not parsed!"
               << std::endl;
         }
      }
      else if (name == "LRX")
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "key " << name << " not parsed!"
               << std::endl;
         }
      }
      else if (name == "LRY")
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "key " << name << " not parsed!"
               << std::endl;
         }
      }
      else if (name == "LLX")
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "key " << name << " not parsed!"
               << std::endl;
         }
      }
      else if (name == "LLY")
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "key " << name << " not parsed!"
               << std::endl;
         }
      }
      else if(name == "END;")
      {
         break;
      }
      else
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "key " << name << " not parsed! Exiting with error..."
               << std::endl;
         }

         setErrorStatus();
         break;
      }
   }
   
   if(theErrorStatus == rspfErrorCodes::RSPF_OK)
   {
      theTileMap.insert(std::make_pair(info.theFilename.string(), info));
   }
}

void rspfQuickbirdTile::parseNameValue(rspfString& name,
                                        rspfString& value,
                                        const rspfString& line)const
{
   name  = rspfString(line).before("=");
   name  = name.trim();
   value = rspfString(line).after("=");
   value = value.trim();
}

bool rspfQuickbirdTile::getInfo(rspfQuickbirdTileInfo& result,
                                 const rspfFilename& filename)const
{
   std::map<std::string,rspfQuickbirdTileInfo>::const_iterator iter =
      theTileMap.find(filename.string());

   if(iter != theTileMap.end())
   {
      result = (*iter).second;
      return true;
   }

   return false;
}
