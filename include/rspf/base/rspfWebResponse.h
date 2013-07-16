#ifndef rspfWebResponse_HEADER
#define rspfWebResponse_HEADER
#include <rspf/base/rspfObject.h>
#include <iostream>
#include <vector>

class RSPF_DLL rspfWebResponse : public rspfObject
{
public:
   typedef std::vector<char> ByteBuffer;
   rspfWebResponse():rspfObject(){}
   
   /**
    * This might have to change in the future if we start doing other protocols 
    * such as ftp scp, ... etc.
    *
    * We currently will return the main input stream that can be used to read the 
    * bytes returned from the request.  If you need access to say a header stream 
    * from an http protocol then you can dynamic_cast to an rspfHttpResponse.
    */
   virtual std::istream* getInputStream()=0;

   void copyAllDataFromInputStream(ByteBuffer& buffer);
   
TYPE_DATA;
};
#endif
