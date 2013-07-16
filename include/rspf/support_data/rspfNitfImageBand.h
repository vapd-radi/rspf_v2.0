//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
// 
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfImageBand.h 14244 2009-04-07 20:36:40Z dburken $
#ifndef rspfNitfImageBand_HEADER
#define rspfNitfImageBand_HEADER
#include <iosfwd>
#include <string>
#include <rspf/support_data/rspfNitfImageLut.h>
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfRefPtr.h>

class rspfNitfImageBand : public rspfObject
{
public:
   rspfNitfImageBand();
   virtual ~rspfNitfImageBand();
   virtual void parseStream(std::istream& in)=0;
   virtual void writeStream(std::ostream& out)=0;

   /**
    * @brief print method that outputs a key/value type format adding prefix
    * to keys.
    * @param out Stream to output to.
    * @param prefix Like "image0."
    * @param band zero based band.
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix,
                               rspf_uint32 band)const=0;

   virtual rspf_uint32 getNumberOfLuts()const=0;
   virtual const rspfRefPtr<rspfNitfImageLut> getLut(rspf_uint32 idx)const=0;
   virtual rspfRefPtr<rspfNitfImageLut> getLut(rspf_uint32 idx)=0;

TYPE_DATA;
};

#endif
