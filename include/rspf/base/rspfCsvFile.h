#ifndef rspfCsvFile_HEADER
#define rspfCsvFile_HEADER
#include <rspf/base/rspfReferenced.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfString.h>
#include <map>
#include <vector>
#include <fstream>

class RSPF_DLL rspfCsvFile : public rspfReferenced
{
public:
   typedef std::vector<rspfString> StringListType;
   class RSPF_DLL Record : public rspfReferenced
      {
      public:
         typedef std::map<rspfString, rspfString, rspfStringLtstr> FieldMapType;
         typedef std::vector<rspfString> FieldValueListType;
                  
         Record(rspfCsvFile* csvFile);
         void setCsvFile(rspfCsvFile* file)
         {
            theCsvFile = file;
         }
         rspfCsvFile* csvFile()
         {
            return theCsvFile;
         }
         const rspfCsvFile* csvFile()const
         {
            return theCsvFile;
         }
         
         StringListType& values()
         {
            return theValues;
         }
         const StringListType& values()const
         {
            return theValues;
         }
         
         /**
          * Allows access to a field as read only.
          */
         bool valueAt(const rspfString& fieldName,
                      rspfString& value)const;
         
         bool valueAt(rspf_uint32 idx,
                      rspfString& value)const;
         /**
          * Allows one to access and write to the field.
          */
         rspfString& operator [](const rspfString& fieldName);
         const rspfString& operator [](const rspfString& fieldName)const;
         rspfString& operator [](rspf_uint32 idx);
         const rspfString& operator [](rspf_uint32 idx)const;
      protected:
         // used in out of bounds returns on the operators []
         //
         rspfString theDummyValue;
         StringListType theValues;
         rspfCsvFile* theCsvFile;
      };
   friend class Record;
   
   rspfCsvFile(const rspfString& separatorList=",");
   ~rspfCsvFile();
   
   /**
    * For now we will only support the read flag and open existing 
    * csv files.  Later we can add support for creating and writing csv files
    *
    */
   bool open(const rspfFilename& file, const rspfString& flags="r");
   void close();
   
   bool readHeader();
   /**
    * Read one record and returns null if no more records exist or returns a valid pointer
    * if a record exists.
    */
   rspfRefPtr<rspfCsvFile::Record> nextRecord();
      
   /**
    * Returns the header of the CSV file.
    */
   const StringListType& fieldHeaderList()const;
   
   rspf_int32 indexOfField(const rspfString& fieldName)const;
   
   static rspf_int32 INVALID_INDEX;
protected:
   bool readCsvLine(std::istream& inStream,
                    rspfCsvFile::StringListType& tokens)const;
   StringListType theFieldHeaderList;
   rspfRefPtr<rspfCsvFile::Record> theRecordBuffer;
   std::istream* theInputStream;
   rspfString theSeparatorList;
   bool theOpenFlag;
};
#endif
