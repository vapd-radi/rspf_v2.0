/*
 * Copied from "gdal" project. See licence below.
 *
 * Global functions for gdal code compatibility.
 *
 * Project:  ISO 8211 Access
 * Purpose:  Main declarations for ISO 8211.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 1999, Frank Warmerdam <warmerdam@pobox.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 * $Id&
 */

#include <cstdlib>
#include <cstring>

#include <rspf/base/rspfCplUtil.h>
#include <rspf/base/rspfString.h>

#ifndef RSPF_CSLT_HONOURSTRINGS
#  define RSPF_CSLT_HONOURSTRINGS      0x0001
#endif
#ifndef RSPF_CSLT_ALLOWEMPTYTOKENS
#  define RSPF_CSLT_ALLOWEMPTYTOKENS   0x0002
#endif
#ifndef RSPF_CSLT_PRESERVEQUOTES
#  define RSPF_CSLT_PRESERVEQUOTES     0x0004
#endif
#ifndef RSPF_CSLT_PRESERVEESCAPES
#  define RSPF_CSLT_PRESERVEESCAPES    0x0008
#endif

/************************************************************************/
/*                           rspfCSLFindString()                            */
/*                                                                      */
/*      Find a string within a string list.  The string must match      */
/*      the full length, but the comparison is case insensitive.        */
/*      Return -1 on failure.                                           */
/************************************************************************/

int rspfCSLFindString( char ** papszList, const char * pszTarget )

{
    int         i;

    if( papszList == NULL )
        return -1;

    for( i = 0; papszList[i] != NULL; i++ )
    {
       rspfString s = papszList[i];
       if( s == pszTarget)
            return i;
    }

    return -1;
}

/**********************************************************************
 *                       rspfCSLTokenizeString()
 *
 * Tokenizes a string and returns a StringList with one string for
 * each token.
 **********************************************************************/

char    **rspfCSLTokenizeString( const char *pszString )
{
    return rspfCSLTokenizeString2( pszString, " ", RSPF_CSLT_HONOURSTRINGS );
}

/************************************************************************/
/*                      rspfCSLTokenizeStringComplex()                      */
/*                                                                      */
/*      Obsolete tokenizing api.                                        */
/************************************************************************/

char ** rspfCSLTokenizeStringComplex( const char * pszString,
                                       const char * pszDelimiters,
                                       int bHonourStrings,
                                       int bAllowEmptyTokens )

{
    int         nFlags = 0;

    if( bHonourStrings )
        nFlags |= RSPF_CSLT_HONOURSTRINGS;
    if( bAllowEmptyTokens )
        nFlags |= RSPF_CSLT_ALLOWEMPTYTOKENS;

    return rspfCSLTokenizeString2( pszString, pszDelimiters, nFlags );
}

/************************************************************************/
/*                         rspfCSLTokenizeString2()                         */
/*                                                                      */
/*      The ultimate tokenizer?                                         */
/************************************************************************/

char ** rspfCSLTokenizeString2( const char * pszString,
                                 const char * pszDelimiters,
                                 int nCSLTFlags )

{
    char        **papszRetList = NULL;
    int         nRetMax = 0, nRetLen = 0;
    char        *pszToken;
    int         nTokenMax, nTokenLen;
    int         bHonourStrings = (nCSLTFlags & RSPF_CSLT_HONOURSTRINGS);
    int         bAllowEmptyTokens = (nCSLTFlags & RSPF_CSLT_ALLOWEMPTYTOKENS);

    pszToken = (char *) calloc(10,1);
    nTokenMax = 10;
    
    while( pszString != NULL && *pszString != '\0' )
    {
        int     bInString = false;

        nTokenLen = 0;
        
        /* Try to find the next delimeter, marking end of token */
        for( ; *pszString != '\0'; pszString++ )
        {

            /* End if this is a delimeter skip it and break. */
            if( !bInString && strchr(pszDelimiters, *pszString) != NULL )
            {
                pszString++;
                break;
            }
            
            /* If this is a quote, and we are honouring constant
               strings, then process the constant strings, with out delim
               but don't copy over the quotes */
            if( bHonourStrings && *pszString == '"' )
            {
                if( nCSLTFlags & RSPF_CSLT_PRESERVEQUOTES )
                {
                    pszToken[nTokenLen] = *pszString;
                    nTokenLen++;
                }

                if( bInString )
                {
                    bInString = false;
                    continue;
                }
                else
                {
                    bInString = true;
                    continue;
                }
            }

            /* Within string constants we allow for escaped quotes, but
               in processing them we will unescape the quotes */
            if( bInString && pszString[0] == '\\' && pszString[1] == '"' )
            {
                if( nCSLTFlags & RSPF_CSLT_PRESERVEESCAPES )
                {
                    pszToken[nTokenLen] = *pszString;
                    nTokenLen++;
                }

                pszString++;
            }

            /* Within string constants a \\ sequence reduces to \ */
            else if( bInString 
                     && pszString[0] == '\\' && pszString[1] == '\\' )
            {
                if( nCSLTFlags & RSPF_CSLT_PRESERVEESCAPES )
                {
                    pszToken[nTokenLen] = *pszString;
                    nTokenLen++;
                }
                pszString++;
            }

            if( nTokenLen >= nTokenMax-3 )
            {
                nTokenMax = nTokenMax * 2 + 10;
                pszToken = (char *) rspfCPLRealloc( pszToken, nTokenMax );
            }

            pszToken[nTokenLen] = *pszString;
            nTokenLen++;
        }

        pszToken[nTokenLen] = '\0';

        /*
         * If the last token is an empty token, then we have to catch
         * it now, otherwise we won't reenter the loop and it will be lost. 
         */

        if( (pszToken[0] != '\0' || bAllowEmptyTokens)
            || (*pszString == '\0' && bAllowEmptyTokens
                && strchr(pszDelimiters, *(pszString-1)) ) )
        {
            if( nRetLen >= nRetMax - 1 )
            {
                nRetMax = nRetMax * 2 + 10;
                papszRetList = (char **) 
                    rspfCPLRealloc(papszRetList, sizeof(char*) * nRetMax );
            }

            papszRetList[nRetLen++] = strdup( pszToken );
            papszRetList[nRetLen] = NULL;
        }
    }

    if( papszRetList == NULL )
        papszRetList = (char **) calloc(sizeof(char *),1);

    free( pszToken );

    return papszRetList;
}

/**********************************************************************
 *                       rspfCSLCount()
 *
 * Return the number of lines in a Stringlist.
 **********************************************************************/
int rspfCSLCount(char **papszStrList)
{
    int nItems=0;

    if (papszStrList)
    {
        while(*papszStrList != NULL)
        {
            nItems++;
            papszStrList++;
        }
    }

    return nItems;
}

/**********************************************************************
 *                       rspfCSLDestroy()
 *
 * Free all memory used by a StringList.
 **********************************************************************/
void rspfCSLDestroy(char **papszStrList)
{
    char **papszPtr;

    if (papszStrList)
    {
        papszPtr = papszStrList;
        while(*papszPtr != NULL)
        {
            free(*papszPtr);
            papszPtr++;
        }

        free(papszStrList);
    }
}

void * rspfCPLRealloc( void * pData, size_t nNewSize )

{
    void        *pReturn;

    if ( nNewSize == 0 )
    {
        free(pData);
        return NULL;
    }

    if( pData == NULL )
        pReturn = malloc( nNewSize );
    else
        pReturn = realloc( pData, nNewSize );

    return pReturn;
}
