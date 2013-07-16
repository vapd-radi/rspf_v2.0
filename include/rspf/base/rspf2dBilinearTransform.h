#ifndef rspf2dBilinearTransform_HEADER
#define rspf2dBilinearTransform_HEADER
#include <rspf/base/rspf2dTo2dTransform.h>
#include <algorithm>
/**
 * rspf2dBilinearTransform allows one to specify a set of input points and output points
 * and will fit a bilinear transform to those points.  Function of the form is solved for each  
 * output dimension.
 *
 * z(x,y) = a + b*x + c*y + d*x*y
 *
 * We use the rspfLeastSquareBilin solver to solve for the coefficients.
 *
 */
class RSPF_DLL rspf2dBilinearTransform : public rspf2dTo2dTransform
{
public: 
   /**
    * Initialize to the identity
    */
   rspf2dBilinearTransform();
   
   /**
    * Initialize the transform that best fits the input and output arrays.  
    * If the input and output are well dispersed and are 4 points it should fit
    * exactly.  Both arrays should be of equal number of points
    *
    * @param input the list of input points
    * @param output the list of output points to transform the input to
    * @param arraySize the number of points for the arrays.
    */
   rspf2dBilinearTransform(const rspfDpt* input,
                            const rspfDpt* output, 
                            rspf_uint32 arraySize)
   {
      setFromPoints(input, output, arraySize);
   }
   
   /**
    * Mapping 4 corners to an output 4 corners.
    *
    */
   rspf2dBilinearTransform(const rspfDpt& in1, const rspfDpt& in2, const rspfDpt& in3, const rspfDpt& in4,
                            const rspfDpt& out1, const rspfDpt& out2, const rspfDpt& out3, const rspfDpt& out4)
   {
      setFromPoints(in1, in2, in3, in4, out1, out2, out3, out4);
   }
   
   /**
    * Copy constructor
    */
   rspf2dBilinearTransform(const rspf2dBilinearTransform& src);
   
   /**
    * Duplication method that duplicates this object
    */
   virtual rspfObject* dup()const{return new rspf2dBilinearTransform(*this);}
   
   /**
    * Overloaded operator equal that allows for assignment.
    *
    * @param src the src data to copy into this object.  It will call the base classes
    * equal operator.
    */
   const rspf2dBilinearTransform& operator =(const rspf2dBilinearTransform& src)
   {
      if(this == &src) return *this;
      
      rspf2dTo2dTransform::operator =(*this); // call base classes equal operator
      std::copy(src.m_coefficientsXTerm, src.m_coefficientsXTerm+4, m_coefficientsXTerm);
      std::copy(src.m_coefficientsYTerm, src.m_coefficientsYTerm+4, m_coefficientsYTerm);
      std::copy(src.m_inverseCoefficientsXTerm, src.m_inverseCoefficientsXTerm+4, m_inverseCoefficientsXTerm);
      std::copy(src.m_inverseCoefficientsYTerm, src.m_inverseCoefficientsYTerm+4, m_inverseCoefficientsYTerm);
      
      return *this;
   }
   
   /**
    * Mapping 4 corners to an output 4 corners.  Will use the rspfLeastSquareBilin class to solve 
    * the bilinear coefficients that maps the given input points to the output points.
    */
   void setFromPoints(const rspfDpt& in1, const rspfDpt& in2, const rspfDpt& in3, const rspfDpt& in4,
                      const rspfDpt& out1, const rspfDpt& out2, const rspfDpt& out3, const rspfDpt& out4);

   /**
    * Initialize the transform that best fits the input and output arrays.  
    * If the input and output are well dispersed and are 4 points it should fit
    * exactly.  Both arrays should be of equal number of points
    *
    * @param input the list of input points
    * @param output the list of output points to transform the input to
    * @param arraySize the number of points for the arrays.
    */
   void setFromPoints(const rspfDpt* input,
                      const rspfDpt* output, 
                      rspf_uint32 arraySize);
 
   /**
    * forward transform will transform an input point to the output.  
    */
   virtual void forward(const rspfDpt& input,
                        rspfDpt& output) const
   {
      output.x = (m_coefficientsXTerm[0] + 
                  m_coefficientsXTerm[1]*input.x + 
                  m_coefficientsXTerm[2]*input.y + 
                  m_coefficientsXTerm[3]*input.x*input.y);
      output.y = (m_coefficientsYTerm[0] + 
                  m_coefficientsYTerm[1]*input.x + 
                  m_coefficientsYTerm[2]*input.y + 
                  m_coefficientsYTerm[3]*input.x*input.y);
   }
   
   /**
    * forward transform will transform an input point to the output and modify the passed in point
    * to the new value
    */
   virtual void forward(rspfDpt&  modify_this) const
   {
      double saveX = modify_this.x;
      modify_this.x = (m_coefficientsXTerm[0] + 
                       m_coefficientsXTerm[1]*modify_this.x + 
                       m_coefficientsXTerm[2]*modify_this.y + 
                       m_coefficientsXTerm[3]*modify_this.x*modify_this.y);
      modify_this.y = (m_coefficientsYTerm[0] + 
                       m_coefficientsYTerm[1]*saveX + 
                       m_coefficientsYTerm[2]*modify_this.y + 
                       m_coefficientsYTerm[3]*saveX*modify_this.y);
   }
   
   /**
    * Inverts the point back to the original input value.
    */
   virtual void inverse(const rspfDpt& input,
                        rspfDpt&       output) const
   {
      output.x = (m_inverseCoefficientsXTerm[0] + 
                  m_inverseCoefficientsXTerm[1]*input.x + 
                  m_inverseCoefficientsXTerm[2]*input.y + 
                  m_inverseCoefficientsXTerm[3]*input.x*input.y);
      output.y = (m_inverseCoefficientsYTerm[0] + 
                  m_inverseCoefficientsYTerm[1]*input.x + 
                  m_inverseCoefficientsYTerm[2]*input.y + 
                  m_inverseCoefficientsYTerm[3]*input.x*input.y);
   }
   
   /**
    * Inverts the point back to the original input value and modifies the passed in point
    * to the new value.
    */
   virtual void inverse(rspfDpt&  modify_this) const
   {
      double saveX = modify_this.x;
      modify_this.x = (m_inverseCoefficientsXTerm[0] + 
                       m_inverseCoefficientsXTerm[1]*modify_this.x + 
                       m_inverseCoefficientsXTerm[2]*modify_this.y + 
                       m_inverseCoefficientsXTerm[3]*modify_this.x*modify_this.y);
      modify_this.y = (m_inverseCoefficientsYTerm[0] + 
                       m_inverseCoefficientsYTerm[1]*saveX + 
                       m_inverseCoefficientsYTerm[2]*modify_this.y + 
                       m_inverseCoefficientsYTerm[3]*saveX*modify_this.y);
   }
   
   /**
    * Saves the state of this object.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix = 0)const;
   
   /**
    * loads the state of this object from a keywordlist.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix = 0);
   
   /**
    * prints the contents of this object.  Will also cal the base classes
    * print method.
    */
   virtual std::ostream& print(std::ostream& out) const
   {
      rspf2dTo2dTransform::print(out);
      out << "xTerm: " << m_coefficientsXTerm[0] << ", " << m_coefficientsXTerm[1] << ", "
          << m_coefficientsXTerm[2] << ", " << m_coefficientsXTerm[3] << "\n";
      out << "yTerm: " << m_coefficientsYTerm[0] << ", " << m_coefficientsYTerm[1] << ", "
          << m_coefficientsYTerm[2] << ", " << m_coefficientsYTerm[3] << "\n";
      
      out << "xInverseTerm: " << m_inverseCoefficientsXTerm[0] << ", " << m_inverseCoefficientsXTerm[1] << ", "
          << m_inverseCoefficientsXTerm[2] << ", " << m_inverseCoefficientsXTerm[3] << "\n";
      out << "yInverseTerm: " << m_inverseCoefficientsYTerm[0] << ", " << m_inverseCoefficientsYTerm[1] << ", "
          << m_inverseCoefficientsYTerm[2] << ", " << m_inverseCoefficientsYTerm[3] << "\n";
      
      return out;
   }
   
protected:
   rspf_float64 m_coefficientsXTerm[4]; // constant, linear x, linear y, cross xy
   rspf_float64 m_coefficientsYTerm[4]; // constant, linear x, linear y, cross xy

   rspf_float64 m_inverseCoefficientsXTerm[4]; // constant, linear x, linear y, cross xy
   rspf_float64 m_inverseCoefficientsYTerm[4]; // constant, linear x, linear y, cross xy
   TYPE_DATA;   
};

#endif
