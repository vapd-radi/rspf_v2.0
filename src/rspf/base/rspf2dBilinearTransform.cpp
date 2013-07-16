#include <rspf/base/rspf2dBilinearTransform.h>
#include <rspf/base/rspfLeastSquaresBilin.h>
#include <rspf/base/rspfKeywordlist.h>
#include <iostream>
RTTI_DEF1(rspf2dBilinearTransform, "rspf2dBilinearTransform", rspf2dTo2dTransform);
rspf2dBilinearTransform::rspf2dBilinearTransform()
{
   std::fill(m_coefficientsXTerm, m_coefficientsXTerm+4, 0.0);
   std::fill(m_coefficientsYTerm, m_coefficientsYTerm+4, 0.0);
   std::fill(m_inverseCoefficientsXTerm, m_inverseCoefficientsXTerm+4, 0.0);
   std::fill(m_inverseCoefficientsYTerm, m_inverseCoefficientsYTerm+4, 0.0);
   
   // setup identity
   //
   m_coefficientsXTerm[1] = 1.0;
   m_coefficientsYTerm[2] = 1.0;
   m_inverseCoefficientsXTerm[1] = 1.0;
   m_inverseCoefficientsYTerm[2] = 1.0;
}

rspf2dBilinearTransform::rspf2dBilinearTransform(const rspf2dBilinearTransform& src)
:rspf2dTo2dTransform(src)
{
   std::copy(src.m_coefficientsXTerm, src.m_coefficientsXTerm+4, m_coefficientsXTerm);
   std::copy(src.m_coefficientsYTerm, src.m_coefficientsYTerm+4, m_coefficientsYTerm);
   std::copy(src.m_inverseCoefficientsXTerm, src.m_inverseCoefficientsXTerm+4, m_inverseCoefficientsXTerm);
   std::copy(src.m_inverseCoefficientsYTerm, src.m_inverseCoefficientsYTerm+4, m_inverseCoefficientsYTerm);
}

void rspf2dBilinearTransform::setFromPoints(const rspfDpt& in1, const rspfDpt& in2, const rspfDpt& in3, const rspfDpt& in4,
                                             const rspfDpt& out1, const rspfDpt& out2, const rspfDpt& out3, const rspfDpt& out4)
{
   rspfDpt input[4];
   rspfDpt output[4];
   
   input[0] = in1;
   input[1] = in2;
   input[2] = in3;
   input[3] = in4;
   
   output[0] = out1;
   output[1] = out2;
   output[2] = out3;
   output[3] = out4;
   
   setFromPoints(&input[0], &output[0], 4);
}

void rspf2dBilinearTransform::setFromPoints(const rspfDpt* input,
                                             const rspfDpt* output, 
                                             rspf_uint32 arraySize)
{
   rspfLeastSquaresBilin inx;
   rspfLeastSquaresBilin inversex;
   rspfLeastSquaresBilin iny;
   rspfLeastSquaresBilin inversey;
   rspf_uint32 idx = 0;
   for(idx=0; idx < arraySize; ++idx)
   {
      inx.addSample(input[idx].x, input[idx].y, output[idx].x);
      inversex.addSample(output[idx].x, output[idx].y, input[idx].x);
      iny.addSample(input[idx].x, input[idx].y, output[idx].y);
      inversey.addSample(output[idx].x, output[idx].y, input[idx].y);
   }
   inx.solveLS();
   inversex.solveLS();
   iny.solveLS();
   inversey.solveLS();
   inx.getLSParms(m_coefficientsXTerm[0], m_coefficientsXTerm[1], m_coefficientsXTerm[2], m_coefficientsXTerm[3]);
   iny.getLSParms(m_coefficientsYTerm[0], m_coefficientsYTerm[1], m_coefficientsYTerm[2], m_coefficientsYTerm[3]);
   inversex.getLSParms(m_inverseCoefficientsXTerm[0], m_inverseCoefficientsXTerm[1], m_inverseCoefficientsXTerm[2], m_inverseCoefficientsXTerm[3]);
   inversey.getLSParms(m_inverseCoefficientsYTerm[0], m_inverseCoefficientsYTerm[1], m_inverseCoefficientsYTerm[2], m_inverseCoefficientsYTerm[3]);
}

bool rspf2dBilinearTransform::saveState(rspfKeywordlist& kwl,
                                       const char* prefix)const
{
   rspfString xterm = (rspfString::toString(m_coefficientsXTerm[0])
                        +" "+rspfString::toString(m_coefficientsXTerm[1])
                        +" "+rspfString::toString(m_coefficientsXTerm[2])
                        +" "+rspfString::toString(m_coefficientsXTerm[3]));
   rspfString yterm = (rspfString::toString(m_coefficientsYTerm[0])
                        +" "+rspfString::toString(m_coefficientsYTerm[1])
                        +" "+rspfString::toString(m_coefficientsYTerm[2])
                        +" "+rspfString::toString(m_coefficientsYTerm[3]));
   rspfString inverse_xterm = (rspfString::toString(m_inverseCoefficientsXTerm[0])
                        +" "+rspfString::toString(m_inverseCoefficientsXTerm[1])
                        +" "+rspfString::toString(m_inverseCoefficientsXTerm[2])
                        +" "+rspfString::toString(m_inverseCoefficientsXTerm[3]));
   rspfString inverse_yterm = (rspfString::toString(m_inverseCoefficientsYTerm[0])
                        +" "+rspfString::toString(m_inverseCoefficientsYTerm[1])
                        +" "+rspfString::toString(m_inverseCoefficientsYTerm[2])
                        +" "+rspfString::toString(m_inverseCoefficientsYTerm[3]));
   
   kwl.add(prefix, "xterm", xterm, true);
   kwl.add(prefix, "yterm", yterm, true);
   kwl.add(prefix, "inverse_xterm", inverse_xterm, true);
   kwl.add(prefix, "inverse_yterm", inverse_yterm, true);
   return rspf2dTo2dTransform::saveState(kwl, prefix);
}

bool rspf2dBilinearTransform::loadState(const rspfKeywordlist& kwl,
                                       const char* prefix)
{
   bool result = true;
   rspfString xterm = kwl.find(prefix, "xterm");
   rspfString yterm = kwl.find(prefix, "yterm");
   rspfString inverse_xterm = kwl.find(prefix, "inverse_xterm");
   rspfString inverse_yterm = kwl.find(prefix, "inverse_yterm");
   
   std::vector<rspfString> values;
   xterm.split(values, " ");
   if(values.size() == 4)
   {
      m_coefficientsXTerm[0] = values[0].toDouble();
      m_coefficientsXTerm[1] = values[1].toDouble();
      m_coefficientsXTerm[2] = values[2].toDouble();
      m_coefficientsXTerm[3] = values[3].toDouble();
   }
   else 
   {
      result = false;
   }
   values.clear();
   yterm.split(values, " ");
   if(values.size() == 4)
   {
      m_coefficientsYTerm[0] = values[0].toDouble();
      m_coefficientsYTerm[1] = values[1].toDouble();
      m_coefficientsYTerm[2] = values[2].toDouble();
      m_coefficientsYTerm[3] = values[3].toDouble();
   }
   else 
   {
      result = false;
   }
   values.clear();
   inverse_xterm.split(values, " ");
   if(values.size() == 4)
   {
      m_inverseCoefficientsXTerm[0] = values[0].toDouble();
      m_inverseCoefficientsXTerm[1] = values[1].toDouble();
      m_inverseCoefficientsXTerm[2] = values[2].toDouble();
      m_inverseCoefficientsXTerm[3] = values[3].toDouble();
   }
   else 
   {
      result = false;
   }
   values.clear();
   inverse_yterm.split(values, " ");
   if(values.size() == 4)
   {
      m_inverseCoefficientsYTerm[0] = values[0].toDouble();
      m_inverseCoefficientsYTerm[1] = values[1].toDouble();
      m_inverseCoefficientsYTerm[2] = values[2].toDouble();
      m_inverseCoefficientsYTerm[3] = values[3].toDouble();
   }
   else 
   {
      result = false;
   }

   if(result)
   {
      result = rspf2dTo2dTransform::loadState(kwl, prefix);
   }
   
   return result;
}
