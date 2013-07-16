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
// $Id: rspfNitfEmbeddedRpfDes.h 22013 2012-12-19 17:37:20Z dburken $
#ifndef rspfNitfEmbeddedRpfDes_HEADER
#define rspfNitfEmbeddedRpfDes_HEADER 1

#include <iosfwd>
#include <rspf/support_data/rspfNitfRegisteredTag.h>
class rspfNitfFileHeader;

class RSPFDLLEXPORT rspfNitfEmbeddedRpfDes : public rspfNitfRegisteredTag
{
public:
   rspfNitfEmbeddedRpfDes();
   
   virtual void parseStream(std::istream &in);

   /**
    * @brief Print method that outputs a key/value type format
    * adding prefix to keys.
    * @param out Stream to output to.
    * @param prefix Prefix added to key like "image0.";
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix=std::string()) const;
protected:
   virtual ~rspfNitfEmbeddedRpfDes();

TYPE_DATA
private:

};

#endif
