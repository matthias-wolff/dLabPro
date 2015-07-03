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

   NAME:  dget.c  -> "DNORM rea. get/set functions"

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
#include "dlp_dnormint.h"


/* ===========================================================================
   PUBLIC
   ===========================================================================
*/
/* ---------------------------------------------------------------------------
.CSD:  dget

.NAM:  
.SHD:  
.DSC:  Realisierung suchen und Parameter lesen
   - der Inhalt der Parameterstruktur wird bis auf die Komponenten
     xt und lxt grundsaetzlich geloescht,
   - alle fuer die gesuchte Realisierung nicht spezifizierten
     Felder (Zeiger darauf) enthalten den NULL-Poiter!
   - wurde die gesuchte Realisierung nicht gefunden, sind in parms
           bis auf xt/lxt ebenfalls alle anderen Komponenten geloescht!  
.RES:  
.REM:  
.SAL:  
.EXF:  
.END.  
   ---------------------------------------------------------------------------
*/
short dget( DNORM_DCB * stream, short  knr, short rnr, DPARA * parms )
{
   RVB     rvb;
   VRVB    vrvb;
   char *  buffer;    /* Puffer fuer Daten des var.Rea.vorblocks  */
   char *  pt;
   KR_NODE *  node;
   char *  m_xt, * m_vdt;
   short  m_lxt, m_lvdt;
   char    m_dtype[2];
   double  m_dres1, m_dres2, m_dres3, m_dres4, m_dres5;
   long    read_bytes;
#if defined _RISCC_ || defined _OSFC_ || defined _GNUC_ || defined _SUNGC_  || defined _MSCWIN_
   union 
   {
      struct
      {
         char l;
         char h;
      } b;
      short s;
   } val;
#endif

   /* ---------- Pruefung des Argumentstreams ----------
   */
   if ( stream == NULL || stream->fp == NULL )    /* garnicht offen!  */
   {
      derrno = DNERR_CLSTREAM;
      return( EOF );
   } 
   if ( stream->fopen_mode == 'w' || 
        stream->fopen_mode == 'a' )      /* nicht fuer mich! */
   {
      derrno = stream->err = DNERR_NOREADSTREAM;
      return( EOF );
   }

   /* ---------- sind ueberhaupt Realisierungen im Baum? ----------
   */
   if ( !stream->tree || !stream->objects )
   {
      derrno = stream->err = DNERR_NOREADOBJS;
      return( EOF );
   }

   /* ---------- aktuell in parms gesetzte Parameter merken
                 (nur die dateiglobalen) ----------
   */
   m_xt = parms->xt;        
   m_lxt = parms->lxt;                        
   m_vdt = parms->vdt;                         
   m_lvdt = parms->lvdt;
   m_dtype[0] = parms->dtype[0];
   m_dtype[1] = parms->dtype[1];
   m_dres1 = parms->dres1;
   m_dres2 = parms->dres2;
   m_dres3 = parms->dres3;
   m_dres4 = parms->dres4;
   m_dres5 = parms->dres5;

   /* ---------- aktuell in parms gesetzte realis.spezif. Parameter 
                 loeschen ----------
   */
   if ( parms->rb && parms->lrb )
      dn_free( parms->rb );        /* geloescht! und der Speich*/
   if ( parms->rt && parms->lrt )
      dn_free( parms->rt );                     /* fuer alte rb/rt/dt...    */
   if ( parms->dt && parms->ldt )  
      dn_free( parms->dt );                     /* geloescht                */
   if ( parms->ht && parms->lht )  
      dn_free( parms->ht );
   if ( parms->vrt && parms->lvrt )
      dn_free( parms->vrt );

   memset( parms, '\0', sizeof( DPARA ) );  /*                          */

   /* ---------- gemerkte dateiglob.Parms zuruecksetzen ----------
   */
   parms->xt = m_xt;        /* und nun wieder xt-Angaben*/
   parms->lxt = m_lxt;                          /* zurueckspeichern         */
   parms->vdt = m_vdt;        /* auch property desrciption*/
   parms->lvdt = m_lvdt;                        /* zurueckspeichern         */
   parms->dtype[0] = m_dtype[0];
   parms->dtype[1] = m_dtype[1];
   parms->dres1 = m_dres1;
   parms->dres2 = m_dres2;
   parms->dres3 = m_dres3;
   parms->dres4 = m_dres4;
   parms->dres5 = m_dres5;

   /* ---------- Suche der spez. Realisierung im Realis.baum 

  folgende Suchvarianten werden unterstuetzt:

  Fall 1:     1.knr/1.rnr der Datei (FIRST/FIRST)

  Fall 2.1:  naechste knr/rnr der Datei (NEXT/NEXT)
      bei erstem Zugriff auf Datei (identisch
      mit (FIRST/FIRST)

  Fall 2.2:  naechste knr/rnr der Datei (NEXT/NEXT)
      bei Folgezugriff auf Datei.

  Fall 3:     erste rnr bei konstanter knr (knr/FIRST)
  (nicht implementiert)

  Fall 4:     naechste rnr bei erster knr (FIRST/NEXT)
  (nicht implementiert)

  Fall 5:     naechste rnr bei konstanter knr (knr/NEXT)
  (nicht implementiert)

  Fall 6:     erste knr bei konstanter rnr (FIRST/rnr)
  (nicht implementiert)

  Fall 7:     naechste knr bei erster rnr (NEXT/FIRST)
  (nicht implementiert)

  Fall 8:     naechste knr bei konstanter rnr (NEXT/rnr)
  (nicht implementiert)

  Fall 9:     bestimmte knr und bestimmte rnr (knr/rnr)
   */
   /* ---------- Fall 1: ----------   
   */
   if ( knr == FIRST && rnr == FIRST )             /* erste Kl/Real.   */
   {
      if ( node = search_tree( stream->tree,
      stream->knr, stream->rnr,
      SEARCH_FIRST ),       /* erste Realis.    */
     node == NULL || node->rba == EOF )     /* Realisierung  su.*/
      {
         derrno = stream->err = DNERR_NOKRN;
   return( EOF );                                 /* nicht gefunden!  */
      }
   }
   else if ( knr == NEXT && rnr == NEXT )            /* naechste Kl/Real.*/
   {
      /* ---------- Fall 2.1: ----------   
      */
      if ( stream->knr == 0 && stream->rnr == 0 )  /* erster Zugriff   */
      {
   if ( node = search_tree( stream->tree,
         stream->knr, stream->rnr,
         SEARCH_FIRST ),       /* erste Realis.    */
     node == NULL || node->rba == EOF )     /* Realisierung  su.*/
         {
            derrno = stream->err = DNERR_NOKRN;
      return( EOF );                              /* nicht gefunden!  */
         }   
      }
      /* ---------- Fall 2.2: ----------   
      */
      else
      {
   if ( node = search_tree( stream->tree,
         stream->knr, stream->rnr,
         SEARCH_NEXT ),      /* naechste Realis. */
        node == NULL || node->rba == EOF )   /* Realisierung  su.*/
         {
            derrno = stream->err = DNERR_NOKRN;
      return( EOF );                               /* nicht gefunden!  */
         } 
      }
   }
   /* ---------- Fall 9: ----------   
   */
   else              /* normal spezifiz. */
   {
      if ( node = search_tree(  stream->tree, knr,      /* suchen nach knr/ */
        rnr, SEARCH_KRN ),      /* rnr              */
     node == NULL || node->rba == EOF )     /* Realisierung  su.*/
      {
         derrno = stream->err = DNERR_NOKRN;
   return( EOF );                                 /* nicht gefunden!  */
      }   
   }

   /* ---------- tatsaechliche knr/rnr fuer stream merken ----------   
   */
   stream->knr = node->knr;
   stream->rnr = node->rnr;

   /* ---------- konstanten Realisierungsvorblock lesen ----------   
   */
   stream->rvb_rba = node->rba;      /* Beginn des Vorbl. merken */
   fseek( stream->fp, node->rba, SEEK_SET );  /* auf RVB positionieren    */
#if defined _RISCC_ || defined _OSFC_ || defined _GNUC_ || defined _SUNGC_ || defined _MSCWIN_
   read_bytes = read_rvb( &rvb, stream->fp );
#else
   fread( &rvb, (size_t)sizeof( RVB ),
          (size_t)1, stream->fp );     /* ... und RVB einlesen     */
   read_bytes = sizeof( RVB );
#endif

   /* ---------- Rest des Realisierungsvorblocks (variablen
                 Teil) lesen und in vrvb aufbereiten ----------   
   */
   pt = buffer = dn_malloc( ((size_t)rvb.vblocks * BL_SIZE) - read_bytes );

   if(fread( pt, ((size_t)rvb.vblocks * BL_SIZE) - read_bytes,     /* var.Parameter einlesen   */
          (size_t)1, stream->fp ) != 1) {
     derrno = stream->err = DNERR_NOREADSTREAM;
     return( EOF );
   }
   memset( &vrvb, '\0', sizeof( vrvb ) );       /* keine Parameter spezifiz.*/
   pt += sizeof( vrvb.rb_sign );
   vrvb.lrb = *(short *)pt;
#if defined _SUNGC_
   myswab( &vrvb.lrb, sizeof( vrvb.lrb ), 1 );
#endif
   pt += sizeof( vrvb.lrb );
   if ( vrvb.lrb == 0 )                         /* Das keine RB spezifiz.ist*/
   {
      derrno = stream->err = DNERR_BADRB;
      return( EOF );                            /* ist nicht zulaessig!     */
   }
   vrvb.rb = (RB *)pt;
   pt += vrvb.lrb;

   pt += sizeof( vrvb.rt_sign );
   vrvb.lrt = *(short *)pt;
#if defined _SUNGC_
   myswab( &vrvb.lrt, sizeof( vrvb.lrt ), 1 );
#endif
   pt += sizeof( vrvb.lrt );
   if ( vrvb.lrt != 0 )        /* nur, wenn rt spezifiziert*/
      vrvb.rt = pt;                     /* sonst bleibt vrvb.rt NULL*/
   pt += vrvb.lrt;

   pt += sizeof( vrvb.dt_sign );
#if defined _RISCC_ || defined _OSFC_ || defined _GNUC_ || defined _SUNGC_ || defined _MSCWIN_
   val.b.l = *pt++; val.b.h = *pt++; 
   vrvb.ldt = val.s;
#else
   vrvb.ldt = *(short *)pt;
   pt += sizeof( vrvb.ldt );
#endif
#if defined _SUNGC_
   myswab( &vrvb.ldt, sizeof( vrvb.ldt ), 1 );
#endif
   if ( vrvb.ldt != 0 )        /* nur, wenn dt spezifiziert*/
      vrvb.dt = pt;
   pt += vrvb.ldt;

   pt += sizeof( vrvb.ht_sign );
#if defined _RISCC_ || defined _OSFC_ || defined _GNUC_ || defined _SUNGC_ || defined _MSCWIN_
   val.b.l = *pt++; val.b.h = *pt++; 
   vrvb.lht = val.s;
#else
   vrvb.lht = *(short *)pt;
   pt += sizeof( vrvb.lht );
#endif
#if defined _SUNGC_
   myswab( &vrvb.lht, sizeof( vrvb.lht ), 1 );
#endif
   if ( vrvb.lht != 0 )        /* nur, wenn ht spezifiziert*/
      vrvb.ht = pt;
   pt += vrvb.lht;

   pt += sizeof( vrvb.vrt_sign );
#if defined _RISCC_ || defined _OSFC_ || defined _GNUC_ || defined _SUNGC_ || defined _MSCWIN_
   val.b.l = *pt++; val.b.h = *pt++; 
   vrvb.lvrt = val.s;
#else
   vrvb.lvrt = *(short *)pt;
   pt += sizeof( vrvb.lvrt );
#endif
#if defined _SUNGC_
   myswab( &vrvb.lvrt, sizeof( vrvb.lvrt ), 1 );
#endif
   if ( vrvb.lvrt != 0 )      /* nur,wenn vrt spezifiziert*/
      vrvb.vrt = pt;
   pt += vrvb.lvrt;

   /* ---------- alle gelesenen Realisierungsparameter in
                 die Parameterstruktur parms eintragen
                 (wenn spezifiziert) ----------   
   */
   /* ---------- 1. Parameter des konstanter Parameterteils ----------   
   */
   parms->knr = stream->knr;    /* tatsaechliche Klassennnummer     */
   parms->rnr = stream->rnr;    /* tatsaechliche Realisierungsnr.   */
   parms->vanz = rvb.vanz;    /* Vektoranzahl                     */
   parms->vdim = rvb.vdim;    /* Vektordimension                  */
   parms->vsize = rvb.vsize;    /* Vektorgroesse in byte            */
   parms->zf = rvb.zf;      /* Zeitfensterfaktor                */
   parms->fsr = rvb.fsr;    /* Fortsetzrate                     */
   parms->ofs = rvb.ofs;    /* Offset                           */

   parms->rres1 = rvb.rres1;    /* Nutzerspezif. Zellen             */
   parms->rres2 = rvb.rres2;    /* Nutzerspezif. Zellen             */
   parms->rres3 = rvb.rres3;    /* Nutzerspezif. Zellen             */
   parms->rres4 = rvb.rres4;    /* Nutzerspezif. Zellen             */
   parms->rres5 = rvb.rres5;    /* Nutzerspezif. Zellen             */
   parms->rb = dn_malloc( vrvb.lrb +1 );

   /* ---------- lesbare Daten vorhanden? ----------
   */
   if ( !parms->vanz || !parms->vdim || !parms->vsize )
   {
      derrno = stream->err = DNERR_NOREADOBJS;
      return( EOF );
   }

   /* ---------- 2. Parameter variabler Laenge ----------   
   */
   memmove( parms->rb, vrvb.rb, (size_t)vrvb.lrb );
   ((char *)parms->rb)[vrvb.lrb] = '\0';
   parms->lrb = vrvb.lrb;

   if ( vrvb.rt )
   {
      parms->rt = dn_malloc( vrvb.lrt +1 );
      memmove( parms->rt, vrvb.rt, (size_t)vrvb.lrt );
      ((char *)parms->rt)[vrvb.lrt] = '\0';
      parms->lrt = vrvb.lrt;
   }
   if ( vrvb.dt )
   {
      parms->dt = dn_malloc( vrvb.ldt +1 );
      memmove( parms->dt, vrvb.dt, (size_t)vrvb.ldt );
      ((char *)parms->dt)[vrvb.ldt] = '\0';
      parms->ldt = vrvb.ldt;
   }
   if ( vrvb.ht )
   {
      parms->ht = dn_malloc( vrvb.lht +1 );
      memmove( parms->ht, vrvb.ht, (size_t)vrvb.lht );
      ((char *)parms->ht)[vrvb.lht] = '\0';
      parms->lht = vrvb.lht;
   }
   if ( vrvb.vrt )
   {
      parms->vrt = dn_malloc( vrvb.lvrt +1 );
      memmove( parms->vrt, vrvb.vrt, (size_t)vrvb.lvrt );
      ((char *)parms->vrt)[vrvb.lvrt] = '\0';
      parms->lvrt = vrvb.lvrt;
   }

   /* ---------- aktuelle Realisierungsdatenblockgroessen fuer
                 stream merken ----------   
   */
   stream->dat_rba = ftell( stream->fp );  /* Beginn der Daten-blocks  */
   stream->r_rest = rvb.vsize * rvb.vanz;  /* noch zu lesende Datenbyte*/

   /* ---------- Speicherfreigaben ----------   
   */
   dn_free( buffer );

   /* ----- Systemfehler bei File-E/A pruefen -----
   */
   if ( ferror( stream->fp ) )
   {
      perror( "dnorm sys error" );
      exit( 1 );
   }

   derrno = stream->err = DNERR_OK;
   return( 0 );
} /* end of dget() */



/* ---------------------------------------------------------------------------
.CSD:  dset

.NAM:  
.SHD:  
.DSC:  Realisierungsvorblock schreiben und Vorbereiten zum Daten-
  schreiben
.RES:  
.REM:  
.SAL:  
.EXF:  
.END.  
   ---------------------------------------------------------------------------
*/
short dset( DNORM_DCB * stream, short  knr, short rnr, DPARA * parms )
{
   RVB     rvb;
   long    rvb_bytes;    /* Anz. geschriebener RVB-Bytes     */
   int    i;
   KR_NODE *  tree_pt;
   long    write_bytes;
   char *  pt;


   /* ---------- Pruefung des Argumentstreams ----------
   */
   if ( stream == NULL || stream->fp == NULL )    /* garnicht offen!  */
   {
      derrno = DNERR_CLSTREAM;
      return( EOF );
   }
   if ( stream->fopen_mode == 'r' )      /* nicht fuer mich! */
   {
      derrno = stream->err = DNERR_NOWRITESTREAM;
      return( EOF );
   }

   /* ---------- Nachsehen, ob Realisierung mit knr/rnr im
                 Realisierungsbaum bereits vorhanden, wenn ja: Fehler

                 Achtung knr/rnr muessen! eindeutige shorts > 0 sein. 
     die Angabe von
                 FIRST, NEXT usw. (wie bei dget erlaubt) sind fuer
                 dset verboten ----------
   */
   if ( knr <= 0 || rnr <= 0 )       /* knr oder rnr == 0 usw.ist*/
   {
      derrno = stream->err = DNERR_BADKRN;
      return( EOF );                            /* nicht zu laessig!        */
   }
   if ( search_tree( stream->tree, knr, rnr,    /* wenn knr/rnr-Kombination */
                       SEARCH_KRN ) != NULL )   /* bereits vorhanden: Fehler*/
   {
      derrno = stream->err = DNERR_DUPLKRN;
      return( EOF );
   }

   /* ---------- einige Limitkontrollen ----------
   */
   if ( (ULONG)parms->lrb > SHRT_MAX )     /* rb zu lang,schwerer Fehl.*/
   {
      derrno = stream->err = DNERR_RBLEN;
      return( EOF );
   }
   if ( (ULONG)parms->lxt > SHRT_MAX )     /* rt zu lang! abschneiden! */
      parms->lxt = SHRT_MAX;
   if ( (ULONG)parms->lht > SHRT_MAX )     /* ht zu lang! abschneiden! */
      parms->lht = SHRT_MAX;
   if ( (ULONG)parms->lvrt > SHRT_MAX )     /* vrt zu lang! abschneiden!*/
      parms->lvrt = SHRT_MAX;

   /* ---------- Pruefung des Datumstring auf dnorm-Systemformat, bei
     Nichtkonsistenz Fehler!
     gueltiges Format mit 26 Zeichen:
            
                 'TAG - XX.MON.XX - HH:MM:SS'

     Falls kein Datumstring spezifiziert ist wird er an dieser 
                 Stelle mit dem aktuellen Datum generiert ----------
   */
   if ( parms->dt && parms->ldt )    /* Datum spezifiziert...    */
   {            /* Konsistenz testen        */
      if ( parms->ldt != 26 )      /* schon falsche Laenge     */
      {
         derrno = stream->err = DNERR_BADDATEFORM;
         return( EOF );
      }
      pt = parms->dt;
      if ( pt[4]  != '-' || pt[16] != '-' ||  /* Stichprobenartiger Test  */
     pt[8]  != '.' || pt[12] != '.' ||
     pt[20] != ':' || pt[23] != ':' )
      {
         derrno = stream->err = DNERR_BADDATEFORM;
         return( EOF );
      }
   }
   else            /* kein Datum spezifiziert  */
   {
      pt = dn_getfmtdate();
      parms->ldt = strlen( pt );
      parms->dt = dn_malloc( parms->ldt +1 );
      memmove( parms->dt, pt, (size_t)parms->ldt );
      ((char *)parms->dt)[parms->ldt] = '\0';
   }

   /* ---------- Berechnung/Ermittlung automatisch zu generierender
                 Realisierungsparameter ----------
   */
   parms->vsize = 0;
   for ( i = 0; i < parms->vdim; i++ )
      parms->vsize += parms->rb[i].size;  /* Vektorgroesse in byte    */
   parms->lrb = parms->vdim * sizeof( RB );     /* Laenge Recordbeschreibg. */


   /* ---------- Parameter aus parms bzw. knr/rnr uebernehmen ----------
   */
   rvb.vblocks = 0;
   rvb.rdblocks = 0;
   rvb.knr_sign[0] = 'K';   rvb.knr_sign[1] = 'N';
   rvb.knr = knr;
   rvb.rnr_sign[0] = 'R';   rvb.rnr_sign[1] = 'N';
   rvb.rnr = rnr;
   rvb.vanz_sign[0] = 'V';   rvb.vanz_sign[1] = 'A';
   rvb.vanz = 0;
   rvb.vdim_sign[0] = 'V';   rvb.vdim_sign[1] = 'D';
   rvb.vdim = parms->vdim;      /* Vektordimension          */
   rvb.vsize_sign[0] = 'V';   rvb.vsize_sign[1] = 'S';
   rvb.vsize = parms->vsize;      /* Vektorgroesse in byte    */
   rvb.zf_sign[0] = 'Z';   rvb.zf_sign[1] = 'F';
   rvb.zf = parms->zf;        /* Zeitfensterfaktor        */
   rvb.fsr_sign[0] = 'F';   rvb.fsr_sign[1] = 'S';
   rvb.fsr = parms->fsr;      /* Fortsetzrate             */
   rvb.ofs_sign[0] = 'O';   rvb.ofs_sign[1] = 'F';
   rvb.ofs = parms->ofs;      /* Offset                   */

   rvb.rres1_sign[0] = 'R';   rvb.rres1_sign[1] = '1';
   rvb.rres1 = parms->rres1;
   rvb.rres2_sign[0] = 'R';   rvb.rres2_sign[1] = '2';
   rvb.rres2 = parms->rres2;
   rvb.rres3_sign[0] = 'R';   rvb.rres3_sign[1] = '3';
   rvb.rres3 = parms->rres3;
   rvb.rres4_sign[0] = 'R';   rvb.rres4_sign[1] = '4';
   rvb.rres4 = parms->rres4;
   rvb.rres5_sign[0] = 'R';   rvb.rres5_sign[1] = '5';
   rvb.rres5 = parms->rres5;

   /* ---------- Pos. des Vorblockanfangs der Realiserung
                 fuer stream merken   ----------
   */
   fseek( stream->fp, 0L, SEEK_END );    /* Pos. ans Dateiende       */

   stream->rvb_rba = ftell( stream->fp );  /* Start-rba d. Realis.merk.*/

   /* ---------- Schreiben der Vorblockinformationen konstanter
                 Teil ----------
   */
   rvb_bytes = 0;                               /* Zelle:Bytelaenge Vorblock*/
#if defined _RISCC_ || defined _OSFC_ || defined _GNUC_ || defined _SUNGC_ || defined _MSCWIN_
   write_bytes = write_rvb( &rvb, stream->fp );
#else
   if(fwrite( &rvb, (size_t)sizeof( RVB ),   /* Vorblock schreiben       */
     (size_t)1, stream->fp ) !=1 ) {
         derrno = stream->err = DNERR_NOWRITESTREAM;
         return( EOF );
      }             /* RVB-blocks & Daten-block */
   write_bytes = sizeof( RVB ); 
#endif
   rvb_bytes += write_bytes;                    /* -anz. noch nicht bekannt!*/

   /* ---------- Schreiben der Recordbeschreibung ----------
   */
   fputc( 'R', stream->fp );  fputc( 'B', stream->fp );
   rvb_bytes += 2;
   if(fwrite_myswab( &parms->lrb, (size_t)sizeof( parms->lrb ),
     (size_t)1, stream->fp ) !=1 ) {
         derrno = stream->err = DNERR_NOWRITESTREAM;
         return( EOF );
      }
   rvb_bytes += sizeof( parms->lrb );
   if(parms->rb && (fwrite( parms->rb, (size_t)parms->lrb,
     (size_t)1, stream->fp ) !=1 )) {
         derrno = stream->err = DNERR_NOWRITESTREAM;
         return( EOF );
      }
   rvb_bytes += parms->lrb;

   /* ---------- Schreiben des Realis.kenntextes ----------
   */
   fputc( 'R', stream->fp );  fputc( 'T', stream->fp );
   rvb_bytes += 2;
   if(fwrite_myswab( &parms->lrt, (size_t)sizeof( parms->lrt ),
     (size_t)1, stream->fp ) !=1 ) {
         derrno = stream->err = DNERR_NOWRITESTREAM;
         return( EOF );
      }
   rvb_bytes += sizeof( parms->lrt );
   if(parms->rt && (fwrite( parms->rt, (size_t)parms->lrt,
     (size_t)1, stream->fp ) !=1 )) {
         derrno = stream->err = DNERR_NOWRITESTREAM;
         return( EOF );
      }
   rvb_bytes += parms->lrt;

   /* ---------- Schreiben des Datumstrings ----------
   */
   fputc( 'D', stream->fp );  fputc( 'T', stream->fp );
   rvb_bytes += 2;
   if(fwrite_myswab( &parms->ldt, (size_t)sizeof( parms->ldt ),
     (size_t)1, stream->fp ) !=1 ) {
         derrno = stream->err = DNERR_NOWRITESTREAM;
         return( EOF );
      }
   rvb_bytes += sizeof( parms->ldt );
   if(parms->dt && (fwrite( parms->dt, (size_t)parms->ldt,
     (size_t)1, stream->fp ) !=1 )) {
         derrno = stream->err = DNERR_NOWRITESTREAM;
         return( EOF );
      }
   rvb_bytes += parms->ldt;

   /* ---------- Schreiben der Modification history ----------
   */
   fputc( 'H', stream->fp );  fputc( 'T', stream->fp );
   rvb_bytes += 2;
   if(fwrite_myswab( &parms->lht, (size_t)sizeof( parms->lht ),
     (size_t)1, stream->fp ) !=1 ) {
         derrno = stream->err = DNERR_NOWRITESTREAM;
         return( EOF );
      }
   rvb_bytes += sizeof( parms->lht );
   if(parms->ht && (fwrite( parms->ht, (size_t)parms->lht,
     (size_t)1, stream->fp ) !=1 )) {
         derrno = stream->err = DNERR_NOWRITESTREAM;
         return( EOF );
      }
   rvb_bytes += parms->lht;

   /* ---------- Schreiben des Realisierungsattributrecords ----------
   */
   fputc( 'V', stream->fp );  fputc( 'R', stream->fp );
   rvb_bytes += 2;
   if(fwrite_myswab( &parms->lvrt, (size_t)sizeof( parms->lvrt ),
     (size_t)1, stream->fp ) !=1 ) {
         derrno = stream->err = DNERR_NOWRITESTREAM;
         return( EOF );
      }
   rvb_bytes += sizeof( parms->lvrt );
   if(parms->vrt && (fwrite( parms->vrt, (size_t)parms->lvrt,
     (size_t)1, stream->fp ) !=1 )) {
         derrno = stream->err = DNERR_NOWRITESTREAM;
         return( EOF );
      }
   rvb_bytes += parms->lvrt;

   /* ---------- Auffuellen des Vorblocks mit 0en und Vorblock -
                 groesse ermitteln (Anzahl der blocks) ----------
   */
   rvb.vblocks = (short)(rvb_bytes / BL_SIZE);  /* ganze blocks */
   rvb_bytes %= BL_SIZE;                        /* Rest-bytes vorhanden ? */
   if ( rvb_bytes )        /* nicht 0 */
   {
      for ( i = BL_SIZE; i > rvb_bytes; i-- )   /* angerissenen Block */
         fputc( '\0', stream->fp );    /* mit 0-en auffuellen */
      rvb.vblocks++;        /* ein block mehr */
   }

   /* ---------- Vorblock mit korrekter blocks-Zahl neuschreiben
                 und Pos. des Beginns der Realis.daten merken ----------
   */
   stream->dat_rba = ftell( stream->fp );  /* Beginn der Daten-blocks  */
   fseek( stream->fp, stream->rvb_rba,
    SEEK_SET );        /* Anfang des Vorblocks     */
#if defined _RISCC_ || defined _OSFC_ || defined _GNUC_ || defined _SUNGC_ || defined _MSCWIN_
   write_bytes = write_rvb( &rvb, stream->fp );
#else
   if(fwrite( &rvb, (size_t)sizeof( RVB ),   /* Vorblock neu schreiben...*/
     (size_t)1, stream->fp ) !=1 ) {
         derrno = stream->err = DNERR_NOWRITESTREAM;
         return( EOF );
      }
   write_bytes = sizeof( RVB ); 
#endif
   fseek( stream->fp, stream->dat_rba,    /* und zurueck auf den...   */
          SEEK_SET );        /* Anfang des 1.Datenblock  */
   stream->w_rest = 0L;        /* keine freien bytes mehr  */
            /* wichtige Info fuer dwrite*/

   /* ---------- Einfuegen der Realisierung in Realisierungsbaum ----------
   */
   tree_pt = add_tree( stream->tree, knr,  /* neue Realisierung mit Nr.*/
         rnr,stream->rvb_rba );     /* und Start-rba in R.-Baum */
   stream->objects++;
   stream->tree = tree_pt;
   
   /* ---------- in knr/rnr spez. Nummer wird die aktuelle
                 in parms und stream ----------
   */
   parms->knr = knr;        /* tatsaechliche Klassennnr.*/
   parms->rnr = rnr;        /* tatsaechliche Realis.nr. */
   stream->knr = knr;        /* aktuelle Klassen- und    */
   stream->rnr = rnr;                         /* Realis.nr. uebernehmen   */

   /* ----- Systemfehler bei File-E/A pruefen -----
   */
   if ( ferror( stream->fp ) )
   {
      perror( "dnorm sys error" );
      exit( 1 );
   }

   derrno = stream->err = DNERR_OK;
   return( 0 );
} /* end of dset() */




/* ===========================================================================
   INTERNAL
   ===========================================================================
*/

#if defined _RISCC_ || defined _OSFC_ || defined _GNUC_ || defined _SUNGC_ || defined _MSCWIN_
size_t read_rvb( RVB * rvb, FILE * fp )
{
   size_t b = 0;  


   if(fread_myswab( &rvb->vblocks, (size_t)sizeof( rvb->vblocks ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->vblocks );
          /* Anzahl der Vorblock-blocks       */
   if(fread_myswab( &rvb->rdblocks, (size_t)sizeof( rvb->rdblocks ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rdblocks );
          /* Anzahl Realisierungsdaten-blocks */

   if(fread( rvb->knr_sign, (size_t)sizeof( rvb->knr_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->knr_sign );
   if(fread_myswab( &rvb->knr, (size_t)sizeof( rvb->knr ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->knr );
          /* Klassennummer                    */
   if(fread( rvb->rnr_sign, (size_t)sizeof( rvb->rnr_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rnr_sign );
   if(fread_myswab( &rvb->rnr, (size_t)sizeof( rvb->rnr ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rnr );
          /* Realisierungsnummer              */
   if(fread( rvb->vanz_sign, (size_t)sizeof( rvb->vanz_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->vanz_sign );
   if(fread_myswab( &rvb->vanz, (size_t)sizeof( rvb->vanz ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->vanz );
          /* Anzahl der Realisierungsvektoren */
   if(fread( rvb->vdim_sign, (size_t)sizeof( rvb->vdim_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->vdim_sign );
   if(fread_myswab( &rvb->vdim, (size_t)sizeof( rvb->vdim ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->vdim );
          /* Dimension der Vektoren           */
   if(fread( rvb->vsize_sign, (size_t)sizeof( rvb->vsize_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->vsize_sign );
   if(fread_myswab( &rvb->vsize, (size_t)sizeof( rvb->vsize ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->vsize );
          /* Groesse der Vektoren in bytes    */
   if(fread( rvb->zf_sign, (size_t)sizeof( rvb->zf_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->zf_sign );
   if(fread_myswab( &rvb->zf, (size_t)sizeof( rvb->zf ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->zf );
          /* Zeitfensterfaktor                */
   if(fread( rvb->fsr_sign, (size_t)sizeof( rvb->fsr_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->fsr_sign );
   if(fread_myswab( &rvb->fsr, (size_t)sizeof( rvb->fsr ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->fsr );
          /* Fortsetzrate                     */
   if(fread( rvb->ofs_sign, (size_t)sizeof( rvb->ofs_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->ofs_sign );
   if(fread_myswab( &rvb->ofs, (size_t)sizeof( rvb->ofs ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->ofs );
          /* Offset des Realis.beginns        */
   if(fread( rvb->rres1_sign, (size_t)sizeof( rvb->rres1_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rres1_sign );
   if(fread_myswab( &rvb->rres1, (size_t)sizeof( rvb->rres1 ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rres1 );
   if(fread( rvb->rres2_sign, (size_t)sizeof( rvb->rres2_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rres2_sign );
   if(fread_myswab( &rvb->rres2, (size_t)sizeof( rvb->rres2 ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rres2 );
   if(fread( rvb->rres3_sign, (size_t)sizeof( rvb->rres3_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rres3_sign );
   if(fread_myswab( &rvb->rres3, (size_t)sizeof( rvb->rres3 ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rres3 );
   if(fread( rvb->rres4_sign, (size_t)sizeof( rvb->rres4_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rres4_sign );
   if(fread_myswab( &rvb->rres4, (size_t)sizeof( rvb->rres4 ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rres4 );
   if(fread( rvb->rres5_sign, (size_t)sizeof( rvb->rres5_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rres5_sign );
   if(fread_myswab( &rvb->rres5, (size_t)sizeof( rvb->rres5 ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rres5 );
          /* insgesamt 5 Reserveeintraege f.  */

   return ( b );
}


size_t write_rvb( RVB * rvb, FILE * fp )
{
   size_t b = 0;  

   if(fwrite_myswab( &rvb->vblocks, (size_t)sizeof( rvb->vblocks ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->vblocks );
          /* Anzahl der Vorblock-blocks       */
   if(fwrite_myswab( &rvb->rdblocks, (size_t)sizeof( rvb->rdblocks ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rdblocks );
          /* Anzahl Realisierungsdaten-blocks */

   if(fwrite( rvb->knr_sign, (size_t)sizeof( rvb->knr_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->knr_sign );
   if(fwrite_myswab( &rvb->knr, (size_t)sizeof( rvb->knr ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->knr );
          /* Klassennummer                    */
   if(fwrite( rvb->rnr_sign, (size_t)sizeof( rvb->rnr_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rnr_sign );
   if(fwrite_myswab( &rvb->rnr, (size_t)sizeof( rvb->rnr ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rnr );
          /* Realisierungsnummer              */
   if(fwrite( rvb->vanz_sign, (size_t)sizeof( rvb->vanz_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->vanz_sign );
   if(fwrite_myswab( &rvb->vanz, (size_t)sizeof( rvb->vanz ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->vanz );
          /* Anzahl der Realisierungsvektoren */
   if(fwrite( rvb->vdim_sign, (size_t)sizeof( rvb->vdim_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->vdim_sign );
   if(fwrite_myswab( &rvb->vdim, (size_t)sizeof( rvb->vdim ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->vdim );
          /* Dimension der Vektoren           */
   if(fwrite( rvb->vsize_sign, (size_t)sizeof( rvb->vsize_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->vsize_sign );
   if(fwrite_myswab( &rvb->vsize, (size_t)sizeof( rvb->vsize ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->vsize );
          /* Groesse der Vektoren in bytes    */
   if(fwrite( rvb->zf_sign, (size_t)sizeof( rvb->zf_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->zf_sign );
   if(fwrite_myswab( &rvb->zf, (size_t)sizeof( rvb->zf ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->zf );
          /* Zeitfensterfaktor                */
   if(fwrite( rvb->fsr_sign, (size_t)sizeof( rvb->fsr_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->fsr_sign );
   if(fwrite_myswab( &rvb->fsr, (size_t)sizeof( rvb->fsr ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->fsr );
          /* Fortsetzrate                     */
   if(fwrite( rvb->ofs_sign, (size_t)sizeof( rvb->ofs_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->ofs_sign );
   if(fwrite_myswab( &rvb->ofs, (size_t)sizeof( rvb->ofs ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->ofs );
          /* Offset des Realis.beginns        */
   if(fwrite( rvb->rres1_sign, (size_t)sizeof( rvb->rres1_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rres1_sign );
   if(fwrite_myswab( &rvb->rres1, (size_t)sizeof( rvb->rres1 ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rres1 );
   if(fwrite( rvb->rres2_sign, (size_t)sizeof( rvb->rres2_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rres2_sign );
   if(fwrite_myswab( &rvb->rres2, (size_t)sizeof( rvb->rres2 ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rres2 );
   if(fwrite( rvb->rres3_sign, (size_t)sizeof( rvb->rres3_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rres3_sign );
   if(fwrite_myswab( &rvb->rres3, (size_t)sizeof( rvb->rres3 ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rres3 );
   if(fwrite( rvb->rres4_sign, (size_t)sizeof( rvb->rres4_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rres4_sign );
   if(fwrite_myswab( &rvb->rres4, (size_t)sizeof( rvb->rres4 ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rres4 );
   if(fwrite( rvb->rres5_sign, (size_t)sizeof( rvb->rres5_sign ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rres5_sign );
   if(fwrite_myswab( &rvb->rres5, (size_t)sizeof( rvb->rres5 ), (size_t)1, fp )!=1) return b;
   b += (size_t)sizeof( rvb->rres5 );
          /* insgesamt 5 Reserveeintraege f.  */

   return( b );
}
#endif

/* ===========================================================================
   END OF FILE dset.c
   ===========================================================================
*/
