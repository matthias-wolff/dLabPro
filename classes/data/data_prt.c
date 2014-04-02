/* dLabPro class CData (data)
 * - Data content printing
 *
 * AUTHOR : Matthias Wolff
 * PACKAGE: dLabPro/classes
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

#include "dlp_cscope.h" /* Indicate C scope */
#include "dlp_data.h"

#define __TIMEOUT 2000

/**
 * Prints the instance in text mode (exactly one component of type 1).
 */
INT16 CGEN_PRIVATE CData_PrintText(CData* _this)
{
  INT32  nR   = 0;                                                               /* Current record                    */
  INT32  nXR  = 0;                                                               /* Number of records                 */
  char* tx   = NULL;                                                            /* Auxilary char pointer #1          */
  char* tx0  = NULL;                                                            /* Pointer to string                 */
  char* ty   = NULL;                                                            /* Auxilary char pointer #2          */
  char* ty0  = NULL;                                                            /* Pointer to last white space       */
  INT32  nCtr = 0;                                                               /* Line counter                      */
  char  sBuf[255];                                                              /* Printing buffer                   */

  /* Initialize */                                                              /* --------------------------------- */
  nXR = CData_GetNRecs(_this);                                                  /* Get number of records             */

  /* Print data contents as text */                                             /* --------------------------------- */
  DLPASSERT(FMSG("ALPHA: String mode not tested yet"));                         /* TODO: Remove after debugging      */
  printf("\n  String Mode"); dlp_inc_printlines(1);                             /* Protocol                          */
  tx0=tx=(char*)CData_XAddr(_this,0,0);                                         /* Initialize char pointers          */
  if (!*tx)                                                                     /* Empty string                      */
  {                                                                             /* >>                                */
    printf("\n  [empty]");                                                      /*   Protocol                        */
    dlp_inc_printlines(1);                                                      /*   Adjust no. of printed lines     */
  }                                                                             /* <<                                */
  else while (*tx)                                                              /* Loop over characters              */
  {                                                                             /* >>                                */
    /* Make line breaks */                                                      /*   - - - - - - - - - - - - - - - - */
    for (ty=tx,ty0=NULL; *ty && ty<tx0+nXR; )                                   /*   Do until end of string          */
    {                                                                           /*   >>                              */
      while (*ty && ty<tx0+nXR && !iswspace(*ty)) ty++;                         /*     Seek next white space         */
      while (*ty && ty<tx0+nXR &&  iswspace(*ty)) ty++;                         /*     Skip following white spaces   */
      ty0=ty;                                                                   /*     Remember previous white spc.  */
      if (ty>tx+dlp_maxprintcols()-16) break;                                   /*     Line full                     */
    }                                                                           /*   <<                              */
    if (ty0) ty=ty0;                                                            /*   Go back to last white space     */

    /* Print one line */                                                        /*   - - - - - - - - - - - - - - - - */
    dlp_memset(sBuf,0,255);                                                     /*   Clear printing buffer           */
    dlp_memmove(sBuf,tx,ty-tx);                                                 /*   Copy characters in              */
    printf("\n  %4d(%06d): %s",(int)nCtr,(int)(tx-tx0),sBuf);                   /*   Print 'em                       */
    dlp_inc_printlines(1);                                                      /*   Adjust no. of printed lines     */
    if (dlp_if_printstop()) break;                                              /*   Break listing                   */

    /* End-of-line actions */                                                   /*   - - - - - - - - - - - - - - - - */
    DLPASSERT(ty>tx);                                                           /*   Should have made some progress  */
    tx=ty;                                                                      /*   Move to end of printed line     */
    nCtr++;                                                                     /*   Increment line counter          */
  }                                                                             /* <<                                */
  if (nR>=nXR) printf("\n  No more data - Stop.");                              /* Protocol                          */
  else         printf("\n  Cancelled - Stop.");                                 /* Protocol                          */
  return O_K;                                                                   /* Ok                                */
}

/**
 * Prints the content of one record formatted as columns. If printing requires
 * more than <a href="dlp_base.html#cfn_dlp_maxprintcols">dlp_maxprintcols</a>
 * characters the listing will be continued on the next line(s).</p>
 *
 * @param _this
 *          Pointer to data instance
 * @param nRec
 *          Index of record to print
 * @param nIcFirst
 *          Index of first component to print
 * @param nComps
 *          Number of components to print
 * @param nIndent
 *          Indentation (spaces) at beginning of lines (<b>Note</b>: the first
 *          line will <em>not</em> be indented!)
 * @return The number of lines printed
 */
INT16 CGEN_PUBLIC CData_PrintRec
(
  CData* _this,
  INT32    nRec,
  INT32    nIcFirst,
  INT32    nComps,
  INT16   nIndent
)
{
  INT16 nLines = 1;
  INT16 nCol   = nIndent;
  INT32  nXC    = 0;
  INT32  nC     = 0;
  INT16 i      = 0;
  INT16 I      = 0;
  char  sBuf[L_SSTR+1];

  nXC = CData_GetNComps(_this);

  if (nIcFirst<0 || nIcFirst>=nXC) return 1; /* NOTE: This is still one line! */
  if (nIcFirst+nComps>nXC) nComps=nXC-nIcFirst;
  if (nRec>=CData_GetNRecs(_this)) return 0;

  for (nC=nIcFirst,nCol=nIndent; nC<nIcFirst+nComps; nC++)
  {
    if (nRec<0)
    {
      /* Heading */
      I=dlp_printlen(CData_GetCompType(_this,nC));
      strcpy(sBuf," ");
      for (i=I-(INT16)dlp_strlen(CData_GetCname(_this,nC))-1; i>0; i--) strcat(sBuf," ");
      if(CData_CompIsMarked(_this,nC)) sBuf[dlp_strlen(sBuf)-2]='*';
      strcat(sBuf,CData_GetCname(_this,nC));
      nCol+=dlp_printlen(CData_GetCompType(_this,nC)); /* Count standard width (!) */
      printf(sBuf);
    }
    else
    {
      /* Print values */
      dlp_sprintx(sBuf,(char*)CData_XAddr(_this,nRec,nC),CData_GetCompType(_this,nC),_this->m_bExact);
      nCol+=dlp_printlen(CData_GetCompType(_this,nC)); /* Count standard width (!) */
      dlp_strconvert(SC_PRC_ESCAPE,sBuf,sBuf);
      dlp_strreplace(sBuf,"\n","\\n");
      dlp_strreplace(sBuf,"\r","\\r");
      dlp_strreplace(sBuf,"\t","\\t");
      printf(sBuf);
    }

    if ((nC<nIcFirst+nComps-1)                                               &&
        (nCol+dlp_printlen(CData_GetCompType(_this,nC+1))>dlp_maxprintcols()) )
    {
      /* Line break */
      strcpy(sBuf,"\n");
      for (i=nIndent-(nIndent>7?7:0); i>0;i--) strcat(sBuf," ");
      printf(sBuf);
      if (nIndent>7) printf("%5ld  ",(long)(nC+1));
      nCol=nIndent;
      nLines++;
    }
  }

  return nLines;
}

/**
 * Prints the instance in list mode (old style, with option /list).
 */
INT16 CGEN_PUBLIC CData_PrintList(CData* _this)
{
  INT32 nR   = 0;                                                                /* Current record                    */
  INT32 nXR  = 0;                                                                /* Number of records                 */
  INT32 nXC  = 0;                                                                /* Number of components              */
  INT32 nRpb = 0;                                                                /* Number of records per block       */

  /* Initialize */                                                              /* --------------------------------- */
  nXR  = CData_GetNRecs(_this);                                                 /* Get number of records             */
  nXC  = CData_GetNComps(_this);                                                /* Get number of components          */
  nRpb = CData_GetNRecsPerBlock(_this);                                         /* Get number of records per block   */

  /* Print headings */                                                          /* --------------------------------- */
  printf("\n   Rec.(offset):");                                                 /* Protocol                          */
  dlp_inc_printlines(CData_PrintRec(_this,-1,0,nXC,16)-1);                      /* Protocol                          */

  /* Print values */                                                            /* --------------------------------- */
  for (nR=0; nR<nXR; )                                                          /* Loop over records                 */
  {                                                                             /* >>                                */
    printf("\n%c %4ld (%06ld):",CData_RecIsMarked(_this,nR)?'*':' ',(long)nR,   /*   Print line header               */
      (long)((char*)CData_XAddr(_this,nR,0)-(char*)CData_XAddr(_this,0,0)));    /*   |                               */
    dlp_inc_printlines(CData_PrintRec(_this,nR,0,nXC,16));                      /*   Print values                    */
    if (nRpb>0 && (nR+1)%nRpb==0)                                               /*   Block boundary                  */
    {                                                                           /*   >>                              */
      printf("\n - %c End of block %ld - - - - -",                              /*     Print block delimiter         */
        CData_BlockIsMarked(_this,nR/nRpb)?'*':' ',(long)nR/nRpb);              /*     |                             */
      dlp_inc_printlines(1);                                                    /*     Adjust no. of printed lines   */
    }                                                                           /*   <<                              */
    if ((nR=dlp_printstop_nix(nR,"record",NULL))==-1) break;                    /*   Break listing                   */
    if (nR< -2 ) nR=0;                                                          /*   Bad user reply -> start over    */
    if (nR>=nXR) break;                                                         /*   No more records -> break        */
  }                                                                             /* <<                                */
  if (nR>=nXR) printf("\n  No more data - Stop.");                              /* Protocol                          */
  else         printf("\n  Cancelled - Stop.");                                 /* Protocol                          */
  return O_K;                                                                   /* Ok                                */
}

/**
 * dlp_sprintx w/o space padding.
 */
INT32 __sprintx(char* sBuf, const void* nData, INT16 nType, BOOL bExact)
{
  dlp_sprintx(sBuf,(char*)nData,nType,bExact);
  dlp_strconvert(SC_PRC_ESCAPE,sBuf,sBuf);
  dlp_strreplace(sBuf,"\n","\\n");
  dlp_strreplace(sBuf,"\r","\\r");
  dlp_strreplace(sBuf,"\t","\\t");
  dlp_strtrimleft(dlp_strtrimright(sBuf));
  return strlen(sBuf);
}

/**
 * Utility function for CData_PrintVectors; pads a string up to a given length
 * with spaces.
 */
char* __pad
(
  char* sStr,                                                                   /* The string to pad                 */
  INT32  nLen,                                                                   /* The target length                 */
  char  nMode                                                                   /* Alignment: 'l', 'r' or 'c'        */
)                                                                               /* Returns sStr                      */
{
  char* tx = NULL;                                                              /* Current character in string       */
  if ((INT32)dlp_strlen(sStr)==nLen) return sStr;                                /* Nothing to be done                */
  if ((INT32)dlp_strlen(sStr)> nLen)                                             /* String longer than nLen           */
  {                                                                             /* >>                                */
    for (tx=sStr;tx<sStr+nLen;tx++) *tx='#';                                    /*   Make it "#######"               */
    *tx='\0';                                                                   /*   Terminate it                    */
    return sStr;                                                                /*   Return it                       */
  }                                                                             /* <<                                */
  while ((INT32)dlp_strlen(sStr)<nLen)                                           /* While string shorter than required*/
  {                                                                             /* >>                                */
    if (nMode=='c' || nMode=='r')                                               /*   Padding at left side            */
    {                                                                           /*   >>                              */
      dlp_memmove(&sStr[1],sStr,dlp_strlen(sStr)+1);                            /*     Move data                     */
      sStr[0]=' ';                                                              /*     Write heading space           */
      if (nMode=='c') nMode='C';                                                /*     Toggle centering flag         */
    }                                                                           /*   <<                              */
    else if (nMode=='C' || nMode=='l')                                          /*   Paddig at right side            */
    {                                                                           /*   >>                              */
      sStr[dlp_strlen(sStr)+2]='\0';                                            /*     Write second terminal 0       */
      sStr[dlp_strlen(sStr)+1]=' ';                                             /*     Write trailing space          */
      if (nMode=='C') nMode='c';                                                /*     Toggle centering flag         */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */
  return sStr;                                                                  /* Return sStr                       */
}

/**
 * Computes the print widths of the head column and one data vector.
 */
INT16 CGEN_PRIVATE CData_PrintVectors_GetColWidth
(
  CData* _this,                                                                 /* Pointer to data instance          */
  INT32   nR0,                                                                   /* First record to be printed        */
  INT32   nC0,                                                                   /* First component to be printed     */
  INT32*  lpnWI,                                                                 /* Print width of comp.idx.col.(ret) */
  INT32*  lpnW0,                                                                 /* Print width of head column (ret)  */
  INT32*  lpnW                                                                   /* Print width of data column (ret)  */
)                                                                               /* Returns O_K or (neg.) error code  */
{
  INT32   nR   = 0;                                                              /* Current record                    */
  INT32   nXR  = 0;                                                              /* Number of records                 */
  INT32   nC   = 0;                                                              /* Current component                 */
  INT32   nXC  = 0;                                                              /* Number of components              */
  INT32   nWn  = 0;                                                              /* Widest number in cols. of screen  */
  INT32   nWs  = 0;                                                              /* Widest string in cols. of screen  */
  FLOAT64 nBuf = 0.;                                                             /* Double buffer                     */
  char   sBuf[L_SSTR+1];                                                        /* String buffer                     */
  UINT64 nTime = 0;

  /* Initialize */                                                              /* --------------------------------- */
  *lpnW  = 0;                                                                   /* Data column width                 */
  *lpnWI = 0;                                                                   /* Component index column width      */
  *lpnW0 = 0;                                                                   /* Head column width                 */
  nXR    = CData_GetNRecs(_this);                                               /* Get number of records             */
  nXC    = CData_GetNComps(_this);                                              /* Get number of components          */

  /* Compute head column print width */                                         /* --------------------------------- */
  for (*lpnW0=0,nC=nC0; nC<nXC; nC++)                                           /* Loop over remaining components    */
  {                                                                             /* >>                                */
    if                                                                          /*   Displaying physical units?      */
    (                                                                           /*   |                               */
      dlp_is_numeric_type_code(CData_GetCompType(_this,nC)) &&                  /*   | Numeric component             */
      dlp_strlen(_this->m_lpCunit) && _this->m_nCinc!=0.                        /*   | Physical units specified      */
    )                                                                           /*   |                               */
    {                                                                           /*   >>                              */
      nBuf = _this->m_nCofs + nC*_this->m_nCinc;                                /*     The physical coordinate       */
      __sprintx(sBuf,&nBuf,T_DOUBLE,_this->m_bExact);                           /*     Print to a string             */
    }                                                                           /*   <<                              */
    else                                                                        /*   String comp. or no phys. units  */
      __sprintx(sBuf,CData_GetCname(_this,nC),10,_this->m_bExact);              /*     Print component name to str.  */
    *lpnW0 = MAX(*lpnW0,(INT32)dlp_strlen(sBuf));                               /*   Get length of phys. unit / name */
  }                                                                             /* <<                                */

  /* Compute component index column print width */                              /* --------------------------------- */
  nC--;                                                                         /* Last component to be printed      */
  __sprintx(sBuf,&nC,T_INT,_this->m_bExact);                                    /* Print greatest comp. index to str.*/
  *lpnWI = (INT32)dlp_strlen(sBuf);                                             /* Get length of component index col.*/

  /* Compute data vector print width */                                         /* --------------------------------- */
  nTime = dlp_time();
  for (*lpnW=0,nR=nR0; nR<nXR; nR++)                                            /* Loop over remaining records       */
  {                                                                             /* >>                                */
    /* Determine width of physical unit */                                      /*   - - - - - - - - - - - - - - - - */
    if (dlp_strlen(_this->m_lpRunit) && _this->m_lpTable->m_fsr!=0.)            /*   Displaying physical units?      */
    {                                                                           /*   >>                              */
      nBuf = _this->m_lpTable->m_ofs + nR*_this->m_lpTable->m_fsr;              /*     The physical coordinate       */
      __sprintx(sBuf,&nBuf,T_DOUBLE,_this->m_bExact);                           /*     Print to a string             */
      *lpnW = MAX((INT32)dlp_strlen(sBuf),*lpnW);                               /*     Aggregate actual print width  */
    }                                                                           /*   <<                              */

    /* Determine width of record index */                                       /*   - - - - - - - - - - - - - - - - */
    __sprintx(sBuf,&nR,T_INT,_this->m_bExact);                                  /*   Print record index to a string  */
    *lpnW = MAX((INT32)dlp_strlen(sBuf),*lpnW);                                 /*     Aggregate actual print width  */

    /* Determine greatest component width */                                    /*   - - - - - - - - - - - - - - - - */
    for (nC=nC0; nC<nXC; nC++)                                                  /*   Loop over remaining components  */
    {                                                                           /*   >>                              */
      __sprintx(sBuf,CData_XAddr(_this,nR,nC),                                  /*     Print cell value to a string  */
          CData_GetCompType(_this,nC),_this->m_bExact);                         /*       |                           */
      if (dlp_is_numeric_type_code(CData_GetCompType(_this,nC)))                /*     It is a number                */
        nWn = MAX((INT32)dlp_strlen(sBuf),nWn);                                 /*       Aggr. number print width    */
      else if (dlp_is_symbolic_type_code(CData_GetCompType(_this,nC)))          /*     It is a string                */
        nWs = MAX((INT32)dlp_strlen(sBuf),nWs);                                  /*       Aggr. string print width    */
      if (dlp_time()-nTime>__TIMEOUT) break;                                    /*      Takes too long -> forget it! */
    }                                                                           /*   <<                              */

    if (dlp_time()-nTime>__TIMEOUT) break;                                      /*    Takes too long -> forget it!   */
    if ((nR-nR0+2)*((*lpnW)+1)>dlp_maxprintcols()-*lpnWI-*lpnW0-3) break;       /*    Next vec. would not fit anymore*/
  }                                                                             /*  <<                               */

  /* If computing data vector print width timed out ... */                      /* --------------------------------- */
  if (dlp_time()-nTime>__TIMEOUT)                                               /* There was a time out              */
    for (nC=nC0; nC<nXC; nC++)                                                  /*   Loop over remaining components  */
      if (dlp_is_numeric_type_code(CData_GetCompType(_this,nC)))                /*     It is a number                */
        nWn = MAX(dlp_printlen(CData_GetCompType(_this,nC)),nWn);               /*       Use standard print width    */
      else if (dlp_is_symbolic_type_code(CData_GetCompType(_this,nC)))          /*     It is a string                */
        nWs = MAX(dlp_printlen(CData_GetCompType(_this,nC)),nWs);               /*       Also use std. print width   */

  /* Aftermath */                                                               /* --------------------------------- */
  if (nWs>dlp_maxprintcols()-(*lpnW0)-(*lpnWI)-3)                               /* Limit string width to line length */
    nWs=dlp_maxprintcols()-(*lpnW0)-(*lpnWI)-3;                                 /* ...                               */
  if (nWn<=0) *lpnW = MAX(nWs,*lpnW);                                           /* No numbers -> complete strings    */
  else *lpnW = MAX(nWn,*lpnW);                                                  /* Minimal space req. for numbers    */
  /*if (nWn>0 && *lpnW+3<nWs) (*lpnW) += 3;*/                                       /* Print a little more of the strs.  */
  if (nWn>0 && *lpnW<nWs) *lpnW = MIN(16,nWs);                                  /* Print max. 16 chars. of strings   */
  return O_K;                                                                   /* Ok                                */
}

/**
 * Prints one block of the instance in vector mode (standard).
 */
INT32 CGEN_PRIVATE CData_PrintVectors_Block
(
  CData* _this,                                                                 /* Pointer to data instance          */
  INT32   nBlock                                                                 /* Block index (<0: ignore blocks)   */
)                                                                               /* Returns number of lines printed   */
{
  INT32 i      = 0;                                                              /* Universal loop counter            */
  INT32 nR     = 0;                                                              /* Current record                    */
  INT32 nR_    = 0;                                                              /* First record to be printed        */
  INT32 nR0    = 0;                                                              /* First record of current page      */
  INT32 nSR    = 0;                                                              /* Number of records of currenr page */
  INT32 nXR    = 0;                                                              /* Last record to print plus one     */
  INT32 nC     = 0;                                                              /* Current component                 */
  INT32 nXC    = 0;                                                              /* Number of components              */
  INT32 nWI    = 0;                                                              /* Component index column width      */
  INT32 nW0    = 0;                                                              /* Head column width                 */
  INT32 nW     = 0;                                                              /* Column width                      */
  INT32 nP     = 0;                                                              /* Current page                      */
  INT32 nPps   = 0;                                                              /* Pages per screen                  */
  INT32 nL     = 0;                                                              /* Line counter                      */
  BOOL bPur   = FALSE;                                                          /* Print physical record unit flag   */
  BOOL bPuc   = FALSE;                                                          /* Print physical component unit flg.*/
  FLOAT64 nBuf = 0.;                                                             /* Double buffer                     */
  char   sBuf[L_SSTR+1];                                                        /* String buffer                     */

  /* Validate */                                                                /* --------------------------------- */
  if (nBlock>=CData_GetNBlocks(_this)) return 0;                                /* Requested block does not exist    */

  /* Initialize */                                                              /* --------------------------------- */
  nR_ = nBlock>=0 ? CData_GetNRecsPerBlock(_this)*nBlock : 0;                   /* Get first record to print         */
  nXR = nBlock>=0 ? nR_+CData_GetNRecsPerBlock(_this) : CData_GetNRecs(_this);  /* Get number of records             */
  nXC = CData_GetNComps(_this);                                                 /* Get number of components          */
  bPur = dlp_strlen(_this->m_lpRunit) && _this->m_lpTable->m_fsr!=0.;           /* Displaying physical record units? */
  bPuc = dlp_strlen(_this->m_lpCunit) && _this->m_nCinc!=0.;                    /* Displaying physical comp. units?  */
  nPps = dlp_maxprintlines()/(nXC+4);                                           /* Compute no. of pages per sceeen   */

  /* Print vectors */                                                           /* --------------------------------- */
  if (nBlock>=0)                                                                /* Printing blockwise?               */
  {                                                                             /* >>                                */
    printf("\n   Block %ld (offset %ld)",(long)nBlock,(long)nR_);               /*   Show current block index        */
    dlp_inc_printlines(1);  nL++;                                               /*   Adjust number of printed lines  */
  }                                                                             /* <<                                */
  for (nR0=nR_; nR0<nXR; )                                                      /* Loop over records                 */
  {                                                                             /* >>                                */
    CData_PrintVectors_GetColWidth(_this,nR0,0,&nWI,&nW0,&nW);                  /*   Comp. head and data col. widths */
    nSR = (dlp_maxprintcols()-nWI-nW0-4)/(nW+1);                                /*   Number of columns to print      */
    if (nR0+nSR>nXR) nSR = nXR-nR0;                                             /*   No more than there are records! */

    /* Print record header */                                                   /*   - - - - - - - - - - - - - - - - */
    if (bPur)                                                                   /*   Display physical record units?  */
    {                                                                           /*   >>                              */
      printf("\n %s ->",__pad(strcpy(sBuf,_this->m_lpRunit),nW0+nWI,'r'));      /*     Print name of physical unit   */
      for (nR=nR0; nR<nR0+nSR; nR++)                                            /*     Loop over remaining records   */
      {                                                                         /*     >>                            */
        nBuf = _this->m_lpTable->m_ofs + (nR-nR_)*_this->m_lpTable->m_fsr;      /*       Compute abscissa value      */
        __sprintx(sBuf,&nBuf,T_DOUBLE,_this->m_bExact);                         /*       Print to a string           */
        printf("%s ",__pad(sBuf,nW,'r'));                                       /*       Format and print to screen  */
      }                                                                         /*     <<                            */
      dlp_inc_printlines(1); nL++;                                              /*     Adjust number of printed lines*/
    }                                                                           /*   <<                              */
    sBuf[0]='\0';                                                               /*   Clear string buffer             */
    if (bPuc) sprintf(sBuf,"%s| ",_this->m_lpCunit);                            /*   Print phys. comp. unit name...  */
    printf("\n %s",__pad(sBuf,nW0+nWI+3,'r'));                                  /*   ... or empty string             */
    for (nR=nR0; nR<nR0+nSR; nR++)                                              /*   Loop over remaining records     */
    {                                                                           /*   >>                              */
      i=nR-nR_; __sprintx(sBuf,&i,T_INT,_this->m_bExact);                       /*     Print record index to a str.  */
      printf("%s%c",__pad(sBuf,nW,'r'),CData_RecIsMarked(_this,nR)?'*':' ');    /*     Format and print to screen    */
    }                                                                           /*   <<                              */
    sBuf[0]='\0';                                                               /*   Clear string buffer             */
    if (bPuc) sprintf(sBuf,"%c ",bPuc?'v':' ');                                 /*   Print down arrow ...            */
    printf("\n %s",__pad(sBuf,nW0+nWI+3,'r'));                                  /*   ... or empty string             */
    sBuf[0]='\0'; for (i=0; i<nW; i++) sBuf[i]='.'; sBuf[i]='\0';               /*   Make horizonal delimiter        */
    for (nR=nR0; nR<nR0+nSR; nR++) printf("%s ",sBuf);                          /*   Print one per vector            */
    dlp_inc_printlines(2);  nL+=2;                                              /*   Adjust number of printed lines  */

    /* Print data */                                                            /*   - - - - - - - - - - - - - - - - */
    for (nC=0; nC<nXC; )                                                        /*   Loop over components            */
    {                                                                           /*   >>                              */
      __sprintx(sBuf,&nC,T_INT,_this->m_bExact);                                /*     Print comp. index to a string */
      printf("\n%c%s ",                                                         /*     Format and print to screen    */
        CData_CompIsMarked(_this,nC)?'*':' ',__pad(sBuf,nWI,'r'));              /*     | (incl. "*" for "marked")    */
      if (bPuc && dlp_is_numeric_type_code(CData_GetCompType(_this,nC)))        /*     Display ordinate value?       */
      {                                                                         /*     >>                            */
        nBuf = _this->m_nCofs + nC*_this->m_nCinc;                              /*       Compute it                  */
        __sprintx(sBuf,&nBuf,T_DOUBLE,_this->m_bExact);                         /*       Print it to a string        */
      }                                                                         /*     <<                            */
      else strcpy(sBuf,CData_GetCname(_this,nC));                               /*     else display component name   */
      printf("%s: ",__pad(dlp_strtrimleft(dlp_strtrimright(sBuf)),nW0,'r'));    /*     Format and print to screen    */
      for (nR=nR0; nR<nR0+nSR; nR++)                                            /*     Loop over remaining records   */
      {                                                                         /*     >>                            */
        __sprintx(sBuf,CData_XAddr(_this,nR,nC),                                /*       Print cell value to a str.  */
            CData_GetCompType(_this,nC),_this->m_bExact);                       /*         |                         */
        if (dlp_is_symbolic_type_code(CData_GetCompType(_this,nC)))             /*       Is string value             */
          if ((INT32)dlp_strlen(sBuf)>nW)                                        /*         Will not fit in column    */
            dlp_strabbrv(sBuf,sBuf,nW);                                         /*           Abbreviate it           */
        if (dlp_is_numeric_type_code(CData_GetCompType(_this,nC)))              /*       Is numeric value            */
          if (_this->m_bNz && CMPLX_EQUAL(CData_Cfetch(_this,nR,nC),CMPLX(0.)))
            dlp_strcpy(sBuf,"-");
        printf("%s%c",__pad(sBuf,nW,'r'),                                       /*       Format and print to screen  */
          CData_CellIsMarked(_this,nR*CData_GetNComps(_this)+nC)?'*':' ');      /*       | (incl. "*" for "marked")  */
      }                                                                         /*     <<                            */
      dlp_inc_printlines(1); nL++;                                              /*     Adjust number of printed lines*/

      /* Break component listing */                                             /*     - - - - - - - - - - - - - - - */
      if (nPps==0)                                                              /*     Not all comps. fit on screen  */
      {                                                                         /*     >>                            */
        sprintf(sBuf,"component (0..%ld), cancel -3",(long)nXC-1);              /*       Make user hint              */
        if ((nC=dlp_printstop_nix(nC,sBuf,NULL))==-1) break;                    /*       Break listing               */
        if (nC< -2 ) return -1;                                                 /*       Cancelled by user           */
        if (nC>=nXC) break;                                                     /*       No more components -> break */
      }                                                                         /*     <<                            */
      else nC++;                                                                /*     No breaking -> count comps.   */
    }                                                                           /*   <<                              */
    nR0+=nSR;                                                                   /*   First record on next page       */
    nP++;                                                                       /*   Count pages                     */
    if (nR0<nXR)                                                                /*   There are more records          */
    {                                                                           /*   >>                              */
      printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());           /*     Print a separator             */
      dlp_inc_printlines(1); nL++;                                              /*     Adjust number of printed lines*/
    }                                                                           /*   <<                              */

    /* Break record listing */                                                  /*   - - - - - - - - - - - - - - - - */
    if (((nPps>0 && nP>=nPps) || nPps==0) && nR0<nXR)                           /*   Complicated break condition :)  */
    {                                                                           /*   >>                              */
      dlp_inc_printlines(dlp_maxprintlines());                                  /*     Do stop right here            */
        sprintf(sBuf,"record (%ld..%ld)%s",(long)nR_,(long)nXR-1,               /*     Make user hint                */
          nBlock>=0?", cancel -3":"");                                          /*     |                             */
      if ((nR0=dlp_printstop_nix(--nR0,sBuf,NULL))==-1) break;                  /*     Break listing                 */
      if (nR0< -2 ) return -1;                                                  /*     Cancelled by user             */
      if (nR0< nR_) nR0=nR_;                                                    /*     No previous blocks, please!   */
      if (nR0>=nXR) break;                                                      /*     No more records -> break      */
      nP=0;                                                                     /*     Reset page counter            */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */

  return nL;                                                                    /* Return number of printed lines    */
}

/**
 * Prints the instance in vector mode (standard).
 * @param _this
 *          Pointer to data instance
 * @return <code>O_K</code> if successfull, a (negative) errorcode otherwise
 */
INT16 CGEN_PUBLIC CData_PrintVectors(CData* _this)
{
  INT32 i   = 0;                                                              /* Universal loop counter            */
  INT32 nB  = 0;                                                              /* Current block index               */
  INT32 nL  = 0;                                                              /* Number of recently printed lines  */
  INT32 nXL = 0;                                                              /* Total number of printed lines     */
  char sBuf[L_SSTR+1];                                                          /* String buffer                     */

  if (CData_GetNBlocks(_this)<=1)                                               /* No blocks                         */
    return (INT16)CData_PrintVectors_Block(_this,-1);                         /*   Print all records at once       */
  else for (nB=0; nB<CData_GetNBlocks(_this); )                                 /* Else loop over blocks             */
  {                                                                             /* >>                                */
    nL = CData_PrintVectors_Block(_this,nB);                                    /*   Print blocks separately         */
    if (nL<0) { printf("\n   Cancelled"); return O_K; }                         /*   Cancelled by user               */
    if (nB<CData_GetNBlocks(_this)-1)                                           /*   Not the last block              */
      { printf("\n"); for (i=0; i<dlp_maxprintcols(); i+=2) printf(" -"); }     /*     Print a nice separator        */
    nXL += nL;                                                                  /*   Count printed lines             */
    if                                                                          /*   Next block won't fit on screen  */
    (                                                                           /*   |                               */
      nXL+nL>dlp_maxprintlines() ||                                             /*   | if it is as long as the last  */
      CData_GetNRecsPerBlock(_this)>dlp_maxprintlines()                         /*   | because it has too many recs. */
    )                                                                           /*   |                               */
    {                                                                           /*   >>                              */
      if (nB==CData_GetNBlocks(_this))                                          /*     This was the last block       */
        { printf("\n"); for (i=0; i<dlp_maxprintcols(); i+=2) printf(" -"); }   /*       Separator not yet printed   */
      dlp_inc_printlines(dlp_maxprintlines());                                  /*     Do stop right here            */
      sprintf(sBuf,"block (0..%ld)",(long)CData_GetNBlocks(_this)-1);           /*     Make user hint                */
      if ((nB=dlp_printstop_nix(nB,sBuf,NULL))==-1) break;                      /*     Break listing                 */
      if (nB< -2 ) nB=0;                                                        /*     Bad user reply -> start over  */
      if (nB>=CData_GetNBlocks(_this)) break;                                   /*     No more blocks -> break       */
      nXL=0;                                                                    /*     Reset line counter            */
    }                                                                           /*   <<                              */
    else nB++;                                                                  /*   No list break -> next block     */
  }                                                                             /* <<                                */
  return O_K;                                                                   /* Ok                                */
}

/* EOF */
