//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
// Description: CSCCGA tag class definition
//
//----------------------------------------------------------------------------
// $Id$
#include <rspf/support_data/rspfNitfRegisteredTag.h>

class RSPF_DLL rspfNitfCsproaTag : public rspfNitfRegisteredTag
{
public:
   enum{
      RESERVE1_SIZE = 12,
      RESERVE2_SIZE = 12,
      RESERVE3_SIZE = 12,
      RESERVE4_SIZE = 12,
      RESERVE5_SIZE = 12,
      RESERVE6_SIZE = 12,
      RESERVE7_SIZE = 12,
      RESERVE8_SIZE = 12,
      RESERVE9_SIZE = 12,
      BWC_SIZE = 12
   };

   rspfNitfCsproaTag();

   virtual void parseStream(std::istream& in);
   virtual void writeStream(std::ostream& out);

   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix=std::string()) const;
   virtual void clearFields();

protected:
   char m_reserved1[RESERVE1_SIZE+1];
   char m_reserved2[RESERVE1_SIZE+1];
   char m_reserved3[RESERVE1_SIZE+1];
   char m_reserved4[RESERVE1_SIZE+1];
   char m_reserved5[RESERVE1_SIZE+1];
   char m_reserved6[RESERVE1_SIZE+1];
   char m_reserved7[RESERVE1_SIZE+1];
   char m_reserved8[RESERVE1_SIZE+1];
   char m_reserved9[RESERVE1_SIZE+1];
   char m_bwc[BWC_SIZE+1];
};
