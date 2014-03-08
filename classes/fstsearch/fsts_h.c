/* dLabPro class CFstsearch (fstsearch)
 * - Search state hash
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

/* Number of initial elements in the hash memory */
#define HMEMSIZE    32768
/* Number of initial chains in the hash */
#define HINITCHS    65536
/* Left shift threshold for hash resize */
#define HRESIZETHR  0
/* Left shift in hash resize */
#define HRESIZESHF  2


/* Hash initialization function
 *
 * This function initializes the visited state hash memory.
 *
 * @param h      Hash memory
 * @param ssize  Size of one state
 * @param snum   Number of states per element
 * @param cmp    State compare function
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_hinit(struct fsts_h *h,UINT32 ssize,UINT16 snum,UINT8 (*cmp)(void *a,void *b)){
  if(ssize) h->mask=HINITCHS-1;
  if(ssize) h->ssize=ssize;
  if(snum)  h->snum=snum;
  if(cmp)   h->cmp=cmp;
  h->mem.size=sizeof(struct fsts_hs)+h->ssize*h->snum;
  fsts_meminit(&h->mem,h->mem.size,HMEMSIZE,0,0);
  if(!(h->hs=(struct fsts_hs **)calloc(h->mask+1,sizeof(struct fsts_hs *)))) return FSTSERR("out of memory");
  return NULL;
}

/* Hash free function
 *
 * This function free's the visited state hash memory.
 *
 * @param h  Hash memory
 */
void fsts_hfree(struct fsts_h *h){
  fsts_memfree(&h->mem);
  free(h->hs);
  h->hs=NULL;
}

/* Hash debug function
 *
 * This function returns a string for debug
 * output of the maximal used elements
 * and the memory usage.
 *
 * @param h  The hash
 * @param t  Type (0=short, 1=long)
 * @return The debug string (static memory!)
 */
const char *fsts_hdbg(struct fsts_h *h,UINT8 t){
  static char str[256];
  UINT32 mb=FSTSMB(fsts_memsize(&h->mem)+sizeof(void*)*(h->mask+1));
  snprintf(str,256,"h %4iMB",mb);
  #ifdef _DEBUG
  if(t) snprintf(str,256,"h %8u/%8u %4iMB (ch %6u)",h->mem.usedmax,h->mem.num,mb,h->mask);
  #endif
  return str;
}

/* This macro gets the state id from a state */
#define fsts_hsid(s)  (*((uint64_t*)s))

/* This macro gets the chain id from the state id */
/* TODO: change after resize */
#define fsts_hich(h,id)  (((id) ^ ((id)>>16) ^ ((id)>>32) ^ ((id)>>48))&(h)->mask)

/* This macro gets the chain id from a state */
#define fsts_hch(h,s)    (fsts_hich(h,fsts_hsid(s)))

/* Hash resize function
 *
 * This function expands the number of chains
 * in the visited states hash. The number is
 * left shifted by HRESIZESHF. Failure is not
 * reported, than the old size is used.
 *
 * @param h  The hash
 */
void fsts_hresize(struct fsts_h *h){
  struct fsts_h hn=*h;
  struct fsts_hs *hs;
  UINT32 ch;
  hn.mask=((h->mask+1)<<HRESIZESHF)-1;
  if(!(hn.hs=(struct fsts_hs **)calloc(hn.mask+1,sizeof(struct fsts_hs *)))) return;
  for(ch=0;ch<=h->mask;ch++) for(hs=h->hs[ch];hs;){
    struct fsts_hs *hsn=hs->nxt;
    UINT32 chn=fsts_hich(&hn,hs->id);
    if(hn.hs[chn]) hn.hs[chn]->prv=hs;
    hs->nxt=hn.hs[chn]; hn.hs[chn]=hs;
    hs->prv=NULL;
    hs=hsn;
  }
  free(h->hs);
  *h=hn;
}

/* Hash find function
 *
 * This function finds an element in the hash.
 *
 * @param h  Hash memory
 * @param s  State as reference
 * @return   The hash element or <code>NULL</code> if not found
 */
struct fsts_hs *fsts_hfind(struct fsts_h *h,void *s){
  struct fsts_hs *hs=h->hs[fsts_hch(h,s)];
  while(hs && (fsts_hsid(s)!=hs->id || (h->cmp && !h->cmp(s,fsts_hs2s(hs))))) hs=hs->nxt;
  return hs;
}

/* Hash insert function
 *
 * This function inserts a new state as new hash element.
 * The number of chains is expanded if more than
 * chains<<HRESIZETHR elements are used.
 *
 * @param h  Hash memory
 * @param s  State for insertion
 * @return   The pointer to the state in the hash element or <code>NULL</code> on error
 */
void *fsts_hins(struct fsts_h *h,void *s){
  struct fsts_hs *hs;
  INT32 ch;
  if(h->mem.used>h->mask<<HRESIZETHR) fsts_hresize(h);
  ch=fsts_hch(h,s);
  if(!(hs=(struct fsts_hs *)fsts_memget(&h->mem))) return NULL;
  hs->id=fsts_hsid(s);
  hs->use=1;
  hs->done=0;
  memcpy(fsts_hs2s(hs),s,h->ssize);
  if(h->hs[ch]) h->hs[ch]->prv=hs;
  hs->nxt=h->hs[ch]; h->hs[ch]=hs;
  hs->prv=NULL;
  return fsts_hs2s(hs);
}

/* Hash element from state function
 *
 * This function gets the hash element of a state.
 *
 * @param h  The hash
 * @param s  Selected state
 * @param i  If not NULL, will be set to the position of the state
 * @return   The hash element
 */
struct fsts_hs *fsts_hs2hs(struct fsts_h *h,void *s,UINT16 *i){
  struct fsts_hs *hs=((struct fsts_hs *)s)-1;
  if(i) *i=0;
  while(fsts_hsid(s)!=hs->id){
    hs=(struct fsts_hs *)(((BYTE*)hs)-h->ssize);
    if(i) (*i)++;
  }
  return hs;
}

/* Hash element free function
 *
 * This function removes one hash element
 * from the hash.
 *
 * @param h   The hash
 * @param hs  The element to remove
 */
void fsts_hsfree(struct fsts_h *h,struct fsts_hs *hs){
  if(hs->nxt) hs->nxt->prv=hs->prv;
  if(hs->prv) hs->prv->nxt=hs->nxt;
  else h->hs[fsts_hich(h,hs->id)]=hs->nxt;
  fsts_memput(&h->mem,hs);
}

/* Hash finalize state function
 *
 * This function deactives the state s.
 * The hash element is removed if all
 * states are deactivated or if the state
 * was pruned by frame threshold all used
 * states are deactivated. If the element
 * was removed it is returned.
 *
 * @param h    The hash
 * @param s    The deactivated state
 * @param prn  Wether s was pruned by frame threshold
 * @return     The removed hash element or NULL
 */
struct fsts_hs *fsts_hfins(struct fsts_h *h,void *s,UINT8 prn){
  struct fsts_hs *hs=fsts_hs2hs(h,s,NULL);
  if(prn && ++hs->done<h->snum) return NULL;
  fsts_hsfree(h,hs);
  return hs;
}

/* Hash state deletion function
 *
 * This function removes one state from it's
 * hash element (normally pruned by queue pruning).
 * It may move another state. Every pointer to any
 * state in the hash must be in schg. Then it will
 * be updated. If a state was moved it will be
 * returned to fix the pointer in the queue.
 *
 * @param h  The hash
 * @param s  The state to remove
 * @return   The moved state or NULL
 */
void *fsts_hdels(struct fsts_h *h,void *s){
  UINT16 i;
  struct fsts_hs *hs=fsts_hs2hs(h,s,&i);
  void *sn=NULL;
  if(i<--hs->use){
    void *so=fsts_hs2si(h,hs,hs->use);
    sn=fsts_hs2si(h,hs,i);
    if(*h->schg[0]==so) *h->schg[0]=sn;
    #ifdef OPT_AVL
    if(h->schg[1] && *h->schg[1]==so) *h->schg[1]=sn;
    #else
    if(*h->schg[1]==so) *h->schg[1]=sn;
    #endif
    memcpy(sn,so,h->ssize);
  }
  if(!hs->use) fsts_hsfree(h,hs);
  return sn;
}

