//*************************************************************************************************
//                          RSPF -- Open Source Software Image Map
//
// License:  See top level LICENSE.txt file.
//
// Author: Oscar Kramer
//
//*************************************************************************************************
//  $Id$

#ifndef rspfEpsgDatumFactory_HEADER
#define rspfEpsgDatumFactory_HEADER 1

#include <rspf/base/rspfDatumFactoryInterface.h>
#include <map>
#include <string>

class rspfDatum;
class rspfString;

//*************************************************************************************************
//! Class for creating datum instances from EPSG codes. 
//! Presently only a small subset of EPSG datums are supported. These are hardcoded here or,
//! alternatively, the EPSG code is mapped to the RSPF/Geotrans code and rspfDatumFactory is
//! used to produce the final datum. 
//! 
//! TO DO:
//! Eventually we should have a disk-file based database lookup of datums with associated parameters
//! similar to the EPSG projection factory and database.
//*************************************************************************************************
class RSPF_DLL rspfEpsgDatumFactory : public rspfDatumFactoryInterface
{
public:
   //! Singleton implementation.
   static rspfEpsgDatumFactory* instance();

   //! Destructor
   virtual ~rspfEpsgDatumFactory();

   //! Creates a datum instance given an EPSG spec in the form "EPSG:<datum_code>".
   virtual const rspfDatum* create(const rspfString &epsg_spec)const;

   //! Creates a datum instance given a KWL and prefix
   virtual const rspfDatum* create(const rspfKeywordlist& kwl, const char *prefix=0)const;

   //! Returns list of all datums supported, in the form of "EPSG:<code>, <datum_name>".
   virtual void getList(std::vector<rspfString>& list)const;
   
   //! Specific to this factory only. Creates a datum instance given an EPSG integer code.
   const rspfDatum* create(rspf_uint32 epsg_code)const;

   //! Specific to this factory only. Given an alpha-code, returns equivalent EPSG datum code or
   //! 0 if not found.
   rspf_uint32 findEpsgCode(const rspfString& alpha_code) const;

   //! Specific to this factory only. Given an EPSG, returns equivalent alpha-code datum code or
   //! empty string if not found.
   rspfString findAlphaCode(rspf_uint32 epsg_code) const;

protected:
   //! Hidden constructors for singleton implementation.
   rspfEpsgDatumFactory();

   //! Presently implemented as a mapping from EPSG code to RSPF/Geotrans alpha-codes found in 
   //! rspfDatumFactory. Eventually should be independent (and deprecate) rspfDatumFactory.
   std::map<rspf_uint32, std::string> m_epsgToAlphaMap;
   
   static rspfEpsgDatumFactory* m_instance;
};
#endif
