//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts
//*******************************************************************
//  $Id: rspfResampler.h 17195 2010-04-23 17:32:18Z dburken $

#ifndef rspfResampler_HEADER
#define rspfResampler_HEADER

#include <rspf/base/rspfConnectableObject.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfRationalNumber.h>

class rspfImageData;

/*!
 * This is currently implemented as a  symmetric kernel resampler.
 * It will use a lookup table idea to precompute
 *
 * The resampler will support Nearest neighbor, Bilinear and Bicubic
 *
 * The lookup table approach will be borrowed from Michael J. Aramini
 * implementation of the Bicubic convolution:
 *
 *       http://www.ultranet.com/~aramini/
 *       under:
 *        Efficient Image Magnification by Bicubic Spline Interpolation.
 */
class rspfResampler : public rspfConnectableObject
{   
public:
   enum rspfResLevelResamplerType
   {
      rspfResampler_NONE             = 0,
      rspfResampler_NEAREST_NEIGHBOR = 1,
      rspfResampler_BILINEAR         = 2,
      rspfResampler_BICUBIC          = 3
   };
   rspfResampler();

   
   /*!
    * Will apply the kernel to the input and write it to the output.
    *
    * Note: theTable is re-generated if the Max(out_width, out_height).
    *       changes from the previous call.
    */
   virtual void resample(rspfImageData* input, // input buffer
                         rspfImageData* output);


   virtual void resample(rspfImageData* input,
                         rspfImageData* output,
                         const rspfDpt& ul,
                         const rspfDpt& ur,
                         const rspfDpt& deltaUl,
                         const rspfDpt& deltaUr,
                         const rspfDpt& length);

   virtual void resample(rspfImageData* input,
                         rspfImageData* output,
                         const rspfIrect& outputSubRect,
                         const rspfDpt& ul,
                         const rspfDpt& ur,
                         const rspfDpt& deltaUl,
                         const rspfDpt& deltaUr,
                         const rspfDpt& length);

   virtual void resampleNearestNeighbor(rspfImageData* input,
                                        rspfImageData* output,
                                        const rspfIrect& outputSubRect,
                                        const rspfDpt& ul,
                                        const rspfDpt& ur,
                                        const rspfDpt& deltaUl,
                                        const rspfDpt& deltaUr,
                                        const rspfDpt& length);

   virtual void resampleNearestNeighbor(rspfImageData* input,
                                        rspfImageData* output,
                                        const rspfDpt& ul,
                                        const rspfDpt& ur,
                                        const rspfDpt& deltaUl,
                                        const rspfDpt& deltaUr,
                                        const rspfDpt& length);
   /*!
    * Will re-allocate the table
    */
   virtual void setResamplerType(rspfResLevelResamplerType type);

   virtual rspfResLevelResamplerType getResamplerType()const
      {
         return theResamplerType;
      }

   virtual double getCubicParameter()const
      {
         return theCubicAdjustableParameter;
      }
   virtual void setCubicParameter(double parameter)
      {
         theCubicAdjustableParameter = parameter;
         theCubicAdjustableParameter = theCubicAdjustableParameter<-1?-1:theCubicAdjustableParameter;
         theCubicAdjustableParameter = theCubicAdjustableParameter>0?0:theCubicAdjustableParameter;
         if(theResamplerType ==  rspfResampler_BICUBIC)
         {
            generateWeightTable();
         }
      }
   /*!
    * Will reallocate the table
    */
   void setRatio(double outputToInputRatio);
   void setRatio(const rspfDpt& outputToInputRatio);
   rspfDpt getRatio()const
      {
         return theOutputToInputRatio;
      }
   virtual rspf_int32 getKernelWidth()const;
   virtual rspf_int32 getKernelHeight()const;
   
   /*!
    * Method to save the state of an object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   
   /*!
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

   bool canConnectMyInputTo(rspf_int32 /* inputIndex */,
                            const rspfConnectableObject* /* object */)const
   {
      return false;
   }
protected:
   virtual ~rspfResampler();

   rspfDpt                   theOutputToInputRatio;
   rspfResLevelResamplerType theResamplerType;  

   rspf_int32                     theTableWidthX;
   rspf_int32                     theTableWidthY;
   rspf_int32                     theTableHeight;
   rspf_int32                     theKernelWidth;
   rspf_int32                     theKernelHeight;
   /*!
    */
   double **theWeightTableX;
   double **theWeightTableY;

   /*!
    * This adjustable parameter can vary between
    * -1 to 0.  as the paramter goes from 0
    * to -1 the output goes from blocky to smooth.
    * The default value is -.5
    */
   double   theCubicAdjustableParameter;

   template <class T>
   void resampleTile(T, // dummy tmeplate variable
                     rspfImageData* input,
                     rspfImageData* output);
   
   template <class T>
   void resampleFullTile(T, // dummy tmeplate variable
                         rspfImageData* input,
                         rspfImageData* output);
   template <class T>
   void resamplePartialTile(T, // dummy tmeplate variable
                            rspfImageData* input,
                            rspfImageData* output);
   
   template <class T>
   void resampleTile(T, // dummy template variable
                     rspfImageData* input,
                     rspfImageData* output,
                     const rspfIrect& outputSubRect,
                     const rspfDpt& ul,
                     const rspfDpt& ur,
                     const rspfDpt& deltaUl,
                     const rspfDpt& deltaUr,
                     const rspfDpt& length);

   template <class T>
   void resampleTileNearestNeighbor(T, // dummy template variable
                                    rspfImageData* input,
                                    rspfImageData* output,
                                    const rspfDpt& ul,
                                    const rspfDpt& ur,
                                    const rspfDpt& deltaUl,
                                    const rspfDpt& deltaUr,
                                    const rspfDpt& length);

   template <class T>
   void resampleTileNearestNeighbor(T, // dummy template variable
                                    rspfImageData* input,
                                    rspfImageData* output,
                                    const rspfIrect& subRect,
                                    const rspfDpt& ul,
                                    const rspfDpt& ur,
                                    const rspfDpt& deltaUl,
                                    const rspfDpt& deltaUr,
                                    const rspfDpt& length);
   
   virtual void deleteWeightTable();
   virtual void allocateWeightTable();
   virtual void generateWeightTable();

   double getCubicC0(double t)const;
   double getCubicC1(double t)const;
   double getCubicC2(double t)const;
   double getCubicC3(double t)const;
   
TYPE_DATA
};

#endif
