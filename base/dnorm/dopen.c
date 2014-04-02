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

   NAME:  dopen.c  -> "DNORM file open/close functions"

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
static DNORM_DCB   dcbs[DOPEN_MAX];
static long  open_count = 0;


/* ---------------------------------------------------------------------------
   PROTOTYPES
   ---------------------------------------------------------------------------
*/
#if defined _RISCC_ || defined _OSFC_ || defined _GNUC_ || defined _SUNGC_ || defined _MSCWIN_
static void read_dvb( DVB * dvb, FILE * fp );
static void write_dvb( DVB * dvb, FILE * fp );
#endif


/* ===========================================================================
   PUBLIC
   ===========================================================================
*/
/* ---------------------------------------------------------------------------
.CSD:  dopen

.NAM:  
.SHD:  
.DSC:  Dnorm-Datei oeffnen, liefert DNORM_DCB-Zeiger auf Datei

     * Behandlung der Parameterstruktur parms:
       ---------------------------------------  
    - Parameterelemente werden beim Zugriff via 'r' zu Beginn
      alle geloescht! (mit 0/NULL initialisiert)
    - beim Zugriff via 'w':
            die vom Nutzer belegte Parameterstruktur bleibt unveraendert
            alle Dateiglobalen Informationen werden aus ihr uebernommen
            und in die Datei geschrieben   
    - beim Zugriff via 'a':
      1. die Datei enthaelt schon 3.0-Realisierungen:
         * aktualisieren des dort befindlichen Dateikenntextes
                 (Ueberschreiben oder anhaengen entsprechen des gesetzten
                 Modus') 
         * aktualisieren des Dateityps (wenn sinnvoll gesetzt) und
         * Lesen des Realisierungsbaumes
      2. die Datei ist leer:
         * gleiche Reaktion wie bei Zugriff via 'w'
     * Aktionen:
       ---------
  1.      Oeffnen der Datei entsrechend Modus
        2.  E/A-Puffer C-like vergroessern (Zugriff effektiver!!)
  3.  freien DNORM_DCB suchen

  4. "r"  - vollstaendiges Loeschen der Parameterstruktur!
    - lesen DVB (konstanten Teil), Fehler bei falschem Dateikennz.
                - Parameterstruktur mit Dateiglob.Infos belegen, alle
                  Vorblockinformationen werden vollstaendig abgelegt!

        5. "a"
           Fall 1 (Es sind bereits Realisierungen enthalten):
                - eingetragen werden: - Vorblockkennzeichen ('D''N')
                  (ohne Kontrolle     - Versionsnummer
                   durch Nutzer)      - Vorblockgroesse (errechnet)
                                      - Hardwaretyp (Konsistenzpruefung ist
                                        noch zu ergaenzen!!!!!)
                - ist der Parameter parms->modus auf DNMODE_NO_CHANGE ge-
                  setzt, bleiben alle urspruenglichen dateiglobalen, die
                  bereits in der Datei eingetragen waren vollstaendig
                  erhalten; sonst werden:
                - Dateivorblockinformationen aus der Parameterstruktur
                  uebernommen:        - Dateityp (wenn nicht '\0'\0')
                                      - dres* (immer)
                                      - xt (wenn != NULL und lxt != 0)
                                      - vdt (wenn != NULL und lvdt != 0)
                - ist der Parameter parms->modus auf DNMODE_APPEND ge-
                  setzt, wird der ueber parms->xt spezifizierte Dateikenn-
                  text an den bereits enthaltenen angehaengt (wenn
                  xt != NULL und lxt != 0). DNMODE_APPEND wirkt auch, wenn
                  alle anderen Infos per DNMODE_NO_CHANGE nicht geaendert
                  werden!

           Fall 2 (Es sind noch keine Realisierungen enthalten):
                - eingetragen werden: - Vorblockkennzeichen 
                  (ohne Kontrolle     - Versionsnummer
                   durch Nutzer)      - Vorblockgroesse (errechnet)
                                      - Hardwaretyp (Konsistenzpruefung)
                - Dateivorblockinformationen werden aus der Parameterstruktur
                  uebernommen:        - Dateityp (immer)
                                      - dres* (immer)
                                      - xt (wenn != NULL und lxt != 0, sonst
                                            als leer eingetragen)
                                      - vdt (wenn != NULL und lvdt != 0,
                                            sonst als leer eingetragen)
        5. "w"
                - loeschen des vorherigen Dateiinhaltes,
                  danach behandlung analog zu "a", Fall 1
  
.RES:  
.REM:  
.SAL:  
.EXF:  
.END.  
   ---------------------------------------------------------------------------
*/
DNORM_DCB * dopen( const char * filename, const char * mode, DPARA * parms )
{
   FILE * fp;
   DVB dvb;
   VDVB vdvb;
   long  dat_bytes;
   int i;        /* index fuer aktuellen DNORM_DCB */
   int j;        /* Zaehlvariable */
   char * xtpt = NULL;
   int  newfile = TRUE;      /* fuer "w" oder "a"  */

   /* Reaktion auf unterschiedlichen Computertyp ist noch
      zu ergaenzen 
      (bei Kompatibilitaet nur zwischen IBM-kompatiblen
      PC und DEC-RISC 5000 sind keine Datenprobleme aufgetreten
      (gleiche Formate fuer
    - long
    - float
    - double )
   */

   /* ----- Fehlerzelle ruecksetzen -----
   */
   derrno = DNERR_OK;

   /* ----- 1. Oeffnen der Datei entsprechend Modus -----
   */
   if ( *mode == 'r' )           /* nur lesen                */
   {
      if ( ( fp = fopen( filename, MODUS_r ) ) == NULL )
      {
         derrno = DNERR_SYSFOPEN;
   return( NULL );
      }
   }
   else if ( *mode == 'w' )                     /* schreiben & vorh. Loesch.*/
   {
      if ( ( fp = fopen( filename, MODUS_wr ) ) == NULL )
      {
         derrno = DNERR_SYSFOPEN;
   return( NULL );                        /* Fileinhalt wird geloescht*/
      }
      newfile = TRUE;
   }
   else if ( *mode == 'a' )                     /* anhaengen                */
   {
      if ( ( fp = fopen( filename, MODUS_rw ) ) /* Versuch zum Lesen zu offn*/
       == NULL )                          /* Datei muss existieren    */
      {
   if ( ( fp = fopen( filename, MODUS_wr ) ) == NULL ) /* wenn nicht  */
   {
            derrno = DNERR_SYSFOPEN;
      return( NULL );                     /* normaler Ueberschreibmode*/
   }
         newfile = TRUE;
      }
      else
         newfile = FALSE;
   }
   else
   {
      derrno = DNERR_BADFOPENMODE;
      return( NULL );
   }

   /* TODO: call zlib extraction */

   /* ----- E/A-Puffer vergroessern -----  
   */            
   if ( setvbuf( fp, NULL, _IOFBF,    /* Filepuffer vergroessern  */
                 SETV_BUFSIZE ) )               /* wenn Speicher alle...    */
      derrno = DNERR_SYSSETVBUF;    /* Flag setzen, sonst nichts*/

   /* ----- freien DNORM_DCB suchen und DNORM_DCB loeschen -----
   */
   for ( i = 0; i < DOPEN_MAX && dcbs[i].fp != NULL; i++ )
      ;                                        /* freien DNORM_DCB suchen        */
   if ( i >= DOPEN_MAX )      /* kein freier DNORM_DCB mehr     */
   {
      fclose( fp );
      derrno = DNERR_TOMANYOPEN;
      return( NULL );                           /* ..zurueck!               */
   }
   else            /* falls o.k., DNORM_DCB loeschen */
      memset( &dcbs[i], '\0', sizeof( DNORM_DCB ) );

   /* ----- weiter entsprechend Oeffnungsmodus... -----
   */
   /* ------------------------------------------------------------------
      Fall: Datei mit "r" geoeffnet
      ------------------------------------------------------------------
   */
   if ( *mode == 'r' )        /* zum Lesen geoeffnet ...  */
   {
      /* ----- lesen Dateivorblock, konst Teil 
               Achtung! die Elemente der Strukturen muessen
               Elementweise gelesen werden (Tribut an Strukturalignment von
               RISC C) -----
      */
#if defined _RISCC_ || defined _OSFC_ || defined _GNUC_ || defined _SUNGC_ || defined _MSCWIN_
      read_dvb( &dvb, fp );
#else 
      fread( &dvb, (size_t)sizeof( DVB ),       /* Lesen des Dateivorblocks */
       (size_t)1, fp );
#endif
      /* ----- Kontrolle Dateikennzeichen und Version -----
      */
      if ( dvb.vbkennz[0] != 'D' ||             /* Kontrollieren der Datei- */
     dvb.vbkennz[1] != 'N' )              /* kennzeichnung ('D''N')   */
      {
   fclose( fp );
         derrno = DNERR_NODNORMFILE;
   return( NULL );
      }
     if ( dvb.version != DN_VERSION )
      {
   fclose( fp );
         derrno = DNERR_BADVERSION;
   return( NULL );
      }
      /* ----- lesen Dateivorblock, variabler Teil -----
      */
      memset( &vdvb, '\0', sizeof( vdvb ) );    /* keine Infos spezifizert  */
                                                /* (im var.Vorblockteil)    */
      memset( parms, '\0', sizeof( DPARA ) );  /* Parameterstrukt. loeschen*/

      /* ----- Dateikenntextinfos lesen und in vdvb ablegen -----
      */
      /* --- 'X' 'T' ---
      */
      if(fread( vdvb.xt_sign, (size_t)sizeof( vdvb.xt_sign ), (size_t)1L, fp )!=1L)
      {
   fclose( fp );
         derrno = DNERR_NOREADSTREAM;
   return( NULL );
      }
      if ( vdvb.xt_sign[0] != 'X' || vdvb.xt_sign[1] != 'T' )
      {
   fclose( fp );
         derrno = DNERR_BADFIELDMARK;
   return( NULL );
      }
      if(fread_myswab( &vdvb.lxt, (size_t)sizeof( vdvb.lxt ), (size_t)1L, fp )!=1L)
      {
   fclose( fp );
         derrno = DNERR_NOREADSTREAM;
   return( NULL );
      }
      if ( vdvb.lxt )        /* Wenn Dateikenntext spez. */
      {
   parms->xt = dn_malloc( vdvb.lxt +1 );
         if(fread( parms->xt, (size_t)vdvb.lxt, (size_t)1L, fp )!=1L)
      {
   fclose( fp );
         derrno = DNERR_NOREADSTREAM;
   return( NULL );
      }
   ((char *)parms->xt)[vdvb.lxt] = '\0';  /* fuer alle Faelle! String!*/
      }
      parms->lxt = vdvb.lxt;                    /* Laenge merken (als Param)*/
                                                /* hinter xt befindet sich: */
      /* --- 'V' 'D' ---
      */
      if(fread( vdvb.vdt_sign, (size_t)sizeof( vdvb.vdt_sign ), (size_t)1L, fp )!=1L)
      {
   fclose( fp );
         derrno = DNERR_NOREADSTREAM;
   return( NULL );
      }
      if ( vdvb.vdt_sign[0] != 'V' || vdvb.vdt_sign[1] != 'D' )
      {
   fclose( fp );
         derrno = DNERR_BADFIELDMARK;
   return( NULL );
      }
      if(fread_myswab( &vdvb.lvdt, (size_t)sizeof( vdvb.lvdt ), (size_t)1L, fp )!=1L)
      {
   fclose( fp );
         derrno = DNERR_NOREADSTREAM;
   return( NULL );
      }
      if ( vdvb.lvdt )        /* Wenn spezifiziert        */
      {
   parms->vdt = dn_malloc( vdvb.lvdt +1 );
         if(fread( parms->vdt, (size_t)vdvb.lvdt, (size_t)1L, fp )!=1L)
      {
   fclose( fp );
         derrno = DNERR_NOREADSTREAM;
   return( NULL );
      }
   ((char *)parms->vdt)[vdvb.lvdt] = '\0';/* fuer alle Faelle! String!*/
      }
      parms->lvdt = vdvb.lvdt;                  /* Laenge merken (als Param)*/
                                                /* hinter vdt befindet sich: */
      /* ----- Realisierungsbaum laden -----
      */
      dcbs[i].tree = load_tree( fp,            /* Realisierungsbaum,der nun*/
       &dcbs[i].objects );  /* anzulegen ist.           */
      if ( !dcbs[i].tree )
      {
   fclose( fp );
         derrno = DNERR_BADTREE;
   return( NULL );
      }

      /* ----- Auf Beginn der ersten Realis. positionieren -----
      */
      fseek( fp, (long)(dvb.blocks * BL_SIZE), SEEK_SET ); 

      /* ----- weitere Parameterstruktur mit Dateiglob. Infos belegen -----
      */
      parms->hwtype = dvb.comptype;    /* Hardwaretyp festhalten   */
      parms->dtype[0] = dvb.dtype[0];           /* Datentyp ermitteln       */
      parms->dtype[1] = dvb.dtype[1];
      parms->dres1 = dvb.dres1;
      parms->dres2 = dvb.dres2;
      parms->dres3 = dvb.dres3;
      parms->dres4 = dvb.dres4;
      parms->dres5 = dvb.dres5;
   }

   /* ---------------------------------------------------------------------
      Fall: Datei mit "w" geoeffnet oder mit "a" geoeffnet und nicht leer 
      ---------------------------------------------------------------------
   */
   else            /* Oeffnen zum Schreiben    */
   {                                            /* "a" und "w"              */
      FILE * tmpfp;

      /* ----- Dateinamen merken -----
      */
    /* Aenderung durch Matthias Eichner, 30.10.2000 :
     Tmpnam() gibt den Namen eines Temp-Files im System-Temp-Verzeichnisses 
     zurueck. Ist dieses voll schlaegt das Speichern fehl, obwohl im 
     Zielverzeichniss ausreichend Platz ist. Es gibt eine Systemfunktion, die
     das Zielverzeichniss als Argument nimmt (tempnam()), allerdings wertet diese
     die Umgebungsvariable TEMPDIR aus und schreibt dorthin, wenn die Variable
     gesetzt ist. Wir wollen in jedem Fall in das Zielverzeichnis schreiben, 
     deshalb generieren wir einen eigenen temporaeren Dateinamen im Zielverzeichnis. 
     old: --> 
     dcbs[i].dvbfname = dn_malloc( L_tmpnam +1 );  -* temp. DVB-filename *-
     tmpnam( dcbs[i].dvbfname );    -* men ermitteln            *-
    <-- */
    /* new: --> */
      dcbs[i].dvbfname = dn_malloc( strlen(filename) +6 );  /* temp. DVB-filena-*/
    sprintf(dcbs[i].dvbfname,"%s.tmp~",filename);
    /* <-- */

      if ( ( tmpfp = fopen( dcbs[i].dvbfname, 
             MODUS_wr ) ) == NULL )    /* tmp.DVorbl.file oeffnen  */
      {
   dn_free( dcbs[i].dvbfname );
         derrno = DNERR_SYSTMPFOPEN;
      return( NULL );                        /* ..zurueck!               */
      }
      if ( setvbuf( tmpfp, NULL, _IOFBF,  /* Filepuffer vergroess.    */
        SETV_BUFSIZE ) )    /* wenn Speicher alle...    */
         derrno = DNERR_SYSSETVBUF;    /* Flag setzen, sonst nichts*/
                                                /* mit tmpfile alles o.K.:  */
      dcbs[i].dvb_file = tmpfp;                  /* tmpfp an stream binden   */

      memset( &dvb, '\0', sizeof( dvb ) );      /* keine Infos spezifizert  */
      memset( &vdvb, '\0', sizeof( vdvb ) );    /* keine Infos spezifizert  */
                                                /* (im var.Vorblockteil)    */

      /* ------------------------------------------------------------------
         Fall: Datei mit "w" geoeffnet oder
               Datei mit "a" geoeffnet und noch leer
         ------------------------------------------------------------------
      */
      if ( newfile )                      /* keine Real.enthalten     */
      {            
         /* ----- konst. Vorblock belegen und Laenge bestimmen -----
         */
   dvb.vbkennz[0] = 'D';      /* 'D''N' setzen            */
   dvb.vbkennz[1] = 'N';      /*                          */
   dvb.version = DN_VERSION;    /* Dateinorm-Version setzen */
   dvb.comptype = RECHNERTYP;             /* Hardwaretyp setzen       */

   dvb.dtype[0] = parms->dtype[0];  /* Datentyp uebernehmen     */
   dvb.dtype[1] = parms->dtype[1];        /*                          */
   dvb.dres1 = parms->dres1;              /* dres* uebernehmen        */
   dvb.dres2 = parms->dres2;
   dvb.dres3 = parms->dres3;
   dvb.dres4 = parms->dres4;
   dvb.dres5 = parms->dres5;

   dat_bytes = sizeof( dvb.vbkennz )  /* Groesse des in die Datei */
               + sizeof( dvb.version )    /* geschriebenen konstan-   */
               + sizeof( dvb.blocks )     /*                          */
               + sizeof( dvb.comptype )   /* ten Dateivorblocks in    */
               + sizeof( dvb.dtype )      /* byte ermitteln           */ 
               + 5 * sizeof( dvb.dres1 );

         /* ----- var. Vorblock belegen -----
         */
         vdvb.xt_sign[0] = 'X';
         vdvb.xt_sign[1] = 'T';
         if ( (ULONG)parms->lxt > SHRT_MAX )    /* xt zu lang!              */
            parms->lxt = SHRT_MAX;
         if ( parms->xt && parms->lxt )         /* xt != NULL               */
   {
            vdvb.lxt = parms->lxt;          /* Dateikenntextlaenge      */
         vdvb.xt = parms->xt;    /* Dateikenntextadresse     */
         }                                      /* sonst bleibt alles 0/NULL*/   
         vdvb.vdt_sign[0] = 'V';
         vdvb.vdt_sign[1] = 'D';
         if ( (ULONG)parms->lvdt > SHRT_MAX )   /* vdt zu lang!             */
            parms->lvdt = SHRT_MAX;
         if ( parms->vdt && parms->lvdt )       /* vdt != NULL              */
   {
            vdvb.lvdt = parms->lvdt;          /* Dateikenntextlaenge      */
      vdvb.vdt = parms->vdt;    /* Adresse prop.descr.      */
         }                                      /* sonst bleibt alles 0/NULL*/   

         /* ----- konst. Vorblock schreiben -----
         */
   dat_bytes += ( sizeof( vdvb.lxt ) + sizeof( vdvb.lvdt )
       + sizeof( vdvb.xt_sign )
       + sizeof( vdvb.vdt_sign ) ); /* soviel Vorblockbytes     */
            /* insges.(konst.+var.Teil) */
         if ( vdvb.lxt )
       dat_bytes += parms->lxt;
         if ( vdvb.lvdt )
       dat_bytes += parms->lvdt;

   dvb.blocks = dat_bytes / BL_SIZE;  /* soviel ganze blocks      */
   dat_bytes %= BL_SIZE;      /* uebrige Daten-bytes      */
   if ( dat_bytes )      /* noch Daten-bytes vorhand.*/
      dvb.blocks++;      /* ein block mehr           */
#if defined _RISCC_ || defined _OSFC_ || defined _GNUC_ || defined _SUNGC_ || defined _MSCWIN_
         write_dvb( &dvb, tmpfp );
         write_dvb( &dvb, fp );
#else 
   if(fwrite( &dvb, (size_t)sizeof( DVB ),   /* Vorblock schreiben...    */
     (size_t)1, tmpfp )!=1L)
      {
   fclose( fp );
         derrno = DNERR_NOWRITESTREAM;
   return( NULL );
      }            /* in temp.Dat.Vorblock file*/
   if(fwrite( &dvb, (size_t)sizeof( DVB ),   /* Vorblock schreiben...    */
     (size_t)1, fp )!=1L)
      {
   fclose( fp );
         derrno = DNERR_NOWRITESTREAM;
   return( NULL );
      }         /* ..auch in noch leeres Fil*/
#endif

         /* ----- var. Vorblock schreiben -----
         */                                     /* Dateikenntext in beide   */
   fputc( 'X', tmpfp ); fputc( 'T', tmpfp );    /* Dateien schreiben  */
   if(fwrite_myswab( &vdvb.lxt, (size_t)sizeof( vdvb.lxt ),
     (size_t)1, tmpfp )!=1L)
      {
   fclose( fp );
         derrno = DNERR_NOWRITESTREAM;
   return( NULL );
      }
   if(vdvb.xt && (fwrite( vdvb.xt, (size_t)vdvb.lxt,
     (size_t)1, tmpfp )!=1L))
      {
   fclose( fp );
         derrno = DNERR_NOWRITESTREAM;
   return( NULL );
      }
   fputc( 'X', fp ); fputc( 'T', fp );
   if(fwrite_myswab( &vdvb.lxt, (size_t)sizeof( vdvb.lxt ),
     (size_t)1, fp )!=1L)
      {
   fclose( fp );
         derrno = DNERR_NOWRITESTREAM;
   return( NULL );
      }
   if(vdvb.xt && (fwrite( vdvb.xt, (size_t)vdvb.lxt,
     (size_t)1, fp )!=1L))
      {
   fclose( fp );
         derrno = DNERR_NOWRITESTREAM;
   return( NULL );
      }
                   /* property description in  */
   fputc( 'V', tmpfp ); fputc( 'D', tmpfp );    /* Dateien schreiben  */
   if(fwrite_myswab( &vdvb.lvdt, (size_t)sizeof( vdvb.lvdt ),
     (size_t)1, tmpfp )!=1L)
      {
   fclose( fp );
         derrno = DNERR_NOWRITESTREAM;
   return( NULL );
      }
   if(vdvb.vdt && (fwrite( vdvb.vdt, (size_t)vdvb.lvdt,
     (size_t)1, tmpfp )!=1L))
      {
   fclose( fp );
         derrno = DNERR_NOWRITESTREAM;
   return( NULL );
      }
   fputc( 'V', fp ); fputc( 'D', fp );
   if(fwrite_myswab( &vdvb.lvdt, (size_t)sizeof( vdvb.lvdt ),
     (size_t)1, fp )!=1L)
      {
   fclose( fp );
         derrno = DNERR_NOWRITESTREAM;
   return( NULL );
      }
   if(vdvb.vdt && (fwrite( vdvb.vdt, (size_t)vdvb.lvdt,
     (size_t)1, fp )!=1L))
      {
   fclose( fp );
         derrno = DNERR_NOWRITESTREAM;
   return( NULL );
      }

   if ( dat_bytes )      /* noch Daten-bytes vorhand.*/
      for ( j = BL_SIZE; j > dat_bytes; j-- )   /* angerissenen Block */
         fputc( '\0', fp );              /* mit 0en auffuell.(Dat.f.)*/
            /* nicht! im Vorbl.file!!!  */
   dcbs[i].objects = 0L;      /* keine Realisierungen vorh*/
         dcbs[i].tree = NULL;             /* Realisierungsbaum leer!  */
      }

      /* ------------------------------------------------------------------
         Fall: Datei mit "a" geoeffnet und nicht leer
         ------------------------------------------------------------------
      */
      else          /* bereits Realis. enthalten*/
      {                                         /* Infos aus altem und neuem*/
         /* ----- alten Vorblock lesen und Konsistenz pruefen -----
         */
   fseek( fp, 0L, SEEK_SET );    /* zurueck an Dateianfang   */
#if defined _RISCC_ || defined _OSFC_ || defined _GNUC_ || defined _SUNGC_ || defined _MSCWIN_
         read_dvb( &dvb, fp );
#else 
         if(fread( &dvb, (size_t)sizeof( DVB ),    /* Lesen des Dateivorblocks */
       (size_t)1, fp )!=1L)
      {
   fclose( fp );
         derrno = DNERR_NOREADSTREAM;
   return( NULL );
      }
#endif
   if ( dvb.vbkennz[0] != 'D' ||          /* Kontrollieren der Datei- */
        dvb.vbkennz[1] != 'N' )           /* kennzeichnung ('D''N')   */
   {
      fclose( fp );
            derrno = DNERR_NODNORMFILE;
      return( NULL );
   }
         if ( dvb.version != DN_VERSION )
         {
      fclose( fp );
            derrno = DNERR_BADVERSION;
      return( NULL );
         }

         /* ----- konst. Vorblock aktualisieren -----
         */
   dvb.version = DN_VERSION;    /* Dateinorm-Version setzen */
   dvb.comptype = RECHNERTYP;             /* Hardwaretyp setzen       */

         if ( parms->mode & DNMODE_NO_CHANGE )  /* dateiglobale sollen nicht*/
            ;                                   /* geaendert werden         */
         else                                   /* sonst aendern:           */
         {
            if ( parms->dtype[0] != '\0' &&     /* wenn Datentyp sinnvoll   */
                 parms->dtype[1] != '\0' )      /* setzen, sonst alten      */
            {                                   /* Datentyp lassen          */
         dvb.dtype[0] = parms->dtype[0];
         dvb.dtype[1] = parms->dtype[1];
            }
      dvb.dres1 = parms->dres1;           /* dres* uebernehmen        */
      dvb.dres2 = parms->dres2;           /* zwangslaeufig ohne Kon-  */
      dvb.dres3 = parms->dres3;           /* trolle                   */
      dvb.dres4 = parms->dres4;
      dvb.dres5 = parms->dres5;
         }

         /* ----- lesen alten Dateivorblock, variabler Teil -----
         */
         /* --- 'X' 'T' ---
         */
         if(fread( vdvb.xt_sign, (size_t)sizeof( vdvb.xt_sign ), (size_t)1L, fp )!=1L)
      {
   fclose( fp );
         derrno = DNERR_NOREADSTREAM;
   return( NULL );
      }
         if ( vdvb.xt_sign[0] != 'X' || vdvb.xt_sign[1] != 'T' )
         {
      fclose( fp );
            derrno = DNERR_BADFIELDMARK;
      return( NULL );
         }
         if(fread_myswab( &vdvb.lxt, (size_t)sizeof( vdvb.lxt ), (size_t)1L, fp )!=1L)
      {
   fclose( fp );
         derrno = DNERR_NOREADSTREAM;
   return( NULL );
      }
         if ( vdvb.lxt )      /* Wenn Dateikenntext spez. */
         {
      vdvb.xt = dn_malloc( vdvb.lxt +1 );
            if(fread( vdvb.xt, (size_t)vdvb.lxt, (size_t)1L, fp )!=1L)
      {
   fclose( fp );
         derrno = DNERR_NOREADSTREAM;
   return( NULL );
      }
      ((char *)vdvb.xt)[vdvb.lxt] = '\0';  /* fuer alle Faelle!String!*/
         }
                                                 /* hinter xt befindet sich:*/
         /* --- 'V' 'D' ---
         */
         if(fread( vdvb.vdt_sign, (size_t)sizeof( vdvb.vdt_sign ), (size_t)1L, fp )!=1L)
      {
   fclose( fp );
         derrno = DNERR_NOREADSTREAM;
   return( NULL );
      }
         if ( vdvb.vdt_sign[0] != 'V' || vdvb.vdt_sign[1] != 'D' )
         {
         fclose( fp );
            derrno = DNERR_BADFIELDMARK;
      return( NULL );
         }
         if(fread_myswab( &vdvb.lvdt, (size_t)sizeof( vdvb.lvdt ), (size_t)1L, fp )!=1L)
      {
   fclose( fp );
         derrno = DNERR_NOREADSTREAM;
   return( NULL );
      }
         if ( vdvb.lvdt )       /* Wenn spezifiziert       */
         {
      vdvb.vdt = dn_malloc( vdvb.lvdt +1 );
            if(fread( vdvb.vdt, (size_t)vdvb.lvdt, (size_t)1L, fp )!=1L)
      {
   fclose( fp );
         derrno = DNERR_NOREADSTREAM;
   return( NULL );
      }
      ((char *)vdvb.vdt)[vdvb.lvdt] = '\0';/* fuer alle Faelle! String*/
         }
                                                /* hinter vdt befindet sich:*/
         /* ----- lesen alten Realisierungsbaum -----
         */
         dcbs[i].tree = load_tree( fp,          /* Realisierungsbaum,der nun*/
          &dcbs[i].objects );  /* anzulegen ist.           */

         /* ----- aktualisieren var. Dateivorblock -----
         */
         if ( parms->mode & DNMODE_NO_CHANGE )  /* var. dateiglobale sollen */
         {                                      /* nicht geaendert werden   */
            if ( parms->mode & DNMODE_APPEND )  /* dann kann hoechstens noch*/
      {                                   /* was an den Dateikenntext */
               xtpt = dn_malloc( parms->lxt     /* angehaengt werden        */
                                 + vdvb.lxt +1 );
               memmove( xtpt, vdvb.xt, (size_t)vdvb.lxt );
               memmove( xtpt + vdvb.lxt, parms->xt, (size_t)parms->lxt );

               if ( vdvb.xt )                   /* alten leoschen           */
      dn_free( vdvb.xt );
               vdvb.lxt = parms->lxt
                          + vdvb.lxt;           /* Dateikenntextlaenge      */
               if ( (ULONG)vdvb.lxt > SHRT_MAX )/* xt zu lang!              */
               vdvb.lxt = SHRT_MAX;
            vdvb.xt = xtpt;                  /* Dateikenntextadresse     */
            }                                   /* sonst bleibt alter stehen*/
         }
         else                                   /* sonst aendern:           */
         {
            if ( parms->lxt && parms->xt )      /* neuer xt != NULL         */
      {
               if ( parms->mode & DNMODE_APPEND )
         {
                  xtpt = dn_malloc( parms->lxt
                       + vdvb.lxt +1 );
                  memmove( xtpt, vdvb.xt, (size_t)vdvb.lxt );
                  memmove( xtpt + vdvb.lxt, parms->xt, (size_t)parms->lxt );

                  if ( vdvb.xt )                /* alten leoschen           */
         dn_free( vdvb.xt );
                  vdvb.lxt = parms->lxt
                             + vdvb.lxt;        /* Dateikenntextlaenge      */
               vdvb.xt = xtpt;               /* Dateikenntextadresse     */
               }                                /* sonst bleibt alter stehen*/
               else
               {
                  if ( vdvb.xt )                /* alten leoschen           */
         dn_free( vdvb.xt );
                  vdvb.lxt = parms->lxt;  /* Dateikenntextlaenge      */
               vdvb.xt = parms->xt;    /* Dateikenntextadresse     */
               }
            }                                   /* sonst bleibt alter stehen*/
            if ( (ULONG)vdvb.lxt > SHRT_MAX )   /* xt zu lang!              */
               vdvb.lxt = SHRT_MAX;

            if ( (ULONG)vdvb.lvdt > SHRT_MAX )  /* vdt zu lang!             */
               vdvb.lvdt = SHRT_MAX;
            if ( parms->lvdt && parms->vdt )    /* vdt != NULL              */
      {
               if ( vdvb.vdt )                  /* alten leoschen           */
      dn_free( vdvb.vdt );
               vdvb.lvdt = parms->lvdt;          /* Dateikenntextlaenge      */
         vdvb.vdt = parms->vdt;    /* Adresse prop.descr.      */
            }                                   /* sonst bleibt alter stehen*/
         }

         /* ----- konst. Vorblock schreiben -----
         */
#if defined _RISCC_ || defined _OSFC_ || defined _GNUC_ || defined _SUNGC_ || defined _MSCWIN_
         write_dvb( &dvb, tmpfp );
#else 
   if(fwrite( &dvb, (size_t)sizeof( DVB ),   /* neuen Vorblock schreib...*/
     (size_t)1, tmpfp )!=1L)
      {
   fclose( fp );
         derrno = DNERR_NOWRITESTREAM;
   return( NULL );
      }            /* in temp.Dat.Vorblock file*/
#endif

         /* ----- var. Vorblock schreiben -----
         */
   fputc( 'X', tmpfp ); fputc( 'T', tmpfp );
   if(fwrite_myswab( &vdvb.lxt, (size_t)sizeof( vdvb.lxt ),
     (size_t)1, tmpfp )!=1L)
      {
   fclose( fp );
         derrno = DNERR_NOWRITESTREAM;
   return( NULL );
      }
   if(fwrite( vdvb.xt, (size_t)vdvb.lxt,
     (size_t)1, tmpfp )!=1L)
      {
   fclose( fp );
         derrno = DNERR_NOWRITESTREAM;
   return( NULL );
      }
         if ( xtpt )
      dn_free( xtpt );
   fputc( 'V', tmpfp ); fputc( 'D', tmpfp );
   if(fwrite_myswab( &vdvb.lvdt, (size_t)sizeof( vdvb.lvdt ),
     (size_t)1, tmpfp )!=1L)
      {
   fclose( fp );
         derrno = DNERR_NOWRITESTREAM;
   return( NULL );
      }
   if(fwrite( vdvb.vdt, (size_t)vdvb.lvdt,
     (size_t)1, tmpfp )!=1L)
      {
   fclose( fp );
         derrno = DNERR_NOWRITESTREAM;
   return( NULL );
      }

   fseek( fp, 0L,        /* Auf Ende der letzten     */
    SEEK_END );                     /* Realis. positionieren:   */
                                                /* hier wird dann angehaengt*/
      }

      /* ----- Systemfehler bei tmp-File-E/A pruefen -----
      */
      if ( ferror( tmpfp ) )
      {
         perror( "dnorm sys error" );
         exit( 1 );
      }
   }


   /* ------------------------------------------------------------------
      Fall: fuer alle Oeffnungsmodi
      ------------------------------------------------------------------
   */
   /* ----- Systemfehler bei File-E/A pruefen -----
   */
   if ( ferror( fp ) )
   {
      perror( "dnorm sys error" );
      exit( 1 );
   }

   /* ---------- Rest des DNORM_DCB belegen ---------
   */
   dcbs[i].fp = fp;                            /* C-Filepointer            */
   dcbs[i].fopen_mode = *mode;                 /* Zugriffsmodus            */
   dcbs[i].knr = 0;                            /* keine akt. Klassennr.    */
   dcbs[i].rnr = 0;             /* keine akt. Realis.nr.    */
   dcbs[i].rvb_rba = 0L;                       /* keine Realisierung ausgew*/
                 /* (allg.Test-Kennzeichen   */
                 /* fuer:Datei frisch offen!)*/
   dcbs[i].dat_rba = 0L;                       /* keine Realisierung ausgew*/
   dcbs[i].r_rest = 0L;
   dcbs[i].w_rest = 0L;

   dcbs[i].i_key1 = 0L;                         /* keine Nutzerindizierung  */
   dcbs[i].i_key2 = 0L;
   dcbs[i].i_tree = NULL;

   /* ---------- Dateinamen merken ----------
   */
   dcbs[i].fname = dn_malloc( strlen( filename ) +1 );
   strcpy( dcbs[i].fname, filename );

   /* ----- beim ersten Mal die atexit-Funktion installieren -----
   */
   if ( !open_count )
   {
      atexit( dn_onexit );
      open_count++;
   }
   
   return( &dcbs[i] );
} /* end of dopen() */




/* ---------------------------------------------------------------------------
.CSD:  dclose

.NAM:  
.SHD:  
.DSC:  Dnorm-Datei schliessen

           Speicher fuer alle Datei- und Realisierungsinformationen
           wird freigegeben!!!!
.RES:  
.REM:  
.SAL:  
.EXF:  
.END.  
   ---------------------------------------------------------------------------
*/
short dclose( DNORM_DCB * stream, DPARA * parms )
{
   DVB dvb;            /* Dateivorblock    */
   int i;

   /* ---------- Speicher fuer belegte Parametereintraege 
                 freigeben und parms-struktur loeschen ----------
   */
   if ( parms )
   {
      if ( parms->xt )  dn_free( parms->xt );
      if ( parms->vdt )  dn_free( parms->vdt );

      if ( parms->rb )  dn_free( parms->rb );
      if ( parms->rt )  dn_free( parms->rt );
      if ( parms->dt )  dn_free( parms->dt );
      if ( parms->ht )  dn_free( parms->ht );
      if ( parms->vrt )  dn_free( parms->vrt );

      memset( parms, '\0', (size_t)sizeof( DPARA ) );  /* Param.loeschen   */
   }

   /* ----------------------------------------------------------------------
              Fall 1: war mit "a" oder "w" geoeffnet
      ----------------------------------------------------------------------
   */
   if ( stream->fopen_mode != 'r' )    /* war nicht zum Lesen offen*/
   {
      long dat_bytes;
      char * pt;
      short old_blocks;

      /* ---------- DVB aus temp. Vorblockfile lesen und gesamte
                    Vorblockbyteanzahl inclusive aktuellem Reali-
                    sierungsbaum ermitteln ----------
      */
      fseek( stream->dvb_file, 0L, SEEK_SET );  /* Anfang tmp. Vorblockfile */
#if defined _RISCC_ || defined _OSFC_ || defined _GNUC_ || defined _SUNGC_ || defined _MSCWIN_
      read_dvb( &dvb, stream->dvb_file );
#else 
      if(fread( &dvb, (size_t)sizeof( DVB ),       /* Lesen des Dateivorblocks */
       (size_t)1, stream->dvb_file )!=1L)
      {
        fclose( stream->fp );
        fclose( stream->dvb_file );
        remove( stream->fname );
        remove( stream->dvbfname );
         derrno = DNERR_NOREADSTREAM;
   return( EOF );
      }
#endif
      old_blocks = dvb.blocks;
      fseek( stream->dvb_file, 0L, SEEK_END );  /* Dateiende tmp.Vbl.file   */
                  /* steht hinter Dateikenntxt*/
      dat_bytes = ftell( stream->dvb_file ) +
      ( (stream->objects +1) *      /* Realis. + Dummyknoten!   */
        sizeof( FNODE_INFO ) );  /* eff.Laenge des Vorblocks */
      dvb.blocks = dat_bytes / BL_SIZE;    /* soviel ganze blocks      */
      dat_bytes %= BL_SIZE;      /* uebrige Daten-bytes      */
      if ( dat_bytes )        /* noch Daten-bytes vorhand.*/
   dvb.blocks++;        /* ein block mehr           */

      /* ---------- DVB mit korrekter blocks-zahl neu schreiben
                    anschliessend Realisierungsbaum in Vor.bl.datei und
                    bis zur naechsten Blockgrenze mit 0en fuellen ----------
      */
      fseek( stream->dvb_file, 0L, SEEK_SET );  /* Anfang tmp. Vorblockfile */
#if defined _RISCC_ || defined _OSFC_ || defined _GNUC_ || defined _SUNGC_ || defined _MSCWIN_
      write_dvb( &dvb, stream->dvb_file );
#else 
      if(fwrite( &dvb, (size_t)sizeof( DVB ),
       (size_t)1, stream->dvb_file )!=1L)
      {
        fclose( stream->fp );
        fclose( stream->dvb_file );
        remove( stream->fname );
        remove( stream->dvbfname );
         derrno = DNERR_NOWRITESTREAM;
   return( EOF );
      }  /* Dateivorblock neu schreib*/
#endif
      fseek( stream->dvb_file, 0L, SEEK_END );  /* Dateiende tmp.Vbl.file   */
                  /* steht hinter Real.kenntxt*/

      /* ----- leerer Realisierunsbaum ist fataler Fehler:
               Datei wird geschlossen und entfernt -----
      */
      if ( !stream->tree || !stream->objects )
      {
         fclose( stream->fp );
         fclose( stream->dvb_file );
         remove( stream->fname );
         remove( stream->dvbfname );
         derrno = stream->err = DNERR_BADTREE; 
         return( EOF );
      }
      save_tree( stream->dvb_file,     /* Schreiben des R.-Baumes  */
      stream->tree,      /* hinter Realis.kenntext   */
      stream->objects,      /* Offset fuer rba's!       */
      (short)((dvb.blocks - old_blocks) * BL_SIZE) );  /* (in blocks)      */
      if ( dat_bytes )        /* noch Daten-bytes vorhand.*/
   for ( i = BL_SIZE; i > dat_bytes; i-- )   /* angerissenen Block    */
      fputc( '\0', stream->dvb_file );  /* mit 0-en auffuellen      */

      /* ----- alle Realisierungsdaten aus Orignal-write-file 
               an Vor.bl.file anhaengen -----
      */
      pt = dn_malloc( BL_SIZE );
      fseek( stream->fp, 0L, SEEK_SET );  /* Dateianfang Originalfile */
#if defined _RISCC_ || defined _OSFC_ || defined _GNUC_ || defined _SUNGC_ || defined _MSCWIN_
      read_dvb( &dvb, stream->fp );
#else 
      fread( &dvb, (size_t)sizeof( DVB ),       /* Vorblock lesen...        */
       (size_t)1, stream->fp );           /* ...(der Datendatei)      */
#endif
      fseek( stream->fp, (size_t)dvb.blocks * BL_SIZE,  /* auf erste Realis.*/
       SEEK_SET );                        /* in der Datendatei        */
      while ( (fread( pt, (size_t)BL_SIZE, (size_t)1,    /* Daten in temp.   */
              stream->fp )==1) && !feof( stream->fp ) )       /* Datei schreiben  */
   if(fwrite( pt, (size_t)BL_SIZE, (size_t)1,
                 stream->dvb_file )!=1L)
      {
         derrno = DNERR_NOWRITESTREAM;
      }
      dn_free( pt );

      /* ----- Systemfehler bei File-E/A pruefen -----
      */
      if ( ferror( stream->fp ) || ferror( stream->dvb_file ) )
      {
         perror( "dnorm sys error" );
         exit( 1 );
      }

      /* ---------- Vorblock und Datenfile schliessen und 
                    Vorblockfile in Datenfile umbenennen ----------
      */
      fclose( stream->dvb_file );    /* DVorbl.Datei schliessen  */
      fclose( stream->fp );         /* Datei C-like schliessen  */

      /* TODO: call zlib here */

      
#if defined _TC_ || defined _BCWIN_ || defined _MSC_ || defined _MSCWIN_ || defined __MINGW32__ || __CYGWIN32__ || defined __TMS__
                            /* rename funktioniert nur, */
      remove( stream->fname );                  /* wenn Ziel nicht exist.!  */
      rename( stream->dvbfname, stream->fname );/* Namen der Ergebnisdatei  */
#else            /* vermeide in UNIX: remove */
      {            /* geht nicht bei bad links */
         char cmd[L_tmpnam + FILENAME_MAX + 10];  /* von quelle & ziel*/
         sprintf( cmd, "mv %s %s", stream->dvbfname, stream->fname );
         if(system( cmd ) != 0) {
           derrno = DNERR_NOWRITETARGET;
           return( EOF );
         }
      } 
#endif
 
      /* ---------- Speicher fuer Dateinamen in stream freigeben ----------
      */
      dn_free( stream->dvbfname );                 /* DVB-filenamen freigeben  */
      dn_free( stream->fname );                    /* Dateinamenspeich.freigeb.*/
   }
   /* ----------------------------------------------------------------------
              Fall 2: war mit "r" geoeffnet
      ----------------------------------------------------------------------
   */
   else            /* nur lese-Zugriff(mode"r")*/
   {
      if ( stream->fp )
      {
         fclose( stream->fp );                  /* Datei C-like schliessen  */
   dn_free( stream->fname );                 /* Dateinamenspeich.freigeb.*/
      }
   }

   /* ----------------------------------------------------------------------
              fuer alle Oeffnungsmdi
      ----------------------------------------------------------------------
   */
   del_tree( (KR_NODE**)&stream->tree );   /* Realisierungsbaum loesch.*/
   stream->fp = NULL;                           /* Kennz.: stream-Zelle frei*/

   derrno = DNERR_OK;
   return( 0 );
} /* end of dclose() */



/* ===========================================================================
   INTERNAL
   ===========================================================================
*/

/* dn_onexit: atexit - Diese Funktion wird bei exit ausgefuehrt;
  Kontrolle, ob alle zum Schreiben geoeffneten Files auch geschlossen
  wurden. Falls nicht: Meldung nach stderr.
*/
void dn_onexit( void )
{
   int i;

   for ( i = 0; i < DOPEN_MAX; i++ )
   {
      if ( dcbs[i].fp )
      {
         if ( dcbs[i].fopen_mode == 'w' || dcbs[i].fopen_mode == 'a' )
            fprintf( stderr, 
               "dnorm error: Datei %s wurde nicht geschlossen\n", dcbs[i].fname );
      }
   }

   return;
} /* end of dn_onexit() */




/* read_dvb/write_dvb:
*/

#if defined _RISCC_ || defined _OSFC_ || defined _GNUC_ || defined _SUNGC_ || defined _MSCWIN_
static void read_dvb( DVB * dvb, FILE * fp )
{
   if(fread( dvb->vbkennz, (size_t)sizeof( dvb->vbkennz ), (size_t)1, fp )!=1L) return;
        /* Zwei byte Vorblockkennzeichen            */
   if(fread_myswab( &dvb->version, (size_t)sizeof( dvb->version ), (size_t)1, fp )!=1L) return;
        /* Dnorm-Versionskennzeichnung              */
   if(fread_myswab( &dvb->blocks, (size_t)sizeof( dvb->blocks ), (size_t)1, fp )!=1L) return;
        /* Vorblockgroesse in blocks                */
   if(fread_myswab( &dvb->comptype, (size_t)sizeof( dvb->comptype ), (size_t)1, fp )!=1L) return;
        /* Rechner-(Hardware-)                      */
   if(fread( dvb->dtype, (size_t)sizeof( dvb->dtype ), (size_t)1, fp )!=1L) return;
        /* Dateityp (z.B. 'X''V' )                  */

   if(fread_myswab( &dvb->dres1, (size_t)sizeof( dvb->dres1 ), (size_t)1, fp )!=1L) return;
   if(fread_myswab( &dvb->dres2, (size_t)sizeof( dvb->dres2 ), (size_t)1, fp )!=1L) return;
   if(fread_myswab( &dvb->dres3, (size_t)sizeof( dvb->dres3 ), (size_t)1, fp )!=1L) return;
   if(fread_myswab( &dvb->dres4, (size_t)sizeof( dvb->dres4 ), (size_t)1, fp )!=1L) return;
   if(fread_myswab( &dvb->dres5, (size_t)sizeof( dvb->dres5 ), (size_t)1, fp )!=1L) return;
        /* insgesamt 5 Reserveeintraege             */
 
   return;
}

static void write_dvb( DVB * dvb, FILE * fp )
{
   if(fwrite( dvb->vbkennz, (size_t)sizeof( dvb->vbkennz ), (size_t)1, fp )!=1) return;
        /* Zwei byte Vorblockkennzeichen            */
   if(fwrite_myswab( &dvb->version, (size_t)sizeof( dvb->version ), (size_t)1, fp )!=1) return;
        /* Dnorm-Versionskennzeichnung              */
   if(fwrite_myswab( &dvb->blocks, (size_t)sizeof( dvb->blocks ), (size_t)1, fp )!=1) return;
        /* Vorblockgroesse in blocks                */
   if(fwrite_myswab( &dvb->comptype, (size_t)sizeof( dvb->comptype ), (size_t)1, fp )!=1) return;
        /* Rechner-(Hardware-)                      */
   if(fwrite( dvb->dtype, (size_t)sizeof( dvb->dtype ), (size_t)1, fp )!=1) return;
        /* Dateityp (z.B. 'X''V' )                  */

   if(fwrite_myswab( &dvb->dres1, (size_t)sizeof( dvb->dres1 ), (size_t)1, fp )!=1) return;
   if(fwrite_myswab( &dvb->dres2, (size_t)sizeof( dvb->dres2 ), (size_t)1, fp )!=1) return;
   if(fwrite_myswab( &dvb->dres3, (size_t)sizeof( dvb->dres3 ), (size_t)1, fp )!=1) return;
   if(fwrite_myswab( &dvb->dres4, (size_t)sizeof( dvb->dres4 ), (size_t)1, fp )!=1) return;
   if(fwrite_myswab( &dvb->dres5, (size_t)sizeof( dvb->dres5 ), (size_t)1, fp )!=1) return;
        /* insgesamt 5 Reserveeintraege             */
 
   return;
}
#endif

/* ===========================================================================
   END OF FILE dopen.c
   ===========================================================================
*/
