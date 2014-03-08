/* dLabPro class CFstsearch (fstsearch)
 * - A* queue
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
#include "fsts_as_algo.h"

/* Default size of the queue */
#define QDEFSIZE  32767

/* Weight threshold for queue pruning */
#define QPRNW  0.3
/* Maximal fraction of nodes to prune */
#define QPRNN  0.05

UINT32 nextbase2(UINT32 v){
  UINT32 i=1;
  while(v>i) i<<=1;
  return i;
}

/* A* active state queue init function
 *
 * This function initializes the active state queue.
 *
 * @param q    The active state queue
 * @param len  Size of the queue (0=auto + init with QDEFSIZE)
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_as_qinit(struct fsts_as_q *q,UINT32 len){
  q->prn=len!=0;
  q->used=0;
  q->usedmax=0;
  q->wmax=0.;
  q->len=len;
  #ifdef OPT_AVL
  fsts_meminit(&q->qsmem,sizeof(struct fsts_as_qs),32768,0,0);
  q->head=(struct fsts_as_qs*)fsts_memget(&q->qsmem);
  q->head->s=NULL;
  q->head->pa=q->head->ch[0]=q->head->ch[1]=NULL;
  q->head->nxt=q->head->prv=NULL;
  q->head->h=0;
  q->first=q->last=NULL;
  #else
  q->len = len ? nextbase2(len)-1 : QDEFSIZE;
  if(!(q->s=(struct fsts_as_s **)malloc((q->len+1)*sizeof(struct fsts_as_s *)))) return FSTSERR("out of memory");
  #endif
  return NULL;
}

/* A* active state queue free function
 *
 * This function free's the active state queue.
 *
 * @param q    The active state queue
 */
void fsts_as_qfree(struct fsts_as_q *q){
  #ifdef OPT_AVL
  fsts_memfree(&q->qsmem);
  #else
  free(q->s);
  #endif
}

/* A* active state queue debug function
 *
 * This function returns a string for debug
 * output of the currently used elements
 * and the memory usage.
 *
 * @param q  The active state queue
 * @param t  Type (0=short, 1=long)
 * @return The debug string (static memory!)
 */
const char *fsts_as_qdbg(struct fsts_as_q *q,UINT8 t){
  static char str[256];
  if(!t) snprintf(str,256,"q %8u",q->used);
  else
    #ifdef OPT_AVL
    #ifdef _DEBUG
    snprintf(str,256,"q %8u/%8u %4iMB",q->usedmax,q->len,(int)FSTSMB(fsts_memsize(&q->qsmem)));
    #else
    snprintf(str,256,"q %4iMB",(int)FSTSMB(fsts_memsize(&q->qsmem)));
    #endif
    #else
    #ifdef _DEBUG
    snprintf(str,256,"q %8u/%8u %4iMB",q->usedmax,q->len,(int)FSTSMB(q->len*sizeof(struct fsts_as_s*)));
    #else
    snprintf(str,256,"q %4iMB",(int)FSTSMB(q->len*sizeof(struct fsts_as_s*)));
    #endif
    #endif
  return str;
}

#ifndef OPT_AVL
/* A* active state queue resize function
 *
 * This function resizes the active state queue.
 * The new size will be twice the old one plus one.
 *
 * @param q  The active state queue
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_as_qresize(struct fsts_as_q *q){
  q->len+=q->len+1;
  if(!(q->s=(struct fsts_as_s **)realloc(q->s,(q->len+1)*sizeof(struct fsts_as_s *)))) return FSTSERR("out of memory");
  return NULL;
}

/* A* active state queue heapify function
 *
 * This function inserts a new active state s
 * into the queue by replacing the one at
 * position pn. The active state queue is then
 * reordered to hold the heap criterium:
 * the whole child tree of each node has higher weight.
 *
 * @param q   The active state queue
 * @param s   The state to insert (or move)
 * @param pn  The position to replace (or add)
 */
#define OPT_PV
void fsts_as_qheapify(struct fsts_as_q *q,struct fsts_as_s *s,UINT32 pn){
  register UINT32 p = pn;
  register UINT32 p2;
  register const FLOAT64 wh = s->w;
  #ifdef OPT_PV
  register UINT32 pv=0;
  #endif
  while(p>1){
    p2=p>>1;
    if(q->s[p2]->w<=wh) break;
    q->s[p]=q->s[p2];
    q->s[p]->qp=p;
    #ifdef OPT_PV
    pv=p^1;
    #endif
    p=p2;
  }
  #ifdef OPT_PV
  if(pv && pv<=q->used){
    if(q->s[pv]->w>=wh) goto end;
    q->s[p]=q->s[pv];
    q->s[p]->qp=p;
    p=pv;
  }
  #endif
  while(1){
    register FLOAT64 w2,w3;
    register UINT32 p3;
    p2=p<<1;
    if(p2>q->used) break;
    p3=p2+1;
    w3=p3>q->used ? T_DOUBLE_MAX : q->s[p3]->w;
    w2=q->s[p2]->w;
    if(w3<wh){ if(w2>=w3) p2=p3; }else if(w2>=wh) break;
    q->s[p]=q->s[p2];
    q->s[p]->qp=p;
    p=p2;
  }
#ifdef OPT_PV
end:
#endif
  q->s[p]=s;
  s->qp=p;
}

/* A* active state queue prune function
 *
 * This function prunes the active state queue
 * by removing up to QPRNN fraction of leaf
 * nodes whose weight is QPRNW fraction between
 * q->wmax and the best node.
 *
 * @param q  The active state queue
 */
void fsts_as_qprn(struct fsts_as_q *q){
  FLOAT64 wmin=q->wmax-(q->wmax-q->s[1]->w)*QPRNW;
  UINT32  uend=q->used-q->len*QPRNN;
  UINT32  pend=(q->len+1)/2;
  UINT32  p;
  for(p=q->used;uend<q->used && p>=pend;p--) if(q->s[p]->w>=wmin){
    struct fsts_as_s *sfix;
    fsts_as_sfree(q->s[p],NULL,*(((struct fsts_btm**)q)-1)); /* TODO: remove dirty btm pointer calc */
    sfix=(struct fsts_as_s*)fsts_hdels((struct fsts_h*)(q+1),q->s[p]); /* TODO: remove dirty hash address calc */
    if(sfix && sfix->qp) q->s[sfix->qp]=sfix;
    /* TODO: if(hs->done==hs->use && prnf && q->s[p]->f<algo->f-prnf) fsts_hdel(hs) */
    fsts_as_qheapify(q,q->s[q->used--],p);
  }
  q->wmax=wmin;
/*  printf("qprn: prn %i of %i\n",q->len-q->used,q->len-uend);*/
}

/* A* active state queue replace function
 *
 * This function replaces the state at position p
 * in the queue with the new one s or adds the
 * new state (p=0).
 *
 * @param q  The active state queue
 * @param s  The state to insert
 * @param p  The position to replace (p=0: add)
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_as_qrpl(struct fsts_as_q *q,struct fsts_as_s *s,UINT32 p){
  const char *err;
  if(!p){
    if(q->used==q->len){
      if(q->prn){
        s->qp=0;
        ((struct fsts_h*)(q+1))->schg[1]=(void**)&s; /* TODO: remove dirty hash address calc */
        while(q->used==q->len) fsts_as_qprn(q);
      }else{ if((err=fsts_as_qresize(q))) return err; }
    }
    p=++q->used;
    if(q->used>q->usedmax) q->usedmax=q->used;
  }
  if(q->prn && s->w>q->wmax) q->wmax=s->w;
  fsts_as_qheapify(q,s,p);
  return NULL;
}

/* A* active state queue pop function
 *
 * This function removes and returns the topmost
 * state in the queue (with the lowest weight).
 *
 * @param q  The active state queue
 * @return   The topmost state
 */
struct fsts_as_s *fsts_as_qpop(struct fsts_as_q *q,UINT32 p){
  struct fsts_as_s *s;
  if(!q->used) return NULL;
  s=q->s[p];
  if(--q->used) fsts_as_qheapify(q,q->s[q->used+1],p);
  s->qp=0;
  return s;
}

#else

#define Hd(avl,d)	((avl)->ch[d]?(avl)->ch[d]->h:0)
#define H0(avl)		Hd(avl,0)
#define H1(avl)		Hd(avl,1)
#define POS(qs)	  ((qs)->pa->ch[0]==(qs)?0:1)
#define QCH(qs)   ((qs)->h=MAX(H0(qs),H1(qs))+1)

struct fsts_as_qs *fsts_as_qrot1(struct fsts_as_qs *qs,INT32 dir){
  register struct fsts_as_qs *rot=qs->ch[dir];
  if(Hd(rot,dir) < rot->h-1){
    register struct fsts_as_qs *sub=rot->ch[!dir];
    qs->pa->ch[POS(qs)]=sub;         sub->pa=qs->pa;
    if((rot->ch[!dir]=sub->ch[dir])) rot->ch[!dir]->pa=rot;
    sub->ch[dir]=rot;                rot->pa=sub;
    if((qs->ch[dir]=sub->ch[!dir]))  qs->ch[dir]->pa=qs;
    sub->ch[!dir]=qs;                qs->pa=sub;
    QCH(qs); QCH(rot); QCH(sub);
    return sub;
  }else{
    qs->pa->ch[POS(qs)]=rot;         rot->pa=qs->pa;
    if((qs->ch[dir]=rot->ch[!dir]))  qs->ch[dir]->pa=qs;
    rot->ch[!dir]=qs;                qs->pa=rot;
    QCH(qs); QCH(rot);
    return rot;
  }
}

void fsts_as_qrot(struct fsts_as_qs *qs){
  register INT32 ho,h0,h1;
  if(!qs || !qs->s) return;
  ho=qs->h; h0=H0(qs); h1=H1(qs);
  qs->h=MAX(h0,h1)+1;
  if(h0<h1-1) qs=fsts_as_qrot1(qs,1);
  if(h1<h0-1) qs=fsts_as_qrot1(qs,0);
  if(ho!=qs->h) fsts_as_qrot(qs->pa);
}

const char *fsts_as_qins(struct fsts_as_q *q,struct fsts_as_s *s){
  struct fsts_as_qs *qs=(struct fsts_as_qs*)fsts_memget(&q->qsmem);
  register struct fsts_as_qs **p=q->head->ch+0, *pa=q->head;
  register INT32 pos=-1;
  if(!qs) return FSTSERR("out of memory");
  qs->s=s;
  s->qs=qs;
  while(*p){ pa=*p; p=p[0]->ch+(pos=(s->w > p[0]->s->w)?1:0); }
  *p=qs;
  qs->pa=pa;
  qs->ch[0]=qs->ch[1]=NULL;
  qs->h=1;
  if(pos<0){
    qs->nxt=qs->prv=NULL;
    q->first=q->last=qs;
  }else if(!pos){
    qs->nxt=pa;
    qs->prv=pa->prv;
    pa->prv=qs;
    if(!qs->prv) q->first=qs;
    else qs->prv->nxt=qs;
  }else{
    qs->prv=pa;
    qs->nxt=pa->nxt;
    pa->nxt=qs;
    if(!qs->nxt) q->last=qs;
    else qs->nxt->prv=qs;
  }
  q->used++;
  if(q->used>q->usedmax) q->usedmax=q->used;
  fsts_as_qrot(pa);
  return NULL;
}

void fsts_as_qdel(struct fsts_as_q *q,struct fsts_as_qs *qs){
  q->used--;
  if(qs->prv) qs->prv->nxt=qs->nxt; else q->first=qs->nxt;
  if(qs->nxt) qs->nxt->prv=qs->prv; else q->last=qs->prv;
  if(qs->pa){
    register struct fsts_as_qs *rot=qs->pa, *ins=NULL;
    register INT32 pos=POS(qs);
    if(!qs->ch[0]){ if((qs->pa->ch[pos]=qs->ch[1])) qs->ch[1]->pa=qs->pa; }
    else if(!qs->ch[1]){ qs->pa->ch[pos]=qs->ch[0]; qs->ch[0]->pa=qs->pa; }
    else{
      rot=qs->prv;
      if(qs->ch[0]!=rot){
        if((rot->pa->ch[1]=rot->ch[0])) rot->ch[0]->pa=rot->pa;
        ins=rot->pa;
        if((rot->ch[0]=qs->ch[0])) rot->ch[0]->pa=rot;
      }
      qs->pa->ch[pos]=rot;       rot->pa=qs->pa;
      if((rot->ch[1]=qs->ch[1])) rot->ch[1]->pa=rot;
      rot->h=qs->h;
    }
    if(ins) fsts_as_qrot(ins);
    if(rot) fsts_as_qrot(rot);
    fsts_memput(&q->qsmem,qs);
  }
}

const char *fsts_as_qrpl(struct fsts_as_q *q,struct fsts_as_s *s,struct fsts_as_qs *qs){
  if(!qs && q->len && q->used==q->len){
    if(s->w>=q->last->s->w){
      ((struct fsts_h*)(q+1))->schg[1]=NULL; /* TODO: remove dirty hash address calc */
      fsts_hdels((struct fsts_h*)(q+1),s); /* TODO: remove dirty hash address calc */
      return NULL;
    }else{
      ((struct fsts_h*)(q+1))->schg[1]=(void**)&s; /* TODO: remove dirty hash address calc */
      struct fsts_as_s *sfix=(struct fsts_as_s*)fsts_hdels((struct fsts_h*)(q+1),q->last->s); /* TODO: remove dirty hash address calc */
      if(sfix && sfix->qs) sfix->qs->s=sfix;
      qs=q->last;
    }
  }
  if(qs) fsts_as_qdel(q,qs);
  return fsts_as_qins(q,s);
}

struct fsts_as_s *fsts_as_qpop(struct fsts_as_q *q,struct fsts_as_qs *qs){
  struct fsts_as_s *s;
  if(!qs) qs=q->first;
  if(!qs) return NULL;
  s=qs->s;
  s->qs=NULL;
  fsts_as_qdel(q,qs);
  return s;
}

#endif

