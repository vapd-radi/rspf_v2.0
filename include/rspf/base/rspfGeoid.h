//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// DESCRIPTION:
//   Contains declaration of class rspfGeoid. Maintains a grid of elevation
//   offsets for the geoid (mean sea level) relative to WGS-84 ellipsoid.
//
// SOFTWARE HISTORY:
//>
//   17Apr2001  Oscar Kramer
//              Initial coding.
//<
//*****************************************************************************

#ifndef rspfGeoid_HEADER
#define rspfGeoid_HEADER

#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfErrorStatusInterface.h>

class rspfGpt;
class rspfFilename;

class RSPFDLLEXPORT rspfGeoid : public rspfObject,
				  public rspfErrorStatusInterface
{
public:
   rspfGeoid();

   virtual bool open(const rspfFilename& dir, rspfByteOrder byteOrder=RSPF_BIG_ENDIAN) = 0;

   /**
    *  @return The offset from the ellipsoid to the geoid.  Returns
    *  rspf::nan() if grid does not contain the point.
    */
   virtual double offsetFromEllipsoid(const rspfGpt& gpt) const = 0;

protected:
   virtual ~rspfGeoid();
   
   TYPE_DATA
};

/**
 * Identity geoid.
 */
class RSPF_DLL rspfIdentityGeoid : public rspfGeoid
{
public:
   virtual rspfString getShortName()const
   {
      return rspfString("identity");
   }
   virtual bool open(const rspfFilename& /*dir*/, rspfByteOrder)
   {
      return false; // can't be opened 
   }
   virtual double offsetFromEllipsoid(const rspfGpt& /*gpt*/) const
   {
      return 0.0;
   }
   
   TYPE_DATA
};
#endif
