/*********************************************************************************************
 *
 * License: LGPL Please see the top level LICENSE.txt
 *
 * Author: Garrett Potts
 * 
 *********************************************************************************************/
#ifndef rspfJpipMessage_HEADER
#define rspfJpipMessage_HEADER
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfReferenced.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/support_data/rspfJpipMessageHeader.h>
#include <vector>
class RSPF_DLL rspfJpipMessage : public rspfReferenced
{
public:
    typedef std::vector<rspf_uint8> MessageBodyType;
    
    rspfJpipMessage();
    void reset();
    void setHeaderLength(rspf_uint64 length){m_headerLength = length;}
    rspfJpipMessageHeader* header(){return m_header.get();}
    const rspfJpipMessageHeader* header()const{return m_header.get();}
    MessageBodyType& messageBody(){return m_messageBody;}
    const MessageBodyType& messageBody()const{return m_messageBody;}
    void dumpBodyAsHex(std::ostream& out)const;
    void dumpBodyAsText(std::ostream& out)const;
    
protected:
    rspfRefPtr<rspfJpipMessageHeader> m_header;
    rspf_uint64             m_headerLength;
    MessageBodyType m_messageBody;
};

#endif
