//-------------------------------------------------------------------------
//
// This code was taken from Open Scene Graph and incorporated from into
// RSPF.
//
//-------------------------------------------------------------------------
// $Id: rspfApplicationUsage.h 19692 2011-05-31 16:55:47Z dburken $
#ifndef rspfApplicationUsage_HEADER
#define rspfApplicationUsage_HEADER 1

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfString.h>
#include <map>

class RSPFDLLEXPORT rspfApplicationUsage
{
    public:
        
        static rspfApplicationUsage* instance();

        rspfApplicationUsage() {}

        rspfApplicationUsage(const rspfString& commandLineUsage);

        typedef std::map<rspfString,rspfString,rspfStringLtstr> UsageMap;

        void setApplicationName(const rspfString& name);
        const rspfString& getApplicationName() const;

        void setDescription(const rspfString& desc);
        const rspfString& getDescription() const;

        enum Type
        {
            RSPF_COMMAND_LINE_OPTION    = 0x1,
            RSPF_ENVIRONMENTAL_VARIABLE = 0x2
        };
        
        void addUsageExplanation(Type type,const rspfString& option,const rspfString& explanation);
        
        void setCommandLineUsage(const rspfString& explanation);

        const rspfString& getCommandLineUsage() const;


        void addCommandLineOption(const rspfString& option,const rspfString& explanation);
        
        const UsageMap& getCommandLineOptions() const;


        void addEnvironmentalVariable(const rspfString& option,const rspfString& explanation);
        
        const UsageMap& getEnvironmentalVariables() const;

        void getFormatedString(rspfString& str, const UsageMap& um,unsigned int widthOfOutput=80);

        void write(std::ostream& output,const UsageMap& um,unsigned int widthOfOutput=80);
        
        void write(std::ostream& output,unsigned int type=RSPF_COMMAND_LINE_OPTION|RSPF_ENVIRONMENTAL_VARIABLE, unsigned int widthOfOutput=80);

    protected:
    
        rspfString theApplicationName;
        rspfString theDescription;
        rspfString theCommandLineUsage;
        UsageMap    theCommandLineOptions;
        UsageMap    theEnvironmentalVariables;
};

class ApplicationUsageProxy
{   
    public:

        /** register an explanation of commandline/evironmentalvaraible/keyboard mouse usage.*/
        ApplicationUsageProxy(rspfApplicationUsage::Type type,const rspfString& option,const rspfString& explanation)
        {
            rspfApplicationUsage::instance()->addUsageExplanation(type,option,explanation);
        }
};


#endif
