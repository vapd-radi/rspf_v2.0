#ifndef rspfVpfFeatureClass_HEADER
#define rspfVpfFeatureClass_HEADER
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfErrorContext.h>
class rspfVpfCoverage;
class rspfVpfFeatureClass
{
public:
   rspfVpfFeatureClass();
   /*!
    * Will initialize the feature class for this
    * coverage and do some validation.
    */
   bool openFeatureClass(const rspfString& featureClassName,
                         const rspfFilename& featureClassFullPath,
                         const rspfFilename& table1,
                         const rspfString&   columnNameIdTable1,
                         const rspfFilename& table2,
                         const rspfString&   columnNameIdTable2);
  const rspfString& getName()const;
  
  /*!
   * This will specify if this feature is a primitive.
   * the names of certain primitives are:
   *
   * end   Entity node primitive
   * cnd   Connected node primitive
   * edg   Edge Primitive
   * fac   Face Primitive
   * txt   Text primitive
   */
  static bool isPrimitive(const rspfString& featureName);
  
   /*!
    * This just specifies if the input is a simple feature.
    *
    * Some of the simple features are:
    *
    * .pft  Point feature table
    * .lft  Line feature table
    * .aft  area feature table
    * .tft  text feature table
    */
   static bool isSimpleFeature(const rspfString& featureName);
   /*!
    * This just specifies if the input is a comples feature.
    *
    * .cft  Complex feature table.
    * 
    */
   static bool isComplexFeature(const rspfString& featureName);
   /*!
    * Will use the isComplexFeature and isSimpleFeature to
    * determine if the input is a feature
    */
   static bool isFeature(const rspfString& featureName);
   /*!
    * Will specifiy if it is a join table.
    *
    * Some of the join table type are:
    *
    * .cjt  Complex join table
    * .pjt  Point join table
    * .ljt  Line join table
    * .ajt  area join table
    * .tjt  text join table
    */
   static bool isJoin(const rspfString& featureName);
  rspfFilename getTable1()const;
  rspfFilename getTable2()const;
private:
  rspfFilename     theFeatureClassFullPath;
  rspfString       theFeatureClassName;
  rspfFilename     theTableName1;
  rspfString       theColumnNameId1;
  rspfFilename     theTableName2;
  rspfString       theColumnNameId2;
};
#endif
