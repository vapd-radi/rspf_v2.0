//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: SPADAC Inc
//         Adaptd from the GDAL package hfa dataset
// Description: This class provides some simple utilities for aux file.
//
//********************************************************************
// $Id: rspfAuxFileHandler.h 2644 2011-05-26 15:20:11Z oscar.kramer $

#ifndef rspfAuxFileHandler_HEADER
#define rspfAuxFileHandler_HEADER

#include <cstring>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfString.h>

#define EPT_u1	0
#define EPT_u2	1
#define EPT_u4	2
#define EPT_u8	3
#define EPT_s8	4
#define EPT_u16	5
#define EPT_s16	6
#define EPT_u32	7
#define EPT_s32	8
#define EPT_f32	9
#define EPT_f64	10
#define EPT_c64	11
#define EPT_c128 12

class rspfFilename;
class rspfAuxEntry;
class rspfAuxType;
class rspfAuxDictionary;

class rspfAuxInfo{

public:
  rspfAuxInfo();

  virtual ~rspfAuxInfo();   

  FILE *fp;

  rspf_uint32  nEndOfFile;
  rspf_uint32	nRootPos;
  rspf_uint32	nDictionaryPos;

  rspf_int16	nEntryHeaderLength;
  rspf_int32	nVersion;

  rspfAuxEntry* m_Root;

  rspfAuxDictionary*  m_Dictionary;
  char* dictionaryData;
};


class rspfAuxEntry
{
  rspf_uint32	nFilePos;

  rspfAuxInfo*   m_auxInfo;
  rspfAuxEntry*  m_Parent;
  rspfAuxEntry*  m_Prev;
  rspfAuxEntry*  m_Next;
  rspfAuxEntry*  m_Child;

  rspf_uint32	  nNextPos;
  rspf_uint32	  nChildPos;
  
  char	entryNodeName[64];
  char	entryType[32];

  rspfAuxType* m_Type;
  rspf_uint32	nDataPos;
  rspf_uint32	nDataSize;
  char*  entryData;

  void	     loadData();

  int 	     getFieldValue(const char* auxFieldPath,
                           char chReqType, void *reqReturn);

public:
  rspfAuxEntry( rspfAuxInfo* auxInfoIn, rspf_uint32 nPos,
    rspfAuxEntry* parentIn, rspfAuxEntry* prevIn);

  virtual     ~rspfAuxEntry();                

  rspf_uint32	getFilePos() { return nFilePos; }

  const char*   getName() { return entryNodeName; }
  void          setName( const char *nodeName );

  const char*   getType() { return entryType; }

  char*  getData() { loadData(); return entryData; }
  rspf_uint32	getDataPos() { return nDataPos; }
  rspf_uint32	getDataSize() { return nDataSize; }

  rspfAuxEntry*     getChild();
  rspfAuxEntry*     getNext();
  rspfAuxEntry*     getNamedChild( const char * name);

  rspf_int16	  getIntField( const char * auxFieldPath);
  const char*   getStringField( const char * auxFieldPath);

};

class rspfAuxField
{
public:
  int		nBytes;

  int		nItemCount;
  char	chPointer; 	/* '\0', '*' or 'p' */
  char	chItemType;	/* 1|2|4|e|... */

  char	*itemObjectType; /* if chItemType == 'o' */
  rspfAuxType* m_auxItemObjectType;

  char** enumNames; /* normally NULL if not an enum */

  char	*fieldName;

  rspfAuxField();
  ~rspfAuxField();

  int 	extractInstValue( const char * auxField, int nIndexValue,
    char *entryData, rspf_uint32 nDataOffset, int nDataSize,
    char chReqType, void *reqReturn );

  int		getInstCount( char * entryData, int nDataSize );

  int   getInstBytes( char *entryData, int nDataSize );

  const char* initialize( const char * fieldInput );

  void completeDefn( rspfAuxDictionary * auxDict );
};


class rspfAuxType
{
public:
  int		nBytes;
  int		nFields;

  rspfAuxField** m_auxFields;

  char* auxTypeName;

  rspfAuxType();
  ~rspfAuxType();

  const char* initialize( const char * typeInput);

  void	completeDefn( rspfAuxDictionary * auxDict);

  int		getInstBytes(char *entryData, int nDataSize );

  int   extractInstValue( const char * auxField,
    char *entryData, rspf_uint32 nDataOffset, int nDataSize,
    char chReqType, void *reqReturn );
};

class rspfAuxDictionary
{
public:
  int		nTypes;
  int   nTypesMax;
  rspfAuxType**  m_auxTypes;

  rspfAuxDictionary(const char *auxDictStr);
  ~rspfAuxDictionary();

  rspfAuxType* findType( const char * name);
  void          addType( rspfAuxType * type);

  static int	  getItemSize( char chType);
};


class RSPF_DLL rspfAuxFileHandler
{
public:


  rspfAuxFileHandler();
  virtual ~rspfAuxFileHandler();
  /**
  * @read .aux file
  */
  bool isAuxFile(const rspfFilename& file);

  /**
  * @open the file for reading
  */
  bool open(const rspfFilename& file);

  /**
  *  Reads a single record from a aux file.
  *  Returns true if succesful. Returns false if EOF or error.
  */
  const rspfString& getProjectionName() const { return m_projName; }

  const rspfString& getDatumName() const { return m_datumName; }

  const rspfString& getUnitType() const { return m_unitsType; }

private:

  rspfAuxInfo* m_auxInfo;
  rspfString m_projName;
  rspfString m_datumName;
  rspfString m_unitsType;
};

#endif  // #ifndef rspfAuxFileHandlerl_HEADER
