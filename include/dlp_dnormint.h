/* dLabPro DNorm3 library
 * - DNorm 3.1 library
 *
 * AUTHOR : Torsten Rudolph
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

/* ===========================================================================

   NAME:  dnormint.h  -> "Dnorm3.1 internal global header file"

   DESCRIPTION:

   HINTS:

   PORTABILITY:

     - Microsoft C 6.0
  - TURBO C++ v1.0
  - Borland C++ fuer Windows 3.1
  - DECUS C (PDP 11)
     - DEC VAX C
     - DEC ULTRIX RISC C
     - DEC OSF C (ALPHA)
     - SUN SPARC GNU C

   AUTHOR:  t.rudolph

   COPYRIGHT:  copyright (c) 1994 by tu dresden/ita ag sprachkommunikation

   UPDATE:  16.02.94

   ===========================================================================
*/

#ifndef __DNORMINT_H__
#define __DNORMINT_H__

#ifdef _DEBUG
  /* /////////////////////////// */
  /* use DLP memory management   */
  /* /////////////////////////// */
  /*#define DLPALLOC_EXTERN*/
#endif

/* ----- Includes -----
*/

#include "dlp_dnorm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#if !defined RC_INVOKED
  #include <limits.h>      /* Ressource Compiler will's nicht!  */
#endif
#include <float.h>
#include <ctype.h>


/* ----- Compilertypen -----
*/
#if defined MSDOS
  #define _MSC_      /* Microsoft C 6.0 or higher under MS-DOS  */
#elif defined __TURBOC__ && !defined _Windows
  #define _TC_      /* Borl. TURBO C v2.0 and higher under MS-DOS  */
#elif (defined __BORLANDC__ && defined _Windows)
  #define _BCWIN_      /* Borland C++ 3.1 fuer Windows oder 4.5 fuer Win95      */
  #if defined WIN32
    #define _BCW95_    /* zusaetzlich in Win95 definert */
  #else
    #define _BCW31_    /* zusaetzlich in Win3.1 definert */
  #endif
#elif ((defined __MICROSOFT__ && defined _WINDOWS) || defined _WIN32)
  #define _MSCWIN_  /* Microsoft Visual C++ fuer Windows  */
  #if defined WIN32
    #define _MSCW95_  /* zusaetzlich in Win95 definert */
  #else
    #define _MSCW31_  /* zusaetzlich in Win3.1 definert */
  #endif
#elif defined __MSVC
  #define _MSCWIN_  /* Microsoft Visual C++ fuer Windows  */
  #define _MSCW95_  /* zusaetzlich in Win95 definert */
#elif defined rsx
  #define _DECUSC_    /* DECUS C on DEC PDP-11 midframes    */
#elif defined vax && !defined ultrix
  #define _VAXC_      /* DEC VAX C on DEC VAX-11 workstations    */
#elif defined ultrix
  #define _RISCC_      /* DEC ULTRIX RISC C on DEC Risc workstations  */
#elif defined __GNUC__ && defined __osf__

  #define _GNUC_      /* GNU compiler on alpha workstations  */

#elif defined __osf__

  #define _OSFC_      /* DEC OSF/1 compiler on alpha workstations  */

#elif defined __GNUC__ && defined __sparc
  #define _GNUC_      /* Gnu-C compiler on linux PC       */
  #define _SUNGC_      /* Gnu-C compiler on Sun Sparc Workstation   */
#elif defined __GNUC__ && defined __linux
/*#undef   _SUNGC_      / * Who defines this symbol ?                   */
  #define  _GNUC_      /* Gnu-C compiler on linux PC       */
#elif defined __GNUC__
  #define  _GNUC_      /* Gnu-C compiler on Windows PC       */
#elif defined _TMS320C6X
  #define  _GNUC_
#else
  #error Unbekannter Compiler
#endif

/* ---- Compilertyp-abhaengige Definitonen ---------
*/
/* --- Microsoft C ---
*/
#if defined _MSC_
  #define RECHNERTYP IBM_PC
  #include <io.h>
  #include <malloc.h>
  #include <memory.h>
  #include <stdarg.h>

  #define movmem( p1, p2, p3 )  memmove( p2, p1, p3 )
  #define setmem( p1, p2, p3 )  memset( p1, p3, p2 )
  #define _strlen    strlen
  #define _strcpy    strcpy
  #define _strncpy  strncpy
  #include <graph.h>
  #define gotoxy( p1, p2 )  _settextposition( p2, p1 )
  #define clrscr()    _clearscreen( _GCLEARSCREEN )
  #define PATH_DELIMETER '\\'
  #if defined( _USE_FARPOINTER_  )  /* fuer TSR-Prog.       */
    #define FAR far                         /* unter TURBO-C        */
  #else
    #define FAR
  #endif

  #if defined M_I86LM || defined M_I86HM \
                || defined M_I86CM    /* Microsoft-C    */
    #if _MSC_VER >= 600
      #define HUGE _huge
    #else
      #define HUGE huge
    #endif
  #else
    #define HUGE
  #endif
                            /* Matthias Eichner, 17.07.2001 */
/*#define MODUS_r "r+b"    / * Changed because read only files can not be read using 'r+' */
#define MODUS_r "rb"       /* Modus zum Fileoeffnen        */
#define MODUS_a "a+b"
#define MODUS_w "w+b"
#define MODUS_wr "w+b"            /* Lesen&Schr.(vorh.Inh.Loesch.)*/
#define MODUS_ra "a+b"      /* Lesen und schreiben ans Ende */
#define MODUS_rw "r+b"            /* Lesen&Schr. Datei muss exist.*/
#define MODUS "r+b"      /* Modus zum Fileoeffnen:Binaerf*/
/* --- Turbo C ---
*/
#elif defined _TC_
#define RECHNERTYP IBM_PC
#include <io.h>
#include <alloc.h>
#include <mem.h>
#include <stdarg.h>
#define PATH_DELIMETER '\\'
#if defined( _USE_FARPOINTER_  )    /* fuer TSR-Prog.       */
#define FAR far                           /* unter TURBO-C        */
#else
#define FAR
#endif
#if defined __LARGE__ || defined __HUGE__ \
          || defined __COMPACT__  /* tur der verwendeten  */
#define HUGE huge        /* FAR-Zeiger in den    */
#else            /* Modellen LARGE und   */
#define HUGE          /* HUGE, ...            */
#endif
                            /* Matthias Eichner, 17.07.2001 */
/*#define MODUS_r "r+b"    / * Changed because read only files can not be read using 'r+' */
#define MODUS_r "rb"       /* Modus zum Fileoeffnen        */
#define MODUS_a "a+b"
#define MODUS_w "w+b"
#define MODUS_wr "w+b"            /* Lesen&Schr.(vorh.Inh.Loesch.)*/
#define MODUS_ra "a+b"      /* Lesen und schreiben ans Ende */
#define MODUS_rw "r+b"            /* Lesen&Schr. Datei muss exist.*/
#define MODUS "r+b"      /* Modus zum Fileoeffnen:Binaerf*/
/* --- Borland C win 3.1 ---
*/
#elif defined _BCWIN_
#define RECHNERTYP IBM_PC
#include <io.h>
#include <alloc.h>
#include <mem.h>
#include <stdarg.h>
#define cfree free
#define PATH_DELIMETER '\\'
#if defined __LARGE__ || defined __HUGE__ \
          || defined __COMPACT__  /* tur der verwendeten  */
#define HUGE huge        /* FAR-Zeiger in den    */
#else            /* Modellen LARGE und   */
#define HUGE          /* HUGE, ...            */
#endif
                            /* Matthias Eichner, 17.07.2001 */
/*#define MODUS_r "r+b"    / * Changed because read only files can not be read using 'r+' */
#define MODUS_r "rb"       /* Modus zum Fileoeffnen        */
#define MODUS_a "a+b"
#define MODUS_w "w+b"
#define MODUS_wr "w+b"            /* Lesen&Schr.(vorh.Inh.Loesch.)*/
#define MODUS_ra "a+b"      /* Lesen und schreiben ans Ende */
#define MODUS_rw "r+b"            /* Lesen&Schr. Datei muss exist.*/
#define MODUS "r+b"      /* Modus zum Fileoeffnen:Binaerf*/

/* --- Microsoft Visual C fuer Windows ---
*/
#elif defined _MSCWIN_
#define RECHNERTYP IBM_PC
#include <io.h>
#include <malloc.h>
#include <stdarg.h>

#define cfree free
#define movmem( p1, p2, p3 )  memmove( p2, p1, p3 )
#define setmem( p1, p2, p3 )  memset( p1, p3, p2 )
#define _fstrlen(p1)    strlen(p1)
#define _fstrcpy(p1,p2)    strcpy(p1,p2)
#define _fstrncpy(p1,p2,p3)  strncpy(p1,p2,p3)
#define PATH_DELIMETER '\\'
#if defined __LARGE__ || defined __HUGE__ \
          || defined __COMPACT__  /* tur der verwendeten  */
#define HUGE huge        /* FAR-Zeiger in den    */
#else            /* Modellen LARGE und   */
#define HUGE          /* HUGE, ...            */
#endif
                            /* Matthias Eichner, 17.07.2001 */
/*#define MODUS_r "r+b"    / * Changed because read only files can not be read using 'r+' */
#define MODUS_r "rb"       /* Modus zum Fileoeffnen        */
#define MODUS_a "a+b"
#define MODUS_w "w+b"
#define MODUS_wr "w+b"            /* Lesen&Schr.(vorh.Inh.Loesch.)*/
#define MODUS_ra "a+b"      /* Lesen und schreiben ans Ende */
#define MODUS_rw "r+b"            /* Lesen&Schr. Datei muss exist.*/
#define MODUS "r+b"      /* Modus zum Fileoeffnen:Binaerf*/
#pragma warning( disable : 4244 )

/* --- DEC DECUS C (PDP11) ---
*/
#elif defined _DECUSC_
#define RECHNERTYP DEC_PDP11
#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>
#define movmem( p1, p2, p3 )  memmove( p2, p1, p3 )
#define setmem( p1, p2, p3 )  memset( p1, p3, p2 )
#define PATH_DELIMETER
#define FAR
#define MODUS_r "rn"
#define MODUS_a "an"
#define MODUS_w "wn"
#define MODUS_rw "rwn"
#define MODUS_wr "rwn"
#define MODUS "rn"
#define const      /* DECUS C kann const-Modifizierer nicht! */
/* --- DEC VAX C ---
*/
#elif defined _VAXC_
#define RECHNERTYP DEC_VAX
#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>
#define movmem( p1, p2, p3 )  memmove( p2, p1, p3 )
#define setmem( p1, p2, p3 )  memset( p1, p3, p2 )
#define PATH_DELIMETER
#define FAR
#define MODUS_r "rb"
#define MODUS_a "ab"
#define MODUS_w "wb"
#define MODUS_rw "rwb"
#define MODUS_wr "wrb"
#define MODUS "rb"
/* --- DEC RISC C ---
*/
#elif defined _RISCC_
#define RECHNERTYP DEC_RISC
#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>
#define movmem( p1, p2, p3 )  memmove( p2, p1, p3 )
#define setmem( p1, p2, p3 )  memset( p1, p3, p2 )
/* folgende Massnahme ist notwendig, da unter RISCC/OSFC in math.h die
   Konstante HUGE (unverstaendlicherweise oeffentlich!) mit
   #define  HUGE_VAL  1.8e+308
   #if !defined(_POSIX_SOURCE)
   #define HUGE HUGE_VAL
   definiert wird!
*/
#include <math.h>
#undef HUGE      /* freimachen fuer folgende HUGE-Defin.*/
#define HUGE
#define PATH_DELIMETER '/'
#define FAR
#define MODUS_r "rb"
#define MODUS_a "ab"
#define MODUS_w "wb"
#define MODUS_rw "r+b"
#define MODUS_wr "w+b"
#define MODUS "rb"
#ifdef const
#undef const
#endif
#define const      /* RISC C kann const-Modifizierer nicht! */
/* --- DEC OSF C (Alpha) ---
*/
#elif defined _OSFC_
#define RECHNERTYP DEC_ALPHA
#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>
#define movmem( p1, p2, p3 )  memmove( p2, p1, p3 )
#define setmem( p1, p2, p3 )  memset( p1, p3, p2 )
/* begruendung der folgenden Massnahme siehe RISCC!
*/
#include <math.h>
#undef HUGE      /* freimachen fuer folgende HUGE-Defin.  */
#define HUGE
#define PATH_DELIMETER '/'
#define FAR

#ifdef long
#undef long
#endif
#define long int    /* long ist hier 8 bytes breit!    */
#undef LONG_MAX
#define LONG_MAX INT_MAX
#undef LONG_MIN
#define LONG_MIN INT_MIN
#undef ULONG_MAX
#define ULONG_MAX UINT_MAX
#if defined ULONG_MIN
#undef ULONG_MIN
#endif
#define ULONG_MIN 0

#define MODUS_r "rb"
#define MODUS_a "ab"
#define MODUS_w "wb"
#define MODUS_rw "r+b"
#define MODUS_wr "w+b"
#define MODUS "rb"
/* --- LINUX GNUC C ---
*/
#elif defined _GNUC_
#define RECHNERTYP IBM_PC
#ifndef _TMS320C6X
#  include <fcntl.h>
#endif
#include <stdlib.h>
#include <stdarg.h>
#define movmem( p1, p2, p3 )  memmove( p2, p1, p3 )
#define setmem( p1, p2, p3 )  memset( p1, p3, p2 )
/* folgende Massnahme ist notwendig, da unter RISCC/OSFC in math.h die
   Konstante HUGE (unverstaendlicherweise oeffentlich!) mit
   #define  HUGE_VAL  1.8e+308
   #if !defined(_POSIX_SOURCE)
   #define HUGE HUGE_VAL
   definiert wird!
*/
#include <math.h>
#undef HUGE      /* freimachen fuer folgende HUGE-Defin.*/
#define HUGE
#define PATH_DELIMETER '/'
#define FAR
#define MODUS_r "rb"
#define MODUS_a "ab"
#define MODUS_w "wb"
#define MODUS_rw "r+b"
#define MODUS_wr "w+b"
#define MODUS "rb"
/* --- SUn SPARC GNUC C ---
*/
#elif defined _SUNGC_
#define RECHNERTYP SUN_SPARC
#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>
#define movmem( p1, p2, p3 )  memmove( p2, p1, p3 )
#define setmem( p1, p2, p3 )  memset( p1, p3, p2 )
/* folgende Massnahme ist notwendig, da unter RISCC/OSFC in math.h die
   Konstante HUGE (unverstaendlicherweise oeffentlich!) mit
   #define  HUGE_VAL  1.8e+308
   #if !defined(_POSIX_SOURCE)
   #define HUGE HUGE_VAL
   definiert wird!
*/
#include <math.h>
#undef HUGE      /* freimachen fuer folgende HUGE-Defin.*/
#define HUGE
#define PATH_DELIMETER '/'
#define FAR
#define MODUS_r "rb"
#define MODUS_a "ab"
#define MODUS_w "wb"
#define MODUS_rw "r+b"
#define MODUS_wr "w+b"
#define MODUS "rb"
/* --- sonstige Compiler...  ---
*/
#else
/*
#error Unbekannter Compiler!
*/
#endif


/* ----- sonstige Compiler-unabhaengige Definitionen ---
*/
#ifndef TRUE
#define   TRUE    (1==1)
#endif
#ifndef FALSE
#define   FALSE    (1==0)
#endif

#define   ON      TRUE
#define   OFF             FALSE

#define    YES    TRUE
#define    NO    FALSE


/* ---------------------------------------------------------------------------
   DNORM-spezifische Definitionen (user interface)
   ---------------------------------------------------------------------------
*/
/* in dnorm.h */


/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   start of internal DNORM definitions
   +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/* --- Standardfehlercodes ---
*/
#define OK_  TRUE

/*--- Limits fuer Dnorm Elemente, die uneingeschraenkt unterstuetzt werden ---
*/
/* max. Anzahl parrallel offener Dnorm-files
*/
#define DOPEN_MAX    FOPEN_MAX /2
/* max. Laenge einer Stringkomponente im Vektor: 16 kbyte
*/
#define MAX_STRING_KOMP         UCHAR_MAX

/* Konstanten fuer die Behandlung der Komponentenmaske
*/
#define KOMP_OFF 0      /* Komponente anzeigen          */
#define KOMP_ON 1      /* Komponente nicht anzeigen    */
#define KOMP_ALL 2      /* alle Komponenten anzeigen    */
#define KOMP_STOP 3      /* Ende des Komp.maskenfeldes   */

/*----------------------------------------------------------------------*/
#define BL_SIZE    128             /* kleinste schreibbare Einheit */
#define  DN_VERSION  3    /* Dateinorm-Versionsnummer     */
#define SETV_BUFSIZE  0x1000    /* 4kByte Filepuff.(f.setvbuf())*/

#define  SEARCH_KRN     0  /* suche knr/rnr entspr.Spez.  */
#define  SEARCH_NEXT    -1  /* naechste Klasse/Realis.   */
#define  SEARCH_NEXT_K    -2  /* naechste Klasse (Real.konst.)*/
#define  SEARCH_NEXT_R    -3  /* naechste Realis. (Kl.konst.) */
#define SEARCH_FIRST    -4  /* erste Kl./Realis. d. Datei   */
#define SEARCH_LAST    -5  /* letzte Kl./Realis. d. Datei  */
/*----------------------------------------------------------------------*/
typedef struct KR_NODE
{
   short     knr;  /* primaerer Suchschluessel: Knr   */
   short     rnr;    /* sekundaerer Suchschluessel: Real.nr. */

   t_i4 rba;      /* Knoteninfo: relative byte-Adresse   */
   struct KR_NODE *   left;
   struct KR_NODE *   right;
} KR_NODE;

typedef struct FNODE_INFO
{
   short   knr;
   short   rnr;
   t_i4         rba;
} FNODE_INFO;
/*----------------------------------------------------------------------*/
typedef struct DVB
{
   char   vbkennz[2];  /* Zwei byte Vorblockkennzeichen        */
   short        version;  /* Dnorm-Versionskennzeichnung    */
   short  blocks;    /* Vorblockgroesse in blocks       */
   short   comptype;  /* Rechner-(Hardware-)                */
   char    dtype[2];  /* Dateityp (z.B. 'X''V' )              */

   double       dres1;          /* insgesamt 5 Reserveeintraege fuer    */
   double       dres2;          /* nutzerindividuelle Dateibezogene      */
   double       dres3;          /* Informationen ----->               */
   double       dres4;
   double       dres5;          /*                             <----- */

} DVB;

typedef struct VDVB    /* variable Datei-Vorblockelemente   */
{
   char    xt_sign[2];     /* 'X''T'                             */
   short  lxt;    /* Lnge des Dateikenntextes in byte     */
   char *  xt;    /* Dateikenntext                       */

   char    vdt_sign[2];    /* 'V''D'                                */
   short  lvdt;    /* Lnge der Beschreibung in byte       */
   char *  vdt;    /* Str.beschr. d. Verbal.Real.Eigensch. */

} VDVB;
/*----------------------------------------------------------------------*/
typedef struct RVB
{
   short  vblocks;    /* Anzahl der Vorblock-blocks    */
   t_i4         rdblocks;    /* Anz. Realis.daten-blocks   */

   char    knr_sign[2];            /* 'K''N'                      */
   short         knr;      /* Klassennummer              */

   char    rnr_sign[2];            /* 'R''N'                   */
   short  rnr;      /* Realisierungsnummer           */

   char    vanz_sign[2];           /* 'V''A'                       */
   t_i4    vanz;      /* Anzahl der Realis.vektoren   */

   char    vdim_sign[2];           /* 'D''I'                        */
   t_i4         vdim;      /* Dimension der Vektoren      */

   char    vsize_sign[2];          /* 'V''S'                        */
   t_i4          vsize;      /* Groesse d.Vektoren in bytes  */

   char    zf_sign[2];             /* 'Z''F'                       */
   double  zf;      /* Zeitfensterfaktor          */

   char    fsr_sign[2];            /* 'F''S'                        */
   double  fsr;      /* Fortsetzrate                 */

   char    ofs_sign[2];            /* 'O''F'                        */
   double  ofs;                    /* Offset des Realis.beginns    */

   char    rres1_sign[2];          /* 'R''1'                       */
   double       rres1;                  /* insgesamt 5 Reserveeintr. f.  */
   char    rres2_sign[2];          /* 'R''2'                        */
   double       rres2;                  /* nutzerindivid. dateibezogene */
   char    rres3_sign[2];          /* 'R''3'                      */
   double       rres3;                  /* Informationen ----->       */
   char    rres4_sign[2];          /* 'R''4'                        */
   double       rres4;
   char    rres5_sign[2];          /* 'R''5'                       */
   double       rres5;                  /*                       <----- */

} RVB;


typedef struct VRVB      /* variable Realis.Vorblockelem.*/
{
   char    rb_sign[2];             /* 'R''B'                     */
   short  lrb;      /* Lnge Recordbeschr. in byte   */
   RB *    rb;      /* Recordbeschreibung            */

   char    rt_sign[2];             /* 'R''T'                       */
   short  lrt;      /* Lnge Realis.kenntext in byte  */
   char *  rt;      /* Realisierungskenntext         */

   char    dt_sign[2];             /* 'D''T'                       */
   short  ldt;      /* Lnge Datumstring in byte     */
   char *  dt;      /* Datum-String                 */

   char    ht_sign[2];             /* 'H''T'                        */
   short  lht;      /* Laenge "modif.hist." in byte */
   char *  ht;      /* "modification history text"   */

   char    vrt_sign[2];            /* 'V''R'                       */
   short  lvrt;      /* Lnge vr in byte              */
   char *  vrt;      /* verb. Beschr.d.Real.Eigensch.*/

} VRVB;
/*----------------------------------------------------------------------*/

/* --- Parameterbehandlungsmodi ---
*/
#define DNMODE_DEFAULT          0x0L
#define DNMODE_NO_CHANGE        0x1L
#define DNMODE_APPEND           0x2L

/* --- Typen ---
*/
#if defined ULONG
#undef ULONG
#endif
#if defined _OSFC_
#define ULONG      unsigned int
#else
#define ULONG      unsigned long
#endif

/*----------------------------------------------------------------------*/

/* --- Funktionsprototypen aus dopen.c ---
*/
void   dn_onexit( void );

/* --- Funktionsprototypen-Deklaration  Baum-Funktionen ---
*/
KR_NODE *   load_tree( FILE * fp, t_i4 * objects );
void      del_tree( KR_NODE ** rootpt );
KR_NODE *   search_tree( KR_NODE * root, short knr, short rnr, int mode );
KR_NODE *   add_tree( KR_NODE * root, short knr, short rnr, long rba );
void     save_tree( FILE * fp, KR_NODE * root, long objects,
         short offset );
/* --- Funktionsprototypen sonstiger Service-funktionen ---
*/
/* -- ...in dnget.c --
*/
#if defined _RISCC_ || defined _OSFC_ || defined _GNUC_ || defined _SUNGC_ || defined _MSCWIN_
size_t   read_rvb( RVB * rvb, FILE * fp );
size_t   write_rvb( RVB * rvb, FILE * fp );
#endif
/* -- ...in dnsvl.c --
*/
#if defined _SUNGC_
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#if !defined FILENAME_MAX
#define FILENAME_MAX 2048
#endif
#if !defined FOPEN_MAX
#define FOPEN_MAX 32
#endif
#if !defined M_PI
#define M_PI 3.14159
#endif
#if !defined M_E
#define M_E 2.71828
#endif

void * memmove( void *s1, const void *s2, size_t n );
void * memset( void *s, int c, size_t n );
/*int    atexit( void (* function)( void ) );*/

void   myswab( void * a, long size, long n );
void   myswab_data( void * dat, long vanz, DCB * stream );
size_t fread_myswab( void * pointer, size_t size, size_t num_items, FILE * stream );
size_t fwrite_myswab( void * pointer, size_t size, size_t num_items, FILE * stream );

#else
#define fread_myswab fread
#define fwrite_myswab fwrite
#endif

_DECLSPEC void * _EXPORT dn_malloc( long n );
_DECLSPEC void * _EXPORT dn_calloc( long n );
_DECLSPEC void    _EXPORT dn_free( void * pt );
_DECLSPEC char * _EXPORT dn_getfmtdate( void );


/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   end of internal DNORM definitions
   +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/


/* /////////////////////////// */
/* use DLP memory management   */
/* /////////////////////////// */
#ifdef _DEBUG
  #ifdef DLPALLOC_EXTERN
    #include "dlp_alloc_extern.h"
  #endif
  #define dn_malloc(A)      malloc(A)
  #define dn_calloc(A,B)    calloc(A,B)
  #define dn_realloc(A,B)   realloc(A,B)
  #define dn_free(A)        free(A)
#endif

#endif             /* defined __DNORMINT_H__  */

/* ===========================================================================
   END OF FILE dnormint.h
   ===========================================================================
*/
