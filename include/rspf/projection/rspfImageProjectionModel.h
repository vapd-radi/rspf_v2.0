#ifndef rspfImageProjectionModel_HEADER
#define rspfImageProjectionModel_HEADER 1
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfRtti.h>
#include <rspf/imaging/rspfImageModel.h>
class rspfProjection;
/**
 * @brief Class derived from rspfImageModel, this adds an image projection
 * for lineSampleToWorld and worldToLineSample.
 *
 * Note that image points fed to projection methods should be full
 * resolution with any sub image offset applied.
 */
class RSPF_DLL rspfImageProjectionModel : public rspfImageModel
{
public:
   
   /** @brief default constructor */
   rspfImageProjectionModel();
   /**
    * @brief Method to initialize class from an image handler.
    *
    * @param ih Image handler.
    */
   virtual void initialize(const rspfImageHandler& ih);
   /**
    * @brief Method to get projection.
    *
    * @return Constant pointer to projection or 0 if not initialized.
    */
   const rspfProjection* getProjection() const;
protected:
   /** @brief virtual destructor */
   virtual ~rspfImageProjectionModel();
   rspfProjection* theProjection;
TYPE_DATA
};
#endif /* #ifndef rspfImageProjectionModel_HEADER */
