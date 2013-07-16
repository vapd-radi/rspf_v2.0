//-------------------------------------------------------------------------
//
// This code was taken from Open Scene Graph and incorporated from into
// RSPF.
//
//-------------------------------------------------------------------------
// $Id: rspfArgumentParser.h 19900 2011-08-04 14:19:57Z dburken $
#ifndef rspfArgumentParser_HEADER
#define rspfArgumentParser_HEADER 1
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfString.h>
#include <map>
#include <string>
#include <iosfwd>

class rspfApplicationUsage;

class RSPFDLLEXPORT rspfArgumentParser
{
public:
   
   class rspfParameter
   {
   public:
      enum rspfParameterType
      {
         RSPF_FLOAT_PARAMETER,
         RSPF_DOUBLE_PARAMETER,
         RSPF_INT_PARAMETER,
         RSPF_UNSIGNED_INT_PARAMETER,
         RSPF_STRING_PARAMETER,
      };
      
      union rspfValueUnion
      {
         float*          theFloat;
         double*         theDouble;
         int*            theInt;
         unsigned int*   theUint;
         std::string*    theString;
      };
      
      rspfParameter(float& value)
      {
         theType = RSPF_FLOAT_PARAMETER; theValue.theFloat = &value;
      }
      
      rspfParameter(double& value)
      {
         theType = RSPF_DOUBLE_PARAMETER; theValue.theDouble = &value;
      }
      
      rspfParameter(int& value)
      {
         theType = RSPF_INT_PARAMETER; theValue.theInt = &value;
      }
      
      rspfParameter(unsigned int& value)
      {
         theType = RSPF_UNSIGNED_INT_PARAMETER; theValue.theUint = &value;
      }
      
      rspfParameter(std::string& value)
      {
         theType = RSPF_STRING_PARAMETER; theValue.theString = &value;
      }
      
      rspfParameter(rspfString& value)
      {
         theType = RSPF_STRING_PARAMETER; theValue.theString =
                                              &(value.string());
      }
      
      bool valid(const char* str) const;
      bool assign(const char* str);
      
   protected:
      
      rspfParameterType   theType;
      rspfValueUnion      theValue;
   };
   
   /** return return true if specified string is an option in the form of
    * -option or --option .
    */
   static bool isOption(const char* str);
   
   /** return return true if string is any other string apart from an option.*/
   static bool isString(const char* str);
   
   /** return return true if specified parameter is an number.*/
   static bool isNumber(const char* str);
   
public:
   
   rspfArgumentParser(int* argc,char **argv);

   ~rspfArgumentParser();

   /** @brief Initialize from command arguments. */
   void initialize(int* argc, const char **argv); 
   
   void setApplicationUsage(rspfApplicationUsage* usage) { theUsage = usage; }
   rspfApplicationUsage* getApplicationUsage() { return theUsage; }
   const rspfApplicationUsage* getApplicationUsage() const { return theUsage; }
   
   /** return the argument count.*/
   int& argc() { return *theArgc; }
   
   /** return the argument array.*/
   char** argv() { return theArgv; }
   
   /** return char* argument at specificed position.*/
   char* operator [] (int pos) { return theArgv[pos]; }
   
   /** return const char* argument at specificed position.*/
   const char* operator [] (int pos) const { return theArgv[pos]; }
   
   /** return the application name, as specified by argv[0] */
   std::string getApplicationName() const;
   
   /** return the position of an occurence of a string in the argument list.
    * return -1 when no string is found.*/      
   int find(const std::string& str) const;
   
   /** return return true if specified parameter is an option in the form of -option or --option .*/
   bool isOption(int pos) const;
   
   /** return return true if specified parameter is an string, which can be any other string apart from an option.*/
   bool isString(int pos) const;
   
   /** return return true if specified parameter is an number.*/
   bool isNumber(int pos) const;
   
   bool containsOptions() const;
   
   /** remove one or more arguments from the argv argument list, and decrement the argc respectively.*/
   void remove(int pos,int num=1);
   
   /** return true if specified argument matches string.*/        
   bool match(int pos, const std::string& str) const;
   
   /** search for an occurance of a string in the argument list, on sucess
    * remove that occurance from the list and return true, otherwise return false.*/
   bool read(const std::string& str);
   bool read(const std::string& str, rspfParameter value1);
   bool read(const std::string& str, rspfParameter value1, rspfParameter value2);
   bool read(const std::string& str, rspfParameter value1, rspfParameter value2, rspfParameter value3);
   bool read(const std::string& str, rspfParameter value1, rspfParameter value2, rspfParameter value3, rspfParameter value4);
   
   /** Returns the number of parameters of type value associated with specified option, 
    *  or -1 if option not found */
   int numberOfParams(const std::string& str, rspfParameter value) const;
   
   /** if the argument value at the position pos matches specified string, and subsequent
    * paramters are also matched then set the paramter values and remove the from the list of arguments.*/
   bool read(int pos, const std::string& str);
   bool read(int pos, const std::string& str, rspfParameter value1);
   bool read(int pos, const std::string& str, rspfParameter value1, rspfParameter value2);
   bool read(int pos, const std::string& str, rspfParameter value1, rspfParameter value2, rspfParameter value3);
   bool read(int pos, const std::string& str, rspfParameter value1, rspfParameter value2, rspfParameter value3, rspfParameter value4);
   
   
   enum rspfErrorSeverity
   {
      RSPF_BENIGN = 0,
      RSPF_CRITICAL = 1
   };
   
   typedef std::map<std::string,rspfErrorSeverity> rspfErrorMessageMap;
   
   /** return the error flag, true if an error has occured when reading arguments.*/
   bool errors(rspfErrorSeverity severity=RSPF_BENIGN) const;
   
   /** report an error message by adding to the ErrorMessageMap.*/
   void reportError(const std::string& message,rspfErrorSeverity severity=RSPF_CRITICAL);
   
   /** for each remaining option report it as an unrecongnized.*/
   void reportRemainingOptionsAsUnrecognized(rspfErrorSeverity severity=RSPF_BENIGN);
   
   /** return the error message, if any has occured.*/
   rspfErrorMessageMap& getErrorMessageMap() { return theErrorMessageMap; }
   
   /** return the error message, if any has occured.*/
   const rspfErrorMessageMap& getErrorMessageMap() const { return theErrorMessageMap; }
   
   /** write out error messages at an above specified .*/
   void writeErrorMessages(std::ostream& output,rspfErrorSeverity sevrity=RSPF_BENIGN);
   
   
protected:
   
   int*                     theArgc;
   char**                   theArgv;
   rspfErrorMessageMap     theErrorMessageMap;
   rspfApplicationUsage*   theUsage;
        
};

#endif
