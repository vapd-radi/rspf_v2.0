//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: Container class for a tile of image data.
//
//*******************************************************************
// $Id: rspfImageData.h 19931 2011-08-10 11:53:25Z gpotts $
#ifndef rspfImageData_HEADER
#define rspfImageData_HEADER 1

#include <vector>
#include <iosfwd> /* for ostream */

#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfRectilinearDataObject.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfRefPtr.h>

class rspfMultiBandHistogram;

class RSPFDLLEXPORT rspfImageData : public rspfRectilinearDataObject
{
public:

   /** Definitions for the unloadBand routines. */
   enum OverwriteBandRule
   {
      /*
       * The COLOR_DISCREPANCY rule directs that the selected band of a pixel
       * of the destination buffer is to be overwritten if *any* one of the
       * other bands of the same destination pixel have color values that 
       * deviate from its color. 
       * Deprecated in favor of COLOR_DISCREPANCY_OF_ANY_FROM_DEST.
       */
      COLOR_DISCREPANCY = 0,

      /*
       * The COLOR_DISCREPANCY_OF_ANY_FROM_DEST rule directs that the selected 
       * band of a pixel of the destination buffer is to be overwritten if 
       * *any* one of the other bands of the same destination pixel have color 
       * values that deviate from its color. 
       * Same as the deprecated COLOR_DISCREPANCY.
       */
      COLOR_DISCREPANCY_OF_ANY_FROM_DEST = 1,

      /*
       * The COLOR_DISCREPANCY_OF_ALL_FROM_DEST rule directs that the selected 
       * band of a pixel of the destination buffer is to be overwritten only if 
       * *all* of the other bands of the same destination pixel have color 
       * values that deviate from its color. 
       */
      COLOR_DISCREPANCY_OF_ALL_FROM_DEST = 2,

      /*
       * The COLOR_EQUALITY_OF_ANY_TO_SRC rule directs that the selected 
       * band of a pixel of the destination buffer is to be overwritten if 
       * *any* one of the other bands of the same destination pixel have color 
       * values that are equal to the color of the selected band of the source
       * pixel. 
       */
      COLOR_EQUALITY_OF_ANY_TO_SRC = 3,

      /*
       * The COLOR_EQUALITY_OF_ALL_TO_SRC rule directs that the selected 
       * band of a pixel of the destination buffer is to be overwritten only if 
       * *all* of the other bands of the same destination pixel have color 
       * values that are equal to the color of the selected band of the source
       * pixel. 
       */
      COLOR_EQUALITY_OF_ALL_TO_SRC = 4,

      /*
       * The selected band of the destination buffer is to be overwritten 
       * by the selected band of the source image data (no questions asked).
       */
      NULL_RULE = 5
   };

   /** @brief copy constructor */
   rspfImageData(const rspfImageData &rhs);
   
   rspfImageData();

   /**
    * @brief Constructor
    * @param source The owner.  This can be null.
    * @param scalar The Scalar type like RSPF_UINT8, RSPF_UINT16...
    * @param bands  The number of bands.
    * 
    * @note This does not initialize the undelying data buffer.
    *       Call the initialize method to allocate storage.
    */
   rspfImageData(rspfSource* source,
                  rspfScalarType scalar,
                  rspf_uint32 bands = 1);

   /**
    * @brief Constructor
    * @param source The owner.  This can be null.
    * @param scalar The Scalar type like RSPF_UINT8, RSPF_UINT16...
    * @param bands  The number of bands.
    * @param width  The width or number of samples in the buffer.
    * @param height The height or numer of lines in the buffer.
    * 
    * @note This does not initialize the undelying data buffer.
    *       Call the initialize method to allocate storage.
    */
   rspfImageData(rspfSource* source,
                  rspfScalarType scalar,
                  rspf_uint32 bands,
                  rspf_uint32 width,
                  rspf_uint32 height);

   /** @brief virtual destructor */
   virtual ~rspfImageData();

   /**
    * @brief assignment operator=
    * @param rhs The data to assign from.
    * @param A reference to this object.
    */
   virtual const rspfImageData& operator=(const rspfImageData &rhs);


   /**
    * @return The width of the data object.
    */
   virtual rspf_uint32 getWidth()const;

   /**
    * @return The height of the data object.
    */
   virtual rspf_uint32 getHeight()const;

   /**
    * @return The image rectangle relative to origin, height and width.
    */
   virtual rspfIrect getImageRectangle() const;
   virtual void getWidthHeight(rspf_uint32& w, rspf_uint32& h);
   virtual void setWidth(rspf_uint32 width);
   virtual void setHeight(rspf_uint32 height);
   virtual void setWidthHeight(rspf_uint32 w, rspf_uint32 h);
   virtual void setImageRectangleAndBands(const rspfIrect& rect,
                                          rspf_uint32 numberOfBands);
   virtual void setImageRectangle(const rspfIrect& rect);
   virtual void setOrigin(const rspfIpt& origin);   
                                    
   virtual void assign(const rspfDataObject* data);
   virtual void assign(const rspfImageData* data);
   virtual void assignBand(const rspfImageData* data,
                           rspf_uint32 source_band,
                           rspf_uint32 output_band);
   
   virtual rspfObject* dup() const;

  /**
   * Uses prime numbers as coefficients for this summation.  
   * Take the the fours bytes of each origin and multiply 
   * by the prime numbers and sum together.  This has
   * been proven by others to give nicely distributed 
   * id number.
   */
   virtual rspf_uint32 getHashId()const;
   
   virtual void setNullPix(rspf_float64 null_pix);
   virtual void setNullPix(rspf_float64 null_pix, rspf_uint32 band);
   virtual void setNullPix(const rspf_float64* nullPixArray,
                           rspf_uint32 numberOfValues);
   virtual void setMinPix(rspf_float64 min_pix);
   virtual void setMinPix(rspf_float64 min_pix, rspf_uint32 band);
   virtual void setMinPix(const rspf_float64* minPixArray,
                          rspf_uint32 numberOfValues);
   virtual void setMaxPix(rspf_float64 max_pix);
   virtual void setMaxPix(rspf_float64 max_pix, rspf_uint32 band);
   virtual void setMaxPix(const rspf_float64* maxPixArray,
                          rspf_uint32 numberOfValues);

   virtual const rspfIpt& getOrigin()const{return m_origin;}

   /**
    * If the minBands and maxBands are empty or not equal to the imageData's
    * current number of bands it will erase the contents, resize, and then
    * replace the values in the passed in array with this objects min max
    * values.  If the arrays are the same size as this objects number of
    * bands it will just update the values and use the current values in the
    * arrays as initial min max values.
    *
    * If the status of this object(getDataObjectStatus()) is RSPF_NULL or
    * RSPF_EMPTY no action is performed.
    */
   virtual void computeMinMaxPix(std::vector<rspf_float64>& minBands,
                                 std::vector<rspf_float64>& maxBands) const;

   /**
    * @brief Scans tile for min, max, nulls.
    * 
    * If the nullBands, minBands and maxBands are empty or not equal to the
    * imageData's current number of bands it will erase the contents, resize,
    * and then replace the values in the passed in array with this objects
    * min max values.  If the arrays are the same size as this objects number
    * of bands it will just update the values and use the current values in the
    * arrays as initial min max values.
    *
    * If the status of this object(getDataObjectStatus()) is RSPF_NULL no
    * action is performed.
    */
   virtual void computeMinMaxNulPix(
      std::vector<rspf_float64>& minBands,
      std::vector<rspf_float64>& maxBands,
      std::vector<rspf_float64>& nulBands) const;
   
   virtual const rspf_float64* getNullPix()const;
   virtual const rspf_float64* getMinPix()const;
   virtual const rspf_float64* getMaxPix()const;
   virtual rspf_float64        getNullPix(rspf_uint32 band) const;
   virtual rspf_float64        getMinPix(rspf_uint32 band)  const;
   virtual rspf_float64        getMaxPix(rspf_uint32 band)  const;

   /**
    * returns normalized minimum pixel value of band zero.
    */
   virtual rspf_float64   getMinNormalizedPix() const;
   virtual rspfString     getScalarTypeAsString()const;
   virtual rspf_uint32    getNumberOfBands() const;
   virtual void            setNumberOfBands(rspf_uint32 bands,
                                            bool reallocate=false);

   /**
    * Will return the pixel at location position.  Note it will first get
    * the passed in position relative to the origin or upper left
    * corner of this tile and then return the result.
    */
   virtual rspf_float64 getPix(const rspfIpt& position,
                                rspf_uint32 band=0) const;

   /**
    * Will return the pixel at offset and band number.
    */
   virtual rspf_float64 getPix(rspf_uint32 offset,
                                rspf_uint32 band = 0) const;
   
   /**
    * will fill the entire band with
    * the value.
    */
   void fill(rspf_uint32 band, rspf_float64 value);

   /**
    * will fill all bands with the passed in
    * value.
    */
   void fill(rspf_float64 value);
   bool isNull(rspf_uint32 offset)const;
   bool isNull(rspf_uint32 offset, rspf_uint32 band)const;
   void setNull(rspf_uint32 offset);
   void setNull(rspf_uint32 offset, rspf_uint32 band);

   bool isNull(const rspfIpt& pt)const;
   void setNull(const rspfIpt& pt);
   bool isNull(const rspfIpt& pt, rspf_uint32 band)const;
   void setNull(const rspfIpt& pt, rspf_uint32 band);
   
   virtual bool   isValidBand(rspf_uint32 band) const;
   virtual rspfDataObjectStatus validate() const;

   /**
    * Will take this tile and normalize it to a newly
    * allocated floating point tile.
    */
   virtual rspfRefPtr<rspfImageData> newNormalizedFloat()const;

   /**
    * will go to the band and offset and compute the
    * normalized float and return it back to the
    * caller through the result argument.
    */
   virtual void getNormalizedFloat(rspf_uint32 offset,
                                   rspf_uint32 bandNumber,
                                   rspf_float32& result)const;

   /**
    * This will assign to this object a normalized
    * value by unnormalizing to its native type.
    */
   virtual void setNormalizedFloat(rspf_uint32 offset,
                                   rspf_uint32 bandNumber,
                                   rspf_float32 input);

   /**
    * Will use the memory that you pass in to normalize
    * this data object.
    */
   virtual void convertToNormalizedFloat(rspfImageData* result)const;

   /**
    * Will take this tile and normalize it to a newly
    * allocated double point tile.
    */
   virtual rspfImageData* newNormalizedDouble()const;

   /**
    * Will use the memory that you pass in to normalize
    * this data object.
    */
   virtual void convertToNormalizedDouble(rspfImageData* result)const;

   /**
    * Will take the normalized input and convert it
    * to this tile's data type.  Example:  if this
    * tile is of type UCHAR and its input is of type
    * NORALIZED_FLOAT it will unnormalize the data by
    * doing:
    *
    * minPix + normalizedInput*(maxPix-minPix)
    *
    * on a per band basis.
    */
   virtual void unnormalizeInput(rspfImageData* normalizedInput);

   /**
    * This will compute the average value for the band.
    */
   virtual rspf_float64 computeAverageBandValue(
      rspf_uint32 bandNumber = 0) const;

   /**
    * This will call the compute average band value and then
    * use that in the calculation of:
    
    * It will then do a SUM[(Mean - Actual)^2]/width*height.
    *
    * This is the average variance from the passed in
    * mean.  
    */
   virtual rspf_float64 computeMeanSquaredError(
      rspf_float64 meanValue,
      rspf_uint32 bandNumber = 0) const;
  
   virtual void populateHistogram(rspfRefPtr<rspfMultiBandHistogram> histo);

   /**
    * @return true if alpha channel is initialized, false if not.
    */
   virtual bool hasAlpha() const;
   
   /**
    * @return const rspf_uint8* to the alpha channel or null if the alpha
    * channel was not computed.
    */
   virtual const rspf_uint8* getAlphaBuf() const;

   /**
    * @return rspf_uint8* to the alpha channel or null if the alpha channel
    * was not computed.
    */
   virtual rspf_uint8* getAlphaBuf();

   /**
    * @return const void* to theDataBuffer
    */
   virtual const void* getBuf() const;
   
   /**
    * @return const rspf_uint8* to theDataBuffer if scalar is of type
    * RSPF_UCHAR; else, NULL.
    */
   virtual const rspf_uint8*    getUcharBuf()  const;

   /**
    * @return const rspf_uint16* to theDataBuffer if scalar is of type
    * RSPF_USHORT16 or USHORT11; else, NULL.
    */
   virtual const rspf_uint16*   getUshortBuf() const;

   /**
    * @return const rspf_sint16* to theDataBuffer if scalar is of type
    * RSPF_SSHORT16; else, NULL.
    */
   virtual const rspf_sint16*   getSshortBuf() const;

   /**
    * @return const rspf_float32* to theDataBuffer if scalar is of type
    * RSPF_FLOAT or RSPF_NORMALIZED_FLOAT; else, NULL.
    */
   virtual const rspf_float32*  getFloatBuf()  const;

   /**
    * @return const rspf_float64* to theDataBuffer if scalar is of type
    * RSPF_DOUBLE or RSPF_NORMALIZED_DOUBLE; else, NULL.
    */
   virtual const rspf_float64*  getDoubleBuf() const;

   /**
    * @return void* to theDataBuffer
    */
   virtual void* getBuf();
   
   /**
    * @return rspf_uint8* to theDataBuffer if scalar is of type
    * RSPF_UCHAR; else, NULL.
    */
   virtual rspf_uint8*    getUcharBuf();
   
   /**
    * @return rspf_uint16* to theDataBuffer if scalar is of type
    * RSPF_USHORT16 or RSPF_USHORT11; else, NULL.
    */
   virtual rspf_uint16*   getUshortBuf();
   
   /**
    * @return rspf_sint16* to theDataBuffer if scalar is of type
    * RSPF_SSHORT16; else, NULL.
    */
   virtual rspf_sint16*   getSshortBuf();

   /**
    * @return rspf_float32* to theDataBuffer if scalar is of type
    * RSPF_FLOAT or RSPF_NORMALIZED_FLOAT; else, NULL.
    */
   virtual rspf_float32*  getFloatBuf();
   
   /**
    * @return float64* to theDataBuffer if scalar is of type RSPF_DOUBLE or
    * RSPF_NORMALIZED_DOUBLE; else, NULL.
    */
   virtual rspf_float64*  getDoubleBuf();

   /**
    * @param band (zero based)
    * @return void* to theDataBuffer positioned at band.
    */
   virtual const void*           getBuf(rspf_uint32 band) const;
   
   /**
    * @param band (zero based)
    * @return rspf_uint8* to theDataBuffer positioned at band if scalar is
    * of type RSPF_UCHAR; else, NULL.
    */
   virtual const rspf_uint8*    getUcharBuf(rspf_uint32 band) const;

   /**
    * @param band (zero based)
    * @return rspf_uint16* to theDataBuffer positioned at band if scalar is
    * of type RSPF_USHORT16 or RSPF_USHORT11; else, NULL.
    */
   virtual const rspf_uint16*   getUshortBuf(rspf_uint32 band) const;
   
   /**
    * @param band (zero based)
    * @return rspf_sint16* to theDataBuffer positioned at band if scalar is
    * of type RSPF_SSHORT16; else, NULL.
    */
   virtual const rspf_sint16*   getSshortBuf(rspf_uint32 band) const;
   
   /**
    * @param band (zero based)
    * @return rspf_float32* to theDataBuffer positioned at band if scalar is
    * of type RSPF_FLOAT or RSPF_NORMALIZED_FLOAT; else, NULL.
    */
   virtual const rspf_float32*  getFloatBuf(rspf_uint32 band) const;
   
   /**
    * @param band (zero based)
    * @return rspf_float64* to theDataBuffer positioned at band if scalar is
    * of type RSPF_DOUBLE or RSPF_NORMALIZED_DOUBLE; else, NULL.
    */
   virtual const rspf_float64*  getDoubleBuf(rspf_uint32 band) const;

   /**
    * @param band (zero based)
    * @return void* to theDataBuffer positioned at band.
    */
   virtual void*           getBuf(rspf_uint32 band);
   
   /**
    * @param band (zero based)
    * @return rspf_uint8* to theDataBuffer positioned at band if scalar is
    * of type RSPF_UCHAR; else, NULL.
    */
   virtual rspf_uint8*    getUcharBuf(rspf_uint32 band);
   
   /**
    * @param band (zero based)
    * @return rspf_uint16* to theDataBuffer positioned at band if scalar is
    * of type RSPF_USHORT16 or RSPF_USHORT11; else, NULL.
    */
   virtual rspf_uint16*   getUshortBuf(rspf_uint32 band);
   
   /**
    * @param band (zero based)
    * @return rspf_sint16* to theDataBuffer positioned at band if scalar is
    * of type RSPF_SSHORT16; else, NULL.
    */
   virtual rspf_sint16*   getSshortBuf(rspf_uint32 band);
   
   /**
    * @param band (zero based)
    * @return rspf_float32* to theDataBuffer positioned at band if scalar is
    * of type RSPF_FLOAT or RSPF_NORMALIZED_FLOAT; else, NULL.
    */
   virtual rspf_float32*  getFloatBuf(rspf_uint32 band);
   
   /**
    * @param band (zero based)
    * @return rspf_float64* to theDataBuffer positioned at band if scalar is
    * of type RSPF_DOUBLE or RSPF_NORMALIZED_DOUBLE; else, NULL.
    */
   virtual rspf_float64*  getDoubleBuf(rspf_uint32 band);
   
   /**
    *  Returns the total number of pixels in a tile for all bands.
    *  Note:  This is not in bytes but in pixels.
    */
   virtual rspf_uint32 getSize() const;

   /**
    *  Returns the number of pixels in a single band in a tile.
    *  Note:  This is not in bytes but in pixels.
    */
   virtual rspf_uint32 getSizePerBand()const;

   /**
    *  Returns the number of bytes in single band of the tile.
    */
   virtual rspf_uint32 getSizePerBandInBytes() const;

   virtual rspf_uint32 getDataSizeInBytes()const;

   /**
    * Returns the total number of bytes for all bands.
    */
   virtual rspf_uint32  getSizeInBytes() const;

   /**
    *  Initializes data to null pixel values.
    */
   virtual void makeBlank();

   /**
    *  Initialize the data buffer.
    */
   virtual void initialize();

   /**
    * @brief Writes tile to stream.
    *
    * This will write the buffer to the stream assuming a contiguous buffer in
    * BSQ format.  Currently does not support converting to BIP or BIL
    * or byte swapping but probably should add at some point.
    *
    * @param f File to write.
    *
    * @return true on success, false on error.
    */
   virtual bool write(const rspfFilename& f) const;

   /**
    * Copies entire tile to buf passed in.  Data put in buf is normalized.
    * The "buf" passed to method is assumed to be at least as big as:
    * "getSize() * sizeof(double)"
    */
   virtual void copyTileToNormalizedBuffer(rspf_float64* buf) const;
   virtual void copyTileToNormalizedBuffer(rspf_float32* buf)const;

   /**
    * Will copy this tiles specified band number to the normalized buffer.
    * if the band is out of range then nothing is done and returns.
    *
    * @param band Zero based band to copy.
    *
    * @param buf Buffer to copy to.
    */
   virtual void copyTileBandToNormalizedBuffer(rspf_uint32 band,
                                               rspf_float64* buf) const;
   
   /**
    * Will copy this tiles specified band number to the normalized buffer.
    * if the band is out of range then nothing is done and returns.
    *
    * @param band Zero based band to copy.
    *
    * @param buf Buffer to copy to.
    */
   virtual void copyTileBandToNormalizedBuffer(rspf_uint32 band,
                                               rspf_float32* buf)const;

   /**
    * Copies buf passed in to tile.  Data is unnormalized to the tile's
    * scalar type.
    * The "buf" passed to method is assumed to be at least as big as the tiles:
    * "getSize() * sizeof(double)"
    */
   virtual void copyNormalizedBufferToTile(rspf_float64* buf);
   virtual void copyNormalizedBufferToTile(rspf_float32* buf);

   /**
    * Will copy the normalized buffer to this tiles
    * specified band.  If band is out of range then nothing
    *
    * is done and returns.
    */
   virtual void copyNormalizedBufferToTile(rspf_uint32 band,
                                           rspf_float64* buf);
   virtual void copyNormalizedBufferToTile(rspf_uint32 band,
                                           rspf_float32* buf);

   virtual bool isWithin(rspf_int32 x, rspf_int32 y);
   virtual void setValue(rspf_int32 x, rspf_int32 y, rspf_float64 color);

   virtual void loadBand(const void* src,
                         const rspfIrect& src_rect,
                         rspf_uint32 band);

   virtual void loadBand(const void* src,
                         const rspfIrect& src_rect,
                         const rspfIrect& clip_rect,
                         rspf_uint32 band);
   
   virtual void loadTile(const void* src,
                         const rspfIrect& src_rect,
                         rspfInterleaveType il_type);

   virtual void loadTile(const void* src,
                         const rspfIrect& src_rect,
                         const rspfIrect& clip_rect,                         
                         rspfInterleaveType il_type);

   virtual void nullTileAlpha(const rspf_uint8* src,
                              const rspfIrect& src_rect,
                              bool mutliplyAlphaFlag=false);
   virtual void nullTileAlpha(const rspf_uint8* src,
                              const rspfIrect& src_rect,
                              const rspfIrect& clip_rect,
                              bool multiplyAlphaFlag=false);
   /**
    * Will load a tile of different types.  If they
    * are the same then it will call loadTile(void*...)
    * method.  Note if they are of different types we
    * will have to go through a much slower assign
    * process by normalizing the right side and then
    * unnormalizing to the destination type. For this
    * method to work they both must have the same
    * number of bands.
    */
   virtual void loadTile(const rspfImageData* src);
   
   virtual void loadTileFrom1Band(const rspfImageData* data);

   virtual void loadTileFrom1Band(const void* src,
                                  const rspfIrect& src_rect);
   /**
    * Specialized to load a tile from a short (16 bit scalar type) buffer.
    * Performs byte swapping if swap_bytes is set to true.
    */
   virtual void loadShortBand(const void* src,
                              const rspfIrect& src_rect,
                              rspf_uint32 band,
                              bool swap_bytes=false);

   /**
    * This routine is designed for overwriting a selected band of the 
    * destination buffer 'dest' by an independent selected band of the
    * source buffer according to a user-selected rule, defined by 
    * OverwriteBandRule.
    *
    * Currently the rules available in OverwriteBandRule are: 
    *
    * The COLOR_DISCREPANCY_OF_ANY_FROM_DEST rule directs that the selected 
    * band of a pixel of the destination buffer is to be overwritten if *any* 
    * one of the other bands of the same destination pixel have color values 
    * that deviate from its color. Same as the deprecated COLOR_DISCREPANCY.
    *
    * The COLOR_DISCREPANCY_OF_ALL_FROM_DEST rule directs that the selected 
    * band of a pixel of the destination buffer is to be overwritten only if 
    * *all* of the other bands of the same destination pixel have color values
    * that deviate from its color. 
    *
    * The COLOR_EQUALITY_OF_ANY_TO_SRC rule directs that the selected 
    * band of a pixel of the destination buffer is to be overwritten if 
    * *any* one of the other bands of the same destination pixel have color 
    * values that are equal to the color of the selected band of the source
    * pixel. 
    *
    * The COLOR_EQUALITY_OF_ALL_TO_SRC rule directs that the selected 
    * band of a pixel of the destination buffer is to be overwritten only if 
    * *all* of the other bands of the same destination pixel have color 
    * values that are equal to the color of the selected band of the source
    * pixel. 
    *
    * The NULL_RULE rule directs that the selected band of the 
    * destination buffer is to be overwritten by the selected band of the 
    * source image data (no questions asked).
    *
    * @note: The 'dest' buffer should have at least the same number of bands 
    * as the 'src' object. 
    *
    * Currently this routine is only implemented for il_type set to RSPF_BSQ.
    *
    * @param dest      The destination buffer, which should have at least the 
    * same number of bands as the 'src' object.
    * @param src_band  The 0-based band of the source image data.
    * @param dest_band The 0-based band of the dest buffer.
    * @param dest_rect The rectangle of the destination buffer.
    * @param il_type   The interleave type. Only RSPF_BSQ available.
    * @param ow_type   The rule for overwriting the destination buffer.
    */
   virtual void unloadBand( void* dest,
                            rspf_uint32 src_band,
                            rspf_uint32 dest_band,
                            const rspfIrect& dest_rect,
                            rspfInterleaveType il_type=RSPF_BSQ,
                            OverwriteBandRule ow_type=NULL_RULE ) const;

   /**
    * This routine is designed for overwriting a selected band of the 
    * destination buffer 'dest' by an independent selected band of the
    * source buffer according to a user-selected rule, defined by 
    * OverwriteBandRule.
    *
    * Currently the rules available in OverwriteBandRule are: 
    *
    * The COLOR_DISCREPANCY_OF_ANY_FROM_DEST rule directs that the selected 
    * band of a pixel of the destination buffer is to be overwritten if *any* 
    * one of the other bands of the same destination pixel have color values 
    * that deviate from its color. Same as the deprecated COLOR_DISCREPANCY.
    *
    * The COLOR_DISCREPANCY_OF_ALL_FROM_DEST rule directs that the selected 
    * band of a pixel of the destination buffer is to be overwritten only if 
    * *all* of the other bands of the same destination pixel have color values
    * that deviate from its color. 
    *
    * The COLOR_EQUALITY_OF_ANY_TO_SRC rule directs that the selected 
    * band of a pixel of the destination buffer is to be overwritten if 
    * *any* one of the other bands of the same destination pixel have color 
    * values that are equal to the color of the selected band of the source
    * pixel. 
    *
    * The COLOR_EQUALITY_OF_ALL_TO_SRC rule directs that the selected 
    * band of a pixel of the destination buffer is to be overwritten only if 
    * *all* of the other bands of the same destination pixel have color 
    * values that are equal to the color of the selected band of the source
    * pixel. 
    *
    * The NULL_RULE rule directs that the selected band of the 
    * destination buffer is to be overwritten by the selected band of the 
    * source image data (no questions asked).
    *
    * @note: The 'dest' buffer should have at least the same number of bands 
    * as the 'src' object. 
    *
    * Currently this routine is only implemented for il_type set to RSPF_BSQ.
    *
    * @param dest      The destination buffer, which should have at least the 
    * same number of bands as the 'src' object.
    * @param src_band  The 0-based band of the source image data.
    * @param dest_band The 0-based band of the dest buffer.
    * @param dest_rect The rectangle of the destination buffer.
    * @param clip_rect Only data within will be copied.
    * @param il_type   The interleave type. Only RSPF_BSQ available.
    * @param ow_type   The rule for overwriting the destination buffer.
    */
   virtual void unloadBand( void* dest,
                            rspf_uint32 src_band,
                            rspf_uint32 dest_band,
                            const rspfIrect& dest_rect,
                            const rspfIrect& clip_rect,
                            rspfInterleaveType il_type=RSPF_BSQ,
                            OverwriteBandRule ow_type=NULL_RULE ) const;

   /**
    * @param dest The destination buffer.  It is assumed this is a single band
    * and the size of dest_rect.
    * @param dest_rect The rectangle of the destination buffer.
    * @param src_band The band to copy (unload) from the tile.
    */
   virtual void unloadBand(void* dest,
                           const rspfIrect& dest_rect,
                           rspf_uint32 src_band) const;

   /**
    * @param dest The destination buffer.  It is assumed this is a single band
    * and the size of dest_rect.
    * @param dest_rect The rectangle of the destination buffer.
    * param clip_rect Only data within the clip_rect will be copied.
    * @param src_band The band to copy (unload) from the tile.
    */
   virtual void unloadBand(void* dest,
                           const rspfIrect& dest_rect,
                           const rspfIrect& clip_rect,
                           rspf_uint32 src_band) const;
   
   virtual void unloadTile(void* dest,
                           const rspfIrect& dest_rect,
                           rspfInterleaveType il_type ) const;

   virtual void unloadTile(void* dest,
                           const rspfIrect& dest_rect,
                           const rspfIrect& clip_rect,
                           rspfInterleaveType il_type) const;

   /**
    * @param dest The destination buffer.
    * @param dest_rect The rectangle of the destination buffer.
    * param clip_rect Only data within the clip_rect will be copied.
    * @param src_band The band to copy (unload) from the tile.
    */
   virtual void unloadTileToBipAlpha(void* dest,
                                     const rspfIrect& dest_rect,
                                     const rspfIrect& clip_rect) const;
   
   virtual bool isPointWithin(const rspfIpt& point)const;
   virtual bool isPointWithin(rspf_int32 x,
                              rspf_int32 y)const;

   virtual std::ostream& print(std::ostream& out) const;

   /**
    * @brief Performs linear stretch on tile data from min/max to limits of
    * scalar type.
    */
   virtual void stretchMinMax();

   /**
    * @brief Computes the alpha channel.
    *
    * For each pixel if any band value is not null then alpha will be 255;
    * else, 0.
    */
   virtual void computeAlphaChannel();

   /**
    * @brief Method to copy a single line to the tile.
    *
    * This will copy a line to the tile.  It is assumed that all bands are in
    * the buffer "src".  Only pixels within the tile rect will be copied.
    * Note that all coordinates are relative to the tile coordinates.
    * Clipping will be performed on pixels outside of the tile rectangle.
    * No fill is added if a partial tile line is copied so callers
    * should do a rspfImageData::makeBlank if the whole tile is not to be
    * stuffed.  rspfImageData::validate should be called after all lines
    * are copied if in doubt of a full tile.
    *
    * @param src The source buffer or line to copy containing all bands.
    *
    * @param lineNumber Line number to copy.
    *
    * @param lineStartSample The start sample of the source buffer relative
    * to tile coordinates.
    *
    * @param lineStopSample The stop sample of the source buffer relative
    * to tile coordinates.
    *
    * @param lineInterleave Interleave of src buffer.  If RSPF_BIP assumed
    * rgbrgbrgb; else, red row, green row, blue row.
    *
    * Usage example in method: rspfTiffTileSource::loadFromScanLine
    */
   virtual void copyLine(const void* src,
                         rspf_int32 lineNumber,
                         rspf_int32 lineStartSample,
                         rspf_int32 lineStopSample,
                         rspfInterleaveType lineInterleave);

   /**
    * @brief Sets the indexed flag.
    *
    * This indicates the data contains palette indexes.  Default is false.
    * 
    * @param flag If true data is indexed.
    */
   void setIndexedFlag(bool flag);

   /** @return The indexed flag. */
   bool getIndexedFlag() const;
   
   
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=0)const;
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);
protected:
   /** @brief Templated copy line method. */
   template <class T> void copyLineTemplate(T dummyTemplate,
                                            const void* src,
                                            rspf_int32 lineNumber,
                                            rspf_int32 lineStartSample,
                                            rspf_int32 lineStopSample,
                                            rspfInterleaveType lineInterleave);
   
   /**
    * @brief Templated stretch method.
    *
    * Performs linear stretches on tile data from min/max to limits of
    * scalar type.
    */
   template <class T> void stretchMinMax(T dummyTemplate);

   /**
    * @brief Templated compute alpha channel.
    *
    * For each pixel if any band value is not null then alpha will be 255;
    * else, 0.
    */
   template <class T> void computeAlphaChannel(T dummyTemplate);   

   /**
    * Templated validate method.
    *
    * @return The status of the rspfImageDataMethod.
    *
    * @note See rspfConstants.h for rspfDataObjectStatus enumerations.
    */
   template <class T> rspfDataObjectStatus validate(T dummyTemplate) const;

   /**
    * Templated makeBlank method.
    */
   template <class T> void makeBlank(T dummyTemplate);

   /**
    * Templated computeMeanSquaredError method.
    */
   template <class T>
      rspf_float64 computeMeanSquaredError(T dummyTemplate,
                                            rspf_float64 meanValue,
                                            rspf_uint32 bandNumber) const;

   /**
    * Templated computeAverageBandValue method.
    */
   template <class T>
      rspf_float64 computeAverageBandValue(T dummyTemplate,
                                            rspf_uint32 bandNumber) const;
   
   void loadTileFromBip(const void* src, const rspfIrect& src_rect);
   void loadTileFromBil(const void* src, const rspfIrect& src_rect);
   void loadTileFromBsq(const void* src, const rspfIrect& src_rect);

   void loadTileFromBip(const void* src,
                        const rspfIrect& src_rect,
                        const rspfIrect& clip_rect);
   void loadTileFromBil(const void* src,
                        const rspfIrect& src_rect,
                        const rspfIrect& clip_rect);
   void loadTileFromBsq(const void* src,
                        const rspfIrect& src_rect,
                        const rspfIrect& clip_rect);

   void unloadTileToBip(void* dest,
                        const rspfIrect& dest_rect,
                        const rspfIrect& clip_rect) const;
   void unloadTileToBil(void* dest,
                        const rspfIrect& dest_rect,
                        const rspfIrect& clip_rect) const;
   void unloadTileToBsq(void* dest,
                        const rspfIrect& dest_rect,
                        const rspfIrect& clip_rect) const;

   /**
    * Called from public unloadBand() routines that have an 
    * OverwriteBandRule interface.
    *
    * @param dest      The destination buffer.
    * @param src_band  The 0-based band of the source image data.
    * @param dest_band The 0-based band of the destination buffer.
    * @param dest_rect The rectangle of the destination buffer.
    * @param clip_rect Only data within will be copied.
    * @param ow_type   The rule for overwriting the destination buffer.
    */
   virtual void unloadBandToBsq( void* dest,
                                 rspf_uint32 src_band,
                                 rspf_uint32 dest_band,
                                 const rspfIrect& dest_rect,
                                 const rspfIrect& clip_rect,
                                 OverwriteBandRule ow_type=NULL_RULE ) const;

   /**
    * Copies entire tile to buf passed in.  Data put in buf is normalized.
    * The "buf" passed to method is assumed to be at least as big as:
    * "getSize() * sizeof(double)"
    */
   template <class T>
   void copyTileToNormalizedBuffer(T dummyTemplate,
                                   rspf_float64* buf) const;
   template <class T>
   void copyTileToNormalizedBuffer(T dummyTemplate,
                                   rspf_float32* buf)const;

   /**
    * Will copy this tiles specified band number to the normalized buffer.
    * if the band is out of range then nothing is done and returns.
    */
   template <class T>
   void copyTileToNormalizedBuffer(T dummyTemplate,
                                   rspf_uint32 band, rspf_float64* buf) const;
   template <class T>
   void copyTileToNormalizedBuffer(T dummyTemplate,
                                   rspf_uint32 band, rspf_float32* buf)const;

   /**
    * Copies buf passed in to tile.  Data is unnormalized to the tile's
    * scalar type.
    * The "buf" passed to method is assumed to be at least as big as the tiles:
    * "getSize() * sizeof(double)"
    */
   template <class T>
   void copyNormalizedBufferToTile(T dummyTemplate,
                                   rspf_float64* buf);
   template <class T>
   void copyNormalizedBufferToTile(T dummyTemplate,
                                   rspf_float32* buf);
   template <class T>
   void copyNormalizedBufferToTile(T dummyTemplate,
                                   rspf_uint32 band,
                                   rspf_float64* buf);
   template <class T>
   void copyNormalizedBufferToTile(T dummyTemplate,
                                   rspf_uint32 band,
                                   rspf_float32* buf);

   
   template <class T>
   void computeMinMaxPix(T dummyValue,
                         std::vector<rspf_float64>& minBands,
                         std::vector<rspf_float64>& maxBands) const;

   template <class T>
   void computeMinMaxNulPix(T dummyValue,
                            std::vector<rspf_float64>& minBands,
                            std::vector<rspf_float64>& maxBands,
                            std::vector<rspf_float64>& nulBands) const;
   
   template <class T> void loadBandTemplate(T, // dummy template variable
                                            const void* src,
                                            const rspfIrect& src_rect,
                                            rspf_uint32 band);

   template <class T> void loadBandTemplate(T, // dummy template variable
                                            const void* src,
                                            const rspfIrect& src_rect,
                                            const rspfIrect& clip_rect,
                                            rspf_uint32 band);

   template <class T> void nullTileAlphaTemplate(T,
                                                 const rspf_uint8* src,
                                                 const rspfIrect& src_rect,
                                                 const rspfIrect& clip_rect,
                                                 bool multiplyAlphaFlag=false);
   
   template <class T> void loadTileFromBipTemplate(T, // dummy template variable
                                                   const void* src,
                                                   const rspfIrect& src_rect);

   template <class T> void loadTileFromBipTemplate(T, // dummy template variable
                                                   const void* src,
                                                   const rspfIrect& src_rect,
                                                   const rspfIrect& clip_rect);
   
   template <class T> void loadTileFromBilTemplate(T, // dummy template variable
                                                   const void* src,
                                                   const rspfIrect& src_rect);
   
   template <class T> void loadTileFromBilTemplate(T, // dummy template variable
                                                   const void* src,
                                                   const rspfIrect& src_rect,
                                                   const rspfIrect& clip_rect);
   
   template <class T> void loadTileFromBsqTemplate(T, // dummy template variable
                                                   const void* src,
                                                   const rspfIrect& src_rect);
   
   template <class T> void loadTileFromBsqTemplate(T, // dummy template variable
                                                   const void* src,
                                                   const rspfIrect& src_rect,
                                                   const rspfIrect& clip_rect);
   
   template <class T> void unloadTileToBipTemplate(T, // dummy template variable
                                                   void* dest,
                                                   const rspfIrect& dest_rect,
                                                   const rspfIrect& clip_rect) const;

   template <class T> void unloadTileToBipAlphaTemplate(T, // dummy template variable
                                                        void* dest,
                                                        const rspfIrect& dest_rect,
                                                        const rspfIrect& clip_rect) const;

   template <class T> void unloadTileToBilTemplate(T, // dummy template variable
                                                   void* dest,
                                                   const rspfIrect& dest_rect,
                                                   const rspfIrect& clip_rect) const;

   template <class T> void unloadTileToBsqTemplate(T, // dummy template variable
                                                   void* dest,
                                                   const rspfIrect& dest_rect,
                                                   const rspfIrect& clip_rect) const;

   template <class T> void unloadBandTemplate(T, // dummy template variable
                                              void* dest,
                                              const rspfIrect& dest_rect,
                                              const rspfIrect& clip_rect,
                                              rspf_uint32 band) const;

   template <class T> void unloadBandToBsqTemplate(T, // dummy template variable
                                                   void* dest,
                                                   rspf_uint32 src_band,
                                                   rspf_uint32 dest_band,
                                                   const rspfIrect& dest_rect,
                                                   const rspfIrect& clip_rect,
                                                   OverwriteBandRule ow_type=NULL_RULE) const;

   /**
    * @brief initializeDefaults() Resizes and sets min/max/null arrays to
    * number of bands and some default value for scalar type.
    */
   virtual void initializeDefaults();

   /**
    * @brief initializeMinDefault() Resizes theMinPixelValue array to number
    * of bands and initializes to the default min for scalar type.
    */
   virtual void initializeMinDefault();

   /**
    * @brief initializeMaxDefault() Resizes theMaxPixelValue array to number
    * of bands and initializes to the default max for scalar type.
    */
   virtual void initializeMaxDefault();

   /**
    * @brief initializeNullDefault() Resizes theNullPixelValue array to number
    * of bands and initializes to the default null for scalar type.
    */
   virtual void initializeNullDefault();

   virtual bool isEqualTo(const rspfDataObject& rhs,
                          bool deepTest=false)const;
   
   bool hasSameDimensionsAs(rspfImageData* data)const
   {
      bool result = false;
      if(data)
      {
         result = ((m_spatialExtents.size() ==
                    data->m_spatialExtents.size())&&
                   (m_spatialExtents[0] == data->m_spatialExtents[0])&&
                   (m_spatialExtents[1] == data->m_spatialExtents[1])&&
                   (m_scalarType == data->m_scalarType));
      }
      return result;
   }
   
   /**  Null pixel value for each band. */
   std::vector<rspf_float64> m_nullPixelValue;

   /** Min pixel value for each band. */
   std::vector<rspf_float64> m_minPixelValue;

   /** Max pixel value for each band. */
   std::vector<rspf_float64> m_maxPixelValue;

   /** Alpha channel */
   std::vector<rspf_uint8> m_alpha;
   
   rspfIpt m_origin;

   /** Indicates data contains palette indexes. */
   bool m_indexedFlag;

private:

   
TYPE_DATA
};

inline bool rspfImageData::hasAlpha() const
{
   return (m_alpha.size()?true:false);
}

inline rspfIrect rspfImageData::getImageRectangle() const
{
   rspfIpt lr(m_origin.x + getWidth()  - 1,
               m_origin.y + getHeight() - 1);
   
   return rspfIrect(m_origin, lr);
}

inline const rspf_float64* rspfImageData::getNullPix()const
{
   return ( m_nullPixelValue.size() > 0 ? &m_nullPixelValue.front() : 0 );
}

inline const rspf_float64* rspfImageData::getMinPix()const
{
   return ( m_minPixelValue.size() > 0 ? &m_minPixelValue.front() : 0 );
}

inline const rspf_float64* rspfImageData::getMaxPix()const
{
   return ( m_maxPixelValue.size() > 0 ? &m_maxPixelValue.front() : 0 );
}

inline rspf_float64 rspfImageData::getNullPix(rspf_uint32 band) const
{
   return ( band < m_nullPixelValue.size() ? m_nullPixelValue[band] :
            rspf::defaultNull( getScalarType() ) );
}
      
inline rspf_float64 rspfImageData::getMinPix(rspf_uint32 band) const
{
   return ( band < m_minPixelValue.size() ? m_minPixelValue[band] :
            rspf::defaultMin( getScalarType() ) );
}

inline rspf_float64 rspfImageData::getMaxPix(rspf_uint32 band) const
{
   return ( band < m_maxPixelValue.size() ? m_maxPixelValue[band] :
            rspf::defaultMax( getScalarType() ) );
}

inline rspf_uint32 rspfImageData::getHashId()const
{
   const unsigned char *bufx = (unsigned char*)(&m_origin.x);
   const unsigned char *bufy = (unsigned char*)(&m_origin.y);
   return (rspf_uint32)(bufx[0]*101 + bufx[1]*103 +
                         bufx[2]*107 + bufx[3]*109 +
                         bufy[0]*139 + bufy[1]*149 +
                         bufy[2]*151 + bufy[3]*157);
}

#endif
