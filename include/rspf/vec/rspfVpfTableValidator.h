#ifndef rspfVpfTableValidator_HEADER
#define rspfVpfTableValidator_HEADER
class rspfVpfTable;
class rspfVpfTableValidator
{
public:
   virtual ~rspfVpfTableValidator(){}
   virtual bool isValid(rspfVpfTable& aTable)const=0;
};
#endif
