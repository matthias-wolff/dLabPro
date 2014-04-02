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

   NAME:  dsvl.c  -> "DNORM several functions"

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
#include <stdarg.h>

/* MWX 2001-10-01
   PATCH: DLPASSERT macro definition not found in relase mode (MSVC)
   --> */
#ifndef DLPASSERT
#define DLPASSERT(A) if (A) {}
#endif
/* <-- */


/* ===========================================================================
   PUBLIC
   ===========================================================================
*/
/* ---------------------------------------------------------------------------
.CSD:  dparmget

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
short dparmget( DPARA * parms, short which, ... )
{
   va_list   ap;
   char *   a;
   long   la;
   short  ret;

#if defined _SUNGC_
#define short int
#endif

   va_start( ap, which );      /* last fix */
   switch( which )
   {
      case P_HWTYPE:
    *( va_arg( ap, short * ) ) = parms->hwtype;
    ret = 1;
    break;
      case P_DTYPE:
    strncpy( va_arg( ap, char * ), parms->dtype, 2 );
    ret = 1;
    break;
      case PA_XT:
    a = va_arg( ap, char * );
    la = va_arg( ap, int );
    memmove( a, parms->xt, la -1 );
    a[la] = '\0';
    ret = parms->lxt;
     break;
      case P_LXT:
    *( va_arg( ap, short * ) ) = parms->lxt;
    ret = 1;
    break;
      case PA_VDT:
    a = va_arg( ap, char * );
    la = va_arg( ap, int );
    memmove( a, parms->vdt, la );
    a[la] = '\0';
    ret = parms->lvdt;
     break;
      case P_LVDT:
    *( va_arg( ap, short * ) ) = parms->lvdt;
    ret = 1;
    break;
      case P_DRES1:
    *( va_arg( ap, double * ) ) = parms->dres1;
    ret = 1;
    break;
      case P_DRES2:
    *( va_arg( ap, double * ) ) = parms->dres2;
    ret = 1;
    break;
      case P_DRES3:
    *( va_arg( ap, double * ) ) = parms->dres3;
    ret = 1;
    break;
      case P_DRES4:
    *( va_arg( ap, double * ) ) = parms->dres4;
    ret = 1;
    break;
      case P_DRES5:
    *( va_arg( ap, double * ) ) = parms->dres5;
    ret = 1;
    break;

      case P_KNR:
    *( va_arg( ap, short * ) ) = parms->knr;
    ret = 1;
    break;
      case P_RNR:
    *( va_arg( ap, short * ) ) = parms->rnr;
    ret = 1;
    break;
      case P_VANZ:
    *( va_arg( ap, long * ) ) = parms->vanz;
    ret = 1;
    break;
      case P_VDIM:
    *( va_arg( ap, long * ) ) = parms->vdim;
    ret = 1;
    break;
      case P_VSIZE:
    *( va_arg( ap, long * ) ) = parms->vsize;
    ret = 1;
    break;
      case P_ZF:
    *( va_arg( ap, double * ) ) = parms->zf;
    ret = 1;
    break;
      case P_FSR:
    *( va_arg( ap, double * ) ) = parms->fsr;
    ret = 1;
    break;
      case P_OFS:
    *( va_arg( ap, double * ) ) = parms->ofs;
    ret = 1;
    break;
      case P_RRES1:
    *( va_arg( ap, double * ) ) = parms->rres1;
    ret = 1;
    break;
      case P_RRES2:
    *( va_arg( ap, double * ) ) = parms->rres2;
    ret = 1;
    break;
      case P_RRES3:
    *( va_arg( ap, double * ) ) = parms->rres3;
    ret = 1;
    break;
      case P_RRES4:
    *( va_arg( ap, double * ) ) = parms->rres4;
    ret = 1;
    break;
      case P_RRES5:
    *( va_arg( ap, double * ) ) = parms->rres5;
    ret = 1;
    break;
      case PA_RB:
    a = va_arg( ap, char * );
    la = va_arg( ap, int );
    memmove( a, parms->rb, la );
    a[la] = '\0';
    ret = parms->lrb;
     break;
      case P_LRB:
    *( va_arg( ap, short * ) ) = parms->lrb;
    ret = 1;
    break;
      case PA_RT:
    a = va_arg( ap, char * );
    la = va_arg( ap, int );
    memmove( a, parms->rt, la );
    a[la] = '\0';
    ret = parms->lrt;
     break;
      case P_LRT:
    *( va_arg( ap, short * ) ) = parms->lrt;
    ret = 1;
    break;
      case PA_DT:
    a = va_arg( ap, char * );
    la = va_arg( ap, int );
    memmove( a, parms->dt, la );
    a[la] = '\0';
    ret = parms->ldt;
     break;
      case P_LDT:
    *( va_arg( ap, short * ) ) = parms->ldt;
    ret = 1;
    break;
      case PA_HT:
    a = va_arg( ap, char * );
    la = va_arg( ap, int );
    memmove( a, parms->ht, la );
    a[la] = '\0';
    ret = parms->lht;
     break;
      case P_LHT:
    *( va_arg( ap, short * ) ) = parms->lht;
    ret = 1;
    break;
      case PA_VRT:
    a = va_arg( ap, char * );
    la = va_arg( ap, int );
    memmove( a, parms->vrt, la );
    a[la] = '\0';
    ret = parms->lvrt;
     break;
      case P_LVRT:
    *( va_arg( ap, short * ) ) = parms->lvrt;
    ret = 1;
    break;
      default:
    ret = 0;
     break;
   }
   va_end( ap );

#if defined _SUNGC_
#undef short
#endif

   return( ret );
} /* end of dparmget() */




/* ---------------------------------------------------------------------------
.CSD:  dparmset

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
short dparmset( DPARA * parms, short which, ... )
{
   va_list   ap;
   char *   a;
   long   la;
   short  ret;

#if defined _SUNGC_
#define short int
#endif

   va_start( ap, which );      /* last fix */
   switch( which )
   {
      case P_FREE:
    if ( parms->xt && parms->lxt )    dn_free( parms->xt );
    if ( parms->vdt && parms->lvdt )  dn_free( parms->vdt );
    if ( parms->rb && parms->lrb )    dn_free( parms->rb );
    if ( parms->rt && parms->lrt )    dn_free( parms->rt );
    if ( parms->dt && parms->ldt )    dn_free( parms->dt );
    if ( parms->ht && parms->lht )    dn_free( parms->ht );
    if ( parms->vrt && parms->lvrt )  dn_free( parms->vrt );
      case P_CLEAR:
    memset( parms, 0, sizeof( DPARA ) );
    ret = 1;
    break;
      case P_FREE_R:
    if ( parms->rb && parms->lrb )    dn_free( parms->rb );
    if ( parms->rt && parms->lrt )    dn_free( parms->rt );
    if ( parms->dt && parms->ldt )    dn_free( parms->dt );
    if ( parms->ht && parms->lht )    dn_free( parms->ht );
    if ( parms->vrt && parms->lvrt )  dn_free( parms->vrt );
      case P_CLEAR_R:
                memset( &parms->knr, 0, sizeof( DPARA )
                        - (((char *)&parms->knr) - ((char *)parms)) );
    ret = 1;
    break;
      case P_DUPLICATE:
                {
                   DPARA * tparms;

                   /* ----- Strukturspeicher allokieren -----
                   */
                   tparms = va_arg( ap, DPARA * );
                   memset( tparms, 0, sizeof( DPARA ) );

                   /* ----- Strukturinhalt kopieren -----
                   */
                   *tparms = *parms;

                   /* ----- Arrays explizit allokieren & kopieren -----
                   */
                   if ( tparms->xt && tparms->lxt )
                   {
                      tparms->xt = dn_malloc( tparms->lxt +1 );
                      memmove( tparms->xt, parms->xt, tparms->lxt );
                      tparms->xt[tparms->lxt] = '\0';
                   }
                   if ( tparms->vdt && tparms->lvdt )
                   {
                      tparms->vdt = dn_malloc( tparms->lvdt +1 );
                      memmove( tparms->vdt, parms->vdt, tparms->lvdt );
                      tparms->vdt[tparms->lvdt] = '\0';
                   }
                   if ( tparms->rb && tparms->lrb )
                   {
                      tparms->rb = dn_malloc( tparms->lrb +1 );
                      memmove( tparms->rb, parms->rb, tparms->lrb );
                   }
                   if ( tparms->rt && tparms->lrt )
                   {
                      tparms->rt = dn_malloc( tparms->lrt +1 );
                      memmove( tparms->rt, parms->rt, tparms->lrt );
                      tparms->rt[tparms->lrt] = '\0';
                   }
                   if ( tparms->dt && tparms->ldt )
                   {
                      tparms->dt = dn_malloc( tparms->ldt +1 );
                      memmove( tparms->dt, parms->dt, tparms->ldt );
                      tparms->dt[tparms->ldt] = '\0';
                   }
                   if ( tparms->ht && tparms->lht )
                   {
                      tparms->ht = dn_malloc( tparms->lht +1 );
                      memmove( tparms->ht, parms->ht, tparms->lht );
                      tparms->ht[tparms->lht] = '\0';
                   }
                   if ( tparms->vrt && tparms->lvrt )
                   {
                      tparms->vrt = dn_malloc( tparms->lvrt +1 );
                      memmove( tparms->vrt, parms->vrt, tparms->lvrt );
                      tparms->vrt[tparms->lvrt] = '\0';
                   }
                }
    ret = 1;
    break;

      case P_HWTYPE:
    parms->hwtype = va_arg( ap, int );
    ret = 1;
    break;
      case P_DTYPE:
    strncpy( parms->dtype, va_arg( ap, char * ), 2 );
    ret = 1;
    break;
      case PA_XT:
    a = va_arg( ap, char * );
    la = va_arg( ap, int );
    if ( parms->xt && parms->lxt )
       dn_free( parms->xt );
    parms->xt = dn_malloc( la +1 );
                parms->lxt = la;
    memmove( parms->xt, a, la );
    ret = 1;
     break;
      case PA_VDT:
    a = va_arg( ap, char * );
    la = va_arg( ap, int );
    if ( parms->vdt && parms->lvdt )
       dn_free( parms->vdt );
    parms->vdt = dn_malloc( la +1 );
                parms->lvdt = la;
    memmove( parms->vdt, a, la );
    ret = 1;
     break;
      case P_DRES1:
    parms->dres1 = va_arg( ap, double );
    ret = 1;
    break;
      case P_DRES2:
    parms->dres2 = va_arg( ap, double );
    ret = 1;
    break;
      case P_DRES3:
    parms->dres3 = va_arg( ap, double );
    ret = 1;
    break;
      case P_DRES4:
    parms->dres4 = va_arg( ap, double );
    ret = 1;
    break;
      case P_DRES5:
    parms->dres5 = va_arg( ap, double );
    ret = 1;
    break;

      case P_KNR:
    parms->knr = va_arg( ap, int );
    ret = 1;
    break;
      case P_RNR:
    parms->rnr = va_arg( ap, int );
    ret = 1;
    break;
      case P_VANZ:
    parms->vanz = va_arg( ap, long );
    ret = 1;
    break;
      case P_VDIM:
    parms->vdim = va_arg( ap, long );
    ret = 1;
    break;
      case P_VSIZE:
    parms->vsize = va_arg( ap, long );
    ret = 1;
    break;
      case P_ZF:
    parms->zf = va_arg( ap, double );
    ret = 1;
    break;
      case P_FSR:
    parms->fsr = va_arg( ap, double );
    ret = 1;
    break;
      case P_OFS:
    parms->ofs = va_arg( ap, double );
    ret = 1;
    break;
      case P_RRES1:
    parms->rres1 = va_arg( ap, double );
    ret = 1;
    break;
      case P_RRES2:
    parms->rres2 = va_arg( ap, double );
    ret = 1;
    break;
      case P_RRES3:
    parms->rres3 = va_arg( ap, double );
    ret = 1;
    break;
      case P_RRES4:
    parms->rres4 = va_arg( ap, double );
    ret = 1;
    break;
      case P_RRES5:
    parms->rres5 = va_arg( ap, double );
    ret = 1;
    break;
      case PA_RB:
    a = va_arg( ap, char * );
    la = va_arg( ap, int );
    if ( parms->rb && parms->lrb )
       dn_free( parms->rb );
    parms->rb = dn_malloc( la +1 );
                parms->lrb = la;
    memmove( parms->rb, a, la );
    ret = 1;
     break;
      case PA_RT:
    a = va_arg( ap, char * );
    la = va_arg( ap, int );
    if ( parms->rt && parms->lrt )
       dn_free( parms->rt );
    parms->rt = dn_malloc( la +1 );
                parms->lrt = la;
    memmove( parms->rt, a, la );
    ret = 1;
     break;
      case PA_DT:
    a = va_arg( ap, char * );
    la = va_arg( ap, int );
    if ( parms->dt && parms->ldt )
       dn_free( parms->dt );
    parms->dt = dn_malloc( la +1 );
                parms->ldt = la;
    memmove( parms->dt, a, la );
    ret = 1;
     break;
      case PA_HT:
    a = va_arg( ap, char * );
    la = va_arg( ap, int );
    if ( parms->ht && parms->lht )
       dn_free( parms->ht );
    parms->ht = dn_malloc( la +1 );
                parms->lht = la;
    memmove( parms->ht, a, la );
    ret = 1;
     break;
      case PA_VRT:
    a = va_arg( ap, char * );
    la = va_arg( ap, int );
    if ( parms->vrt && parms->lvrt )
       dn_free( parms->vrt );
    parms->vrt = dn_malloc( la +1 );
                parms->lvrt = la;
    memmove( parms->vrt, a, la );
    ret = 1;
     break;
      default:
    ret = 0;
     break;
   }
   va_end( ap );

#if defined _SUNGC_
#undef short
#endif

   return( ret );
}
/* end of dparmset() */



/* ---------------------------------------------------------------------------
.CSD:  dsize

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
short dsize( DNORM_DCB * stream )
{
   KR_NODE * node;
   short i = 0;

   if ( ( node = search_tree( stream->tree,
         0, 0, SEARCH_FIRST ) ) != NULL ) {};  /* erste Realis.    */
      i++;

   while ( ( node = search_tree( stream->tree,
      node->knr, node->rnr,
      SEARCH_NEXT ) ) != NULL )     /* Realisierung  su.*/
      i++;

   return( i );
}
/* end of dsize() */




/* ===========================================================================
   INTERNAL
   ===========================================================================
*/

#ifndef _DEBUG

void * dn_malloc( long n )
{
   char * p;
  DLPASSERT(FALSE);

   if ( n == 0 )
      return( NULL );

#if defined _TC_ || defined _BCWIN_ || defined _MSCWIN_
#if defined __COMPACT__ || defined __LARGE__ || defined __HUGE__
   p = farmalloc( n );
#else
   p = malloc( n );
#endif
#elif defined _RISCC_ || defined _OSFC_ || defined _GNUC_ || defined _SUNGC_
   p = malloc( n );
#endif

   if ( p == NULL )
   {
      printf( "dn3base runtime: out of memory" );
      exit( 1 );
   }

   /*MWX 00-12-18: Clear memory */
   memset(p,0,n);
   /*<-- MWX*/

   return( (void *)p );
} /* end of dn_malloc() */

/*
void * dn_calloc( long n )
{
   char * p;
  DLPASSERT(FALSE);

   if ( n == 0L )
      return( NULL );

#if defined _TC_ || defined _BCWIN_ || defined _MSCWIN_
#if defined __COMPACT__ || defined __LARGE__ || defined __HUGE__
   p = farcalloc( n, 1 );
#else
   p = calloc( n, 1 );
#endif
#elif defined _RISCC_ || defined _OSFC_ || defined _GNUC_ || defined _SUNGC_
   p = calloc( n, 1 );
#endif

   if ( p == NULL )
   {
      printf( "dn3base runtime: out of memory" );
      exit( 1 );
   }

   return( (void *)p );
} / * end of dn_calloc() */



void dn_free( void * pt )
{
  DLPASSERT(FALSE);

   if ( !pt )
      return;

#if defined _TC_ || defined _BCWIN_ || defined _MSCWIN_
#if defined __COMPACT__ || defined __LARGE__ || defined __HUGE__
   farfree( pt );
#else
   free( pt );
#endif
#elif defined _RISCC_ || defined _OSFC_ || defined _GNUC_ || defined _SUNGC_
   free( pt );
#endif

   return;
} /* end of dn_free() */

#endif   /* #ifndef _DEBUG */


/* dn_getfmtdate:
*/
#include <time.h>
static char dn_time[31];

char * dn_getfmtdate( void )
{
   time_t t;
   struct tm *area;

   t = time(NULL);
   area = localtime(&t);

   strftime( dn_time, 30, "%a - %d.%b.%y - %H:%M:%S", area );
   return( dn_time );
} /* End of dn_getfmtdate */


/* ---------------------------------------------------------------------------
   SUN SPARC GNU C
   ---------------------------------------------------------------------------
*/
#if defined _SUNGC_

void * memmove( void * s1, const void * s2, size_t n )
{
   size_t i;

   if ( !s1 || !s2 )
      return( NULL );
   for ( i = 0; i < n; i++ )
      ((char *)s1)[i] = ((char *)s2)[i];

   return( s1 );
} /* end of memmove() */

void * memset( void * s, int c, size_t n )
{
   size_t i;

   if ( !s )
      return( NULL );
   for ( i = 0; i < n; i++ )
      ((char *)s)[i] = c;

   return( s );
} /* end of memset() */

/* ME: already defined in libc * /
int atexit( void (* function)( void ) )
{
   function = function;
   return( 1 );
} / * end of atexit() */

void myswab( void * a, long size, long n )
{
   long   i,j;
   char * tmp, * pt;

   tmp = (char *)dn_malloc( size >> 1 );
   for ( size >>= 1, pt = a; size > 0; size >>= 1, pt = a, n <<= 1 )
   {
      for ( i = 0; i < n; i++, pt += size << 1 )
      {
         memmove( tmp, pt, size );
         memmove( pt, pt + size, size );
         memmove( pt + size, tmp, size );
      }
   }
   dn_free( tmp );

   return;
} /* end of myswab() */

void myswab_data( void * dat, long vanz, DNORM_DCB * stream )
{
   size_t   pos;
   RVB     rvb;
   VRVB    vrvb;
   char *  buffer;    /* Puffer fuer Daten des var.Rea.vorblocks  */
   char *  pt;
   long    c, v, read_bytes;

   /* ----- RB der aktuellen Realisierung ermitteln -----
   */
   pos = ftell( stream->fp );
   fseek( stream->fp, stream->rvb_rba, SEEK_SET );
   read_bytes = read_rvb( &rvb, stream->fp );
   buffer = pt = dn_malloc( ((size_t)rvb.vblocks * BL_SIZE) - read_bytes );
   fread( pt, ((size_t)rvb.vblocks * BL_SIZE) - read_bytes, (size_t)1, stream->fp );
   memset( &vrvb, '\0', sizeof( vrvb ) );
   pt += sizeof( vrvb.rb_sign );
   vrvb.lrb = *(short *)pt;
   myswab( &vrvb.lrb, sizeof( vrvb.lrb ), 1 );
   pt += sizeof( vrvb.lrb );
   vrvb.rb = (RB *)pt;

   /* ----- myswab -----
   */
   for ( v = 0; v < vanz; v++ )
   {
      for ( c = 0; c < rvb.vdim; c++ )
      {
         if ( toupper( vrvb.rb[c].format ) != 'A' )
            myswab( dat, vrvb.rb[c].size, 1 );
         dat += vrvb.rb[c].size;
      }
   }

   dn_free( buffer );
   fseek( stream->fp, pos, SEEK_SET );

   return;
} /* end of myswab_data() */


size_t fread_myswab( void * pointer, size_t size, size_t num_items, FILE * stream )
{
   size_t n;

   n = fread( pointer, size, num_items, stream );
   if ( n > 0 )
      myswab( pointer, size, num_items );

   return( n );
} /* end of fread_myswab() */

size_t fwrite_myswab( void * pointer, size_t size, size_t num_items, FILE * stream )
{
   size_t n;

   myswab( pointer, size, num_items );
   n = fwrite( pointer, size, num_items, stream );
   myswab( pointer, size, num_items );

   return( n );
} /* end of fread_myswab() */

#endif /*defined _SUNGC_*/

/* ===========================================================================
   END OF FILE dsvl.c
   ===========================================================================
*/
