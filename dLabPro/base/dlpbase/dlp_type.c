/* dLabPro base library
 * - Data types
 *
 * AUTHOR : Matthias Eichner
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
#include <math.h>

#define CLIP(A,B) (A<B##_MIN ? B##_MIN : (A>B##_MAX ? B##_MAX : A ))            /* Clip numeric values               */

static char __sBuf[TYPE_NAME_MAXLEN] = "";                                      /* Static type name buffer           */

typedef struct                                                                  /* Type description struct           */
{                                                                               /* >>                                */
  char  ttext[TYPE_NAME_MAXLEN];                                                /*   Type name                       */
  char  ctype[TYPE_NAME_MAXLEN];                                                /*   C type name                     */
  INT16 tcode;                                                                  /*   Type code                       */
  INT16 size;                                                                   /*   Type size                       */
} typetable;                                                                    /* <<                                */

static const typetable tab[] =                                                  /* Type list (except static strings) */
{                                                                               /* >>                                */
  {"unsigned char"      ,"UINT8"       ,T_UCHAR     ,sizeof (UINT8)     },      /*   unsigned char                   */
  {"char"               ,"char"        ,T_CHAR      ,sizeof (INT8)      },      /*   signed char alias char          */
  {"unsigned short"     ,"UINT16"      ,T_USHORT    ,sizeof (UINT16)    },      /*   unsigned short (16bit)          */
  {"short"              ,"INT16"       ,T_SHORT     ,sizeof (INT16)     },      /*   (signed) short (16bit)          */
  {"unsigned int"       ,"UINT32"      ,T_UINT      ,sizeof (UINT32)    },      /*   unsigned int (32bit)            */
  {"int"                ,"INT32"       ,T_INT       ,sizeof (INT32)     },      /*   Integer (32bit)                 */
  {"unsigned long"      ,"UINT64"      ,T_ULONG     ,sizeof (UINT64)    },      /*   unsigned long  (64bit)          */
  {"long"               ,"INT64"       ,T_LONG      ,sizeof (INT64)     },      /*   (signed) long  (64bit)          */
  {"float"              ,"FLOAT32"     ,T_FLOAT     ,sizeof (FLOAT32)   },      /*   float (32 bit)                  */
  {"double"             ,"FLOAT64"     ,T_DOUBLE    ,sizeof (FLOAT64)   },      /*   double (64bit)                  */
  {"complex"            ,"COMPLEX64"   ,T_COMPLEX   ,sizeof (COMPLEX64) },      /*   complex (64bit)                 */
  {"void*"              ,"void*"       ,T_PTR       ,sizeof (void*)     },      /*   (void) pointer                  */
  {"object"             ,"CDlpObject*" ,T_INSTANCE  ,sizeof (void*)     },      /*   Instance of a dLabPro object    */
  {"string"             ,"char*"       ,T_STRING    ,sizeof (char*)     },      /*   (Dynamic) character string      */
  {"cstring"            ,"const char*" ,T_CSTRING   ,sizeof (char*)     },      /*   (Constant) character string     */
  {"text"               ,"char*"       ,T_TEXT      ,sizeof (char*)     },      /*   Synonymous for string           */
  {"char*"              ,"char*"       ,T_TEXT      ,sizeof (char*)     },      /*   Synonymous for string           */
  {"bool"               ,"BOOL"        ,T_BOOL      ,sizeof (BOOL)      },      /*   Boolean                         */
  {""                   ,""            ,0           ,0                  }       /*   End of list (DO NOT REMOVE!)    */
};                                                                              /* <<                                */

/**
 * Prints a table of type names at stdout.
 */
void dlp_type_printtab()
{
  INT16 j;
  printf("\n   Table of types");
  for (j=0; tab[j].tcode>0; j++)
    printf("\n   %2hd: %5ld %-16s %2d",(short)j,(long)tab[j].tcode,tab[j].ttext,(int)tab[j].size);
}

/**
 * Get dLabPro type name from type code.
 *
 * @param nTypeCode
 *          The type code of the requested type
 * @return Returns a pointer to a character array containing the requested type
 *         name for all dynamic types, <code>"char[nTypeCode]"</code> for valid
 *         static string types and <code>NULL</code> for unknown types.
 * @see dlp_get_c_type
 * @see dlp_get_type_code
 * @see dlp_get_type_size
 */
const char* dlp_get_type_name(INT16 nTypeCode)
{
  INT16 j;
  if (nTypeCode==T_IGNORE) return NULL;
  if((nTypeCode>0) && (nTypeCode<=L_SSTR))                                      /* Is static string type code?       */
  {
    sprintf(__sBuf,"char[%d]",(int)nTypeCode);
    return(__sBuf);
  }
  else
    for (j=0; tab[j].tcode>0; j++)
      if (nTypeCode==tab[j].tcode)
        return tab[j].ttext;

  DLPASSERT(FMSG("Unknown type code"));                                         /* TODO: Remove after debugging      */
  return NULL;
}

/**
 * Get C type name from type code.
 *
 * @param nTypeCode
 *          The type code of the requested type
 * @return Returns a pointer to a character array containing the requested C
 *         type name for all dynamic types, <code>"char[nTypeCode]"</code> for
 *         valid static string types and <code>NULL</code> for unknown types.
 * @see dlp_get_type_name
 * @see dlp_get_type_code
 * @see dlp_get_type_size
 */
const char* dlp_get_c_type(INT16 nTypeCode)
{
  INT16 j;
  if((nTypeCode>0) && (nTypeCode<=L_SSTR))                                      /* Is static string type code?       */
  {
    sprintf(__sBuf,"char[%d]",(int)nTypeCode);
    return(__sBuf);
  }
  else
    for (j=0; tab[j].tcode>0; j++)
      if (nTypeCode==tab[j].tcode)
        return tab[j].ctype;

  DLPASSERT(FMSG("Unknown type code"));                                         /* TODO: Remove after debugging      */
  return NULL;
}

/**
 * Get dLabPro type code from a dLabPro type name.
 *
 * @param lpsTname
 *          A pointer to a string containing the name of a type
 * @return Returns the dLabPro type code associated with the type name or -1 if
 *         no such type exists.
 * @see dlp_get_c_type
 * @see dlp_get_type_name
 * @see dlp_get_type_size
 */
INT16 dlp_get_type_code(const char *lpsTypeName)
{
  INT16     j          = 0;
  int         l          = 0;
  char        t[L_NAMES];
  const char* tx         = NULL;

  if (dlp_strlen(lpsTypeName)<=0) return -1;

  /* Scan for static symbolic type code in lpTypeName */
  if
  (
    strstr(lpsTypeName,"char[")==lpsTypeName    &&
    lpsTypeName[dlp_strlen(lpsTypeName)-1]==']'
  )
  {
    dlp_strncpy(t,lpsTypeName,L_NAMES);
    t[dlp_strlen(t)-1] = '\0';
    tx = &t[5];
  }
  else tx = lpsTypeName;

  if (sscanf(tx,"%d",&l)==1)
  {
    if (0<l && l<=L_SSTR) return (INT16)l;                                      /* Valid static string type code     */
    else                  return -1;                                            /* Not a valid type code             */
  }

  /* Do name lookup */
  for(j=0; tab[j].tcode > 0; j++)
    if ((strcmp(tab[j].ttext,lpsTypeName)==0) || (strcmp(tab[j].ctype,lpsTypeName)==0))
      return tab[j].tcode;

  return -1;
}

/**
 * Get size of type in bytes.
 *
 * @param nTypeCode
 *          The type code of the requested type.
 * @return Returns the size of type <code>nTypeCode</code> in bytes or 0 if no
 *         <code>nTypeCode</code> is invalid.
 * @see dlp_get_type_name
 * @see dlp_get_c_type
 * @see dlp_get_type_code
 */
INT16 dlp_get_type_size(INT16 nTypeCode)
{
  INT16 j;
  if((nTypeCode>0) && (nTypeCode<=L_SSTR))                                      /* Is static string type code?       */
    return nTypeCode;
  else
    for (j=0; tab[j].tcode>0; j++)
      if (nTypeCode==tab[j].tcode)
        return tab[j].size;

  DLPASSERT(FMSG("Unknown type code"));                                         /* TODO: Remove after debugging      */
  return 0;
}

/**
 * Check for valid type code.
 *
 * @param nTypeCode
 *          The type code of the requested type.
 * @return Returns <code>TRUE</code> if <code>nTypeCode</code> is valid, 
 *         <code>FALSE</code> otherwise.
 * @see dlp_is_numeric_type_code
 * @see dlp_is_integer_type_code
 * @see dlp_is_float_type_code
 * @see dlp_is_symbolic_type_code
 * @see dlp_is_pointer_type_code
 */
INT16 dlp_is_valid_type_code(INT16 nTypeCode)
{
  return (dlp_get_type_size(nTypeCode)>0);
}

/**
 * Check for numeric type code.
 *
 * @param nTypeCode
 *          The type code of the requested type.
 * @return Returns <code>TRUE</code> if <code>nTypeCode</code> is valid and
 *         denotes a numeric (integer or floating point) type,
 *         <code>FALSE</code> otherwise.
 * @see dlp_is_integer_type_code
 * @see dlp_is_float_type_code
 * @see dlp_is_symbolic_type_code
 * @see dlp_is_pointer_type_code
 * @see dlp_is_valid_type_code
 */
INT16 dlp_is_numeric_type_code(INT16 nTypeCode) 
{
  switch(nTypeCode)
  {
    case T_BOOL   : /* Fall through */
    case T_UCHAR  : /* Fall through */
    case T_CHAR   : /* Fall through */
    case T_USHORT : /* Fall through */
    case T_SHORT  : /* Fall through */
    case T_UINT   : /* Fall through */
    case T_INT    : /* Fall through */
    case T_ULONG  : /* Fall through */
    case T_LONG   : /* Fall through */
    case T_FLOAT  : /* Fall through */
    case T_DOUBLE : /* Fall through */
    case T_COMPLEX: return TRUE;
    default       : return FALSE;
  }
}

/**
 * Check for integer type code.
 *
 * @param nTypeCode
 *          The type code of the requested type.
 * @return Returns <code>TRUE</code> if <code>nTypeCode</code> is valid and
 *         denotes an integer type, <code>FALSE</code> otherwise.
 * @see dlp_is_numeric_type_code
 * @see dlp_is_float_type_code
 * @see dlp_is_symbolic_type_code
 * @see dlp_is_pointer_type_code
 * @see dlp_is_valid_type_code
 */
INT16 dlp_is_integer_type_code(INT16 nTypeCode) 
{
  switch(nTypeCode)
  {
    case T_UCHAR : /* Fall through */
    case T_CHAR  : /* Fall through */
    case T_USHORT: /* Fall through */
    case T_SHORT : /* Fall through */
    case T_UINT  : /* Fall through */
    case T_INT   : /* Fall through */
    case T_ULONG : /* Fall through */
    case T_LONG  : return TRUE;
    default      : return FALSE;
  }
}

/**
 * Check for floating point type code.
 *
 * @param nTypeCode
 *          The type code of the requested type.
 * @return Returns <code>TRUE</code> if <code>nTypeCode</code> is valid and
 *         denotes a floating point type, <code>FALSE</code> otherwise.
 * @see dlp_is_numeric_type_code
 * @see dlp_is_integer_type_code
 * @see dlp_is_symbolic_type_code
 * @see dlp_is_pointer_type_code
 * @see dlp_is_valid_type_code
 */
INT16 dlp_is_float_type_code(INT16 nTypeCode) 
{
  switch(nTypeCode)
  {
    case T_FLOAT  : /* Fall through */
    case T_DOUBLE : /* Fall through */
    case T_COMPLEX: return TRUE;
    default      : return FALSE;
  }
}

/**
 * Check for complex type code.
 *
 * @param nTypeCode
 *          The type code of the requested type.
 * @return Returns <code>TRUE</code> if <code>nTypeCode</code> is valid and
 *         denotes a numeric (integer or floating point) type,
 *         <code>FALSE</code> otherwise.
 * @see dlp_is_integer_type_code
 * @see dlp_is_float_type_code
 * @see dlp_is_symbolic_type_code
 * @see dlp_is_pointer_type_code
 * @see dlp_is_valid_type_code
 */
INT16 dlp_is_complex_type_code(INT16 nTypeCode)
{
  switch(nTypeCode)
  {
    case T_COMPLEX: return TRUE;
    default       : return FALSE;
  }
}

/**
 * Check for symbolic (i.e. string) type code.
 *
 * @param nTypeCode
 *          The type code of the requested type.
 * @return Returns <code>TRUE</code> if <code>nTypeCode</code> is valid and
 *         denotes a symbolic type (i.e. a static or dynamic string type),
 *         <code>FALSE</code> otherwise.
 * @see dlp_is_numeric_type_code
 * @see dlp_is_integer_type_code
 * @see dlp_is_float_type_code
 * @see dlp_is_pointer_type_code
 * @see dlp_is_valid_type_code
 */
INT16 dlp_is_symbolic_type_code(INT16 nTypeCode)
{
  if((nTypeCode>0) && (nTypeCode<=L_SSTR)) return TRUE;

  switch(nTypeCode)
  {
    case T_TEXT   : /* Fall through */
    case T_CSTRING: /* Fall through */
    case T_STRING : return TRUE;
    default       : return FALSE;
  }
}

/**
 * Check for pointer type code.
 *
 * @param nTypeCode
 *          The type code of the requested type.
 * @return Returns <code>TRUE</code> if <code>nTypeCode</code> is valid and
 *         denotes a pointer type (<code>T_PTR</code> or
 *         <code>T_INSTANCE</code>), <code>FALSE</code> otherwise.
 * @see dlp_is_numeric_type_code
 * @see dlp_is_integer_type_code
 * @see dlp_is_float_type_code
 * @see dlp_is_symbolic_type_code
 * @see dlp_is_valid_type_code
 */
INT16 dlp_is_pointer_type_code(INT16 nTypeCode)
{
  switch (nTypeCode)
  {
    case T_PTR     : /* Fall trough */
    case T_INSTANCE: return TRUE;
    default        : return FALSE;
  }
}

/**
 * Converts a double number into a requested type and writes the result into
 * a buffer. If an overflow or underflow of the target type occurs, the result
 * is set to the max- e.g min-value of that type (clipped).
 *
 * @param nVal
 *          The number to convert
 * @param lpBuffer
 *           A pointer to buffer to write result to
 * @param nTypeCode
 *           The type code of the target type
 * @return <code>O_K</code> if conversion was successful, <code>NOT_EXEC</code>
 *         otherwise. Value clipping is <em>not</em> reported, the return value
 *         will still be <code>O_K</code>!
 * @see dlp_fetch
 */
INT16 dlp_store(COMPLEX64 nVal, void* lpBuffer, INT16 nTypeCode)
{
  /* NOTE: Max and min values of the types could be fetched from typetable.
     The resulting code would be more readable. */
  if (!lpBuffer) return NOT_EXEC;
  switch (nTypeCode)
  {
    case T_BOOL   : *(     BOOL*)lpBuffer = (   BOOL)CLIP(nVal.x,T_BOOL  ); return O_K;
    case T_UCHAR  : *(    UINT8*)lpBuffer = (  UINT8)CLIP(nVal.x,T_UCHAR ); return O_K;
    case T_CHAR   : *(     INT8*)lpBuffer = (   INT8)CLIP(nVal.x,T_CHAR  ); return O_K;
    case T_USHORT : *(   UINT16*)lpBuffer = ( UINT16)CLIP(nVal.x,T_USHORT); return O_K;
    case T_SHORT  : *(    INT16*)lpBuffer = (  INT16)CLIP(nVal.x,T_SHORT ); return O_K;
    case T_UINT   : *(   UINT32*)lpBuffer = ( UINT32)CLIP(nVal.x,T_UINT  ); return O_K;
    case T_INT    : *(    INT32*)lpBuffer = (  INT32)CLIP(nVal.x,T_INT   ); return O_K;
    case T_ULONG  : *(   UINT64*)lpBuffer = ( UINT64)CLIP(nVal.x,T_ULONG ); return O_K;
    case T_LONG   : *(    INT64*)lpBuffer = (  INT64)CLIP(nVal.x,T_LONG  ); return O_K;
    case T_FLOAT  : *(  FLOAT32*)lpBuffer = (FLOAT32)CLIP(nVal.x,T_FLOAT ); return O_K;
    case T_DOUBLE : *(  FLOAT64*)lpBuffer = (FLOAT64)CLIP(nVal.x,T_DOUBLE); return O_K;
    case T_COMPLEX: *(COMPLEX64*)lpBuffer = CMPLXY(CLIP(nVal.x,T_DOUBLE),CLIP(nVal.y,T_DOUBLE)); return O_K;
    default      : return NOT_EXEC;
  }
}

/**
 * Reads a numeric value from a buffer and converts it to a double.
 * 
 * @param lpBuffer
 *          Pointer to a buffer containing the number to convert
 * @param nTypeCode
 *          Type code of the number to convert
 * @return Returns converted number in double format and 0. in case of an
 *         invalid type code or an invalid buffer pointer.
 * @see dlp_store
 */
COMPLEX64 dlp_fetch(const void* lpBuffer, INT16 nTypeCode)
{
 /* NOTE: Size of the types could be fetched from typetable.
          The resulting code would be more consistent.*/
  if (!lpBuffer) return CMPLX(0);
  switch (nTypeCode)
  {
    case T_BOOL   : { return (COMPLEX64){*(     BOOL*)lpBuffer,0.};  }
    case T_UCHAR  : { return (COMPLEX64){*(    UINT8*)lpBuffer,0.};  }
    case T_CHAR   : { return (COMPLEX64){*(     INT8*)lpBuffer,0.};  }
    case T_USHORT : { return (COMPLEX64){*(   UINT16*)lpBuffer,0.};  }
    case T_SHORT  : { return (COMPLEX64){*(    INT16*)lpBuffer,0.};  }
    case T_UINT   : { return (COMPLEX64){*(   UINT32*)lpBuffer,0.};  }
    case T_INT    : { return (COMPLEX64){*(    INT32*)lpBuffer,0.};  }
    case T_ULONG  : { return (COMPLEX64){*(   UINT64*)lpBuffer,0.};  }
    case T_LONG   : { return (COMPLEX64){*(    INT64*)lpBuffer,0.};  }
    case T_FLOAT  : { return (COMPLEX64){*(  FLOAT32*)lpBuffer,0.};  }
    case T_DOUBLE : { return (COMPLEX64){*(  FLOAT64*)lpBuffer,0.};  }
    case T_COMPLEX: { return (COMPLEX64) *(COMPLEX64*)lpBuffer;      }
    default       : { return CMPLX(0);                               }
  }
}

/**
 * Checks given double-precision floating-point value for not a number (NaN).
 * 
 * @param nVal
 *          Double-precision floating-point value
 * @return A nonzero value (<code>TRUE</code>) if the argument nVal is a NaN,
 *         0 (<code>FALSE</code>) otherwise
 */
INT32 dlp_isnan(FLOAT64 nVal)
{
  return isnan(nVal);
}

/**
 * Determines whether the given double-precision floating-point value is finite.
 * 
 * @param nVal
 *          Double-precision floating-point value.
 * @return A nonzero value if <code>nVal</code> is not infinite - that is, if
 *         <code>�INF &lt; nVal &lt; +INF</code> - or 0 if <code>nVal</code>
 *         is infinite or a <code>NAN</code>.
 */
INT16 dlp_finite(FLOAT64 nVal)
{
  return (nVal<T_DOUBLE_MAX && nVal > T_DOUBLE_MIN);
}

/* EOF */
