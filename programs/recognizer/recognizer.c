/* dLabPro program recognizer (dLabPro recognizer)
 * - standalone recognizer
 *
 * AUTHOR : Gunntram Strecha
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

#include "recognizer.h"
#include "dlp_math.h"

#ifdef __USE_PORTAUDIO
#include "portaudio.h"
#endif

#ifdef __MINGW32__
	#ifdef __MEASURE_TIME
		#undef __MEASURE_TIME
	#endif
#endif

#ifdef __MEASURE_TIME
#ifdef __TMS
#include <time.h>
#else
#include <sys/times.h>
#include <unistd.h>
#endif
#endif

/*#define HTKFEA*/

INT64 nFrame = 0;         /* Global frame number */
INT64 nLastActive = 0;    /* Frame number of most recent activity */

struct log {
  FILE *fdRaw;
  FILE *fdLab1;
  FILE *fdLab2;
  BOOL bLab1;
  BOOL bLab2;
} lpLog = { NULL, NULL, NULL, FALSE, FALSE };

void rerror(const char *lpsErr,...){
  va_list ap;
  fprintf(stderr,"\nError: ");
  va_start(ap,lpsErr);
  vfprintf(stderr,lpsErr,ap);
  va_end(ap);
  fprintf(stderr,"\n"); fflush(stderr);
}

void routput(enum recoout eOut,char bMark,const char *lpsMsg,...){
  va_list ap;
  if(eOut>rCfg.eOut) return;
  if(bMark) fprintf(stdout,"%s: ",recoout_str[eOut]);
  va_start(ap,lpsMsg);
  vfprintf(stdout,lpsMsg,ap); fflush(stdout);
  va_end(ap);
}

#if defined __TMS || !defined __MEASURE_TIME
struct tms {
  UINT64 tms_utime;
  UINT64 tms_stime;
  UINT64 tms_cutime;
  UINT64 tms_cstime;
};
#endif
struct tms  tms_c_star;
struct tms  tms_c_anal;
struct tms  tms_c_dens;
struct tms  tms_c_reco;
struct tms  tms_all = { 0, 0, 0, 0 };
struct tms  tms_anal = { 0, 0, 0, 0 };
struct tms  tms_dens = { 0, 0, 0, 0 };
struct tms  tms_reco = { 0, 0, 0, 0 };

void updatetimes(struct tms *dst,struct tms *start,struct tms *end)
{
#ifdef __MEASURE_TIME
  if(!start || !end){
    dst->tms_cstime = 0;
    dst->tms_cutime = 0;
    dst->tms_stime  = 0;
    dst->tms_utime  = 0;
  }else{
    dst->tms_cstime += end->tms_cstime - start->tms_cstime; 
    dst->tms_cutime += end->tms_cutime - start->tms_cutime; 
    dst->tms_stime  += end->tms_stime  - start->tms_stime; 
    dst->tms_utime  += end->tms_utime  - start->tms_utime; 
  }
#endif
}

#ifdef __MEASURE_TIME
#ifdef __TMS
INT64 clk_timer=0;

void clk_timer_inc(){
  clk_timer++;
}

#define CLK_TCK    1000.0
#endif
#endif

UINT64 measuretime(struct tms *buf)
{
#ifdef __MEASURE_TIME
#ifdef __TMS
  buf->tms_cutime=clk_timer;
  buf->tms_utime=clk_timer;
  buf->tms_cstime=0;
  buf->tms_stime=0;
  return clk_timer;
#else
  return times(buf);
#endif
#else
	return 0;
#endif
}

void outputtimes()
{
#ifdef __MEASURE_TIME
  FLOAT32 nDuration=rTmp.nDuration;
#ifndef __TMS
  FLOAT32 norm=(FLOAT32)sysconf(_SC_CLK_TCK);
#else
  FLOAT32 norm=CLK_TCK;
#endif
  routput(O_sta,1,"Processor times for %.1fs signal\n",nDuration);
  routput(O_sta,1,"   sum: %-7s   %-7s  %-7s  %-7s","all","anal","dens","reco");
  routput(O_sta,0,"  rtf: %-7s  %-7s  %-7s  %-7s\n","all","anal","dens","reco");
  routput(O_sta,1,"user: %7.3fs  %7.3fs %7.3fs %7.3fs",  (float)tms_all.tms_utime/norm,(float)tms_anal.tms_utime/norm,(float)tms_dens.tms_utime/norm,(float)tms_reco.tms_utime/norm);
  norm*=(FLOAT32)nDuration/100.;
  routput(O_sta,0,"    %7.3f%%  %7.3f%% %7.3f%% %7.3f%%\n",(float)tms_all.tms_utime/norm,(float)tms_anal.tms_utime/norm,(float)tms_dens.tms_utime/norm,(float)tms_reco.tms_utime/norm);
  norm/=(FLOAT32)nDuration/100.;
  routput(O_sta,1,"syst: %7.3fs  %7.3fs %7.3fs %7.3fs",  (float)tms_all.tms_stime/norm,(float)tms_anal.tms_stime/norm,(float)tms_dens.tms_stime/norm,(float)tms_reco.tms_stime/norm);
  norm*=(FLOAT32)nDuration/100.;
  routput(O_sta,0,"    %7.3f%%  %7.3f%% %7.3f%% %7.3f%%\n",(float)tms_all.tms_stime/norm,(float)tms_anal.tms_stime/norm,(float)tms_dens.tms_stime/norm,(float)tms_reco.tms_stime/norm);
#endif
}

void evaluation()
{
  struct recores *lpRes=&rTmp.rRes;
/*  FLOAT32 nFaqTP=2;
  FLOAT32 nFaqTN=1;
  FLOAT32 nCor=lpRes->nTP+lpRes->nFN;
  FLOAT32 nNCor=lpRes->nTN+lpRes->nFP;
  FLOAT32 nAcc2=((nCor ? lpRes->nTP/nCor : 1.)*nFaqTP + (nNCor ? lpRes->nTN/nNCor : 1.)*nFaqTN) / (nFaqTN+nFaqTP);*/
  routput(O_cmd,1,"N:   %i\n",(int)lpRes->nN);
  routput(O_cmd,1,"TP:  %i\n",(int)lpRes->nTP);
  if(rCfg.rRej.eTyp!=RR_off){
    routput(O_cmd,1,"TP:  %i\n",(int)lpRes->nTP);
    routput(O_cmd,1,"FP:  %i\n",(int)lpRes->nFP);
    routput(O_cmd,1,"TN:  %i\n",(int)lpRes->nTN);
    routput(O_cmd,1,"FN:  %i\n",(int)lpRes->nFN);
    routput(O_cmd,1,"TO:  %i\n",(int)lpRes->nTO);
    routput(O_cmd,1,"FO:  %i\n",(int)lpRes->nFO);
  }
  routput(O_cmd,1,"WRR: %.3f%%\n",(lpRes->nTP+lpRes->nFN)/(float)lpRes->nN*100.);
  if(rCfg.rRej.eTyp!=RR_off){
    routput(O_cmd,1,"ACC: %.3f%%\n",(lpRes->nTP+lpRes->nTN+lpRes->nTO)/(float)(lpRes->nN+lpRes->nNO)*100.);
/*    routput(O_cmd,1,"ACC2:%.3f%%\n",nAcc2*100.);*/
    routput(O_cmd,1,"FAR: %.3f%%\n",(lpRes->nFP+lpRes->nTN+lpRes->nNO==0)?0.:((lpRes->nFP+lpRes->nFO)/(float)(lpRes->nFP+lpRes->nTN+lpRes->nNO)*100.));
    routput(O_cmd,1,"FRR: %.3f%%\n",(lpRes->nTP+lpRes->nFN==0)?0.:(lpRes->nFN/(float)(lpRes->nTP+lpRes->nFN)*100.));
  }
}

const char *result_text(const char *sRes)
{
  static char sCnv[STR_LEN];
  char *sPos;
  snprintf(sCnv,STR_LEN,sRes);
  /* 20130731 MW: <KOMMA> is deprecated! */
  for(sPos=sCnv;(sPos=strstr(sPos,"<KOMMA>"));){
    sPos[0]=',';
    memmove(sPos+1,sPos+strlen("<KOMMA>"),strlen(sPos)-strlen("<KOMMA>")+1);
  }
  /* 20130731 MW: <CNG...> is deprecated! */
  for(sPos=sCnv;(sPos=strstr(sPos,"<CNG"));){
    char *sStart=sPos+4;
    char *sEnd=strstr(sPos,">");
    INT64 num=0;
    if(!sEnd){ sPos+=4; continue; }
    *sEnd='\0';
    sStart=strtok(sStart,"|");
    while(sStart){
      INT64 i,p=1;
      for(i=strlen(sStart)-1;i>=0;i--,p*=10) if(sStart[i]>='0' && sStart[i]<='9') num+=(sStart[i]-'0')*p;
      sStart=strtok(NULL,"|");
    }
    sprintf(sPos,"%i",(int)num);
    sPos+=strlen(sPos);
    memmove(sPos,sEnd+1,strlen(sEnd+1)+1);
  }
  return sCnv;
}

const char *phone_tab="2\0\0" "6\0\0" "9\0\0" ".\0\0" "#\0\0" "@\0\0" "C\0\0" "E\0\0" "E:\0"  "I\0\0" \
                "N\0\0" "O\0\0" "OY\0"  "Q\0\0" "S\0\0" "U\0\0" "Y\0\0" "a\0\0" "a:\0"  "aI\0"  \
                "aU\0"  "b\0\0" "d\0\0" "e:\0"  "f\0\0" "g\0\0" "h\0\0" "i:\0"  "j\0\0" "k\0\0" \
                "l\0\0" "m\0\0" "n\0\0" "o:\0"  "p\0\0" "r\0\0" "s\0\0" "t\0\0" "u:\0"  "v\0\0" \
                "x\0\0" "y:\0"  "z\0\0";

void dlg_upd(const char *lpsRes)
{
  BYTE* lpT=NULL;
  CFst* itRgx;
  if(!rCfg.rDDlg.itDlg || !rCfg.rDDlg.lpTI || CData_IsEmpty(AS(CData,rCfg.rDDlg.itDlg->is))) return;
  ICREATEEX(CFst,itRgx,"regex",NULL);
  while((lpT=CFst_STI_TfromS(rCfg.rDDlg.lpTI,rTmp.nFstSel,lpT))){
    INT32 nTis=*CFst_STI_TTis(rCfg.rDDlg.lpTI,lpT);
    const char *lpsMatch=CData_Sfetch(AS(CData,rCfg.rDDlg.itDlg->is),nTis,0);
    INT32 nS,nL;
    CFst_RegexCompile(itRgx,lpsMatch);
    CFst_RegexMatch_int(itRgx,lpsRes,&nS,&nL);
    if(nS<0 || nL<0) continue;
    searchload(*CFst_STI_TTer(rCfg.rDDlg.lpTI,lpT));
    routput(O_sta,1,"voc change: %i",rTmp.nFstSel);
    if(rCfg.rDDlg.lpTI->nOfTTos>0 && !CData_IsEmpty(AS(CData,rCfg.rDDlg.itDlg->os)))
		routput(O_sta,0," (%s)",
			CData_Sfetch(AS(CData,rCfg.rDDlg.itDlg->os),
				*CFst_STI_TTos(rCfg.rDDlg.lpTI,lpT),0));
    routput(O_sta,0,"\n");
    break;
  }
  IDESTROYFST(itRgx);
}

BOOL isfvr(CFst* itDC,INT32 *nBO,INT32 *nBC){
  /* Check for FVR[ at begin of command */
  if(dlp_strncmp(CData_Sfetch(AS(CData,itDC->ud),0,0),"FVR[",4)) return FALSE;
  /* Check for bracket in symbol table */
  IF_NOK(CFvrtools_CheckSeq(rTmp.iFvr,NULL,AS(CData,itDC->os),nBO,nBC)) return FALSE;
  return TRUE;
}

INT32 data_findsymbol(CData *idS,const char *lpS){
  INT32 nXS=CData_GetNRecs(idS);
  INT32 nS;
  INT32 nLen=MIN(strlen(lpS)+1,CData_GetCompType(idS,0));
  for(nS=0;nS<nXS;nS++) if(!dlp_strncmp(CData_Sfetch(idS,nS,0),lpS,nLen)) return nS;
  return -1;
}

INT32 data_findcompoffset(CData *idX,const char *lpsName){
  INT32 nId=CData_FindComp(idX,lpsName);
  if(nId<0) return nId;
  return CData_GetCompOffset(idX,nId);
}

FLOAT32 confidence_phn(CFst* itDC, CFst* itDCr){
  INT32 ssil=data_findsymbol(AS(CData,itDCr->os),"#");  /* Silence symbol index */
  INT32 sgar=data_findsymbol(AS(CData,itDCr->os),".");  /* Garbage symbol index */
  INT32 sbo=-1,sbc=-1;                                  /* Bracket symbol indices */
  BOOL bfvr=isfvr(itDC,&sbo,&sbc);                      /* FVR confidence calculation? */
  CData *rtd=AS(CData,itDC->td);                        /* Result transition table */
  CData *ftd=AS(CData,itDCr->td);                       /* Reference transition table */
  INT32 rl,fl;                                          /* Transistions record length in result / reference */
  BYTE  *rt,*ft;                                        /* Transistions pointer in result / reference */
  BYTE  *ret,*fet;                                      /* Pointer after last transition in result / reference */
  INT32 orp,ofp;                                        /* Offset of ~PHN component in result / reference */
  INT32 orw,ofw;                                        /* Offset of ~LSR component in result / reference */
  INT32 oro;                                            /* Offset of ~TOS component in result */
  INT32 orc;                                            /* Offset of ~CNF component in result */
  FLOAT32 nad,ned;                                      /* Normalized acoustic and edit distant */
  FLOAT32 tad,ted,lam;                                  /* Threshold for acoustic and edit distant */
  struct {
    INT32 n,neq;
    FLOAT32 rw,fw;
  } *frm, *fi;
  struct {
    INT32 ibo;
    FLOAT32 *tcnf;
  } *lvl=NULL, *li=NULL;

  if(bfvr) CData_AddComp(rtd,"~CNF",T_FLOAT);
  rt =CData_XAddr(rtd,0,0);
  ft =CData_XAddr(ftd,0,0);
  rl =CData_GetRecLen(rtd);
  fl =CData_GetRecLen(ftd);
  ret=rt+rl*CData_GetNRecs(rtd);
  fet=ft+fl*CData_GetNRecs(ftd);
  orp=data_findcompoffset(rtd,"~PHN");
  ofp=data_findcompoffset(ftd,"~PHN");
  if(orp<0 || ofp<0) return 0;
  orw=data_findcompoffset(rtd,"~LSR");
  ofw=data_findcompoffset(ftd,"~LSR");
  oro=data_findcompoffset(rtd,"~TOS");
  orc=data_findcompoffset(rtd,"~CNF");
  if(oro<0 || orc<0) bfvr=FALSE;
  frm=fi=dlp_malloc((MAX(CData_GetNRecs(rtd),CData_GetNRecs(ftd))+1)*sizeof(*frm));
  fi[0].n=fi[0].neq=0;
  fi[0].rw=fi[0].fw=0.;
  if(bfvr) lvl=li=dlp_malloc((MAX(CData_GetNRecs(rtd),CData_GetNRecs(ftd))+1)*sizeof(*lvl));

  tad=rCfg.rRej.nTAD;
  if(rCfg.rSearch.eTyp==RS_as) tad = rCfg.rRej.nASTAD;
  ted=rCfg.rRej.nFVRTED;
  lam=rCfg.rRej.nFVRLAM;

  for(;; rt+=rl,ft+=fl){
    INT32 rp=-1,fp=-1, ro;
    for( ; rt<ret ; rt+=rl){
      if(bfvr && (ro=*(FST_STYPE*)(rt+oro))>=0){
        if(ro==sbo){
          li++;
          li->ibo=fi-frm;
          li->tcnf=NULL;
        }else if(ro==sbc){
          if(li>lvl){
            INT32   n  =fi->n  -frm[li->ibo].n;
            INT32   neq=fi->neq-frm[li->ibo].neq;
            FLOAT32 rw =fi->rw -frm[li->ibo].rw;
            FLOAT32 fw =fi->fw -frm[li->ibo].fw;
            nad = orw>=0 && ofw>=0 ? rw ? ABS(rw-fw) / ABS(rw) : tad : 0.f;
            ned = n ? 1.f - neq/(FLOAT32)n : ted;
            if(li->tcnf) *li->tcnf=lam*MAX(1.f-ned/ted,-1.f)+(1.f-lam)*MAX(1.f-nad/tad,-1.f);
            else rerror("FVR confidence: no output symbol at certained bracket level");
            li--;
          }else rerror("FVR confidence: too many closing brackets ']'");
        }else if(!li->tcnf) li->tcnf=(FLOAT32*)(rt+orc);
      }
      if((rp=*(FST_STYPE*)(rt+orp))>=0) break;
    }
    while(ft<fet && (fp=*(FST_STYPE*)(ft+ofp))<0) ft+=fl;
    if(rt>=ret || ft>=fet) break;
    if(rp==ssil || rp==sgar || fp==ssil || fp==sgar) continue;
    fi[1].n=fi[0].n+1;
    fi[1].neq=fi[0].neq+(rp==fp);
    if(orw>=0 && ofw>=0){
      fi[1].rw=fi[0].rw+*(FST_WTYPE*)(rt+orw);
      fi[1].fw=fi[0].fw+*(FST_WTYPE*)(ft+ofw);
    }
    fi++;
  }
  if(bfvr && li!=lvl) rerror("FVR confidence: too less closing brackets ']'");

  ted=rCfg.rRej.nTED;

  nad = orw>=0 && ofw>=0 ? fi[0].rw ? ABS(fi[0].rw-fi[0].fw) / ABS(fi[0].rw) : tad : 0.f;
  ned = fi[0].n ? 1.f-fi[0].neq / (FLOAT32)fi[0].n : ted;
  routput(O_dbg,1,"rec nad: %.4g ned: %.4g tnad: %.4g tned: %.4g\n",nad,ned,tad,ted);

  dlp_free(frm);
  if(bfvr) dlp_free(lvl);

  return nad<tad && ned<ted;
}

void confidence(CFst* itDC, CFst* itDCr, const char *sLab)
{
  char    lpsRRes[STR_LEN];
  FLOAT32 nGW0 = 0.f;
  FLOAT32 nGW1 = 0.f;
  short   nRAcc=0;
  short   nRCor=0;

  /* Get & store result sentence */
  snprintf(lpsRRes,sizeof(lpsRRes),"%s",result_text(AS(CData,itDC->ud)->m_ftext));
  nGW0=CData_Dfetch(AS(CData,itDC->ud),0,CData_FindComp(AS(CData,itDC->ud),"~GW"));
  switch(rCfg.rRej.eTyp){
  case RR_off: break;
  case RR_phn: nGW1=CData_Dfetch(AS(CData,itDCr->ud),0,CData_FindComp(AS(CData,itDCr->ud),"~GW")); break;
  case RR_two: nGW1=CData_Dfetch(AS(CData,itDC->ud),1,CData_FindComp(AS(CData,itDC->ud),"~GW")); break;
  }
  routput(O_dbg,1,"rec gw: %.5g %.5g\n",nGW0,nGW1);

  if(nGW0 && nGW1){
    switch(rCfg.rRej.eTyp){
    case RR_phn: nRAcc=confidence_phn(itDC,itDCr); break;
    case RR_off: nRAcc = 1; break;
    case RR_two: {
      FLOAT32 nWD = nGW1-nGW0;
      FLOAT32 nNWD = (nGW1-nGW0)/nGW0;
      nRAcc = nWD>rCfg.rRej.nTWOTWD && nNWD>rCfg.rRej.nTWOTNWD;
      routput(O_dbg,1,"rec wd: %.4g nwd: %.4g twd: %.4g tnwd: %.4g\n",nWD,nNWD,rCfg.rRej.nTWOTWD,rCfg.rRej.nTWOTNWD);
    }break;
    }
  }else if(rCfg.rRej.eTyp==RR_off) nRAcc=1;
  CData_AddComp(AS(CData,itDC->ud),"~ACC",T_INT);
  CData_Dstore(AS(CData,itDC->ud),nRAcc,0,CData_GetNComps(AS(CData,itDC->ud))-1);
  /* COR = sRes==sRRes */
  nRCor = (sLab!=NULL && strlen(sLab)>0 && strncmp(sLab,"OOV",3)) ?
    (strlen(lpsRRes)==strlen(sLab) && !strncmp(sLab,lpsRRes,strlen(sLab))) :
    -1;

  if(rCfg.eOut==O_res) printf("%s\n",lpsRRes);
  else{
    snprintf(rTmp.rRes.sLastRes,sizeof(rTmp.rRes.sLastRes),"%s%s%s",nRAcc?"":"(",lpsRRes,nRAcc?"":")");
    routput(O_cmd,1,"");
    if(sLab) routput(O_cmd,0,"lab: %s ",sLab);
  	routput(O_cmd,0,"res: %s ",rTmp.rRes.sLastRes);
    if(sLab) routput(O_cmd,0,"cor: %i ",nRCor);
    if(rCfg.rRej.eTyp!=RR_off) routput(O_cmd,0,"acc: %i",nRAcc);
    routput(O_cmd,0,"\n");
    if(!CData_IsEmpty(AS(CData,itDCr->ud)))
      routput(O_dbg,1,"refrec %s\n",CData_Sfetch(AS(CData,itDCr->ud),0,0));
  }
  
  if((nRCor==1)  && (nRAcc==1)) rTmp.rRes.nTP++;
  if((nRCor==0)  && (nRAcc==1)) rTmp.rRes.nFP++;
  if((nRCor==-1) && (nRAcc==1)) rTmp.rRes.nFO++;
  if((nRCor==1)  && (nRAcc==0)) rTmp.rRes.nFN++;
  if((nRCor==0)  && (nRAcc==0)) rTmp.rRes.nTN++;
  if((nRCor==-1) && (nRAcc==0)) rTmp.rRes.nTO++;
  if(nRCor >= 0) rTmp.rRes.nN++; else rTmp.rRes.nNO++;

  if(nRAcc) dlg_upd(lpsRRes);
}

CFst* fvrgen(CFst* itDC)
{
  CFst *itFvr;

  /* Check for FVR[ at begin of command */
  if(!isfvr(itDC,NULL,NULL)) return NULL;

  /* Copy first unit */
  ICREATEEX(CFst, itFvr,"itFvr",NULL);
  CFst_CopyUi(itFvr,itDC,NULL,0);

  /* Extract output symbol sequence */
  CFst_Invert(itFvr,0);
  CFst_Project(itFvr);
  CData_Xstore(AS(CData,itFvr->td),AS(CData,itFvr->td),
    CData_FindComp(AS(CData,itFvr->td),"~CNF"),1,
    CData_FindComp(AS(CData,itFvr->td),"~LSR"));
  IRESETOPTIONS(AS(CData,itFvr->td));
  CData_DeleteComps(AS(CData,itFvr->td),4,CData_GetNComps(AS(CData,itFvr->td))-4);
  CFst_Lazymin(itFvr);

  /* Convert to FVR */
  IF_NOK(CFvrtools_FromFst(rTmp.iFvr,itFvr,itFvr)){
    IDESTROYFST(itFvr);
    itFvr=NULL;
  }

  return itFvr;
}

void postprocess(CFst* itDC, CFst* itDCr, CFst *itFvr)
{
  routput(O_sta,1,"rec post (fst: %i)\n",rTmp.nFstSel);
  char *sTmp=dlp_tempnam(NULL,"recognizer");
  char sTmpDC[L_PATH];
  char sTmpDCr[L_PATH];
  char sTmpNld[L_PATH];
  char sTmpSig[L_PATH];
  char sTmpFvr[L_PATH];
  char sTmpCfg[L_PATH];
  char sCmd[L_PATH*4];
  CData *idColSig;
  unlink(sTmp);
  snprintf(sTmpDC, L_PATH,"%s-dc.fst", sTmp); dlp_strreplace(sTmpDC ,"\\","/");
  snprintf(sTmpDCr,L_PATH,"%s-dcr.fst",sTmp); dlp_strreplace(sTmpDCr,"\\","/");
  snprintf(sTmpNld,L_PATH,"%s-nld.fst",sTmp); dlp_strreplace(sTmpNld,"\\","/");
  snprintf(sTmpSig,L_PATH,"%s-sig.wav",sTmp); dlp_strreplace(sTmpSig,"\\","/");
  snprintf(sTmpFvr,L_PATH,"%s-fvr.fst",sTmp); dlp_strreplace(sTmpSig,"\\","/");
  snprintf(sTmpCfg,L_PATH,"%s.cfg",    sTmp); dlp_strreplace(sTmpSig,"\\","/");
  rTmp.idNld->m_lpTable->m_fsr = 1000./rCfg.nSigSampleRate*rCfg.rPfa.lpFba.nCrate;
  CDlpObject_Save(BASEINST(itDC),      sTmpDC, SV_XML);
  CDlpObject_Save(BASEINST(itDCr),     sTmpDCr,SV_XML);
  CDlpObject_Save(BASEINST(rTmp.idNld),sTmpNld,SV_XML);
  ICREATEEX(CData,idColSig,"colsig",NULL);
  CData_Array(idColSig,T_FLOAT,1,rTmp.nColSigLen);
  memcpy(CData_XAddr(idColSig,0,0),rTmp.lpColSig,rTmp.nColSigLen*sizeof(FLOAT32));
  idColSig->m_lpTable->m_fsr=1000./rCfg.nSigSampleRate;
  CDlpFile_LibsndfileExport(rTmp.iFile,sTmpSig,BASEINST(idColSig),"wav");
  IDESTROY(idColSig);
  if(itFvr) CDlpObject_Save(BASEINST(itFvr),sTmpFvr,SV_XML);
  else snprintf(sTmpFvr,L_PATH,"NULL");
  opt_tmpcfg(sTmpCfg);
  snprintf(sCmd,L_PATH*4,"%s \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"",
          rCfg.sPostProc,sTmpDC,sTmpDCr,sTmpNld,sTmpSig,sTmpFvr,cfgload[CL_SES],sTmpCfg);
  system(sCmd);
  unlink(sTmpDC);
  unlink(sTmpDCr);
  unlink(sTmpNld);
  unlink(sTmpSig);
  if(itFvr) unlink(sTmpFvr);
  unlink(sTmpCfg);
}

INT32 decode(CFst* itGP, CFst* itRN, CFstsearch *itSP, CFst* itDC)
{
  INT32     ret   = O_K;
#ifndef __TMS
  CFst*   itAux;
  CData*  idAux;
  ICREATEEX(CFst, itAux,"itAux",NULL);
  ICREATEEX(CData,idAux,"idAux",NULL);
#endif
  CData* idLtab = CData_IsEmpty(AS(CData,itRN->os)) ? NULL : AS(CData,itRN->os);

/* decoding */
  itSP->m_bFinal=TRUE;
  CFstsearch_Isearch(itSP,rCfg.rSearch.bIter || CData_IsEmpty(rTmp.idNld) ? NULL : rTmp.idNld);
  CFstsearch_Backtrack(itSP,itDC);

  if(!UD_XXU(itDC)){
    CFst_Addunit(itDC,"");
    CFst_Addstates(itDC,0,1,TRUE);
  }
  
/* add component ~PHN to itDC.td with phonem id's */
#ifndef __NORTTI /* TODO: phn labels in RTTI mode */
  if(rCfg.rRej.eTyp==RR_phn){
    IF_NOK(ret = CFst_Copy(BASEINST(itAux), BASEINST(itDC)))                               goto end;
    IF_NOK(ret = CFst_Invert(itAux, 0))                                                    goto end;
    IF_NOK(ret = CFst_Compose(itAux, itAux, itGP, 0, 0))                                   goto end;
    IF_NOK(ret = CFst_BestN(itAux, itAux, 0, 1, 0))                                        goto end;
    IF_NOK(ret = CData_SelectComps(idAux, AS(CData,itAux->td),
      CData_FindComp(AS(CData,itAux->td),itAux->m_lpsNCTDTOS),1))                        goto end;
    IF_NOK(ret = CData_Join(AS(CData,itDC->td),idAux))                                     goto end;
    IF_NOK(ret = CData_SetCname(AS(CData,itDC->td),
      CData_GetNComps(AS(CData,itDC->td))-1, "~PHN"))                                    goto end;
  }
#endif
/* set unit name to joined word names */
  IF_NOK(ret = CData_InsertComp(AS(CData,itDC->ud),
    CData_GetCname(AS(CData,itDC->ud), 0), 255, 0))                                    goto end;
  IF_NOK(ret = CData_Delete(AS(CData,itDC->ud), AS(CData,itDC->ud), 1, 1))               goto end;
#ifdef __TMS
  if(idLtab)
#endif
  {
    INT64 nNT=UD_XT(itDC,0);
    INT64 nT,nC=0;
    char lpsBuf[STR_LEN];
    INT64 nLen=0;
    BYTE *lpTos=CData_XAddr(AS(CData,itDC->td),0,CData_FindComp(AS(CData,itDC->td),itDC->m_lpsNCTDTOS));
    BYTE *lpTis=CData_XAddr(AS(CData,itDC->td),0,CData_FindComp(AS(CData,itDC->td),itDC->m_lpsNCTDTIS));
    BYTE *lpLab=idLtab ? CData_XAddr(idLtab,0,0) : (BYTE*)phone_tab;
    INT64 nTRecLen=CData_GetRecLen(AS(CData,itDC->td));
    INT64 nLRecLen=idLtab ? CData_GetRecLen(idLtab) : 3;
    lpsBuf[0]='\0';
    for(nT=0;nT<nNT;nT++,lpTos+=nTRecLen,lpTis+=nTRecLen){
      FST_STYPE nTos=*(FST_STYPE*)lpTos;
      FST_STYPE nTis=*(FST_STYPE*)lpTis;
      char lpL[255];
      INT64 nLLen;
      if(nTis>=0) nC++;
      if(nTos<0) continue;
      if(idLtab) snprintf(lpL,254,"%s",lpLab+nLRecLen*nTos);
      else       snprintf(lpL,254,"%i%s ",nC,lpLab+nLRecLen*nTos);
      nC=0;
      nLLen=strlen(lpL);
      if(nLen+nLLen>2046) break;
      /*if(nLen) strcpy(lpsBuf+(nLen++)," ");*/
      strcpy(lpsBuf+nLen,lpL);
      nLen+=nLLen;
    }
    IF_NOK(ret = CData_Sstore(AS(CData,itDC->ud), lpsBuf, 0, 0)) goto end;
    
    char *ft;
    if(!(ft=dlp_malloc(strlen(lpsBuf)+1))) goto end;
    memcpy(ft,lpsBuf,strlen(lpsBuf)+1);
    AS(CData,itDC->ud)->m_ftext=ft;
  }
  
end:
#ifndef __TMS
  IDESTROYFST(itAux);
  IDESTROY(idAux);
#endif
    
  return ret;
}

INT32 recognize(const char *sLab)
{
  INT32      ret = O_K;
  CFst*  itDC;
  CFst*  itDCr;
  CFst*  itRNforce=NULL;
  CFst*  itFvr=NULL;

  ICREATEEX(CFst, itDC, "itDC", NULL);
  ICREATEEX(CFst, itDCr,"itDCr",NULL);

  if(rCfg.bForce){
    FILE *lpForce;
    lpForce=fopen("reco_force_rn.fst","r");
    if(lpForce){
      fclose(lpForce);
      ICREATEEX(CFst,itRNforce,"itRNforce",NULL);
      IF_NOK(restore(BASEINST(itRNforce),"reco_force_rn.fst",CFst_Deserialize,CFst_DeserializeXml)){
        IDESTROY(itRNforce);
        itRNforce=NULL;
      }else
        CFstsearch_Load(rCfg.rDSession.itSP,itRNforce,0);
    }
  }

  /* Decoding */
  IF_NOK(ret=decode(rCfg.rDSession.itGP,itRNforce?itRNforce:rCfg.rDSession.itRN,rCfg.rDSession.itSP,itDC))
    goto end;

  /* Reference decoding */
  if(rCfg.rRej.eTyp==RR_phn)
    IF_NOK(ret=decode(rCfg.rDSession.itGP,rCfg.rDSession.itRNr,rCfg.rDSession.itSPr,itDCr))
      goto end;

  /* Confidence computation */
  confidence(itDC,itDCr,sLab);

  /* FVR generation */
  itFvr=fvrgen(itDC);
  
  /* External post-processing */
  if(rCfg.sPostProc[0])
    postprocess(itDC,itDCr,itFvr);

end:
  IDESTROYFST(itDCr);
  IDESTROYFST(itDC);
  if(itFvr) IDESTROYFST(itFvr);
  if(itRNforce){
    CFstsearch_Load(rCfg.rDSession.itSP,rCfg.rDSession.itRN,0);
    IDESTROYFST(itRNforce);
  }
  
  return ret;
}

INT32 delta(FLOAT32 *lpFrames, INT64 *nFDim, INT64 nXFrames, INT64 nDim)
{
  INT64    nVDim;
  INT64    nADim;

  if(*nFDim!=rCfg.rDFea.nPfaDim) return NOT_EXEC;

  /* Get number of nVDim and nADim */                                          /* ------------------------------------ */
  dlm_fba_deltafba(NULL,rCfg.rDFea.lpDeltaT,rCfg.rDFea.lpDeltaW,nXFrames,TRUE,rCfg.rDFea.nDeltaWL,*nFDim,&nVDim,&nADim,0);

  if(lpFrames)
  /* Compute dynamic features */                                                /* ------------------------------------ */
    dlm_fba_deltafba(lpFrames,rCfg.rDFea.lpDeltaT,rCfg.rDFea.lpDeltaW,nXFrames,TRUE,rCfg.rDFea.nDeltaWL,*nFDim,&nVDim,&nADim,nDim);

  *nFDim += nVDim+nADim;

  return O_K; /* Return error code */
}

/*
 * Do secondary feature analysis.
 *
 * @param lpFeaInfo see test_feature_info.txt
 * @param lpFrames  MEL coefficients buffer (input) - size: nXFrames*nDim
 * @param lpFea     Feature buffer (output) - size: nXFrames*nMsf
 * @param nFDim     Pointer to input dimensionality (default: 30) - is changed to delta vector dimensionality (default: 60)
 * @param nXFrames  Number of frames in input buffer (if nFrame>=0: minimum: rCfg.rDFea.nDeltaWL*4+1)
 * @param nFrame    process only frame with index nFrame in lpFrames (lpFrames is assumed to be a ring buffer)
 * @param nMsf      Output dimensionality (default: 24)
 * @param nDim      Input vector buffer size (default: 512)
 */
INT32 sfa(FLOAT32 *lpFrames, FLOAT32 *lpFea, INT64 *nFDim, INT64 nXFrames, INT64 nFrame, INT64 nMsf, INT64 nDim)
{
  INT64 nC;
  INT64 nR;

  delta(lpFrames,nFDim,nXFrames,nDim);

  memset(lpFea,0,sizeof(FLOAT32)*nMsf);

  for(nC=0;nC<nMsf;nC++) for(nR=0;nR<*nFDim;nR++)
    lpFea[nC]+=(lpFrames[nFrame*nDim+nR]+rCfg.rDFea.lpX[nR])*rCfg.rDFea.lpW[nR*(*nFDim)+nC];

  return O_K;
}

INT32 doframing(FLOAT32 *lpSignal,INT64 nXSamples,FLOAT32 **lpFrames,INT64 *nXFrames,FLOAT32 *lpWindow,struct dlm_fba_doframing_param *lpParam)
{
  if(!*nXFrames){
  *nXFrames = nXSamples/lpParam->nCrate;
  if(nXSamples%lpParam->nCrate!=0.0) *nXFrames += 1;
  }  
  if(!*lpFrames) *lpFrames=(FLOAT32*)dlp_calloc(lpParam->nLen*(*nXFrames),sizeof(FLOAT32));

  IF_NOK(dlm_fba_doframing(lpSignal, nXSamples, NULL, 0, lpWindow, NULL, NULL, NULL, *lpFrames, *nXFrames, NULL, lpParam)) return NOT_EXEC;

  return O_K;
}

INT32 pfa(FLOAT32 *lpSignal, INT64 nXSamples, FLOAT32 **lpFrames, INT64 *nXFrames)
{
  INT64 nFrame;

  IF_NOK(doframing(lpSignal,nXSamples,lpFrames,nXFrames,rCfg.rPfa.lpWindow,&rCfg.rPfa.lpFba)) return NOT_EXEC;

  for(nFrame=0;nFrame<*nXFrames;nFrame++)
    IF_NOK(dlm_mf_analyze(
          &rCfg.rPfa.lpCnvc,
          (*lpFrames)+nFrame*rCfg.rPfa.lpFba.nLen,
          (*lpFrames)+nFrame*rCfg.rPfa.lpFba.nLen,
          rCfg.rPfa.lpFba.nLen,
          rCfg.rDFea.nPfaDim,
          rCfg.rPfa.lpFba.nMinLog
      )) return NOT_EXEC;

  return O_K;
}

BOOL vad_pfa_gmm(FLOAT32 *lpFPfa)
{
  INT32 nDim = rCfg.rDFea.nPfaDim;
  INT32 nMsf = rCfg.rDVAD.nSfaDim;
  INT32 nC,nR;
/*  FLOAT32 nSumV=T_FLOAT_MAX;
  FLOAT32 nSumU=T_FLOAT_MAX;*/
  FLOAT32 *lpFea;
  lpFea=(FLOAT32*)CData_XAddr(rTmp.idVFea,0,0);
  if(nDim>rCfg.rDVAD.nPfaDim) nDim=rCfg.rDVAD.nPfaDim;
  memset(lpFea,0,sizeof(FLOAT32)*nMsf);
  for(nC=0;nC<nMsf;nC++) for(nR=0;nR<nDim;nR++)
    lpFea[nC]+=(lpFPfa[nR]+rCfg.rDVAD.lpX[nR])*rCfg.rDVAD.lpW[nR*(rCfg.rDVAD.nPfaDim)+nC];
  rCfg.rDVAD.itGM->m_bNeglog = TRUE;
 /* for(nC=0;nC<nMsf;nC++) printf("%.3f ",lpFea[nC]); printf("\n");*/
  CGmm_Density(rCfg.rDVAD.itGM,rTmp.idVFea,NULL,rTmp.idVNld);
  /*{ static FILE *fd=NULL; if(!fd) fd=fopen("recognizer_vad.bin","w"); for(nC=0;nC<CData_GetNComps(rTmp.idVNld);nC++){ FLOAT32 f=CData_Dfetch(rTmp.idVNld,0,nC); fwrite(&f,1,sizeof(FLOAT32),fd); } }*/
/*  for(nC=0;nC<CData_GetNComps(rTmp.idVNld);nC++)
    if(nC>=2) nSumV=dlp_scalopF(nSumV,CData_Dfetch(rTmp.idVNld,0,nC),OP_LSADD); else nSumU=dlp_scalopF(nSumU,CData_Dfetch(rTmp.idVNld,0,nC),OP_LSADD);
  nSumV-=dlp_scalopF(nSumV,nSumU,OP_LSADD);
  if(nDebug>0){ for(nC=0;nC<CData_GetNComps(rTmp.idVNld);nC++) printf("%5.1f ",CData_Dfetch(rTmp.idVNld,0,nC)); printf(" => %.2f ",exp(-nSumV)); }
  return nSumV<=nGmmThr;*/
  routput(O_vad,1,"%5.1f %5.1f ",CData_Dfetch(rTmp.idVNld,0,0),CData_Dfetch(rTmp.idVNld,0,CData_GetNComps(rTmp.idVNld)-1));
  if(rCfg.eOut==O_dbg){ static int c=0; if(!(c=(c+1)%50)) routput(O_dbg,1,"vad: %5.1f %5.1f\n",CData_Dfetch(rTmp.idVNld,0,0),CData_Dfetch(rTmp.idVNld,0,5)); }
  return CData_Dfetch(rTmp.idVNld,0,CData_GetNComps(rTmp.idVNld)-1)*3<=CData_Dfetch(rTmp.idVNld,0,0);
}

BOOL vad_pfa(FLOAT32 *lpFPfa)
{
  switch(rCfg.rVAD.nVadType){
  case VAD_NONE: return TRUE;
  case VAD_ENG:
    return dlm_vad_single_pfaengF(lpFPfa,rCfg.rDFea.nPfaDim,rCfg.rVAD.nPfaThr);
  case VAD_GMM:
    if(!rCfg.rDVAD.itGM) return TRUE;
    return vad_pfa_gmm(lpFPfa);
  }
  return TRUE;
}

#ifdef __USE_PORTAUDIO
#define PABUF_SIZE    160
#define PABUF_NUM     50
#define PABUF_NXT(i)  ((i)+1<PABUF_NUM ? (i)+1 : 0)
struct paBuf { /* Audio ring buffer */
  FLOAT32 lpDat[PABUF_SIZE*PABUF_NUM];
  INT32 nWPos;
  INT32 nRPos;
  INT32 nSkip;
  INT32 nCrate;
  INT32 nWlen;
  FLOAT32 nVol;
  FLOAT32 nPeak;
};

int paCallback( const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData )
{
  struct paBuf* lpBuf=(struct paBuf*)userData;
  INT32 i;
  FLOAT32 max=0.;

  if(framesPerBuffer!=PABUF_SIZE) rerror("pa_input_size_missmatch (%i)\n",framesPerBuffer);

  INT32 nWnxt=PABUF_NXT(lpBuf->nWPos);
  if(nWnxt==lpBuf->nRPos){ lpBuf->nSkip++; return 0; }

  for(i=0;i<PABUF_SIZE;i++){
    FLOAT32 v=((FLOAT32*)inputBuffer)[i]*lpBuf->nVol;
    lpBuf->lpDat[lpBuf->nWPos*PABUF_SIZE+i]=v;
    v=fabs(v);
    if(v>max) max=v;
  }

  lpBuf->nWPos=nWnxt;
  if(max>lpBuf->nPeak) lpBuf->nPeak=max;

  return 0;
}
#endif

void audio_devlist()
{
#ifndef __USE_PORTAUDIO
  rerror("enable __USE_PORTAUDIO for online recognition\n");
#else
  INT32 i;
  if(Pa_Initialize()!=paNoError) return;
  printf("Devices: %i\n",Pa_GetDeviceCount());
  for(i=0;i<Pa_GetDeviceCount();i++){
    const PaDeviceInfo* deviceInfo=Pa_GetDeviceInfo(i);
     if(deviceInfo->maxInputChannels>0)
       printf("Device %2i: In-Ch: %3i (In-Time: %8.4f->%8.4f) Out-Ch: %3i (Out-Time: %8.4f->%8.4f) SampRate: %g Name: %s\n",i,
        deviceInfo->maxInputChannels,deviceInfo->defaultLowInputLatency,deviceInfo->defaultHighInputLatency,
        deviceInfo->maxOutputChannels,deviceInfo->defaultLowOutputLatency,deviceInfo->defaultHighOutputLatency,
        deviceInfo->defaultSampleRate,deviceInfo->name);
  }
  if(Pa_Terminate()!=paNoError) return;
#endif
}

INT16 online(struct recosig *lpSig)
{
  /* Local variables */
  INT16       ret = O_K;
  /* Frame buffers */
  FLOAT32*     lpFPfa;       /* Buffer for PFA */
  FLOAT32*     lpFSfa;       /* Buffer for SFA */
  /* Vad variables */
  struct dlm_vad_param* lpVadParam = &rCfg.rVAD.lpBas;
  struct dlm_vad_state  lpVadState;
  BOOL   bVadPfa;             /* VAD decision for PFA frame */
  INT16  nVadSfa = -1;        /* VAD decision for SFA frame (-1: offline, 0: silence, 1; speech) */
  INT16  nVadSfaLast = -1;    /* Most recent VAD decision for SFA frame */
  /* Buffer sizes */
  INT64   nXDelta;            /* Frame dimension after delta calculation */
  INT64   nXSfa;              /* Size of buffer for sfa */
  INT64   nMsf = rCfg.rDFea.nSfaDim;
  /* Buffer iterators */
  INT64   nFSfaW;             /* Current writing position in lpFSfa */
  INT64   nFSfaR;             /* Current reading position in lpFSfa */
  INT64   nFea = 0;           /* Current writing position in lpFea */
#define SIGH_LEN        50
  INT32   nSigMax = 0;        /* Signal maximum */
  INT32   *lpSigMaxH;         /* Signal maximum buffer */
  INT32   nSigMaxPos=0;       /* Position in the buffer */
  INT32   nSigMaxI=0;         /* Number of samples stored in the current position */
  const INT32 nPfaDim = rCfg.rDFea.nPfaDim;
  const INT32 nCrate  = rCfg.rPfa.lpFba.nCrate;

  /* Signal fetch */
#ifdef __USE_PORTAUDIO
  PaStream*    stream;
  struct paBuf lpBuf;
  FLOAT32 lpWindow[400];
#endif
  INT64        nSigPos = 0;
  FLOAT32     *lpDat=NULL;
  INT32        nWlen=0;
  FLOAT32     *lpColSigBuf=NULL;
  INT32        nColSigBufPos=0;
/*  THREADHANDLE lpCmdThread;*/

  nFrame=nLastActive=0;

  if(!lpSig){ /* ----- online ----- */

#ifndef __USE_PORTAUDIO
    rerror("enable __USE_PORTAUDIO for online recognition\n");
    return NOT_EXEC;
#else

    /* Init portaudio */
    if(Pa_Initialize()!=paNoError) return NOT_EXEC;

    /* Init signal fetch buffer */
    memset(lpBuf.lpDat,0,sizeof(lpBuf.lpDat[0])*PABUF_SIZE*PABUF_NUM);
    memset(lpWindow,0,sizeof(lpWindow[0])*400);
    lpBuf.nWPos=0;
    lpBuf.nRPos=0;
    lpBuf.nSkip=0;
    lpBuf.nCrate=nCrate;
    lpBuf.nWlen=rCfg.rPfa.lpFba.nWlen;
    lpBuf.nPeak=0.;
    lpBuf.nVol=1.;

    /* Init portaudio device */
    if(rCfg.nAudioDev<0){
      if(Pa_OpenDefaultStream(&stream,1,0,paFloat32,rCfg.rPfa.nSrate,nCrate,paCallback,&lpBuf)!=paNoError) return NOT_EXEC;
    }else{
      PaStreamParameters paIn;
      memset(&paIn,0,sizeof(PaStreamParameters));
      paIn.channelCount=1;
      paIn.device=rCfg.nAudioDev;
      paIn.sampleFormat=paFloat32;
      if(Pa_OpenStream(&stream,&paIn,NULL,rCfg.rPfa.nSrate,nCrate,paNoFlag,paCallback,&lpBuf)) return NOT_EXEC;
    }
    if(Pa_StartStream(stream)!=paNoError) return NOT_EXEC;
#endif

    if(rCfg.eIn==I_cmd) /*lpCmdThread=*/dlp_create_thread(cmdthread,NULL);
  }

  /* Get feature dimension after delta calculation */
  nXDelta=nPfaDim;
  delta(NULL,&nXDelta,0,nXDelta);
  /* Calc SFA buffer size from delta length and VAD parameters */
  dlm_vad_initstate(&lpVadState,lpVadParam,rCfg.rDFea.nDeltaWL*2);
  nXSfa = rCfg.rDFea.nDeltaWL*2 + 1 + lpVadState.nDelay;
  /* Alloc memory for frame buffers */
  lpFPfa=(FLOAT32*)dlp_malloc(rCfg.rPfa.lpFba.nLen*sizeof(FLOAT32));
  lpFSfa=(FLOAT32*)dlp_calloc(nXDelta*nXSfa,sizeof(FLOAT32));
  CData_Array(rTmp.idFea,T_FLOAT,nMsf,rCfg.rSearch.bIter?1:lpVadParam->nMaxSp);
  /* Init SFA buffer positions */
  nFSfaR = 0;
  nFSfaW = nFSfaR + lpVadState.nDelay;
  /* Init signal maximum buffer */
  lpSigMaxH = (INT32*)dlp_calloc(lpVadState.nDelay,sizeof(INT32));

  if(rCfg.sPostProc[0]){
    lpColSigBuf=dlp_calloc(lpVadState.nDelay+1,sizeof(FLOAT32)*nCrate);
    rTmp.lpColSig=dlp_malloc(rCfg.rVAD.nMaxSp*nCrate*sizeof(FLOAT32));
    rTmp.nColSigLen=0;
  }

  if(!lpSig) routput(O_sta,1,"online recognizer initialized\n");
  /* Loop until there is a signal */
  while((!lpSig || nSigPos<lpSig->nLen+lpVadState.nDelay*nCrate) && !rCfg.bExit)
  {
    if
    (
      nLastActive &&
      nVadSfa<=0 &&
      rCfg.nFstSleep &&
      nFrame-nLastActive>rCfg.nFstSleep*rCfg.rPfa.nSrate/nCrate
    )
    {
      dlg_upd("__SLEEP__");
      nLastActive=0;
    }

    /* Check for new signal fetched */
#ifdef __USE_PORTAUDIO
    if(lpSig || lpBuf.nWPos!=lpBuf.nRPos)
#endif
    {
      INT32 nXProcess=1;
      INT32  nI;
      if(rCfg.bMeasureTime) measuretime(&tms_c_star);

      /* Get new Frame */
      if(lpSig){
		if(nSigPos<lpSig->nLen){
	        lpDat=lpSig->lpSamples+nSigPos;
    	    nWlen=MIN(lpSig->nLen-nSigPos,rCfg.rPfa.lpFba.nWlen);
		}
        nSigPos+=nCrate;
      }else{
#ifdef __USE_PORTAUDIO
        memmove(lpWindow,lpWindow+160,(400-160)*sizeof(FLOAT32));
        memcpy(lpWindow+400-160,lpBuf.lpDat+lpBuf.nRPos*PABUF_SIZE,160*sizeof(FLOAT32));
        lpBuf.nRPos=PABUF_NXT(lpBuf.nRPos);
        lpDat=lpWindow;
        nWlen=lpBuf.nWlen;
#endif
      }
      rTmp.nDuration+=nCrate/(FLOAT32)rCfg.nSigSampleRate;
      for(nI=0;nI<nCrate;nI++){
        INT32 nD=(INT32)(fabs(lpDat[nWlen-nI-1])*32768.);
        if(nD>lpSigMaxH[nSigMaxPos]) lpSigMaxH[nSigMaxPos]=nD;
        if(++nSigMaxI>=160){
          if(++nSigMaxPos>=lpVadState.nDelay) nSigMaxPos=0;
          if(lpSigMaxH[nSigMaxPos]>nSigMax) nSigMax=lpSigMaxH[nSigMaxPos];
          lpSigMaxH[nSigMaxPos]=0;
          nSigMaxI=0;
        }
      }

      /* GUI output */
      if(rCfg.eOut==O_gui)
      {
        FLOAT32 nPeak = 0.f;
        UINT64  nTime = dlp_time()%1000;
        for (nI=0;nI<nWlen;nI++)
          if (nPeak<fabs(lpDat[nI]))
            nPeak = fabs(lpDat[nI]);
        if (nPeak>0.f) nPeak = 20*log10(nPeak); else nPeak = -96;
        routput(O_gui,1,"frm: %i lvl: %0.1f tim: %03i\n",nFrame,nPeak,nTime);
      }

      /* Do primary feature analysis and vad decision for that frame */
      if(lpLog.fdRaw) fwrite(lpDat+nWlen-nCrate,sizeof(FLOAT32),nCrate,lpLog.fdRaw);
      if(rCfg.sPostProc[0]){
        memcpy(lpColSigBuf+nCrate*nColSigBufPos,lpDat,sizeof(FLOAT32)*nCrate);
        if(++nColSigBufPos>=lpVadState.nDelay+1) nColSigBufPos=0;
#ifdef __USE_PORTAUDIO
        if(!lpSig) lpBuf.nPeak=0.;
#endif
      }
      IF_NOK(pfa(lpDat,nWlen,&lpFPfa,&nXProcess)) break;
      bVadPfa=vad_pfa(lpFPfa);

      /* Do final Vad decision for last frame in pre buffer */
      if (!rCfg.rVAD.bOffline){ /* VAD is on-line */
        nVadSfa=dlm_vad_process(bVadPfa,&lpVadState)?1:0;
		if(nFrame-lpVadState.nDelay<0) nVadSfa=0;
	  }else /* VAD is off-line */
        nVadSfa=-1;
      if(rCfg.bVADForce || rCfg.bFSTForce){
        UINT8 nVal = (nFrame-lpVadState.nDelay)<0 || (nFrame-lpVadState.nDelay)>=rTmp.nNVadForce ? 0 : rTmp.lpVadForce[nFrame-lpVadState.nDelay];
        if(rCfg.bVADForce) nVadSfa = nVal;
        if(rCfg.bFSTForce && nVal) searchload(nVal);
      }
      if(rCfg.rSearch.bPermanent) nVadSfa=1;

      /* VAD Debug output */
      if(nVadSfa!=nVadSfaLast)
      {
        const char* s = "on";
        if      (nVadSfa==0) s = "off";
        else if (nVadSfa <0) s = "offline";
        routput(O_dbg,1,"vad %s at frame %i\n",s,nFrame-lpVadState.nDelay);
        nVadSfaLast=nVadSfa;
        if(rCfg.rSearch.bIter && nVadSfa>0){
          CData_Reset(BASEINST(rTmp.idNld),TRUE);
          CFstsearch_Restart(rCfg.rDSession.itSP);
          if(rCfg.rRej.eTyp==RR_phn) CFstsearch_Restart(rCfg.rDSession.itSPr);
        }
      }
      routput(O_vad,0,"pF%4i: V:%i Sw:%3i => VS:%i ViS:%2i VP:%2i VC:%2i => sF%4i V:%i ",
        nFrame,bVadPfa,nFSfaW,
        lpVadState.nState,lpVadState.nInState,lpVadState.nPre,lpVadState.nChange,
        nFrame-lpVadState.nDelay,nVadSfa);
      if (nVadSfa>0) routput(O_vad,0,"Sr:%3i",nFSfaR);
      routput(O_vad,0," max: %5i",nSigMax);
      routput(O_vad,0," LastRes: %s\n",rTmp.rRes.sLastRes);

      /* Write VAD labels to label files */
      if(lpLog.fdLab1 && bVadPfa!=lpLog.bLab1){
        lpLog.bLab1=bVadPfa;
        fprintf(lpLog.fdLab1,"%.f %c\n",(nFrame+.5)*nCrate-nWlen*.5,bVadPfa?'V':'U');
        fflush(lpLog.fdLab1);
      }
      if(lpLog.fdLab2 && (nVadSfa>0)!=lpLog.bLab2){
        lpLog.bLab2=(nVadSfa>0);
        fprintf(lpLog.fdLab2,"%.f %c\n",(nFrame-lpVadState.nDelay+.5)*nCrate-nWlen*.5,(nVadSfa>0)?'V':'U');
        fflush(lpLog.fdLab2);
      }

      /* Copy frame in signal collect buffer */
      if(rCfg.sPostProc[0] && nVadSfa>0 && rTmp.nColSigLen+nCrate<=lpVadParam->nMaxSp*nCrate){
          memcpy(rTmp.lpColSig+rTmp.nColSigLen,
            lpColSigBuf+nCrate*nColSigBufPos,
            sizeof(FLOAT32)*nCrate);
          rTmp.nColSigLen+=nCrate;
      }

      /* Copy frame to buffer for SFA (lpFPfa -> lpFSfa[nFSfaW++]) */
	  if(!nFrame){
		  INT64 nI;
		  for(nI=0;nI<nXSfa;nI++)
      		memcpy(lpFSfa+nI*nXDelta,     lpFPfa, sizeof(FLOAT32)*nPfaDim);
	  }else memcpy(lpFSfa+nFSfaW*nXDelta, lpFPfa, sizeof(FLOAT32)*nPfaDim);
      if(++nFSfaW==nXSfa) nFSfaW=0;

      if(nVadSfa>0){
        INT64 nFDim = nPfaDim;
        /* Do secondary feature analysis */
        sfa(lpFSfa, (FLOAT32*)CData_XAddr(rTmp.idFea,rCfg.rSearch.bIter?0:nFea,0), &nFDim, nXSfa, nFSfaR, nMsf, nXDelta);
        nFea++;
      }      

      /* Update SFA buffer reading position */
      if(++nFSfaR==nXSfa) nFSfaR=0;

      /* Update global frame number */
      nFrame++;

      if(rCfg.bMeasureTime){
        measuretime(&tms_c_anal);
        updatetimes(&tms_anal,&tms_c_star,&tms_c_anal);
        updatetimes(&tms_all ,&tms_c_star,&tms_c_anal);
      }

      if(nVadSfa>0 && rCfg.rSearch.bIter){
        rCfg.rDSession.itGM->m_bNeglog = TRUE;
        IF_NOK(CGmm_Density(rCfg.rDSession.itGM,rTmp.idFea,NULL,rTmp.idFea)) break;
        rCfg.rDSession.itSP->m_bFinal=FALSE;
        if(rCfg.rSearch.bPermanent) rCfg.rDSession.itSP->m_bStart=TRUE;
        CFstsearch_Isearch(rCfg.rDSession.itSP,rTmp.idFea);
        if(rCfg.rRej.eTyp==RR_phn){
          rCfg.rDSession.itSPr->m_bFinal=FALSE;
          if(rCfg.rSearch.bPermanent) rCfg.rDSession.itSPr->m_bStart=TRUE;
          CFstsearch_Isearch(rCfg.rDSession.itSPr,rTmp.idFea);
        }
        if(rCfg.sPostProc[0]) CData_Cat(rTmp.idNld,rTmp.idFea);
        CData_Array(rTmp.idFea,T_FLOAT,nMsf,1);
        if(nFea%10==0){
          CFst *itDC;
          INT32 nT,nCTos,nCTis;
          ICREATEEX(CFst, itDC, "itDC", NULL);
          rCfg.rDSession.itSP->m_bFinal=TRUE;
          CFstsearch_Backtrack(rCfg.rDSession.itSP,itDC);
          nCTos=CData_FindComp(AS(CData,itDC->td),"~TOS");
          nCTis=CData_FindComp(AS(CData,itDC->td),"~TIS");
          if(nCTos>=0 && nCTis>=0 && !CData_IsEmpty(AS(CData,itDC->os))){
            INT32 nLen=0;
            routput(O_sta,1,"liveres: ");
            for(nT=0;nT<CData_GetNRecs(AS(CData,itDC->td));nT++){
              INT32 nTos=CData_Dfetch(AS(CData,itDC->td),nT,nCTos);
              INT32 nTis=CData_Dfetch(AS(CData,itDC->td),nT,nCTis);
              if(nTos>=0) routput(O_sta,0,"%s",CData_Sfetch(AS(CData,itDC->os),nTos,0));
              if(nTis>=0) nLen++;
            }
            if(rCfg.rSearch.bPermanent){
              routput(O_sta,0," [range %i-%i (%i) - max %i]\n",nFea-nLen,nFea,nLen,nSigMax); nSigMax=0;
            }else routput(O_sta,0,"\n");
          }
          IDESTROYFST(itDC);
        }
      }
    }
    /* Wait and go on */
#if !defined __TMS && defined __USE_PORTAUDIO
    else dlp_sleep(5);
#endif

    /* End of feature vector collection reached ? */
    if(!rCfg.rSearch.bPermanent && nFea && ((nVadSfa<=0) || nFea==lpVadParam->nMaxSp || (lpSig && nSigPos>=lpSig->nLen+lpVadState.nDelay*nCrate))){
      if(rCfg.bMeasureTime) measuretime(&tms_c_star);
      routput(O_sta,1,"vad collected %3i frames from %3i (sigmax: %5i)\n",nFea,nFrame-1-lpVadState.nDelay-nFea,nSigMax);
      if(nFea>=rCfg.rVAD.nMinSp && nSigMax>rCfg.rVAD.nSigMin){
        nLastActive=nFrame;
#ifdef __USE_PORTAUDIO
        if(!lpSig && lpBuf.nSkip){ routput(O_sta,1,"buf skipped: %i\n",lpBuf.nSkip); lpBuf.nSkip=0; }
#endif
        routput(O_sta,1,"rec start (fst: %i)\n",rTmp.nFstSel);
        if(rCfg.bSkipNld && lpSig && lpSig->nNldNum){
          /* Use pre-calculated neglog densities */
          INT64 nOff=nFrame-lpVadState.nDelay-nFea;
          if(nOff<0) nOff=0;
          if(nOff+nFea>lpSig->nNldNum) nFea=lpSig->nNldNum-nOff;
          CData_Array(rTmp.idNld,T_FLOAT,lpSig->nNldDim,nFea);
          memcpy(CData_XAddr(rTmp.idNld,0,0),lpSig->lpNld+lpSig->nNldDim*nOff,sizeof(FLOAT32)*lpSig->nNldDim*nFea);
        }else if(!rCfg.rSearch.bIter){
          /* Calculate neglog densities */
          CData_DeleteRecs(rTmp.idFea,nFea,CData_GetNRecs(rTmp.idFea)-nFea);
#ifdef HTKFEA
  char cmd[256];
  snprintf(cmd,256,"python htkfeagen.py %i %s",CData_GetNRecs(rTmp.idFea),rTmp.sSigFname);
  rTmp.iFile->m_bExecute=TRUE;
  CDlpFile_Export(rTmp.iFile,cmd,"ascii",BASEINST(rTmp.idFea));
  rTmp.iFile->m_bExecute=FALSE;
#endif
           rCfg.rDSession.itGM->m_bNeglog = TRUE;
          IF_NOK(CGmm_Density(rCfg.rDSession.itGM,rTmp.idFea,NULL,rTmp.idNld)) break;
        }
        if(rCfg.bMeasureTime) measuretime(&tms_c_dens);
        IF_NOK(recognize(lpSig ? lpSig->lpsLab : NULL)) {
          routput(O_sta,1,"rec end (failed)\n");
          break;
        }
        if(rCfg.bMeasureTime){
          measuretime(&tms_c_reco);
          updatetimes(&tms_dens,&tms_c_star,&tms_c_dens);
          updatetimes(&tms_reco,&tms_c_dens,&tms_c_reco);
          updatetimes(&tms_all ,&tms_c_star,&tms_c_reco);
        }
        routput(O_sta,1,"rec end (ok)\n");
        if(!lpSig && !dlp_strcmp(rTmp.rRes.sLastRes,"__NAVI__")) break;
        nLastActive=nFrame;
      }else{
        routput(O_sta,1,"rec skip for %s\n",nFea<rCfg.rVAD.nMinSp ? "time length" : "signal maximum");
      }
      if(!rCfg.rSearch.bIter) CData_Array(rTmp.idFea,T_FLOAT,nMsf,lpVadParam->nMaxSp);
      nFea=0;
    }

    if(nVadSfa<=0) nSigMax=0;
    if(rCfg.sPostProc[0] && nVadSfa<=0) rTmp.nColSigLen=0;

    if(rCfg.eIn==I_cmd) cmdqueueget();
  }

  /*if(lpCmdThread) dlp_terminate_thread(lpCmdThread,0);*/

  /* Free frame buffers */
  dlp_free(lpSigMaxH);
  dlp_free(lpFPfa);
  dlp_free(lpFSfa);
  if(rCfg.sPostProc[0]){
    dlp_free(lpColSigBuf);
    dlp_free(rTmp.lpColSig);
  }


#ifdef __USE_PORTAUDIO
  if(!lpSig){
    /* Free portaudio */
    if(Pa_StopStream(stream)!=paNoError) return NOT_EXEC;
    if(Pa_CloseStream(stream)!=paNoError) return NOT_EXEC;
    if(Pa_Terminate()!=paNoError) return NOT_EXEC;
  }
#endif

  /* All done */
  return ret;
}

void do_feastdin()
{
#define IBUF_SI 4096
#define LAB_SI  256
#define VAL_SI  256
  char lpIBuf[IBUF_SI];
  char lpLab[LAB_SI];
  INT32 nR=0;
  INT32 nMsf=rCfg.rDFea.nSfaDim;
  CData_Reset(BASEINST(rTmp.idFea),TRUE);
  CData_AddNcomps(rTmp.idFea,T_FLOAT,nMsf);
  lpLab[0]=lpLab[LAB_SI-1]='\0';
  printf("\n");
  while(!feof(stdin) && fgets(lpIBuf,IBUF_SI,stdin)){
    INT32 nI=0;
    INT32 nC,nC2,nNC;
    char *lpBuf;
    FLOAT32 lpVal[VAL_SI];
    /* skip spaces */
    while(nI<IBUF_SI && (lpIBuf[nI]==' ' || lpIBuf[nI]=='\t')) nI++;
    if(nI==IBUF_SI) continue;
    /* empty line -> do recognition */
    if(lpIBuf[nI]=='\0' || lpIBuf[nI]=='\n' || lpIBuf[nI]=='\r'){
      routput(O_sta,1,"collected %i frames (label: %s)\n",nR,lpLab);
      rCfg.rDSession.itGM->m_bNeglog = TRUE;
      IF_NOK(CGmm_Density(rCfg.rDSession.itGM,rTmp.idFea,NULL,rTmp.idNld)) continue;
      IF_NOK(recognize(lpLab[0]?lpLab:NULL)) continue;
      rTmp.nDuration+=nR*0.01;
      CData_Reallocate(rTmp.idFea,0);
      nR=0;
      lpLab[0]='\0';
      continue;
    }
    /* label line */
    if(lpIBuf[nI]=='l'){
      nI++;
      while(nI<IBUF_SI && (lpIBuf[nI]==' ' || lpIBuf[nI]=='\t')) nI++;
      snprintf(lpLab,MIN(LAB_SI,IBUF_SI-nI-1),lpIBuf+nI);
      for(nI=strlen(lpLab)-1;nI>=0 && (lpLab[nI]=='\n' || lpLab[nI]=='\r' || lpLab[nI]==' ' || lpLab[nI]=='\t');nI--)
        lpLab[nI]='\0';
      continue;
    }
    /* feature vector line */
    CData_Reallocate(rTmp.idFea,nR+1);
    lpBuf=lpIBuf+nI;
    for(nC=0;lpBuf && nC<VAL_SI;nC++){
      char *lpBufE;
      lpVal[nC]=strtof(lpBuf,&lpBufE);
      if(lpBufE==lpBuf) break;
      lpBuf=lpBufE;
    }
    nNC=nC;
    memset(CData_XAddr(rTmp.idFea,nR,0),0,CData_GetRecLen(rTmp.idFea));
    for(nC=0;nC<nMsf;nC++) for(nC2=0;nC2<nNC;nC2++)
      *(FLOAT32*)CData_XAddr(rTmp.idFea,nR,nC)+=(lpVal[nC2]+rCfg.rDFea.lpX[nC2])*rCfg.rDFea.lpW[nC2*rCfg.rDFea.nPfaDim+nC];
    nR++;
  }
}

void processfile(struct recofile lpF){
  struct recosig lpSig;
  CData *idSig=rTmp.idSig;
  INT32 nI;
  INT32 nSR;
  INT32 nC;
  FLOAT32 nNorm;
  const char *errstr="";

  rTmp.sSigFname=lpF.lpsFName;
  IF_NOK(CDlpFile_LibsndfileImport(rTmp.iFile, lpF.lpsFName, BASEINST(idSig), "wav")) goto err;
  nSR=1000./idSig->m_lpTable->m_fsr+.5;
  nC=CData_GetNComps(idSig);
  if(nSR!=rCfg.nSigSampleRate){ errstr="sample rate missmatch"; goto err; }
  if(nC>1){
    if(rCfg.nSigChannel<0 || rCfg.nSigChannel>=nC){ errstr="not exisiting channel selected"; goto err; }
    if(rCfg.nSigChannel)      CData_DeleteComps(idSig,0,rCfg.nSigChannel);
    if(rCfg.nSigChannel<nC-1) CData_DeleteComps(idSig,rCfg.nSigChannel+1,nC-1-rCfg.nSigChannel);
  }
  switch(CData_GetCompType(idSig,0)){
  case T_UCHAR:  nNorm=T_UCHAR_MAX; break;
  case T_CHAR:   nNorm=T_CHAR_MAX;  break;
  case T_USHORT: nNorm=T_USHORT_MAX;break;
  case T_SHORT:  nNorm=T_SHORT_MAX; break;
  case T_UINT:   nNorm=T_UINT_MAX;  break;
  case T_INT:    nNorm=T_INT_MAX;   break;
  case T_ULONG:  nNorm=T_ULONG_MAX; break;
  case T_LONG:   nNorm=T_LONG_MAX;  break;
  default:       nNorm=1.;          break;
  }
  CData_Tconvert(idSig, idSig, T_FLOAT);
  lpSig.nLen=CData_GetNRecs(idSig);
  lpSig.lpSamples=(FLOAT32*)CData_XAddr(idSig,0,0);
  lpSig.lpsLab=lpF.lpsLab[0] ? lpF.lpsLab : NULL;
  if(nNorm!=1.) for(nI=0;nI<lpSig.nLen;nI++) lpSig.lpSamples[nI]/=nNorm;
  routput(O_sta,1,"processing %s %.1fs\n", strrchr(lpF.lpsFName,'/') ? strrchr(lpF.lpsFName,'/')+1 : lpF.lpsFName,(FLOAT32)lpSig.nLen/(FLOAT32)rCfg.nSigSampleRate);
  if(rCfg.bVADForce || rCfg.bFSTForce){
    char lpsFN[STR_LEN];
    FILE *FD;
    snprintf(lpsFN,STR_LEN-1,"%s.vadforce",lpF.lpsFName);
    if(!(FD=fopen(lpsFN,"r"))){ errstr="open .vadforce failed"; goto err; }
    fseek(FD,0,SEEK_END);
    rTmp.nNVadForce=ftell(FD);
    fseek(FD,0,SEEK_SET);
    rTmp.lpVadForce=(UINT8*)dlp_realloc(rTmp.lpVadForce,
        rTmp.nNVadForce,sizeof(UINT8));
    fread(rTmp.lpVadForce,sizeof(UINT8),rTmp.nNVadForce,FD);
    fclose(FD);
  }
  online(&lpSig);
  rTmp.sSigFname="";
  return;
err:
  rTmp.sSigFname="";
  routput(O_sta,1,"file read error (%s)\n",errstr);
}

int main(int argc, char** argv)                                               /* Main function */
#ifdef __TMS
{ return 0; }
char* main_argv[]={"","-b","-t","-debug","0","recognizer_data.bin",NULL};
INT32 main_proc(INT32 argc, char** argv)                                               /* Main function */
#endif
{

#ifdef __TMS
  for(argc=0;main_argv[argc];) argc++;
  argv=main_argv;
#endif

#ifdef _DEBUG                                                                  /* Initialize */
/*  dlp_set_kernel_debug(DM_KERNEL); */
  dlp_xalloc_init(XA_DLP_MEMLEAKS|XA_HEAP_INTEGRITY|XA_HEAP_MEMLEAKS);
#else
  dlp_xalloc_init(XA_DLP_MEMLEAKS);
#endif

  REGISTER_CLASS(CDlpObject);
  REGISTER_CLASS(CData);
  REGISTER_CLASS(CFst);
#ifndef __TMS
  REGISTER_CLASS(CDlpFile);
#endif
  REGISTER_CLASS(CGmm);
  REGISTER_CLASS(CVmap);

  dlp_set_pipe_mode(TRUE);
  if(!cfginit(argc,argv)) goto end;

  if(rCfg.eIn==I_fea){
    do_feastdin();
    evaluation();
    goto end;
  }

  if(!rCfg.rFlst.nNum) online(NULL);
  else{
    INT32 nI;
    for(nI=0;nI<rCfg.rFlst.nNum;nI++) processfile(rCfg.rFlst.lpF[nI]);
  }

  evaluation();
  if(rCfg.bMeasureTime) outputtimes();


end:
  cfgdone();

  CDlpObject_UnregisterAllClasses();
#ifdef _DEBUG                                                                  /* Clean up */
  dlp_xalloc_done(TRUE);
#else
  dlp_xalloc_done(FALSE);
#endif

  return 0;
}
