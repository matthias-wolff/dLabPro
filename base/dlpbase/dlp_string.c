/* dLabPro base library
 * - Protected <string.h> functions, plus extensions
 *
 * AUTHOR : Matthias Wolff
 * PACKAGE: dLabPro/base
 * 
 * Copyright 2013 dLabPro contributors and others (see COPYRIGHT file) 
 * - Chair of System Theory and Speech Technology, TU Dresden
 * - Chair of Communications Engineering, BTU Cottbus
 * 
 * This file is part of dLabPro.
 * 
 * dLabPro is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 * 
 * dLabPro is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with dLabPro. If not, see <http://www.gnu.org/licenses/>.
 */

#include "dlp_kernel.h"
#include "dlp_base.h"

/* Static variables */
static char __a_buffer[4096];

/**
 * Get a pointer to a static 4096 bytes char buffer. This buffer is
 * allocated by the kernel. The pointer may be stored for later use.
 * However, there is no guarantee that the content of the buffer has
 * not changed.
 *
 * @return A pointer to a static 4096 byte character buffer
 */
char* dlp_get_a_buffer()
{
  return __a_buffer;
}

/**
 * Move one buffer to another. Protected implementation of the
 * memmove function.
 *
 * Fixes a bug (right shift with overlapping buffers) for the
 * SUN Solaris OS.
 *
 * @param lpDst  Destination pointer
 * @param lpSrc  Source pointer
 * @param nCount Number of bytes to copy
 * @return The value of dest or NULL if an error occured
 */
void* dlp_memmove(void* lpDst, const void* lpSrc, size_t nCount)
{
  if (lpDst == NULL || lpSrc == NULL || nCount<1) return NULL;

#ifndef __SPARC

  memmove(lpDst,lpSrc,nCount);

#else
  /* WORK AROUND: memmove doesn't seem to do shifting to the right properly */
  /*              on sparc machines.                                        */

  if (lpDst <= lpSrc || (BYTE*)lpDst >= ((BYTE*)lpSrc + count))
  {
    /* Non-Overlapping Buffers                       */
    /* copy from lower addresses to higher addresses */
    while (nCount--)
    {
      *(BYTE*)lpDst = *(BYTE*)lpSrc;
      lpDst = (BYTE*)lpDst+1;
      lpSrc = (BYTE*)lpSrc+1;
    }
  }
  else
  {
    /* Overlapping Buffers                           */
    /* copy from higher addresses to lower addresses */
    lpDst = (BYTE*)lpDst+nCount-1;
    lpSrc = (BYTE*)lpSrc+nCount-1;
    while (nCount--)
    {
      *(BYTE*)lpDst = *(BYTE*)lpSrc;
      lpDst = (BYTE*)lpDst-1;
      lpSrc = (BYTE*)lpSrc-1;
    }
  }
#endif

  return lpDst;
}

/**
 * Sets a buffer to a specified character. Protected implementation
 * of the memset function.
 *
 * @param lpDst  Destination object
 * @param nVal   Character to set
 * @param nCount Number of characters
 * @return The value of dest or NULL if an error occured
 */
void* dlp_memset(void* lpDst, INT32 nVal, size_t nCount)
{
  if (lpDst == NULL || nCount< 1) return NULL;
  memset(lpDst,nVal,nCount);
  return lpDst;
}

/**
 * Determines if a character is contained in a given character set.
 *
 * @param nChr The character
 * @param sSet Pointer to null-terminated string containing the character set
 * @return <code>TRUE</code> if <code>nChr</code> is element of
 *         <code>sSet</code>, <code>FALSE</code> otherwise
 */
BOOL dlp_charin(char nChr, const char* sSet)
{
  for (; *sSet; sSet++)
    if (nChr==*sSet) return TRUE;
  return FALSE;
}

/**
 * Get the length of a string. Protected implementation of the
 * strlen function.
 *
 * @param lpsStr Null-terminated string
 * @return The length of the string in characters excluding the
 *         terminal NULL
 */
size_t dlp_strlen(const char* lpsStr)
{
  if (lpsStr == NULL) return 0;
  return strlen(lpsStr);
}

/**
 * Counts the number of occurrances of a character in a string.
 *
 * @param lpsStr Null-terminated string
 * @param nChr   The character to count
 * @return The number of occurrances of <code>nChr</code> in <code>lpsStr</code>
 */
INT32 dlp_strcnt(const char* lpsStr, const char nChr)
{
  INT32 nCnt = 0;
  if (lpsStr == NULL) return 0;
  for (; *lpsStr; lpsStr++)
    if (*lpsStr==nChr) nCnt++;
  return nCnt;
}

/**
 * Copies a string. Protected implementation of the strcpy function.
 *
 * @param lpsDst Destination string
 * @param lpsSrc Source string
 * @return The destination string or NULL if an error occurs.
 */
char* dlp_strcpy(char* lpsDst, const char *lpsSrc)
{
  if (lpsDst == NULL) return NULL;
  if (lpsSrc == NULL) { lpsDst[0]='\0'; return lpsDst; }
  return strcpy(lpsDst,lpsSrc);
}

/**
 * Copies characters of one string to another. Protected
 * implementation of the strncpy function.
 *
 * @param lpsDst Destination string
 * @param lpsSrc Source string
 * @param nCount Number of characters to be copied
 * @return The destination string or NULL of an error occurs
 * @see dlp_strabbrv
 */
char* dlp_strncpy(char* lpsDst, const char* lpsSrc, size_t nCount)
{
  if (lpsDst == NULL) return NULL;
  if (lpsSrc == NULL) lpsDst[0] = '\0';
  else strncpy(lpsDst,lpsSrc,MIN(strlen(lpsSrc)+1,nCount));
  return lpsDst;
}

/**
 * This function is to abbreviate a string for display. If the source string is
 * longer than <code>nCount</code> characters, the last three characters of the
 * destination string will be set to "...". The destination string will be
 * zero-terminated.
 *
 * @param lpsDst
 *          Destination string
 * @param lpsSrc
 *          Source string (may be identical with <code>lpsDst</code>
 * @param nCount
 *          Number of characters to be copied. <code>lpsDst</code> must be
 *          capable of holding <code>nCount+1</code> characters.
 * @return The destination string or NULL of an error occurs
 * @see dlp_strncpy
 */
char* dlp_strabbrv(char* lpsDst, const char* lpsSrc, size_t nCount)
{
  if (lpsDst!=lpsSrc)
  {
    if (dlp_strlen(lpsSrc)>nCount)
    {
      dlp_strncpy(lpsDst,lpsSrc,nCount-3);
      dlp_strcat(lpsDst,"...");
    }
    else dlp_strncpy(lpsDst,lpsSrc,nCount);
  }
  else if (dlp_strlen(lpsSrc)>nCount)
  {
    lpsDst[nCount-3]='.';
    lpsDst[nCount-2]='.';
    lpsDst[nCount-1]='.';
  }
  lpsDst[nCount]='\0';
  return lpsDst;
}

/**
 * Compares two strings. Protected implementation of the strcmp
 * function.
 *
 * @param lpsStr1 String to compare
 * @param lpsStr2 String to compare
 * @return <0: lpsStr1 less than lpsStr2
 *          0: lpsStr1 identical to lpsStr2
 *         >0: lpsStr1 greater than lpsStr2
 */
INT32 dlp_strcmp(const char* lpsStr1, const char* lpsStr2)
{
  if (lpsStr1 == NULL && lpsStr2 == NULL) return 0;
  if (lpsStr1 == NULL) return -1;
  if (lpsStr2 == NULL) return 1;
  return strcmp(lpsStr1,lpsStr2);
}

/**
 * Compares two strings regardless of case. Protected implementation
 * of the stricmp function.
 *
 * @param lpsStr1 String to compare
 * @param lpsStr2 String to compare
 * @return <0: lpsStr1 less than lpsStr2
 *          0: lpsStr1 identical to lpsStr2
 *         >0: lpsStr1 greater than lpsStr2
 */
INT32 dlp_stricmp(const char* lpsStr1, const char* lpsStr2)
{
  if (lpsStr1 == NULL && lpsStr2 == NULL) return 0;
  if (lpsStr1 == NULL) return -1;
  if (lpsStr2 == NULL) return 1;
#ifndef __TMS
  return stricmp(lpsStr1,lpsStr2);
#else
  {INT32 nI;
    for(nI=0;lpsStr1[nI] && lpsStr2[nI];nI++)
    {
      char nC1=lpsStr1[nI];
      char nC2=lpsStr2[nI];
      if(nC1>='A' && nC1<='Z') nC1+='a'-'A';
      if(nC2>='A' && nC2<='Z') nC2+='a'-'A';
      if(nC1!=nC2) return nC1-nC2;
    }
    return lpsStr1[nI]-lpsStr2[nI];
  }
#endif
}

/**
 * Compares two strings regardless of case. Protected implementation
 * of the strnicmp function.
 *
 *
 * @param lpsStr1 String to compare
 * @param lpsStr2 String to compare
 * @param count   Number of characters to compare
 * @return <0: lpsStr1 substring less than lpsStr2 substring
 *          0: lpsStr1 substring identical to lpsStr2 substring
 *         >0: lpsStr1 substring greater than lpsStr2 substring
 */
INT32 dlp_strnicmp(const char* lpsStr1, const char* lpsStr2, size_t nCount)
{
  if (lpsStr1 == NULL && lpsStr2 == NULL) return 0;
  if (lpsStr1 == NULL) return -1;
  if (lpsStr2 == NULL) return 1;

#ifndef __TMS
  return strnicmp(lpsStr1,lpsStr2,nCount);
#else
  {INT32 nI;
    for(nI=0;(nI<nCount) && lpsStr1[nI] && lpsStr2[nI];nI++)
    {
      char nC1=lpsStr1[nI];
      char nC2=lpsStr2[nI];
      if(nC1>='A' && nC1<='Z') nC1+='a'-'A';
      if(nC2>='A' && nC2<='Z') nC2+='a'-'A';
      if(nC1!=nC2) return nC1-nC2;
    }
    return lpsStr1[nI]-lpsStr2[nI];
  }
#endif
}

/**
 * Compares two path names.
 *
 * @param lpsStr1 String to compare
 * @param lpsStr2 String to compare
 * @return <0: lpsStr1 less than lpsStr2
 *          0: lpsStr1 identical to lpsStr2
 *         >0: lpsStr1 greater than lpsStr2
 */
INT32 dlp_strpcmp(const char* lpsStr1, const char* lpsStr2)
{
#ifdef _WINDOWS                                                                 /* -- WINDOZE -->                    */
	return dlp_stricmp(lpsStr1,lpsStr2);                                          /* Compare case independent          */
#else                                                                           /* <-- NOT WINDOZE -->               */
	return dlp_strcmp(lpsStr1,lpsStr2);                                           /* Compare case dependent            */
#endif                                                                          /* <--                               */
}

/**
 * Compares characters of two strings. Protected implementation of
 * the strncmp function.
 *
 * @param lpsStr1 String to compare
 * @param lpsStr2 String to compare
 * @param count   Number of characters to compare
 * @return <0: lpsStr1 substring less than lpsStr2 substring
 *          0: lpsStr1 substring identical to lpsStr2 substring
 *         >0: lpsStr1 substring greater than lpsStr2 substring
 */
INT32 dlp_strncmp(const char* lpsStr1, const char* lpsStr2, size_t nCount)
{
  if (lpsStr1 == NULL && lpsStr2 == NULL) return 0;
  if (lpsStr1 == NULL) return -1;
  if (lpsStr2 == NULL) return 1;
  if (nCount==0) return 0;
  return strncmp(lpsStr1,lpsStr2,nCount);
}

/**
 * Appends one string to another. Protected implementation of the
 * strcat function.
 *
 * @param lpsDst Destination string
 * @param lpsSrc Source string
 * @return The destination string
 */
char* dlp_strcat(char* lpsDst, const char* lpsSrc)
{
  if (lpsDst == NULL) return NULL;
  if (lpsSrc == NULL) return lpsDst;
  return strcat(lpsDst,lpsSrc);
}

/**
 * Converts a string to lower case.
 *
 * @param lpsStr The string to convert
 * @return The pointer to the string
 */
char* dlp_strlwr(char* lpsStr)
{
  char* p = NULL;

  if (lpsStr == NULL) return NULL;
  for (p=lpsStr; *p; p++) CHARLWR(*p);
  return lpsStr;
}

/**
 * Converts a string to upper case.
 *
 * @param lpsStr The string to convert
 * @return The pointer to the string
 */
char* dlp_strupr(char* lpsStr)
{
  char* p = NULL;

  if (lpsStr == NULL) return NULL;
  for (p=lpsStr; *p; p++) CHARUPR(*p);
  return lpsStr;
}

/**
 * Puts a string in quotation marks. If one or both quotation marks
 * are already present, no additional quotation marks will be inserted.
 *
 * @param lpsStr The string to convert
 * @param nLeft  The left quotation mark character
 * @param nRight The left quotation mark character
 * @return The pointer to the string
 */
char* dlp_strquotate(char* lpsStr, char nLeft, char nRight)
{
  char* lpsBuf;

  if (lpsStr == NULL) return NULL;

  lpsBuf = (char*)calloc(dlp_strlen(lpsStr)+3,sizeof(char));

  if (lpsStr[0] != nLeft) sprintf(lpsBuf,"%c%s",nLeft,lpsStr);
  else sprintf(lpsBuf,"%s",lpsStr);
  if (lpsBuf[strlen(lpsBuf) -1]!=nRight || strlen(lpsBuf)==1)
  {
    lpsBuf[strlen(lpsBuf)+1]='\0';
    lpsBuf[strlen(lpsBuf)  ]=nRight;
  }
  dlp_strcpy(lpsStr,lpsBuf);

  free(lpsBuf);

  return lpsStr;
}

/**
 * Trims leading and trailing quotation marks from a string.
 *
 * @param lpsStr The string to convert
 * @param nLeft  The left quotation mark character
 * @param nRight The left quotation mark character
 * @return The pointer to the string
 */
char* dlp_strunquotate(char* lpsStr, char nLeft, char nRight)
{
  if (dlp_strlen(lpsStr) == 0) return lpsStr;

  if (lpsStr[0] == nLeft) dlp_memmove(lpsStr,&lpsStr[1],dlp_strlen(lpsStr));
  if (lpsStr[dlp_strlen(lpsStr)-1] == nRight) lpsStr[dlp_strlen(lpsStr)-1] = '\0';

  return lpsStr;
}

/**
 * Replaces escape sequences in a string by character codes.
 *
 * @param lpsStr The string to process
 * @return The pointer to the string
 * /
char* dlp_strunquote(char* lpsStr)
{
  size_t i;

  for (i=0; i<dlp_strlen(lpsStr); i++) if (lpsStr[i] == '\\')
  {
    if (lpsStr[i+1] == 'n')
    {
      lpsStr[i+1] = 10;
      dlp_memmove((void*)&lpsStr[i],(void*)&lpsStr[i+1],dlp_strlen(lpsStr)-i);
    }
    if (lpsStr[i+1] == 't')
    {
      lpsStr[i+1] = 9;
      dlp_memmove((void*)&lpsStr[i],(void*)&lpsStr[i+1],dlp_strlen(lpsStr)-i);
    }
  }

  return lpsStr;
}
*/

/**
 * Trims leading white spaces from a string.
 *
 * @param lpsStr The string to process
 * @return The pointer to the string
 */
char* dlp_strtrimleft(char* lpsStr)
{
  INT32 l = (INT32)dlp_strlen(lpsStr);

  if (lpsStr == NULL) return NULL;
  if (0      == l   ) return lpsStr;

  while (l && iswspace(*lpsStr)) memmove(lpsStr,&lpsStr[1],l--);

  return lpsStr;
}

/**
 * Trims trailing white spaces from a string.
 *
 * @param lpsStr The string to process
 * @return The pointer to the string
 */
char* dlp_strtrimright(char* lpsStr)
{
  INT32 l = (INT32)dlp_strlen(lpsStr);

  if (lpsStr == NULL) return NULL;
  if (0      == l   ) return lpsStr;

  while (l && iswspace(lpsStr[l-1])) lpsStr[(l--)-1] = '\0';

  return lpsStr;
}

/**
 * Splits a fully qualified file name in path and simple file name.
 *
 * @param lpsFQP
 *          Fully qualified path to be splitted
 * @param lpsPath
 *          Pointer to a buffer to be filled with the path (is expected to be
 *          at least <code>L_PATH</code> bytes long, may be <code>NULL</code>)
 * @param lpsFile
 *          Pointer to a buffer to be filled with the file name (is expected to
 *          be at least <code>L_PATH</code> bytes long, may be
 *          <code>NULL</code>)
 */
void dlp_splitpath(const char* lpsFQP, char* lpsPath, char* lpsFile)
{
  char  lpsBuf[L_PATH+1];
  char* p;

  if (!lpsFQP || !dlp_strlen(lpsFQP)) return;
  strcpy(lpsBuf,lpsFQP);

  /* Find last path delimiter */
  p = &lpsBuf[dlp_strlen(lpsBuf)-1];
  while (*p != '/' && *p != '\\' && p != lpsBuf) p--;

  /* Do the splitting */
  if (*p == '/' || *p == '\\')
  {
    *p++ = 0;
    dlp_strcpy(lpsPath,lpsBuf);
    dlp_strcpy(lpsFile,p);
  }
  else
  {
    dlp_strcpy(lpsPath,"");
    dlp_strcpy(lpsFile,lpsBuf);
  }
}

/**
 * Duplicates a given string.
 * WARNING: This function allocates memory for the new string!
 *
 * @param lpsStr The string to duplicate
 * @return       The pointer to the copy
 */
char* dlp_strduplicate(const char* lpsStr)
{
  size_t nLen   = 0;
  char*  lpsDst = NULL;

  nLen   = dlp_strlen(lpsStr)+1;
  lpsDst = (char*)dlp_malloc(nLen);
  if (lpsDst)
  {
    memset(lpsDst,0,nLen);
    memmove(lpsDst,lpsStr,nLen);
  }
  return lpsDst;
}

/**
 * Get hash code from a string. This function is optimized for dLabPro
 * identifiers.
 *
 * @param lpKey The string to hash
 * @return      The hash code
 */
hash_val_t dlp_strhash(const void* lpKey)
{
  hash_val_t nHash = 0;
  char*  p     = (char*)lpKey;
  for ( ; p; ++p) nHash = 5*nHash + *p;
  return nHash;
}

/**
 * Converts a string
 *
 * @param nHow    The conversion type, one of the SC_XXX or CN_XXX constants
 * @param lpsStr1 The destination string
 * @param lpsStr2 The source string (may be equal to lpsStr1)
 * @return        The destination string, no error report
 */
char* dlp_strconvert(INT16 nHow, char* lpsStr1, const char* lpsStr2)
{
  char         lpsSbuf[L_INPUTLINE+1];
  char         lpsBuf[255];
  char*        tx                     = NULL;
  char*        ty                     = NULL;
  unsigned int c                      = 0;
  INT32      nCtr                   = 0;

  dlp_strcpy(lpsSbuf,lpsStr2);
  if (lpsStr1) lpsStr1[0]='\0';

  switch (nHow)
  {
    case SC_URL_ESCAPE:
      tx = lpsSbuf;
      ty = lpsStr1;
      while (*tx)
      {
        if((*tx>=0x00 && *tx<=0x1F) ||
            *tx>=0x7F               ||  /* GCC says this is always true due to limited range of data type !? */
            *tx=='<'                ||
            *tx=='>'                ||
            *tx=='\"'               ||
            *tx=='#'                ||
            *tx=='{'                ||
            *tx=='}'                ||
            *tx=='|'                ||
            *tx=='\\'               ||
            *tx=='^'                ||
            *tx=='~'                ||
            *tx=='['                ||
            *tx==']'                ||
            *tx=='\''                )
        {
          sprintf(lpsBuf,"%%%02X",(unsigned int)*tx);
          *ty='\0';
          strcat(lpsStr1,lpsBuf);
          ty+=3;
        }
        else *ty++=*tx;
        tx++;
      }
      *ty='\0';
      return lpsStr1;
    case SC_URL_UNESCAPE:
      tx = lpsSbuf;
      ty = lpsStr1;
      while (*tx)
      {
        if (*tx=='%')
        {
          tx++;
          lpsBuf[0] = *tx++;
          lpsBuf[1] = *tx;
          lpsBuf[2] = '\0';
          sscanf(lpsBuf,"%02X",(unsigned int*)&c);
          *ty++ = (char)c;
        }
        else *ty++=*tx;
        tx++;
      }
      *ty='\0';
      return lpsStr1;
    case SC_PRC_ESCAPE:
      tx = lpsSbuf;
      ty = lpsStr1;
      while (*tx)
      {
        *ty=*tx;
        if (*tx=='%')
        {
          ty++;
          *ty='%';
        }
        tx++;ty++;
        *ty='\0';
      }
      return lpsStr1;
    case SC_ESCAPE:
      tx = lpsSbuf;
      ty = lpsStr1;
      while (*tx)
      {
        switch (*tx)
        {
        case '\t': *ty++='\\'; *ty='t' ; break;
        case '\r': *ty++='\\'; *ty='r' ; break;
        case '\n': *ty++='\\'; *ty='n' ; break;
        case '\b': *ty++='\\'; *ty='b' ; break;
        case '\"': *ty++='\\'; *ty='\"'; break;
        case '\'': *ty++='\\'; *ty='\''; break;
        case '\\': *ty++='\\'; *ty='\\'; break;
        default : *ty=*tx;
        }
        tx++;*++ty='\0';
      }
      return lpsStr1;
    case SC_UNESCAPE:       /* FALL THROUGH */
    case SC_UNESCAPE_ITP24:
      tx = lpsSbuf;
      ty = lpsStr1;
      while (*tx)
      {
        if (*tx!='\\') *ty=*tx;
        else switch (*++tx)
        {
        case 't' : *ty='\t'; break;
        case 'r' : *ty='\r'; break;
        case 'n' : *ty='\n'; break;
        case 'b' : *ty='\b'; break;
        case '\"': *ty='\"'; break;
        case '\'': *ty='\''; break;
        case '0' : *ty='\0'; break;
        case '\\': if (nHow!=SC_UNESCAPE_ITP24) { *ty='\\'; break; }            /* else: FALL THROUGH                */
        default  : *ty++='\\'; *ty=*tx;
        }
        tx++;*++ty='\0';
      }
      return lpsStr1;
    case SC_STRIPHTML:
      tx   = lpsSbuf;
      ty   = lpsStr1;
      nCtr = 0;
      while (*tx)
      {
        if      (*tx=='<') nCtr++;
        else if (*tx=='>') { nCtr--; if (nCtr<0) nCtr=0; }
        else if (!nCtr) { *ty++=*tx; *ty='\0'; }
        tx++;
      }
      return lpsStr1;
    break;
    default: return dlp_convert_name(nHow,lpsStr1,lpsStr2);
  }
}

/**
 * Replaces occurrances of a string by another string.
 *
 * @param lpsStr The string to be processed
 * @param lpsKey The string to replace
 * @param lpsRpl The string to replace lpKey with
 * @param bOnce  If TRUE: replace first occurrence of lpKey only
 * @return The number of occurrences of lpKey replaced by lpReplace
 * @see dlp_strreplace
 */
INT16 dlp_strreplace_ex(char* lpsStr, const char* lpsKey, const char* lpsRpl, BOOL bOnce)
{
  INT16 nRpl = 0;
  char* tx   = NULL;
  if (dlp_strlen(lpsStr)==0) return 0;
  if (dlp_strlen(lpsKey)==0) return 0;

  tx = strstr(lpsStr,lpsKey);
  while (tx != NULL)
  {
    nRpl++;
    memmove(tx,&tx[dlp_strlen(lpsKey)],dlp_strlen(tx)-dlp_strlen(lpsKey)+1);
    memmove(&tx[dlp_strlen(lpsRpl)],tx,dlp_strlen(tx)+1);
    if(lpsRpl) memmove(tx,lpsRpl,dlp_strlen(lpsRpl));
    tx+=(dlp_strlen(lpsRpl));
    tx = strstr(tx,lpsKey);
    if (bOnce) return 1;
  }
  return nRpl;
}

/**
 * Replaces occurrances of a string by another string.
 *
 * @param lpsStr The string to be processed
 * @param lpsKey The string to replace
 * @param lpsRpl The string to replace lpKey with
 * @return The number of occurracnes of lpKey replaced by lpReplace
 * @see dlp_strreplace_ex
 */
INT16 dlp_strreplace(char* lpsStr, const char* lpsKey, const char* lpsRpl)
{
  return dlp_strreplace_ex(lpsStr,lpsKey,lpsRpl,FALSE);
}

/**
 * Extract token from string
 *
 * @param stringp
 *          The string to be processed (modified to string after delimiter)
 * @param delims
 *          The delimiter list
 * @param del
 *          Pointer to a buffer for one character, filled with the delimiter
 *          which succeeded the returned token, can be <code>NULL</code>
 * @return Pointer to the token
 */
char* dlp_strsep(char** stringp, const char* delims, char* del)
{
  INT32 p,d;
  INT32 n,nd;
  char* str;
  if (del) *del='\0';
  if(!stringp || !*stringp || !delims) return NULL;
  str=*stringp;
  n=strlen(str);
  nd=strlen(delims);
  for(p=0;p<n;p++)
  {
    for(d=0;d<nd;d++)
      if(str[p]==delims[d])
      {
        if (del) *del=str[p];
        str[p]='\0';
        *stringp=str+p+1;
        return str;
      }
  }
  *stringp=NULL;
  return str;
}

/**
 * Returns a non-zero value if and only if <code>lpsStr</code> starts with the
 * character sequence <code>lpsOther</code>.
 */
INT16 dlp_strstartswith(const char* lpsStr, const char* lpsOther)
{
  const char* x;
  const char* y;
  if (!dlp_strlen(lpsOther)) return TRUE;
  if (!lpsStr              ) return FALSE;
  for (x=lpsStr,y=lpsOther; *y; x++, y++)
    if (*x!=*y)
      return FALSE;
  return TRUE;
}

/**
 * Returns a non-zero value if and only if <code>lpsStr</code> ends with the
 * character sequence <code>lpsOther</code>.
 */
INT16 dlp_strendswith(const char* lpsStr, const char* lpsOther)
{
  const char* x;
  const char* y;
  if (!dlp_strlen(lpsOther)) return TRUE;
  if (!lpsStr              ) return FALSE;
  for (x=lpsStr+dlp_strlen(lpsStr)-1,y=lpsOther+dlp_strlen(lpsOther)-1; (x>=lpsStr) && (y>=lpsOther); x--, y--)
    if (*x!=*y)
      return FALSE;
  return TRUE;
}

/**
 * Protected implementation of the strtod function. Convert the initial portion
 * of the string pointed to by <code>lpsStr</code> to a <code>FLOAT64</code>
 * floating point number.
 *
 * <p>NOTE: The strtod function in older MingGWs does not handle <code>nan</code>
 * and <code>inf</code>.</p>
 *
 * @param lpsStr
 *          The string to be converted.
 * @param lpsEndPtr
 *          Fill with a pointer to the first character in <code>lpsStr</code>
 *          does not belong to the floating point number.
 * @return The floating point number (or 0. in case of errors).
 */
FLOAT64 dlp_strtod(const char* lpsStr, char** lpsEndPtr)
{
  FLOAT64 val = 0.;                                                             /* Return value                      */
  FLOAT64 sgn = 1.;                                                             /* Sign of return value              */
  char*   tx  = NULL;                                                           /* Auxiliary string pointer          */
  char*   ty  = NULL;                                                           /* Auxiliary string pointer #2       */

  if (lpsEndPtr!=NULL) *lpsEndPtr = NULL;                                       /* Initialize end pointer            */
  if (lpsStr==NULL) return 0.;                                                  /* No argument, no service           */

  val = strtod(lpsStr,&ty);                                                     /* Invoke stdlib function            */
  if (lpsStr==ty)                                                               /* No result -> try for ourselves... */
  {                                                                             /* >>                                */
    /* HACK: (for older MinGWs) Convert nan and infinity */                     /*                                   */
    tx = (char*)lpsStr; while (iswspace(*tx)) tx++;                             /*   Skip leading white spaces       */
    if (*tx=='-') { tx++; sgn=-1.0; } else if (*tx=='+') tx++;                  /*   Skip + or -                     */
    if (dlp_strnicmp(tx,"infinity",8)==0) { val = INFINITY; ty = tx+8; }        /*   Test for "infinity"             */
    else if (dlp_strnicmp(tx,"inf",3)==0) { val = INFINITY; ty = tx+3; }        /*   Test for "inf"                  */
    else if (dlp_strnicmp(tx,"nan",3)==0) { val = NAN;      ty = tx+3; }        /*   Test for "nan"                  */
  }                                                                             /* <<                                */

  /* Aftermath */                                                               /* --------------------------------- */
  if (lpsEndPtr!=NULL) *lpsEndPtr=ty;                                           /* Store end pointer                 */
  return sgn*val;                                                               /* Return result                     */
}

/* EOF */
