#ifndef rspfLensDistortion_HEADER
#define rspfLensDistortion_HEADER
#include <rspf/base/rspf2dTo2dTransform.h>
class RSPF_DLL rspfLensDistortion : public rspf2dTo2dTransform
{
public:
   rspfLensDistortion(const rspfDpt& callibratedCenter=rspfDpt(0,0))
      :theCenter(callibratedCenter)
   {
   }
   void setCenter(const rspfDpt& center)
   {
      theCenter = center;
   }
   virtual void forward(const rspfDpt& input,
                        rspfDpt& output) const
   {
      distort(input, output);
   }
   virtual void inverse(const rspfDpt& input,
                        rspfDpt& output)
   {
      undistort(input, output);
   }
   virtual void distort(const rspfDpt& input, rspfDpt& output)const;
   virtual void undistort(const rspfDpt& input, rspfDpt& output)const=0;
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix = 0)const;
   
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix = 0);
   
protected:
   rspfDpt theCenter;
TYPE_DATA   
};
#endif
