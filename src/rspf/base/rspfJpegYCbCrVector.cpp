#include <rspf/base/rspfJpegYCbCrVector.h>
#include <rspf/base/rspfCommon.h>

#include <rspf/base/rspfRgbVector.h>

rspfJpegYCbCrVector::rspfJpegYCbCrVector(const rspfRgbVector& rgbColor)
{
   theBuf[0] = static_cast<unsigned char>(clamp(rspf::round<int>(0.299*rgbColor.getR() +
                                          0.587*rgbColor.getG() +
                                          0.114*rgbColor.getB())));
   
   theBuf[1] = static_cast<unsigned char>(clamp(rspf::round<int>((-0.1687)*rgbColor.getR() -
                                                0.3313*rgbColor.getG() +
                                                0.5*rgbColor.getB() + 128)));
   
   theBuf[2] = static_cast<unsigned char>(clamp(rspf::round<int>(0.5*rgbColor.getR() -
                                                      .4187*rgbColor.getG() -
                                                      .0813*rgbColor.getB() + 128)));
   
}

rspfJpegYCbCrVector& rspfJpegYCbCrVector::operator =(const rspfRgbVector& rgbColor)
{
   theBuf[0] = static_cast<unsigned char>(clamp(rspf::round<int>(0.299*rgbColor.getR() +
                                                0.587*rgbColor.getG() +
                                                0.114*rgbColor.getB())));
   
   theBuf[1] = static_cast<unsigned char>(clamp(rspf::round<int>((-0.1687)*rgbColor.getR() -
                                                      0.3313*rgbColor.getG() +
                                                      0.5*rgbColor.getB() + 128)));
   
   theBuf[2] = static_cast<unsigned char>(clamp(rspf::round<int>(0.5*rgbColor.getR() -
                                                      .4187*rgbColor.getG() -
                                                      .0813*rgbColor.getB() + 128)));
   
   return *this;
}
