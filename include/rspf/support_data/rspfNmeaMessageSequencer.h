#ifndef rspfNmeaMessageSequencer_HEADER
#define rspfNmeaMessageSequencer_HEADER
#include <rspf/support_data/rspfNmeaMessage.h>
#include <rspf/base/rspfFilename.h>
#include <string>

class RSPF_DLL rspfNmeaMessageSequencer 
{
public:
   rspfNmeaMessageSequencer();
   rspfNmeaMessageSequencer(const rspfFilename& file);
   rspfNmeaMessageSequencer(const std::string& str);
   ~rspfNmeaMessageSequencer();
   
   void initialize(const rspfFilename& file);
   void initialize(const std::string& str);
   
   virtual bool next(rspfNmeaMessage& msg);
   
   virtual void reset();
   virtual bool valid()const;
   
protected:
   void destroy();
   std::istream* m_inputStream;
};
#endif
