/* dLabPro base library
 * - Header file
 *
 * AUTHOR : Matthias Wolff
 * PACKAGE: dLabPro/sdk
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

#ifndef __DLPBASE_H
#define __DLPBASE_H

#define __DLP_VERMAJ "2.5"
#define __DLP_VERMIN "5"

/* Detect machine, OS and compiler */
#if defined MSDOS
  #define __I386
  #define __MSDOS                                /* MS-DOS 5.0 or higher      */
  #define __MSC                                  /* MS C 6.0 or higher        */
  #define __MSOS

#elif (defined __BORLANDC__ && defined _Windows)
  #define __I386
  #define __BC                                   /* Borland C++ 3.1 or higher */
  #define __MSOS
  #define _WINDOWS                               /* for convenience           */
  #if defined WIN32
    #define __WIN32                              /* Windows 9x/NT4/2000/ME    */
  #else
    #define __WIN31                              /* Windows 3.1               */
  #endif

#elif (defined __GNUC__)
  #define __GNUC
  #undef  _POSIX_C_SOURCE
  #define _POSIX_C_SOURCE 1
  #undef  _GNU_SOURCE
  #define _GNU_SOURCE     1

  #if (defined __MINGW32__)
    #define __I386
    #if __GNUC__ < 4
      #define __UNIX_EMULATION
      #ifndef _WINDOWS
        #define _WINDOWS                             /* for convenience           */
      #endif
    #endif
  #endif

  #if (defined __CYGWIN__)
    #define __I386
  #endif

  #if (defined __alpha__ && defined __osf__)
    #define __ALPHA                              /* Alpha WS                  */
    #define __OSF                                /* DEC OSF/1                 */
    #define __LONG64                             /* 64 bit long               */
  #endif

  #if (defined __mips__ && defined ultrix)
    #define __MIPS                               /* DEC RISC                  */
    #define __ULTRIX                             /* DEC ULTRIX                */
  #endif

  #if (defined __sparc__)
    #define __SPARC                              /* SUN SPARC WS              */
    /* Don't know OS */
  #endif

  #if (defined __linux__)
    #define __I386
    #define __LINUX
  #endif

#elif ((defined __MICROSOFT__ && defined _WINDOWS) || defined _WIN32)
  #define __I386
  #define __MSVC                                 /* MS Visual C++             */
  #define __MSOS
  #ifndef _WINDOWS
    #define _WINDOWS                             /* for convenience           */
  #endif
  #if defined WIN32
    #define __WIN32                              /* Windows 9x/NT4/2000/ME    */
  #else
    #define __WIN31                              /* Windows 3.1               */
  #endif

#elif defined _TMS320C6X
  #define __GNUC
  #undef  _POSIX_C_SOURCE
  #define _POSIX_C_SOURCE 1
  #undef  _GNU_SOURCE
  #define _GNU_SOURCE     1
  #define __TMS
  #define iswspace(C)  isspace(C)

#else
  #error Unknown C++ compiler, operating system, or machine
#endif

/* Detect machine, OS and compiler - Backward compatibility */
#ifdef __alpha__                                 /* DEC alpha machines       */
  #define _ALPHA
#endif
#ifdef __mips__                                  /* DEC mips machines        */
  #define _MIPS
#endif
#ifdef __sparc__                                 /* SUN sparc machines       */
  #define _SPARC
  #include <ieeefp.h>
#endif
#ifdef __linux__                                 /* Linux Intel PC           */
  #define _LINUX
#endif

/* Includes - common */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <float.h>
#ifndef __TMS
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <memory.h>
#endif
#include <time.h>
#include <string.h>
#include "kzl_hash.h"
#include "dlp_config.h"
#if HAVE_BYTESWAP_H
# include <byteswap.h>
# define BSWAP_16(x) bswap_16(x)
# define BSWAP_32(x) bswap_32(x)
#else
# define BSWAP_16(x) ((((x) & 0xff00) >> 8) | (((x) & 0x00ff) << 8))
# define BSWAP_32(x) ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >> 8) |\
                      (((x) & 0x0000ff00) << 8) | (((x) & 0x000000ff) << 24))
#endif

#if (defined __GNUC__)
  #include <stdint.h>
#endif
#if (defined __GNUC__ && !defined __MINGW32__)
  /* HACK: GCC/Eclipse --> */
  extern char* tempnam(const char*, const char*);
  extern int strcasecmp(const char*, const char*);
  extern int strncasecmp(const char*, const char*, size_t);
  /* <-- */
#endif

#ifdef __TMS
  #include <stdint.h>
#endif

#ifdef __MSVC
  /* MS Visual C++ additional includes */
  #include <direct.h>
  #include <float.h>
  #ifdef _DEBUG
    #include "crtdbg.h"
  #endif
  #define HAVE_NO_MKSTEMP

#elif !defined __MSOS && !defined __UNIX_EMULATION && !defined __TMS
  /* Unix (incl. Linux) additional includes */
  #include <unistd.h>
  #include <signal.h>
/*  #include <values.h>*/
  #include <wctype.h>
  #include <errno.h>
  #define PTW32_STATIC_LIB
  #define HAVE_STRUCT_TIMESPEC
  #include <pthread.h>
  #define HAVE_PTHREAD
#endif

#ifdef __TMS
  #include <errno.h>
#endif

#ifdef __UNIX_EMULATION
  #include <unistd.h>
  #define HAVE_MSTHREAD
#endif

#ifdef __MINGW32__
  #include <windows.h>
  #undef CALLBACK  /* HACK! */
  #undef sleep
  #define sleep(x) Sleep(x)
  #define HAVE_NO_MKSTEMP
	#define expf(x)		exp(x)
	#define logf(x)		log(x)
	#define log10f(x)	log10(x)
	#define sqrtf(x)	sqrt(x)
	#define sinf(x)		sin(x)
	#define asinf(x)	asin(x)
	#define sinhf(x)	sinh(x)
	#define cosf(x)		cos(x)
	#define acosf(x)	acos(x)
	#define coshf(x)	cosh(x)
	#define tanf(x)		tan(x)
	#define atanf(x)	atan(x)
	#define tanhf(x)	tanh(x)
  #undef min
  #undef max
  /* These math functions are not found by MinGW gcc for whatever reason... */
  extern double __cdecl tgamma(double);
  extern double __cdecl round(double);
  extern double __cdecl erf(double);
  extern double __cdecl erfc(double);
#endif

#if defined __ALPHA
  /* File status stuff is in sys/mode.h ! */
  #include <sys/mode.h>

#endif

/* Disable some MSVC warnings */
#ifdef __MSVC
  #pragma warning(disable : 4127) /* Bedingter Ausdruck ist konstant */
  #pragma warning(disable : 4514) /* Nichtreferenzierte Inline-Funktion wurde entfernt */
  #pragma warning(disable : 4786) /* Bezeichner wurde auf '255' Zeichen in den Fehlerinformationen verkuerzt */
  #pragma warning(disable : 4244)
  #pragma warning(disable : 4267)
  #pragma warning(disable : 4996)
#endif

#if (defined __x86_64)
  #define __LONG64                             /* 64 bit long               */
#endif

typedef     float            FLOAT32;
typedef   uint8_t            UINT8;
typedef    int8_t            INT8;
typedef  uint16_t            UINT16;
typedef   int16_t            INT16;
typedef  uint32_t            UINT32;
typedef   int32_t            INT32;
typedef   uint8_t            BYTE;
#define BOOL INT8

#if defined __TMS || defined __MAX_TYPE_32BIT
  #undef  DBL_MANT_DIG
  #define DBL_MANT_DIG       FLT_MANT_DIG
  #define       UINT64       UINT32
  #define        INT64       INT32
  #define      FLOAT64       FLOAT32
#else
  typedef    double          FLOAT64;
  typedef  uint64_t          UINT64;
  typedef   int64_t          INT64;
#endif

typedef struct { FLOAT32 x; FLOAT32 y; } COMPLEX32;
typedef struct { FLOAT64 x; FLOAT64 y; } COMPLEX64;

#ifdef __cplusplus
#define CMPLX(x)             ((COMPLEX64){static_cast<FLOAT64>(x),static_cast<FLOAT64>(0)})
#define CMPLXY(x,y)          ((COMPLEX64){static_cast<FLOAT64>(x),static_cast<FLOAT64>(y)})
#else
#define CMPLX(x)             ((COMPLEX64){(x),(0)})
#define CMPLXY(x,y)          ((COMPLEX64){(x),(y)})
#endif
#define CMPLX_EQUAL(z1,z2)   (((z1).x==(z2).x)&&((z1).y==(z2).y))
#define CMPLX_LESS(z1,z2)    ((z1).x<(z2).x)
#define CMPLX_LEQ(z1,z2)     ((z1).x<=(z2).x)
#define CMPLX_GREATER(z1,z2) ((z1).x>(z2).x)
#define CMPLX_GEQ(z1,z2)     ((z1).x>=(z2).x)
#define CMPLX_ISNAN(z1)      (dlp_isnan((z1).x)||dlp_isnan((z1).y))
#define CMPLX_MIN(z1,z2)     (CMPLX_LESS(z1,z2)?z1:z2)
#define CMPLX_MAX(z1,z2)     (CMPLX_LESS(z1,z2)?z2:z1)
#define CMPLX_PLUS(z1,z2)    ((COMPLEX64){(z1).x+(z2).x,(z1).y+(z2).y})
#define CMPLX_MINUS(z1,z2)   ((COMPLEX64){(z1).x-(z2).x,(z1).y-(z2).y})
#define CMPLX_MULT(z1,z2)    ((COMPLEX64){(z1).x*(z2).x-(z1).y*(z2).y,(z1).x*(z2).y+(z1).y*(z2).x})
#define CMPLX_MULT_R(z,r)    ((COMPLEX64){(z).x*(r),(z).y*(r)})
#define CMPLX_DIV(z1,z2)     dlp_scalopC(z1,z2,OP_DIV)
#define CMPLX_DIV_R(z,r)     ((COMPLEX64){(z).x/(r),(z).y/(r)})
#define CMPLX_CONJ(z)        ((COMPLEX64){(z).x,-(z).y})
#define CMPLX_INVT(z)        dlp_scalopC(z,CMPLX(0),OP_INVT)
#define CMPLX_NEG(z)         ((COMPLEX64){-(z).x,-(z).y})
#define CMPLX_ABS(z)         dlp_scalopC(z,CMPLX(0),OP_ABS).x
#define CMPLX_ANGLE(z)       atan2((z).y,(z).x)
#define CMPLX_SQR(z)         dlp_scalopC(z,CMPLX(0),OP_SQR)
#define CMPLX_ENT(z)         ((COMPLEX64){(INT64)(z).x,(INT64)(z).y})
#define CMPLX_FLOOR(z)       ((COMPLEX64){floor((z).x),floor((z).y)})
#define CMPLX_CEIL(z)        ((COMPLEX64){ceil((z).x),ceil((z).y)})
#define CMPLX_INC(z)         ((COMPLEX64){(z).x+1,(z).y})
#define CMPLX_DEC(z)         ((COMPLEX64){(z).x-1,(z).y})
#define CMPLX_NAN            CMPLXY(0.0/0.0, 0.0/0.0)

/* Disable unused parameter warning with gcc */
#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

/* Defines - Project information */
#define __PROJECT_NAME   "dLabPro"
#define __PROJECT_CODE   "dlp"
#define __VERSION_MAJOR  2.5
#define __VERSION_MINOR  4
#define __VERSION_DATE   "APR13"
#define __ENV_DEBUGGER   "DLP_DEBUGGER"

/* Defines - Variable ranges */
#define TYPE_NAME_MAXLEN 20
#define T_BOOL_MAX       ((BOOL)1)
#define T_BOOL_MIN       ((BOOL)0)
#define T_UCHAR_MAX      ((UINT8)(~(UINT8)0))
#define T_UCHAR_MIN      ((UINT8)0)
#define T_CHAR_MAX       ((INT8)(T_UCHAR_MAX/2))
#define T_CHAR_MIN       ((INT8)(-(T_UCHAR_MAX/2)-1))
#define T_USHORT_MAX     ((UINT16)(~(UINT16)0))
#define T_USHORT_MIN     ((UINT16)0)
#define T_SHORT_MAX      ((INT16)(T_USHORT_MAX/2))
#define T_SHORT_MIN      ((INT16)(-(T_USHORT_MAX/2)-1))
#define T_UINT_MAX       ((UINT32)(~(UINT32)0))
#define T_UINT_MIN       ((UINT32)0)
#define T_INT_MAX        ((INT32)(T_UINT_MAX/2))
#define T_INT_MIN        ((INT32)(-(T_UINT_MAX/2)-1))
#define T_FLOAT_MAX      FLT_MAX
#define T_FLOAT_MIN      -FLT_MAX
#define T_FLOAT_TINY     FLT_MIN
#if defined __TMS || defined __MAX_TYPE_32BIT
#define T_DOUBLE_MAX     T_FLOAT_MAX
#define T_DOUBLE_MIN     T_FLOAT_MIN
#define T_DOUBLE_TINY    FLT_MIN
#define T_LONG_MAX       T_INT_MAX
#define T_LONG_MIN       T_INT_MIN
#define T_ULONG_MAX      T_UINT_MAX
#define T_ULONG_MIN      T_UINT_MIN
#else
#define T_DOUBLE_MAX     DBL_MAX
#define T_DOUBLE_MIN     -DBL_MAX
#define T_DOUBLE_TINY    DBL_MIN
#define T_ULONG_MAX      ((UINT64)(~(UINT64)0))
#define T_ULONG_MIN      ((UINT64)0)
#define T_LONG_MAX       ((INT64)(T_ULONG_MAX/2))
#define T_LONG_MIN       ((INT64)(-(T_ULONG_MAX/2)-1))
#endif

/* Defines - Buffer sizes */
#define L_SSTR            255 /* MUST NOT be greater 255 (because of DNorm!) */ /* Size of static strings            */
#define BUFF_LEN         1024
#define L_INPUTLINE      2048                                                   /* Size of input buffer              */
#define L_MANUAL         2048                                                   /* Size of manual's entries          */
#define L_NAMES            48                                                   /* Names' length, <=L_SSTR!          */
#define L_PATH            260                                                   /* Maximal length of paths           */
#define L_STACK           255                                                   /* Length of stacks                  */

/* Defines - Platform dependent characters */
#ifdef __MSOS
  #define C_DIR          '\\'
  #define NEWLINE        "\n\r"
#else
  #define C_DIR          '/'
  #define NEWLINE        "\n"
#endif

/* Defines - pseudo-random generator */
#ifdef RAND_MAX
#undef RAND_MAX
#endif
#define RAND_MAX 0xFFFFFFFFFFFFFFFF

/* Defines - Constants */
#undef  TRUE
#undef  FALSE
#undef  NULL
#undef  PI
#define TRUE             1
#define FALSE            0
#define NULL             0
#define F_PI             3.1415926535897932384626433832795                      /* PI                                */
#define F_E              2.7182818284590452353602874713527                      /* Euler constant                    */
#define F_TINY           1e-20                                                  /* Tiny float or double              */
#ifndef NAN                                                                     /* Should be defined <math.h> >>     */
#define NAN              ((FLOAT64)(0.0/0.0))                                   /*   If not: define own NaN          */
#endif                                                                          /* <<                                */
#ifndef INFINITY                                                                /* Should be defined <math.h> >>     */
#define INFINITY         ((FLOAT64)(1.0/0.0))                                   /*   If not: define own inf          */
#endif                                                                          /* <<                                */

/* Defines - Declspecs */
#ifdef __MSVC
  #define CALLBACK       __stdcall
#else
  #define CALLBACK
#endif

/* Constants - CGen scanner marks */
#define CGEN_PUBLIC                                                             /* public:                           */
#define CGEN_VPUBLIC                                                            /* public: virtual                   */
#define CGEN_VPPUBLIC                                                           /* public: virtual = 0               */
#define CGEN_SPUBLIC                                                            /* public: static                    */
#define CGEN_PROTECTED                                                          /* protected:                        */
#define CGEN_VPROTECTED                                                         /* protected: virtual                */
#define CGEN_VPPROTECTED                                                        /* protected: virtual = 0            */
#define CGEN_SPROTECTED                                                         /* protected: static                 */
#define CGEN_PRIVATE                                                            /* private:                          */
#define CGEN_SPRIVATE                                                           /* private: static                   */
#define CGEN_EXPORT                                                             /* C-scope function                  */
#define CGEN_INLINE                                                             /* C-scope inline function           */
#define CGEN_IGNORE                                                             /* Ignore function (local)           */
#define DEFAULT(A)                                                              /* Default argument                  */

/* Thread specific stuff *
 *
 * The following functions are defined by macros
 * to speed up execution. They return O_K if
 * successful and NOT_EXEC otherwise. More documentation
 * can be found in the specific implementation
 * (for example pthreads).
 *
 * INT16 dlp_create_mutex(MUTEXHANDLE *mutex);
 * void  dlp_destroy_mutex(MUTEXHANDLE *mutex);
 * INT16 dlp_lock_mutex(MUTEXHANDLE *mutex);
 * void  dlp_unlock_mutex(MUTEXHANDLE *mutex);
 * INT16 dlp_create_cond(CONDHANDLE *cond);
 * void  dlp_destroy_cond(CONDHANDLE *cond);
 * void  dlp_wait_cond(CONDHANDLE *cond,MUTEXHANDLE *mutex);
 * void  dlp_signal_cond(CONDHANDLE *cond);
 * void  dlp_broadcast_cond(CONDHANDLE *cond);
 */
#ifdef HAVE_PTHREAD
  #define THREADHANDLE pthread_t
  #define THREADFUNC void *(*ThreadFunc) (void *)
  #define MUTEXHANDLE pthread_mutex_t
  #define CONDHANDLE  pthread_cond_t
  #define dlp_create_mutex(mutex)   (pthread_mutex_init(mutex,NULL)?NOT_EXEC:O_K)
  #define dlp_destroy_mutex(mutex)  (pthread_mutex_destroy(mutex))
  #define dlp_lock_mutex(mutex)     (pthread_mutex_lock(mutex)     ?NOT_EXEC:O_K)
  #define dlp_unlock_mutex(mutex)   (pthread_mutex_unlock(mutex))
  #define dlp_create_cond(cond)     (pthread_cond_init(cond,NULL)  ?NOT_EXEC:O_K)
  #define dlp_destroy_cond(cond)    (pthread_cond_destroy(cond))
  #define dlp_wait_cond(cond,mutex) (pthread_cond_wait(cond,mutex))
  #define dlp_signal_cond(cond)     (pthread_cond_signal(cond))
  #define dlp_broadcast_cond(cond)  (pthread_cond_broadcast(cond))
#elif defined HAVE_MSTHREAD
  #define THREADHANDLE HANDLE
  #define THREADFUNC void *(*ThreadFunc) (void *)
  #define MUTEXHANDLE void*
  #define CONDHANDLE  void*
  #define dlp_create_mutex(mutex)   NOT_EXEC
  #define dlp_destroy_mutex(mutex)  NOT_EXEC
  #define dlp_lock_mutex(mutex)     NOT_EXEC
  #define dlp_unlock_mutex(mutex)   NOT_EXEC
  #define dlp_create_cond(cond)     NOT_EXEC
  #define dlp_destroy_cond(cond)    NOT_EXEC
  #define dlp_wait_cond(cond,mutex) NOT_EXEC
  #define dlp_signal_cond(cond)     NOT_EXEC
  #define dlp_broadcast_cond(cond)  NOT_EXEC
#else
  #define THREADHANDLE void*
  #define THREADFUNC void*(*ThreadFunc) (void *)
  #define MUTEXHANDLE void*
  #define CONDHANDLE  void*
  #define dlp_create_mutex(mutex)   NOT_EXEC
  #define dlp_destroy_mutex(mutex)  NOT_EXEC
  #define dlp_lock_mutex(mutex)     NOT_EXEC
  #define dlp_unlock_mutex(mutex)   NOT_EXEC
  #define dlp_create_cond(cond)     NOT_EXEC
  #define dlp_destroy_cond(cond)    NOT_EXEC
  #define dlp_wait_cond(cond,mutex) NOT_EXEC
  #define dlp_signal_cond(cond)     NOT_EXEC
  #define dlp_broadcast_cond(cond)  NOT_EXEC
#endif

/* Process id */
#ifdef pid_t
  #define PROCESSID pid_t
#else
  #define PROCESSID INT32
#endif

/* Defines - XA_XXX: dLabPro heap manager settings */
#define XA_HEAP_MEMLEAKS  0x0001
#define XA_HEAP_INTEGRITY 0x0002
#define XA_DLP_MEMLEAKS   0x0004

/* Defines - T_XXX: dLabPro variable type codes */
#define T_BOOL           1000
#define T_BYTE           1001
#define T_UCHAR          1001
#define T_CHAR           2001
#define T_USHORT         1002
#define T_SHORT          2002
#define T_UINT           1004
#define T_INT            2004
#define T_ULONG          1008
#define T_LONG           2008
#define T_FLOAT          3004
#define T_DOUBLE         3008
#define T_COMPLEX        3009
#define T_STRING         5000
#define T_TEXT           5001
#define T_CSTRING        5002
#define T_PTR            6000
#define T_INSTANCE       6002
#define T_IGNORE         10000
#define T_STKBRAKE       10001                                                  /* Stack brake - EXPERIMENTAL        */

/* Defines - Operand types */
#define T_OP_UNKNOWN     0x0000
#define T_OP_REAL        0x0001
#define T_OP_COMPLEX     0x0002
#define T_OP_INTEGER     0x0004
#define T_OP_BOOL        0x0008
#define T_OP_STRING      0x0010
#define T_OP_INSTANCE    0x0100
#define T_OP_DATA        0x0300
#define T_OP_VAR         0x0500

/* Defines - Return values */
#define O_K              1
#define NOT_EXEC         -1
#define ERR_TRUNCATE     -2                                                     /* for CDlpTable                     */
#define ERR_MEM          -3                                                     /* Memory error                      */
#define ERR_MDIM         -101                                                   /* Matrix dimension error            */
#define ERR_MSQR         -102                                                   /* Matrix dim. error (not square)    */
#define ERR_MSGL         -103                                                   /* Matrix is singular                */
#define ERR_MASYM        -104                                                   /* Matrix is asymmetric              */
#define ERR_MNEG         -105                                                   /* All matrix elements negative      */
#define ERR_MTYPE        -106                                                   /* Matrix is of wrong type           */
#define ERR_MSUPP        -107                                                   /* Matrix op. not supported/implem.  */

/* Defines - Evaluation of return values */
#undef  OK
#define OK(A)            ((A)>0)
#define NOK(A)           ((A)<=0)
#define IF_OK(A)         if ((A)>0)
#define IF_NOK(A)        if ((A)<=0)
#define SWITCH_NOK(A)    switch ((A))

/* Defines - OP_XXX: Numeric constants and operation codes */                   /* --------------------------------- */
#define OP_NOOP             0                                                   /* No operation                      */
#define OP_CONST_MIN        1                                                   /* First constant                    */
#define OP_CONST_MAX      999                                                   /* Last constant                     */
#define OP_SCALOP_MIN    1000                                                   /* First scalar operation            */
#define OP_MONADIC_MIN   1100                                                   /* First monadic scalar operation    */
#define OP_DYADIC_MIN    1200                                                   /* First dyadic scalar operation     */
#define OP_SCALOP_MAX    1299                                                   /* Last scalar operation             */
#define OP_MONADIC_MAX   1199                                                   /* Last monadic scalar operation     */
#define OP_DYADIC_MAX    1299                                                   /* Last dyadic scalar operation      */
#define OP_MATROP_MIN    1300                                                   /* First matrix operation            */
#define OP_MATROP_MAX    1499                                                   /* Last matrix operation             */
#define OP_AGGROP_MIN    2000                                                   /* First aggregation operation       */
#define OP_AGGROP_MAX    2999                                                   /* Last aggregation operation        */
#define OP_STROP_MIN     3000                                                   /* First aggregation operation       */
#define OP_STROP_MAX     3999                                                   /* Last aggregation operation        */

/* - Constants */                                                               /* - - - - - - - - - - - - - - - - - */
#define OP_TRUE             1                                                   /* Logical true                      */
#define OP_FALSE            2                                                   /* Logical false                     */
#define OP_NULL             3                                                   /* Null-pointer                      */
#define OP_PI               4                                                   /* Pi                                */
#define OP_E                5                                                   /* Euler constant                    */

/* - Logical operations */                                                      /* - - - - - - - - - - - - - - - - - */
#define OP_OR            1001                                                   /* Logical or                        */
#define OP_AND           1002                                                   /* Logical and                       */
#define OP_NOT           1003                                                   /* Logical not                       */
#define OP_EQUAL         1004                                                   /* Equal                             */
#define OP_NEQUAL        1005                                                   /* Not equal                         */
#define OP_LESS          1006                                                   /* Less than                         */
#define OP_GREATER       1007                                                   /* Greater than                      */
#define OP_LEQ           1008                                                   /* Less or equal                     */
#define OP_GEQ           1009                                                   /* Greater or equal                  */
#define OP_ISNAN         1010                                                   /* Is NaN                            */

/* - Monadic scalar math operations */                                          /* - - - - - - - - - - - - - - - - - */
#define OP_NEG           1100                                                   /* Negation                          */
#define OP_REAL          1101                                                   /* Real part                         */
#define OP_IMAG          1102                                                   /* Imaginary part                    */
#define OP_CONJ          1103                                                   /* Complex conjugate                 */
#define OP_SQR           1104                                                   /* Square                            */
#define OP_ABS           1105                                                   /* Absolute value                    */
#define OP_SQRT          1106                                                   /* Square root                       */
#define OP_SIGN          1107                                                   /* Sign                              */
#define OP_ENT           1108                                                   /* Entire                            */
#define OP_FLOOR         1109                                                   /* Floor                             */
#define OP_CEIL          1110                                                   /* Ceil                              */
#define OP_INC           1111                                                   /* Increment (+1)                    */
#define OP_DEC           1112                                                   /* Decrement (-1)                    */
#define OP_INVT          1113                                                   /* Inversion                         */
#define OP_LOG           1114                                                   /* Decadic logarithm                 */
#define OP_LOG2          1115                                                   /* Binary logarithm                  */
#define OP_LN            1116                                                   /* Natural logarithm                 */
#define OP_EXP           1117                                                   /* Exponential function              */
#define OP_SIN           1118                                                   /* Sine                              */
#define OP_ASIN          1119                                                   /* Arc sine                          */
#define OP_SINH          1120                                                   /* Hyperbolic sine                   */
#define OP_ASINH         1121                                                   /* Arc hyperbolic sine               */
#define OP_COS           1122                                                   /* Cosine                            */
#define OP_ACOS          1123                                                   /* Arc cosine                        */
#define OP_COSH          1124                                                   /* Hyperbolic cosine                 */
#define OP_ACOSH         1125                                                   /* Arc hyperbolic cosine             */
#define OP_TAN           1126                                                   /* Tangent                           */
#define OP_ATAN          1127                                                   /* Arc tangent                       */
#define OP_TANH          1128                                                   /* Hyperbolic tangent                */
#define OP_ATANH         1129                                                   /* Arc hyperbolic tangent            */
#define OP_SET           1130                                                   /* Assignment                        */
#define OP_FCTRL         1131                                                   /* Factorial                         */
#define OP_GAMMA         1132                                                   /* Gamma function                    */
#define OP_LGAMMA        1133                                                   /* Log. Gamma function               */
#define OP_STUDT         1134                                                   /* Student's t-density (k samples)   */
#define OP_ROUND         1135                                                   /* Round                             */
#define OP_ERF           1136                                                   /* Error function                    */
#define OP_ERFC          1137                                                   /* Complementary error function      */
#define OP_SINC          1138                                                   /* Sinc function                     */
#define OP_ANGLE         1139                                                   /* Angle of complex value            */
#define OP_BETA          1140                                                   /* Euler's Beta function             */
#define OP_BETADENS      1141                                                   /* Beta density                      */
#define OP_BETAQUANT     1142                                                   /* P-quantile of Beta CDF            */
/* - Dyadic scalar math operations */                                           /* - - - - - - - - - - - - - - - - - */
#define OP_ADD           1200                                                   /* Addition                          */
#define OP_LSADD         1201                                                   /* Log semiring addition             */
#define OP_EXPADD        1202                                                   /* Exponential addition              */
#define OP_DIFF          1203                                                   /* Difference                        */
#define OP_ABSDIFF       1204                                                   /* Absolute difference               */
#define OP_QDIFF         1205                                                   /* Quadratic difference              */
#define OP_QABSDIFF      1206                                                   /* Quadratic absolute difference     */
#define OP_MULT          1207                                                   /* Multiplication                    */
#define OP_DIV           1208                                                   /* Division                          */
#define OP_DIV1          1209                                                   /* Division by y+1                   */
#define OP_MOD           1210                                                   /* Modulus                           */
#define OP_LNL           1211                                                   /* Natural logarithm, limited        */
#define OP_POW           1212                                                   /* Power                             */
#define OP_NOVERK        1213                                                   /* n over k                          */
#define OP_GAUSS         1214                                                   /* Gaussian function                 */
#define OP_SIGMOID       1215                                                   /* Sigmoid function                  */
#define OP_AIZER         1216                                                   /* Aizer function (?)                */
#define OP_POTENTIAL     1217                                                   /* Potential function (?)            */
#define OP_BITOR         1218                                                   /* Bitwise or                        */
#define OP_BITAND        1219                                                   /* Bitwise and                       */

/* Defines - Dedicated matrix operation codes, scalops also apply */            /* --------------------------------- */
#define OP_TRANSPOSE     1301                                                   /* Transpose                         */
#define OP_LDIV          1302                                                   /* - Reserved -                      */
#define OP_DIAG          1303                                                   /* Make diagonal matrix              */
#define OP_MDIAG         1304                                                   /* Get main diagonal                 */
#define OP_TRACE         1305                                                   /* Trace                             */
#define OP_DET           1306                                                   /* Determinant                       */
#define OP_MULT_SPARSE   1307                                                   /* Sparse multiplication             */
#define OP_NORM          1308                                                   /* Euclidic norm                     */
#define OP_MULT_KRON     1309                                                   /* Kronecker product                 */
#define OP_MULT_AKAT     1310                                                   /* Matrix product A*K*A'             */
#define OP_CONV          1311                                                   /* Convolution                       */
#define OP_CHOLF         1312                                                   /* Cholesky factorization            */
#define OP_CCF           1313                                                   /* Cross correlation function        */
#define OP_SOLV_LU       1314                                                   /* Solve Ax=B using LU decomposition */
#define OP_MULT_EL       1350                                                   /* Elementwise multiplication        */
#define OP_DIV_EL        1351                                                   /* Elementwise division              */
#define OP_LOG_EL        1352                                                   /* Elementwise decadic logarithm     */
#define OP_LOG2_EL       1353                                                   /* Elementwise binary logarithm      */
#define OP_LN_EL         1354                                                   /* Elementwise natural logarithm     */
#define OP_EXP_EL        1355                                                   /* Elementwise exponential function  */
#define OP_SQRT_EL       1356                                                   /* Elementwise square root           */
#define OP_POW_EL        1357                                                   /* Elementwise power                 */
#define OP_OR_EL         1371                                                   /* Elementwise logical or            */
#define OP_AND_EL        1372                                                   /* Elementwise logical and           */
#define OP_EQUAL_EL      1374                                                   /* Elementwise equal                 */
#define OP_NEQUAL_EL     1375                                                   /* Elementwise not equal             */
#define OP_LESS_EL       1376                                                   /* Elementwise less than             */
#define OP_GREATER_EL    1377                                                   /* Elementwise greater than          */
#define OP_LEQ_EL        1378                                                   /* Elementwise less or equal         */
#define OP_GEQ_EL        1379                                                   /* Elementwise greater or equal      */
#define OP_BITOR_EL      1380                                                   /* Elementwise bitwise or            */
#define OP_BITAND_EL     1381                                                   /* Elementwise bitwise and           */

/* Defines - Matrix constants operation codes */                                /* --------------------------------- */
#define OP_ZEROS         1400                                                   /* 0-matrix                          */
#define OP_ONES          1401                                                   /* 1-matrix                          */
#define OP_NOISE         1402                                                   /* Noise matrix                      */
#define OP_UNITMAT       1403                                                   /* Unit matrix                       */
#define OP_HILBMAT       1404                                                   /* Hilbert matrix                    */
#define OP_IHLBMAT       1405                                                   /* Inverse Hilbert matrix            */

/* - Aggregation operations */                                                  /* - - - - - - - - - - - - - - - - - */
#define OP_SUM           2000                                                   /* Sum                               */
#define OP_PROD          2001                                                   /* Product                           */
#define OP_MAX           2002                                                   /* Maximum                           */
#define OP_AMAX          2003                                                   /* Absolute maximum                  */
#define OP_SMAX          2004                                                   /* Signed absolute maximum           */
#define OP_IMAX          2005                                                   /* Index of maximum                  */
#define OP_MIN           2006                                                   /* Minimum                           */
#define OP_AMIN          2007                                                   /* Absolute minimum                  */
#define OP_SMIN          2008                                                   /* Signed absolute minimum           */
#define OP_IMIN          2009                                                   /* Index of minimum                  */
#define OP_SPAN          2010                                                   /* Spanwidth                         */
#define OP_MEAN          2011                                                   /* Arithmetic mean                   */
#define OP_AMEAN         2012                                                   /* Absolute mean                     */
#define OP_QMEAN         2013                                                   /* Quadratic mean                    */
#define OP_MOMENT        2014                                                   /* k-th moment                       */
#define OP_CMOMENT       2015                                                   /* k-th central moment               */
#define OP_GMEAN         2016                                                   /* Geometric mean                    */
#define OP_HMEAN         2017                                                   /* Harmonic mean                     */
#define OP_RANK          2018                                                   /* Element of rank k                 */
#define OP_MED           2019                                                   /* Median                            */
#define OP_QUANTIL       2020                                                   /* k quantil                         */
#define OP_QUARTIL       2021                                                   /* Quartil (k=0.25)                  */
#define OP_IQDIST        2022                                                   /* Interquartil distance             */
#define OP_VAR           2023                                                   /* Variance                          */
#define OP_STDEV         2024                                                   /* Standard deviation                */
#define OP_SKEW          2025                                                   /* Skewness                          */
#define OP_EXC           2026                                                   /* Excess (kurtosis)                 */
#define OP_MINK          2027                                                   /* Minkowski sum (power k)           */
#define OP_MINKPOW       2028                                                   /* Minkowski sum (power k) w/o root  */
#define OP_LSSUM         2029                                                   /* Log semiring sum                  */
#define OP_LSMEAN        2030                                                   /* Log semiring mean                 */
#define OP_VMAX          2031                                                   /* Vector maximum                    */
#define OP_VMIN          2032                                                   /* Vector minimum                    */

/* Defines - AOP_XXX: automaton operation codes */                              /* --------------------------------- */
#define AOP_UNION        1000                                                   /* Union                             */
#define AOP_INTERS       1001                                                   /* Intersection                      */
#define AOP_CLOSE        1002                                                   /* Kleene closure                    */
#define AOP_INV          1003                                                   /* Inversion                         */
#define AOP_PROJ         1004                                                   /* Projection                        */
#define AOP_WRM          1005                                                   /* Weight removal                    */
#define AOP_PROD         1006                                                   /* Cartesian product                 */
#define AOP_CMPS         1007                                                   /* Composition                       */
#define AOP_DET          1008                                                   /* Determinization                   */
#define AOP_MIN          1009                                                   /* Minimization                      */
#define AOP_ERM          1010                                                   /* Epsilon removal                   */
#define AOP_HMM          1011                                                   /* Convert to HMM style              */
#define AOP_TREE         1012                                                   /* Tree                              */
#define AOP_BESTN        1013                                                   /* n best paths                      */

/* Defines - SOP_XXX: string operation codes */                                 /* --------------------------------- */
#define SOP_CCAT         3000                                                   /* Concatenate components            */
#define SOP_CHASH        3001                                                   /* Cell-by-cell hash                 */
#define SOP_CMP          3002                                                   /* Compare                           */
#define SOP_HASH         3003                                                   /* Global hash                       */
#define SOP_LEFT         3004                                                   /* Left n characters                 */
#define SOP_LEN          3005                                                   /* Length                            */
#define SOP_LWR          3006                                                   /* Lower case                        */
#define SOP_RCAT         3007                                                   /* Concatenate records               */
#define SOP_REPLACE      3008                                                   /* Replace keystrings                */
#define SOP_RIGHT        3009                                                   /* Right n characters                */
#define SOP_SEARCH       3010                                                   /* Search sub string                 */
#define SOP_SPLIT        3011                                                   /* Split at delimiters               */
#define SOP_SPLITALL     3012                                                   /* Split at every delimiter          */
#define SOP_SPLITD       3013                                                   /* Split keeping delimiters          */
#define SOP_SPLITP       3014                                                   /* Split path names                  */
#define SOP_TRIM         3015                                                   /* Trim characters                   */
#define SOP_UPR          3016                                                   /* Upper case                        */

/* Defines - FOP_XXX: signal operation codes */
#define FOP_APS          4000                                                   /* Auto power spectrum               */
#define FOP_CCF          4001                                                   /* Cross correlation function        */
#define FOP_CEP2LPC      4002                                                   /* Cepstrum to LPC                   */
#define FOP_CEP2MCEP     4003                                                   /* Cepstrum to Mel-Cepstrum          */
#define FOP_CEP          4004                                                   /* Cepstrum                          */
#define FOP_DEFRAME      4005                                                   /* Signal de-framing                 */
#define FOP_DENOISE      4006                                                   /* M-Cepstrum denoising              */
#define FOP_DESCALE      4007                                                   /* Signal descaling                  */
#define FOP_DISTRIBUTION 4008                                                   /* Value distribution                */
#define FOP_DTW          4009                                                   /* Dynamic Time Warping              */
#define FOP_F02EXC       4010                                                   /* Get excitation from F0 contour    */
#define FOP_FFT          4011                                                   /* Fourier transform                 */
#define FOP_FILTER       4012                                                   /* Infinite impulse response filter  */
#define FOP_FIR          4013                                                   /* Finite impulse response filter    */
#define FOP_FRAME        4014                                                   /* Signal framing                    */
#define FOP_SFRAME       4070                                                   /* Pitch synchron signal framing     */
#define FOP_GCEP         4015                                                   /* GCEP                              */
#define FOP_GCEP2GCEP    4016                                                   /* Generalized Cepstrum transform    */
#define FOP_GCEP2LPC     4017                                                   /* GCEP to LPC transform             */
#define FOP_GCEP2MLPC    4018                                                   /* GCEP to Mel-LPC transform         */
#define FOP_GCEPNORM     4019                                                   /* Gain normalization                */
#define FOP_GETF0        4020                                                   /* F0 Estimation                     */
#define FOP_GMULT        4021                                                   /* Multiply by gamma                 */
#define FOP_IFFT         4022                                                   /* Inverse fourier transform         */
#define FOP_IGCEPNORM    4023                                                   /* Inverse gain normalization        */
#define FOP_IGMULT       4024                                                   /* Divide by gamma                   */
#define FOP_IIR          4025                                                   /* Purely Inf. imp. response filter  */
#define FOP_IMCEP        4026                                                   /* Inverse Mel-Cepstrum              */
#define FOP_IMLT         4027                                                   /* Inverse Modulated Lapped Transform*/
#define FOP_ISVQ         4028                                                   /* Inverse Scalar Vector Quantization*/
#define FOP_IVQ          4029                                                   /* Inverse Vector Quantization       */
#define FOP_LPC          4030                                                   /* LPC                               */
#define FOP_LPC2CEP      4031                                                   /* LPC to Cepstrum                   */
#define FOP_LPC2GCEP     4032                                                   /* LPC to GCEP transform             */
#define FOP_LPC2MGCEP    4033                                                   /* LPC to Mel-GCEP transform         */
#define FOP_LPC2MLPC     4034                                                   /* LPC to Mel-LPC transform          */
#define FOP_LSF2POLY     4035                                                   /* LSF to Polynomial transform       */
#define FOP_MCEP2CEP     4036                                                   /* Mel-Cepstrum to Cepstrum          */
#define FOP_MCEP2MCEP    4037                                                   /* Mel-Cepstrum to Mel-Cepstrum      */
#define FOP_MCEP2MLPC    4038                                                   /* Mel-Cepstrum to Mel-LPC           */
#define FOP_MCEP         4039                                                   /* M-Cepstrum                        */
#define FOP_MCEPENHANCE  4040                                                   /* M-Cepstrum enhancement            */
#define FOP_MFB          4041                                                   /* Mel Filter Bank                   */
#define FOP_MFBS         4069                                                   /* Mel Filter Bank in spectral domain*/
#define FOP_MFFT         4042                                                   /* Mel-FFT                           */
#define FOP_MFILTER      4043                                                   /* Mel-Infinite impulse resp. filter */
#define FOP_MFIR         4044                                                   /* Mel-Finite impulse response filter*/
#define FOP_MGCEP        4045                                                   /* Mel-GCEP                          */
#define FOP_MGCEP2LPC    4046                                                   /* Mel-GCEP to LPC transform         */
#define FOP_MGCEP2MGCEP  4047                                                   /* Mel-Generalized Cepstrum transform*/
#define FOP_MGCEP2MLPC   4048                                                   /* Mel-GCEP to Mel-LPC transform     */
#define FOP_MIIR         4049                                                   /* Purely Mel-Inf. imp. resp- filter */
#define FOP_MLPC         4050                                                   /* M-LPC                             */
#define FOP_MLPC2GCEP    4051                                                   /* Mel-LPC to GCEP transform         */
#define FOP_MLPC2LPC     4052                                                   /* Mel-LPC to LPC transform          */
#define FOP_MLPC2MCEP    4053                                                   /* Mel-LPC to Mel-Cepstrum           */
#define FOP_MLPC2MGCEP   4054                                                   /* Mel-LPC to Mel-GCEP transform     */
#define FOP_MLPC2MLPC    4055                                                   /* Mel-LPC to Mel-LPC transform      */
#define FOP_MLSF2MLSF    4056                                                   /* Mel-LSF to Mel-LSF transform      */
#define FOP_MLT          4057                                                   /* Modulated Lapped Transform        */
#define FOP_NOISIFY      4058                                                   /* Signal noisifying                 */
#define FOP_PITCHMARK    4059                                                   /* Pitch marking                     */
#define FOP_POLY2LSF     4060                                                   /* Polynomial to LSF transform       */
#define FOP_RMDC         4061                                                   /* Remove DC                         */
#define FOP_ROOTS        4062                                                   /* Roots of polynomial               */
#define FOP_SCALE        4063                                                   /* Signal scaling                    */
#define FOP_SVQ          4064                                                   /* Scalar Vector Quantization        */
#define FOP_WINDOW       4065                                                   /* Windowing                         */
#define FOP_WVL          4071                                                   /* Wavelet analysis                  */
#define FOP_VQ           4066                                                   /* Vector Quantization               */
#define FOP_UNWRAP       4067                                                   /* Phase unwrapping                  */
#define FOP_ZCR          4068                                                   /* Zero crossing                     */

/* Deprecated operation codes */                                                /* --------------------------------- */
#define OP_EQUAL_L       2311                                                   /* - deprecated - Class CForm        */
#define OP_NEQUAL_L      2321                                                   /* - deprecated - Class CForm        */
#define OP_LESS_L        2332                                                   /* - deprecated - Class CForm        */
#define OP_GREATER_L     2341                                                   /* - deprecated - Class CForm        */
#define OP_LEQ_L         2351                                                   /* - deprecated - Class CForm        */
#define OP_GEQ_L         2361                                                   /* - deprecated - Class CForm        */

/* Defines - CN_XXX: string conversion codes (dlp_strconvert) */                /* --------------------------------- */
#define CN_QUOTE            1                                                   /* Put in quotation marks            */
#define CN_UNQUOTE          2                                                   /* Remove quotation marks            */
#define CN_XLATPATH         3                                                   /* Translate path delimiters         */
#define CN_HFILE          100                                                   /* Header file name from project name*/
#define CN_CFILE          101                                                   /* C file name from project name     */
#define CN_CPPFILE        102                                                   /* C++ file name from project name   */
#define CN_BSIFILE        103                                                   /* - deprecated -                    */
#define CN_MANFILE        104                                                   /* Manual file name from project name*/
#define CN_MAKEFILE       105                                                   /* make file name from project name  */
#define CN_NMAKFILE       106                                                   /* nmake file name from project name */
#define CN_DSWFILE        107                                                   /* Visual Studio workspace file name */
#define CN_DSPFILE        108                                                   /* Visual Studio project file name   */
#define CN_GENFILE        109                                                   /* - deprecated -                    */
#define CN_NCBFILE        110                                                   /* MSVC class browser info file      */
#define CN_SIDFILE        111                                                   /* Search index file name            */
#define CN_DLP2CXX_CLSN   200                                                   /* dLabPro to C++ class name         */
#define CN_DLP2CXX_CCF    201                                                   /* dLabPro method to C++ callback fct*/
#define CN_DLP2CXX_OUCF   202                                                   /* dLabPro option to C++ callback fct*/
#define CN_DLP2CXX_PUCF   203                                                   /* dLabPro field to C++ callback fct.*/
#define CN_DLP2CXX_BPAR   204                                                   /* dLabPro to C++ field (boolean)    */
#define CN_DLP2CXX_NPAR   205                                                   /* dLabPro to C++ field (numeric)    */
#define CN_DLP2CXX_LPAR   206                                                   /* dLabPro to C++ field (pointer)    */
#define CN_DLP2CXX_SPAR   207                                                   /* dLabPro to C++ field (string ptr) */
#define CN_DLP2CXX_IPAR   208                                                   /* dLabPro to C++ field (instance)   */
#define CN_DLP2CXX_DPAR   209                                                   /* dLabPro to C++ field (data)       */
#define CN_DLP2CXX_TPAR   210                                                   /* dLabPro to C++ field (fst)        */
#define CN_DLP2CXX_PAR    211                                                   /* dLabPro to C++ field (other)      */
#define CN_CSTRUCTNAME    212                                                   /* Project name to C par. truct name */
#define CN_AUTONAME       213                                                   /* Default auto instance identifie   */
#define SC_URL_ESCAPE     300                                                   /* URL style encoding                */
#define SC_URL_UNESCAPE   301                                                   /* URL style decoding                */
#define SC_PRC_ESCAPE     302                                                   /* Quote percent chars. for printf   */
#define SC_ESCAPE         303                                                   /* Escape                            */
#define SC_UNESCAPE       304                                                   /* Unescape                          */
#define SC_STRIPHTML      305                                                   /* Strip HTML tags from string       */
#define SC_UNESCAPE_ITP24 306                                                   /* - for backward compatibility -    */

/* Defines - IDT_XXX : dLabPro class member types (dlp_is_valid_id) */          /* --------------------------------- */
/*           IVIR_XXX: return codes                                 */          /*                                   */
/* MW 2004-07-28 TODO: Move to dlp_inst.h?                          */          /*                                   */
#define IDT_CLASS        1                                                      /* Class                             */
#define IDT_INSTANCE     2                                                      /* Instance                          */
#define IDT_ERROR        3                                                      /* Error                             */
#define IDT_FIELD        4                                                      /* Field                             */
#define IDT_OPTION       5                                                      /* Option                            */
#define IDT_METHOD       6                                                      /* Method                            */
#define IVIR_BADSTRING   -1                                                     /* Bad arguments                     */
#define IVIR_BADTYPE     -2                                                     /* Unknown string type               */
#define IVIR_BADCHAR     -3                                                     /* Contains bad charcter(s)          */
#define IVIR_BADLEAD     -4                                                     /* Begins with bad character(s)      */

/* Defines - Type arguments to dlp_log(...) */                                  /* --------------------------------- */
#define LOG_IN            1                                                     /* Log of stdin                      */
#define LOG_OUT           2                                                     /* Log of stdout                     */
#define LOG_ERR           3                                                     /* Log of stderr                     */

/* Variable type definitions*/

#ifdef _MSC_VER
  typedef __int16          INT16;
  typedef __int32          INT32;
  typedef unsigned __int16 UINT16;
  typedef unsigned __int32 UINT32;
#endif

/* String macros */
#define CHARUPR(A) if(A>='a' && A<='z') A-='a'-'A';
#define CHARLWR(A) if(A>='A' && A<='Z') A+='a'-'A';
#define SCSTR(A)   A?A:"(null)"

/* Arithmetic macros */
#ifdef MAX
  #undef MAX
#endif
#ifdef MIN
  #undef MIN
#endif
#ifdef ABS
  #undef ABS
#endif
#define MAX(A,B) ((A)>(B)?(A):(B))
#define MIN(A,B) ((A)<(B)?(A):(B))
#define ABS(A)   (((A)<0)?-(A):(A))
#define EPS      1.0e-50

/* Function wrapper macros */
#define printf __dlp_printf

#ifdef __OPTIMIZE_TRIG
  #define sin dlm_sinus
  #define cos dlm_cosinus
#endif

#ifdef __NOXALLOC
  #define dlp_calloc(A,B)    calloc(A,B)
  #define dlp_malloc(A)      malloc(A)
  #define dlp_realloc(A,B,C) realloc(A,(B)*(C))
  #define dlp_free(A)        {free(A); A=NULL;}

#else

#ifdef __DLP_KERNELSCOPE
  #define dlp_calloc(A,B)    __dlp_calloc(A,B,__FILE__,__LINE__,"kernel",NULL)
  #define dlp_malloc(A)      __dlp_malloc(A,__FILE__,__LINE__,"kernel",NULL)
  #define dlp_realloc(A,B,C) __dlp_realloc(A,B,C,__FILE__,__LINE__,"kernel",NULL)
  #define dlp_free(A)        {__dlp_free(A); A=NULL;}

#else
  #define dlp_calloc(A,B)    __dlp_calloc(A,B,__FILE__,__LINE__,BASEINST(_this)->m_lpClassName,CDlpObject_GetFQName(BASEINST(_this),dlp_get_a_buffer(),FALSE))
  #define dlp_malloc(A)      __dlp_malloc(A,__FILE__,__LINE__,BASEINST(_this)->m_lpClassName,CDlpObject_GetFQName(BASEINST(_this),dlp_get_a_buffer(),FALSE))
  #define dlp_realloc(A,B,C) __dlp_realloc(A,B,C,__FILE__,__LINE__,BASEINST(_this)->m_lpClassName,CDlpObject_GetFQName(BASEINST(_this),dlp_get_a_buffer(),FALSE))
  #define dlp_free(A)        {__dlp_free(A); A=NULL;}
#endif

#endif

#if !defined __MSOS && !defined __MINGW32__
  #define stricmp(A,B)    strcasecmp(A,B)
  #define strnicmp(A,B,N) strncasecmp(A,B,N)
#endif

#ifdef __TMS
  #define unlink(A)    remove(A)
#endif

#ifdef __MSVC
  #define stricmp(A,B) _stricmp(A,B)
  #define strnicmp(A,B,N) _strnicmp(A,B,N)
  #define getcwd(A,B)  _getcwd(A,B)
  #define chdir        _chdir
  #define mkdir        _mkdir
  #define sleep(A)     _sleep(A)
  #define isnan(A)     _isnan(A)
  #define snprintf     _snprintf
  #define random()     rand()
  #define log2(A)      (log(A)/log(2.))
#endif

#if (defined __MSOS && defined GetClassInfo)
  #undef GetClassInfo

#endif

#ifdef __SPARC
  #define memmove dlp_memmove

#endif

/* dLabPro assertions */
#define FMSG(A) FALSE

#ifdef _DEBUG

  #ifdef __MSOS
    #define DLPASSERT(A)                                 \
      if (!(A)) switch(dlp_assert(#A,__FILE__,__LINE__)) \
      {                                                  \
        case 1: exit(-1);                                \
        case 2: __asm {int 3}                            \
      };

  #else
    #define DLPASSERT(A)                                 \
      if (!(A)) switch(dlp_assert(#A,__FILE__,__LINE__)) \
      {                                                  \
        case 1: exit(-1);                                \
        case 2: abort();                                 \
      }
  #endif

#else /* #ifdef _DEBUG */
  #define DLPASSERT(A) if (A) {}

#endif /* #ifdef _DEBUG */

/* MSVC Debug Heap */
#if (defined __MSVC && defined _DEBUG)
  #define DLP_CHECK_MEMINTEGRITY if (dlp_xalloc_flags() & XA_HEAP_INTEGRITY) { DLPASSERT(_CrtCheckMemory());      }
  #define DLP_CHECK_MEMLEAKS     if (dlp_xalloc_flags() & XA_HEAP_MEMLEAKS ) { DLPASSERT(!_CrtDumpMemoryLeaks()); }

#else /* #if (defined __MSVC && defined _DEBUG) */
  #define DLP_CHECK_MEMINTEGRITY
  #define DLP_CHECK_MEMLEAKS

#endif /* #if (defined __MSVC && defined _DEBUG) */

/* Debug message macros */
#ifdef _DEBUG
  #ifdef __DLP_KERNELSCOPE
    #define DEBUGMSG(A,B,C,D,E)                 \
      if (A>=dlp_get_kernel_debug())            \
        dlp_message(__FILE__,__LINE__,B,C,D,E); \

  #else /* #ifdef __DLP_KERNELSCOPE */
    #define DEBUGMSG(A,B,C,D,E)                                               \
      if (A>=dlp_get_kernel_debug() || (A>0 && BASEINST(_this)->m_nCheck>=A)) \
        dlp_message(__FILE__,__LINE__,B,C,D,E); \

  #endif /* #ifdef __DLP_KERNELSCOPE */

#else /* #ifdef _DEBUG */
  #define DEBUGMSG(A,B,C,D,E)

#endif /* #ifdef _DEBUG */

#if defined __NORTTI && ! defined __NOITP
  #define __NOITP
#endif /* #if defined __NORTTI && ! defined __NOITP */

/* XAlloc heap management */
typedef struct
{
  const void* lpMemblock;
  char        nType;
  size_t      nNum;
  size_t      nSize;
  char        lpsFilename[L_PATH];
  INT64        nLine;
  char        lpsClassname[L_NAMES];
  char        lpsInstancename[4*L_NAMES];
} alloclist_t;

/* File pointer */
typedef struct zlib_file {
  void *m_lpFile;
  char m_nCompressed;
} DLP_FILE;


/**
 * This structure holds the description of generalized operations
 *
 * Signature declares input and output type:
 *   - Upper letters -> matrix
 *   - Lower letters -> scalar
 *   - o             -> operation,
 *   - R,r           -> real matrix, scalar,
 *   - N,n           -> real integer matrix, scalar
 *   - C,c           -> complex matrix, scalar,
 *   - D,d           -> input: auto-determine numeric types,
 *   - s             -> string
 *   - b             -> boolean
 *                   -> output depends on input.
 */
typedef struct
{
  INT16       opc;                                                              /* Operation code                    */
  INT16       res;                                                              /* Number of results                 */
  INT16       ops;                                                              /* Number of operands                */
  const char* sig;                                                              /* Signature                         */
  const char* nam;                                                              /* Name of operation                 */
  const char* sym;                                                              /* Symbol of operation               */
} opcode_table;


/* Defines - For expat parser ... --> */
#define XPT_VERSION        "expat_1.95.1"
#ifdef _WINDOWS
  #define XML_NS             1
  #define XML_DTD            1
  #define XML_BYTE_ORDER     12
  #define XML_CONTEXT_BYTES  1024
#elif defined __LINUX
  #define HAVE_MMAP 1
  #define STDC_HEADERS 1
  #define HAVE_BCOPY 1
  #define HAVE_GETPAGESIZE 1
  #define HAVE_MEMMOVE 1
  #define HAVE_FCNTL_H 1
  #define HAVE_UNISTD_H 1
  #define XML_NS
  #define XML_DTD
  #ifdef WORDS_BIGENDIAN
    #define XML_BYTE_ORDER 21
  #else
    #define XML_BYTE_ORDER 12
  #endif
  #define XML_CONTEXT_BYTES 1024
  #ifndef HAVE_MEMMOVE
    #ifdef HAVE_BCOPY
    #define memmove(d,s,l) bcopy((s),(d),(l))
    #else
    #define memmove(d,s,l) ;punting on memmove;
    #endif
  #endif
#elif defined __SPARC
  #define HAVE_MMAP 1
  #define STDC_HEADERS 1
  #define WORDS_BIGENDIAN 1
  #define HAVE_BCOPY 1
  #define HAVE_GETPAGESIZE 1
  #define HAVE_MEMMOVE 1
  #define HAVE_FCNTL_H 1
  #define HAVE_UNISTD_H 1
  #define XML_NS
  #define XML_DTD
  #ifdef WORDS_BIGENDIAN
    #define XML_BYTE_ORDER 21
  #else
    #define XML_BYTE_ORDER 12
  #endif
  #define XML_CONTEXT_BYTES 1024
  #ifndef HAVE_MEMMOVE
    #ifdef HAVE_BCOPY
      #define memmove(d,s,l) bcopy((s),(d),(l))
    #else
      #define memmove(d,s,l) ;punting on memmove;
    #endif
  #endif
#elif defined __ALPHA
  #define BYTEORDER 1234
  #define HAVE_BCOPY 1
  #define HAVE_DLFCN_H 1
  #define HAVE_FCNTL_H 1
  #define HAVE_GETPAGESIZE 1
  #define HAVE_MEMMOVE 1
  #define HAVE_MEMORY_H 1
  #define HAVE_MMAP 1
  #define HAVE_STDLIB_H 1
  #define HAVE_STRINGS_H 1
  #define HAVE_STRING_H 1
  #define HAVE_SYS_STAT_H 1
  #define HAVE_SYS_TYPES_H 1
  #define HAVE_UNISTD_H 1
  #define PACKAGE_BUGREPORT "expat-bugs@mail.libexpat.org"
  #define PACKAGE_NAME "expat"
  #define PACKAGE_STRING "expat 1.95.6"
  #define PACKAGE_TARNAME "expat"
  #define PACKAGE_VERSION "1.95.6"
  #define STDC_HEADERS 1
  #define XML_CONTEXT_BYTES 1024
  #define XML_DTD 1
  #define XML_NS 1
#elif defined __GNUC
  #define XML_NS             1
  #define XML_DTD            1
  #define XML_BYTE_ORDER     12
  #define XML_CONTEXT_BYTES  1024
#else
  #error "Unknown platform."
#endif

/* <-- */

/* Functions */
#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

/* Functions - dlp_arith.c */
void          dlp_arith_cleanup();
FLOAT64       dlp_scalop(FLOAT64 nParam1, FLOAT64 nParam2, INT16 nOpcode);
COMPLEX64     dlp_scalopC(COMPLEX64 nParam1, COMPLEX64 nParam2, INT16 nOpcode);
COMPLEX64     dlp_scalopC3(COMPLEX64 nParam1, COMPLEX64 nParam2, COMPLEX64 nParam3, INT16 nOpcode);
FLOAT32       dlp_scalopF(FLOAT32 nParam1, FLOAT32 nParam2, INT16 nOpcode);
INT16         dlp_aggrop(const FLOAT64* lpVec, FLOAT64* lpMask, FLOAT64 nParam, INT32 nDim, INT32 nFirst, INT32 nOffs, INT16 nOpcode, FLOAT64* lpnResult);
INT16         dlp_aggropC(const COMPLEX64* lpVec, COMPLEX64* lpMask, COMPLEX64 nParam, INT32 nDim, INT32 nFirst, INT32 nOffs, INT16 nOpcode, COMPLEX64* lpnResult);

/* Functios - dlp_optab.c */
INT16               dlp_op_opstype(const char* lpSignature, INT16 nOp);
INT16               dlp_constant_code(const char* lpsOpname);
COMPLEX64           dlp_constant(INT16 nOpcode);
void                dlp_constant_printtab();
const opcode_table* dlp_scalop_entry(INT16 nEntry);
const char*         dlp_scalop_name(INT16 nOpcode);
const char*         dlp_scalop_sym(INT16 nOpcode);
INT16               dlp_scalop_code(const char* lpsOpname);
const char*         dlp_aggrop_name(INT16 nOpcode);
INT16               dlp_aggrop_code(const char* lpsOpSymbol);
const char*         dlp_strop_name(INT16 nOpcode);
INT16               dlp_strop_code(const char* lpsOpSymbol);
const opcode_table* dlp_sigop_entry(INT16 nEntry);
const char*         dlp_sigop_name(INT16 nOpcode);
const char*         dlp_sigop_sym(INT16 nOpcode);
INT16               dlp_sigop_code(const char* lpsOpname);
INT16               dlp_sigop_ops(INT16 nOpcode);
void                dlp_scalop_printtab();
void                dlp_aggrop_printtab();
void                dlp_strop_printtab();
INT16               dlp_is_pointer_op_code(INT16 nOpCode);
INT16               dlp_is_logic_op_code(INT16 nOpCode);

/* Functions - dlp_assert.c */
INT16         dlp_assert(const char* lpExpression, const char* lpFilename, INT32 nLine);
#ifndef __MSOS
void          dlp_signal_handler(INT32 nSignal);
#endif /* #ifndef __MSOS */

/* Functions - dlp_heap.c */
BOOL          dlp_xalloc_init(INT32 nFlags);
void          dlp_xalloc_done(BOOL bCleanup);
void*         __dlp_calloc(size_t nNum, size_t nSize, const char* lpsFilename, INT32 nLine, const char* lpsClassname, const char* lpsInstancename);
void*         __dlp_malloc(size_t nSize, const char* lpsFilename, INT32 nLine, const char* lpsClassname, const char* lpsInstancename);
void*         __dlp_realloc(void* lpMemblock, size_t nNum, size_t nSize, const char* lpsFilename, INT32 nLine, const char* lpsClassname, const char* lpsInstancename);
void          __dlp_free(void* lpMemblock);
void          dlp_xalloc_register_object(char nType, const void* lpMemblock, size_t nNum, size_t nSize, const char* lpsFilename, INT32 nLine, const char* lpsClassname, const char* lpsInstancename);
void          dlp_xalloc_unregister_object(const void* lpMemblock);
BOOL          dlp_xalloc_find_object(const void* lpMemblock);
alloclist_t*  dlp_xalloc_find_object_ex(const void* lpMemblock);
size_t        dlp_size(const void* lpMemblock);
BOOL          dlp_in_xalloc(const void* lpMemblock);
INT32         dlp_xalloc_flags();
void          dlp_xalloc_print();

/* Functions - dlp_io.c */
void          dlp_chgwinsz(void);
void          dlp_message(const char* lpsFilename, INT32 nLine, const char* lpsFormat, ...);
void          dlp_error_message(const char* lpsFilename, INT32 nLine, const char* lpsFormat, ...);
INT32         __dlp_printf(const char* lpsFormat, ...);
void          dlp_init_printstop();
INT16         dlp_get_nonstop_mode();
INT16         dlp_set_nonstop_mode(INT16 nMode);
void          dlp_set_pipe_mode(BOOL bMode);
void          dlp_set_color_mode(BOOL bMode);
INT32         dlp_inc_printlines(INT32 nInc);
BOOL          dlp_if_printstop();
INT32         dlp_printstop_ni(INT32 nItem);
INT32         dlp_printstop_nix(INT32 nItem, const char* lpNext, char* lpAnswer);
INT16         dlp_maxprintcols();
INT16         dlp_maxprintlines();
INT16         dlp_printx(void* lpDest, void* lpBuffer, INT16 nType, INT32 nArrIdx, BOOL bCols, BOOL bStr);
INT16         dlp_printx_ext(void* lpDest, void* lpBuffer, INT16 nType, INT32 nArrIdx, BOOL bCols, BOOL bStr, BOOL bExact);
INT16         dlp_getx(INT16 nType, void* lpDest);
INT16         dlp_sscanc(const char* lpsStr, COMPLEX64* lpnDst);
INT16         dlp_sscanx(const char* lpsStr, INT16 nType, void* lpsDst);
void          dlp_fprint_x_line(FILE* lpFout, char c, INT16 n);
INT32         dlp_printlen(INT32 nType);
INT16         dlp_sprintc(char* lpsDst, COMPLEX64 what, BOOL bExact);
INT32         dlp_sprintx(char* lpsBuffer, char* nWhat, INT32 nType, BOOL bExact);
void          dlp_puts_ex(INT16 nArgs, ...);
char*         dlp_fgetl(char* lpsBuffer, INT16 nBufferLen, FILE* lpfIn, INT32* nLines);
void          dlp_openLogFile(const char* lpsLogFileName);
void          dlp_closeLogFile();
void          dlp_log(INT16 nType, const char* lpsFormat, ...);

/* Functions - dlp_session.c */
void          dlp_set_kernel_debug(INT16 nLevel);
INT16         dlp_get_kernel_debug();
void          dlp_set_binary_name(const char* lpName);
const char*   dlp_get_binary_name();
void          dlp_set_binary_path(const char* lpName);
const char*   dlp_get_binary_path();
INT16         dlp_scancmdlineoption(INT32* argc, char** argv, const char* lpOption, const char* lpDelimiter, char* lpValue, BOOL bRemove);
INT32         dlp_system(char* lpsCommand);
const char*   dlp_get_version_info();
void          dlp_set_retval(INT32 nVal);
INT32         dlp_get_retval();
void          dlp_rand_fix(BOOL bFix);
UINT64        dlp_rand();
FLOAT64       dlp_frand();
void          dlp_sleep(INT32 nMilliSec);
void          dlp_register_signals();
void          dlp_interrupt(int nSig);
void          dlp_set_interrupt(BOOL bRequestInterrupt);
BOOL          dlp_get_interrupt();

/* Functions - dlp_sort.c */
void dlpsort(void *base, size_t nmemb, size_t size, int(*compar)(const void *, const void *));

/* Functions - dlp_string.c */
char*         dlp_get_a_buffer();
void*         dlp_memmove(void* lpDst, const void* lpSrc, size_t count);
void*         dlp_memset(void* lpDst, INT32 nVal, size_t nCount);
BOOL          dlp_charin(char nChr, const char* sSet);
size_t        dlp_strlen(const char *lpsStr);
INT32         dlp_strcnt(const char* lpsStr, const char nChr);
char*         dlp_strcpy(char *lpsDst, const char *lpsSrc);
char*         dlp_strncpy(char *lpsDst, const char *lpsSrc, size_t nCount);
char*         dlp_strabbrv(char* lpsDst, const char* lpsSrc, size_t nCount);
INT32         dlp_strcmp(const char* lpsStr1, const char* lpsStr2);
INT32         dlp_stricmp(const char* lpsStr1, const char* lpsStr2);
INT32         dlp_strnicmp(const char* lpsStr1, const char* lpsStr2, size_t nCount);
INT32         dlp_strpcmp(const char* lpsStr1, const char* lpsStr2);
INT32         dlp_strncmp(const char* lpsStr1, const char* lpsStr2, size_t nCount);
char*         dlp_strcat(char* lpsStr1, const char* lpsStr2);
char*         dlp_strlwr(char* lpsStr);
char*         dlp_strupr(char* lpsStr);
char*         dlp_strquotate(char* lpsStr, char nLeft, char nRight);
char*         dlp_strunquotate(char* lpsStr, char nLeft, char nRight);
char*         dlp_strtrimleft(char* lpsStr);
char*         dlp_strtrimright(char* lpsStr);
void          dlp_splitpath(const char* lpsFQP, char* lpsPath, char* lpsFile);
char*         dlp_strduplicate(const char* lpsStr);
hash_val_t    dlp_strhash(const void* lpKey);
char*         dlp_strconvert(INT16 nHow, char* lpsStr1, const char* lpsStr2);
INT16         dlp_strreplace_ex(char* lpsStr, const char* lpsKey, const char* lpsRpl, BOOL bOnce);
INT16         dlp_strreplace(char* lpsText, const char* lpsKey, const char* lpsRpl);
INT16         dlp_strreplace_env(char* lpsStr, BOOL bOnce);
char*         dlp_strsep(char** stringp, const char* delim, char* del);
INT16         dlp_strstartswith(const char* lpsStr, const char* lpsOther);
INT16         dlp_strendswith(const char* lpsStr, const char* lpsOther);
FLOAT64       dlp_strtod(const char* lpsStr, char** lpsEndPtr);

/* Functions - dlp_type.c */
void          dlp_type_printtab();
const char*   dlp_get_type_name(INT16 nTypeCode);
const char*   dlp_get_c_type(INT16 nTypeCode);
INT16         dlp_get_type_code(const char* lpsTypeName);
INT16         dlp_get_type_size(INT16 nTypeCode);
INT16         dlp_is_valid_type_code(INT16 nTypeCode);
INT16         dlp_is_numeric_type_code(INT16 nTypeCode);
INT16         dlp_is_integer_type_code(INT16 nTypeCode);
INT16         dlp_is_float_type_code(INT16 nTypeCode);
INT16         dlp_is_complex_type_code(INT16 nTypeCode);
INT16         dlp_is_symbolic_type_code(INT16 nTypeCode);
INT16         dlp_is_pointer_type_code(INT16 nTypeCode);
INT16         dlp_store(COMPLEX64 nVal, void* lpBuffer, INT16 nTypeCode);
COMPLEX64     dlp_fetch(const void* lpBuffer, INT16 nTypeCode);
INT32         dlp_isnan(FLOAT64 nVal);
INT16         dlp_finite(FLOAT64 nVal);

/* Functions - dlp_ncnv.c */
char*         dlp_convert_name(INT16 nHow, char* lpsStr1, const char* lpsStr2);
INT16         dlp_is_valid_id(INT16 nHow, const char* lpsStr, char* lpsFaulty);
char*         dlp_errorcode2id(char* lpsStr1, INT16 nErrorcode);

/* Functions - dlp_thread.c */
INT32         dlp_getpid();
THREADHANDLE  dlp_create_thread(THREADFUNC, void* lpThreadArg);
INT16         dlp_join_thread(THREADHANDLE);
INT16         dlp_terminate_thread(THREADHANDLE, INT32 nExitCode);
UINT64        dlp_time();

/* Functions - dlp_file.c */
DLP_FILE*     dlp_fopen(const char *path,const char *mode);
INT32         dlp_fclose(DLP_FILE *lpZF);
size_t        dlp_fread(void *ptr,size_t size,size_t nmemb,DLP_FILE *lpZF);
size_t        dlp_fwrite(const void *ptr, size_t size, size_t nmemb, DLP_FILE *lpZF);
int           dlp_ferror(DLP_FILE *lpZF);
INT32         dlp_feof(DLP_FILE *lpZF);
INT32         dlp_fprintf(DLP_FILE *lpZF,const char *format, ...);
BOOL          dlp_fzip(const char *lpsInfile, const char *lpsMode);
BOOL          dlp_funzip(const char *lpsInfile,char const ** lpsOutfile);
INT16         dlp_chdir(const char* lpsDirname, BOOL bCreate);
BOOL          dlp_mkdirs(const char* lpsDirname);
char*         dlp_tempnam(const char* lpsDir, const char* lpsPfx);
char*         dlp_fullpath(char* lpsAbsPath, const char* lpsRelPath, INT32 nMaxLen);

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif /* if !defined __DLPBASE_H */

/* EOF */
