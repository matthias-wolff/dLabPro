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

   NAME:  derror.c  -> "DNORM error functions"

   DESCRIPTION:

   AUTHOR:  t.rudolph

   COPYRIGHT:  copyright (c) 1994 by tu dresden/ita ag sprachkommunikation

   UPDATE:  26.01.94

   ===========================================================================
*/


/* ---------------------------------------------------------------------------
   INCLUDES
   ---------------------------------------------------------------------------
*/
#include <dlp_dnormint.h>


/* ---------------------------------------------------------------------------
   GLOBALS
   ---------------------------------------------------------------------------
*/
short  derrno;

static struct ERR_TAB
{
   short nr;
   const char * str;
} strerr_tab[] = {

 { DNERR_OK, "Kein Fehler festgestellt." },

 { DNERR_SYSFOPEN, "Oeffnen der Datei durch Betriebssystem nicht moeglich." },
 { DNERR_SYSSETVBUF, "C-Laufzeitsystem kann Filepuffer nicht erweitern." },
 { DNERR_SYSTMPFOPEN, "Oeffnen einer temporaeren Datei nicht moeglich." },
 { DNERR_SYSREAD, "Betriebssystemfehler beim Datenlesen." },
 { DNERR_SYSWRITE, "Betriebssystemfehler beim Datenschreiben." },

 { DNERR_BADFOPENMODE, "Unzulaessiger Dnorm-Dateioeffnungsmodus." },
 { DNERR_TOMANYOPEN, "Zu viele offene Dnorm-Dateien." },
 { DNERR_NODNORMFILE, "Datei besitzt kein Dnorm-kompatibles Format." },
 { DNERR_BADVERSION, "ungueltige DNORM-Version." },
 { DNERR_BADTREE, "ungueltige oder leerer Realisierungsbaum." },
 { DNERR_BADFIELDMARK, "ungueltige Feldidentifikatoren." },

 { DNERR_CLSTREAM, "Versuchter Zugriff auf nicht geoeffnete Datei." },
 { DNERR_NOREADSTREAM, "Datei darf nur geschrieben werden." },
 { DNERR_NOWRITESTREAM, "Datei darf nur gelesen werden." },

 { DNERR_NOKRN, "Realisierung mit knr/rnr existiert nicht." },
 { DNERR_BADKRN, "Unzulaessige knr/rnr - Spezifikation." },
 { DNERR_DUPLKRN, "Realisierung mit knr/rnr ist bereits vorhanden." },

 { DNERR_BADRB, "Keine oder fehlerhafte Recordbeschreibung." },
 { DNERR_RBLEN, "Unzulaessige Laenge der Recordbeschreibung." },
 { DNERR_BADDATEFORM, "Unzulaessiges Datumsformat." },

 { DNERR_NOREADTARGET, "Defekter Zielpuffer fuer Vektorlese-Operation." },  
 { DNERR_NOREADOBJS, "Keine weiteren Vektoren in der Realisierung." },  
 { DNERR_NOWRITETARGET, "Defekter Quellpuffer fuer Vektorschreib-Operation." },  
 { DNERR_NOWRITEOBJS, "Keine Schreib-Vektoren spezifiziert." },  

 { 0, NULL }
};


/* ===========================================================================
   PUBLIC
   ===========================================================================
*/
/* ---------------------------------------------------------------------------
.CSD:  derror

.NAM:  
.SHD:  
.DSC:  
.RES:  
.REM:  
.SAL:  
.EXF:  
.END.  
   ---------------------------------------------------------------------------
*/
short derror( DNORM_DCB * stream )
{
   if ( stream == NULL || stream->fp == NULL )
      return( derrno );
   else
      return( stream->err );
} /* end of derror() */



/* ---------------------------------------------------------------------------
.CSD:  dstrerror

.NAM:  
.SHD:  
.DSC:  
.RES:  
.REM:  
.SAL:  
.EXF:  
.END.  
   ---------------------------------------------------------------------------
*/

static char defstr[] = "Unbekannte Fehlernummer.";

const char * dstrerror( short errnum )
{
   struct ERR_TAB * ept = strerr_tab;

   while ( ept->str != NULL )
   {
      if ( ept->nr == errnum )
         return( ept->str );
      ept++;
   }

   return( defstr );           /* Nummer nicht gefunden */
} /* end of dstrerror() */


/* ===========================================================================
   END OF FILE derror.c
   ===========================================================================
*/

