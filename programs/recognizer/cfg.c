/* dLabPro program recognizer (dLabPro recognizer)
 * - Config management
 *
 * AUTHOR : Frank Duckhorn
 * PACKAGE: dLabPro/programs
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
#define _CFG_C
#include "recognizer.h"
#undef _CFG_C

extern char *cfgdef;

struct recocfg rCfg;
struct recotmp rTmp;

char cfgload[CL_N][STR_LEN];

BOOL loadobj(char *lpsVal,enum cfgload eTyp);

void usage(const char *lpsErr,...){
  fprintf(stderr,"\n");
  if(lpsErr){
    va_list ap;
    fprintf(stderr,"Error: ");
    va_start(ap,lpsErr);
    vfprintf(stderr,lpsErr,ap);
    va_end(ap);
    fprintf(stderr,"\n");
  }
  fprintf(stderr,"Usage: recognizer [-cfg FILE.cfg] {-OPTION VALUE} [-h] [-opts] [-l] [-d N] {FILE.wav|FILELIST.flst}\n");
  fprintf(stderr,"   -cfg     Load configuration file (defaults in recognizer.cfg).\n");
  fprintf(stderr,"   -OPTION  Change options value (all options in recognizer.cfg may be changed).\n");
  fprintf(stderr,"  Some special options:\n");
  fprintf(stderr,"   -h       Print this help.\n");
  fprintf(stderr,"   -opts    List all options.\n");
  fprintf(stderr,"   -l       List audio devices.\n");
  fprintf(stderr,"   -d N     Set audio device number for online recognition.\n");
  fprintf(stderr,"  Without any files or filelists recognizer will use online mode.\n");
  fprintf(stderr,"  A filelist should have one real file name with space separated recognition result per line.\n");
  exit(lpsErr?1:0);
}

void addfile(char *lpsFN,const char *lpsLab){
  char *lpsE=lpsFN+strlen(lpsFN);
  if(strcmp(lpsE-4,".wav")) usage("File is not of type wav or flst: \"%s\"",lpsFN);
  if(rCfg.rFlst.nNum==rCfg.rFlst.nSize)
    rCfg.rFlst.lpF=dlp_realloc(rCfg.rFlst.lpF,rCfg.rFlst.nSize+=128,sizeof(struct recofile));
  snprintf(rCfg.rFlst.lpF[rCfg.rFlst.nNum].lpsFName,STR_LEN,lpsFN);
  snprintf(rCfg.rFlst.lpF[rCfg.rFlst.nNum++].lpsLab,STR_LEN,lpsLab);
}

void readfilelist(char *lpsFN){
  FILE *fd;
  char *lpsE=lpsFN+strlen(lpsFN);
  if(strcmp(lpsE-5,".flst")){ addfile(lpsFN,""); return; }
  if(!(fd=fopen(lpsFN,"r"))) usage("Unable to open file list: \"%s\"",lpsFN);
  while(!feof(fd)){
    char lpsLine[STR_LEN*2];
    char *lpsFN, *lpsLab;
    if(!fgets(lpsLine,STR_LEN*2,fd)) continue;
    lpsLab=lpsLine;
    lpsFN=dlp_strsep(&lpsLab," \t", NULL);
    if(lpsLab){
      while((*lpsLab==' ' || *lpsLab=='\t')) lpsLab++;
      lpsLab=dlp_strsep(&lpsLab,"\n\t\r", NULL);
    }
    addfile(lpsFN,lpsLab?lpsLab:"");
  }
  fclose(fd);
}

void setoptbool(const char *lpsOpt,const char *lpsVal,void *lpDst){
  if(!strcmp(lpsVal,"no")) *(BOOL*)lpDst=FALSE;
  else if(!strcmp(lpsVal,"yes")) *(BOOL*)lpDst=TRUE;
  else usage("Value for option %s is not yes or no: \"%s\"",lpsOpt,lpsVal);
}

void setopttrue(const char *lpsOpt,const char *lpsVal,void *lpDst){
  setoptbool(lpsOpt,"yes",lpDst);
}

void setoptfloat(const char *lpsOpt,const char *lpsVal,void *lpDst){
  char *lpsRet;
  *(FLOAT32*)lpDst=strtof(lpsVal,&lpsRet);
  if(lpsVal==lpsRet) usage("Value for option %s is not of type float: \"%s\"",lpsOpt,lpsVal);
}

void setoptint(const char *lpsOpt,const char *lpsVal,void *lpDst){
  char *lpsRet;
  *(INT32*)lpDst=(INT32)strtof(lpsVal,&lpsRet);
  if(lpsVal==lpsRet) usage("Value for option %s is not of type int: \"%s\"",lpsOpt,lpsVal);
}

void setoptstr(const char *lpsOpt,const char *lpsVal,void *lpDst){
  snprintf((char*)lpDst,STR_LEN,lpsVal);
}

const char **optenum_getstr(void *lpDst){
  if(lpDst==&rCfg.eOut) return recoout_str;
  else if(lpDst==&rCfg.eIn) return recoin_str;
  else if(lpDst==&rCfg.rSearch.eTyp) return recosearchtyp_str;
  else if(lpDst==&rCfg.rSearch.eRejTyp) return recorejtyp_str;
  else usage("Unknown enum in setoptenum");
  return NULL;
}

void setoptenum(const char *lpsOpt,const char *lpsVal,void *lpDst){
  const char **lpsStr = NULL;
  int nI;
  if(!(lpsStr=optenum_getstr(lpDst))) return;
  for(nI=0;lpsStr[nI];nI++) if(!strcmp(lpsVal,lpsStr[nI])){
    *(enum recoout *)lpDst=nI;
    return;
  }
  fprintf(stderr,"Unknown enum value: \"%s\" for %s\n",lpsVal,lpsOpt);
  fprintf(stderr,"Possible values:\n");
  for(nI=0;lpsStr[nI];nI++) fprintf(stderr,"  %s\n",lpsStr[nI]);
  fflush(stderr);
  usage("Unknown enum value: \"%s\"",lpsVal);
}

void setoptbindat(const char *lpsOpt,const char *lpsVal,void *lpDst){
  if(!(rCfg.bBinData=lpsVal[0]!='\0')) return;
  /* TODO: load binary data */
}

void setoptload(const char *lpsOpt,const char *lpsVal,void *lpDst){
  snprintf(cfgload[(enum cfgload)lpDst],STR_LEN,lpsVal);
  cfgload[(enum cfgload)lpDst][STR_LEN-1]='\0';
}

#define _CFG_C_SETOPT
#include "recognizer.h"
#undef _CFG_C_SETOPT

void options_list(){
  INT32 nI;
  printf("%-15s  %-8s %6s %s\n","Option-Name","Type","Online","Description");
  printf("-----------------------------------------\n");
  for(nI=0;rRecoOpts[nI].lpsName;nI++){
    const char *lpsTyp = rRecoOptTyp[rRecoOpts[nI].eTyp].lpsName;
    char *lpsDesc;
    char lpsFind[STR_LEN];
    char lpsEnum[STR_LEN]={'\0'};
    snprintf(lpsFind,STR_LEN,"\n%s ",rRecoOpts[nI].lpsName);
    lpsDesc=strstr(cfgdef,lpsFind);
    if(!lpsDesc){
      snprintf(lpsFind,STR_LEN,"# Shortcut: -%s ",rRecoOpts[nI].lpsName);
      lpsDesc=strstr(cfgdef,lpsFind);
    }
    if(lpsDesc){
      char *lpsPos=cfgdef, *lpsLPos=NULL;
      while((lpsPos=strstr(lpsPos+1,"\n\n# ")) && lpsPos<lpsDesc) lpsLPos=lpsPos;
      if(lpsLPos){
        lpsLPos+=4;
        lpsPos=strstr(lpsLPos,"\n");
        snprintf(lpsFind,MIN(STR_LEN,lpsPos-lpsLPos+1),lpsLPos);
        lpsDesc=lpsFind;
      }else lpsDesc=NULL;
    }
    if(rRecoOpts[nI].eTyp==OT_ENUM){
      const char** lpsStr=optenum_getstr(rRecoOpts[nI].lpDst);
      INT32  nLen=snprintf(lpsEnum,STR_LEN," ");
      for(;lpsStr && lpsStr[0];lpsStr++)
        nLen+=snprintf(lpsEnum+nLen,STR_LEN-nLen,",%s",lpsStr[0]);
      if(nLen>1){
        lpsEnum[1]='(';
        snprintf(lpsEnum+nLen,STR_LEN-nLen,")");
      }else lpsEnum[0]='\0';
    }
    printf("%-15s: %-8s  %-5s %s%s\n",rRecoOpts[nI].lpsName,lpsTyp,rRecoOpts[nI].bOnline?"Yes":"No",lpsDesc?lpsDesc:"No description",lpsEnum);
  }
  usage(NULL);
}

/**
 * Dumps the current values of all options at stdout.
 */
void optdump()
{
  INT32 nI;
  INT32 nJ;
  const char* lpsVal;
  char  lpsBuf[STR_LEN];
  for (nI=0; rRecoOpts[nI].lpsName; nI++)
  {
    /* Ignore shortcuts */
    snprintf(lpsBuf,STR_LEN,"# Shortcut: -%s ",rRecoOpts[nI].lpsName);
    lpsVal=strstr(cfgdef,lpsBuf);
    if (lpsVal) continue;

    /* Print option value */
    switch (rRecoOpts[nI].eTyp)
    {
    case OT_TRUE:
      continue;
    case OT_BOOL:
      lpsVal = (*(BOOL*)rRecoOpts[nI].lpDst)?"yes":"no";
      break;
    case OT_INT:
      snprintf(lpsBuf,STR_LEN,"%d",*(INT32*)rRecoOpts[nI].lpDst);
      lpsVal = lpsBuf;
      break;
    case OT_FLOAT:
      snprintf(lpsBuf,STR_LEN,"%f",*(FLOAT32*)rRecoOpts[nI].lpDst);
      lpsVal = lpsBuf;
      break;
    case OT_STR:
      lpsVal = (char*)rRecoOpts[nI].lpDst;
      break;
    case OT_ENUM:
      nJ = *(enum recoout *)rRecoOpts[nI].lpDst;
      lpsVal = (char*)optenum_getstr(rRecoOpts[nI].lpDst)[nJ];
      break;
    /* TODO: Other option types!?*/
    default:
      continue;
    }
    printf("\nopt: %s %s",rRecoOpts[nI].lpsName,lpsVal);
  }
  printf("\n"); fflush(stdout);
}

BOOL setoption(char *lpsOpt,char *lpsVal,char bOnline){
  INT32 nI;
  for(nI=0;rRecoOpts[nI].lpsName;nI++) if(!strcmp(lpsOpt,rRecoOpts[nI].lpsName)){
    if(bOnline && !rRecoOpts[nI].bOnline){
      rerror("Option %s may not been set online",lpsOpt);
      return FALSE;
    }
    if(rRecoOptTyp[rRecoOpts[nI].eTyp].bArg && !lpsVal){
      rerror("Option %s may not been set without value",lpsOpt);
      return FALSE;
    }
    (*rRecoOptTyp[rRecoOpts[nI].eTyp].lpFnc)(lpsOpt,lpsVal,rRecoOpts[nI].lpDst);
    if(bOnline && rRecoOpts[nI].eTyp==OT_LOAD){
      enum cfgload eTyp=(enum cfgload)rRecoOpts[nI].lpDst;
      loadobj(cfgload[eTyp],eTyp);
    }
    return rRecoOptTyp[rRecoOpts[nI].eTyp].bArg;
  }
  usage("Unknown option: \"%s\"\n",lpsOpt);
  return TRUE;
}

void loadcfg(char *lpBuf){
  char *lpsLine;
  while((lpsLine=dlp_strsep(&lpBuf,"\n\r",NULL))){
    char *lpsP1, *lpsP2, *lpsP3;
    lpsP1=lpsLine;
    if((lpsP2=strchr(lpsP1,'#'))) *lpsP2='\0';
    else lpsP2=lpsP1+strlen(lpsP1);
    while(*lpsP1==' ' || *lpsP1=='\t') lpsP1++;
    while(lpsP1!=lpsP2 && (*(lpsP2-1)==' ' || *(lpsP2-1)=='\t' ||
          *(lpsP2-1)=='\n' || *(lpsP2-1)=='\r'))
      *(--lpsP2)='\0';
    if(lpsP1==lpsP2) continue;
    if(!(lpsP3=lpsP2=strchr(lpsP1,'=')))
      usage("Malformed line in config file: \"%s\"",lpsLine);
    *(lpsP2++)='\0';
    while(*lpsP2==' ' || *lpsP2=='\t') lpsP2++;
    while(lpsP1!=lpsP3 && (*(lpsP3-1)==' ' || *(lpsP3-1)=='\t')) *(--lpsP3)='\0';
    if(lpsP1==lpsP3) usage("Malformed option in config file: \"%s\"",lpsLine);
    setoption(lpsP1,lpsP2,FALSE);
  }
}

void loadcfgfile(char *lpsFN){
  FILE *fd;
  char *lpBuf;
  INT32 nLen;
  if(!(fd=fopen(lpsFN,"r"))) usage("Unable to open config file \"%s\"",lpsFN);
  fseek(fd,0,SEEK_END);
  nLen=ftell(fd);
  fseek(fd,0,SEEK_SET);
  lpBuf=dlp_malloc(nLen+1);
  nLen=fread(lpBuf,1,nLen,fd);
  fclose(fd);
  lpBuf[nLen]='\0';
  loadcfg(lpBuf);
  dlp_free(lpBuf);
}

void loadcfgbuf(char *lpBuf){
  INT32 nLen=strlen(lpBuf)+1;
  char *lpBuf2=dlp_malloc(nLen);
  memcpy(lpBuf2,lpBuf,nLen);
  loadcfg(lpBuf2);
  dlp_free(lpBuf2);
}

void pfainit(){
  dlm_fba_doframing_initparam(&rCfg.rPfa.lpFba);
  rCfg.rPfa.nCoeff                   = rCfg.rDFea.nPfaDim;
  rCfg.rPfa.nSrate                   = rCfg.nSigSampleRate;
  rCfg.rPfa.lpFba.nCrate             = 160;
  rCfg.rPfa.lpFba.nLen               = 512;
  rCfg.rPfa.lpFba.nPreem             = 0.;
  rCfg.rPfa.lpFba.nWlen              = 400;
  strcpy(rCfg.rPfa.lpFba.lpsWtype,   "Blackman");
  memset(&rCfg.rPfa.lpCnvc,0,sizeof(MLP_CNVC_TYPE));
  rCfg.rPfa.lpCnvc.lambda            = rCfg.rPfa.lpFba.nLambda;
  dlp_strcpy(rCfg.rPfa.lpCnvc.type,rCfg.bNoiseRdc?"MTN":"MT");
  IF_NOK(dlm_mf_init(&rCfg.rPfa.lpCnvc,rCfg.rPfa.lpFba.nLen,rCfg.rPfa.nCoeff,rCfg.rPfa.lpFba.nMinLog)) usage("dlm_mf_init failed.");
  rCfg.rPfa.lpWindow=(FLOAT32 *)dlp_malloc(sizeof(FLOAT32)*rCfg.rPfa.lpFba.nWlen);
  IF_NOK(dlm_fba_makewindow(rCfg.rPfa.lpWindow,rCfg.rPfa.lpFba.nWlen,rCfg.rPfa.lpFba.lpsWtype,rCfg.rPfa.lpFba.nWnorm)) usage("dlm_fba_makewindowF failed.");
  if(rCfg.bNoiseRdc) dlm_noisesetup(rCfg.nNoiseRdcLen,rCfg.nNoiseRdcPrc,NULL,0);
}

void vadinit(){
  rCfg.rVAD.nVadType      = VAD_GMM;
  rCfg.rVAD.nSigThr       = 0.001;
  rCfg.rVAD.nPfaThr       = 10000.;
  rCfg.rVAD.nGmmThr       = -log(0.4);
  dlm_vad_initparam(&rCfg.rVAD.lpBas);
  rCfg.rVAD.lpBas.nPre    = 10;
  rCfg.rVAD.lpBas.nPost   = 10;
  rCfg.rVAD.lpBas.nMinSi  = 20;
  rCfg.rVAD.lpBas.nMaxSp  = rCfg.rVAD.nMaxSp;
}

void searchload(INT32 nFstSel){
  CFstsearch_Load(rCfg.rDSession.itSP,rCfg.rDSession.itRN,nFstSel);
  rTmp.nFstSel=nFstSel;
}

void searchinit(){
  CFstsearch *itSP, *itSPr;
  if(!rCfg.rDSession.itSP){ ICREATEEX(CFstsearch,rCfg.rDSession.itSP, "itSP", NULL); }
  else CFstsearch_Reset(BASEINST(rCfg.rDSession.itSP),TRUE);
  itSP=rCfg.rDSession.itSP;
  BASEINST(itSP)->m_nCheck=rCfg.rSearch.nDebug;
  switch(rCfg.rSearch.eTyp){
    case RS_tp: snprintf(itSP->m_lpsAlgo,3,"tp"); break;
    case RS_as: snprintf(itSP->m_lpsAlgo,3,"as"); break;
  }
  itSP->m_nNumpaths=1;
  switch(rCfg.rSearch.eRejTyp){
  case RR_two:
    itSP->m_nNumpaths=2;
  break;
  case RR_off: break;
  case RR_phn: snprintf(itSP->m_lpsBt,2,"t"); break;
  }
  if(rCfg.rSearch.bPrn){
    itSP->m_nAsQsize=MAX(UD_XXS(rCfg.rDSession.itRN),32767);
    itSP->m_nAsPrnf= rCfg.rSearch.eRejTyp==RR_two ? rCfg.rSearch.nAS2Prn : rCfg.rSearch.nASPrn1;
    itSP->m_nTpPrnw=rCfg.rSearch.nTPPrnW;
    itSP->m_nTpPrnh=rCfg.rSearch.nTPPrnH;
  }
  itSP->m_nTpThreads=rCfg.rSearch.nThreads;
  CFstsearch_Restart(itSP);
  searchload(rTmp.nFstSel);
  if(rCfg.rSearch.eRejTyp!=RR_phn) return;
  if(!rCfg.rDSession.itSPr){ ICREATEEX(CFstsearch,rCfg.rDSession.itSPr,"itSPr",NULL); }
  else CFstsearch_Reset(BASEINST(rCfg.rDSession.itSPr),TRUE);
  itSPr=rCfg.rDSession.itSPr;
  switch(rCfg.rSearch.eTyp){
    case RS_tp: snprintf(itSPr->m_lpsAlgo,3,"tp"); break;
    case RS_as: snprintf(itSPr->m_lpsAlgo,3,"as"); break;
  }
  if(rCfg.rSearch.bPrn){
    itSPr->m_nAsQsize=MAX(UD_XXS(rCfg.rDSession.itRNr),32767);
    itSPr->m_nAsPrnf=rCfg.rSearch.nASPrn2;
    itSPr->m_nTpPrnw=rCfg.rSearch.nTPPrnW;
    itSPr->m_nTpPrnh=rCfg.rSearch.nTPPrnH;
  }
  if(rCfg.rSearch.eRejTyp==RR_phn) snprintf(itSPr->m_lpsBt,2,"t");
/*  itSPr->m_lpsAsAheutype="exist"; TODO: set through setfield
  itSPr->m_lpsAsSheutype="exist"; */
  CFstsearch_Restart(itSPr);
  CFstsearch_Load(itSPr,rCfg.rDSession.itRNr,0);
}

void tmpfileinit(){
  ICREATEEX(CDlpFile,rTmp.iFile,"tmp.file",NULL);
}

void tmpinit(){
  ICREATEEX(CData,rTmp.idSig,"tmp.sig",NULL);
  ICREATEEX(CData,rTmp.idFea,"tmp.fea",NULL);
  ICREATEEX(CData,rTmp.idNld,"tmp.nld",NULL);
  if(rCfg.rDVAD.nSfaDim){
    ICREATEEX(CData,rTmp.idVFea,"tmp.vfea",NULL);
    ICREATEEX(CData,rTmp.idVNld,"tmp.vnld",NULL);
    CData_Array(rTmp.idVFea,T_FLOAT,rCfg.rDVAD.nSfaDim,1);
  }
  rTmp.sSigFname="";
  rTmp.lpColSig=NULL;
  rTmp.nColSigLen=0;
}

void tmpdone(){
  if(rTmp.iFile) IDESTROYFILE(rTmp.iFile);
  if(rTmp.idSig) IDESTROY(rTmp.idSig);
  if(rTmp.idFea) IDESTROY(rTmp.idFea);
  if(rTmp.idNld) IDESTROY(rTmp.idNld);
  if(rTmp.idVFea) IDESTROY(rTmp.idVFea);
  if(rTmp.idVNld) IDESTROY(rTmp.idVNld);
}

BOOL loadgmm(char *lpsVal,CGmm **itGM,INT32 *nDim){
  if(!itGM[0]) ICREATEEX(CGmm, itGM[0], "session.gmm",NULL);
  IF_NOK(restore(BASEINST(itGM[0]), env_replace(lpsVal), CGmm_Deserialize, CGmm_DeserializeXml)) return FALSE;
  *nDim = CGmm_GetDim(itGM[0]);
  return TRUE;
}

BOOL loadfst(char *lpsVal,CFst **itFst,FST_TID_TYPE **lpTI){
  ICREATEEX(CFst, itFst[0], "dlg", NULL);
  IF_NOK(restore(BASEINST(itFst[0]), env_replace(lpsVal), CFst_Deserialize, CFst_DeserializeXml)) return FALSE;
  if(lpTI) lpTI[0] = CFst_STI_Init(itFst[0],0,FSTI_SORTINI);
  return TRUE;
}

BOOL loadobj(char *lpsVal,enum cfgload eTyp){
  CDlpObject*  iObj;
  CDlpObject** idFields;
  static const char *lpFld_feainfo[]={"idDlt","data","idDltW","data","idX","data","idW","data",NULL};
  static const char *lpFld_sesinfo[]={"itRN","fst","itRNr","fst","itGP","fst","idLMtos","data",NULL};
  static const char *lpFld_vadinfo[]={"idX","data","idW","data","itGm","gmm",NULL};
  const char** lpFld=NULL;
  INT32  nNFld;
  INT32  nI,nJ;

  switch(eTyp){
    case CL_FEA: lpFld=lpFld_feainfo; break;
    case CL_SES: lpFld=lpFld_sesinfo; break;
    case CL_VAD: lpFld=lpFld_vadinfo; break;
    case CL_GMM: return loadgmm(lpsVal,&rCfg.rDSession.itGM,&rCfg.rDFea.nSfaDim);
    case CL_DLG: return loadfst(lpsVal,&rCfg.rDDlg.itDlg,&rCfg.rDDlg.lpTI);
    default: return FALSE;
  }
  for(nNFld=0;lpFld[nNFld*2];) nNFld++;

  ICREATEEX(CDlpObject,iObj,"setopt.obj",NULL);
  idFields = (CDlpObject**)dlp_calloc(nNFld,sizeof(CDlpObject*));
  for(nI=0;nI<nNFld;nI++)
    REGISTER_FIELD_EX(iObj, lpFld[nI*2], "",idFields+nI,NULL,"",0,6002,1,lpFld[nI*2+1],0);

  IF_NOK(restore(BASEINST(iObj), env_replace(lpsVal), CDlpObject_Deserialize, CDlpObject_DeserializeXml)) return FALSE;
  
  if(eTyp==CL_FEA){
    CData *idDeltaT=AS(CData,CDlpObject_FindInstanceWord(iObj,"idDlt",NULL));
    CData *idDeltaW=AS(CData,CDlpObject_FindInstanceWord(iObj,"idDltW",NULL));
    CData *idX=AS(CData,CDlpObject_FindInstanceWord(iObj,"idX", NULL));
    CData *idW=AS(CData,CDlpObject_FindInstanceWord(iObj,"idW", NULL));
    rCfg.rDFea.nDeltaWL=CData_GetNComps(idDeltaW)/2;
    rCfg.rDFea.lpDeltaW=(FLOAT32*)dlp_malloc((2*rCfg.rDFea.nDeltaWL+1)*sizeof(FLOAT32));
    for(nI=0;nI<2*rCfg.rDFea.nDeltaWL+1;nI++)
      rCfg.rDFea.lpDeltaW[nI]=CData_Dfetch(idDeltaW,0,nI);
    rCfg.rDFea.nDeltaDim=rCfg.rDFea.nPfaDim=CData_GetNComps(idDeltaT);
    rCfg.rDFea.lpDeltaT=(FLOAT32*)dlp_malloc(rCfg.rDFea.nPfaDim*2*sizeof(FLOAT32));
    for(nI=0;nI<2;nI++) for(nJ=0;nJ<rCfg.rDFea.nPfaDim;nJ++)
      if((rCfg.rDFea.lpDeltaT[nI*rCfg.rDFea.nPfaDim+nJ] =
          CData_Dfetch(idDeltaT,nI,nJ))!=0.)
        rCfg.rDFea.nDeltaDim++;
    if(!CData_IsEmpty(idX)){
      if(CData_GetNRecs(idX)!=1 || CData_GetNComps(idX)!=rCfg.rDFea.nDeltaDim)
        usage("Dimension of normalization vector missmatch.");
      rCfg.rDFea.lpX=(FLOAT32*)dlp_malloc(rCfg.rDFea.nDeltaDim*sizeof(FLOAT32));
      for(nI=0;nI<rCfg.rDFea.nDeltaDim;nI++)
        rCfg.rDFea.lpX[nI]=CData_Dfetch(idX,0,nI);
    }else if(rCfg.rDFea.lpX) dlp_free(rCfg.rDFea.lpX);
    if(!CData_IsEmpty(idW)){
      if(CData_GetNRecs(idW)!=rCfg.rDFea.nDeltaDim || CData_GetNComps(idW)!=rCfg.rDFea.nDeltaDim)
        usage("Dimension of PCA matrix missmatch.");
      rCfg.rDFea.lpW=(FLOAT32*)dlp_malloc(rCfg.rDFea.nDeltaDim*rCfg.rDFea.nDeltaDim*sizeof(FLOAT32));
      for(nI=0;nI<rCfg.rDFea.nDeltaDim;nI++)
        for(nJ=0;nJ<rCfg.rDFea.nDeltaDim;nJ++)
          rCfg.rDFea.lpW[nI*rCfg.rDFea.nDeltaDim+nJ]=CData_Dfetch(idW,nI,nJ);
    }else if(rCfg.rDFea.lpW) dlp_free(rCfg.rDFea.lpW);
  }

  if(eTyp==CL_SES){
    if(!rCfg.rDSession.itRN)    ICREATEEX(CFst, rCfg.rDSession.itRN,   "session.rn",   NULL);
    if(!rCfg.rDSession.itRNr)   ICREATEEX(CFst, rCfg.rDSession.itRNr,  "session.rnr",  NULL);
    if(!rCfg.rDSession.itGP)    ICREATEEX(CFst, rCfg.rDSession.itGP,   "session.gp",   NULL);
    CFst_Copy(BASEINST(rCfg.rDSession.itRN),CDlpObject_FindInstanceWord(iObj,"itRN", NULL));
    CFst_Copy(BASEINST(rCfg.rDSession.itRNr),CDlpObject_FindInstanceWord(iObj,"itRNr", NULL));
    CFst_Copy(BASEINST(rCfg.rDSession.itGP),CDlpObject_FindInstanceWord(iObj,"itGP", NULL));
    if(CData_IsEmpty(AS(CData,rCfg.rDSession.itRN->os)))
      CData_Copy(rCfg.rDSession.itRN->os,CDlpObject_FindInstanceWord(iObj,"idLMtos", NULL));
    searchinit();
  }

  if(eTyp==CL_VAD){
    CData *idX=AS(CData,CDlpObject_FindInstanceWord(iObj,"idX", NULL));
    CData *idW=AS(CData,CDlpObject_FindInstanceWord(iObj,"idW", NULL));
    rCfg.rDVAD.nPfaDim=CData_GetNComps(idX);
    if(!rCfg.rDVAD.nPfaDim) usage("empty vad.idX not supported");
    if(CData_GetNRecs(idW)!=rCfg.rDVAD.nPfaDim || CData_GetNComps(idW)!=rCfg.rDVAD.nPfaDim)
        usage("Dimension of VAD PCA matrix missmatch.");
    rCfg.rDVAD.lpX=(FLOAT32*)dlp_malloc(rCfg.rDVAD.nPfaDim*sizeof(FLOAT32));
    for(nI=0;nI<rCfg.rDVAD.nPfaDim;nI++)
      rCfg.rDVAD.lpX[nI]=CData_Dfetch(idX,0,nI);
    rCfg.rDVAD.lpW=(FLOAT32*)dlp_malloc(rCfg.rDVAD.nPfaDim*rCfg.rDVAD.nPfaDim*sizeof(FLOAT32));
    for(nI=0;nI<rCfg.rDVAD.nPfaDim;nI++)
      for(nJ=0;nJ<rCfg.rDVAD.nPfaDim;nJ++)
        rCfg.rDVAD.lpW[nI*rCfg.rDVAD.nPfaDim+nJ]=CData_Dfetch(idW,nI,nJ);
    ICREATEEX(CGmm,rCfg.rDVAD.itGM,"vad.gmm",NULL);
    CGmm_Copy(BASEINST(rCfg.rDVAD.itGM),CDlpObject_FindInstanceWord(iObj,"itGm", NULL));
    rCfg.rDVAD.nSfaDim=CGmm_GetDim(rCfg.rDVAD.itGM);
  }

  IDESTROY(iObj);
  dlp_free(idFields);

  return TRUE;
}

BOOL cfginit(int argc,char** argv){
  INT32 nI;
  tmpfileinit();
  memset(&rCfg,0,sizeof(rCfg));
  for(nI=0;nI<CL_N;nI++) cfgload[nI][0]='\0';
  loadcfgbuf(cfgdef);
  for(nI=1;nI<argc;nI++){
    if(argv[nI][0]!='-') readfilelist(argv[nI]);
    else if(!strncmp(argv[nI],"-cfg",5)) loadcfgfile(argv[++nI]);
    else if(setoption(argv[nI]+1,nI+1<argc ? argv[nI+1] : NULL,FALSE)) nI++;
  }
  if(rCfg.bHelp) usage(NULL);
  if(rCfg.bAudioDevList){ audio_devlist(); return FALSE; }
  if(rCfg.bOptionsList){ options_list(); return FALSE; }
  if(rCfg.rVAD.bNoLimit){
    rCfg.rVAD.nMinSp  = 0;
    rCfg.rVAD.nMaxSp  = 6000;
    rCfg.rVAD.nSigMin = 0;
  }
  if(!cfgload[CL_FEA][0]) usage("No feature info found (set data.feainfo).");
  if(!cfgload[CL_SES][0]) usage("No session info found (set data.sesinfo).");
  if(!cfgload[CL_GMM][0]) usage("No gmm found (set data.gmm).");
  for(nI=0;nI<CL_N;nI++) if(cfgload[nI][0]) if(!loadobj(cfgload[nI],nI))
    usage("Unable to load: \"%s\"",cfgload[nI]);
  pfainit();
  vadinit();
  tmpinit();
  if (rCfg.eOut==O_gui) optdump();
  return TRUE;
}

void cfgdone(){
  dlm_mf_done(&rCfg.rPfa.lpCnvc);
  dlm_fft_cleanup();
  if(rCfg.rFlst.lpF) dlp_free(rCfg.rFlst.lpF);
  if(rCfg.rDDlg.lpTI) CFst_STI_Done(rCfg.rDDlg.lpTI);
  if(rCfg.rDDlg.itDlg) IDESTROYFST(rCfg.rDDlg.itDlg);
  if(rCfg.rDSession.itGM) IDESTROY(rCfg.rDSession.itGM);
  if(rCfg.rDFea.lpDeltaW) dlp_free(rCfg.rDFea.lpDeltaW);
  if(rCfg.rDFea.lpDeltaT) dlp_free(rCfg.rDFea.lpDeltaT);
  if(rCfg.rDFea.lpX) dlp_free(rCfg.rDFea.lpX);
  if(rCfg.rDFea.lpW) dlp_free(rCfg.rDFea.lpW);
  if(rCfg.rDSession.itRN) IDESTROYFST(rCfg.rDSession.itRN);
  if(rCfg.rDSession.itRNr) IDESTROYFST(rCfg.rDSession.itRNr);
  if(rCfg.rDSession.itGP) IDESTROYFST(rCfg.rDSession.itGP);
  if(rCfg.rDSession.itSP) IDESTROY(rCfg.rDSession.itSP);
  if(rCfg.rDSession.itSPr) IDESTROY(rCfg.rDSession.itSPr);
  if(rCfg.rDVAD.lpX) dlp_free(rCfg.rDVAD.lpX);
  if(rCfg.rDVAD.lpW) dlp_free(rCfg.rDVAD.lpW);
  if(rCfg.rDVAD.itGM) IDESTROY(rCfg.rDVAD.itGM);
  if(rCfg.rPfa.lpWindow) dlp_free(rCfg.rPfa.lpWindow);
  tmpdone();
}
