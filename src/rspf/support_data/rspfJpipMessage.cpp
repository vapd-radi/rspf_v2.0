#include <rspf/support_data/rspfJpipMessage.h>

rspfJpipMessage::rspfJpipMessage()
:m_header(new rspfJpipMessageHeader())
{    
}

void rspfJpipMessage::reset()
{
    m_header->reset();
    m_messageBody.clear();
    m_headerLength = 0;
    
}

void rspfJpipMessage::dumpBodyAsHex(std::ostream& out)const
{
    rspf_uint32 idx = 0;
    for(idx = 0; idx < m_messageBody.size();++idx)
    {
        std::cout << std::hex<<(int)m_messageBody[idx]<< " " ;
        if((idx+1)%16 == 0)
        {
            out << "\n";
        }
    }
}
void rspfJpipMessage::dumpBodyAsText(std::ostream& out)const
{
    rspf_uint32 idx = 0;
    for(idx = 0; idx < m_messageBody.size();++idx)
    {
        out << (char)m_messageBody[idx];
    }
}
