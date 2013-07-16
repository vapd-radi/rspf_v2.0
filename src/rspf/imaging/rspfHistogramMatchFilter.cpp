#include <rspf/imaging/rspfHistogramMatchFilter.h>
#include <rspf/base/rspfFilenameProperty.h>
#include <rspf/base/rspfBooleanProperty.h>
#include <rspf/base/rspfVisitor.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageHandler.h>

RTTI_DEF1(rspfHistogramMatchFilter, "rspfHistogramMatchFilter", rspfImageSourceFilter);
rspfHistogramMatchFilter::rspfHistogramMatchFilter()
   :rspfImageSourceFilter(),
    theAutoLoadInputHistogramFlag(false)
{
   theInputHistogramEqualizer = new rspfHistogramEqualization;
   theTargetHistogramEqualizer = new rspfHistogramEqualization;

   theInputHistogramEqualizer->setInverseFlag(false);
   theTargetHistogramEqualizer->setInverseFlag(true);
   theTargetHistogramEqualizer->connectMyInputTo(0, theInputHistogramEqualizer.get());
}

rspfRefPtr<rspfImageData> rspfHistogramMatchFilter::getTile(const rspfIrect& tileRect,
                                                               rspf_uint32 resLevel)
{
   if(isSourceEnabled())
   {
      return theTargetHistogramEqualizer->getTile(tileRect, resLevel);
   }
   if(theAutoLoadInputHistogramFlag&&
      (theInputHistogramFilename==""))
   {
      autoLoadInputHistogram();
   }
   if(theInputConnection)
   {
      return theInputConnection->getTile(tileRect, resLevel);
   }

   return 0;
}


void rspfHistogramMatchFilter::setInputHistogram(const rspfFilename& inputHistogram)
{
   theInputHistogramFilename = inputHistogram;
   theInputHistogramEqualizer->setHistogram(inputHistogram);
   theInputHistogramEqualizer->initialize();
}

void rspfHistogramMatchFilter::setTargetHistogram(const rspfFilename& targetHistogram)
{
   theTargetHistogramFilename = targetHistogram;
   theTargetHistogramEqualizer->setHistogram(targetHistogram);
   theTargetHistogramEqualizer->initialize();
}

void rspfHistogramMatchFilter::connectInputEvent(rspfConnectionEvent& event)
{
   rspfImageSourceFilter::connectInputEvent(event);
   if(getInput())
   {
      theInputHistogramEqualizer->connectMyInputTo(0, getInput());
      theInputHistogramEqualizer->initialize();
      theTargetHistogramEqualizer->initialize();
   }
   else
   {
      theInputHistogramEqualizer->disconnectMyInput(0, false, false);
      theInputHistogramEqualizer->initialize();
      theTargetHistogramEqualizer->initialize();
   }
}

void rspfHistogramMatchFilter::disconnectInputEvent(rspfConnectionEvent& event)
{
   rspfImageSourceFilter::disconnectInputEvent(event);
   if(getInput())
   {
      theInputHistogramEqualizer->connectMyInputTo(0, getInput());
      theInputHistogramEqualizer->initialize();
      theTargetHistogramEqualizer->initialize();
   }
   else
   {
      theInputHistogramEqualizer->disconnectMyInput(0, false, false);
      theInputHistogramEqualizer->initialize();
      theTargetHistogramEqualizer->initialize();
   }
}

void rspfHistogramMatchFilter::initialize()
{
   rspfImageSourceFilter::initialize();
   if(theAutoLoadInputHistogramFlag)
   {
      if(theInputHistogramFilename == "")
      {
         autoLoadInputHistogram();
      }
   }

   theInputHistogramEqualizer->initialize();
   theTargetHistogramEqualizer->initialize();
}

void rspfHistogramMatchFilter::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property.valid()) return;
   
   rspfString name = property->getName();
   if(name == "input_histogram_filename")
   {
      setInputHistogram(rspfFilename(property->valueToString()));
      return;
   }
   else if(name == "target_histogram_filename")
   {
      setTargetHistogram(rspfFilename(property->valueToString()));
      return;
   }
   else if(name == "auto_load_input_histogram_flag")
   {
      theAutoLoadInputHistogramFlag = property->valueToString().toBool();
      if(theAutoLoadInputHistogramFlag)
      {
         initialize();
      }
      return;
   }
   rspfImageSourceFilter::setProperty(property);
}

rspfRefPtr<rspfProperty> rspfHistogramMatchFilter::getProperty(const rspfString& name)const
{
   if(name == "input_histogram_filename")
   {
      rspfFilenameProperty* filenameProp =
         new rspfFilenameProperty(name, theInputHistogramFilename);
      
      filenameProp->setIoType(rspfFilenameProperty::rspfFilenamePropertyIoType_INPUT);
      filenameProp->setCacheRefreshBit();
      return filenameProp;
   }
   else if(name == "target_histogram_filename")
   {
      rspfFilenameProperty* filenameProp =
         new rspfFilenameProperty(name, theTargetHistogramFilename);
      
      filenameProp->setIoType(rspfFilenameProperty::rspfFilenamePropertyIoType_INPUT);
      filenameProp->setCacheRefreshBit();

      return filenameProp;
   }
   if(name == "auto_load_input_histogram_flag")
   {
      rspfBooleanProperty* boolProp = new rspfBooleanProperty(name,
                                                                theAutoLoadInputHistogramFlag);
      boolProp->setCacheRefreshBit();
      return boolProp;
   }
   return rspfImageSourceFilter::getProperty(name);
}

void rspfHistogramMatchFilter::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageSourceFilter::getPropertyNames(propertyNames);
   
   propertyNames.push_back("auto_load_input_histogram_flag");
   propertyNames.push_back("input_histogram_filename");
   propertyNames.push_back("target_histogram_filename");
}

bool rspfHistogramMatchFilter::loadState(const rspfKeywordlist& kwl,
                                          const char* prefix)
{
   const char* inputFilename     = kwl.find(prefix, "input_histogram_filename");
   const char* targetFilename    = kwl.find(prefix, "target_histogram_filename");
   const char* autoLoadInputFlag = kwl.find(prefix, "auto_load_input_histogram_flag");

   if(autoLoadInputFlag)
   {
      theAutoLoadInputHistogramFlag = rspfString(autoLoadInputFlag).toBool();
   }
   
   if(inputFilename&&!theAutoLoadInputHistogramFlag)
   {
      setInputHistogram(rspfFilename(inputFilename));
   }
   else
   {
      setInputHistogram(rspfFilename(""));
   }

   if(targetFilename)
   {
      setTargetHistogram(rspfFilename(targetFilename));
   }
   else
   {
      setTargetHistogram(rspfFilename(""));
   }
   
   return rspfImageSourceFilter::loadState(kwl, prefix);
}

bool rspfHistogramMatchFilter::saveState(rspfKeywordlist& kwl,
                                          const char* prefix)const
{
   if(theAutoLoadInputHistogramFlag)
   {
      kwl.add(prefix,
              "input_histogram_filename",
              "",
              true);
   }
   else
   {
      kwl.add(prefix,
              "input_histogram_filename",
              theInputHistogramFilename.c_str(),
              true);
   }
   kwl.add(prefix,
           "target_histogram_filename",
           theTargetHistogramFilename.c_str(),
           true);
   kwl.add(prefix,
           "auto_load_input_histogram_flag",
           theAutoLoadInputHistogramFlag,
           true);
   

   return rspfImageSourceFilter::saveState(kwl, prefix);
}

void rspfHistogramMatchFilter::autoLoadInputHistogram()
{
   rspfTypeNameVisitor visitor(rspfString("rspfImageHandler"),
                                true,
                                rspfVisitor::VISIT_CHILDREN|rspfVisitor::VISIT_INPUTS);
   accept(visitor);
   
   if ( visitor.getObjects().size() )
   {
      rspfRefPtr<rspfImageHandler> ih = visitor.getObjectAs<rspfImageHandler>( 0 );
      if ( ih.valid() )
      {
         rspfRefPtr<rspfProperty> prop = ih->getProperty("histogram_filename");
         if( prop.valid() )
         {
            rspfFilename inputHisto = rspfFilename(prop->valueToString());
            if( inputHisto.exists() )
            {
               setInputHistogram(inputHisto);
            }
         }
      }
   }
}
