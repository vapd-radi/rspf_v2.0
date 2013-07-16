#include <rspf/support_data/rspfNmeaMessageSequencer.h>
#include <fstream>
#include <sstream>

rspfNmeaMessageSequencer::rspfNmeaMessageSequencer()
:m_inputStream(0)
{
   
}

rspfNmeaMessageSequencer::rspfNmeaMessageSequencer(const rspfFilename& file)
:m_inputStream(0)
{
   initialize(file);
}

rspfNmeaMessageSequencer::rspfNmeaMessageSequencer(const std::string& str)
:m_inputStream(0)
{
   initialize(str);
}


rspfNmeaMessageSequencer::~rspfNmeaMessageSequencer()
{
   destroy();
}

void rspfNmeaMessageSequencer::initialize(const rspfFilename& file)
{
   destroy();
   m_inputStream = new std::ifstream(file.c_str());
}

void rspfNmeaMessageSequencer::initialize(const std::string& str)
{
   destroy();
   m_inputStream = new std::istringstream(str);
}

void rspfNmeaMessageSequencer::destroy()
{
   if(m_inputStream)
   {
      delete m_inputStream;
      m_inputStream = 0;
   }
}

bool rspfNmeaMessageSequencer::next(rspfNmeaMessage& msg)
{
   bool result = false;
   
   try {
      msg.parseMessage(*m_inputStream);
      result = true;
   } 
   catch (...) 
   {
      result = false;
   }
   
   return result;
}

bool rspfNmeaMessageSequencer::valid()const
{
   bool result = false;
   
   if(m_inputStream)
   {
      result = m_inputStream->good();
   }
   
   return result;
}

void rspfNmeaMessageSequencer::reset()
{
   if(m_inputStream)
   {
      m_inputStream->clear();
      m_inputStream->seekg(0);
   }
}
