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

   NAME:  krtree.c  -> "DNORM internal tree functions"

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
   INTERNAL
   ===========================================================================
*/
/* interne Funktionen */

static KR_NODE * search_krn( KR_NODE * p );
static KR_NODE * search_next( KR_NODE * pt );
static KR_NODE * search_first( KR_NODE * root );
static KR_NODE * search_last( KR_NODE * root );
static KR_NODE * add_or_change( KR_NODE * root );
static KR_NODE * pb_tree( long n, FILE * fp );
static void      write_tree( KR_NODE *p, FILE * fp );
static long    i_index( short knr, short rnr );
/*static void    krn( long index, short * knr, short * rnr );*/


/* global definie rte statische Zellen */

static FNODE_INFO info;                 /* Temp.Zelle fuer Lesen und Schreib*/

static short s_knr;
static short s_rnr;
static KR_NODE * s_next_object;

static int s_offset;

static short a_knr;
static short a_rnr;
static long  a_rba;


/* load_tree: Laden des Realisierungsbaumes von einer Datei als perfekt
        ausbalancierten Binaerbaum
*/

KR_NODE * load_tree(        /* Laden Baum aus der Dnorm-Datei */
  FILE * fp,                      /* Filezeig.muss auf Dummy-Info steh*/
  t_i4 * objects )    /* Knoten/Obj./Realis.anzahl */
{                                       
   KR_NODE * root = NULL;               /* Baumwurzel */

   if(fread( &info, (size_t)sizeof( info ), (size_t)1, fp ) != 1) return NULL;
#if defined _SUNGC_
   myswab( &info.knr, sizeof( info.knr ), 1 );  
   myswab( &info.rnr, sizeof( info.rnr ), 1 );  
   myswab( &info.rba, sizeof( info.rba ), 1 ); 
#endif
   *objects = info.rba;      /* Dummy-Info:rba enthaelt obj.anz. */

   root = pb_tree( *objects, fp );  /* Baum aufbauen */

   return( root );
}
/* end of load_tree() */



/* del_tree: Loeschen des Realisierungsbaumes im Hauptspeicher
*/

void del_tree( 
  KR_NODE ** rootpt )
{
   if ( *rootpt != NULL )
   {
      del_tree( &( *rootpt )->left );
      del_tree( &( *rootpt )->right );
      dn_free( *rootpt );
      *rootpt = NULL;
   }

   return;
}
/* end of del_tree() */



/* search_tree: Suchen eines Knotens im Binaerbaum
                globaler Eintrittspunkt fuer verschiedene Suchkriterien
*/

KR_NODE * search_tree(
  KR_NODE * root,
  short   knr,
  short   rnr,
        int  mode )
{
   KR_NODE * node;

   s_knr = knr;
   s_rnr = rnr;

   switch( mode )
   {
      case SEARCH_KRN:                    /* Suchen mit vollstaendiger*/
   node = search_krn( root );             /* knr/rnr-Spezifikation    */
   break;
      case SEARCH_NEXT_K:
      case SEARCH_NEXT_R:
      case SEARCH_NEXT:        /* Suchen des naechsten Obj.*/
   s_next_object = NULL;      /* Standard: kein naechster */
   search_next( root );
   node = s_next_object;
   break;
      case SEARCH_FIRST:
   node = search_first( root );
   break;
      case SEARCH_LAST:
   node = search_last( root );
   break;
      default:
   return( NULL );
   }

   return( node );
}
/* end of search_tree() */



/* search_krn: Suchen eines Knotens entsprechend vollstaendig
         angegebener knr/rnr im Binaerbaum
*/

static KR_NODE * search_krn( 
  KR_NODE * p )
{
   long indic;

   if ( p == NULL )        /* keine Unterbaeume */
      return( NULL );

   indic = i_index( s_knr, s_rnr )
           - i_index( p->knr, p->rnr );    /* > 0, wenn p-index < s-ind*/
   return( indic < 0L ?
     search_krn( p->left ) :    /* linken Unterbaum absuchen*/
     indic > 0L ?
     search_krn( p->right ) :              /* rechten Unterb. absuchen */
     p->rba == -1L ? NULL :          /* Realisierung geloescht   */
     p                         /* gefunden!                */
   );
}
/* end of search_krn() */




/* search_next: Suchen eines Knotens mit naechsthoeherer Klassen-
    /Realisierungsnummer nummer im Binaerbaum
*/

static KR_NODE * search_next( 
  KR_NODE * pt )
{
   long indic;

   if ( pt == NULL )
      return( NULL );

   indic = i_index( pt->knr, pt->rnr )
           - i_index( s_knr, s_rnr );
   if ( indic <= 0L )
      return( search_next( pt->right ) );
   else
   {
      s_next_object = pt;
      return( search_next( pt->left ) );
   }
}
/* end of search_next() */



/* search_first: Suchen des allerersten Knotens im
     Binaerbaum
*/

static KR_NODE * search_first( 
  KR_NODE * root )
{
   KR_NODE * p = root;

   while( p->left != NULL )
      p = p->left;

   return( p );
}
/* End of search_first() */



/* search_last: Suchen des allerletzten Knotens im
    Binaerbaum
*/

static KR_NODE * search_last( 
  KR_NODE * root )
{
   KR_NODE * p = root;

   while( p->right != NULL )
      p = p->right;

   return( p );
}
/* End of search_tree_last() */



/* add_tree:
*/

KR_NODE * add_tree( 
  KR_NODE * root,
  short knr,
  short rnr,
  long rba )
{
   a_knr = knr;
   a_rnr = rnr;
   a_rba = rba;

   return( add_or_change( root ) );   /* NULL: Element schon vorhanden!   */
}
/* end of add_tree() */



/* add_or_change:
*/

static KR_NODE * add_or_change( 
  KR_NODE * p )
{
   long indic;

   if ( p == NULL )        /* jetzt einfuegen! */
   {
      p = dn_malloc( sizeof( KR_NODE ) );  /* Speicher fuer Knoten */
      p->knr = a_knr;
      p->rnr = a_rnr;
      p->rba = a_rba;
      p->left = p->right = NULL;    /* keine Unterbaeume */
   }
   else
   {
      indic = i_index( a_knr, a_rnr )
              - i_index( p->knr, p->rnr );        /* > 0, wenn p_knr < s_knr */
      if ( indic < 0 )
   p->left = add_or_change( p->left );
      else if ( indic > 0 )
   p->right = add_or_change( p->right );
   }

   return( p );
}
/* end of add_or_change() */



/* pb_tree: Lesen der Dnorm-Knoteninformationen in einen perfekt
      ausbalancierten Binaerbaum
*/

static KR_NODE * pb_tree(     /* erzeugt perf.ausbal.Baum */
  long n,                         /* Anzahl der Objekte (Realis.) */
  FILE * fp )
{
   long nleft;
   long nright;
   KR_NODE * p;


   if ( n == 0 )            /* Aubbruchbedingung f. Rekursion */
      return( NULL );

   nleft = n >> 1;      /* Obj.-Zahl halbieren->rechts-Obj. */
   nright = n - nleft -1;    /* Zahl der Obj. im linken Baum */

   p = dn_malloc( sizeof( KR_NODE ) );  /* Speicher fuer Baumknoten reserv. */
   p->left = pb_tree( nleft, fp );  /* linken Unterbaum behandeln */

   if(fread( &info, (size_t)sizeof( info ),
    (size_t)1, fp ) != 1) return NULL;              /* Knoteninfo aus Datei lesen */
#if defined _SUNGC_
   myswab( &info.knr, sizeof( info.knr ), 1); 
   myswab( &info.rnr, sizeof( info.rnr ), 1 ); 
   myswab( &info.rba, sizeof( info.rba ), 1 ); 
#endif
   p->knr = info.knr;
   p->rnr = info.rnr;
   p->rba = info.rba;      /* rel.byte-Adresse ablegen */

   p->right = pb_tree( nright, fp );  /* rechten Unterbaum bearbeiten */

   return( p );        /* generierten Knoten zurueck */
}
/* end of pb_tree() */



/* save_tree: Sichern des Baums in eine Datei (Dateizeiger muss auf der
        ersten schreibbaren Bytezelle stehen!)
*/

void save_tree( 
  FILE *     fp,
  KR_NODE * root,
  long       objects,
        short     offset )
{
   memset( &info, '\0', sizeof( info ) );
   info.rba = objects;      /* Dummy-Info:rba enthaelt obj.anz. */

   s_offset = offset;

#if defined _SUNGC_
   myswab( &info.knr, sizeof( info.knr ), 1 ); 
   myswab( &info.rnr, sizeof( info.rnr ), 1 ); 
   myswab( &info.rba, sizeof( info.rba ), 1 ); 
#endif
   if(fwrite( &info, (size_t)sizeof( info ),
     (size_t)1, fp ) != 1) return;             /* Pseudo-Info schreiben */
#if defined _SUNGC_
   myswab( &info.knr, sizeof( info.knr ), 1 ); 
   myswab( &info.rnr, sizeof( info.rnr ), 1 ); 
   myswab( &info.rba, sizeof( info.rba ), 1 ); 
#endif
   write_tree( root, fp );    /* Baumknoten schreiben */

   return;
}
/* end of save_tree() */



/* write_tree: Schreiben der Baumknoten in Datei
*/

static void write_tree( 
  KR_NODE * p,
        FILE * fp )
{
   if ( p != NULL )
   {
      write_tree( p->left, fp );  /* linker Unterbaum */
      if ( p->rba != -1L )    /* -1L: Realisierung ist geloescht */
      {
   info.knr = p->knr;
   info.rnr = p->rnr;
   info.rba = p->rba + s_offset;  /* rel.byte-Adresse uebernehmen */
#if defined _SUNGC_
         myswab( &info.knr, sizeof( info.knr ), 1 ); 
         myswab( &info.rnr, sizeof( info.rnr ), 1 ); 
         myswab( &info.rba, sizeof( info.rba ), 1 ); 
#endif
   if(fwrite( &info, (size_t)sizeof( info ),
     (size_t)1, fp ) != 1) return;       /* Pseudo-Info schreiben */
#if defined _SUNGC_
         myswab( &info.knr, sizeof( info.knr ), 1 ); 
         myswab( &info.rnr, sizeof( info.rnr ), 1 ); 
         myswab( &info.rba, sizeof( info.rba ), 1 ); 
#endif
      }
      write_tree( p->right, fp );  /* rechten Unterbaum abhandeln */
   }

   return;
}
/* end of write_tree() */


/* i_index: Produziert eindeutigen Index aus Klassennummer (high-Teil) und
    Realisierungsnummer (low-Teil) in Form einer long-Zahl
    !!!Achtung: knr/rnr duerfen nur im Wertebereich 1...32000 liegen
*/

static long i_index( 
  short knr,
  short rnr )
{
   long i = knr;        /* knr im low-Teil */

   i <<= 16;                                    /* knr in high-Teil rein */
   return( i + rnr );        /* rnr draufadieren und */
}                                               /* zurueck */
/* end of i_index() */




/* krn: Produziert aus einem eindeutigen Index der im high-Teil knr und
  im low-Teil rnr repraesentiert die Klassen- und Realisierungsnummer
  !!!Achtung: knr/rnr duerfen nur im Wertebereich 1...32000 liegen
  es sind die Adressen der knr/rnr-Zellen zu uebergeben!!!
*/             

/* ME_21102004: defined but never used
static void krn(
  long index,
  short *knr,
  short *rnr )
{
   *rnr = (short)index;        / * low-Teil als rnr * /
   *knr = index >> 16;        / * high-Teil als knr * /
   return;
}
/ * end of krn() */


/* ===========================================================================
   END OF FILE krtree.c
   ===========================================================================
*/
