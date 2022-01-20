/* dLabPro class CDlpFile (file)
 * - Import/export functions of graph (fst/structure) instances
 *
 * AUTHOR : M. Eichner and M. Wolff
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
#include "dlp_file.h"
#include "dlp_fst.h"

static char __lpsBuf[L_SSTR];
#define __QUOT(A) dlp_strlen(A) ? \
  dlp_strquotate(dlp_strconvert(SC_ESCAPE,__lpsBuf,A),'\"','\"'):"\"\""

/*
 * Graph info struct
 */
typedef struct tag_SGphInfo
{
  BOOL   bIsStr;
  CData* idUd;
  CData* idSd;
  CData* idTd;
  INT32   nIcUdFs;
  INT32   nIcUdFt;
  INT32   nIcUdXs;
  INT32   nIcUdXt;
  INT32   nIcUdData;
  INT32   nIcSdData;
  INT32   nIcTdIni;
  INT32   nIcTdTer;
  INT32   nIcTdTis;
  INT32   nIcTdTos;
  INT32   nIcTdW;
  INT32   nIcTdStk;
  INT16  nWsr;
  INT32   nIcTdData;
}
SGphInfo;

/**
 * Fill graph information data struct.
 */
BOOL CDlpFile_Gph_GetGraphInfo(CDlpObject* iSrc, SGphInfo* lpGi)
{
  DLPASSERT(lpGi);

  dlp_memset(lpGi,0,sizeof(SGphInfo));
  if (!iSrc) return FALSE;

  if (dlp_strcmp(iSrc->m_lpClassName,"fst")==0)
  {
    lpGi->bIsStr    = FALSE;
    lpGi->idUd      = AS(CData,AS(CFst,iSrc)->ud);
    lpGi->idSd      = AS(CData,AS(CFst,iSrc)->sd);
    lpGi->idTd      = AS(CData,AS(CFst,iSrc)->td);
    lpGi->nIcUdFs   = IC_UD_FS;
    lpGi->nIcUdFt   = IC_UD_FT;
    lpGi->nIcUdXs   = IC_UD_XS;
    lpGi->nIcUdXt   = IC_UD_XT;
    lpGi->nIcUdData = IC_UD_DATA;
    lpGi->nIcSdData = IC_SD_DATA;
    lpGi->nIcTdIni  = IC_TD_INI;
    lpGi->nIcTdTer  = IC_TD_TER;
    lpGi->nIcTdData = IC_TD_DATA;
    lpGi->nIcTdTis  = CData_FindComp(lpGi->idTd,NC_TD_TIS);
    lpGi->nIcTdTos  = CData_FindComp(lpGi->idTd,NC_TD_TOS);
    lpGi->nWsr      = CFst_Wsr_GetType(AS(CFst,iSrc),&lpGi->nIcTdW);
    lpGi->nIcTdStk  = CData_FindComp(lpGi->idTd,"~STK");
    return TRUE;
  }

  return FALSE;
}

/**
 * Export fst or structure in DOT format for plotting using GraphViz.
 *
 * @param lpsFilename Name of file to export
 * @param iSrc        Pointer to instance to export
 * @param lpsFiletype Type of file to export
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_Gph_ExportDot
(
  CDlpFile*   _this,
  const char* lpsFilename,
  CDlpObject* iSrc,
  const char* lpsFiletype
)
{
  FILE*    fDst        = NULL;                                                 /* Output file                    */
  SGphInfo gi;                                                                 /* Graph info struct              */
  INT32     nU          = 0;                                                    /* Current unit                   */
  INT32     nXU         = 0;                                                    /* Number of units                */
  INT32     nS          = 0;                                                    /* Current state                  */
  INT32     nFS         = 0;                                                    /* First state of current unit    */
  INT32     nXS         = 0;                                                    /* Number of states of cur. unit  */
  INT32     nT          = 0;                                                    /* Current transition             */
  INT32     nFT         = 0;                                                    /* First trans. of current unit   */
  INT32     nXT         = 0;                                                    /* Number of trans. of. cur. unit */ 
  INT32     nIni        = 0;                                                    /* Initial state of cur. trans    */
  INT32     nTer        = 0;                                                    /* Terminal state of cur. trans.  */
  INT32     nIcSname    = -1;                                                   /* Component carrying state name  */
  INT32     nIcSrank    = -1;                                                   /* Component carrying state rank  */
  char     lpsBuf[256] = "";                                                   /* A name buffer                  */
  INT32     i           = 0;                                                    /* Loop counter                   */

  /* Initialize - Get graph information on iSrc */
  IF_NOK(CDlpFile_Gph_GetGraphInfo(iSrc,&gi)) return NOT_EXEC;
  nXU = CData_GetNRecs(gi.idUd);

  /* Initialize - Look for first symbolic component of idSd and use it as node name */
  for (i=gi.nIcSdData; i<CData_GetNComps(gi.idSd); i++)
    if (dlp_is_symbolic_type_code(CData_GetCompType(gi.idSd,i)))
    {
      nIcSname=i;
      break;
    }

  /* Initialize - Get state rank component */
  nIcSrank = CData_FindComp(gi.idSd,NC_SD_RNK);

  /* NO RETURNS BEYOND THIS POINT! */
  /* Open file open for writing */
  if ((fDst=fopen(lpsFilename,"w")) == NULL) return NOT_EXEC;

  fprintf(fDst,"digraph \"%s\" {\n",1<nXU?iSrc->m_lpInstanceName:(char*)CData_XAddr(gi.idUd,0,0));
  fprintf(fDst,"\trankdir=LR;ranksep=0.1\n");

  for (nU=0; nU<nXU; nU++)
  {
    if (1<nXU)
      fprintf(fDst,"\tsubgraph %ld {\n\t\tlabel = \"%s\";\n",(long)nU,(char*)CData_XAddr(gi.idUd,nU,0));

    nFS = (INT32)CData_Dfetch(gi.idUd,nU,gi.nIcUdFs);
    nXS = (INT32)CData_Dfetch(gi.idUd,nU,gi.nIcUdXs);
    nFT = (INT32)CData_Dfetch(gi.idUd,nU,gi.nIcUdFt);
    nXT = (INT32)CData_Dfetch(gi.idUd,nU,gi.nIcUdXt);

    /* Write states */
    if (gi.bIsStr)
    {
      fprintf(fDst,"\t\tS%ld_0 [label=\"0\",shape=circle];\n"      ,(long)nU);
      fprintf(fDst,"\t\tS%ld_F [label=\"0\",shape=doublecircle];\n",(long)nU);
    }
    for (nS=nFS; nS<nFS+nXS; nS++)
    {
      if (nIcSname>=0) strcpy(lpsBuf,(char*)CData_XAddr(gi.idSd,nS,nIcSname));
      else             sprintf(lpsBuf,"%ld",(long)(nS-nFS+(gi.bIsStr?1:0)));

      fprintf
      (
        fDst,"\t\tS%ld_%ld [label=\"%s\",shape=%scircle];\n",(long)nU,(long)(nS-nFS+(gi.bIsStr?1:0)),lpsBuf,
        (!gi.bIsStr && ((INT32)CData_Dfetch(gi.idSd,nS,0)&0x01)!=0)?"double":""
      );
    }

    /* Write transitions */
    for (nT=nFT; nT<nFT+nXT; nT++)
    {
      nIni = (INT32)CData_Dfetch(gi.idTd,nT,gi.nIcTdIni);
      nTer = (INT32)CData_Dfetch(gi.idTd,nT,gi.nIcTdTer);

      if (nTer==0 && gi.bIsStr) strcpy(lpsBuf,"F");
      else                      sprintf(lpsBuf,"%ld",(long)nTer);

      fprintf(fDst,"\t\tS%ld_%ld -> S%ld_%s",(long)nU,(long)nIni,(long)nU,lpsBuf);
      if (CData_GetNComps(gi.idTd)>gi.nIcTdData)
      {
        fprintf(fDst," [label=\"");
        for (i=gi.nIcTdData; i<CData_GetNComps(gi.idTd); i++)
        {
          if (i>gi.nIcTdData) fprintf(fDst," ");
          dlp_printx(fDst,CData_XAddr(gi.idTd,nT,i),CData_GetCompType(gi.idTd,i),0,FALSE,FALSE);
        }
        fprintf(fDst,"\"");
        if (nIcSrank>=0)
          if (CData_Dfetch(gi.idSd,nIni+nFS,nIcSrank)>=CData_Dfetch(gi.idSd,nTer+nFS,nIcSrank))
            fprintf(fDst,",constraint=false");
        /* HACK: No constraint if rankIni>=rankTer; but this will cause dot.exe to crash */

        fprintf(fDst,"]");
      }
      fprintf(fDst,";\n");
    }

    /* Close subgraph */
    if(1<nXU) fprintf(fDst,"\t}\n\n");
  }

  /* Close graph */
  fprintf(fDst,"}\n");

  /* Clean up */
  fclose(fDst);
  return O_K;
}

/**
 * Export fst or structure into a graphic file. Uses
 * {@link Gph_ExportDot CDlpFile_Gph_ExportDot} and dot(.exe). The latter is
 * expected in the dLabPro binary directory or in the path environment
 * variable.
 *
 * @param lpsFilename Name of file to export
 * @param iSrc        Pointer to instance to export
 * @param lpsFiletype Type of file to export
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_Gph_ExportDotTx
(
  CDlpFile*   _this,
  const char* lpsFilename,
  CDlpObject* iSrc,
  const char* lpsFiletype
)
{
  INT16 nErr                     =O_K;
#ifndef __TMS
  FILE* fDst                     =NULL;
  char  lpsPsep       []         ="/";
  char  lpsDotexe     [L_PATH]   = "";
  char  lpsTmpFilename[L_PATH]   = "";
  char* lpsDlpHome               = NULL;
  char* lpsMachine               = NULL;
  char  lpsCmdline    [3*L_PATH] = "";

  lpsPsep[0]=C_DIR;

  /* Find dot(.exe) */
  lpsDlpHome = getenv("DLABPRO_HOME");
  lpsMachine = getenv("MACHINE");
  if (lpsDlpHome && lpsMachine)
  {
    sprintf(lpsDotexe,"%s/bin.debug.%s/ATT/Graphviz/bin/dot",lpsDlpHome,lpsMachine);
    dlp_strreplace(lpsDotexe,"/" ,lpsPsep);
    dlp_strreplace(lpsDotexe,"\\",lpsPsep);
  #ifdef _WINDOWS
    dlp_strcat(lpsDotexe,".exe");
  #endif
  
    if ((fDst=fopen(lpsDotexe,"r"))==NULL)
    {
      dlp_strreplace(lpsDotexe,"bin.","bin.release.");
      if ((fDst=fopen(lpsDotexe,"r"))==NULL)
      {
        /* Just hope "dot" to be found in path */
        strcpy(lpsDotexe,"dot");
      }
      else fclose(fDst);
    }
    else fclose(fDst);
  }
  else strcpy(lpsDotexe,"dot");
  

  /* Write dot file */
  sprintf(lpsTmpFilename,"%s~",lpsFilename);
  IF_NOK(CDlpFile_Gph_ExportDot(_this,lpsTmpFilename,iSrc,"dot")) return NOT_EXEC;

  /* Convert file using dot */
  sprintf(lpsCmdline,"%s -T%s %s -o%s",lpsDotexe,lpsFiletype,lpsTmpFilename,lpsFilename);
  if (system(lpsCmdline)!=0) { nErr=NOT_EXEC; IERROR(_this,FIL_EXEC,lpsCmdline,0,0); }
  if (remove(lpsTmpFilename)==-1) IERROR(_this,FIL_REMOVE,"temporary ",lpsTmpFilename,0);

#endif
  return nErr;
}

const char * CGEN_IGNORE CDlpFile_Gph_SymFilename(const char *lpsFn,const char *lpsExt)
{
  const char *p=strrchr(lpsFn,'.');
  static char lpsSFn[L_PATH];
  int l;
  if(!p) p=lpsFn+strlen(lpsFn);
  snprintf(lpsSFn,MIN(L_PATH,p-lpsFn+1),"%s",lpsFn);
  l=strlen(lpsSFn);
  snprintf(lpsSFn+l,L_PATH-l,"_%s%s",lpsExt,p);
  return lpsSFn;
}

/**
 * Export fst or structure in FSM format for FSM Toolkit by AT&T.
 *
 * @param lpsFilename Name of file to export
 * @param iSrc        Pointer to instance to export
 * @param lpsFiletype Type of file to export
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_Gph_ExportFsm
(
  CDlpFile*   _this,
  const char* lpsFilename,
  CDlpObject* iSrc,
  const char* lpsFiletype
)
{
  FILE*    fDst    = NULL;                                                     /* Output file                    */
  SGphInfo gi;                                                                 /* Graph info struct              */
  INT32     nS      = 0;                                                        /* Current state                  */
  INT32     nFS     = 0;                                                        /* First state of current unit    */
  INT32     nXS     = 0;                                                        /* Number of states of cur. unit  */
  INT32     nT      = 0;                                                        /* Current transition             */
  INT32     nFT     = 0;                                                        /* First trans. of current unit   */
  INT32     nXT     = 0;                                                        /* Number of trans. of. cur. unit */ 
  INT32     nIni    = 0;                                                        /* Initial state of cur. trans    */
  INT32     nTer    = 0;                                                        /* Terminal state of cur. trans.  */
  INT32     nTis    = -1;                                                       /* Input symbol of cur. trans.    */
  INT32     nTos    = -1;                                                       /* Output symbol of cur. trans.   */
  FLOAT64   nW      = 0.;                                                       /* Weight of current transition   */
  INT32     nStk    = 0;                                                        /* Output symbol of cur. trans.   */
  BOOL     bDstFst = FALSE;                                                    /* Export fst                     */
  CData *idIs=AS(CData,AS(CFst,iSrc)->is); if(CData_IsEmpty(idIs)) idIs=NULL;  /* Input symbol table              */
  CData *idOs=AS(CData,AS(CFst,iSrc)->os); if(CData_IsEmpty(idOs)) idOs=NULL;  /* Output symbol table             */

  /* Initialize */
  bDstFst = (strcmp(lpsFiletype,"fsm-t")==0);

  /* Initialize - Get graph information on iSrc */
  IF_NOK(CDlpFile_Gph_GetGraphInfo(iSrc,&gi)) return NOT_EXEC;
  if (gi.nIcTdTis<0) return NOT_EXEC;

  /* NO RETURNS BEYOND THIS POINT! */
  /* Open file open for writing */
  if ((fDst=fopen(lpsFilename,"w")) == NULL) return NOT_EXEC;

  /* Write unit 0 */
  nFS = (INT32)CData_Dfetch(gi.idUd,0,gi.nIcUdFs);
  nXS = (INT32)CData_Dfetch(gi.idUd,0,gi.nIcUdXs);
  nFT = (INT32)CData_Dfetch(gi.idUd,0,gi.nIcUdFt);
  nXT = (INT32)CData_Dfetch(gi.idUd,0,gi.nIcUdXt);

  /* Write transitions */
  for (nT=nFT; nT<nFT+nXT; nT++)
  {
    nIni = (INT32)CData_Dfetch(gi.idTd,nT,gi.nIcTdIni);
    nTer = (INT32)CData_Dfetch(gi.idTd,nT,gi.nIcTdTer);
    nTis = gi.nIcTdTis>=0 ? (INT32)CData_Dfetch(gi.idTd,nT,gi.nIcTdTis) : -1;
    nTos = gi.nIcTdTos>=0 ? (INT32)CData_Dfetch(gi.idTd,nT,gi.nIcTdTos) : -1;
    nW   = gi.nIcTdW  >=0 ? CData_Dfetch(gi.idTd,nT,gi.nIcTdW)         : 0.;
    nStk = gi.nIcTdStk>=0 ? CData_Dfetch(gi.idTd,nT,gi.nIcTdStk)       : 0;
    if (gi.nWsr==FST_WSR_PROB) nW=-log(nW);

    if (gi.bIsStr && nTer==0)
    {
      if (gi.nIcTdW>=0) fprintf(fDst,"%ld\t%lG\t\n",(long)nIni,(double)nW);
      else              fprintf(fDst,"%ld\t\n",(long)nIni);
    }
    else
    {
      fprintf(fDst,"%ld\t%ld",(long)nIni,(long)nTer);
      if(idIs) fprintf(fDst,"\t%s",nTis<0?"<eps>":CData_Sfetch(idIs,nTis,0));
      else     fprintf(fDst,"\t%ld",(long)nTis+1);
      if(bDstFst){
        if(idOs) fprintf(fDst,"\t%s",nTos<0?"<eps>":CData_Sfetch(idOs,nTos,0));
        else     fprintf(fDst,"\t%ld",(long)nTos+1);
      }
      if (gi.nIcTdW>=0) fprintf(fDst,"\t%lG",(double)nW);
      if (gi.nIcTdW>=0 && gi.nIcTdStk>=0) fprintf(fDst,"\t%ld",(long)nStk);
      fprintf(fDst,"\n");
    }
  }

  /* Write final states (fst-instances only) */
  if (!gi.bIsStr)
    for (nS=nFS; nS<nFS+nXS; nS++)
      if ((((BYTE)CData_Dfetch(gi.idSd,nS,0))&0x01)==0x01)
        fprintf(fDst,"%ld\t\n",(long)nS);

  /* Clean up */
  fclose(fDst);

  /* Write input symbol table */
  if(idIs){
    if(!(fDst=fopen(CDlpFile_Gph_SymFilename(lpsFilename,"is"),"w"))) return NOT_EXEC;
    fprintf(fDst,"<eps>\t0\n");
    for(nT=0;nT<CData_GetNRecs(idIs);nT++)
      fprintf(fDst,"%s\t%ld\n",CData_Sfetch(idIs,nT,0),(long)(nT+1));
    fclose(fDst);
  }

  /* Write output symbol table */
  if(idOs){
    if(!(fDst=fopen(CDlpFile_Gph_SymFilename(lpsFilename,"os"),"w"))) return NOT_EXEC;
    fprintf(fDst,"<eps>\t0\n");
    for(nT=0;nT<CData_GetNRecs(idOs);nT++)
      fprintf(fDst,"%s\t%ld\n",CData_Sfetch(idOs,nT,0),(long)(nT+1));
    fclose(fDst);
  }

  return O_K;
}

FST_STYPE CGEN_IGNORE CDlpFile_Gph_Sym2Id(char *sBuf,CData *idS)
{
  INT32 r;
  char *sChg;
  if(!strcmp(sBuf,"<eps>")) return -1;
  for(r=0;r<CData_GetNRecs(idS);r++)
    if(!strcmp(sBuf,CData_Sfetch(idS,r,0)))
      return r;
  r=strtol(sBuf,&sChg,10);
  if(sBuf!=sChg) return r;
  r=CData_GetNRecs(idS);
  CData_Reallocate(idS,r+1);
  CData_Sstore(idS,sBuf,r,0);
  return r;
}

/**
 * Import fst or structure from FSM format (FSM Toolkit by AT&T).
 *
 * @param lpsFilename Name of file to import
 * @param iDst        Pointer to instance to import
 * @param lpsFiletype Type of file to import
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_Gph_ImportFsm
(
  CDlpFile*   _this,
  const char* lpsFilename,
  CDlpObject* iDst,
  const char* lpsFiletype
)
{
  FILE*    fSrc                 = NULL;                                        /* Output file                    */
  SGphInfo gi;                                                                 /* Graph info struct              */
  char     lpsLine[L_INPUTLINE] = "";                                          /* Line reading buffer            */
  BOOL     bSrcFst              = FALSE;                                       /* Source is an FST               */
  BOOL     bExtraFinal          = FALSE;                                       /* Added extra final state        */
  BOOL     bWeighted            = FALSE;                                       /* Source is weighted             */
  BOOL     bStk                 = FALSE;                                       /* Source is weighted             */
  INT32  nLine                = 0;                                           /* Current line in source file    */
  INT32  nFld                 = 0;                                           /* Number of scanned fields       */
  INT32  nXS                  = 0;                                           /* Number of states               */
  INT32  nXT                  = 0;                                           /* Number of transitions          */
  INT32  nFinal               = 0;                                           /* Number of final states         */
  int      nIni                 = 0;                                           /* Initial state of cur. trans.   */
  int      nTis                 = 0;                                           /* Input symbol of cur. trans.    */
  double   nBuf1                = 0.;                                          /* Copy buffer                    */
  double   nBuf2                = 0.;                                          /* Copy buffer                    */
  double   nBuf3                = 0.;                                          /* Copy buffer                    */
  double   nBuf4                = 0.;                                          /* Copy buffer                    */
  char sBuf1[L_INPUTLINE];                                                     /* Copy buffer                    */
  char sBuf2[L_INPUTLINE];                                                     /* Copy buffer                    */
  CData *idIs;                                                                 /* Input symbol table             */
  CData *idOs;                                                                 /* Output symbol table            */

  /* Initialize */
  bSrcFst = (strcmp(lpsFiletype,"fsm-t")==0);

  /* Analyze input file */
  if ((fSrc=fopen(lpsFilename,"r"))==NULL) return NOT_EXEC;
  for (nLine=0; !feof(fSrc); nLine++)
  {
    if (fgets(lpsLine,L_INPUTLINE,fSrc)==NULL) break;
    dlp_strtrimleft(lpsLine);
    dlp_strtrimright(lpsLine);
    if (strlen(lpsLine)==0) continue;

    /* Scan line */
    nFld = sscanf(lpsLine,"%d %lG %d %lG %lG %lG",&nIni,&nBuf1,&nTis,&nBuf2,&nBuf3,&nBuf4);
    if (bSrcFst  && (nFld==5 || nFld==2)) bWeighted = TRUE;
    if (!bSrcFst && (nFld==4 || nFld==2)) bWeighted = TRUE;
    if (bSrcFst  && (nFld==6 || nFld==2)) bStk = TRUE;
    if (!bSrcFst && (nFld==5 || nFld==2)) bStk = TRUE;
    if (nFld<3                          ) nFinal++; else nXT++;
    if (nIni>nXS                        ) nXS=nIni;
  }
  fclose(fSrc);
  nXS++;
  
  /* Initialize destination instance */
  IF_NOK(CDlpFile_Gph_GetGraphInfo(iDst,&gi)) return NOT_EXEC;
  CFst_Reset(BASEINST(iDst),TRUE);
  CFst_Addunit((CFst*)iDst,"fsm");
  CData_AddComp(gi.idTd,NC_TD_TIS,DLP_TYPE(FST_STYPE));
  if (bSrcFst  ) CData_AddComp(gi.idTd,NC_TD_TOS,DLP_TYPE(FST_STYPE));
  if (bWeighted) CData_AddComp(gi.idTd,NC_TD_LSR,DLP_TYPE(FST_WTYPE));
  if (bStk     ) CData_AddComp(gi.idTd,"~STK",DLP_TYPE(FST_WTYPE));

  CDlpFile_Gph_GetGraphInfo(iDst,&gi);
  CData_Realloc(gi.idTd,nXT+nFinal);
  CData_Realloc(gi.idSd,nXS+1);

  /* Read input symbols */
  idIs=AS(CData,((CFst*)iDst)->is);
  CData_AddComp(idIs,"~IS",255);
  if((fSrc=fopen(CDlpFile_Gph_SymFilename(lpsFilename,"is"),"r"))){
    while(!feof(fSrc)) if(fgets(lpsLine,L_INPUTLINE,fSrc)){
      char *sid=lpsLine, *eid, *sym=dlp_strsep(&sid," \t", NULL);
      long id;
      if(!sid) continue;
      id=strtol(sid,&eid,10)-1;
      if(sid==eid || id<0) continue;
      if(CData_GetNRecs(idIs)<=id) CData_Reallocate(idIs,id+1);
      CData_Sstore(idIs,sym,id,0);
    }
    fclose(fSrc);
  }

  /* Read output symbols */
  idOs=AS(CData,((CFst*)iDst)->os);
  CData_AddComp(idOs,"~OS",255);
  if((fSrc=fopen(CDlpFile_Gph_SymFilename(lpsFilename,"os"),"r"))){
    while(!feof(fSrc)) if(fgets(lpsLine,L_INPUTLINE,fSrc)){
      char *sid=lpsLine, *eid, *sym=dlp_strsep(&sid," \t", NULL);
      long id;
      if(!sid) continue;
      id=strtol(sid,&eid,10)-1;
      if(sid==eid || id<0) continue;
      if(CData_GetNRecs(idOs)<=id) CData_Reallocate(idOs,id+1);
      CData_Sstore(idOs,sym,id,0);
    }
    fclose(fSrc);
  }

  /* Import file */
  if ((fSrc=fopen(lpsFilename,"r"))==NULL) return NOT_EXEC;
  for (nLine=0,nXT=0; !feof(fSrc); nLine++)
  {
    if (fgets(lpsLine,L_INPUTLINE,fSrc)==NULL) break;
    dlp_strtrimleft(lpsLine);
    dlp_strtrimright(lpsLine);
    if (strlen(lpsLine)==0) continue;

    /* Scan line */
    nIni  = 0;
    nTis  = 0;
    nBuf1 = 0.;
    nBuf2 = 0.;
    nBuf3 = 0.;
    nBuf4 = 0.;
    nFld  = sscanf(lpsLine,"%d %lG %s %s %lG %lG",&nIni,&nBuf1,sBuf1,sBuf2,&nBuf3,&nBuf4);
    if (nFld>2)
    {
      /* Transition */
      nTis=CDlpFile_Gph_Sym2Id(sBuf1,idIs);
      CData_Dstore(gi.idTd,nIni ,nXT,gi.nIcTdIni);
      CData_Dstore(gi.idTd,nBuf1,nXT,gi.nIcTdTer);
      CData_Dstore(gi.idTd,nTis ,nXT,gi.nIcTdTis);
      if (bSrcFst)
      {
        nBuf2=CDlpFile_Gph_Sym2Id(sBuf2,idOs);
        CData_Dstore(gi.idTd,nBuf2,nXT,gi.nIcTdTos);
        CData_Dstore(gi.idTd,nBuf3,nXT,gi.nIcTdW);
        CData_Dstore(gi.idTd,nBuf4,nXT,gi.nIcTdStk);
      }
      else
      {
        nBuf2=dlp_strtod(sBuf2,NULL);
        CData_Dstore(gi.idTd,nBuf2,nXT,gi.nIcTdW);
        CData_Dstore(gi.idTd,nBuf3,nXT,gi.nIcTdStk);
      }

      nXT++;
    }
    else
    {
      /* Final state */
      if (nBuf1!=0.)
      {
        /* Add a new transition */
        CData_Dstore(gi.idTd,nIni,nXT,gi.nIcTdIni);
        CData_Dstore(gi.idTd,nXS ,nXT,gi.nIcTdTer);
        CData_Dstore(gi.idTd,-1  ,nXT,gi.nIcTdTis);

        if (bSrcFst)        {
          CData_Dstore(gi.idTd,-1,nXT,gi.nIcTdTos);
          CData_Dstore(gi.idTd,nBuf1,nXT,gi.nIcTdW);
        }
        else
          CData_Dstore(gi.idTd,nBuf1,nXT,gi.nIcTdW);

        nXT++;

        /* Make the final state final */
        CData_Dstore(gi.idSd,((BYTE)CData_Dfetch(gi.idSd,nXS+1,IC_SD_FLAG))|0x01,nXS,IC_SD_FLAG);
        bExtraFinal = TRUE;
      }
      else
      {
        /* Make state final */
        CData_Dstore(gi.idSd,((BYTE)CData_Dfetch(gi.idSd,nIni,IC_SD_FLAG))|0x01,nIni,IC_SD_FLAG);
      }
    }
  }
  fclose(fSrc);

  /* Finalize destination instance */
  if (bExtraFinal) nXS++;
  CData_Dstore(gi.idUd,0  ,0,gi.nIcUdFs);
  CData_Dstore(gi.idUd,0  ,0,gi.nIcUdFt);
  CData_Dstore(gi.idUd,nXS,0,gi.nIcUdXs);
  CData_Dstore(gi.idUd,nXT,0,gi.nIcUdXt);
  CData_SetNRecs(gi.idSd,nXS);
  CData_SetNRecs(gi.idTd,nXT);

  /* Clean up */
  return O_K;
}

/**
 * Writes a constant data record initializer. There are NO checks performed
 * 
 * @param fDst
 *          Output file
 * @param idSrc
 *          The source data instance
 * @param nFC
 *          The zero-based index of the first component to be printed
 * @param nR
 *          The zero-based index of the record to be printed
 */
void CGEN_PRIVATE CDlpFile_Gph_ExportItp_WriteCI
(
  FILE*  fDst,
  CData* idSrc,
  INT32   nFC,
  INT32   nR
)
{
  INT32 nC             = 0;
  char lpsBuf[L_SSTR] = "";
  
  fprintf(fDst," {");
  for (nC=nFC; nC<CData_GetNComps(idSrc); nC++)
  {
    dlp_printx(lpsBuf,CData_XAddr(idSrc,nR,nC),CData_GetCompType(idSrc,nC),0,0,
      TRUE);
    fprintf(fDst," %s",
      dlp_is_symbolic_type_code(CData_GetCompType(idSrc,nC))?
        __QUOT(lpsBuf):lpsBuf);
  }
  fprintf(fDst," }");
}

/**
 * Export fst or structure as dLabPro script creating the equivalent fst
 * instance.
 *
 * @param lpsFilename Name of file to export
 * @param iSrc        Pointer to instance to export
 * @param lpsFiletype Type of file to export
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_Gph_ExportItp
(
  CDlpFile*   _this,
  const char* lpsFilename,
  CDlpObject* iSrc,
  const char* lpsFiletype
)
{
  FILE*       fDst    = NULL;                                                   /* Output file                   */
  SGphInfo    gi;                                                               /* Graph info struct             */
  const char* lpsIname = NULL;                                                   /* Pointer to instance name      */
  INT32        nC      = 0;                                                      /* Current component             */
  INT32        nU      = 0;                                                      /* Current unit                  */
  INT32        nS      = 0;                                                      /* Current state                 */
  INT32        nFS     = 0;                                                      /* First state of current unit   */
  INT32        nXS     = 0;                                                      /* Number of states of cur. unit */
  INT32        nT      = 0;                                                      /* Current transition            */
  INT32        nFT     = 0;                                                      /* First trans. of current unit  */
  INT32        nXT     = 0;                                                      /* Number of trans. of. cur. unit*/ 
  INT32        nIni    = 0;                                                      /* Initial state of cur. trans   */
  INT32        nTer    = 0;                                                      /* Terminal state of cur. trans. */

  /* Initialize - Get graph information on iSrc */
  IF_NOK(CDlpFile_Gph_GetGraphInfo(iSrc,&gi)) return NOT_EXEC;
  if (gi.nIcTdTis<0) return NOT_EXEC;
  lpsIname = iSrc->m_lpInstanceName?iSrc->m_lpInstanceName:"f";

  /* NO RETURNS BEYOND THIS POINT! */
  /* Open file open for writing */
  if ((fDst=fopen(lpsFilename,"w")) == NULL) return NOT_EXEC;

  /* Write preamble */
  fprintf(fDst,"# Exported by dLabPro\n");
  fprintf(fDst,"\nfst %s",lpsIname);
  for (nC=gi.nIcUdData; nC<CData_GetNComps(gi.idUd); nC++)
    fprintf(fDst,"\n%-4s %5d %s.ud -addcomp;",
      __QUOT(CData_GetCname(gi.idUd,nC)),
      (int)CData_GetCompType(gi.idUd,nC),lpsIname);
  for (nC=gi.nIcSdData; nC<CData_GetNComps(gi.idSd); nC++)
    fprintf(fDst,"\n%-4s %5d %s.sd -addcomp;",
      __QUOT(CData_GetCname(gi.idSd,nC)),
      (int)CData_GetCompType(gi.idSd,nC),lpsIname);
  for (nC=gi.nIcTdData; nC<CData_GetNComps(gi.idTd); nC++)
    fprintf(fDst,"\n%-4s %5d %s.td -addcomp;",
      __QUOT(CData_GetCname(gi.idTd,nC)),
      (int)CData_GetCompType(gi.idTd,nC),lpsIname);

  for (nU=0; nU<CData_GetNRecs(gi.idUd); nU++)
  {
    /* Write unit nU */
    fprintf(fDst,"\n\n# -- Unit %ld --",(long)nU);

    nFS = (INT32)CData_Dfetch(gi.idUd,nU,gi.nIcUdFs);
    nXS = (INT32)CData_Dfetch(gi.idUd,nU,gi.nIcUdXs);
    nFT = (INT32)CData_Dfetch(gi.idUd,nU,gi.nIcUdFt);
    nXT = (INT32)CData_Dfetch(gi.idUd,nU,gi.nIcUdXt);

    /* Write unit header */
    fprintf(fDst,"\n%s %s -addunit",
      __QUOT((const char*)CData_XAddr(gi.idUd,nU,0)),lpsIname);
    if (CData_GetNComps(gi.idUd)>gi.nIcUdData)
      CDlpFile_Gph_ExportItp_WriteCI(fDst,gi.idUd,gi.nIcUdData,nU);
    fprintf(fDst,";");
  
    /* Write states */
    for (nS=nFS; nS<nFS+nXS; nS++)
    {
      fprintf(fDst,"\n%03ld    1      %s %s-addstates",(long)nU,lpsIname,
        (!gi.bIsStr && ((((BYTE)CData_Dfetch(gi.idSd,nS,0))&0x01)==0x01) ?
          "/final " : ""));
      if (CData_GetNComps(gi.idSd)>gi.nIcSdData)
        CDlpFile_Gph_ExportItp_WriteCI(fDst,gi.idSd,gi.nIcSdData,nS);
      fprintf(fDst,";");
    }
    if (gi.bIsStr) fprintf(fDst,"\n%03ld    1      %s /final -addstates;",(long)nU,lpsIname);

    /* Write transitions */
    for (nT=nFT; nT<nFT+nXT; nT++)
    {
      nIni = (INT32)CData_Dfetch(gi.idTd,nT,gi.nIcTdIni);
      nTer = (INT32)CData_Dfetch(gi.idTd,nT,gi.nIcTdTer);
      if (gi.bIsStr && nTer==0) nTer=nXS;

      fprintf(fDst,"\n%03ld %4ld %4ld %s -addtrans",(long)nU,(long)nIni,(long)nTer,lpsIname);
      if (CData_GetNComps(gi.idTd)>gi.nIcTdData && nTer<nXS)
        CDlpFile_Gph_ExportItp_WriteCI(fDst,gi.idTd,gi.nIcTdData,nT);
      fprintf(fDst,";");
    }
  }

  /* Clean up */
  fprintf(fDst,"\n\n# EOF\n");
  fclose(fDst);
  return O_K;
}

/* EOF */
