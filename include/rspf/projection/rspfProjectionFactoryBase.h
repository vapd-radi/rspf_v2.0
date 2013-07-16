#ifndef rspfProjectionFactoryBase_HEADER
#define rspfProjectionFactoryBase_HEADER
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfObjectFactory.h>
class rspfProjection;
class rspfImageHandler;
class RSPF_DLL rspfProjectionFactoryBase : public rspfObjectFactory
{
public:
   
   /**
    * takes a filename. This filename can be an image file or
    * it can also be a rspf .geom file.  It could be other
    * forms of geom files as well.  The factories job will be to
    * determine what parser to use and return a projection if
    * successful.
    */
   virtual rspfProjection* createProjection(const rspfFilename& filename,
                                             rspf_uint32 entryIdx)const=0;
   /**
    * Take a projection type name.
    */
   virtual rspfProjection* createProjection(const rspfString& name)const=0;
   /**
    * Take a keywordlist.
    */
   virtual rspfProjection* createProjection(const rspfKeywordlist& kwl,
                                             const char* prefix)const=0;
   
   virtual rspfProjection* createProjection(rspfImageHandler* handler)const;
protected:
   /**
    * This method takes the filename and tries to find an external ".geom"
    * file.
    *
    * If the "entryIndx" is 0 and the "imageFile" is foo.ntf this will
    * look for "foo.geom".  If that fails it will look for foo_e0.geom.
    *
    * If the "entryIndx" is 5 and the "imageFile" is foo.ntf this will
    * look for foo_e5.geom.
    *
    * @param imageFile Image file that needs a projection.
    *
    * @param entryIdx For image files that support multiple images this is
    * the zero base index to that image.  Images that don't support multiple
    * indexes should just pass 0.
    *
    * @return An rspfProjection* on success or NULL on failure.
    */
   rspfProjection*
   createProjectionFromGeometryFile(const rspfFilename& imageFile,
                                    rspf_uint32 entryIdx)const;
   
TYPE_DATA
};
#endif
