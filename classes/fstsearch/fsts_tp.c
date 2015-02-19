/* dLabPro class CFstsearch (fstsearch)
 * - TP main programm
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
#include "fsts_tp_algo.h"

/* Number of initial elements in the queue's */
#define JOBSMEMSIZE 8192

void *fsts_tp_job(void *arg);

/* TP decoder config function
 *
 * The function copies the configuration from fstsearch fields
 * to the internal structure.
 *
 * @param cfg   Pointer to TP configuration
 * @param _this Pointer to fstsearch instance
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_tp_cfg(struct fsts_cfg *cfg,CFstsearch *_this){
  INT32 j;
  cfg->tp.jobs =_this->m_nTpThreads;
  cfg->tp.prnw =_this->m_nTpPrnw;
  cfg->tp.prnh =_this->m_nTpPrnh;
  if(cfg->bt==BT_LAT && cfg->tp.jobs>1) return FSTSERR("lattice backtracking not yet implemented with multiple threads");
  if(cfg->tp.prnw<0.)                   return FSTSERR("negative value for tp_prnw");
  if(cfg->tp.prnh<0 )                   return FSTSERR("negative value for tp_prnh");
  for(j=1;j<cfg->tp.jobs;) j<<=1;
  if(j!=cfg->tp.jobs)                   return FSTSERR("tp_threads is no power of two");
  if(cfg->tp.jobs>1 && cfg->tp.prnh)    return FSTSERR("tp_prnh is not available with multiple threads");
  return NULL;
}

/* TP decoder check source function
 *
 * This function checks if the source transducer is usable for decoding.
 *
 * @param src  Pointer to the source transducer
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_tp_chksrc(struct fsts_fst *src){
  UINT8 *lu, *lu2, *t;
  INT32 l;
  FLOAT64 mid=0.;
  if(!(lu =(UINT8*)calloc(src->nunits,sizeof(UINT8)))) return FSTSERR("out of memory");
  if(!(lu2=(UINT8*)calloc(src->nunits,sizeof(UINT8)))) return FSTSERR("out of memory");
  lu[0]=1;
  for(l=0;l<MAXLAYER;l++){
    UINT32 nsmax=0;
    INT32 u;
    for(u=0;u<src->nunits;u++) if(lu[u]){
      if(src->units[u].ns>nsmax) nsmax=src->units[u].ns;
      if(src->units[u].sub){
        UINT32 s;
        struct fsts_t *t;
        if(l==MAXLAYER-1) return FSTSERR("transducer has more than MAXLAYER layers");
        for(s=0;s<src->units[u].ns;s++) for(t=src->units[u].tfroms[s];t;t=t->nxt) if(t->is>=0) lu2[t->is]=1;
      }
    }
    if(nsmax) mid+=log((FLOAT64)nsmax)+(l?log((FLOAT64)src->nunits):0.);
    t=lu2;
    lu2=lu;
    lu=t;
    memset(lu2,0,sizeof(UINT8)*src->nunits);
  }
  free(lu);
  free(lu2);
  if(mid/log(2.)>=sizeof(uint64_t)*8.) return FSTSERR("too many states for uniq 64-Bit state id");
  return NULL;
}

/* TP decoder generate initial state
 *
 * This function generates an initial state for the current
 * frame and adds it to the queue.
 *
 * @param glob  Pointer to the global memory structure
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_tp_geninitial(struct fsts_glob *glob){
  struct fsts_tp_algo *algo=(struct fsts_tp_algo *)glob->algo;
  struct fsts_tp_s s;
  const char *err;
  if((err=fsts_tp_sgen(&s,NULL,NULL,&glob->src,NULL,&algo->btm,0,0))) return err;
  return fsts_tp_lsadd(glob->cfg.tp.jobs>1?&algo->jobs[0].ls1:&algo->ls1,&s,0);
}

/* TP decoder initialize function
 *
 * This function initializes the decoder.
 * It prepares the internal memory.
 *
 * @param glob  Pointer to the global memory structure
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_tp_init(struct fsts_glob *glob){
  struct fsts_tp_algo *algo;
  const char *err;
  if(glob->cfg.bt==BT_T && !glob->src.itSrc) return FSTSERR("fast loading not possible with tp_bt=\"t\"");
  if((err=fsts_tp_chksrc(&glob->src))) return err;
  if(!(algo=(struct fsts_tp_algo *)calloc(1,sizeof(struct fsts_tp_algo)))) return FSTSERR("out of memory");
  glob->algo=algo;
  if((err=fsts_btm_init(&algo->btm,&glob->cfg))) return err;
  if(glob->cfg.tp.jobs>1){
    INT32 j,jshf=0;
    struct fsts_tp_job *job;
    while((UINT32)(1<<(jshf+1))*glob->cfg.tp.jobs<=glob->src.units[0].ns) jshf++;
    if(dlp_create_mutex(&algo->mutex)!=O_K) return FSTSERR("mutex creation failed");
    if(dlp_create_cond(&algo->cond)!=O_K) return FSTSERR("condition creation failed");
    if(!(algo->jobs=(struct fsts_tp_job*)calloc(glob->cfg.tp.jobs,sizeof(struct fsts_tp_job)))) return FSTSERR("out of memory");
    for(j=0,job=algo->jobs;j<glob->cfg.tp.jobs;j++,job++){
      if((err=fsts_tp_lsinit(&job->ls1,&algo->btm,&glob->cfg,T_DOUBLE_MIN))) return err;
      if((err=fsts_tp_lsinit(&job->ls2,&algo->btm,&glob->cfg,glob->cfg.tp.prnw?0.:T_DOUBLE_MIN))) return err;
      if((err=fsts_meminit(&job->smem,sizeof(struct fsts_tp_js),JOBSMEMSIZE,0,0))) return err;
      if(dlp_create_mutex(&job->mutex)!=O_K) return FSTSERR("mutex creation failed");
      if(dlp_create_cond(&job->cond)!=O_K) return FSTSERR("condition creation failed");
      job->glob=glob;
      job->jid=j;
      job->tid=dlp_create_thread(fsts_tp_job,job);
      job->jshf=jshf;
    }
  }else{
    if((err=fsts_tp_lsinit(&algo->ls1,&algo->btm,&glob->cfg,T_DOUBLE_MIN))) return err;
    if((err=fsts_tp_lsinit(&algo->ls2,&algo->btm,&glob->cfg,glob->cfg.tp.prnw?0.:T_DOUBLE_MIN))) return err;
  }
  if((err=fsts_tp_lsinit(&algo->lsf,&algo->btm,&glob->cfg,T_DOUBLE_MIN))) return err;
  return fsts_tp_geninitial(glob);
}

/* TP decoder free function
 *
 * This function frees the internal memory of the decoder.
 *
 * @param glob  Pointer to the global memory structure
 */
void fsts_tp_free(struct fsts_glob *glob){
  struct fsts_tp_algo *algo=(struct fsts_tp_algo *)glob->algo;
  if(!algo) return;
  if(glob->cfg.tp.jobs>1){
    INT32 j;
    struct fsts_tp_job *job;
    for(j=0,job=algo->jobs;j<glob->cfg.tp.jobs;j++,job++){
      if(dlp_lock_mutex(&job->mutex)==O_K){
        job->fin=1;
        dlp_signal_cond(&job->cond);
        dlp_unlock_mutex(&job->mutex);
        dlp_join_thread(job->tid);
      }else dlp_terminate_thread(job->tid,0);
      fsts_tp_lsfree(&job->ls1);
      fsts_tp_lsfree(&job->ls2);
      fsts_memfree(&job->smem);
      dlp_destroy_cond(&job->cond);
      dlp_destroy_mutex(&job->mutex);
    }
    free(algo->jobs);
    dlp_destroy_cond(&algo->cond);
    dlp_destroy_mutex(&algo->mutex);
  }else{
    fsts_tp_lsfree(&algo->ls1);
    fsts_tp_lsfree(&algo->ls2);
  }
  fsts_tp_lsfree(&algo->lsf);
  fsts_btm_free(&algo->btm);
  free(glob->algo);
  glob->algo=NULL;
}

/* Some debug code for checking backtrack memories */
#if 0
struct chk{
  struct fsts_tp_bt *bt;
  INT32 ref;
};

void btchkb(struct chk *chk,struct fsts_tp_bt *bt){
  INT32 i=0;
  if(!bt) return;
  while(chk[i].bt && chk[i].bt!=bt) i++;
  if(!chk[i].bt){
    chk[i].bt=bt;
    chk[i+1].bt=NULL;
    chk[i].ref=0;
    btchkb(chk,bt->pa);
  }
  chk[i].ref++;
}

void btchk(struct fsts_tp_algo *algo,struct fsts_tp_bt *bt1,struct fsts_tp_bt *bt2){
  static struct chk *chk=NULL;
  static UINT32 chksize=0;
  struct fsts_tp_s *s;
//  struct fsts_tp_hs *hs;
  UINT32 i;
  if(chksize!=algo->osmem.num+1)
    chk=(struct chk *)realloc(chk,(chksize=algo->osmem.num+1)*sizeof(struct chk));
  chk[0].bt=NULL;
  for(s=algo->ls1.qs;s;s=s->nxt) btchkb(chk,s->os);
  for(s=algo->ls2.qs;s;s=s->nxt) btchkb(chk,s->os);
  btchkb(chk,bt1);
  btchkb(chk,bt2);
  for(i=0;chk[i].bt;i++) if(chk[i].bt->ref!=chk[i].ref) abort();
  if(i!=algo->osmem.used) abort();
/*  for(i=0;i<=algo->ls1.h.mask;i++)
    for(hs=algo->ls1.h.hs[i];hs;hs=hs->nxt)
      for(j=0;j<hs->use;j++) btchks(chk,fsts_hs2s(hs)+j,algo,0);
  for(i=0;i<=algo->ls2.h.mask;i++)
    for(hs=algo->ls2.h.hs[i];hs;hs=hs->nxt)
      for(j=0;j<hs->use;j++) btchks(chk,fsts_hs2s(hs)+j,algo,0);*/
}
#endif

/* Mulithreading state add function
 *
 * This function adds the state to the active state queue
 * of the correct job (selected by state index).
 *
 * @param jobs  Pointer the job's internal memory
 * @param ls    Queue index
 * @param s     Active state
 * @param jmask Job index mask
 * @param jid   Current job id
 * @return      0 on failure and 1 on success
 */
UINT8 fsts_tp_rbput(struct fsts_tp_job *jobs,UINT8 ls,struct fsts_tp_s *s,INT32 jmask,INT32 jid){
  INT32 j=(s->s[0]>>jobs->jshf)&jmask;
  struct fsts_tp_js *js;
  if(j==jid) return 0;
  if(dlp_lock_mutex(&jobs[j].mutex)!=O_K) return 0;
  js=(struct fsts_tp_js*)fsts_memget(&jobs[j].smem);
  js->s=*s;
  js->ls=ls;
  if(!jobs[j].js) dlp_signal_cond(&jobs[j].cond);
  js->nxt=jobs[j].js;
  jobs[j].js=js;
  jobs[j].run=1;
  dlp_unlock_mutex(&jobs[j].mutex);
  return 1;
}

/* Marco adding a state to the active state queue */
#define fsts_tp_lsaddj(ls,s,dbg) \
  (glob->cfg.tp.jobs>1 && fsts_tp_rbput(algo->jobs,ls,s,glob->cfg.tp.jobs-1,jid) ? NULL : \
   fsts_tp_lsadd((ls)?ls2:ls1,s,dbg))

/* TP decoder propagate function
 *
 * This function propagates one active state out of the first active state queue.
 *
 * @param ls1  First active state queue
 * @param ls2  Second active state queue
 * @param wprn Currnet pruning threshold
 * @param glob Pointer to the global memory structure
 * @param w    Pointer to the timevariant weight array
 * @param jid  Job id
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_tp_propagate(struct fsts_tp_ls *ls1,struct fsts_tp_ls *ls2,FLOAT64 wprn,struct fsts_glob *glob,struct fsts_w *w,INT32 jid){
  struct fsts_tp_algo *algo=(struct fsts_tp_algo *)glob->algo;
  struct fsts_tp_s *s1;
  const char *err;
  if((s1=fsts_tp_lsdel(ls1))){
    INT32 s1i=s1->s[s1->ds];
    INT32 u1i=s1->ds?s1->u[s1->ds-1]:0;
    struct fsts_unit *u=glob->src.units+u1i;
    struct fsts_t *t=u->tfroms[s1i];
    if(!wprn || s1->wn<wprn){
      algo->nstates++;
      #ifdef _DEBUG
      if(glob->debug>=3) printf(" state: f %4i s %8i wc %8.1f wn %8.1f os: 0x%08x\n",algo->f,s1->id,s1->wc,s1->wn,glob->cfg.numpaths>1?BTHASH(s1->bt.os):0);
      #endif
      for(;t;t=t->nxt){
        struct fsts_tp_s s2;
        if(t->is>=0 && !u->sub && !w && algo->f) continue;
        if(t->stk<0 && (!s1->nstk || s1->stk[s1->nstk-1]!=-t->stk)) continue;
        if((err=fsts_tp_sgen(&s2,s1,t,&glob->src,w,&algo->btm,u->sub,u1i==0))) return err;
        #ifdef _DEBUG
        #if 0
        { uint64_t id=s2.s[0],m=glob->src.units[0].ns; INT32 d;
          for(d=1;d<=s2.ds;d++){
            id+=s2.u[d-1]*m; m*=glob->src.nunits;
            id+=s2.s[d]*m; m*=glob->src.units[s2.u[d-1]].ns;
          }
          if(id!=s2.id) abort();
        }
        #endif
        if(glob->debug>=4) printf("  => t: f %4i s %8i wc %8.1f wn %8.1f os: 0x%08x",
          algo->f+(t->is>=0 && w && !u->sub?1:0),s2.id,s2.wc,s2.wn,glob->cfg.numpaths>1?BTHASH(s2.bt.os):0);
        #endif
        if((err=fsts_tp_lsaddj(
          t->is>=0 && w && !u->sub,
          &s2, glob->debug))) return err;
        #ifdef _DEBUG
        if(glob->debug>=4) printf("\n");
        #endif
      }
      if(u->sfin[s1i]){
        if(s1->ds){
          struct fsts_tp_s s2;
          if((err=fsts_tp_sgen(&s2,s1,NULL,&glob->src,NULL,&algo->btm,0,0))) return err;
          #ifdef _DEBUG
          if(glob->debug>=4) printf("  => u: f %4i s %8i wc %8.1f wn %8.1f os: 0x%08x",algo->f,s2.id,s2.wc,s2.wn,glob->cfg.numpaths>1?BTHASH(s2.bt.os):0);
          #endif
          if((err=fsts_tp_lsaddj(0,&s2,glob->debug))) return err;
          #ifdef _DEBUG
          if(glob->debug>=4) printf("\n");
          #endif
        }else if(!s1->nstk){
          struct fsts_tp_s s2;
          if((err=fsts_tp_sgen(&s2,s1,NULL,&glob->src,NULL,&algo->btm,0,0))) return err;
          #ifdef _DEBUG
          if(glob->debug>=4) printf("  >fin: f %4i s %8i wc %8.1f wn %8.1f os: 0x%08x\n",algo->f,s2.id,s2.wc,s2.wn,glob->cfg.numpaths>1?BTHASH(s2.bt.os):0);
          #endif
          if((err=fsts_tp_lsadd(&algo->lsf,&s2,glob->debug))) return err;
          #ifdef _DEBUG
          if(glob->debug>=4) printf("\n");
          #endif
        }
      }
    }
    fsts_tp_sfree(s1,NULL,&algo->btm);
  }
  return NULL;
}

/* TP decoder job function
 *
 * This is the root function for each decoding thread.
 *
 * @param arg  Pointer to the job internal memory
 * @return <code>NULL</code>
 */
void *fsts_tp_job(void *arg){
  struct fsts_tp_job  *job=(struct fsts_tp_job*)arg;
  struct fsts_tp_algo *algo=(struct fsts_tp_algo *)job->glob->algo;
  struct fsts_tp_js   *js;
  if(dlp_lock_mutex(&job->mutex)!=O_K){ job->ret=FSTSERR("lock mutex failed"); return NULL; }
  dlp_wait_cond(&job->cond,&job->mutex);
  while(!job->fin){
    job->run=1;
    if((js=job->js)){
      job->js=js->nxt;
      dlp_unlock_mutex(&job->mutex);
      fsts_tp_lsadd(js->ls?&job->ls2:&job->ls1,&js->s,0);
      if(dlp_lock_mutex(&job->mutex)!=O_K){ job->ret=FSTSERR("lock mutex failed"); return NULL; }
      fsts_memput(&job->smem,js);
    }else if(job->ls1.qs){
      dlp_unlock_mutex(&job->mutex);
      if((job->ret=fsts_tp_propagate(&job->ls1,&job->ls2,job->wprn,job->glob,job->w,job->jid))) break;
      if(dlp_lock_mutex(&job->mutex)!=O_K){ job->ret=FSTSERR("lock mutex failed"); return NULL; }
    }else{
      if(dlp_lock_mutex(&algo->mutex)!=O_K){ job->ret=FSTSERR("lock mutex failed"); return NULL; }
      job->run=0;
      dlp_signal_cond(&algo->cond);
      dlp_unlock_mutex(&algo->mutex);
      dlp_wait_cond(&job->cond,&job->mutex);
    }
  }
  if(dlp_lock_mutex(&algo->mutex)!=O_K){ job->ret=FSTSERR("lock mutex failed"); return NULL; }
  job->run=0;
  dlp_signal_cond(&algo->cond);
  dlp_unlock_mutex(&algo->mutex);
  dlp_unlock_mutex(&job->mutex);
  return NULL;
}

/* TP decoder multithreading search function for one frame
 *
 * This function decodes over one frame with the
 * timevariant weights in w. If w is NULL than
 * timeinvariant decoding is performed until final
 * states are reached. It uses multiple frames for decoding.
 *
 * @param glob  Pointer to the global memory structure
 * @param w     Pointer to the timevariant weight array
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_tp_isearchj(struct fsts_glob *glob,struct fsts_w *w){
  struct fsts_tp_algo *algo=(struct fsts_tp_algo *)glob->algo;
  INT32 j;
  const char *err;
  struct fsts_tp_job *job;
  FLOAT64 wprn=0.;
  if(w && glob->cfg.tp.prnw){
    FLOAT64 wmin=algo->jobs[0].ls1.wmin;
    for(j=1,job=algo->jobs+1;j<glob->cfg.tp.jobs;j++,job++)
      if(job->ls1.wmin<wmin) wmin=job->ls1.wmin;
    wprn=wmin+glob->cfg.tp.prnw;
  }
  for(j=0,job=algo->jobs;j<glob->cfg.tp.jobs;j++,job++){
    job->wprn=wprn;
    job->w=w;
    job->run=1;
  }
  for(j=0,job=algo->jobs;j<glob->cfg.tp.jobs;j++,job++){
    if(dlp_lock_mutex(&job->mutex)!=O_K) return FSTSERR("lock mutex failed");
    dlp_signal_cond(&job->cond);
    dlp_unlock_mutex(&job->mutex);
  }
  if(dlp_lock_mutex(&algo->mutex)!=O_K) return FSTSERR("lock mutex failed");
  while(1){
    for(j=0,job=algo->jobs;j<glob->cfg.tp.jobs && !job->run;j++) job++;
    if(j==glob->cfg.tp.jobs) break;
    dlp_wait_cond(&algo->cond,&algo->mutex);
  }
  dlp_unlock_mutex(&algo->mutex);
  for(j=0,job=algo->jobs;j<glob->cfg.tp.jobs;j++,job++){
    if(job->run) abort();
    if(job->ret) return job->ret;
    fsts_tp_lsfree(&job->ls1);
    job->ls1=job->ls2;
    if((err=fsts_tp_lsinit(&job->ls2,NULL,NULL,T_DOUBLE_MIN))) return err;
  }
  fsts_memputdelay(&algo->btm.os);
  algo->f++;
  #ifdef _DEBUG
  if(glob->debug>=1)
  #else
  if(glob->debug>=2)
  #endif
  {
    UINT32 nls=0;
    struct fsts_tp_s *s;
    for(j=0;j<glob->cfg.tp.jobs;j++) for(s=algo->jobs[j].ls1.qs;s;s=s->nxt) nls++;
    if(glob->debug>=2){
      printf("next frame %4i: s %5i %s",algo->f,nls,
          fsts_hdbg(&algo->ls1.h,0));
      printf(" %s %s\n",
          fsts_hdbg(&algo->ls2.h,0),
          fsts_btmdbg(&algo->btm,0));
    }
    #ifdef _DEBUG
    if(nls>algo->nlsmax) algo->nlsmax=nls;
    if(algo->ls1.h.mem.usedmax>algo->hmax) algo->hmax=algo->ls1.h.mem.usedmax;
    #endif
  }
  return NULL;
}

/* TP decoder search function for one frame
 *
 * This function decodes over one frame with the
 * timevariant weights in w. If w is NULL than
 * timeinvariant decoding is performed until final
 * states are reached.
 *
 * @param glob  Pointer to the global memory structure
 * @param w     Pointer to the timevariant weight array
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_tp_isearch1(struct fsts_glob *glob,struct fsts_w *w){
  struct fsts_tp_algo *algo=(struct fsts_tp_algo *)glob->algo;
  FLOAT64 wprn=0.;
  FLOAT64 mem;
  const char *err;
  if(w){
    if(glob->cfg.tp.prnh) wprn=fsts_tp_histthr(&algo->ls1.hist,glob->cfg.tp.prnh);
    if(glob->cfg.tp.prnw && (!wprn || algo->ls1.wmin+glob->cfg.tp.prnw>wprn))
       wprn=algo->ls1.wmin+glob->cfg.tp.prnw;
  }
  while(algo->ls1.qs) if((err=fsts_tp_propagate(&algo->ls1,&algo->ls2,wprn,glob,w,-1))) return err;
  #ifdef _DEBUG
  if(glob->debug>=1)
  #else
  if(glob->debug>=2)
  #endif
  {
    UINT32 nls=0;
    struct fsts_tp_s *s1;
    for(s1=algo->ls2.qs;s1;s1=s1->nxt) nls++;
    if(glob->debug>=2){
      printf("next frame %4i: s %5i %s",algo->f+1,nls,
          fsts_hdbg(&algo->ls1.h,0));
      printf(" %s %s\n",
          fsts_hdbg(&algo->ls2.h,0),
          fsts_btmdbg(&algo->btm,0));
    }
    #ifdef _DEBUG
    if(nls>algo->nlsmax) algo->nlsmax=nls;
    if(algo->ls1.h.mem.usedmax>algo->hmax) algo->hmax=algo->ls1.h.mem.usedmax;
    #endif
  }
  mem = 
    sizeof(struct fsts_tp_algo) +
    fsts_hmem(&algo->ls1.h) +
    fsts_hmem(&algo->ls2.h) +
    fsts_btmmem1(&algo->btm);
  if(mem>glob->mem) glob->mem=mem;
  fsts_memputdelay(&algo->btm.os);
  fsts_tp_lsfree(&algo->ls1);
  algo->ls1=algo->ls2;
  algo->f++;
  return fsts_tp_lsinit(&algo->ls2,NULL,NULL,glob->cfg.tp.prnh?algo->ls1.wmin:T_DOUBLE_MIN);
}

/* TP decoder search function
 *
 * This function decodes using the timevariant weights.
 * Iterative decoding is possible by calling multiple times with final=FALSE.
 * The last call should be with final=TRUE to decode until final states are reached.
 * If the weight array is empty at the first call timeinvariant
 * decoding is performed.
 *
 * @param glob  Pointer to the global memory structure
 * @param w     Pointer to the timevariant weight array
 * @param final Decode until final states if TRUE
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_tp_isearch(struct fsts_glob *glob,struct fsts_w *w,UINT8 final,UINT8 start){
  struct fsts_tp_algo *algo=(struct fsts_tp_algo *)glob->algo;
  const char *err;
  INT32 f;
  for(f=0;f<w->nf;f++){
    struct fsts_w wf;
    fsts_tp_lsfree(&algo->lsf);
    if((err=fsts_tp_lsinit(&algo->lsf,NULL,NULL,T_DOUBLE_MIN))) return err;
    if(start) fsts_tp_geninitial(glob);
    fsts_wf(w,f,&wf);
    if((err=glob->cfg.tp.jobs>1 ? fsts_tp_isearchj(glob,&wf) : fsts_tp_isearch1(glob,&wf))) return err;
  }
  if(!final) return NULL;
  if((err=glob->cfg.tp.jobs>1 ? fsts_tp_isearchj(glob,NULL) : fsts_tp_isearch1(glob,NULL))) return err;
  if(glob->debug>=1){
    if(glob->debug>=2) printf("Info:\n");
    printf("  nstates %4i\n",algo->nstates);
    #ifdef _DEBUG
    if(algo->ls1.h.mem.usedmax>algo->hmax) algo->hmax=algo->ls1.h.mem.usedmax;
    printf("  nlsmax  %4i\n",algo->nlsmax);
    printf("  hmax    %4i\n",algo->hmax);
    printf("  %s\n",fsts_hdbg(&algo->ls1.h,1));
    printf("  %s\n",fsts_hdbg(&algo->ls2.h,1));
    #else
    printf("  %s",fsts_hdbg(&algo->ls1.h,0));
    printf(" %s\n",fsts_hdbg(&algo->ls2.h,0));
    #endif
    printf("  %s\n",fsts_btmdbg(&algo->btm,1));
  }
  return NULL;
}

/* TP decoder backtrack function
 *
 * This function performes the backtracking after decoding.
 *
 * @param glob  Pointer to the global memory structure
 * @param itDst Destination transducer for backtracking
 * @param final In iterative decoding: backtrack only paths ending at a final state in the previous time frame
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_tp_backtrack(struct fsts_glob *glob,CFst *itDst,UINT8 final){
  struct fsts_tp_algo *algo=(struct fsts_tp_algo *)glob->algo;
  struct fsts_btinfo bti;
  struct fsts_tp_s *s;
  const char *err;
  struct fsts_tp_ls *ls=&algo->lsf;
  if(glob->state==FS_SEARCHING && !final){
    ls=&algo->ls1;
    if(glob->cfg.tp.jobs>1){
      INT32 j;
      for(j=0;j<glob->cfg.tp.jobs;j++) if(algo->jobs[j].ls1.qs){
        if(algo->ls1.qe) algo->ls1.qe->nxt=algo->jobs[j].ls1.qs;
        else algo->ls1.qs=algo->jobs[j].ls1.qs;
        algo->ls1.qe=algo->jobs[j].ls1.qe;
      }
    }
  }
  if((err=fsts_btstart(&bti,glob,&algo->btm,itDst))) return err;
  while((s=fsts_tp_lsbest(ls,glob->state!=FS_SEARCHING))){
    fsts_btpath(&bti,s->wc,&s->bt);
    if(glob->state==FS_SEARCHING) break;
    fsts_tp_sfree(s,NULL,&algo->btm);
  }
  if(glob->debug>=1 && algo->btm.lat.head) printf("  lat %4iMB\n",FSTSMB(fsts_latsize(&algo->btm.lat)));
  glob->mem += fsts_btmmem2(&algo->btm);
  return NULL;
}

