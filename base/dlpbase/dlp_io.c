/* dLabPro base library
 * - Console I/O
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

/* Get mktemp() on MSVC */
#ifdef __MSVC
#undef _POSIX_
#undef __STDC__
#include <io.h>
#endif
#ifdef SIGWINCH
#include <sys/ioctl.h>
#include <termios.h>
#endif

/* Local defines */
#define FPRINTF0(A,B)     bStr ? sprintf((char*)A,B  )   : ((stdout==(FILE*)A) ? printf(B    ) : fprintf((FILE*)A,B    ))
#define FPRINTF1(A,B,C)   bStr ? sprintf((char*)A,B,C)   : ((stdout==(FILE*)A) ? printf(B,C  ) : fprintf((FILE*)A,B,C  ))
#define FPRINTF2(A,B,C,D) bStr ? sprintf((char*)A,B,C,D) : ((stdout==(FILE*)A) ? printf(B,C,D) : fprintf((FILE*)A,B,C,D))

/* Global static variables */
static INT32 __nRows         = 80;
static INT32 __nColumns      = 120;
static INT32 __nLinectr      = 0;
static INT16 __nNonstop      = 0;
static INT16 __nNonstopMode  = 0;
static BOOL  __bColMode      = FALSE;
static BOOL  __bPipeMode     = FALSE;

void dlp_chgwinsz(void) {
#ifdef SIGWINCH
  struct winsize win;

  if (ioctl(fileno(stdout), TIOCGWINSZ, &win) != -1) {
    if (win.ws_row != 0) {
      __nRows = win.ws_row;
    }
    if (win.ws_col != 0)
      __nColumns = win.ws_col;
  }
#endif
}

/**
 * The dLabPro version of printf. printf itself is "overwritten" by a macro.
 *
 * @param lpsFormat Output format string
 * @param ...       Arguments according to format string
 * @return 0
 */
INT32 __dlp_printf(const char* lpsFormat, ...)
{
  va_list ap;

  /* Output everything immediately */
  va_start(ap,lpsFormat);
  vfprintf(stdout,lpsFormat,ap);
  va_end(ap);
  fflush(stdout);

  /* ok ... */
  return 0;
}

/* Local function. */
INT32 __dlp_printstop_ni(INT32 nItem, const char* lpNext, char* lpAnswer)
{
  INT32 i = 0;
  int   nTmpInt;
  char  lpBuf[L_INPUTLINE];

  __nLinectr = 0;

  if ((__nNonstop==1 || __nNonstopMode!=0) && !dlp_get_interrupt()) return nItem + 1;
  else __nNonstop = 0;

  printf("\n  continue <cr>, stop -1, nonstop -2");
  if (dlp_strlen(lpNext)) printf(", next %s: ",lpNext);
  else                    printf(": ");

  if(fgets(lpBuf,L_INPUTLINE,stdin)==NULL) return -1;
  dlp_strcpy(lpAnswer,lpBuf);
  __nLinectr = 1; /* users return key */
  dlp_set_interrupt(FALSE);
  if (sscanf(lpBuf,"%d",&nTmpInt)!=1) return nItem + 1;
  i = (INT32)nTmpInt;
  if (i == -1)  return -1;
  if (i == -2)
  {
    __nNonstop = 1;
    return nItem + 1;
  }
  else return i;
}

/**
 * <p>Prints a dLabPro message (debug, verbose, warning and error messages).
 * All messages from dLabPro kernel and classes should be printed using
 * this function. The following macros are supplied for convenience:</p>
 *
 * <table>
 *   <tr><th>Macro</th><th>defined in</th><th>Description</th></tr>
 *   <tr>
 *     <td><code><b>MSG</b>(nCheck,lpsFormat,arg1,arg2,arg3)</code></td>
 *     <td><code>dlp_inst.h</code></td>
 *     <td>Print verbose message if <code>CDlpInstance::m_nCheck>=nCheck</code></td>
 *   </tr>
 *   <tr>
 *     <td><code><b>DEBUGMSG</b>(nDlv,lpsFormat,arg1,arg2,arg3)</code></td>
 *     <td><code>dlp_base.h</code></td>
 *     <td>Print (kernel) debug message <code>if nDlv>=dlp_get_kernel_debug()</code></td>
 *   </tr>
 *   <tr>
 *     <td><code><b>ERRORMSG</b>(nError,arg1,arg2,arg3)</code></td>
 *     <td><code>dlp_inst.h</code></td>
 *     <td>Print error message</td>
 *   </tr>
 *   <tr>
 *     <td><code><b>ERRORRET</b>(nError,arg1,arg2,arg3,nRetval)</code></td>
 *     <td><code>dlp_inst.h</code></td>
 *     <td>Print error message and returns from function</td>
 *   </tr>
 * </table>
 *
 * @param lpsFilename Source file name of function call (use macro <code>__FILE__</code>)
 * @param nLine       Line number of function call (use macro <code>__LINE__</code>)
 * @param lpsFormat   A <a href="#cfn___dlp_printf">printf</a> format string
 * @see __dlp_printf printf
 */
 void dlp_message(const char* lpsFilename, INT32 nLine, const char* lpsFormat, ...)
 {
  va_list ap;

  if (dlp_strlen(lpsFilename)) __dlp_printf("\n%s(%d): ",lpsFilename,(int)nLine);
  if(__bColMode) __dlp_printf("[32m");

  va_start(ap,lpsFormat);
  vfprintf(stdout,lpsFormat,ap);
  va_end(ap);

  if(__bColMode) __dlp_printf("[0m");

  fflush(stdout);
}

/**
 * EXPERIMENTAL
 */
void dlp_error_message(const char* lpsFilename, INT32 nLine, const char* lpsFormat, ...)
{
  va_list ap;

#ifdef __TMS
  if (dlp_strlen(lpsFilename)) fprintf(stderr,"\n%s(%d): ",lpsFilename,(int)nLine);
#else
  if (dlp_strlen(lpsFilename)) fprintf(stderr,"\n%s(%ld): ",lpsFilename,(long)nLine);
#endif

  if (__bColMode) fprintf(stderr,"[31m");

  va_start(ap,lpsFormat);
  vfprintf(stderr,lpsFormat,ap);
  va_end(ap);

  if (__bColMode) fprintf(stderr,"[0m");

  if (__bPipeMode) fprintf(stderr,"\n");
  fflush(stderr);
}

/**
 * Initializes the list printing support. This feature allows to break
 * a listing after a certain number of printed lines and ask the user
 * whether to continue or stop the listing. <code>dlp_init_printstop</code>
 * resets the internal (static) line counter and the non-stop flag.
 *
 * @see #dlp_if_printstop dlp_if_printstop
 * @see #dlp_set_nonstop_mode dlp_set_nonstop_mode
 */
void dlp_init_printstop()
{
  __nLinectr = 0;
  __nNonstop = 0;
}

/**
 * Get the nonstop mode of the list printing support. In nonstop mode,
 * list printing never breaks.
 *
 * @param nMode The new nonstop mode (0 for break mode, no print stop
 *              otherwise
 * @return The previous setting of the nonstop mode
 * @see dlp_set_nonstop_mode
 */
INT16 dlp_get_nonstop_mode()
{
  return __nNonstopMode;
}

/**
 * Set the nonstop mode of the list printing support. In nonstop mode,
 * list printing never breaks.
 *
 * @param nMode The new nonstop mode (0 for break mode, no print stop
 *              otherwise
 * @return The previous setting of the nonstop mode
 * @see dlp_get_nonstop_mode
 */
INT16 dlp_set_nonstop_mode(INT16 nMode)
{
  INT16 nOld = __nNonstopMode;
  __nNonstopMode = nMode;
  return nOld;
}

/**
 * Set the pipe mode of error outputs.
 *
 * @param bMode
 *   If TRUE an additional line break is appended after each error message.
 */
void dlp_set_pipe_mode(BOOL bMode)
{
  __bPipeMode = bMode;
}

/**
 * Set the color mode of console outputs.
 *
 * @param bMode If TRUE switch on colored output, otherwise switch off.
 */
void dlp_set_color_mode(BOOL bMode)
{
  __bColMode = bMode;
}

/**
 * Increments the listung line counter by nInc without breaking
 * the listing in any case.
 *
 * @param nInc The increment
 * @return The current number of printed lines
 */
INT32 dlp_inc_printlines(INT32 nInc)
{
  __nLinectr += nInc;
  return __nLinectr;
}

/**
 * <p>Breaks a listing if a certain number (<code>__nRows</code>,
 * of lines was printed (i.e. any of the functions {@link dlp_if_printstop},
 * {@link dlp_printstop_ni}, {@link dlp_printstop_nix} or
 * {@link dlp_inc_printlines} were called <code>__nRows</code> times).
 * Prompts the user whether to stop or continue with or without further breaks.</p>
 *
 * @return TRUE if the listing shall be stopped, FALSE otherwise
 * @see #dlp_init_printstop dlp_init_printstop
 * @see #dlp_set_nonstop_mode dlp_set_nonstop_mode
 */
BOOL dlp_if_printstop()
{
  __nLinectr++;
  if (__nLinectr < dlp_maxprintlines()) return FALSE;
  return __dlp_printstop_ni(0,NULL,NULL)==-1;
}

/**
 * <p>Breaks a listing if a certain number (<code>__nRows</code>,
 * of lines was printed (i.e. any of the functions {@link dlp_if_printstop},
 * {@link dlp_printstop_ni}, {@link dlp_printstop_nix} or
 * {@link dlp_inc_printlines} were called <code>__nRows</code> times).
 * Prompts the user whether to stop or continue with or without further breaks.</p>
 *
 * @param nItem Currently printed item, this value is returned if
 *              the listing shall be continued at the current item.
 * @return The next item to print, or -1 if user broke the listing
 * @see dlp_init_printstop
 * @see dlp_set_nonstop_mode
 * @see dlp_if_printstop
 * @see dlp_printstop_nix
 */
INT32 dlp_printstop_ni(INT32 nItem)
{
  __nLinectr++;
  if (__nLinectr < dlp_maxprintlines()) return nItem + 1;
  return __dlp_printstop_ni(nItem,NULL,NULL);
}

/**
 * <p>Breaks a listing if a certain number (<code>__nRows</code>,
 * of lines was printed (i.e. any of the functions {@link dlp_if_printstop},
 * {@link dlp_printstop_ni}, {@link dlp_printstop_nix} or
 * {@link dlp_inc_printlines} were called <code>__nRows</code> times).
 * Prompts the user whether to stop or continue with or without further breaks.</p>
 *
 * <p>Additionally, the answer of the user is return in the string
 * buffer <code>lpAnswer</code>.</p>
 *
 * @param nItem    Currently printed item, this value is returned if
 *                 the listing shall be continued at the current item.
 * @param lpNext   Hint for the user, denotes the type of the next
 *                 item to print; e.g. "node"; may be <code>NULL</code>
 * @param lpAnswer Contains the user reply to the print stop message
 *                 or an empty string if there was no user interaction;
 *                 may be <code>NULL</code>
 * @return The next item to print, or -1 if user broke the listing
 * @see dlp_init_printstop
 * @see dlp_set_nonstop_mode
 * @see dlp_if_printstop
 * @see dlp_printstop_ni
 */
INT32 dlp_printstop_nix(INT32 nItem, const char* lpNext, char* lpAnswer)
{
  dlp_strcpy(lpAnswer,"");
  if (__nLinectr < dlp_maxprintlines()) return nItem + 1;
  return __dlp_printstop_ni(nItem,lpNext,lpAnswer);
}

/**
 * Returns the width of the console window in characters. In the
 * current implementation this value is a constant.
 */
INT16 dlp_maxprintcols()
{
  dlp_chgwinsz();
  return __nColumns;
}

/**
 * Returns the height of the console window in character lines. In
 * the current implementation this value is a constant.
 */
INT16 dlp_maxprintlines()
{
  dlp_chgwinsz();
  return __nRows;
}

/**
 * Prints a variable to an output stream in an default format.
 *
 * @param lpDest   Output; either FILE* (bStr=FALSE) or char* (bStr=TRUE)
 * @param lpBuffer Pointer to variable
 * @param nType    Variable type (dLabPro type code)
 * @param nArrIdx  Array index if lpBuffer points to an array,
 *                 must be 0 if lpBuffer points to a single value
 * @param bCols    Print with minimal width (for justified output)
 * @param bStr     TRUE: out is char*; FALSE: out is FILE* (incl. stdout)
 * @return Number of characters printed.
 * @see #dlp_getx dlp_getx
 */
INT16 dlp_printx(void* lpDest, void* lpBuffer, INT16 nType, INT32 nArrIdx, BOOL bCols, BOOL bStr)
{
  return dlp_printx_ext(lpDest,lpBuffer,nType,nArrIdx,bCols,bStr,FALSE);
}

/**
 * Prints a variable to an output stream in an default format.
 *
 * @param lpDest   Output; either FILE* (bStr=FALSE) or char* (bStr=TRUE)
 * @param lpBuffer Pointer to variable
 * @param nType    Variable type (dLabPro type code)
 * @param nArrIdx  Array index if lpBuffer points to an array,
 *                 must be 0 if lpBuffer points to a single value
 * @param bCols    Print with minimal width (for justified output)
 * @param bStr     TRUE: out is char*; FALSE: out is FILE* (incl. stdout)
 * @param bExact   Print exact value (for double and float values)
 * @return Number of characters printed.
 * @see #dlp_getx dlp_getx
 */
INT16 dlp_printx_ext(void *lpDest, void* lpBuffer, INT16 nType, INT32 nArrIdx, BOOL bCols, BOOL bStr, BOOL bExact)
{
  INT16 i   = 0;
  INT32   len = 0;

  if (lpBuffer  == 0   ) return 0;
  if (lpDest    == NULL) lpDest = stdout;

  if ((nType > 0) && (nType <= L_SSTR))
  {
    DLPASSERT(nArrIdx==0); /* String arrays not supported! */

    if (bStr)
    {
      char *tx = (char*)lpDest;
      if (bCols) *tx++=' ';
      for (i=0; i<nType && ((char*)lpBuffer)[i]!=0; i++) *tx++=((char*)lpBuffer)[i];
      *tx='\0';
    }
    else
    {
      if (bCols) FPRINTF0(lpDest," ");
      for (i=0; i<nType && ((char*)lpBuffer)[i]!=0; i++)
        FPRINTF1(lpDest,"%c",((char*)lpBuffer)[i]);
    }
    return (INT16)(nType+1);
  }

  len = 0;
  switch (nType)
  {
  case T_BOOL  : len = FPRINTF1(lpDest,"%s",((BOOL*)lpBuffer)[nArrIdx] ? "TRUE" : "FALSE"); break;
  case T_UCHAR : len = FPRINTF1(lpDest,bExact?"%hhu":(bCols?" 0x%02hhx":"0x%02hhx"),(unsigned char )((UINT8* )lpBuffer)[nArrIdx]); break;
  case T_CHAR  : len = FPRINTF1(lpDest,bExact?"%hhd":(bCols?" 0x%02hhx":"0x%02hhx"),(char          )(( INT8* )lpBuffer)[nArrIdx]); break;
  case T_USHORT: len = FPRINTF1(lpDest,(bCols&&!bExact)?"%6hu"    :"%u"            ,(unsigned short)((UINT16*)lpBuffer)[nArrIdx]); break;
  case T_SHORT : len = FPRINTF1(lpDest,(bCols&&!bExact)?"%6hd"    :"%d"            ,(short         )(( INT16*)lpBuffer)[nArrIdx]); break;
  case T_UINT  : len = FPRINTF1(lpDest,(bCols&&!bExact)?"%8u"     :"%u"            ,(unsigned int  )((UINT32*)lpBuffer)[nArrIdx]); break;
  case T_INT   : len = FPRINTF1(lpDest,(bCols&&!bExact)?"%8d"     :"%d"            ,(int           )(( INT32*)lpBuffer)[nArrIdx]); break;
  case T_ULONG : len = FPRINTF1(lpDest,(bCols&&!bExact)?"%12lu"   :"%lu"           ,(unsigned long )((UINT64*)lpBuffer)[nArrIdx]); break;
  case T_LONG  : len = FPRINTF1(lpDest,(bCols&&!bExact)?"%12ld"   :"%ld"           ,(long          )(( INT64*)lpBuffer)[nArrIdx]); break;
  case T_TEXT  :
  case T_STRING: len = FPRINTF1(lpDest,(bCols&&!bExact)?" %s"     :"%s"            ,(char*         )            lpBuffer          ); break;

  case T_FLOAT : {
    FLOAT32 f = ((FLOAT32*)lpBuffer)[nArrIdx];
    if (bExact) len = FPRINTF1(lpDest,"%.7e",(double)f); else
    if (bCols)
    {
      if (f > 10E8 || f < 10E-2) len = FPRINTF1(lpDest,"%12.3g",(double)f);
      else                       len = FPRINTF1(lpDest,"%12.3f",(double)f);
    }
    else len = FPRINTF1(lpDest,"%g",(double)f);
    break;}

  case T_DOUBLE: {
    FLOAT64 d = ((FLOAT64*)lpBuffer)[nArrIdx];
    if (bExact) len = FPRINTF1(lpDest,"%.16e",(double)d); else
    if (bCols)
    {
      if (d > 10E12 || d < 10E-5) len = FPRINTF1(lpDest,"%18.6g",(double)d);
      else                        len = FPRINTF1(lpDest,"%18.6f",(double)d);
    }
    else len = FPRINTF1(lpDest,"%g",(double)d);
    break; }

  case T_COMPLEX: {
    COMPLEX64 z = ((COMPLEX64*)lpBuffer)[nArrIdx];
    dlp_sprintc(lpDest,z,bExact);
    break; }

  default:
    len=FPRINTF0(lpDest," ? ");
    break;
  }

  if (len == EOF) len = 0;
  return (INT16)len;
}

/**
 * Gets a value of the specified type from stdin.
 *
 * @param lpDest Pointer to destination variable
 * @param nType  Variable type (dLabPro type code)
 * @return Number values read (0 or 1).
 * @see #dlp_getx dlp_printx
 */
INT16 dlp_getx(INT16 nType, void* lpDest)
{
  char lpBuf[L_INPUTLINE];

  if (lpDest == NULL) return NOT_EXEC;

  dlp_memset(lpBuf,0,L_INPUTLINE);
  if (fgets(lpBuf,L_INPUTLINE,stdin) == NULL) return 0;
  if (dlp_strlen(lpBuf) == 0) return NOT_EXEC;

  return dlp_sscanx(lpBuf,nType,lpDest);
}

/*
 * Scans one (complex) number including inf's and nan.
 */
INT16 CGEN_IGNORE dlp_sscanc(const char* lpsStr, COMPLEX64* lpnDst) {
  const char* ctx = lpsStr;                                                     /* TODO: ...                         */
  char* tx        = NULL;                                                       /* Char pointer                      */
  BOOL  bNan      = FALSE;                                                      /* Input is means NaN                */

  *lpnDst = CMPLX(0.);                                                          /* Initialize output                 */
  bNan  = (dlp_stricmp(lpsStr,"nan")==0)||(dlp_stricmp(lpsStr,"-nan")==0);      /* Real NaN                          */
  bNan |= (dlp_stricmp(lpsStr,"nan+nani")==0);                                  /* Complex NaN                       */
  if (bNan)                                                                     /* Input is NaN                      */
  {                                                                             /* >>                                */
    lpnDst->x=0.0/0.0; lpnDst->y=0.0/0.0;                                       /*   Store complex NaN               */
    return O_K;                                                                 /*   Return ok                       */
  }                                                                             /* <<                                */

  lpnDst->x = strtod(ctx,&tx);                                                  /* Get number                        */
  if(tx == ctx) {                                                               /* Doesn't start with number         */
    if((*ctx=='i') && (*(ctx+1)=='\0')) {                                       /* Only imaginary unit -> ok         */
      lpnDst->x=0;lpnDst->y= 1;                                                 /*   |                               */
      return O_K;                                                               /*   |                               */
    }                                                                           /* <<                                */
    if((*ctx=='+') && (*(ctx+1)=='i') && (*(ctx+2)=='\0')) {                    /* Only imaginary unit -> ok         */
      lpnDst->x=0;lpnDst->y= 1;                                                 /*   |                               */
      return O_K;                                                               /*   |                               */
    }                                                                           /* <<                                */
    if((*ctx=='-') && (*(ctx+1)=='i') && (*(ctx+2)=='\0')) {                    /* Only imaginary unit -> ok         */
      lpnDst->x=0;lpnDst->y=-1;                                                 /*   |                               */
      return O_K;                                                               /*   |                               */
    }                                                                           /* <<                                */
    return NOT_EXEC;                                                            /*   no number at all                */
  }                                                                             /*   <<                              */
  switch(*tx) {                                                                 /*   What is next?                   */
  case '\0': return O_K;                                                        /*   Pure real part -> ok            */
  case 'i' :                                                                    /*   Is it the imaginary part        */
    lpnDst->y = lpnDst->x;                                                      /*   Store it                        */
    ctx = tx+1;                                                                 /*   Skip 'i'                        */
    lpnDst->x = strtod(ctx,&tx);                                                /*   Get real part                   */
    if (*tx=='\0') return O_K;                                                  /*     End of input -> ok            */
    break;
  default:                                                                      /* Number is not imaginary part      */
    ctx = tx;                                                                   /*   Set start                       */
    lpnDst->y = strtod(ctx,&tx);                                                /*   Get imaginary part              */
    switch(*tx) {                                                               /*   What is next?                   */
    case 'i'  :                                                                 /*     i                             */
      if( *(tx+1)=='\0')                  {              return O_K; } break;   /*       ok                          */
    case '+'  :                                                                 /*     +i                            */
      if((*(tx+1)=='i')&&(*(tx+2)=='\0')) { lpnDst->y= 1;return O_K; } break;   /*       ok                          */
    case '-'  :                                                                 /*     -i                            */
      if((*(tx+1)=='i')&&(*(tx+2)=='\0')) { lpnDst->y=-1;return O_K; } break;   /*       ok                          */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */
  return NOT_EXEC;                                                              /* Something went wrong              */
}
/**
 * <p>Converts a string into exactly one value of the given type. If
 * <code>nType</code> is a string type <em>and</em> <code>lpsStr</code> is
 * enclosed in quotation marks, the function will remove the quotation marks and
 * unquote the string.</p>
 * <h4>Remarks</h4>
 * <ul>
 *   <li>The function performs no checks of the destination pointer (except that
 *     it rules out <code>NULL</code> pointers). If the scanned value does not
 *     fit into the buffer, the behaviour is undefined.</li>
 * </ul>
 *
 * @param lpsStr The string to scan
 * @param nType  The data type to convert the string into (T_XXX)
 * @param lpsDst The pointer to the destination variable
 * @return O_K if successful, NOT_EXEC otherwise. In case the function returns
 *         NOT_EXEC nothing has been written into <code>lpsDst</code>
 */
INT16 dlp_sscanx(const char* lpsStr, INT16 nType, void* lpsDst)
{
  INT32 nLen = 0;                                                               /* Length of destination buffer      */

  /* Validation */                                                              /* --------------------------------- */
  if (!lpsDst            ) return NOT_EXEC;                                     /* No dest. buffer - no service      */
  if (!dlp_strlen(lpsStr)) return NOT_EXEC;                                     /* No source string - no service     */
  nLen = (INT32)dlp_size(lpsDst);                                               /* Try to get size of dest. buffer   */
  if (dlp_is_numeric_type_code(nType))                                          /* Validation for numeric types      */
  {                                                                             /* >>                                */
    if (nLen>0 && nLen<dlp_get_type_size(nType))                                /*   Buffer known to be too small    */
    {                                                                           /*   >>                              */
      dlp_message(__FILE__,__LINE__,"dlp_sscanx - buffer too small!");          /*     TODO: Remove after debugging  */
      return NOT_EXEC;                                                          /*     Function failed               */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */
  else if (dlp_is_symbolic_type_code(nType) && nType<=L_SSTR)                   /* Validation for character arrays   */
  {                                                                             /* >>                                */
    if (nLen>0 && nLen<dlp_get_type_size(nType))                                /*   Buffer known to be too small    */
    {                                                                           /*   >>                              */
      dlp_message(__FILE__,__LINE__,"dlp_sscanx - buffer too small!");          /*     TODO: Remove after debugging  */
      return NOT_EXEC;                                                          /*     Function failed               */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */
  else if (dlp_is_symbolic_type_code(nType) && nType>L_SSTR)                    /* Validation for string types       */
  {                                                                             /* >>                                */
    if (nLen>0 && nLen<(INT32)dlp_strlen(lpsStr)+1)                             /*   Buffer known to be too small    */
    {                                                                           /*   >>                              */
      dlp_message(__FILE__,__LINE__,"dlp_sscanx - buffer too small!");          /*     TODO: Remove after debugging  */
      return NOT_EXEC;                                                          /*     Function failed               */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */
  else                                                                          /* Other than numeric or string type */
  {                                                                             /* >>                                */
    dlp_message(__FILE__,__LINE__,"dlp_sscanx - unsupported variable type");    /*   TODO: Remove after debugging    */
    DLPASSERT(FALSE);
    return NOT_EXEC;                                                            /*   Function failed                 */
  }                                                                             /* <<                                */

  /* Branch for types */                                                        /* --------------------------------- */
  if (nType == T_BOOL)                                                          /* Boolean                           */
  {                                                                             /* >>                                */
    if (dlp_strcmp(lpsStr,"TRUE")==0 || dlp_strcmp(lpsStr,"true")==0)           /*   True                            */
      return dlp_store((COMPLEX64){TRUE,0},lpsDst,nType);                       /*     Store TRUE                    */
    else if (dlp_strcmp(lpsStr,"FALSE")==0 || dlp_strcmp(lpsStr,"false")==0)    /*   False                           */
      return dlp_store((COMPLEX64){FALSE,0},lpsDst,nType);                      /*     Store FALSE                   */
    else                                                                        /*   No boolean literal              */
      return NOT_EXEC;                                                          /*     Not good                      */
  }                                                                             /* <<                                */
  else if (dlp_is_numeric_type_code(nType))                                     /* All numeric types but Boolean     */
  {                                                                             /* >>                                */
    COMPLEX64 c;                                                                /*   Complex buffer                  */
    if (dlp_sscanc(lpsStr,&c) == O_K)                                           /*   Scan (complex) number           */
      return dlp_store(c,lpsDst,nType);                                         /*   Store number                    */
  }                                                                             /* <<                                */
  else if (dlp_is_symbolic_type_code(nType))                                    /* Character array or string type    */
  {                                                                             /* >>                                */
    if (nType>=1 && nType<=L_SSTR) dlp_strncpy((char*)lpsDst,lpsStr,nType);     /*   Copy to character array         */
    else                           dlp_strcpy((char*)lpsDst,lpsStr);            /*   Copy to string buffer           */
    if                                                                          /*   Non-empty string in quot. marks */
    (                                                                           /*   |                               */
      dlp_strlen((char*)lpsDst) && ((char*)lpsDst)[0]=='\"' &&                  /*   |                               */
      ((char*)lpsDst)[dlp_strlen((char*)lpsDst)-1]=='\"'                        /*   |                               */
    )                                                                           /*   |                               */
    {                                                                           /*   >>                              */
      dlp_strunquotate((char*)lpsDst,'\"','\"');                                /*     Unquotate                     */
      dlp_strconvert(SC_UNESCAPE,(char*)lpsDst,(char*)lpsDst);                  /*     Unescape                      */
    }                                                                           /*   <<                              */
    return O_K;                                                                 /*   Function successful             */
  }                                                                             /* <<                                */
  return NOT_EXEC;                                                              /* Function failed                   */
}

/**
 * Prints a character multiple times to an output stream.
 *
 * @param lpFout Pointer to output stream or NULL for stdout
 * @param c      Character to print
 * @param n      Number of repetitions of character
 */
void dlp_fprint_x_line(FILE* lpFout, char c, INT16 n)
{
  INT16 i;
  char  lpBuf[4096];

  if (lpFout == NULL) lpFout = stdout;
  for (i=0; i<n && i<4095; i++) lpBuf[i]=c;
  lpBuf[i]='\0';

  fputs(lpBuf,lpFout);
}

/**
 * Returns the standard print width of the given type code.
 *
 * @param nType The dLabPro type code to get the print width for, one of the
 *              T_XXX constants.
 */
INT32 dlp_printlen(INT32 nType)
{
  switch (nType)
  {
    case T_UCHAR  :
    case T_CHAR   : return 5;
    case T_USHORT :
    case T_SHORT  : return 7;
    case T_UINT   :
    case T_INT    :
    case T_ULONG  :
    case T_LONG   :
    case T_FLOAT  :
    case T_DOUBLE : return 10;
    case T_COMPLEX: return 19;
    default      :
      if      ((nType > 0)&&(nType<32 ))  return nType+1;
      else if ((nType>=32)&&(nType<=255)) return 32;
      return 5;
  }
}

/*
 * Exactly format one double number to char buffer.
 */
const char* CGEN_IGNORE dlp_sfmtexactd(const char *fmt,double x){
  INT32 len;
  static char buf[L_NAMES];
  for(len=15;len<=17;len++){
    snprintf(buf,L_NAMES,fmt,len,x);
    if(strtod(buf,NULL)==x) break;
  }
  return buf;
}

/*
 * Print one complex number to char buffer.
 */
INT16 CGEN_IGNORE dlp_sprintc(char* lpsDst, COMPLEX64 what, BOOL bExact) {
  INT16 nLen;
  if(what.y == 0.0) {
    nLen = bExact ? sprintf(lpsDst," %s",dlp_sfmtexactd("% .*g",what.x))
                  : sprintf(lpsDst," % 9.6g",(double)what.x);
  } else {
    if(what.x == 0.0) {
      if(what.y == 1.0) {
        nLen = sprintf(lpsDst,"i");
      } else if(what.y == -1.0) {
        nLen = sprintf(lpsDst,"-i");
      } else {
        nLen = bExact ? sprintf(lpsDst, "%si",dlp_sfmtexactd("% .*g",what.y))
                      : sprintf(lpsDst, "% gi",(double)what.y);
      }
    } else {
      if(what.y == 1.0) {
        nLen = bExact ? sprintf(lpsDst," %s+i",dlp_sfmtexactd("% .*g",what.x))
                      : sprintf(lpsDst," % g+i",(double)what.x);
      } else if(what.y == -1.0) {
        nLen = bExact ? sprintf(lpsDst," %s-i",dlp_sfmtexactd(" %.*g",what.x))
                      : sprintf(lpsDst," % g-i",(double)what.x);
      } else if(bExact){
        char bufx[L_NAMES];
        strncpy(bufx,dlp_sfmtexactd("% .*g",what.x),L_NAMES);
        nLen = sprintf(lpsDst," %s%si",bufx,dlp_sfmtexactd("%+.*g",what.y));
      } else {
        nLen = sprintf(lpsDst," % g%+gi",(double)what.x,(double)what.y);
      }
    }
  }
  return nLen;
}
/**
 * Prints a variable to a string in an default format.
 *
 * @param lpsBuffer The string to print to
 * @param nWhat     The pointer to the variable
 * @param nType     The variable type, one of the T_XXX constants or
 *                  1 through 255 for character arrays
 * @param bExact    Prints exact values for floating point types.
 * @return The number of characters written to buffer (excluding the
 *         terminal null, or 0 in case of an error.
 */
INT32 dlp_sprintx(char* lpsBuffer, char* nWhat, INT32 nType, BOOL bExact)
{
  int nLen = 0;
  if(lpsBuffer==NULL) return(0);

  switch(nType)
  {
    case T_BOOL:
    {
      BOOL b;
      memcpy(&b,(BOOL*)(nWhat),sizeof(b));
      nLen = sprintf(lpsBuffer,b ? "TRUE" : "FALSE");
      break;
    }
    case T_UCHAR:
    {
      UINT8 uc;
      memcpy(&uc,(UINT8*)(nWhat),sizeof(uc));
      nLen = sprintf(lpsBuffer," %4hhu",(unsigned char)uc);
      break;
    }
    case T_CHAR:
    {
      INT8 c;
      memcpy(&c,(INT8*)(nWhat),sizeof(c));
      nLen = sprintf(lpsBuffer," %4hhd",(char)c);
      break;
    }
    case T_USHORT:
    {
      UINT16 ud;
      memcpy(&ud,(UINT16*)(nWhat),sizeof(ud));
      nLen = sprintf(lpsBuffer," %6hu",(unsigned short)ud);
      break;
    }
    case T_SHORT:
    {
      INT16 sd;
      memcpy(&sd,(INT16*)(nWhat),sizeof(sd));
      nLen = sprintf(lpsBuffer," %6hd",(short)sd);
      break;
    }
    case T_UINT:
    {
      UINT32 d;
      memcpy(&d,(UINT32*)(nWhat),sizeof(d));
      nLen = sprintf(lpsBuffer," %12u",(unsigned int)d);
      break;
    }
    case T_INT:
    {
      INT32 d;
      memcpy(&d,(INT32*)(nWhat),sizeof(d));
      nLen = sprintf(lpsBuffer," %12d",(int)d);
      break;
    }
    case T_ULONG:
    {
      UINT64 ul;
      memcpy(&ul,(UINT64*)(nWhat),sizeof(ul));
      nLen = sprintf(lpsBuffer," %12lu",(unsigned long)ul);
      break;
    }
    case T_LONG:
    {
      INT64 ld;
      memcpy(&ld,(INT64*)(nWhat),sizeof(ld));
      nLen = sprintf(lpsBuffer," %12ld",(long)ld);
      break;
    }
    case T_FLOAT:
    {
      FLOAT32 f=*((FLOAT32*)nWhat);
      nLen = dlp_sprintc(lpsBuffer,CMPLX(f),bExact);
      break;
    }
    case T_DOUBLE:
    {
      FLOAT64 lf=*((FLOAT64*)nWhat);
      nLen = dlp_sprintc(lpsBuffer,CMPLX(lf),bExact);
      break;
    }
    case T_COMPLEX:
    {
      COMPLEX64 lf=*((COMPLEX64*)nWhat);
      nLen = dlp_sprintc(lpsBuffer,lf,bExact);
      break;
    }
    default:
      if((nType>0)&&(nType<=L_SSTR))
      {
        char lpsFormat[10];
        char lpsBuf[257];
        strncpy(lpsBuf,nWhat,256);
        lpsBuf[nType]=0;
        sprintf(lpsFormat," %%-%ds",(int)((nType>32)?32:nType));
        nLen = sprintf(lpsBuffer,lpsFormat,lpsBuf);
      }
      else nLen = sprintf(lpsBuffer," ? ");
  }
  return nLen;
}

/**
 * Prints an unformatted string at stdout.
 *
 * @param nArgs
 *          The number of arguments that follow, all must be of type
 *          <code>char*</code>.
 */
void dlp_puts_ex(INT16 nArgs, ...)
{
  const char* tx;
  INT16       i;
  va_list     ap;

  va_start(ap,nArgs);
  for (i=0; i<nArgs; i++)
  {
    tx = va_arg(ap, const char*);
    if (tx==NULL) tx = "(null)";
    for (; *tx!='\0'; tx++) putchar(*tx);
  }

  va_end(ap);
}

/**
 * Similar to fgets but handles lines continued with a backslash.
 *
 * @param lpBuffer   Storage location for data
 * @param nBufferLen Maximum number of characters to read (including the terminal null)
 * @param lpfIn      Pointer to FILE structure of input file
 * @param nLines     Filled with the number of lines actually read
 * @return lpBuffer of NULL in case of an error.
 */
char* dlp_fgetl(char* lpBuffer, INT16 nBufferLen, FILE* lpfIn, INT32* nLines)
{
  INT16 bBackslash;
  char  lpBuf[L_INPUTLINE];
  lpBuffer[0] = 0;
  *nLines     = 0;

  if (!lpfIn) return NULL;

  do
  {
    bBackslash = FALSE;
    if (fgets(lpBuf,L_INPUTLINE,lpfIn)) {if (nLines) (*nLines)++;}
    else if (!lpBuffer[0]) return NULL;
    dlp_strtrimright(lpBuf);
    if (dlp_strlen(lpBuf) >0 && lpBuf[dlp_strlen(lpBuf)-1] == '\\')
    {
      bBackslash = (INT16)(dlp_strlen(lpBuf) ==1 || lpBuf[dlp_strlen(lpBuf)-2] != '\\');
      lpBuf[strlen(lpBuf)-1] = '\0';
    }
    strncat(lpBuffer,lpBuf,nBufferLen);
  } while (bBackslash);

  return lpBuffer;
}

/* EOF */
