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

   NAME:  dnorm.h    -> "Dnorm3.1 global header file"

   DESCRIPTION:

  Headerfile fuer alle Applikationen, die die Dnorm3.0-Basisfunktionen
  der Objektbibliothek 'dn3base.a' benutzen.

  Einschluss in die Applikationsquellen via #include <dnorm.h>

   HINTS:

  Diese Datei enthaelt insbesondere die Definiton der Parameterstruktur
  'DPARA', die als Kommunikationsstruktur zwischen Applikation und
  Basisfunktionen fungiert.

   PORTABILITY:  

     - Microsoft C 6.0
     - TURBO C++ v1.0
  - DECUS C (PDP 11)
     - DEC VAX C
     - DEC ULTRIX RISC C
     - DEC OSF C (ALPHA)

   AUTHOR:  t.rudolph

   COPYRIGHT:  copyright (c) 1994 by tu dresden/ita ag sprachkommunikation

   UPDATE:  16.02.94

   ===========================================================================
*/

#ifndef __DNORM_H__
#define __DNORM_H__


/* ---------------------------------------------------------------------------
   notwendige Sys-Includes
   ---------------------------------------------------------------------------
*/
#include <stdio.h>
#include <stdlib.h>


#if defined _Windows  /* _BCWIN_ */
  #include <windows.h>
  #define _DECLSPEC
  #if defined __DLL__
    #define _EXPORT _export
  #else
    #define _EXPORT
  #endif
#elif defined _WINDOWS /* _MSCWIN_ */
/*  #include <windows.h>*/
  #define _EXPORT
  #if defined __DLL__
    #define _DECLSPEC __declspec(dllexport)
  #else
    #define _DECLSPEC /*__declspec(dllimport)*/
  #endif
#else
  #define _DECLSPEC
  #define _EXPORT
#endif

/* ---------------------------------------------------------------------------
   Compilerabhaengige Definitionen
   ---------------------------------------------------------------------------
*/
#if defined ultrix
#define const
#endif

#if (defined __osf__ || defined __x86_64)

typedef int         t_i4;  /* long ist hier 8 bytes breit, int 4 bytes! */
typedef unsigned int  t_u4;
#else
typedef long          t_i4;
typedef unsigned long t_u4;
#endif


/* ---------------------------------------------------------------------------
   DNORM-spezifische Definitionen (user interface)
   ---------------------------------------------------------------------------
*/
/* ----- Rechnertypen -----
*/
#define IBM_PC      0
#define DEC_PDP11      1
#define DEC_VAX               2
#define DEC_RISC    3
#define DEC_ALPHA    4
#define SUN_SPARC    5
/* --- ... alias (alt) ---
*/
#define PDP11     DEC_PDP11
#define VAX     DEC_VAX
#define RISC_5000 DEC_RISC


/* --- Konstanten fuer Realiserungszugriff ueber knr/rnr
*/ 
#define NEXT       -1
#define FIRST       -2


/* --- Dnorm3.1 Fehlerbehandlung ---
*/
#define DNERR_OK     0

#define DNERR_SYSFOPEN    -2
#define DNERR_SYSSETVBUF  -3
#define DNERR_SYSTMPFOPEN  -4
#define DNERR_SYSREAD    -5
#define DNERR_SYSWRITE    -6

#define DNERR_BADFOPENMODE  -11
#define DNERR_TOMANYOPEN  -12
#define DNERR_NODNORMFILE  -13
#define DNERR_BADVERSION  -14
#define DNERR_BADFIELDMARK  -15
#define DNERR_BADTREE    -16

#define DNERR_CLSTREAM    -21
#define DNERR_NOREADSTREAM  -22
#define DNERR_NOWRITESTREAM  -23

#define DNERR_NOKRN    -31
#define DNERR_BADKRN    -32
#define DNERR_DUPLKRN    -33

#define DNERR_BADRB    -41
#define DNERR_RBLEN    -42
#define DNERR_BADDATEFORM  -43

#define DNERR_NOREADTARGET  -51
#define DNERR_NOREADOBJS  -52
#define DNERR_NOWRITETARGET  -53
#define DNERR_NOWRITEOBJS  -54

 
/* ---------------------------------------------------------------------------
   Parameteridents fuer die Funktionen dparmget() und dparmset()
   ---------------------------------------------------------------------------
*/
/* ----- fuer beide Funktionen -----
*/
#define    P_HWTYPE  1
#define    P_DTYPE    2

#define    PA_XT    3
#define    PA_VDT    4

#define    P_DRES1    5
#define    P_DRES2    6
#define    P_DRES3    7
#define    P_DRES4    8
#define    P_DRES5    9

#define    P_KNR    10
#define    P_RNR    11

#define    P_VANZ    12
#define    P_VDIM    13
#define    P_VSIZE    14

#define    P_ZF    15
#define    P_FSR    16
#define    P_OFS    17

#define    P_RRES1    18
#define    P_RRES2    19
#define    P_RRES3    20
#define    P_RRES4    21
#define    P_RRES5    22

#define    PA_RB    23
#define    PA_RT    24
#define    PA_DT    25
#define    PA_HT    26
#define    PA_VRT    27

/* ----- nur fuer dparmget() -----
*/
#define    P_LXT    103
#define    P_LVDT    104

#define    P_LRB    1023
#define    P_LRT    1024
#define    P_LDT    1025
#define    P_LHT    1026
#define    P_LVRT    1027

/* ----- nur fuer dparmset() -----
*/
#define    P_CLEAR    -1
#define    P_CLEAR_R  -2

#define    P_DUPLICATE  -3

#define    P_FREE    -4
#define    P_FREE_R  -5


/* ---------------------------------------------------------------------------
   DNORM_DCB: Dateisteuerblock
   ---------------------------------------------------------------------------
*/
typedef struct DNORM_DCB {

   /* ---------- Part 1: physischer Dateizugriff (C-like) ----------
   */
   char *  fname;    /* Name/Pfad der geoeffneten Datei      */
   char *  dvbfname;  /* Name/Pfad tmp. DVB-file            */
   FILE *  fp;      /* Filepointer fuer die geoeffnete Datei    */
   FILE *  dvb_file;    /* Filepointer auf temp. Vorblockdatei      */
   short  fopen_mode;  /* zum Lesen/Schreib./Anhaengen geoeffnet?  */
   t_i4    rvb_rba;  /* rba des Realis.- Vorblocks               */
   t_i4    dat_rba;  /* Beginn der Realisierungs-Daten-blocks    */

   /* ---------- Part 2: Indizierter Realisierungszugriff ---------- 
   */
   t_i4    objects;  /* Anzahl der Objecte/Realisierungen im Baum*/
   void *  tree;    /* Realisierungsbaum (Ordnung nach Klassen/ */
                                /* Realisierungs-Nummern)                   */
   void *   i_tree;         /* indizierter Realisierungsbaum (Ordnung   */
                                /* entsprechend Indizierung durch Nutzer)   */   
   short        i_key1;         /* Nummer des nutzerindizierten Schlussel-  */
   short        i_key2;         /* feldes (Primaer- und Sekundaerschluessel)*/
        /* beide == 0: Indizierung nach knr/rnr!    */

   /* ---------- Part 3: Kennzeichnung des aktuellen Zugriffs ---------- 
   */
   short  knr;    /* letzte behandelte Klassennummer          */
   short  rnr;    /* letzte behandelte Realisierungsnummer    */
   t_i4    r_rest;    /* Anzahl noch zu lesender Datenbytes       */
   t_i4    w_rest;    /* Anzahl noch freier,schreibbarer Datbytes */
        /* im letzten Realis.datenblock             */
   short  err;    /* akt. Fehlercode beim (letzten Zugriff    */
                                /* auf die Datei                            */
} DNORM_DCB;

/* ---------------------------------------------------------------------------
   RB: Recordbeschreibung
   ---------------------------------------------------------------------------
*/
typedef struct
{
   char   name;    /* Name der Vektorkomponente (keine '\0'    */
   char   name2;          /* erforderlich -> 4 gueltige Zeichen)      */
   char   name3;
   char   name4;  
   char   format;    /* Datenformat der Vektorkomponente (ASCIIZ)*/
   unsigned char size;    /* Groesse der Vektorkomponente in byte     */

} RB;


/* ---------------------------------------------------------------------------
   DPARA: Dnorm 3.0 Datei-und Realisierungsglobale Parameter,
          die durch den Nutzer gelesen/gesetzt werden koennen
   ---------------------------------------------------------------------------
*/
typedef struct
{
   /* ---------- Part 1: Dateieigenschaften ---------- 
   */
   short        hwtype;         /* Hardwaretype                             */
   char    dtype[2];  /* Dnorm-Dateityp (z.B. 'X''V' )            */

   char *  xt;    /* Zeiger auf den Dateikenntext             */
   short  lxt;    /* Laenge des Dateikenntextes               */

   char *       vdt;            /* "verbal descr. rea properties -record    */
   short        lvdt;           /* description" und Laengenangabe in byte   */

   double       dres1;          /* insgesamt 5 Reserveeintraege fuer        */
   double       dres2;          /* nutzerindividuelle Dateibezogene         */
   double       dres3;          /* Informationen ----->                     */
   double       dres4;
   double       dres5;          /*                                   <----- */

   /* ---------- Part 2: Merkmale der aktuellen Realisierung ----------
   */
   short  knr;    /* Klassennummer der akt. Realisierung      */
   short  rnr;    /* Realisierungsnummer                      */

   t_i4         vanz;    /* Vektoranzahl der Realisierung            */
   t_i4         vdim;    /* Vektordimension (Komponentenanzahl)      */
   t_i4         vsize;    /* Vektorgroesse in byte                    */

   double  zf;    /* Zeitfensterfaktor in milli-sekunden      */
   double  fsr;    /* Fortsetzrate in milli-sekunden           */
   double       ofs;            /* Offset des 1.Vektors der Realisierung    */
                                /* vom Beginn des Ursprungssignals (in ms)  */
   double       rres1;          /* insgesamt 5 Reserveeintraege fuer        */
   double       rres2;          /* nutzerindividuelle realisierungsbezogene */
   double       rres3;          /* Informationen ----->                     */
   double       rres4;
   double       rres5;          /*                                   <----- */

   /* ---------- Part 3: variable Realisierungstexte ---------- 
   */
   RB *    rb;    /* Zeiger auf die Recordbeschreibung        */
   short  lrb;    /* Laenge der Recordbeschreibung in byte    */

   char  *  rt;    /* Zeiger auf den Realisierungskenntext     */
   short  lrt;    /* Laenge des Realis.kenntextes in byte     */

   char *  dt;    /* Zeig.auf Datum der letzten Rea.-Modifik. */
   short  ldt;    /* Laenge des Datums in byte                */

   char *       ht;             /* Zeig. auf "modification history"-Text    */
   short        lht;            /* Laenge des history-Textes in byte        */

   char *       vrt;            /* "verbal described rea properties" (verbal*/
   short        lvrt;           /* beschriebene Realisierungseigenschaften) */
                                /* und entsprechende Laengenangabe in byte  */
   t_u4   mode;            /* Parameterbehandlungsmodus f. dopen/dclose*/
} DPARA;


/* ---------------------------------------------------------------------------
   Funktionsprototypen der Dnorm-Basis-Funktionen
   ---------------------------------------------------------------------------
*/

#ifdef __cplusplus
extern "C" {
#endif

_DECLSPEC DNORM_DCB *  _EXPORT dopen( const char * filename, const char * mode, DPARA * parms );
_DECLSPEC short        _EXPORT dclose( DNORM_DCB * stream, DPARA * parms );

_DECLSPEC short        _EXPORT dget( DNORM_DCB * stream, short knr, short rnr, DPARA * parms );
_DECLSPEC short        _EXPORT dset( DNORM_DCB * stream, short knr, short rnr, DPARA * parms );

_DECLSPEC t_i4         _EXPORT dread( void * buffer, t_i4 vsize, t_i4 n, DNORM_DCB * stream );
_DECLSPEC t_i4         _EXPORT dwrite( void * buffer, t_i4 vsize, t_i4 n, DNORM_DCB * stream );

_DECLSPEC short        _EXPORT dparmset( DPARA * parms, short which, ... );
_DECLSPEC short        _EXPORT dparmget( DPARA * parms, short which, ... );

_DECLSPEC short        _EXPORT dsize( DNORM_DCB * stream );

_DECLSPEC short        _EXPORT derror( DNORM_DCB * stream );
_DECLSPEC const char * _EXPORT dstrerror( short errnum );

#ifdef __cplusplus
}
#endif


/* ---------------------------------------------------------------------------
   globale Variablen
   ---------------------------------------------------------------------------
*/
extern short derrno;


#endif             /* defined __DNORM_H__  */

/* ===========================================================================
   END OF FILE dnorm.h
   ===========================================================================
*/
