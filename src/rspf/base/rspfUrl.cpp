#include <rspf/base/rspfUrl.h>

rspfUrl::rspfUrl()
{

}

rspfUrl::rspfUrl(const rspfString& url)
{
    setFieldsFromUrl(url);
}

rspfUrl::rspfUrl(const rspfString& protocol,
                   const rspfString& ip,
                   const rspfString& port,
                   const rspfString& path,
                   const rspfString& params)
:m_protocol(protocol),
m_ip(ip),
m_port(port),
m_path(path),
m_params(params)
{

}
void rspfUrl::setIp(const rspfString& ip)
{
    const std::string& s = ip.string();
    std::string::size_type pos = s.find_first_of(':');
    if(pos != std::string::npos)
    {
        m_ip = rspfString(s.begin(), s.begin()+pos);
        m_port = rspfString(s.begin()+pos+1, s.end());
    }
    else
    {
        m_ip = ip;
    }
}

void rspfUrl::setParams(const rspfKeywordlist& kwl)
{
    m_params = "";
    const rspfKeywordlist::KeywordMap& map = kwl.getMap();
    rspfKeywordlist::KeywordMap::const_iterator iter = map.begin();
    while(iter!=map.end())
    {
        if(m_params.empty())
        {
            m_params = iter->first + "=" + iter->second;
        }
        else
        {
            m_params+=("&"+iter->first + "=" + iter->second);
        }
        ++iter;
    }
}

void rspfUrl::setFieldsFromUrl(const rspfString& url)
{
    m_protocol = m_ip = m_port = m_path = m_params = "";
    
    // look for something of the form <protocol>://
    const std::string& s = url.string();
    std::string::size_type pos = s.find_first_of(":");
    
    if(pos == std::string::npos) return;
    
    m_protocol = rspfString(s.begin(), s.begin()+pos);
    
    // now find the ip and then extract port if embedded in IP
    //
    pos = s.find_first_not_of('/', pos+1);
    
    if(pos == std::string::npos) return;
    
    std::string::size_type nextPos = s.find_first_of('/', pos+1);
    
    if(nextPos == std::string::npos)
    {
        setIp(rspfString(s.begin()+pos, s.end()));
        return;
    }
    else
    {
        setIp(rspfString(s.begin()+pos, s.begin()+nextPos));
    }
    // now find the path portion
    //
    pos = s.find_first_of('?', nextPos);
    if(pos == std::string::npos)
    {
        m_path = rspfString(s.begin()+nextPos+1, s.end());
        return;
    }
    else
    {
        m_path = rspfString(s.begin()+nextPos+1, s.begin()+pos);
    }
    
    m_params = rspfString(s.begin()+pos+1, s.end());
    
}

rspfString rspfUrl::toString()const
{
    return (m_protocol+"://"+m_ip + (m_port.empty()?"":":"+m_port) +"/"+m_path + (m_params.empty()?"":("?"+m_params)));
}
