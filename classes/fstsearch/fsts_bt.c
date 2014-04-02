/* dLabPro class CFstsearch (fstsearch)
 * - Backtracking
 *
 * AUTHOR : Frank Duckhorn
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

#include "fsts_glob.h"

/* Number of initial elements in the backtrack memories */
#define BTMEMSIZE   32768

/* Backtrack node insertion function
 *
 * This function generates the backtrack node for
 * a new state by inserting into the backtrack memory.
 *
 * @param btref  Backtrack info of the previous state
 * @param tid    Transition id or output symbol
 * @param w      New path weight
 * @param btmem  Backtrack memory to use
 * @return       The new backtrack node of <code>NULL</code> on error
 */
struct fsts_bt *fsts_btgen(struct fsts_bt *btref,INT32 tid,FLOAT64 w,struct fsts_mem *btmem){
  struct fsts_bt *bt=NULL;
  #ifdef _DEBUG
  if(btref && !btref->ref) abort();
  #endif
  if(!btref){
    if(!(bt=(struct fsts_bt*)fsts_memget(btmem))) return NULL;
    bt->tid=-1;
    bt->ref=1;
    bt->w=w;
    bt->pa=NULL;
    if(btmem->size>sizeof(struct fsts_bt)+(btmem->lock?sizeof(MUTEXHANDLE):0)) BTHASH(bt)=0;
    #ifdef OPT_BTFREE
    if(btmem->lock && dlp_create_mutex(BTMUTEX(bt,btmem))!=O_K) return NULL;
    #endif
  }else if(tid>=0){
    if(!(bt=(struct fsts_bt*)fsts_memget(btmem))) return NULL;
    bt->tid=tid;
    bt->ref=1;
    bt->w=w;
    bt->pa=btref;
    #ifdef OPT_BTFREE
    if(btmem->lock && dlp_lock_mutex(BTMUTEX(btref,btmem))!=O_K) return NULL;
    btref->ref++;
    if(btmem->lock) dlp_unlock_mutex(BTMUTEX(btref,btmem));
    if(btmem->lock && dlp_create_mutex(BTMUTEX(bt,btmem))!=O_K) return NULL;
    #endif
    if(btmem->size>sizeof(struct fsts_bt)+(btmem->lock?sizeof(MUTEXHANDLE):0))
      BTHASH(bt)=((BTHASH(btref)<<11)|(BTHASH(btref)>>21))^tid;
  }else{
    bt=btref;
    #ifdef OPT_BTFREE
    if(btmem->lock && dlp_lock_mutex(BTMUTEX(btref,btmem))!=O_K) return NULL;
    btref->ref++;
    if(btmem->lock) dlp_unlock_mutex(BTMUTEX(btref,btmem));
    #endif
  }
  return bt;
}

/* Backtrack node unreference function
 *
 * This function unrefernces the backtrack node and
 * free's the element if possible.
 *
 * @param bt    Backtrack node
 * @param btmem Backtrack memory
 */
void fsts_btunref(struct fsts_bt *bt,struct fsts_mem *btmem){
  #ifdef OPT_BTFREE
  if(!bt) return;
  if(btmem->lock && dlp_lock_mutex(BTMUTEX(bt,btmem))!=O_K) return;
  if(!bt->ref || --bt->ref){
    if(btmem->lock) dlp_unlock_mutex(BTMUTEX(bt,btmem));
  }else{
    if(btmem->lock) dlp_unlock_mutex(BTMUTEX(bt,btmem));
    if(btmem->lock) dlp_destroy_mutex(BTMUTEX(bt,btmem));
    fsts_btunref(bt->pa,btmem);
    fsts_memput(btmem,bt);
  }
  #endif
}

/* Backtrack node equal
 *
 * This function check's if the history of
 * two backtrack node's is identical.
 *
 * @param a  First backtrack node
 * @param b  Second backtrack node
 * @return   1: equal, 0: not equal
 */
UINT8 fsts_bteql(struct fsts_bt *a,struct fsts_bt *b){
  if(a==b) return 1;
  if(!a || !b) return 0;
  if(BTHASH(a)!=BTHASH(b)) return 0;
  if(a->tid!=b->tid) return 0;
  return fsts_bteql(a->pa,b->pa);
}

/* Backtrack state node free function
 *
 * This function free's the backtrack node of a state.
 *
 * @param bt     State node which will be free'd
 * @param btref  Node with which bt was recombinded (for lattice generation)
 * @param wd     Weight difference between bt and btref
 * @param btm    Bactrack memory
 */ 
void fsts_btsfree(struct fsts_bts *bt,struct fsts_bts *btref,FLOAT64 wd,struct fsts_btm *btm){
  if(bt->ti){ fsts_btunref(bt->ti,&btm->ti); bt->ti=NULL; }
  if(bt->os){ fsts_btunref(bt->os,&btm->os); if(!btm->os.delay) bt->os=NULL; }
  if(btref && btm->lat.head){ btref->lat=fsts_latjoin(bt->lat,btref->lat,wd,&btm->lat); bt->lat=NULL; }
}

/* Backtrack state node generation function
 *
 * This function generates a new state node from a previous one.
 *
 * @param bt     The new active state
 * @param btref  The previous active state
 * @param t      The transition used
 * @param ui0    Switch indicating the highest level
 * @param w      New path weight
 * @param btm    Bactrack memory
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_btsgen(struct fsts_bts *bt,struct fsts_bts *btref,struct fsts_t *t,UINT8 ui0,FLOAT64 w,struct fsts_btm *btm){
  if(!btm->ti.size) bt->ti=NULL;
  else if(!(bt->ti=fsts_btgen(btref?btref->ti:NULL,t?t->id:-1,w,&btm->ti))) return FSTSERR("out of memory");
  if(!btm->os.size) bt->os=NULL;
  else if(!(bt->os=fsts_btgen(btref?btref->os:NULL,(ui0&&t)?t->os:-1,w,&btm->os))) return FSTSERR("out of memory");
  if(!btm->lat.head) bt->lat=NULL;
  else if(!(bt->lat=fsts_latgen(btref?btref->lat:NULL,(ui0&&t)?t->os:-1,&btm->lat))) return FSTSERR("out of memory");
  return NULL;
}

/* Backtrack memory init function
 *
 * This function initializes the backtrack memory.
 *
 * @param btm  The backtrack memory
 * @param cfg  Configuration
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_btm_init(struct fsts_btm *btm,struct fsts_cfg *cfg){
  UINT8 jobs=cfg->algo==FA_TP && cfg->tp.jobs>1;
  const char *err;
  if(cfg->bt==BT_T){
    UINT32 btsize=sizeof(struct fsts_bt);
    #ifdef OPT_BTFREE
    if(jobs) btsize+=sizeof(MUTEXHANDLE);
    #endif
    if((err=fsts_meminit(&btm->ti,btsize,BTMEMSIZE,0,jobs))) return err;
  }
  if(cfg->bt==BT_OS || cfg->numpaths>1){
    UINT32 ossize=sizeof(struct fsts_bt);
    if(cfg->numpaths>1) ossize+=sizeof(UINT32);
    #ifdef OPT_BTFREE
    if(jobs)  ossize+=sizeof(MUTEXHANDLE);
    #endif
    if((err=fsts_meminit(&btm->os,ossize,BTMEMSIZE,cfg->algo==FA_TP && cfg->numpaths>1,jobs))) return err;
  }
  if(cfg->bt==BT_LAT && (err=fsts_latinit(&btm->lat,cfg->latprn))) return err;
  return NULL;
}

/* Backtrack memory free function
 *
 * This function free's the backtrack memory.
 *
 * @param btm  The backtrack memory
 */
void fsts_btm_free(struct fsts_btm *btm){
  if(btm->lat.head) fsts_latfree(&btm->lat);
  fsts_memfree(&btm->ti);
  fsts_memfree(&btm->os);
}

/* Backtrack memory debug function
 *
 * This function returns a string for debug
 * output of the maximal used elements
 * and the memory usage.
 *
 * @param btm  The Backtrack memory
 * @param t    Type (0=short, 1=long)
 * @return     The debug string (static memory!)
 */
const char *fsts_btmdbg(struct fsts_btm *btm,UINT8 t){
  static char str[256];
  size_t len=0;
  #ifdef _DEBUG
  if(!t){
  #endif
    if(btm->ti.size){  snprintf(str+len,256-len,"bt %4iMB ", FSTSMB(fsts_memsize(&btm->ti)));  len=strlen(str); }
    if(btm->os.size){  snprintf(str+len,256-len,"os %4iMB ", FSTSMB(fsts_memsize(&btm->os)));  len=strlen(str); }
    if(btm->lat.head){ snprintf(str+len,256-len,"lat %4iMB ",FSTSMB(fsts_latsize(&btm->lat))); len=strlen(str); }
    if(len) str[len-1]='\0';
  #ifdef _DEBUG
  }else{
    if(btm->ti.size){  snprintf(str+len,256-len,"bt %8u/%8u %4iMB\n  ",btm->ti.usedmax,btm->ti.num,FSTSMB(fsts_memsize(&btm->ti)));  len=strlen(str); }
    if(btm->os.size){  snprintf(str+len,256-len,"os %8u/%8u %4iMB\n  ",btm->os.usedmax,btm->os.num,FSTSMB(fsts_memsize(&btm->os)));  len=strlen(str); }
    if(btm->lat.head){ snprintf(str+len,256-len,"lat %4iMB\n  ",FSTSMB(fsts_latsize(&btm->lat))); len=strlen(str); }
    if(len) str[len-3]='\0';
  }
  #endif
  return str;
}

/* Backtrack start function
 *
 * This function starts the process of
 * backtracking by initializing the backtrack
 * working info object
 *
 * @param bti    Backtrack working info object
 * @param glob   Global memory structure
 * @param btm    Backtrack memory
 * @param itDst  Destination transducer
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_btstart(struct fsts_btinfo *bti,struct fsts_glob *glob,struct fsts_btm *btm,CFst *itDst){
  bti->cfg=&glob->cfg;
  bti->btm=btm;
  bti->ui=0;
  bti->ti=0;
  bti->itDst=itDst;
  bti->src=&glob->src;
  if(bti->cfg->bt==BT_T){
    if(!bti->src->itSrc) return FSTSERR("fast loading not possible with tp_bt=\"t\"");
    CData_Scopy(AS(CData,bti->itDst->td),AS(CData,bti->src->itSrc->td));                         /* Copy structure of transition table*/
  }
  if((bti->gwi=CData_FindComp(AS(CData,bti->itDst->ud),"~GW"))<0){
    CData_AddComp(AS(CData,bti->itDst->ud),"~GW",T_DOUBLE);
    bti->gwi=CData_GetNComps(AS(CData,bti->itDst->ud))-1;
  }
  return NULL;
}

/* Backtrack path function
 *
 * This function performs backtracking for one path.
 *
 * @param bti  Backtrack working info object
 * @param bts  Backtrack state node of final state 
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_btpath(struct fsts_btinfo *bti,FLOAT64 w,struct fsts_bts *bts){
  char uname[32];
  struct fsts_bt *bt,*btf=bti->cfg->bt==BT_T?bts->ti:bts->os;
  INT32 nt,si;
  CFst *itDst=bti->itDst;
  CFst *itSrc=bti->src->itSrc;
  snprintf(uname,32,"path%i",bti->ui);
  if(bti->cfg->bt!=BT_T){
    itDst->m_bFst=TRUE;
    itDst->m_bLsr=TRUE;
  }
  CFst_Addunit(itDst,uname);
  if(!bti->ui){
    bti->rlt=CData_GetRecLen(AS(CData,itDst->td));                                 /* Get record length of trans. table  */
    bti->ctis=CData_FindComp(AS(CData,itDst->td),NC_TD_TIS);
    bti->ctos=CData_FindComp(AS(CData,itDst->td),NC_TD_TOS);
    CFst_Wsr_GetType(itDst,&bti->clsr);
  }
  CData_Dstore(AS(CData,itDst->ud),w,bti->ui,bti->gwi);
  if(bti->cfg->bt==BT_LAT) return fsts_latbt(bts->lat,&bti->btm->lat,itDst,bti->ui++,w);
  for(nt=0,bt=btf;bt->tid>=0;bt=bt->pa) nt++;
  CFst_Addstates(itDst,bti->ui,nt?nt:1,FALSE);
  CFst_Addstates(itDst,bti->ui,1,TRUE);
  if(!nt){
    CFst_AddtransEx(itDst,bti->ui,0,1,-1,-1,w);
    bti->ti++;
    return NULL;
  }
  CData_Reallocate(AS(CData,itDst->td),bti->ti+nt);
  UD_XT(itDst,bti->ui)=nt;
  si=nt;
  bti->ti+=nt-1;
  for(bt=btf;bt->tid>=0;bt=bt->pa,bti->ti--,si--){
    if(bti->cfg->bt==BT_T){
      INT32 uis;
      dlp_memmove(CData_XAddr(AS(CData,itDst->td),bti->ti,0),CData_XAddr(AS(CData,itSrc->td),bt->tid,0),bti->rlt);
      if(bti->src->nunits==UD_XXU(itSrc)){
        if(bti->ctos>=0 && bt->tid>=UD_XT(itSrc,0)) *(FST_STYPE*)CData_XAddr(AS(CData,itDst->td),bti->ti,bti->ctos)=-1;
        if(bti->ctis>=0) for(uis=0;uis<bti->src->nunits;uis++)
          if(bti->src->units[uis].sub && bt->tid>=UD_FT(itSrc,uis) && bt->tid<UD_FT(itSrc,uis)+UD_XT(itSrc,uis)){
            *(FST_STYPE*)CData_XAddr(AS(CData,itDst->td),bti->ti,bti->ctis)=-1;
            break;
          }
      }
    }else{
      if(bti->ctis>=0) *(FST_STYPE*)CData_XAddr(AS(CData,itDst->td),bti->ti,bti->ctis)=-1;
      if(bti->ctos>=0) *(FST_STYPE*)CData_XAddr(AS(CData,itDst->td),bti->ti,bti->ctos)=bt->tid;
    }
    TD_INI(itDst,bti->ti)=si-1;
    TD_TER(itDst,bti->ti)=si;
    if(bti->clsr>=0){
      *(FST_WTYPE*)CData_XAddr(AS(CData,itDst->td),bti->ti,bti->clsr)=w-(si==1?0.:bt->w);
      w=bt->w;
    }
  }
  bti->ti+=nt+1;
  bti->ui++;
  return NULL;
}

