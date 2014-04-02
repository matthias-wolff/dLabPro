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

   NAME:  dread.c  -> "DNORM read/write functions"

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


/* ===========================================================================
   PUBLIC
   ===========================================================================
*/
/* ---------------------------------------------------------------------------
.CSD:  

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
t_i4 dread( void * buffer, t_i4 vsize, t_i4 n, DNORM_DCB * stream )
{
   long   numb;
   long   items;
   RVB    rvb;


   /* ---------- stream-Eigenschaften testen ----------
   */
   if ( stream == NULL || stream->fp == NULL )  /* garnicht offen!          */
   {
      derrno = DNERR_CLSTREAM;
      return( 0 );
   }
   if ( stream->fopen_mode != 'r' )    /* Lesedatei!               */
   {
      derrno = DNERR_NOREADSTREAM;
      return( 0 );
   }

   /* ---------- Limits kontrollieren ----------
   */
   if ( vsize == 0L || n == 0L || buffer == NULL )  
   {
      derrno = DNERR_NOREADTARGET;
      return( 0 );
   }

   /* ---------- Anzahl der zu uebertragenden Objekte feststellen ----------
   */
   if ( stream->r_rest < vsize )    /* keine Datenbyte mehr vorh*/
   {
      derrno = stream->err = DNERR_NOREADOBJS;
      return( 0 );                              /* 0: keine Objekte mehr!   */
   }
   if ( (numb = stream->r_rest / vsize) > n )   /* mehr Objekte vorhanden,  */
      numb = n;                                 /* als gefordert            */

   /* ---------- Objekte lesen und Restbytes der Realisierung
                 berechnen ----------
   */

#if defined _TC_ || defined _BCWIN_ || defined _GNUC_ || defined _SUNGC_
   /* ----- unter DOS-Turbo C ist size_t == unsigned short 
            deshalb mu˜ bei gro˜en Feldern mehrfach gelesen werden -----
   */
   {
      char HUGE * bpt = buffer;
      long read_bytes, nbytes, nread, objects;

      nbytes = vsize * numb;
      read_bytes = 0;
      do
      {
         nread = nbytes;
         if ( nread >= SHRT_MAX )
            nread = (SHRT_MAX/vsize)*vsize;
         objects = fread( bpt, (size_t)sizeof( *bpt ), (size_t)nread, stream->fp );
#if defined _SUNGC_
         myswab_data( bpt, objects / vsize, stream );
#endif
         bpt += objects;
         nbytes -= objects;
         read_bytes += objects;
      } while ( objects && nbytes > 0 );

      items = read_bytes / vsize; 
      stream->r_rest -= read_bytes;    /* soviel bytes bleiben uebr*/

      if ( read_bytes % vsize )      
         derrno = stream->err = DNERR_SYSREAD;
   } 
#else
   items = fread( buffer, (size_t)vsize, (size_t )numb,
                  stream->fp );                 /* Objekte einlesen         */
   stream->r_rest -= ( vsize * items );    /* soviel bytes bleiben uebr*/
#endif

   if ( items != numb )        
      derrno = stream->err = DNERR_SYSREAD;

   if ( stream->r_rest <= 0L )      /* keine weiteren Daten!    */
   {
      stream->r_rest = 0L;
      fseek( stream->fp, stream->rvb_rba,
       SEEK_SET );      /* Anfang des Vorblocks     */
#if defined _RISCC_ || defined _OSFC_ || defined _GNUC_ || defined _SUNGC_ || defined _MSCWIN_
      read_rvb( &rvb, stream->fp );
#else
      fread( &rvb, (size_t)sizeof( RVB ),
             (size_t)1, stream->fp );     /* ... und RVB einlesen     */
#endif
      fseek( stream->fp, (size_t)((rvb.vblocks + rvb.rdblocks) * BL_SIZE),
       SEEK_CUR );      /* hinter letzt.Datenblock  */
   }

   /* ----- Systemfehler bei File-E/A pruefen -----
   */
   if ( ferror( stream->fp ) )
   {
      perror( "dnorm sys error" );
      exit( 1 );
   }

   derrno = stream->err = DNERR_OK;
   return( items );
}
/* end of dread() */



/* ---------------------------------------------------------------------------
.CSD:  dwrite

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
t_i4 dwrite( void * buffer, t_i4 vsize, t_i4 n, DNORM_DCB * stream )
{
   long   rba;
   long    items;
   int    i;
   RVB    rvb;
   long    dat_bytes;


   /* ---------- stream-Eigenschaften testen ----------
   */
   if ( stream == NULL || stream->fp == NULL )  /* garnicht offen!          */
   {
      derrno = DNERR_CLSTREAM;
      return( 0 );
   }
   if ( stream->fopen_mode == 'r' )    /* Lesedatei!               */
   {
      derrno = DNERR_NOWRITESTREAM;
      return( 0 );
   }

   /* ---------- Limits kontrollieren ----------
   */
   if ( buffer == NULL )  
   {
      derrno = DNERR_NOWRITETARGET;
      return( 0 );
   }
   if ( vsize == 0L || n == 0L )  
   {
      derrno = DNERR_NOWRITEOBJS;
      return( 0 );
   }

   /* ---------- Objekte in Datei schreiben ----------
   */
   rba = ftell( stream->fp );                   /* akt Pos. merken          */
   fseek( stream->fp, stream->rvb_rba,
    SEEK_SET );        /* Anfang des Vorblocks     */
#if defined _RISCC_ || defined _OSFC_ || defined _GNUC_ || defined _SUNGC_ || defined _MSCWIN_
   read_rvb( &rvb, stream->fp );
#else
   fread( &rvb, (size_t)sizeof( RVB ),
          (size_t)1, stream->fp );     /* ... und RVB einlesen     */
#endif
   fseek( stream->fp, rba - stream->w_rest,  /* und zurueck auf die...   */
    SEEK_SET );        /* alte Position - w_rest!! */

#if defined _TC_ || defined _BCWIN_ || defined _GNUC_ || defined _SUNGC_
   /* ----- unter DOS-Turbo C ist size_t == unsigned short 
            deshalb mu˜ bei gro˜en Feldern mehrfach gelesen werden -----
   */
   {
      char HUGE * bpt = buffer;
      long write_bytes, nbytes, nwrite, objects;

      nbytes = vsize * n;
      write_bytes = 0;
      do
      {
         nwrite = nbytes;
         if ( nwrite >= SHRT_MAX )
            nwrite = (SHRT_MAX/vsize)*vsize;
#if defined _SUNGC_
         myswab_data( bpt, nwrite / vsize, stream );
#endif
         objects = fwrite( bpt, (size_t)sizeof( *bpt ), (size_t)nwrite, stream->fp );
#if defined _SUNGC_
         myswab_data( bpt, objects / vsize, stream );
#endif
         if ( !objects )
         {
            derrno = stream->err = DNERR_SYSWRITE;
            break;
         }      
         bpt += objects;
         nbytes -= objects;
         write_bytes += objects;
      } while ( nbytes > 0 && objects );

      items = write_bytes / vsize; 
      if ( write_bytes % vsize )      
         derrno = stream->err = DNERR_SYSWRITE;
   } 
#else
   items = fwrite( buffer, (size_t)vsize,  /* Realisierungsdaten schr. */
       (size_t)n, stream->fp );
#endif

   if ( items != n )      
      derrno = stream->err = DNERR_SYSWRITE;

   /* ---------- neue Vektoranzahl ermitteln und Realisierungs
                 vorblock entsprechend korrigieren, Rest
                 des Realisierungsdatenblocks mit 0en auffuellen ----------
   */
   rvb.vanz += n;        /* Vektoranzahl korrigieren */
   dat_bytes = rvb.vanz * vsize;    /* Realis. hat soviel bytes */
   rvb.rdblocks = dat_bytes / BL_SIZE;    /* soviel Datenbloecke      */
   dat_bytes %= BL_SIZE;      /* uebrige Daten-bytes      */
   if ( dat_bytes )        /* noch Daten-bytes vorhand.*/
   {
      for ( i = BL_SIZE; i > dat_bytes; i-- )   /* angerissenen Block */
   fputc( '\0', stream->fp );    /* mit 0-en auffuellen */
      rvb.rdblocks++;        /* ein block mehr */
   }
   if ( dat_bytes )
      stream->w_rest = BL_SIZE - dat_bytes;  /* restbyte im letzt.Dat.blo*/
   else
      stream->w_rest = 0;

   rba = ftell( stream->fp );      /* Position merken          */
   fseek( stream->fp, stream->rvb_rba,
    SEEK_SET );        /* Anfang des Vorblocks     */
#if defined _RISCC_ || defined _OSFC_ || defined _GNUC_ || defined _SUNGC_ || defined _MSCWIN_
   write_rvb( &rvb, stream->fp );
#else
   fwrite( &rvb, (size_t)sizeof( RVB ),   /* Vorblock neu schreiben...*/
     (size_t)1, stream->fp );             /* (ueberschreiben),        */
#endif
   fseek( stream->fp, rba,                 /* und zurueck auf die alte */
          SEEK_SET );        /* Pos.(hinter letzt.Dat.bl)*/

   /* ----- Systemfehler bei File-E/A pruefen -----
   */
   if ( ferror( stream->fp ) )
   {
      perror( "dnorm sys error" );
      exit( 1 );
   }

   derrno = stream->err = DNERR_OK;
   return( items );
} /* end of dwrite() */



/* ===========================================================================
   END OF FILE dread.c
   ===========================================================================
*/
