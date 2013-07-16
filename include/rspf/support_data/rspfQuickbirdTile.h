//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts
//
// Description:
//
//*******************************************************************
//  $Id: rspfQuickbirdTile.h 19682 2011-05-31 14:21:20Z dburken $
#ifndef rspfQuickbirdTil_HEADER
#define rspfQuickbirdTil_HEADER
#include <map>
#include <fstream>
#include <rspf/base/rspfErrorStatusInterface.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfCommon.h>

class rspfQuickbirdTileInfo
{
public:
   friend std::ostream& operator << (std::ostream& out, const rspfQuickbirdTileInfo& /* rhs */)
   {
      return out;
   }
   bool operator ==(const rspfString& rhs)const
      {
         return theFilename == rhs;
      }
   bool operator <(const rspfString& rhs)const
      {
         return theFilename < rhs;
      }
   bool operator >(const rspfString& rhs)const
      {
         return theFilename > rhs;
      }
   bool operator ==(const rspfQuickbirdTileInfo& rhs)const
      {
         return theFilename == rhs.theFilename;
      }
   bool operator <(const rspfQuickbirdTileInfo& rhs)const
      {
         return theFilename < rhs.theFilename;
      }
   bool operator >(const rspfQuickbirdTileInfo& rhs)const
      {
         return theFilename > rhs.theFilename;
      }

   rspfQuickbirdTileInfo()
      {
         initialize();
      }
   
   void initialize()
      {
         theTileGroup = "";
         theFilename  = "";
         theUlXOffset = RSPF_INT_NAN;
         theUlYOffset = RSPF_INT_NAN;
         theUrXOffset = RSPF_INT_NAN;
         theUrYOffset = RSPF_INT_NAN;
         theLrXOffset = RSPF_INT_NAN;
         theLrYOffset = RSPF_INT_NAN;
         theLlXOffset = RSPF_INT_NAN;
         theLlYOffset = RSPF_INT_NAN;

         theUlLon     = rspf::nan();
         theUlLat     = rspf::nan();
         theUrLon     = rspf::nan();
         theUrLat     = rspf::nan();
         theLrLon     = rspf::nan();
         theLrLat     = rspf::nan();
         theLlLon     = rspf::nan();
         theLlLat     = rspf::nan();
      }
   rspfString    theTileGroup;
   rspfFilename  theFilename;
   rspf_int32    theUlXOffset;
   rspf_int32    theUlYOffset;
   rspf_int32    theUrXOffset;
   rspf_int32    theUrYOffset;
   rspf_int32    theLrXOffset;
   rspf_int32    theLrYOffset;
   rspf_int32    theLlXOffset;
   rspf_int32    theLlYOffset;
   rspf_float64  theUlLon;
   rspf_float64  theUlLat;
   rspf_float64  theUrLon;
   rspf_float64  theUrLat;
   rspf_float64  theLrLon;
   rspf_float64  theLrLat;
   rspf_float64  theLlLon;
   rspf_float64  theLlLat;
};

class rspfQuickbirdTile : public rspfErrorStatusInterface
{
public:
   rspfQuickbirdTile();
   bool open(const rspfFilename tileFile);

   bool getInfo(rspfQuickbirdTileInfo& result,
                const rspfFilename& filename)const;

   const std::map<std::string, rspfQuickbirdTileInfo>& getMap() const { return theTileMap; }
   
   typedef std::map<std::string, rspfQuickbirdTileInfo> TileMap;

protected:

   TileMap     theTileMap;
   rspf_int32 theNumberOfTiles;
   rspfString theBandId;
   
   void parseTileGroup(std::istream& in,
                       const rspfString& tileName);
   void parseNameValue(rspfString& name,
                       rspfString& value,
                       const rspfString& line)const;

   
};

#endif
