//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// AUTHOR: Oscar Kramer
//
// DESCRIPTION: Contains declaration of class rspfGeoidManager. Maintains
//   a list of geoids.
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfGeoidManager.h 16139 2009-12-18 18:37:07Z gpotts $

#ifndef rspfGeoidManager_HEADER
#define rspfGeoidManager_HEADER

#include <vector>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfGeoid.h>
/*****************************************************************************
 *
 * CLASS: rspfGeoidManager 
 *
 *****************************************************************************/
class RSPFDLLEXPORT rspfGeoidManager : public rspfGeoid
{
public:


   virtual ~rspfGeoidManager();
      
   /**
    * Implements singelton pattern:
    */
   static rspfGeoidManager* instance();

   
   /**
    * Permits initialization of geoids from directory name. Should never be
    * called since called on specific geoid types:
    */
   virtual bool open(const rspfFilename& dir, rspfByteOrder byteOrder);

   /**
    *  @return The offset from the ellipsoid to the geoid or rspf::nan()
    *  if grid does not contain the point.
    */
   virtual double offsetFromEllipsoid(const rspfGpt& gpt) const;

   /**
    * Method to save the state of the object to a keyword list.
    * Return true if ok or false on error. DO NOTHING
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0) const;

   /**
    * Method to the load (recreate) the state of the object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

   /**
    * Permits adding additional geoids to the list: 
    */
   virtual void addGeoid(rspfRefPtr<rspfGeoid> geoid, bool toFrontFlag=false);

   rspfGeoid* findGeoidByShortName(const rspfString& shortName, bool caseSensitive=true);
private:
   /**
    *  Private constructor.  Use "instance" method.
    */
   rspfGeoidManager();

   static rspfGeoidManager* theInstance;
   mutable std::vector< rspfRefPtr<rspfGeoid> > theGeoidList;
   
   // will use this as a identity if one wants but don't want it part of the internal list
   //
   rspfRefPtr<rspfGeoid> theIdentityGeoid;
   
   TYPE_DATA
};

#endif
