//*******************************************************************
// Copyright (C) 2004 Garrett Potts
//
// LICENSE: LGPL see top level LICENSE.txt for more details
// 
// Author: Garrett Potts
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfCompressionHeader.h 18415 2010-11-11 20:02:44Z gpotts $
#ifndef rspfNitfCompressionHeader_HEADER
#define rspfNitfCompressionHeader_HEADER
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfString.h>

#include <iosfwd>
#include <string>

class RSPF_DLL rspfNitfCompressionHeader : public rspfObject
{
public:
   virtual void parseStream(std::istream& in) = 0;

   /**
    * @brief print method that outputs a key/value type format adding prefix
    * to keys.
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix) const=0;
   
   virtual bool saveState(rspfKeywordlist& kwl, const rspfString& prefix="")const;
protected:

TYPE_DATA;   
};
#endif
