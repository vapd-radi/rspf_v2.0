//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfIgenGenerator.cpp 20206 2011-11-04 15:16:31Z dburken $
#include <rspf/imaging/rspfIgenGenerator.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/imaging/rspfImageChain.h>
#include <rspf/imaging/rspfGeoPolyCutter.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/base/rspfObjectFactoryRegistry.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfUsgsQuad.h>

void rspfIgenGenerator::generateSpecList(bool outputToFileFlag)
{
   if(theTilingFlag&&
      (theTileSpacingUnits!=RSPF_UNIT_UNKNOWN)&&
      (!theTileSpacing.hasNans()))
   {
      generateTiledSpecList(outputToFileFlag);
   }
   else
   {
      generateNoTiledSpecList(outputToFileFlag);
   }
   
}

void rspfIgenGenerator::getImageFilenamesFromSpecList(std::vector<rspfFilename>& filenameList)
{
   if(!theSpecList.size())
   {
      generateSpecList();
   }
   if(theSpecList.size())
   {
      filenameList.clear();
      for(rspf_uint32 i = 0; i < theSpecList.size(); ++i)
      {
         const char* filename = theSpecList[i].find("object2.",  rspfKeywordNames::FILENAME_KW);

         if(filename)
         {
            filenameList.push_back(rspfFilename(filename));
         }
         else
         {
            filenameList.clear();
            return;
         }
      }
   }
   
}

rspfGrect rspfIgenGenerator::getBoundingGround()const
{
   return rspfGrect(theOutputGeoPolygon[0],
                     theOutputGeoPolygon[1],
                     theOutputGeoPolygon[2],
                     theOutputGeoPolygon[3]);
}

void rspfIgenGenerator::executeSpecList()
{
}

void rspfIgenGenerator::setInput(rspfConnectableObject* input)
{
   theSpecList.clear();
   generateInputKwl(input);
   rspfImageSource* inputInterface = PTR_CAST(rspfImageSource,
                                                        input);
   if(inputInterface)
   {
      theInputBoundingRect = inputInterface->getBoundingRect();
   }
}


void rspfIgenGenerator::generateChainForMultiInput(rspfConnectableObject* connectable)
{
   std::stack<rspfConnectableObject*> aStack;

   generateChainForMultiInputRecurse(aStack, connectable);

   rspf_int32 objectIndex = 1;
   while(!aStack.empty())
   {
      rspfConnectableObject* current = aStack.top();
      aStack.pop();
      rspfString prefix = rspfString("object") + rspfString::toString(objectIndex) + ".";

      current->saveState(theInputKwl, prefix.c_str());

      ++objectIndex;
   }
   theInputKwl.add("type",
                   "rspfImageChain",
                   true);
   
}

void rspfIgenGenerator::generateChainForMultiInputRecurse(std::stack<rspfConnectableObject*>& aStack,
                                                           rspfConnectableObject* connectable)
{
   

   if(connectable)
   {
      aStack.push(connectable);
      for(rspf_uint32 i = 0; i < connectable->getNumberOfInputs(); ++i)
      {
         generateChainForMultiInputRecurse(aStack,
                                           connectable->getInput(i));
      }
   }   
}


rspfProjection* rspfIgenGenerator::createProductProjection()const
{
   return rspfProjectionFactoryRegistry::instance()->createProjection(theViewKwl);
}

rspfObject* rspfIgenGenerator::createOutput()const
{
   return rspfObjectFactoryRegistry::instance()->createObject(theOutputKwl);
}

rspfObject* rspfIgenGenerator::createInput()const
{
   return rspfObjectFactoryRegistry::instance()->createObject(theInputKwl);
}

void rspfIgenGenerator::generateInputKwl(rspfConnectableObject* connectable)
{
   theInputKwl.clear();
   if(connectable)
   {
      // if it has at least one non null input
      // then  we must build a chain and save it out
      if(connectable->getInput())
      {
         generateChainForMultiInput(connectable);
      }
      else // it is a single input
      {
         connectable->saveState(theInputKwl);
      }
   }
}

void rspfIgenGenerator::generateOutputKwl(rspfConnectableObject* connectable)
{
   theOutputKwl.clear();
   if(connectable)
   {
      connectable->saveState(theOutputKwl);
   }
}

void rspfIgenGenerator::generateViewKwl(rspfObject* view)
{
   theViewKwl.clear();
   if(view)
   {
      view->saveState(theViewKwl);
   }
}
   
void rspfIgenGenerator::setDefaultOutput()
{
   
}

void rspfIgenGenerator::generateTiledSpecList(bool outputToFileFlag)
{
   theSpecList.clear();
   theSpecFileList.clear();
   rspfDpt spacing = theTileSpacing;

   switch(theTileSpacingUnits)
   {
      case RSPF_MINUTES:
      {
         spacing.x /= 60;
         spacing.y /= 60;
         generateGeoTiledSpecList(spacing, outputToFileFlag);
         break;
      }
      case RSPF_SECONDS:
      {
         spacing.x /= 3600;
         spacing.y /= 3600;
         generateGeoTiledSpecList(spacing, outputToFileFlag);
         break;
      }
      case RSPF_DEGREES:
      {
         generateGeoTiledSpecList(spacing, outputToFileFlag);
         break;
      }
      case RSPF_PIXEL:
      {
         generatePixelTiledSpecList(outputToFileFlag);
         break;
      }
      default:
         break;
   }
}

void rspfIgenGenerator::generatePixelTiledSpecList(bool outputToFileFlag)
{
   rspfRefPtr<rspfProjection> proj = createProductProjection();

   if((!proj)||(theOutputGeoPolygon.size() != 4))
   {
      return;
   }
   
   rspfIrect inputRect = getInputBoundingRect();
   inputRect.stretchToTileBoundary(theTileSpacing);
   rspfGeoPolygon tempPoly;
   rspfObject* obj = createInput();
   rspfImageChain* chain = PTR_CAST(rspfImageChain,
                                     obj);
   rspfGeoPolyCutter* cutter = new rspfGeoPolyCutter;
   chain->add(cutter);
   
   rspfKeywordlist kwl;
   rspfFilename outfile = theOutputKwl.find(rspfKeywordNames::FILENAME_KW);
   
   if(outfile == "")
   {
      cerr << "Valid filename not given, returning!" << endl;
   }
   
   kwl.add("object2.", theOutputKwl, true);
   kwl.add("product.projection.",theViewKwl, true);
   addPixelType(kwl, "product.");   
   rspfDpt p1;
   rspfDpt p2;
   rspfDpt p3;
   rspfDpt p4;

   
   proj->worldToLineSample(theOutputGeoPolygon[0], p1);
   proj->worldToLineSample(theOutputGeoPolygon[1], p2);
   proj->worldToLineSample(theOutputGeoPolygon[2], p3);
   proj->worldToLineSample(theOutputGeoPolygon[3], p4);

   rspfIrect bounds(p1, p2, p3, p4);
   if(inputRect.completely_within(bounds))
   {
      inputRect = bounds;
   }

   bounds.stretchToTileBoundary(theTileSpacing);
   
   rspf_sint32 i = 1;
   for(rspf_sint32 ulY = bounds.ul().y; ulY < bounds.lr().y; ulY+=(rspf_sint32)theTileSpacing.y)
   {
      for(rspf_sint32 ulX = bounds.ul().x; ulX < bounds.lr().x; ulX+=(rspf_sint32)theTileSpacing.x)
      {
         rspfDrect rect(ulX,
                         ulY,
                         ulX + (theTileSpacing.x - 1),
                         ulY + (theTileSpacing.y - 1));
         rspfGpt gpt1;
         rspfGpt gpt2;
         rspfGpt gpt3;
         rspfGpt gpt4;

         proj->lineSampleToWorld(rect.ul(), gpt1);
         proj->lineSampleToWorld(rect.ur(), gpt2);
         proj->lineSampleToWorld(rect.lr(), gpt3);
         proj->lineSampleToWorld(rect.ll(), gpt4);
         
         tempPoly.clear();
         tempPoly.addPoint(gpt1);
         tempPoly.addPoint(gpt2);
         tempPoly.addPoint(gpt3);
         tempPoly.addPoint(gpt4);
         addPadding(tempPoly, tempPoly);
         cutter->setPolygon(tempPoly);
         chain->saveState(kwl, "object1.");
         
         rspfFilename specName(theSpecFileLocation);
         specName = specName.dirCat(rspfFilename("igen" + rspfString::toString(i) + ".spec"));
         rspfFilename tilename = outfile.path();
         if(theTileNamingConvention == "id")
         {
            int tileId = getTileId(inputRect, rspfDpt(ulX+theTileSpacing.x/2, ulY+theTileSpacing.y/2), theTileSpacing);
            tilename = tilename.dirCat(rspfFilename(outfile.fileNoExtension() + "_" +
                                                     rspfString::toString(tileId)));
            tilename.setExtension(outfile.ext());
         }
         
         kwl.add("object2.",
                 rspfKeywordNames::FILENAME_KW,
                 tilename,
                 true);
         kwl.add("object2.input_connection1",
                 kwl.find("object1.id"),
                 true);
         theSpecFileList.push_back(specName);
         if(outputToFileFlag)
         {
            kwl.write(specName.c_str());
         }
         else
         {
            theSpecList.push_back(kwl);
         }
         i++;
      }
   }
}

void rspfIgenGenerator::generateGeoTiledSpecList(const rspfDpt& spacing, bool outputToFileFlag)
{
   rspfRefPtr<rspfProjection> proj = createProductProjection();
   if((proj.valid())&&(theOutputGeoPolygon.size() == 4))
   {
      rspfDrect rect;

      rspfIrect inputRect = getInputBoundingRect();
      rspfGpt gpt1;
      rspfGpt gpt2;
      rspfGpt gpt3;
      rspfGpt gpt4;
      proj->lineSampleToWorld(inputRect.ul(), gpt1);
      proj->lineSampleToWorld(inputRect.ur(), gpt2);
      proj->lineSampleToWorld(inputRect.lr(), gpt3);
      proj->lineSampleToWorld(inputRect.ll(), gpt4);

      rspfDrect inputGroundRect(gpt1,
                                 gpt2,
                                 gpt3,
                                 gpt4,
                                 RSPF_RIGHT_HANDED);
      
      rect = rspfDrect(theOutputGeoPolygon[0],
                        theOutputGeoPolygon[1],
                        theOutputGeoPolygon[2],
                        theOutputGeoPolygon[3],
                        RSPF_RIGHT_HANDED);

      rect.stretchToTileBoundary(spacing);
      inputGroundRect.stretchToTileBoundary(spacing);
      double ulLat = rect.ul().lat;
      double ulLon = rect.ul().lon;
      rspfGeoPolygon tempPoly;
      rspfObject* obj = createInput();
      rspfImageChain* chain = PTR_CAST(rspfImageChain,
                                        obj);
      rspfGeoPolyCutter* cutter = new rspfGeoPolyCutter;
      chain->add(cutter);
      
      rspfKeywordlist kwl;
      rspfFilename outfile = theOutputKwl.find(rspfKeywordNames::FILENAME_KW);

      if(outfile == "")
      {
         cerr << "Valid filename not given, returning!" << endl;
      }
      
      kwl.add("object2.", theOutputKwl, true);
      kwl.add("object2.input_connection1",
              kwl.find("object1.id"),
              true);
      kwl.add("product.projection.",theViewKwl, true);
      addPixelType(kwl, "product.");   
      int i = 1;
      int row = 0;
      int col = 0;
      //      int tileId = 0;
      while(ulLat > rect.ll().lat)
      {
         ulLon = rect.ul().lon;
         col = 0;
         while(ulLon < rect.ur().lon)
         {
            tempPoly.clear();
            tempPoly.addPoint(ulLat, ulLon, rspf::nan(), theOutputGeoPolygon[0].datum());
            tempPoly.addPoint(ulLat, ulLon+spacing.lon, rspf::nan(), theOutputGeoPolygon[0].datum());
            tempPoly.addPoint(ulLat-spacing.lat, ulLon+spacing.lon, rspf::nan(), theOutputGeoPolygon[0].datum());
            tempPoly.addPoint(ulLat-spacing.lat, ulLon, rspf::nan(), theOutputGeoPolygon[0].datum());
            
            addPadding(tempPoly, tempPoly);
            cutter->setPolygon(tempPoly);
            chain->saveState(kwl, "object1.");

            rspfFilename specName(theSpecFileLocation);
            specName = specName.dirCat(rspfFilename("igen" + rspfString::toString(i) + ".spec"));
	    rspfFilename tilename = outfile.path();
	    if(theTileNamingConvention == "id")
	      {
		int tileId = getTileId(inputGroundRect, rspfDpt(ulLon, ulLat), spacing);
		tilename = tilename.dirCat(rspfFilename(outfile.fileNoExtension() + "_" +
							 rspfString::toString(tileId)));
		tilename.setExtension(outfile.ext());
	      }
	    else
	      {
                 rspfUsgsQuad quad(rspfGpt(ulLat - (spacing.x), ulLon+(spacing.x), 0.0));
                 if(spacing.x == 7.5/60.0)
                 {
                    tilename = tilename.dirCat(rspfFilename(quad.quarterQuadName().downcase().trim()));
                 }
                 else
                 {
                    tilename = tilename.dirCat(rspfFilename(quad.quarterQuadSegName().downcase().trim()));
                 }
                 
                 tilename.setExtension(outfile.ext());
	      }

	    kwl.add("object2.",
		    rspfKeywordNames::FILENAME_KW,
		    tilename,
		    true);
            kwl.add("object2.input_connection1",
                    kwl.find("object1.id"),
                    true);
            theSpecFileList.push_back(specName);
            if(outputToFileFlag)
            {
               kwl.write(specName.c_str());
            }
            else
            {
               theSpecList.push_back(kwl);
            }
            ulLon += spacing.lon;
            ++i;
            ++col;
         }
         ++row;
         ulLat -= spacing.lat;
      }

   }   
}


void rspfIgenGenerator::generateNoTiledSpecList(bool outputToFileFlag)
{
   theSpecList.clear();
   theSpecFileList.clear();
   
   if(theViewKwl.getSize()&&
      theInputKwl.getSize()&&
      theOutputKwl.getSize())
   {
      rspfKeywordlist kwl;
      if(theOutputGeoPolygon.size())
      {
         rspfRefPtr<rspfObject> obj = createInput();
         rspfRefPtr<rspfImageChain> chain = PTR_CAST(rspfImageChain,
                                                       obj.get());
         if(chain.valid())
         {
            rspfGeoPolyCutter* cutter = new rspfGeoPolyCutter;
            
            cutter->setPolygon(theOutputGeoPolygon);
            
            chain->add(cutter);
            chain->saveState(kwl, "object1.");
         }
         else
         {
            kwl.add("object1.",theInputKwl, true);
         }
         obj = 0;
      }
      else
      {
         kwl.add("object1.",theInputKwl, true);
      }
      kwl.add("object2.", theOutputKwl, true);
      kwl.add("object2.input_connection1",
              kwl.find("object1.id"),
              true);
      kwl.add("product.projection.",theViewKwl, true);
      addPixelType(kwl, "product.");   

      rspfFilename specName(theSpecFileLocation +
                             "/"+
                             "igen1.spec");
      
      theSpecFileList.push_back(specName);
      if(outputToFileFlag)
      {
         kwl.write(specName.c_str());
      }
      else
      {
         theSpecList.push_back(kwl);
      }
      
      kwl.clear();
   }
}

int rspfIgenGenerator::getTileId(const rspfDrect& rect,
				  const rspfDpt& currentPt,
				  const rspfDpt& spacing)const
{
   rspfDpt ul(rect.ul());
   
   double relY = fabs(ul.y - currentPt.y);
   double relX = fabs(ul.x - currentPt.x);
   
   
   rspf_int32 spacingY           = (rspf_int32)(relY/spacing.y);
   rspf_int32 spacingX           = (rspf_int32)(relX/spacing.x);
   rspf_int32 maxHorizontalTiles = (rspf_int32)( (rect.width()-1)/spacing.x);
   
   return spacingY*maxHorizontalTiles + spacingX;
}


rspfIrect rspfIgenGenerator::getInputBoundingRect()const
{
   rspfRefPtr<rspfObject> obj = createInput();
   rspfImageSource* inter = PTR_CAST(rspfImageSource, obj.get());
   rspfIrect result;

   result.makeNan();
   if(inter)
   {
      result = inter->getBoundingRect();
   }
   return result;
}

void rspfIgenGenerator::addPadding(rspfGeoPolygon& output,
                                    const rspfGeoPolygon& input)const
{
   output = input;
   switch(theTilePaddingUnits)
   {
      case RSPF_PIXEL:
      {
         rspfRefPtr<rspfProjection> proj = createProductProjection();
         if(proj.valid())
         {
            rspfDpt ul;
            rspfDpt ur;
            rspfDpt lr;
            rspfDpt ll;

            if(input.getOrdering() == RSPF_CLOCKWISE_ORDER)
            {
               proj->worldToLineSample(input[0], ul);
               proj->worldToLineSample(input[1], ur);
               proj->worldToLineSample(input[2], lr);
               proj->worldToLineSample(input[3], ll);
            }
            else
            {
               proj->worldToLineSample(input[0], ul);
               proj->worldToLineSample(input[1], ll);
               proj->worldToLineSample(input[2], lr);
               proj->worldToLineSample(input[3], ur);
            
            }
            
            ul -= theTilePadding;
            ur += rspfDpt(theTilePadding.x,
                           -theTilePadding.y);
            lr += theTilePadding;
            ll += rspfDpt(-theTilePadding.x,
                           theTilePadding.y);

            proj->lineSampleToWorld(ul, output[0]);
            proj->lineSampleToWorld(ur, output[1]);
            proj->lineSampleToWorld(lr, output[2]);
            proj->lineSampleToWorld(ll, output[3]);

            output.setOrdering(RSPF_CLOCKWISE_ORDER);
         
         }
         break;
      }
      case RSPF_DEGREES:
      case RSPF_MINUTES:
      case RSPF_SECONDS:
      {
         rspfGpt ul;
         rspfGpt ur;
         rspfGpt lr;
         rspfGpt ll;
         rspfDpt spacing = theTilePadding;

         if(theTilePaddingUnits==RSPF_MINUTES)
         {
            spacing.x /= 60;
            spacing.y /= 60;
         }
         else if(theTilePaddingUnits==RSPF_MINUTES)
         {
            spacing.x /= 3600;
            spacing.y /= 3600;
         }
      
         if(input.getOrdering() == RSPF_CLOCKWISE_ORDER)
         {
            ul = input[0];
            ur = input[1];
            lr = input[2];
            ll = input[3];
         }
         else
         {
            ul = input[0];
            ll = input[1];
            lr = input[2];
            ur = input[3];
         }
         ul.lat += spacing.lat;
         ul.lon -= spacing.lon;
         ur.lat += spacing.lat;
         ur.lon += spacing.lon;
         lr.lat -= spacing.lat;
         lr.lon += spacing.lon;
         ll.lat -= spacing.lat;
         ll.lon -= spacing.lon;

         output[0] = ul;
         output[1] = ur;
         output[2] = lr;
         output[3] = ll;
         output.setOrdering(RSPF_CLOCKWISE_ORDER);
      
         break;
      }
      case RSPF_METERS:
      {
         break;
      }
      default:
         break;
   }
}

void rspfIgenGenerator::addPixelType(rspfKeywordlist& kwl, const char* prefix)
{
   if(thePixelType == RSPF_PIXEL_IS_POINT)
   {
      kwl.add(prefix,
              rspfKeywordNames::PIXEL_TYPE_KW,
              "PIXEL_IS_POINT",
              true);
   }
   else
   {
      kwl.add(prefix,
              rspfKeywordNames::PIXEL_TYPE_KW,
              "PIXEL_IS_AREA",
              true);
   }
}

void rspfIgenGenerator::setOutput(const rspfKeywordlist& kwl)
{
   theOutputKwl.clear();

   theOutputKwl = kwl;
}
