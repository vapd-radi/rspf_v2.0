#include <rspf/support_data/rspfJpipMessageDecoder.h>

rspfJpipMessageDecoder::rspfJpipMessageDecoder()
:m_lastClass(0),
m_lastCSn(0),
m_headerLength(0),
m_inputStream(&m_streamBuffer)
{
}

// rspfJpipMessage* rspfJpipMessageDecoder::readMessage() throw(rspfException)
rspfJpipMessage* rspfJpipMessageDecoder::readMessage()   
{
    rspf_int32 BinIDIndicator = 0;
    rspf_int64 inClassIdentifier = 0;
    bool completeDataBin = false;
    rspf_int64 tempByte = 0;
    
    // Initialization
    m_headerLength = 0;
    rspfRefPtr<rspfJpipMessage> jpipMessage = new rspfJpipMessage();
    
    //	Bin-ID
    tempByte = m_inputStream.get();
    m_headerLength++;
    
    if (tempByte == 0x00) 
    { // EOR is reached
        jpipMessage = readEORMessage();
        jpipMessage->setHeaderLength(m_headerLength);
        return jpipMessage.release();
    }
    
    // b bits
    BinIDIndicator = (tempByte >> 5) & 0x03;
    if ( (BinIDIndicator < 1) || (BinIDIndicator > 3) ) 
    {
        throw new rspfException("Wrong server response: impossible to decode it correctly");
    }
    
    // c bit
    completeDataBin = (tempByte & 0x10)==0 ? false : true;
    
    // d bits (In-Class ID)
    inClassIdentifier = tempByte & 0x0F;
    if ((tempByte >> 7) > 0 ) {
        int numBytesVBAS = 1;
        do {
            tempByte = m_inputStream.get();
            m_headerLength++;
            if (tempByte == -1) {
                throw new rspfException("There is not data available to read the VBAS");
            }
            inClassIdentifier = (inClassIdentifier << 7) | (rspf_int64)(tempByte & 0x7F);
            numBytesVBAS++;
            
            if (numBytesVBAS > 9) {	// maximum long value is 2^63 - 1 => 9 bytes VBAS 
                throw new rspfException("VBAS length is larger than 63 bits (which is the maximum of long)");
            }
        } while ( (tempByte & 0x80) != 0 );
    }
    
    jpipMessage->header()->m_isLastByte = completeDataBin;
    jpipMessage->header()->m_inClassIdentifier = inClassIdentifier;
    
    // Class		
    if ( (BinIDIndicator == 2) || (BinIDIndicator == 3) )  
    {
        jpipMessage->header()->m_classIdentifier = (int)readVBAS();
        m_lastClass = jpipMessage->header()->m_classIdentifier;
        
    } 
    else 
    {
        jpipMessage->header()->m_classIdentifier = m_lastClass;
    }
    if ((jpipMessage->header()->m_classIdentifier < 0) || (jpipMessage->header()->m_classIdentifier > 8) )
        throw new rspfException("Wrong server response: invalid value for Class identifier)");
    
    // CSn				
    if ( BinIDIndicator == 3) 
    {
        jpipMessage->header()->m_CSn = (int)readVBAS();
        m_lastCSn = jpipMessage->header()->m_CSn;
        
    } 
    else 
    {
        jpipMessage->header()->m_CSn = m_lastCSn;
    }
    
    // Msg-Offset
    jpipMessage->header()->m_msgOffset = (int)readVBAS();
    
    // Msg-Length						
    jpipMessage->header()->m_msgLength = (int)readVBAS();			
    
    // Aux
    if ( (jpipMessage->header()->m_classIdentifier % 2) == 1 ) 
    {
        jpipMessage->header()->m_aux = (int)readVBAS();
    }
    
    // Read jpip message body
    if (jpipMessage->header()->m_msgLength > 0) 
    {
        jpipMessage->messageBody().resize(jpipMessage->header()->m_msgLength);
        m_inputStream.read((char*)&jpipMessage->messageBody().front(), (int)jpipMessage->header()->m_msgLength);					
    }
    
    jpipMessage->setHeaderLength(m_headerLength);
    
    return jpipMessage.release();
}

// rspf_int64 rspfJpipMessageDecoder::readVBAS() throw(rspfException)
rspf_int64 rspfJpipMessageDecoder::readVBAS()
{
    rspf_int64 value = 0;
    rspf_int64 tempByte;
    rspf_int32 numBytesVBAS = 0;
    m_headerLength = 0;
    
    do 
    {			
        tempByte = m_inputStream.get();
        m_headerLength++;
        
        if(tempByte == -1) 
        {
            throw rspfException("There is not data available to read the VBAS");
        }
        
        value = (value << 7) | (long) (tempByte & 0x7F);
        numBytesVBAS++;
        
        if (numBytesVBAS > 9) 
        {	// maximum long value is 2^63 - 1 => 9 bytes VBAS 
            throw rspfException("VBAS length is larger than 63 bits (which is the maximum of long)");
        }
        
    } while ( (tempByte & 0x80) != 0 );					
    
    return value;
}

// rspfJpipMessage* rspfJpipMessageDecoder::readEORMessage() throw(rspfException)
rspfJpipMessage* rspfJpipMessageDecoder::readEORMessage()
{
    rspfRefPtr<rspfJpipMessage> jpipMessage; 
    
    jpipMessage = new rspfJpipMessage();			
    jpipMessage->header()->m_isEOR = true;
    
    // Read EOR code		
    jpipMessage->header()->m_EORCode = m_inputStream.get();
    m_headerLength++;
    
    // Read EOR body length
    int EORBodyLength = (int)readVBAS();
    jpipMessage->header()->m_msgLength = EORBodyLength;
    
    // Read EOR body
    if (EORBodyLength > 0 ) 
    {
        jpipMessage->messageBody().resize(EORBodyLength);
        m_inputStream.read((char*)&jpipMessage->messageBody().front(), EORBodyLength);
    }
    
    return jpipMessage.release();
}
