/* dLabPro class CFstsearch (fstsearch)
 * - Global search managment
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

#if _POSIX_C_SOURCE >= 199309L
#include <time.h>
#endif
FLOAT64 fsts_gettime(){
  #if _POSIX_C_SOURCE >= 199309L
  static struct timespec tl={0,0};
  struct timespec tn;
  FLOAT64 time;
  if(clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&tn)) return 0.;
  time=(FLOAT64)(tn.tv_sec-tl.tv_sec)*1000.+(FLOAT64)(tn.tv_nsec-tl.tv_nsec)/1000000.;
  tl=tn;
  return time;
  #else
  /* TODO: precise time measurement for windows system */
  /*       (only process time, at least microseconds)  */
  return 0.;
  #endif
}

/* Macro returning the global internal memory structure */
#define fsts_getglob() { \
  if(!_this->m_lpGlob) \
    _this->m_lpGlob=calloc(1,sizeof(struct fsts_glob)); \
  if(!_this->m_lpGlob) \
    return IERROR(_this,ERR_NOMEM,0,0,0); \
  glob=(struct fsts_glob*)_this->m_lpGlob; \
}

/* Decoder free function
 *
 * Frees the internal memory of the decoder.
 *
 * @param glob  Pointer to the global memory structure
 */
void fsts_freealgo(struct fsts_glob *glob){
  if(!glob->algo) return;
  switch(glob->cfg.algo){
  case FA_TP:  fsts_tp_free(glob);  break;
  case FA_AS:  fsts_as_free(glob);  break;
  case FA_SDP: fsts_sdp_free(glob); break;
  }
  glob->algo=NULL;
  glob->state=FS_FREE;
}

/* see fstsearch.def */
INT16 CGEN_PUBLIC CFstsearch_Load(CFstsearch *_this,CFst *itSrc,long nUnit){
  struct fsts_glob *glob;
  const char *err;
  CFstsearch_Unload(_this);
  fsts_getglob();
  fsts_freealgo(glob);
  _this->m_bLoaded=FALSE;
  if((err=fsts_load(&glob->src,itSrc,nUnit,_this->m_bFast))){
    CFstsearch_Unload(_this);
    return IERROR(_this,FSTS_STR,err,0,0);
  }
  return CFstsearch_Restart(_this);
}

/* see fstsearch.def */
INT16 CGEN_PUBLIC CFstsearch_Isearch(CFstsearch *_this,CData *idWeights){
  struct fsts_glob *glob;
  struct fsts_w w;
  const char *err;
  fsts_getglob();
  glob->debug=BASEINST(_this)->m_nCheck;
  switch(glob->state){
  case FS_FREE:  return IERROR(_this,FSTS_STR,FSTSERR("load transducer first"),0,0);
  case FS_BEGIN: _this->m_nTime=0.; glob->mem=0.; break;
  case FS_SEARCHING:
    if(glob->cfg.algo!=FA_TP) return IERROR(_this,FSTS_STR,FSTSERR("iterative decoding not implemented for that algo"),0,0);
  break;
  case FS_END:   return IERROR(_this,FSTS_STR,FSTSERR("restart search first"),0,0);
  }
  if((err=fsts_wgen(&w,idWeights))) return IERROR(_this,FSTS_STR,err,0,0);
  if(!w.w){
    if(glob->cfg.algo!=FA_TP && glob->cfg.algo!=FA_AS) return IERROR(_this,FSTS_STR,FSTSERR("timeinvariant decoding not implemented for this algo"),0,0);
    if(!_this->m_bFinal) return IERROR(_this,FSTS_STR,FSTSERR("timeinvariant iterative decoding not possible"),0,0);
  }else if(!_this->m_bFinal && glob->cfg.algo!=FA_TP){ err=FSTSERR("iterative decoding not implemented for that algo"); goto end; }
  if(_this->m_bStart && glob->cfg.algo!=FA_TP){ err=FSTSERR("start option not implemented for that algo"); goto end; }
  glob->state=FS_SEARCHING;
  _this->m_bLoaded=FALSE;
  fsts_gettime();
  switch(glob->cfg.algo){
  case FA_TP:  err=fsts_tp_isearch(glob,&w,_this->m_bFinal,_this->m_bStart); break;
  case FA_AS:  err=fsts_as_isearch(glob,&w);  break;
  case FA_SDP: err=fsts_sdp_isearch(glob,&w); break;
  }
  _this->m_nTime+=fsts_gettime();
  _this->m_nMem=glob->mem;
  if(err) goto end;
  if(_this->m_bFinal) glob->state=FS_END;
end:
  fsts_wfree(&w);
  return err ? IERROR(_this,FSTS_STR,err,0,0) : O_K;
}

/* see fstsearch.def */
INT16 CGEN_PUBLIC CFstsearch_Backtrack(CFstsearch *_this,CFst *itDst){
  struct fsts_glob *glob;
  const char *err=NULL;
  fsts_getglob();
  glob->debug=BASEINST(_this)->m_nCheck;
  CFst_Reset(BASEINST(itDst),TRUE);
  if(glob->state==FS_SEARCHING){
    if(glob->cfg.algo!=FA_TP) return IERROR(_this,FSTS_STR,FSTSERR("backtracking while search not possible for this algo"),0,0);
  }else if(glob->state!=FS_END)
    return IERROR(_this,FSTS_STR,FSTSERR("no search before backtracking"),0,0);
  fsts_gettime();
  switch(glob->cfg.algo){
  case FA_TP:  err=fsts_tp_backtrack(glob,itDst);  break;
  case FA_AS:  err=fsts_as_backtrack(glob,itDst);  break;
  case FA_SDP: err=fsts_sdp_backtrack(glob,itDst); break;
  }
  _this->m_nTime+=fsts_gettime();
  _this->m_nMem=glob->mem;
  if(err) return IERROR(_this,FSTS_STR,err,0,0);
  if(glob->src.itSrc){
    CData_Copy(itDst->is,glob->src.itSrc->is);
    CData_Copy(itDst->os,glob->src.itSrc->os);
  }
  if(glob->state==FS_SEARCHING) return NULL;
  return CFstsearch_Restart(_this);
}

/* Unload source transducer
 *
 * This function unloads the source transducer and
 * frees all internal memory.
 *
 * @param _this Pointer to fstsearch instance
 * @return Always O_K
 */
INT16 CGEN_PUBLIC CFstsearch_Unload(CFstsearch *_this){
  struct fsts_glob *glob;
  if(!_this->m_lpGlob) return O_K;
  fsts_getglob();
  fsts_freealgo(glob);
  fsts_unload(&glob->src);
  free(glob);
  _this->m_lpGlob=NULL;
  _this->m_bLoaded=FALSE;
  return O_K;
}

/* see fstsearch.def */
INT16 CGEN_PUBLIC CFstsearch_Restart(CFstsearch *_this){
  struct fsts_glob *glob;
  const char *err=NULL;
  if(!_this->m_lpGlob) return O_K;
  fsts_getglob();
  fsts_freealgo(glob);
  _this->m_bLoaded=FALSE;
  _this->m_bFinal=FALSE;
  if((err=fsts_cfg(&glob->cfg,_this))) return IERROR(_this,FSTS_STR,err,0,0);
  switch(glob->cfg.algo){
  case FA_TP:  err=fsts_tp_init(glob);  break;
  case FA_AS:  err=fsts_as_init(glob);  break;
  case FA_SDP: err=fsts_sdp_init(glob); break;
  }
  if(err){
    fsts_freealgo(glob);
    return IERROR(_this,FSTS_STR,err,0,0);
  }
  glob->state=FS_BEGIN;
  _this->m_bLoaded=TRUE;
  return O_K;
}

/* see fstsearch.def */
INT16 CGEN_PUBLIC CFstsearch_Search(CFstsearch *_this,CFst *itSrc,long nUnit,CData *idWeights,CFst *itDst){
  INT16 ret;
  CREATEVIRTUAL(CFst,itSrc,itDst);
  CFst_Reset(BASEINST(itDst),TRUE);
  if((ret=CFstsearch_Load(_this,itSrc,nUnit))!=O_K) goto end;
  _this->m_bFinal=TRUE;
  if((ret=CFstsearch_Isearch(_this,idWeights))!=O_K) goto end;
  if((ret=CFstsearch_Backtrack(_this,itDst))!=O_K) goto end;
  DESTROYVIRTUAL(itSrc,itDst);
  return CFstsearch_Unload(_this);
end:
  DESTROYVIRTUAL(itSrc,itDst);
  return ret;
}

