//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Ken Melero
// 
// Description:  This class gives access to the Volume Header Label
//               (VOL) of a DTED Level 1 file.
//
//********************************************************************
// $Id: rspfDtedVol.h 16104 2009-12-17 18:09:59Z gpotts $

#ifndef rspfDtedVol_H
#define rspfDtedVol_H

#include <iosfwd>

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfErrorStatusInterface.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfRefPtr.h>

class rspfProperty;

class RSPF_DLL rspfDtedVol : public rspfErrorStatusInterface
{
public:
   rspfDtedVol(const rspfFilename& dted_file="",
                rspf_int32 offset=0);
   rspfDtedVol(std::istream& in);

   enum
   {
      VOL_LENGTH            = 80,
      VOL_ONE_LABEL_1       =  4,
      VOL_REEL_NUMBER       =  5,
      VOL_REEL_ACCESS       = 11,
      VOL_SPACE             = 12,
      VOL_ACCOUNT_NUMBER    = 38,
      VOL_BLANKS            = 52,
      VOL_ONE_LABEL_2       = 80,
      FIELD1_SIZE           = 3,
      FIELD2_SIZE           = 1,
      FIELD3_SIZE           = 6,
      FIELD4_SIZE           = 1,
      FIELD5_SIZE           = 26,
      FIELD6_SIZE           = 14,
      FIELD7_SIZE           = 28,
      FIELD8_SIZE           = 1
   };
   
   // The Recognition Sentinel signifies if the VOL record exists.
   rspfString getRecognitionSentinel() const;
   rspfString getReelNumber()           const;
   rspfString getAccountNumber()        const;
   rspf_int32 startOffset()             const;
   rspf_int32 stopOffset()              const;
   
   friend RSPF_DLL std::ostream& operator<<( std::ostream& os,
                                              const rspfDtedVol& vol);
   
   void parse(std::istream& in);

   /**
    * @brief Gets a property for name.
    * @param name Property name to get.
    * @return rspfRefPtr<rspfProperty> Note that this can be empty if
    * property for name was not found.
    */
   rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;

   /**
    * @brief Adds this class's properties to list.
    * @param propertyNames list to append to.
    */
   void getPropertyNames(std::vector<rspfString>& propertyNames)const;

   /**
    * @brief print method that outputs a key/value type format adding prefix
    * to keys.
    * @param out String to output to.
    * @param prefix This will be prepended to key.
    * e.g. Where prefix = "nitf." and key is "file_name" key becomes:
    * "nitf.file_name:"
    * @return output stream.
    */
   std::ostream& print(std::ostream& out,
                       const std::string& prefix) const;
private:
   // Do not allow...
   rspfDtedVol(const rspfDtedVol& source);
   const rspfDtedVol& operator=(const rspfDtedVol& rhs);

   char theRecSen[FIELD1_SIZE+1];
   char theField2[FIELD2_SIZE+1];
   char theReelNumber[FIELD3_SIZE+1];
   char theField4[FIELD4_SIZE+1];
   char theField5[FIELD5_SIZE+1];
   char theAccountNumber[FIELD6_SIZE+1];
   char theField7[FIELD7_SIZE+1];
   char theField8[FIELD8_SIZE+1];
  
   rspf_int32 theStartOffset;
   rspf_int32 theStopOffset;
};

#endif
