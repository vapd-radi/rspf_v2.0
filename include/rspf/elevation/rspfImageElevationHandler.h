//----------------------------------------------------------------------------
//
// File: rspfImageElevationHandler.h
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: See description for class below.
// 
//----------------------------------------------------------------------------
// $Id$

#ifndef rspfImageElevationHandler_HEADER
#define rspfImageElevationHandler_HEADER 1

#include <rspf/elevation/rspfElevCellHandler.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/imaging/rspfImageHandler.h>

/**
 * @class rspfImageElevationHandler
 *
 * Elevation source for a generic image opened via rspfImageHandler.
 */
class RSPF_DLL rspfImageElevationHandler : public rspfElevCellHandler
{
public:

   /** default constructor */
   rspfImageElevationHandler();

   /**
    * METHOD: getHeightAboveMSL
    * Height access methods.
    */
   virtual double getHeightAboveMSL(const rspfGpt&);

   /**
    *  METHOD:  getSizeOfElevCell
    *  Returns the number of post in the cell.  Satisfies pure virtual.
    *  Note:  x = longitude, y = latitude
    */
   virtual rspfIpt getSizeOfElevCell() const;
      
   /**
    *  METHOD:  getPostValue
    *  Returns the value at a given grid point as a double.
    *  Satisfies pure virtual.
    */
   virtual double getPostValue(const rspfIpt& gridPt) const;

   /** @return True if open, false if not. */
   virtual bool isOpen()const;
   
   /**
    * Opens a stream to the srtm cell.
    *
    * @return Returns true on success, false on error.
    */
   virtual bool open(const rspfFilename& file);

   /** @brief Closes the stream to the file. */
   virtual void close();

   /**
    * @brief pointHasCoverage(gpt)
    *
    * Overrides rspfElevCellHandler::pointHasCoverage
    * @return TRUE if coverage exists over gpt.
    */
   virtual bool pointHasCoverage(const rspfGpt&) const;

protected:
   /**
    * @Brief Protected destructor.
    *
    * This class is derived from rspfReferenced so users should always use
    * rspfRefPtr<rspfImageElevationHandler> to hold instance.
    */
   virtual ~rspfImageElevationHandler();
   
private:

   /** Hidden from use copy constructor */
   rspfImageElevationHandler(const rspfImageElevationHandler&);
   
   /** Hidden from use assignment operator */
   const rspfImageElevationHandler& operator=
      (const rspfImageElevationHandler& rhs);


   /** Pointers to links in chain. */
   rspfRefPtr<rspfImageHandler>      m_ih;
   rspfRefPtr<rspfImageGeometry>     m_geom;

   /** Image space rect stored as drect for inlined pointHasCoverage method. */
   rspfDrect                          m_rect;

   TYPE_DATA
};

inline bool rspfImageElevationHandler::isOpen() const
{
   return m_ih.valid();
}

inline void rspfImageElevationHandler::close()
{
   m_geom  = 0;
   m_ih    = 0;
}

inline bool rspfImageElevationHandler::pointHasCoverage(const rspfGpt& gpt) const
{
   if ( m_geom.valid() )
   {
      rspfDpt dpt;
      m_geom->worldToLocal(gpt, dpt);
      return m_rect.pointWithin(dpt);
   }
   return false;
}

#endif /* rspfImageElevationHandler_HEADER */
