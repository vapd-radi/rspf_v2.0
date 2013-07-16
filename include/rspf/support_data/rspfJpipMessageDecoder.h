#ifndef rspfJpipMessageDecoder_HEADER
#define rspfJpipMessageDecoder_HEADER 1
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfByteStreamBuffer.h>
#include <rspf/base/rspfReferenced.h>
#include <rspf/base/rspfException.h>
#include <rspf/support_data/rspfJpipMessage.h>

class RSPF_DLL rspfJpipMessageDecoder : public rspfReferenced
{
public:
    rspfJpipMessageDecoder();
    rspfByteStreamBuffer& inputStreamBuffer(){return m_streamBuffer;}
    std::istream& inputStream()const{return m_inputStream;}
    
    // virtual rspfJpipMessage* readMessage() throw(rspfException);
    virtual rspfJpipMessage* readMessage();
    
protected:
    /**
     * Reads a Variable-length Byte-Aligned Segment. 
     * 
     * @return the value which has been read from the VBAS.
     * 
     * @throws rspfException if an I/O error occurs.
     */
    // rspf_int64 readVBAS() throw(rspfException);
    rspf_int64 readVBAS();
    
    // rspfJpipMessage* readEORMessage() throw(rspfException);
    rspfJpipMessage* readEORMessage(); 
    
    rspf_int32 m_lastClass;
    
    /**
     * Contains the last CSn value. It is a state variables used when dependent
     * form is used.
     */
    rspf_int32 m_lastCSn;
    
    /**
     * Indicates the length of the JPIP message header. This attribute is
     * useful for statistics.
     */
    rspf_int64 m_headerLength;
    
    mutable rspfByteStreamBuffer m_streamBuffer;
    mutable std::istream m_inputStream;
};

#endif
