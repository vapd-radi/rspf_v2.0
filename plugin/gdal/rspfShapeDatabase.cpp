#include <iomanip>
#include <rspfShapeDatabase.h>
#include <rspfShapeFile.h>
RTTI_DEF1(rspfShapeDatabase, "rspfShapeDatabase", rspfObject);
rspfString rspfShapeDatabaseField::fieldTypeAsString()const
{
   switch(theFieldType)
   {
   case FTString:
   {
      return "String";
   }
   case FTInteger:
   {
      return "Integer";
   }
   case FTDouble:
   {
      return "Double";
   }
   default:
   {
      return "Unknown";
   }
   };
   return "";
   
}
bool rspfShapeDatabaseRecord::getField(rspfShapeDatabaseField& result,
                                        rspf_uint32 i)
{
   if(i < theFieldArray.size())
   {
      result = theFieldArray[i];
      return true;
   }
   return false;
}
bool rspfShapeDatabaseRecord::setField(const rspfShapeDatabaseField& field,
                                        rspf_uint32 i)
{
   if(i < theFieldArray.size())
   {
      theFieldArray[i] = field;
      return true;
   }
   return false;
}
int rspfShapeDatabaseRecord::getNumberOfFields()const
{
   return theFieldArray.size();
}
void rspfShapeDatabaseRecord::setNumberOfFields(int n)
{
   if(n)
   {
      theFieldArray.resize(n);
   }
   else
   {
      theFieldArray.clear();
   }
}
rspf_int32 rspfShapeDatabaseRecord::getFieldIdx(const rspfString& name,
                                                  bool caseInsensitive)const
{
   rspfString searchString = name;
   if(caseInsensitive) searchString = searchString.downcase();
   rspf_int32 idx = 0;
   for(idx = 0; idx < (int)theFieldArray.size(); ++idx)
   {
      if(caseInsensitive)
      {
         if(rspfString(theFieldArray[idx].theName).downcase() == searchString)
         {
            return idx;
         }
      }
      else
      {
         if(theFieldArray[idx].theName == searchString)
         {
            return idx;
         }
      }
   }
   return -1;
}
std::ostream& operator <<(std::ostream& out, const rspfShapeDatabase& rhs)
{
   rhs.print(out);
   
   return out;
}
rspfShapeDatabase::rspfShapeDatabase()
   :theHandle(NULL),
    theFilename("")
{
   theRecordNumber = -1;
}
rspfShapeDatabase::~rspfShapeDatabase()
{
   close();
}
bool rspfShapeDatabase::open(const rspfFilename& file,
                              const rspfString& flags)
{
   if(isOpen()) close();
   theHandle = DBFOpen(file.c_str(), flags.c_str());
   if(theHandle)
   {
      theFilename = file;
      theRecordNumber = -1;
   }
   return (theHandle != NULL);
}
void rspfShapeDatabase::close()
{
   if(isOpen())
   {
      DBFClose(theHandle);
      theHandle = NULL;
      theRecordNumber = -1;
   }
}
int rspfShapeDatabase::getNumberOfRecords()const
{
   if(isOpen())
   {
      return theHandle->nRecords;
   }
   return 0;
}
bool rspfShapeDatabase::getRecord(rspfShapeDatabaseRecord& result)
{
   if(isOpen()&&( (theRecordNumber < theHandle->nRecords) ))
   {
      if(result.getNumberOfFields() != theHandle->nFields)
      {
         result.setNumberOfFields(theHandle->nFields);
      }
      
      char name[1024] = {'\0'};
      int width       = 0;
      int decimals    = 0;
      int iField      = 0;
      std::vector<int>         fieldWidths;
      
      for(iField = 0; iField < theHandle->nFields; ++iField)
      {   
         DBFFieldType fieldType = DBFGetFieldInfo(theHandle,
                                                  iField,
                                                  name,
                                                  &width,
                                                  &decimals);
         rspfShapeDatabaseField field;
         field.theName = name;
         field.theWidth = width;
         field.theDecimals = decimals;
         field.theFieldType = fieldType;
            
         rspfString key = "field";
         key+=rspfString::toString(iField+1);
         key+=(rspfString(".")+name+":");
         
         switch(fieldType)
         {
         case FTString:
         {
            field.theValue = DBFReadStringAttribute(theHandle, theRecordNumber, iField);
            break;
         }
         case FTInteger:
         {
            field.theValue = rspfString::toString(DBFReadIntegerAttribute(theHandle, theRecordNumber, iField));
            break;
         }
         case FTDouble:
         {
            field.theValue = rspfString::toString(DBFReadDoubleAttribute(theHandle, theRecordNumber, iField));
            break;
         }
		 case FTLogical:
		 {
			 break;
		 }
		 case FTInvalid:
		 {
			 break;
		 }
         }
         result.setField(field,
                         iField);
      }
      return true;
   }
   return false;
}
bool rspfShapeDatabase::getRecord(rspfShapeDatabaseRecord& result,
                                   int recordNumber)
{
   if(isOpen())
   {
      if(recordNumber < getNumberOfRecords())
      {
         theRecordNumber = recordNumber;
         return getRecord(result);
      }
   }
   
   return false;
}
   
bool rspfShapeDatabase::getNextRecord(rspfShapeDatabaseRecord& result)
{
   if(isOpen() && ((theRecordNumber+1) < getNumberOfRecords()))
   {
      ++theRecordNumber;
      return getRecord(result);
   }
   
   return false;
}
bool rspfShapeDatabase::isOpen()const
{
   return (theHandle!=NULL);
}
DBFHandle rspfShapeDatabase::getHandle()
{
   return theHandle;
}
const DBFHandle& rspfShapeDatabase::getHandle()const
{
   return theHandle;
}
std::ostream& rspfShapeDatabase::print(std::ostream& out)const
{
   if(isOpen())
   {
      out << std::setw(15)<<setiosflags(std::ios::left)
          <<"DB filename:" << theFilename << std::endl
          << std::setw(15)<<setiosflags(std::ios::left)
          <<"records:" << theHandle->nRecords << std::endl
          << std::setw(15)<<setiosflags(std::ios::left)
          <<"fields:" << theHandle->nFields << std::endl;
      char name[1024] = {'\0'};
      int width       = 0;
      int decimals    = 0;
      int iField      = 0;
      std::vector<int>         fieldWidths;
      
      for(iField = 0; iField < theHandle->nFields; ++iField)
      {
         DBFFieldType fieldType = DBFGetFieldInfo(theHandle,
                                                  iField,
                                                  name,
                                                  &width,
                                                  &decimals);
         rspfString s = "field " + rspfString::toString(iField+1) + " name:";
         switch(fieldType)
         {
         case FTString:
         case FTInteger:
         case FTDouble:
         {
            out << std::setw(15) << setiosflags(std::ios::left) << s.c_str() << name << std::endl;
            break;
         }
         default:
         {
            out << std::setw(15) << setiosflags(std::ios::left) << s.c_str() << "INVALID"<<std::endl;
            break;
         }
         }
      }
      for(int iShape = 0; iShape < theHandle->nRecords; ++iShape)
      {
         for(iField = 0; iField < theHandle->nFields; ++iField)
         {   
            DBFFieldType fieldType = DBFGetFieldInfo(theHandle,
                                                     iField,
                                                     name,
                                                     &width,
                                                     &decimals);
            
            rspfString key = "field";
            key+=rspfString::toString(iField+1);
            key+=(rspfString(".")+name+":");
            
            switch(fieldType)
            {
            case FTString:
            {
               
               out << std::setw(25) << setiosflags(std::ios::left) << key.c_str()
                   << DBFReadStringAttribute(theHandle, iShape, iField) <<std::endl;
               
               break;
            }
            case FTInteger:
            {
               out << std::setw(25) << setiosflags(std::ios::left) << key.c_str()
                   << DBFReadIntegerAttribute(theHandle, iShape, iField) << std::endl;
               
               break;
            }
            case FTDouble:
            {
               out << std::setw(25) << setiosflags(std::ios::left) << key.c_str()
                   << DBFReadDoubleAttribute(theHandle, iShape, iField) << std::endl;
               
               break;
            }
			case FTLogical:
			{
				break;
			}
			case FTInvalid:
			{
				break;
			}
            }
         }
         out << "_________________________________"<<std::endl;
      }
   }
   return out;
}
