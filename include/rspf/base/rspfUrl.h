#ifndef rspfUrl_HEADER
#define rspfUrl_HEADER
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfKeywordlist.h>

class RSPF_DLL rspfUrl
{
public:
    rspfUrl();
    rspfUrl(const rspfString& url);
    rspfUrl(const rspfString& protocol,
             const rspfString& ip,
             const rspfString& port,
             const rspfString& path,
             const rspfString& params);
    const rspfString& getProtocol()const{return m_protocol;}
    const rspfString& getIp()const{return m_ip;}
    const rspfString& getPort()const{return m_port;}
    const rspfString& getPath()const{return m_path;}
    const rspfString& getParams()const{return m_params;}
    
    void setProtocol(const rspfString& protocol){m_protocol = protocol;}
    void setIp(const rspfString& ip);
    void setPort(const rspfString& port){m_port = port;}
    void setPath(const rspfString& path){m_path = path;}
    void setParams(const rspfString& params){m_params = params;}
    
    void setParams(const rspfKeywordlist& kwl);
    
    void setFieldsFromUrl(const rspfString& url);
    rspfString toString()const;
    
protected:
    rspfString m_protocol;
    rspfString m_ip;
    rspfString m_port;
    rspfString m_path;
    rspfString m_params;
};

#endif
