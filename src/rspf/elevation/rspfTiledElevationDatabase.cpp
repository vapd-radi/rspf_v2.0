//----------------------------------------------------------------------------
//
// File: rspfTiledElevationDatabase.cpp
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: See class desciption in header file.
// 
//----------------------------------------------------------------------------
// $Id$

#include <rspf/elevation/rspfTiledElevationDatabase.h>

#include <rspf/base/rspfCallback1.h>
#include <rspf/base/rspfDblGrid.h>
#include <rspf/base/rspfDirectory.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfException.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>
#include <rspf/imaging/rspfOrthoImageMosaic.h>
#include <rspf/imaging/rspfSingleImageChain.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/util/rspfFileWalker.h>

#include <cmath>

static rspfTrace traceDebug(rspfString("rspfTiledElevationDatabase:debug"));

RTTI_DEF1(rspfTiledElevationDatabase, "rspfTiledElevationDatabase", rspfElevationDatabase);

//---
// Call back class to register with rspfFileWalker for call to
// rspfTiledElevationDatabase::processFile
//
// Placed here as it is unique to this class.
//---
class ProcessFileCB: public rspfCallback1<const rspfFilename&>
{
public:
   ProcessFileCB(
      rspfTiledElevationDatabase* obj,
      void (rspfTiledElevationDatabase::*func)(const rspfFilename&))
      :
      m_obj(obj),
      m_func(func)
   {}
      
   virtual void operator()(const rspfFilename& file) const
   {
      (m_obj->*m_func)(file);
   }

private:
   rspfTiledElevationDatabase* m_obj;
   void (rspfTiledElevationDatabase::*m_func)(const rspfFilename& file);
};

rspfTiledElevationDatabase::rspfTiledElevationDatabase()
   :
   rspfElevationDatabase(),
   m_entries(0),
   m_grid(0),
   m_referenceProj(0),
   m_requestedRect(),
   m_entryListRect(),
   m_fileWalker(0)
{
   m_requestedRect.makeNan();
   m_entryListRect.makeNan();

   //---
   // Use the rspfElevSource::theGroundRect to hold the mapped rect expanded to even post
   // boundaries.
   //---
   theGroundRect.makeNan();
}

// Protected destructor as this is derived from rspfRefenced.
rspfTiledElevationDatabase::~rspfTiledElevationDatabase()
{
   m_referenceProj = 0;   
   m_entries.clear();
   if ( m_grid )
   {
      delete m_grid;
      m_grid = 0;
   }
}

bool rspfTiledElevationDatabase::open(const rspfString& connectionString)
{
   return false; // tmp drb...
   
   static const char M[] = "rspfTiledElevationDatabase::open";
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << M << " entered...\n"
         << "\nConnection string: " << connectionString << "\n";
   }                   
   
   bool result = false;

   close();

   if ( connectionString.size() )
   {
      m_connectionString = connectionString.c_str();
      result = true;
   }

   if ( m_entries.size() ) result = true;
   
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << M << " result=" << (result?"true\n":"false\n");
   }

   return result;
}

void rspfTiledElevationDatabase::close()
{
   m_entryListRect.makeNan();
   m_requestedRect.makeNan();
   theGroundRect.makeNan();
   m_referenceProj = 0;
   m_entries.clear();
   if ( m_grid )
   {
      delete m_grid;
      m_grid = 0;
   }
   m_meanSpacing = 0.0;
   m_geoid = 0;
   m_connectionString.clear();
}

void rspfTiledElevationDatabase::mapRegion(const rspfGrect& region)
{
   static const char M[] = "rspfTiledElevationDatabase::mapRegion";
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << M << " entered...\n" << "region: " << region << "\n";
   }
   
   if ( m_connectionString.size() )
   {
      // Put these outside the try block so we can delete if exception thrown.
      rspfCallback1<const rspfFilename&>* cb = 0;

      // Wrap in try catch block as excptions can be thrown under the hood.
      try
      {
         m_requestedRect = region;
         
         rspfFilename f = m_connectionString;
         if ( f.exists() )
         {
            // Walk the directory
            m_fileWalker = new rspfFileWalker();
            m_fileWalker->initializeDefaultFilterList();

            m_fileWalker->setNumberOfThreads( rspf::getNumberOfThreads() );

            // Must set this so we can stop recursion on directory based images.
            m_fileWalker->setWaitOnDirFlag( true );
            
            cb = new ProcessFileCB(this, &rspfTiledElevationDatabase::processFile);
            m_fileWalker->registerProcessFileCallback(cb);
            m_fileWalker->walk(f);

            mapRegion();
         }
         else
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << M << " ERROR: Connection string does not exists: "
               << m_connectionString.c_str()  << endl;
         }
      }
      catch (const rspfException& e)
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "Caught exception: " << e.what() << endl;
         m_entries.clear();
      }

      // cleanup:
      if ( m_fileWalker )
      {
         delete m_fileWalker;
         m_fileWalker = 0;
      }
      if ( cb )
      {
         delete cb;
         cb = 0;
      }
   }

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << M << " exited...\n";
   }
}

void rspfTiledElevationDatabase::processFile(const rspfFilename& file)
{
   static const char M[] = "rspfTiledElevationDatabase::processFile";
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << M << " entered...\n" << "file: " << file << "\n";
   }

   rspfRefPtr<rspfSingleImageChain> sic = new rspfSingleImageChain();
   if ( sic->open(file, false) ) // False for do not open overviews.
   {
      if ( isDirectoryBasedImage( sic->getImageHandler() ) )
      {
         // Tell the walker not to recurse this directory.
         m_fileWalker->setRecurseFlag(false);
      }
     
      rspfRefPtr<rspfImageHandler> ih = sic->getImageHandler();
      if ( ih.valid() && (m_requestedRect.isLonLatNan() == false) )
      {
         rspfRefPtr<rspfImageGeometry> geom = ih->getImageGeometry();
         if ( geom.valid() == false )
         {
            std::string errMsg = M;
            errMsg += " ERROR:\nNo image geometry for image: ";
            errMsg += ih->getFilename().string();
            throw rspfException(errMsg);
         }
         
         rspfRefPtr<rspfProjection> proj = geom->getProjection();
         if ( proj.valid() == false )
         {
            std::string errMsg = M;
            errMsg += " ERROR:\nNo image projection for image: ";
            errMsg += ih->getFilename().string();
            throw rspfException(errMsg);
         }
         
         // Get the bounding rect:
         rspfGrect boundingRect;
         getBoundingRect(geom, boundingRect);
         
         if ( boundingRect.isLonLatNan() )
         {
            std::string errMsg = M;
            errMsg += " ERROR:\nBounding rect has nans for image: ";
            errMsg += ih->getFilename().string();
            throw rspfException(errMsg); 
         }
         
         if ( boundingRect.intersects(m_requestedRect) )
         {
            bool addEntryToList = false;
            
            if ( m_entries.size() == 0 ) // First time through.
            {
               addEntryToList = true;
               m_entryListRect = boundingRect;
               m_referenceProj = proj;
               m_meanSpacing = (geom->getMetersPerPixel().x + geom->getMetersPerPixel().y) / 2.0;
            }
            else 
            {
               addEntryToList = isCompatible( ih.get(), geom.get(), proj.get() );
               if ( addEntryToList )
               {
                  // Expand the rectangle.
                  m_entryListRect.combine(boundingRect);
               }
            }
            
            if ( addEntryToList )
            {
               // If we're keeping it add a cache to the chain.
               sic->addCache();
               
               //---
               // Create the entry and give to addEntry which checks for duplicates in case
               // mapRegion was called consecutively.
               //---
               rspfTiledElevationEntry entry(boundingRect, sic);
               addEntry(entry);

               // Once the requested rect is filled abort the file walk.
               if ( m_requestedRect.completely_within( m_entryListRect ) )
               {
                  m_fileWalker->setAbortFlag(true);
               }
            }
         }
      }
      else
      {
         std::string errMsg = M;
         errMsg += " ERROR:\nNo image geometry for image: ";
         errMsg += ih->getFilename().string();
         throw rspfException(errMsg);
      }
   }

   if(traceDebug())
   {
      // Since rspfFileWalker is threaded output the file so we know which job exited.
      rspfNotify(rspfNotifyLevel_DEBUG) << M << "\nfile: " << file << "\nexited...\n";
   }   
}

void rspfTiledElevationDatabase::addEntry(const rspfTiledElevationEntry& entry)
{
   bool result = true;
   std::vector<rspfTiledElevationEntry>::const_iterator i = m_entries.begin();
   while ( i != m_entries.end() )
   {
      if ( (*i).m_sic->getFilename() == entry.m_sic->getFilename() )
      {
         result = false; // In list already.
         break;
      }
      ++i;
   }
   if ( result )
   {
      m_entries.push_back(entry); // Add to list.
   }
}

void rspfTiledElevationDatabase::initializeReferenceProjection()
{
   if ( m_entries.size() )
   {
      rspfRefPtr<rspfImageHandler> ih = m_entries[0].m_sic->getImageHandler();
      if ( ih.valid() )
      {
         rspfRefPtr<rspfImageGeometry> geom = ih->getImageGeometry();
         if ( geom.valid() )
         {
            m_referenceProj = geom->getProjection();
         }
         // else throw exception ??? (drb)
      }
   }
}

double rspfTiledElevationDatabase::getHeightAboveMSL(const rspfGpt& gpt)
{
   if ( m_grid )
   {
      return (*m_grid)(gpt.lon, gpt.lat);
   }
   return rspf::nan();
}

double rspfTiledElevationDatabase::getHeightAboveEllipsoid(const rspfGpt& gpt)
{
   double h = getHeightAboveMSL(gpt);
   if(h != rspf::nan())
   {
      h += getOffsetFromEllipsoid(gpt);
   }
   return h;
}
 
bool rspfTiledElevationDatabase::pointHasCoverage(const rspfGpt& gpt) const
{
   if ( theGroundRect.isLonLatNan() == false)
   {
      return theGroundRect.pointWithin(gpt);
   }
   return false;
}

bool rspfTiledElevationDatabase::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   static const char M[] = "rspfTiledElevationDatabase::loadState";
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << M << " entered..." << "\nkwl:\n" << kwl << "\n";
   }     
   bool result = false;
   const char* lookup = kwl.find(prefix, "type");
   if ( lookup )
   {
      std::string type = lookup;
      if ( ( type == "image_directory" ) || ( type == "rspfTiledElevationDatabase" ) )
      {
         result = rspfElevationDatabase::loadState(kwl, prefix);
      }
   }

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << M << " result=" << (result?"true\n":"false\n");
   }

   return result;
}

bool rspfTiledElevationDatabase::saveState(rspfKeywordlist& kwl, const char* prefix) const
{
   return rspfElevationDatabase::saveState(kwl, prefix);
}

// Private method:
bool rspfTiledElevationDatabase::isDirectoryBasedImage(rspfRefPtr<rspfImageHandler> ih)
{
   bool result = false;
   if ( ih.valid() )
   {
      // Get the image handler name.
      rspfString imageHandlerName = ih->getClassName();
      if ( (imageHandlerName == "rspfAdrgTileSource") ||
           (imageHandlerName == "rspfCibCadrgTileSource") )  
      {
         result = true;
      }
   }
   return result;
}

bool rspfTiledElevationDatabase::isCompatible(rspfImageHandler* ih,
                                               rspfImageGeometry* geom,
                                               rspfProjection* proj) const
{
   //---
   // Check for match of the following against the first entry:
   // bands
   // projection
   // scalar type
   // scale
   //---
   bool result = false;

   if ( m_entries.size() && ih && geom && proj )
   {
      rspfRefPtr<rspfImageHandler> entry0_ih = m_entries[0].m_sic->getImageHandler();
      
      // Check scalar type and bands.
      if ( (entry0_ih->getOutputScalarType() == ih->getOutputScalarType()) &&
           (entry0_ih->getNumberOfOutputBands() == ih->getNumberOfOutputBands()) )
      {
         // Check the scale.
         rspfRefPtr<const rspfMapProjection> mapProj =
            dynamic_cast<const rspfMapProjection*>(proj);
         if ( mapProj.valid() )
         {
            rspfRefPtr<const rspfMapProjection> refMapProj =
               dynamic_cast<const rspfMapProjection*>(m_referenceProj.get());
            
            if ( mapProj->isGeographic() )
            {
               if ( refMapProj->getDecimalDegreesPerPixel() ==
                    mapProj->getDecimalDegreesPerPixel() )
               {
                  result = true;
               }
               else if ( refMapProj->getMetersPerPixel() ==
                         mapProj->getMetersPerPixel() )
               {
                  result = true;
               }
            }
         }
         else // sensor model
         {
            if ( proj->getMetersPerPixel() == m_referenceProj->getMetersPerPixel() )
            {
               result = true;
            }
         }
      }
   }
   
   return result;
}

void rspfTiledElevationDatabase::getBoundingRect(
   rspfRefPtr<rspfImageGeometry> geom, rspfGrect& boundingRect) const
{
   if ( geom.valid() )
   {
      std::vector<rspfGpt> corner(4);
      if ( geom->getCornerGpts(corner[0], corner[1], corner[2], corner[3]) )
      {
         rspfGpt ulGpt(corner[0]);
         rspfGpt lrGpt(corner[0]);
         for ( rspf_uint32 i = 1; i < 4; ++i )
         {
            if ( corner[i].lon < ulGpt.lon ) ulGpt.lon = corner[i].lon;
            if ( corner[i].lat > ulGpt.lat ) ulGpt.lat = corner[i].lat;
            if ( corner[i].lon > lrGpt.lon ) lrGpt.lon = corner[i].lon;
            if ( corner[i].lat < lrGpt.lat ) lrGpt.lat = corner[i].lat;
         }
         boundingRect = rspfGrect(ulGpt, lrGpt);
      }
      else
      {
         boundingRect.makeNan();
      }
   }
}
void rspfTiledElevationDatabase::mapRegion()
{
   static const char M[] = "rspfTiledElevationDatabase::mapRegion";
   
   if ( m_entries.size() && ( m_requestedRect.isLonLatNan() == false ) )
   {
      rspfRefPtr<rspfOrthoImageMosaic> mosaic = new rspfOrthoImageMosaic();
      std::vector<rspfTiledElevationEntry>::iterator i = m_entries.begin();
      while ( i != m_entries.end() )
      {
         mosaic->connectMyInputTo( (*i).m_sic.get() );
         ++i;
      }

      // Compute the requested rectangle in view space.
      rspfRefPtr<rspfImageGeometry> geom = mosaic->getImageGeometry();
      if ( geom.valid() )
      {
         rspfDpt ulDpt;
         rspfDpt lrDpt;
         geom->worldToLocal(m_requestedRect.ul(), ulDpt);
         geom->worldToLocal(m_requestedRect.lr(), lrDpt);

         // Expand out to fall on even view coordinates.
         ulDpt.x = std::floor(ulDpt.x);
         ulDpt.y = std::floor(ulDpt.y);
         lrDpt.x = std::ceil(lrDpt.x);
         lrDpt.y = std::floor(lrDpt.y);

         // Get new(expanded) corners in ground space.
         rspfGpt ulGpt;
         rspfGpt lrGpt;
         geom->localToWorld(ulDpt, ulGpt);
         geom->localToWorld(lrDpt, lrGpt);
         theGroundRect = rspfGrect(ulGpt, lrGpt);
            
         // Expanded requested rect in view space.
         rspfIpt ulIpt = ulDpt;
         rspfIpt lrIpt = lrDpt;
         const rspfIrect VIEW_RECT(ulIpt, lrIpt);

         // Get the data.
         rspfRefPtr<rspfImageData> data = mosaic->getTile(VIEW_RECT, 0);
         if ( data.valid() )
         {
            // Initialize the grid:
            const rspfIpt SIZE( data->getWidth(), data->getHeight() );
            const rspfDpt ORIGIN(ulGpt.lon, lrGpt.lat); // SouthWest corner
            const rspfDpt SPACING( (lrGpt.lon-ulGpt.lon)/(SIZE.x-1),
                                    (ulGpt.lat-lrGpt.lat)/(SIZE.y-1) );
            if ( !m_grid ) m_grid = new rspfDblGrid();
            m_grid->initialize(SIZE, ORIGIN, SPACING, rspf::nan());

            if ( traceDebug() )
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << M
                  << "\nrequested view rect: " << VIEW_RECT
                  << "\nexpanded ground rect: " << theGroundRect
                  << "\norigin:  " << ORIGIN
                  << "\nsize:    " << SIZE
                  << "\nspacing: " << SPACING << std::endl;
            }

            // Fill the grid:
            switch( data->getScalarType() )
            {
               case RSPF_SINT16:
               {
                  fillGrid(rspf_sint16(0), data);
                  break;
               }
               case RSPF_SINT32:
               {
                  fillGrid(rspf_sint32(0), data);
                  break;
               }
               case RSPF_FLOAT32:
               {
                  fillGrid(rspf_float32(0), data);
                  break;
               }
               case RSPF_FLOAT64:
               {
                  fillGrid(rspf_float64(0), data);
                  break;
               }
               case RSPF_UINT8:
               case RSPF_SINT8:
               case RSPF_USHORT11:
               case RSPF_UINT16:
               case RSPF_UINT32:
               case RSPF_NORMALIZED_DOUBLE:
               case RSPF_NORMALIZED_FLOAT:
               case RSPF_SCALAR_UNKNOWN:
               default:
               {
                  std::string errMsg = M;
                  errMsg += " ERROR:\nUnhandled scalar type: ";
                  errMsg += data->getScalarTypeAsString().string();
                  throw rspfException(errMsg);
               }
            }
            
         } // if ( data.valid() )

      } // if ( geom.valid() )

   } // if ( m_entries.size() && ...
}

template <class T> void rspfTiledElevationDatabase::fillGrid(T /* dummyTemplate */,
                                                              rspfRefPtr<rspfImageData> data)
{
   if ( data.valid() )
   {
      // Copy to grid reversing the lines as the rspfDblGrid's origin is the SW corner.
      const rspf_float64 NP  = data->getNullPix(0);
      const T* buf = static_cast<T*>(data->getBuf(0));
      if ( buf )
      {
         const rspfIpt SIZE( data->getWidth(), data->getHeight() );
         rspf_int32 bufIdx = (SIZE.y-1) * data->getWidth();
         rspf_int32 grdY = 0;
         for (rspf_int32 y = SIZE.y-1; y >= 0; --y)
         {
            for (rspf_int32 x = 0; x < SIZE.x; ++ x)
            {
               rspf_float64 v = static_cast<rspf_float64>(buf[bufIdx+x]);
               m_grid->setNode(x, grdY, (v!=NP?v:rspf::nan()) );
            }
            bufIdx -= data->getWidth();
            ++grdY;
         }
      }
   }
}

// Hidden from use...
rspfTiledElevationDatabase::rspfTiledElevationDatabase(
   const rspfTiledElevationDatabase& /* copy_this */)
{
}

// Hidden from ues...
const rspfTiledElevationDatabase& rspfTiledElevationDatabase::operator=(
   const rspfTiledElevationDatabase& /* rhs */)
{
   return *this;
}

// Private container class:
rspfTiledElevationDatabase::rspfTiledElevationEntry::rspfTiledElevationEntry()
   : m_rect(),
     m_sic(0)
{}
                                                   
rspfTiledElevationDatabase::rspfTiledElevationEntry::rspfTiledElevationEntry(
   const rspfGrect& rect, rspfRefPtr<rspfSingleImageChain> sic )
   : m_rect(rect),
     m_sic(sic)
{}

