//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken, Oscar Kramer
//
// Description: Test code application utility class.
//
//----------------------------------------------------------------------------
// $Id$

#ifndef rspfBatchTest_HEADER
#define rspfBatchTest_HEADER 1

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfReferenced.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfFilename.h>
#include <vector>
#include <fstream>

// Forward class declarations:
class rspfArgumentParser;
class rspfDpt;
class rspfString;
class rspfGpt;
class rspfImageFileWriter;
class rspfImageGeometry;
class rspfIrect;
class rspfKeywordlist;
class rspfTilingRect;

class RSPF_DLL rspfBatchTest : public rspfReferenced
{
public:
   enum TEST_STATUS
   {
      TEST_TBD      = 0x00, // initial state: no test yet attempted
      TEST_PASSED   = 0x01,
      TEST_FAILED   = 0x02,
      TEST_ERROR    = 0x04,
      TEST_DISABLED = 0x08
   };

   //! This constructor only initializes data members to null/defaults
   rspfBatchTest();
   
   //! Initializes the test session given the command line.
   bool initialize(rspfArgumentParser& ap);

   //! Performs the actual test with the config filename previously set in initialize() or
   //! processConfigList() when config is a list of subordinate test config files.
   //! @return The overall bit-wise status of all tests (see TEST_STATUS enum for bit definitions).
   rspf_uint8 execute();

private:
   //! Writes template test config file, either exhaustive long form for flexibility, or simple
   //! short-form for easier test creation.
   void writeTemplate(const rspfFilename& templateFile, bool long_form);
   
   //! Fetches string from OS for naming and tagging the log file.
   void getDateString(rspfString& date);
   
   //! Establishes name of output log file.
   void getLogFilename(rspfFilename& logFile);

   //! When the config file consists of a list of subordinate test config files, this method manages
   //! processing multiple configs.
   //! @return The overall bit-wise status of all configs (see TEST_STATUS enum for bit definitions).
   rspf_uint8 processConfigList(const rspfKeywordlist& kwl);

   //! Within a single config file can be multiple tests, distinguished by the "test*." prefix. This
   //! method manages the execution of a single test.
   //! @return The overall bit-wise status of test (see TEST_STATUS enum for bit definitions).
   rspf_uint8 processTest(const rspfString& prefix, const rspfKeywordlist& kwl);

   //! Runs a single command within a test.
   //! @return The bit-wise status of command (see TEST_STATUS enum for bit definitions).
   rspf_uint8 processCommands(const rspfString& prefix,
                               const rspfKeywordlist& kwl,
                               const rspfString& testName,
                               bool logTime,
                               const rspfFilename& tempFile=rspfFilename(""));

   //! Modifies the config's KWL to explicitly declare implied keywords.
   void preprocessKwl(const std::vector<std::string>& testList,
                      const std::string& testCommand,
                      rspfKeywordlist& kwl);

   // Turns everything off.
   void disableAllKwl(rspfKeywordlist& kwl);

   //! Default preprocessing step makes expected and output results directories. Returns 
   //! @return TRUE if successful.
   bool makeDefaultResultsDir();

   //! Default clean step deletes all files in out and exp dirs.
   //! @return TRUE if successful.
   bool doDefaultClean();

   /**
    * @brief Gets the temp file name.
    *
    * This will either be from the config file lookup of "temp_file" or derived under
    * $(RSPF_BATCH_TEST_RESULTS)/tmp if the lookup fails.  Note that this will create
    * the $(RSPF_BATCH_TEST_RESULTS)/tmp directory if needed.
    *
    * @param prefix Like "test1."
    * @param kwl Keyword list to look for temp_file in.
    * @param tempFile Initialized by this.
    *
    * @return true on success.  False if derived temp file directory could
    * not be created.
    */
   bool getTempFileName( const rspfString& prefix,
                         const rspfKeywordlist& kwl,
                         rspfFilename& tempFile ) const;

   /**
    * @brief Gets the default temp directory $(RSPF_BATCH_TEST_RESULTS)/tmp.
    * @param tempDir Initialized by this.
    *
    * @return true on success, false on error.
    */
   bool getDefaultTempFileDir( rspfFilename& tempDir ) const;

   /** @brief Initializes arg parser and outputs usage. */
   void usage(rspfArgumentParser& ap);

   /**
    * @brief Converts string slashes to either forward or backward taking
    * into account that windows commands with forward slashes in them.
    * @param s String to convert.
    */
   std::string convertToNative( const char* lookup ) const;

   std::vector<std::string> m_acceptTestList;
   std::vector<std::string> m_cleanTestList;
   std::vector<std::string> m_preprocessTestList;
   std::vector<std::string> m_runTestList;

   bool          m_allIsDisabled;
   bool          m_templateModeActive;
   rspfFilename m_configFileName;
   rspfFilename m_outDir;
   rspfFilename m_expDir;
   std::ofstream m_logStr;
};

#endif /* #ifndef rspfBatchTest_HEADER */
