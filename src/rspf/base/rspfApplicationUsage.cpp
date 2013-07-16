//-------------------------------------------------------------------------
//
// This code was taken from Open Scene Graph and incorporated from into
// RSPF.
//
//-------------------------------------------------------------------------
// $Id: rspfApplicationUsage.cpp 19682 2011-05-31 14:21:20Z dburken $

#include <iostream>
#include <rspf/base/rspfApplicationUsage.h>
#include <rspf/base/rspfCommon.h>

rspfApplicationUsage::rspfApplicationUsage(const rspfString& commandLineUsage):
    theCommandLineUsage(commandLineUsage)
{
}

rspfApplicationUsage* rspfApplicationUsage::instance()
{
    static rspfApplicationUsage theApplicationUsage;
    return &theApplicationUsage;
}
void rspfApplicationUsage::setApplicationName(const rspfString& name)
{
   theApplicationName = name;
}

const rspfString& rspfApplicationUsage::getApplicationName() const
{
   return theApplicationName;
}
void rspfApplicationUsage::setDescription(const rspfString& desc)
{
   theDescription = desc;
}

const rspfString& rspfApplicationUsage::getDescription() const
{
   return theDescription;
}

void rspfApplicationUsage::addUsageExplanation(Type type,const rspfString& option,const rspfString& explanation)
{
    switch(type)
    {
        case(RSPF_COMMAND_LINE_OPTION):
            addCommandLineOption(option,explanation);
            break;
        case(RSPF_ENVIRONMENTAL_VARIABLE):
            addEnvironmentalVariable(option,explanation);
            break;
    }
}
void rspfApplicationUsage::setCommandLineUsage(const rspfString& explanation)
{
   theCommandLineUsage=explanation;
}

const rspfString& rspfApplicationUsage::getCommandLineUsage() const
{
   return theCommandLineUsage;
}

void rspfApplicationUsage::addCommandLineOption(const rspfString& option,const rspfString& explanation)
{
    theCommandLineOptions[option]=explanation;
}

const rspfApplicationUsage::UsageMap& rspfApplicationUsage::getCommandLineOptions() const
{
   return theCommandLineOptions;
}

void rspfApplicationUsage::addEnvironmentalVariable(const rspfString& option,const rspfString& explanation)
{
    theEnvironmentalVariables[option]=explanation;
}
const rspfApplicationUsage::UsageMap& rspfApplicationUsage::getEnvironmentalVariables() const
{
   return theEnvironmentalVariables;
}

void rspfApplicationUsage::getFormatedString(rspfString& str, const UsageMap& um,unsigned int widthOfOutput)
{

    unsigned int maxNumCharsInOptions = 0;
    rspfApplicationUsage::UsageMap::const_iterator citr;
    for(citr=um.begin();
        citr!=um.end();
        ++citr)
    {
        maxNumCharsInOptions = rspf::max(maxNumCharsInOptions,(unsigned int)citr->first.length());
    }
    
    unsigned int fullWidth = widthOfOutput;
    unsigned int optionPos = 2;
    unsigned int explanationPos = 2+maxNumCharsInOptions+2;
    unsigned int explanationWidth = fullWidth-explanationPos;

    rspfString line;
    
    for(citr=um.begin();
        citr!=um.end();
        ++citr)
    {
        line.assign(fullWidth,' ');
        line.replace(optionPos,citr->first.length(),citr->first);
        
        const rspfString& explanation = citr->second;
        std::string::size_type pos = 0;
        std::string::size_type offset = 0;
        bool firstInLine = true;
        while (pos<explanation.length())
        {
            if (firstInLine) offset = 0;
                    
            // skip any leading white space.
            while (pos<explanation.length() && *(explanation.begin()+pos)==' ')
            {
                if (firstInLine) ++offset;
                ++pos;
            }
            
            firstInLine = false;
        
            std::string::size_type width = rspf::min((rspf_int64)(explanation.length()-pos),
                                                      (rspf_int64)(explanationWidth-offset));
            std::string::size_type slashn_pos = explanation.find('\n',pos);
            
            unsigned int extraSkip = 0;
            bool concatinated = false;
            if (slashn_pos!=std::string::npos)
            {
                if (slashn_pos<pos+width)
                {
                    width = slashn_pos-pos;
                    ++extraSkip;
                    firstInLine = true;
                }
                else if (slashn_pos==pos+width) 
                {
                    ++extraSkip;
                    firstInLine = true;
                }
            }
            
            if (pos+width<explanation.length())
            {
                // now reduce width until we get a space or a return
                // so that we ensure that whole words are printed.
                while (width>0 && 
                       *(explanation.begin()+(pos+width))!=' ' && 
                       *(explanation.begin()+(pos+width))!='\n') --width;
                       
                if (width==0)
                {
                    // word must be longer than a whole line so will need
                    // to concatinate it.
                    width = explanationWidth-1;
                    concatinated = true;
                }
            }

            line.replace(explanationPos+offset,explanationWidth, explanation, pos, width);

            if (concatinated) { str += line; str += "-\n"; }
            else { str += line; str += "\n"; }
            
            // move to the next line of output.
            line.assign(fullWidth,' ');
            
            pos += width+extraSkip;

            
        }
                
    }
}

void rspfApplicationUsage::write(std::ostream& output, const rspfApplicationUsage::UsageMap& um,unsigned int widthOfOutput)
{
    rspfString str;
    getFormatedString(str, um, widthOfOutput);
    output << str << std::endl;
}

void rspfApplicationUsage::write(std::ostream& output, unsigned int type, unsigned int widthOfOutput)
{

    output << "Usage: "<<getCommandLineUsage()<<std::endl;
    if(theDescription.size()>0)
    {
       // TODO: take into account "widthOfOutput"
       output << "\nDescription:\n" << theDescription.c_str() << "\n"
              << std::endl;
    }
    
    bool needspace = false;
    if ((type&RSPF_COMMAND_LINE_OPTION) && !getCommandLineOptions().empty())
    {
        if (needspace) output << std::endl;
        output << "Options:"<<std::endl;
        write(output,getCommandLineOptions(),widthOfOutput);
        needspace = true;
    }
    
    if ((type&RSPF_ENVIRONMENTAL_VARIABLE) && !getEnvironmentalVariables().empty())
    {
        if (needspace) output << std::endl;
        output << "Environmental Variables:"<<std::endl;
        write(output,getEnvironmentalVariables(),widthOfOutput);
        needspace = true;
    }

}

