/* dLabPro class CFst (fst)
 * - Composition - Implementation
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
#define _CPS_MAPHASH_

/* These error macro produces a string containing file name and line number */
#define STRINGIFY2(X) #X
#define STRINGIFY1(X) STRINGIFY2(X)
#define FSTCERR(TXT)  __FILE__ "(" STRINGIFY1(__LINE__) "): " TXT

#include "fst_cps_mem.c"
#include "fst_cps_map.c"

/* These are all special epsion symbols used in composition */
#define EPSSYM     0 /* Alias for any non-epsilon which matches in both operands */
#define EPSDEF    -1 /* Default existing epsilon */
#define EPSLOOP   -2 /* Existing epsilon at self-loop transition */
#define EPSEPSL   -3 /* EPSLOOP where outer symbol of the operand is also epsilon */
#define EPSCPS    -4 /* Epsilon inserted by composition */

/* Internal transition structure */
struct fstc_t {
  struct fstc_t *nnxt,*nprv; /* Connected list of outgoing transition of initial state (or self-loops) */
  struct fstc_t *bnxt,*bprv; /* Connected list of ingoing transition of final state */
  struct fstc_s *si,*st;     /* Initial and final state */
  UINT32 n;                  /* Final state index */
  INT32  i,m,o;              /* Input, intermediate and output symbol index */
  INT32 ta,tb;               /* Transition index of left and right operand */
  FLOAT64 w;                 /* Transition weight (LSR) */
};

/* Internal state structure */
struct fstc_s {
  struct fstc_s *snxt,*sprv; /* Connected list of all states */
  struct fstc_s *hnxt,*hprv; /* Connected list in the hash chain of that state */
  struct fstc_s *qnxt;       /* Next state in queue */
  struct fstc_t *tn,*tb,*tl; /* First outgoing, self-loop and ingoing transition */
  struct fstc_t *tfl0;       /* Removable transition in universal epsilon filter */
  UINT32 ia,ie,ib;           /* State index in left operand, epsilon filter and right operand */
  char fin;                  /* Final state flag */
  char l0;                   /* Self-loop flag for universal epsilon filter */
};

/* Internal structure for extra transition components */
struct fstc_d {
  struct fstc_d *nxt; /* Next component */
  const char *name;   /* Name */
  BYTE *dat;          /* Data for the first transition */
  INT32 size;         /* Size of data */
  INT32 rlen;         /* Increment to next transition's data */
  INT16 typ;          /* Dlabpro data type */
};

/* Internal fst structure */
struct fstc_f {
  UINT32 ns,nt;         /* Number of states and transitions */
  struct fstc_s *s;     /* State memory for operand / Connected list of all states for destination */
  struct fstc_s *s0;    /* Initial state for destination */
  struct fstc_mem smem; /* State memory for destination */
  struct fstc_mem tmem; /* Transition memory */
  struct fstc_d *d;     /* Extra transition components for operands */
};

#ifndef _CPS_MAPHASH_
/* Internal done map structure */
struct fstc_m {
  unsigned char *buf; /* Bitmap data */
  uint64_t ne,nb;     /* Number of states in epsilon filter and right operand */
};
#endif

/* Internal algorithm structure */
struct fstc_c {
  char dbg;          /* Debug level */
  struct fstc_s **h; /* State hash */
  struct fstc_s  *q; /* State queue */
  struct fstc_m   m; /* Done map */
};

/* This is set to qnxt if a state is not in the queue */
#define QNONE	((struct fstc_s*)(-1))

/* Hash chain dimension */
#define HBIT            23
#define HMASK	          ((1<<HBIT)-1)
/* Calculating the chain index */
#define HCH1(ia,ie,ib)  (((ia)+1)*((ie)+1)*((ib)+1))
#define HCH(ia,ie,ib)   (((HCH1(ia,ie,ib)>>HBIT)^HCH1(ia,ie,ib))&HMASK)

#ifndef _CPS_MAPHASH_
/* Get map index from state indizies */
#define MAPID(m,a,e,b)  (((uint64_t)(a)*(m).ne+(uint64_t)(e))*(m).nb+(uint64_t)(b))
/* Done map set and get by map index */
#define MAPGETI(m,i)		((m).buf[(i)>>3]&(0x1<<((i)&0x7)))
#define MAPSETI(m,i)		((m).buf[(i)>>3]|=0x1<<((i)&0x7))
/* Done map set and get by state indizies */
#define MAPGET(m,a,e,b) MAPGETI(m,MAPID(m,a,e,b))
#define MAPSET(m,a,e,b) MAPSETI(m,MAPID(m,a,e,b))
/* Done map initialize */
#define MAPINIT(m,ans,ens,bns) { \
  if(!((m).buf=(unsigned char *)calloc((((uint64_t)(ans)*(ens)*(bns))>>3)+1,sizeof(unsigned char)))) return "Out of memory"; \
  (m).ne=(ens); (m).nb=(bns); \
}
#define MAPFREE(m)     free((m).buf)
#endif

/* Add a new transition to one of the operands
 *
 * @param f   Operand transducer
 * @param b   Initial state index
 * @param n   Final state index
 * @param i   Input symbol
 * @param o   Output symbol
 * @param w   Transition weight
 * @param ta  Transition index
 */
void fstc_faddt(struct fstc_f *f,UINT32 b,UINT32 n,INT32 i,INT32 o,FLOAT64 w,INT32 ta){
  struct fstc_t *t=(struct fstc_t *)fstc_memget(&f->tmem);
  t->n=n;
  t->i=i;
  t->o=o;
  t->w=w;
  t->ta=ta;
  t->nnxt=f->s[b].tn;
  f->s[b].tn=t;
}

/* Load one of the operands into internal structure
 *
 * @param f       The internal operand transducer
 * @param c       The dlabpro transducer
 * @param ui      The unit index to load
 * @param epsout  1 for left operand, 0 for right one
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fstc_fload(struct fstc_f *f,CFst *c,INT32 ui,char epsout){
  FST_TID_TYPE *tit;
  BYTE *tc;
  UINT32 si;
  INT32 ci;
  char psr;
  const char *err;
  struct fstc_d **dp;
  f->ns=UD_XS(c,ui);
  f->nt=UD_XT(c,ui)+f->ns;
  psr=CFst_Wsr_GetType(c,NULL)==FST_WSR_PROB;
  if((err=fstc_meminit(&f->tmem,sizeof(struct fstc_t),f->nt))) return err;
  if((err=fstc_meminit(&f->smem,sizeof(struct fstc_s)*f->ns,1))) return err;
  f->s=(struct fstc_s*)fstc_memget(&f->smem);
  if(!(tit=CFst_STI_Init(c,ui,FSTI_SORTINI))) return FSTCERR("Iterator init failed");
  for(si=0;si<f->ns;si++){
    f->s[si].tn=NULL;
    fstc_faddt(f,si,si,EPSCPS,EPSCPS,0.,-1);
    for(tc=NULL;(tc=CFst_STI_TfromS(tit,si,tc));){
      UINT32 n=*CFst_STI_TTer(tit,tc);
      INT32 i=*CFst_STI_TTis(tit,tc);
      INT32 o=*CFst_STI_TTos(tit,tc);
      FLOAT64 w=*CFst_STI_TW(tit,tc);
      if(psr) w=-1.*log(w);
      if(!epsout && i<0) i=EPSDEF;
      if( epsout && o<0) o=EPSDEF;
      if(epsout){ if(si==n && o<0) o=i<0?EPSEPSL:EPSLOOP; }
      else{ if(si==n && i<0) i=o<0?EPSEPSL:EPSLOOP; }
      fstc_faddt(f,si,n,i,o,w,CFst_STI_GetTransId(tit,tc));
    }
    f->s[si].fin=SD_FLG(c,si+tit->nFS)&SD_FLG_FINAL;
  }
  f->d=NULL;
  dp=&f->d;
  for(ci=IC_TD_DATA;ci<CData_GetNComps(AS(CData,c->td));ci++){
    INT32 off=CData_GetCompOffset(AS(CData,c->td),ci);
    struct fstc_d *d;
    if(off==tit->nOfTTis || off==tit->nOfTTos || off==tit->nOfTW) continue;
    if(!(d=(struct fstc_d*)malloc(sizeof(struct fstc_d)))) return FSTCERR("Ouf of memory");
    *dp=d; d->nxt=NULL; dp=&d->nxt;
    d->name=CData_GetCname(AS(CData,c->td),ci);
    d->dat=tit->lpFT+off;
    d->size=CData_GetCompSize(AS(CData,c->td),ci);
    d->rlen=CData_GetRecLen(AS(CData,c->td));
    d->typ=CData_GetCompType(AS(CData,c->td),ci);
  }
  CFst_STI_Done(tit);
  return NULL;
}

/* Create an epsilon filter
 *
 * @param f      Destination transducer
 * @param noeps  Select universal or fixed filter
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fstc_feps(struct fstc_f *f,char noeps){
  const char *err;
  if((err=fstc_meminit(&f->tmem,sizeof(struct fstc_t),16))) return err;
  if(noeps){
    if((err=fstc_meminit(&f->smem,sizeof(struct fstc_s)*(f->ns=1),1))) return err;
    f->s=(struct fstc_s*)fstc_memget(&f->smem);
    f->s[0].fin=1; f->s[0].tn=NULL;
    fstc_faddt(f,0,0,EPSSYM, EPSSYM, 0.,-1);
    fstc_faddt(f,0,0,EPSEPSL,EPSDEF, 0.,-1);
    fstc_faddt(f,0,0,EPSEPSL,EPSLOOP,0.,-1);
    fstc_faddt(f,0,0,EPSDEF, EPSEPSL,0.,-1);
    fstc_faddt(f,0,0,EPSLOOP,EPSEPSL,0.,-1);
  }else{
    if((err=fstc_meminit(&f->smem,sizeof(struct fstc_s)*(f->ns=4),1))) return err;
    f->s=(struct fstc_s*)fstc_memget(&f->smem);
    f->s[0].fin=1; f->s[0].tn=NULL;
    f->s[1].fin=1; f->s[1].tn=NULL;
    f->s[2].fin=1; f->s[2].tn=NULL;
    f->s[3].fin=1; f->s[3].tn=NULL;
    fstc_faddt(f,0,0,EPSLOOP,EPSCPS, 0.,-1);
    fstc_faddt(f,0,0,EPSEPSL,EPSCPS, 0.,-1);
    fstc_faddt(f,0,1,EPSCPS, EPSCPS, 0.,-1);
    fstc_faddt(f,1,1,EPSCPS, EPSLOOP,0.,-1);
    fstc_faddt(f,1,1,EPSCPS, EPSEPSL,0.,-1);
    fstc_faddt(f,1,0,EPSDEF, EPSDEF, 0.,-1);
    fstc_faddt(f,1,2,EPSCPS, EPSDEF, 0.,-1);
    fstc_faddt(f,2,2,EPSCPS, EPSDEF, 0.,-1);
    fstc_faddt(f,2,2,EPSCPS, EPSLOOP,0.,-1);
    fstc_faddt(f,2,2,EPSCPS, EPSEPSL,0.,-1);
    fstc_faddt(f,1,3,EPSDEF, EPSCPS, 0.,-1);
    fstc_faddt(f,3,3,EPSDEF, EPSCPS, 0.,-1);
    fstc_faddt(f,3,3,EPSLOOP,EPSCPS, 0.,-1);
    fstc_faddt(f,3,3,EPSEPSL,EPSCPS, 0.,-1);
    fstc_faddt(f,1,0,EPSSYM, EPSSYM, 0.,-1);
    fstc_faddt(f,2,0,EPSSYM, EPSSYM, 0.,-1);
    fstc_faddt(f,3,0,EPSSYM, EPSSYM, 0.,-1);
  }
  return NULL;
}

/* Find a used state or create a new one
 *
 * @param c   Algorithm structure
 * @param fr  Destination transducer
 * @param ia  State index in left operand
 * @param ie  State index in epsilon filter
 * @param ib  State index in right operand
 * @return    State pointer in destination transducer or NULL if expanded before
 */
struct fstc_s *fstc_news(struct fstc_c *c,struct fstc_f *fr,UINT32 ia,UINT32 ie,UINT32 ib){
  UINT32 ch=HCH(ia,ie,ib);
  struct fstc_s *s=c->h[ch];
  if(MAPGET(c->m,ia,ie,ib)) return NULL;
  while(s && (s->ia!=ia || s->ie!=ie || s->ib!=ib)) s=s->hnxt;
  if(s) return s;
  s=(struct fstc_s *)fstc_memget(&fr->smem);
  s->ia=ia; s->ie=ie; s->ib=ib;
  s->tn=s->tb=s->tl=NULL;
  s->fin=s->l0=0;
  s->tfl0=NULL;
  if(fr->s) fr->s->sprv=s;
  s->snxt=fr->s; fr->s=s;
  s->sprv=NULL;
  if(c->h[ch]) c->h[ch]->hprv=s;
  s->hnxt=c->h[ch]; c->h[ch]=s;
  s->hprv=NULL;
  s->qnxt=c->q; c->q=s;
  return s;
}

/* Free a state which is not used by any valid path
 *
 * @param c   Algorithm structure
 * @param fr  Destination transducer
 * @param s   State to free
 */
const char *fstc_sfree(struct fstc_c *c,struct fstc_f *fr,struct fstc_s *s){
  struct fstc_t *t,*t2;
  const char *err;
	if(c->dbg) printf("sfree %i,%i,%i\n",s->ia,s->ie,s->ib);
  if(s->snxt) s->snxt->sprv=s->sprv;
  if(s->sprv) s->sprv->snxt=s->snxt;
  else fr->s=s->snxt;
  for(t=s->tl;t;t=t2){ t2=t->nnxt; fstc_memput(&fr->tmem,t); }
  for(t=s->tb;t;t=t2){
    if(t->nnxt) t->nnxt->nprv=t->nprv;
    if(t->nprv) t->nprv->nnxt=t->nnxt;
    else t->si->tn=t->nnxt;
    if(!t->si->tn && !t->si->fin && t->si->qnxt==QNONE){ if((err=fstc_sfree(c,fr,t->si))) return err; }
    t2=t->bnxt;
    fstc_memput(&fr->tmem,t);
  }
  MAPSET(c->m,s->ia,s->ie,s->ib);
  if(s->hnxt) s->hnxt->hprv=s->hprv;
  if(s->hprv) s->hprv->hnxt=s->hnxt;
  else{
    UINT32 ch=HCH(s->ia,s->ie,s->ib);
    c->h[ch]=s->hnxt;
  }
  fstc_memput(&fr->smem,s);
  return NULL;
}

/* Perform internal composition
 *
 * @param fa   The internal left operand
 * @param fe   The internal epsilon filter
 * @param fb   The internal right operand
 * @param fr   The internal destination transducer
 * @param dbg  Debug level
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fstc_cps(struct fstc_f *fa,struct fstc_f *fe,struct fstc_f *fb,struct fstc_f *fr,char dbg){
  struct fstc_c c;
  const char *err;
  memset(fr,0,sizeof(struct fstc_f));
  fstc_meminit(&fr->smem,sizeof(struct fstc_s),4096);
  fstc_meminit(&fr->tmem,sizeof(struct fstc_t),4096);
  if(!(c.h=(struct fstc_s **)calloc(HMASK+1,sizeof(struct fstc_s *)))) return "Out of memory";
  MAPINIT(c.m,fa->ns,fe->ns,fb->ns);
  c.q=NULL;
  c.dbg=dbg;
  fr->s0=fstc_news(&c,fr,0,0,0);
  while(c.q){
    struct fstc_s *sb=c.q;
    struct fstc_t *ta,*te,*tb;
    struct fstc_s *sn;
    c.q=sb->qnxt; sb->qnxt=QNONE;
    if(fa->s[sb->ia].fin && fb->s[sb->ib].fin) sb->fin=1;
    if(c.dbg) printf("sexp  %i,%i,%i%s\n",sb->ia,sb->ie,sb->ib,sb->fin?" (fin)":"");
    for(ta=fa->s[sb->ia].tn;ta;ta=ta->nnxt)
      for(te=fe->s[sb->ie].tn;te;te=te->nnxt) if(ta->o==te->i || (te->i==EPSSYM && ta->o>0))
        for(tb=fb->s[sb->ib].tn;tb;tb=tb->nnxt) if((te->o!=EPSSYM && te->o==tb->i) || (te->o==EPSSYM && tb->i==ta->o))
          if((sn=fstc_news(&c,fr,ta->n,te->n,tb->n))){
            struct fstc_t *tr=(struct fstc_t *)fstc_memget(&fr->tmem);
            if(c.dbg) printf("tadd  %i,%i,%i => %i/%i,%i/%i => %i,%i,%i\n",sb->ia,sb->ie,sb->ib,ta->o,te->i,te->o,tb->i,sn->ia,sn->ie,sn->ib);
            tr->si=sb; tr->st=sn;
            tr->i=ta->i; tr->o=tb->o;
            tr->w=ta->w+tb->w;
            tr->ta=ta->ta; tr->tb=tb->ta;
            tr->m=ta->o<0?-1:ta->o;
            if(sb==sn){
              if(sb->tl) sb->tl->nprv=tr;
              tr->nnxt=sb->tl; sb->tl=tr;
              tr->nprv=NULL;
            }else{
              if(sb->tn) sb->tn->nprv=tr;
              tr->nnxt=sb->tn; sb->tn=tr;
              tr->nprv=NULL;
              if(sn->tb) sn->tb->bprv=tr;
              tr->bnxt=sn->tb; sn->tb=tr;
              tr->bprv=NULL;
            }
            if(sb->ie==sn->ie && (te->i==EPSLOOP || te->o==EPSLOOP)) sn->l0=1;
            if(sb->ie!=sn->ie && te->i==EPSCPS && te->o==EPSCPS) sn->tfl0=tr;
          }
    if(fe->ns==4 && sb->ie==1 && sb->tfl0 && (!sb->l0 || !sb->tfl0->si->l0)){
      struct fstc_s *sd=sb->tfl0->si;
      struct fstc_t *tr=sb->tfl0;
      if(c.dbg) printf("tskip %i,%i,%i => %i,%i,%i\n",sb->ia,sb->ie,sb->ib,sd->ia,sd->ie,sd->ib);
      /* remove tr=tfl0 */
      if(tr->nprv) tr->nprv->nnxt=tr->nnxt;
      else tr->si->tn=tr->nnxt;
      if(tr->nnxt) tr->nnxt->nprv=tr->nprv;
      if(tr->bprv) tr->bprv->bnxt=tr->bnxt;
      else tr->st->tb=tr->bnxt;
      if(tr->bnxt) tr->bnxt->bprv=tr->bprv;
      fstc_memput(&fr->tmem,tr);
      /* change tn in sb => sd */
      if((tr=sb->tn)){
        for(;tr->nnxt;tr=tr->nnxt) tr->si=sd;
        tr->si=sd;
        if(sd->tn) sd->tn->nprv=tr;
        tr->nnxt=sd->tn;
        sd->tn=sb->tn;
        sb->tn=NULL;
      }
      /* change tl in sb => sd */
      if((tr=sb->tl)){
        for(;tr->nnxt;tr=tr->nnxt) tr->st=tr->si=sd;
        tr->st=tr->si=sd;
        if(sd->tl) sd->tl->nprv=tr;
        tr->nnxt=sd->tl;
        sd->tl=sb->tl;
        sb->tl=NULL;
      }
      /* free sb */
      if((err=fstc_sfree(&c,fr,sb))) return err;
    }else if(!sb->fin && !sb->tn){ if((err=fstc_sfree(&c,fr,sb))) return err; }
  }
  free(c.h);
  MAPFREE(c.m);
  return NULL;
}

/* Save one transition of internal destination transducer into dlabpro one
 *
 * @param td     Pointer to transition's data in transition table
 * @param nr     Number of components
 * @param t      Internal transition
 * @param noint  If 1 do not create intermediate symbols ~TMS
 * @param da     Extra transition components in left operand
 * @param db     Extra transition components in right operand
 */
void fstc_fsavet(BYTE *td,UINT32 *roff,UINT32 nr,struct fstc_t *t,char noint,struct fstc_d *da,struct fstc_d *db){
  UINT32 i=noint?5:6;
  struct fstc_d *d;
  *(FST_ITYPE*)(td+roff[0])=t->st->ia;
  *(FST_ITYPE*)(td+roff[1])=t->si->ia;
  *(FST_STYPE*)(td+roff[2])=t->i<0?-1:t->i;
  *(FST_STYPE*)(td+roff[3])=t->o<0?-1:t->o;
  *(FST_WTYPE*)(td+roff[4])=t->w;
  if(!noint) *(FST_STYPE*)(td+roff[5])=t->m;
  for(d=da;d;d=d->nxt,i++) if(t->ta>=0) memcpy(td+roff[i],d->dat+t->ta*d->rlen,d->size);
  for(d=db;d;d=d->nxt,i++) if(t->tb>=0) memcpy(td+roff[i],d->dat+t->tb*d->rlen,d->size);
}

/* Save internal destination transducer into dlabpro one
 *
 * @param f      The internal destination transducer
 * @param c      The dlabpro destination transducer
 * @param noint  If 1 do not create intermediate symbols ~TMS
 * @param da     Extra transition components in left operand
 * @param db     Extra transition components in right operand
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fstc_fsave(struct fstc_f *f,CFst *c,char noint,struct fstc_d *da,struct fstc_d *db){
  struct fstc_s *s;
  UINT32 i;
  UINT32 rlen;
  UINT32 *roff;
  UINT32 nr=5;
  BYTE *td;
  struct fstc_d *d;
  /* Count states */
	for(i=1,s=f->s;s;s=s->snxt) s->ia=i++;
  f->s0->ia=0;
  /* Init fst */
  CFst_Reset(BASEINST(c),TRUE);
  CFst_Addunit(c,"CPS");
  CData_AddComp(AS(CData,c->td),NC_TD_TIS,DLP_TYPE(FST_STYPE));
  CData_AddComp(AS(CData,c->td),NC_TD_TOS,DLP_TYPE(FST_STYPE));
  CData_AddComp(AS(CData,c->td),NC_TD_LSR,DLP_TYPE(FST_WTYPE));
  /* Count + add data components */
  if(!noint){ CData_AddComp(AS(CData,c->td),"~TMS",DLP_TYPE(FST_STYPE)); nr++; }
  for(d=da;d;d=d->nxt,nr++) CData_AddComp(AS(CData,c->td),d->name,d->typ);
  for(d=db;d;d=d->nxt,nr++) CData_AddComp(AS(CData,c->td),d->name,d->typ);
  /* Add states + transitions */
  CFst_Addstates(c,0,i,FALSE);
  CData_Reallocate(AS(CData,c->td),f->tmem.used);
  /* Get component offsets */
  td=CData_XAddr(AS(CData,c->td),0,0);
  rlen=CData_GetRecLen(AS(CData,c->td));
  if(!(roff=(UINT32*)malloc(nr*sizeof(UINT32)))) return "Out of memory";
  for(i=0;i<nr;i++) roff[i]=CData_GetCompOffset(AS(CData,c->td),i);
  /* Fill transitions */
  for(i=0,s=f->s;s;s=s->snxt){
    struct fstc_t *t;
    if(s->fin) SD_FLG(c,s->ia)|=SD_FLG_FINAL;
    for(t=s->tn;t;t=t->nnxt) fstc_fsavet(td+(i++)*rlen,roff,nr,t,noint,da,db);
    for(t=s->tl;t;t=t->nnxt) fstc_fsavet(td+(i++)*rlen,roff,nr,t,noint,da,db);
  }
  CData_Reallocate(AS(CData,c->td),i);
  UD_XT(c,0)=i;
  free(roff);
  return NULL;
}

/* Free internal transducer
 *
 * @param fr  Internal transducer
 */
void fstc_ffree(struct fstc_f *fr){
  fstc_memfree(&fr->smem);
  fstc_memfree(&fr->tmem);
}

/* Perform composition algorithm
 *
 * @param ca     Left operand
 * @param ua     Unit index in left operand
 * @param cb     Right operand
 * @param ub     Unit index in right operand
 * @param noeps  Select universal or fixed filter
 * @param noint  If 1 do not create intermediate symbols ~TMS
 * @param dbg    Debug level
 * @param cr     Destination transducer
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fstc_compose(CFst *ca,INT32 ua,CFst *cb,INT32 ub,char noeps,char noint,char dbg,CFst *cr){
  struct fstc_f fa,fe,fb,fr;
  const char *err;
  if((err=fstc_fload(&fa,ca,ua,1)))   return err;
  if((err=fstc_feps(&fe,noeps)))      return err;
  if((err=fstc_fload(&fb,cb,ub,0)))   return err;
  if((err=fstc_cps(&fa,&fe,&fb,&fr,dbg))) return err;
  fstc_ffree(&fa);
  fstc_ffree(&fe);
  fstc_ffree(&fb);
  if((err=fstc_fsave(&fr,cr,noint,fa.d,fb.d))) return err;
  fstc_ffree(&fr);
  return NULL;
}


