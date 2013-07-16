//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfIgenGenerator.h 15833 2009-10-29 01:41:53Z eshirschorn $
#ifndef rspfIgenGenerator_HEADER
#define rspfIgenGenerator_HEADER
#include <stack>

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfGrect.h>
#include <rspf/base/rspfGeoPolygon.h>
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/imaging/rspfImageFileWriter.h>

class RSPFDLLEXPORT rspfIgenGenerator
{
public:
   rspfIgenGenerator(rspfConnectableObject* input=NULL,
                      rspfConnectableObject*  output=NULL,
                      rspfObject* view=NULL)
      :thePixelType(RSPF_PIXEL_IS_POINT),
       theTilingFlag(false),
       theTileSpacing(7.5, 7.5),
       theTileSpacingUnits(RSPF_MINUTES),
       theTilePadding(0,0),
       theTilePaddingUnits(RSPF_PIXEL),
       theNamingConvention("DEFAULT"),
       theSpecFileLocation(""),
       theIgenExecutable("igen"),
       theSpecFileIndex(0),
       theSpecListIndex(0),
       theTileNamingConvention("id")
      {
         theUpperLeftTileStart.makeNan();
         setInput(input);
         setOutput(output);
         setView(view);
         theInputBoundingRect.makeNan();
      }
   
   ~rspfIgenGenerator()
      {
      }
   void setOutput(rspfConnectableObject* output)
      {
         theSpecList.clear();
         generateOutputKwl(output);
      }

   void setOutput(const rspfKeywordlist& kwl);

    void setInput(rspfConnectableObject* input);
   
   void setView(rspfObject* view)
      {
         generateViewKwl(view);
      }
   
   void generateSpecList(bool outputToFileFlag = false);
   void getImageFilenamesFromSpecList(std::vector<rspfFilename>& filenameList);
   rspfGrect getBoundingGround()const;
   
   void executeSpecList();

   void setIteratorsToStart()
      {
         theSpecFileIndex = 0;
         theSpecListIndex = 0;
      }
   bool nextSpecFile(rspfFilename& specFile)
      {
         if(theSpecFileIndex >= theSpecFileList.size())
         {
            return false;
         }
         specFile = theSpecFileList[theSpecFileIndex];
         ++theSpecFileIndex;
         return true;
      }
   void getSpecs(vector<rspfKeywordlist>& specList)const
      {
         specList = theSpecList;
      }
   
   rspf_uint32 getNumberOfSpecFiles()const
      {
         return (rspf_uint32)theSpecFileList.size();
      }

   rspfFilename getSpecFilename(rspf_uint32 specFileIndex = 0)const
      {
         if(specFileIndex < theSpecFileList.size())
         {
            return theSpecFileList[specFileIndex];
         }

         return rspfFilename("");
      }
   rspfFilename buildExe(rspf_uint32 i = 0)const
      {
         rspfFilename igenExe  = getIgenExecutable();
         rspfFilename specFile = getSpecFilename(i);

         return rspfFilename( igenExe + " " + specFile);
      }
   
   void getSpec(rspfKeywordlist& kwl,
                rspf_uint32 specIndex=0)
      {
         kwl.clear();

         if(specIndex < theSpecList.size())
         {
            kwl = theSpecList[specIndex];
         }
      }
   void getInput(rspfKeywordlist& kwl,
                 const char* prefix=NULL)const
      {
         kwl.add(prefix,
                 theInputKwl,
                 true);
      }
   void getOutput(rspfKeywordlist& kwl,
                  const char* prefix=NULL)const
      {
         kwl.add(prefix,
                 theOutputKwl,
                 true);
      }
   
   void getView(rspfKeywordlist& kwl,
                const char* prefix=NULL)const
      {
         kwl.add(prefix,
                 theViewKwl,
                 true);
      }

   void setView(rspfKeywordlist& kwl,
                const char* prefix = NULL)
      {
         theViewKwl.clear();
         theViewKwl.add(prefix,
                        kwl,
                        true);
      }
   void setOutput(rspfKeywordlist& kwl,
                  const char* prefix = NULL)
      {
         theOutputKwl.clear();
         theOutputKwl.add(prefix,
                          kwl,
                          true);
      }
   void setSpecFileLocation(const rspfFilename& filename)
      {
         theSpecFileLocation = filename;
      }
   const rspfFilename& getSpecFileLocation()const
      {
         return theSpecFileLocation;
      }
   void setIgenExecutable(const rspfFilename& igenExecutable)
      {
         theIgenExecutable = igenExecutable;
      }
   const rspfFilename& getIgenExecutable()const
      {
         return theIgenExecutable;
      }
   void setOutputPolygon(const rspfGeoPolygon& poly)
      {
         theOutputGeoPolygon = poly;
      }
   void setPixelType(rspfPixelType pixelType)
      {
         thePixelType = pixelType;
      }
   void getOutputGeoPolygon(rspfGeoPolygon& poly)const
      {
         poly = theOutputGeoPolygon;
      }
   const rspfGeoPolygon& getOutputGeoPolygon()const
      {
         return theOutputGeoPolygon;
      }
   void setTilingFlag(bool flag)
      {
         theTilingFlag = flag;
      }
   void setTileSpacing(const rspfDpt& spacing)
      {
         theTileSpacing = spacing;
      }
   void setTilePadding(const rspfDpt& padding)
      {
         theTilePadding = padding;
      }
   void setTileSpacingUnits(rspfUnitType unitType)
      {
         theTileSpacingUnits = unitType;
      }
   void setTilePaddingUnits(rspfUnitType unitType)
      {
         theTilePaddingUnits = unitType;
      }
   bool getTilingFlag()const
      {
         return theTilingFlag;
      }
   const rspfDpt& getTileSpacing()const
      {
         return theTileSpacing;
      }
   const rspfDpt& getTilePadding()const
      {
         return theTilePadding;
      }
   rspfUnitType getTileSpacingUnits()const
      {
         return theTileSpacingUnits;
      }
   rspfUnitType getTilePaddingUnits()const
      {
         return theTilePaddingUnits;
      }
   rspfPixelType getPixelType()const
      {
         return thePixelType;
      }
  /*!
   * Tile naming convention is a string that can be "id" or "usgs".
   * this is used if tiling is enabled.  If id is used it will use the filename
   * as the prefix and then add to it the id's.  If usgs is used it will name 
   * it based on the USGS name.
   */
  void setTileNamingConvention(const rspfString& namingConvention)
  {
    theTileNamingConvention = namingConvention;
    theTileNamingConvention = theTileNamingConvention.downcase();

  }
   const rspfString& getTileNamingConvention()const
      {
         return theTileNamingConvention;
      }
   rspfIrect getInputBoundingRect()const;
   
   rspfProjection* createProductProjection()const;
   rspfObject*     createOutput()const;
   rspfObject*     createInput()const;
   
protected:
   rspfKeywordlist  theInputKwl;
   rspfKeywordlist  theOutputKwl;
   rspfKeywordlist  theViewKwl;
   rspfPixelType    thePixelType;
   bool              theTilingFlag;
   rspfDpt          theUpperLeftTileStart;
   rspfDpt          theTileSpacing;
   rspfUnitType     theTileSpacingUnits;
   rspfDpt          theTilePadding;
   rspfUnitType     theTilePaddingUnits;
   rspfString       theNamingConvention;
   rspfFilename     theSpecFileLocation;
   rspfFilename     theIgenExecutable;
   rspf_uint32      theSpecFileIndex;
   rspf_uint32      theSpecListIndex;
   rspfIrect        theInputBoundingRect;
   rspfGeoPolygon   theOutputGeoPolygon;
   rspfString       theTileNamingConvention;
  
   vector<rspfKeywordlist> theSpecList;
   vector<rspfFilename>    theSpecFileList;

   void generateInputKwl(rspfConnectableObject* connectable);
   void generateOutputKwl(rspfConnectableObject* connectable);
   void generateViewKwl(rspfObject* view);
   void setDefaultOutput();
   void generateTiledSpecList(bool outputToFileFlag);
   void generateGeoTiledSpecList(const rspfDpt& spacing, bool outputToFileFlag);
   void generatePixelTiledSpecList(bool outputToFileFlag);
   void generateNoTiledSpecList(bool outputToFileFlag);

   void generateChainForMultiInput(rspfConnectableObject* connectable);
   void generateChainForMultiInputRecurse(std::stack<rspfConnectableObject*>& aStack,
                                          rspfConnectableObject* connectable);

   int getTileId(const rspfDrect& rect,
                 const rspfDpt& currentPt,
                 const rspfDpt& spacing)const;
   void addPadding(rspfGeoPolygon& output,
                   const rspfGeoPolygon& input)const;
   void addPixelType(rspfKeywordlist& kwl, const char* prefix="product.");
};

#endif
