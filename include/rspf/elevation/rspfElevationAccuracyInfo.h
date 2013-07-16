#ifndef rspfElevationAccuracyInfo_HEADER
#define rspfElevationAccuracyInfo_HEADER 1
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfReferenced.h>
#include <rspf/base/rspfString.h>


class RSPF_DLL rspfElevationAccuracyInfo: public rspfReferenced
{
public:
    rspfElevationAccuracyInfo():
        m_confidenceLevel(0.9),
        m_relativeCE(rspf::nan()),
        m_absoluteCE(rspf::nan()),
        m_relativeLE(rspf::nan()),
        m_absoluteLE(rspf::nan()),
        m_surfaceName("None")
    {}

    void makeNan()
    {
     m_relativeCE = rspf::nan();
     m_absoluteCE = rspf::nan();
     m_relativeLE = rspf::nan();
     m_absoluteLE = rspf::nan();
    }
    double getConfidenceLevel()const{return m_confidenceLevel;}

    bool hasValidRelativeError()const
    { 
        return ((!rspf::isnan(m_relativeCE)) && 
                (!rspf::isnan(m_relativeLE)));
    }
    bool hasValidAbsoluteError()const{
        return ((!rspf::isnan(m_absoluteCE)) && 
                (!rspf::isnan(m_absoluteLE)));
    }
    double getRelativeCE()const{return m_relativeCE;}
    void setRelativeCE(double value){m_relativeCE = value;}

    double getAbsoluteCE()const{return m_absoluteCE;}
    void setAbsoluteCE(double value){m_absoluteCE = value;}

    double getRelativeLE()const{return m_relativeLE;}
    void setRelativeLE(double value){m_relativeLE = value;}

    double getAbsoluteLE()const{return m_absoluteLE;}
    void setAbsoluteLE(double value){m_absoluteLE = value;}
    
    const rspfString& getSurfaceName()const{return m_surfaceName;}
    void setSurfaceName(const rspfString& value){m_surfaceName = value;}

    double m_confidenceLevel;
    double m_relativeCE;
    double m_absoluteCE;
    double m_relativeLE;
    double m_absoluteLE;
    rspfString m_surfaceName;
};

#endif
