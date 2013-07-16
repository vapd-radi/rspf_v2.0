//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfArcInfoGridWriter.cpp 16081 2009-12-10 20:56:36Z eshirschorn $

#include <cstring>
#include <cstdio>
#include <fstream>
#include <iostream>

#include <rspf/imaging/rspfArcInfoGridWriter.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/imaging/rspfCastTileSourceFilter.h>

#include <rspf/support_data/rspfAigStatistics.h>
#include <rspf/support_data/rspfAigHeader.h>
#include <rspf/support_data/rspfAigBounds.h>
#include <rspf/support_data/rspfAigIndexFileHeader.h>
#include <rspf/support_data/rspfAigDataFileHeader.h>

#include <rspf/imaging/rspfCastTileSourceFilter.h>

static const rspfTrace traceDebug("rspfArcInfoGridWriter:debug");

RTTI_DEF1(rspfArcInfoGridWriter,
          "rspfArcInfoGridWriter",
          rspfImageFileWriter);

//*******************************************************************
// Constructor:
//*******************************************************************
rspfArcInfoGridWriter::rspfArcInfoGridWriter()
   : rspfImageFileWriter(),
     theOutputTileSize(256, 4)
{
}

rspfArcInfoGridWriter::rspfArcInfoGridWriter(rspfImageSource* inputSource,
                                 const rspfFilename& filename)
   :rspfImageFileWriter(filename, inputSource)
{
}

//*******************************************************************
// Destructor
//*******************************************************************
rspfArcInfoGridWriter::~rspfArcInfoGridWriter()
{
   close();
}

bool rspfArcInfoGridWriter::writeFile()
{
   const char* MODULE = "rspfArcInfoGridWriter::writeFile";
   if(!theInputConnection)
   {
      return false;
   }
   
   rspfRefPtr<rspfCastTileSourceFilter> filter = new rspfCastTileSourceFilter;
   filter->setOutputScalarType(RSPF_FLOAT);
   filter->connectMyInputTo(theInputConnection.get());
   filter->initialize();
   
   open();

   // make sure we have a region of interest
   if(theAreaOfInterest.hasNans())
   {
      theInputConnection->initialize();
      theAreaOfInterest = theInputConnection->getAreaOfInterest();
   }
   else
   {
      theInputConnection->setAreaOfInterest(theAreaOfInterest);
   }
   
   if(theAreaOfInterest.hasNans()) return false;   
   
   theInputConnection->setTileSize(theOutputTileSize);
   theInputConnection->setToStartOfSequence();
   
   bool result = writeBinaryAigHeader()&&
                 writeBinaryAigBounds()&&
                 writeBinaryAigStats();
   if(result)
   {
      rspfFilename indexFile=rspfFilename(theFilename+"/"+"w001001x.adf");
      rspfFilename dataFile=rspfFilename(theFilename+"/"+"w001001.adf");
      
      std::ofstream indexFileStream(indexFile.c_str(),
                               ios::out|ios::binary);
      std::ofstream dataFileStream(dataFile.c_str(),
                              ios::out|ios::binary);
      rspfEndian endian;

      if(indexFileStream&&dataFileStream)
      {
         rspfAigIndexFileHeader aigIndexHeader;
         rspfAigDataFileHeader  aigDataFileHeader;
         
         rspfRefPtr<rspfImageData> currentTile =
            theInputConnection->getNextTile();
         long tileNumber = 0;
         long totalNumberOfTiles = theInputConnection->getNumberOfTiles();
         rspf_int32 tileOffsetShorts = 0;
         rspf_int32 tileSizeShorts   = 0;

         rspfNotify(rspfNotifyLevel_NOTICE)
            << "Tile height  = " << theInputConnection->getTileHeight()
            << std::endl
            << "Tile width   = " << theInputConnection->getTileWidth()
            << std::endl;
         rspf_int32 tileSizeInShorts = 2*(theInputConnection->getTileHeight()*
                                           theInputConnection->getTileWidth());
         // Note: this is in shorts
         //
         aigIndexHeader.theFileSize += (4*totalNumberOfTiles);

         // Note this is in shorts
         //
         
         aigDataFileHeader.theFileSize = 50+
                                         (tileSizeInShorts*totalNumberOfTiles) +
                                         totalNumberOfTiles;
         aigDataFileHeader.theTileSize = tileSizeInShorts;
         
         aigIndexHeader.writeStream(indexFileStream);
         aigDataFileHeader.writeStream(dataFileStream);

         if(traceDebug())
         {
            CLOG << "AIG INDEX HEADER = \n" << aigIndexHeader << std::endl;
            CLOG << "AIG DATA FILE HEADER\n"
                 << "file size = " << aigDataFileHeader.theFileSize
                 << std::endl;
         }
         
         while(currentTile.valid())
         {
            currentTile = filter->applyCast(currentTile);
            
            if(!currentTile->getBuf())
            {
               currentTile->initialize();
            }

            tileOffsetShorts = dataFileStream.tellp()/2;
            tileSizeShorts   = 2*currentTile->getHeight()*currentTile->getWidth();
            rspf_int16 tileSizeShorts16 = 2*currentTile->getHeight()*currentTile->getWidth();
            if(endian.getSystemEndianType() == RSPF_LITTLE_ENDIAN)
            {
               endian.swap(tileOffsetShorts);
               endian.swap(tileSizeShorts);
               endian.swap(tileSizeShorts16);
               endian.swap((rspf_float32*)currentTile->getBuf(),
                           currentTile->getWidth()*
                           currentTile->getHeight());
            }
            
            indexFileStream.write((char*)(&tileOffsetShorts), 4);
            indexFileStream.write((char*)(&tileSizeShorts), 4);
            dataFileStream.write((char*)(&tileSizeShorts16),
                                 2);
            dataFileStream.write((char*)(currentTile->getBuf()),
                                 currentTile->getWidth()*currentTile->getHeight()*4);
            
            currentTile = theInputConnection->getNextTile();
            ++tileNumber;
         }
      }
   }
   
   close();
   filter = 0;
   return result;
}

bool rspfArcInfoGridWriter::saveState(rspfKeywordlist& kwl,
                                const char* prefix)const
{
   return rspfImageFileWriter::saveState(kwl, prefix);
}

bool rspfArcInfoGridWriter::loadState(const rspfKeywordlist& kwl,
                                const char* prefix)
{
   
   return rspfImageFileWriter::loadState(kwl, prefix);
}

bool rspfArcInfoGridWriter::isOpen()const
{
   return (theFilename.isDir() && theFilename.exists());
}

bool rspfArcInfoGridWriter::open()
{
   if(theFilename.isDir()&&
      theFilename.exists())
   {
      return true;
   }
   else
   {
      theFilename = theFilename.path();
      if(theFilename.isDir()&&
         theFilename.exists())
      {
         return true;
      }
      else
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfArcInfoGridWriter::open(): Directory "
            << theFilename << " doesn't exist" << std::endl;
      }
      
   }
   return false;   
}

void rspfArcInfoGridWriter::close()
{
}


bool rspfArcInfoGridWriter::writeBinaryAigHeader()
{
   const char* MODULE = "rspfArcInfoGridWriter::writeBinaryAigHeader";
   rspfAigHeader aigHeader;

   rspfFilename file=rspfFilename(theFilename+"/"+"hdr.adf");

   if(isOpen())
   {
      switch(theInputConnection->getOutputScalarType())
      {
         case RSPF_UCHAR:
         case RSPF_SSHORT16:
         case RSPF_USHORT11:
         case RSPF_USHORT16:
//      {
//         aigHeader.setIntCoverage();
//         break;
//      }
         case RSPF_DOUBLE:
         case RSPF_FLOAT:
         case RSPF_NORMALIZED_FLOAT:
         case RSPF_NORMALIZED_DOUBLE:
         {
            aigHeader.setFloatCoverage();
            break;
         }
         default:
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfArcInfoGridWriter::writeBinaryAigHeader: "
               << "unkown scalar type" << std::endl;
         }
      }
      
      // need to set pixel size
      //

      // code goes here
      
      // now setting number of tiles per row
      aigHeader.theNumberOfTilesPerRow    = theInputConnection->getNumberOfTilesHorizontal();
      aigHeader.theNumberOfTilesPerColumn = theInputConnection->getNumberOfTilesVertical();
      aigHeader.theWidthOfTileInPixels    = theInputConnection->getTileWidth();
      aigHeader.theHeightOfTileInPixels   = theInputConnection->getTileHeight();
      
      std::ofstream out;
      out.open(file.c_str(), ios::out|ios::binary);

      if(out)
      {
         if(traceDebug())
         {
            CLOG << "AIG_HEADER = \n" << aigHeader << std::endl;
         }
         return aigHeader.writeStream(out);
      }
   }
   
   return false;
}

bool rspfArcInfoGridWriter::writeBinaryAigBounds()
{
   const char* MODULE = "rspfArcInfoGridWriter::writeBinaryAigBounds";
   rspfAigBounds aigBounds;
   

   // setup bounds
   rspfIrect bounds = theInputConnection->getBoundingRect();
   if(!bounds.hasNans())
   {
      aigBounds.ll_x = -.5;
      aigBounds.ll_y = -.5;
      aigBounds.ur_x = bounds.width()-.5;
      aigBounds.ur_y = bounds.height()-.5;
      
      //
      rspfFilename file=rspfFilename(theFilename+"/"+"dblbnd.adf");
      
      if(isOpen())
      {
         std::ofstream out;
         out.open(file.c_str(), ios::out|ios::binary);
         
         if(out)
         {
            if(traceDebug())
            {
               CLOG << "AIG BOUNDS =\n" <<aigBounds << std::endl;
            }
            return aigBounds.writeStream(out);
         }
      }
   }
   
   return false;
}

bool rspfArcInfoGridWriter::writeBinaryAigStats()
{
   rspfAigStatistics aigStats;


   // setup bounds
   aigStats.theMin  = theInputConnection->getMinPixelValue();
   aigStats.theMax  = theInputConnection->getMaxPixelValue();
   aigStats.theMean = (aigStats.theMax-aigStats.theMin)/2.0;

   //
   rspfFilename file=rspfFilename(theFilename+"/"+"sta.adf");

   if(isOpen())
   {
      std::ofstream out;
      out.open(file.c_str(), ios::out|ios::binary);
      
      if(out)
      {
         return aigStats.writeStream(out);
      }
   }

   return false;
}

rspfString rspfArcInfoGridWriter::getExtension() const
{
   return rspfString("aig");
}
