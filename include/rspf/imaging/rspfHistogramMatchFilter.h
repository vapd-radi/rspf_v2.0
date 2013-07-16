#ifndef rspfHistogramMatchFilter_HEADER
#define rspfHistogramMatchFilter_HEADER
#include <rspf/imaging/rspfImageSourceFilter.h>
#include <rspf/imaging/rspfHistogramEqualization.h>

/**
 * class rspfHistogramMatchFilter
 *
 * <pre>
 *
 * This class owns two rspfHistogramEqualization filters and will chain them together.  It will
 * make the left most filter the forward transform by transforming the input input to an equalized
 * space defined by the input histogram.  The right filter is the target histogram to match.  It will
 * take the equalized space and invert it to a target histogram.
 *
 * There is an auto load input histogram flag that if set true will always try to search for an input handler
 * and get the input handlers histogram file. and set it.  If this is enabled then the only other field that
 * is required is to specify the target histogram.  If a target histogram is not specified then the output
 *
 * will be an equalized image without the target applied.
 *
 * </pre>
 */ 
class RSPF_DLL rspfHistogramMatchFilter : public rspfImageSourceFilter
{
public:
   rspfHistogramMatchFilter();

   /**
    * Calls the owned equalizers to create a match composite image.
    *
    * @return Histogram matched composite image
    */
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                               rspf_uint32 resLevel=0);
   /**
    * Initializes the input and target equalizer filters.
    */ 
   virtual void initialize();

   /**
    * @param inputHistogram Input histogram to load
    */ 
   void setInputHistogram(const rspfFilename& inputHistogram);

   /**
    * @param targetHistogram Target histogram to match
    */ 
   void setTargetHistogram(const rspfFilename& targetHistogram);

   /**
    * @param event Connection event
    */ 
   virtual void connectInputEvent(rspfConnectionEvent& event);
   
   /**
    * @param event Connection event
    */ 
   virtual void disconnectInputEvent(rspfConnectionEvent& event);

   /**
    * <pre>
    *
    * Valid properties are:
    *
    * auto_load_input_histogram_flag
    *                   Is a boolean property and specifes whether or not the input histogram will be auto searched from
    *                   the conected input source chain.
    *
    * input_histogram_filename
    *                   Holds the input histogram filename
    *
    * output_histogram_filename
    *                   holds the output histogram filename
    *                   
    * </pre>
    * @param property Is the property to set.
    */
   virtual void setProperty(rspfRefPtr<rspfProperty> property);

   /**
    * Returns a property given the name
    *
    * Valid names are:
    * auto_load_input_histogram_flag
    * input_histogram_filename
    * output_histogram_filename
    *
    * Example:
    *
    * rspfRefPtr<rspfProperty> prop = obj->getProperty("auto_load_input_histogram_flag");
    *
    */ 
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;

   /**
    * Returns a list of proeprties.
    *
    * <pre>
    *
    * Valid properties are:
    *
    * auto_load_input_histogram_flag
    *                   Is a boolean property and specifes whether or not the input histogram will be auto searched from
    *                   the conected input source chain.
    *
    * input_histogram_filename
    *                   Holds the input histogram filename
    *
    * output_histogram_filename
    *                   holds the output histogram filename
    *                  
    * </pre>
    */ 
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;

   /**
    *
    * <pre>
    * Keywords to search and load:
    *
    * auto_load_input_histogram_flag
    * input_histogram_filename
    * output_histogram_filename
    *
    * </pre>
    * 
    * @param kwl  Holds the keywords for this object to load prefixed by the passed in prefix parameter
    * @param prefix Is the prefix value prepended to all keywords
    * 
    * @return True if succesful or flase otherwise
    * 
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=NULL);

   /**
    *
    * <pre>
    * Keywords:
    *
    * auto_load_input_histogram_flag
    * input_histogram_filename
    * output_histogram_filename
    *
    * </pre>
    * 
    * @param kwl  This object will store the keywords in kwl and prefix by the parameter prefix
    * @param prefix Is the prefix value prepended to all keywords
    * @return True if succesful or flase otherwise
    * 
    */
   
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=NULL)const;
   
protected:
   /**
    * Will search the input chain and get the histogram_filename property of the image handler.
    * Once retrieved it calls the setInputHistogram with the found histogram filename.  It does nothing
    * if the histogram filename does not exist
    * 
    */
   void autoLoadInputHistogram();
   
   rspfRefPtr<rspfHistogramEqualization> theInputHistogramEqualizer;
   rspfRefPtr<rspfHistogramEqualization> theTargetHistogramEqualizer;
   rspfFilename theInputHistogramFilename;
   rspfFilename theTargetHistogramFilename;
   bool          theAutoLoadInputHistogramFlag;
TYPE_DATA   
};
#endif
