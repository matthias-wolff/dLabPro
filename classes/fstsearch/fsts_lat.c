/* dLabPro class CFstsearch (fstsearch)
 * - Lattice generation
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

/* Lattice init function
 *
 * This function initializes the lattice structure and all sub structures
 *
 * @param lat  Pointer to lattice storage structure
 * @param prn  Pruning threshold
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_latinit(struct fsts_lat *lat,FLOAT64 prn){
  const char *err;
  if((err=fsts_meminit(&lat->asmem,sizeof(struct fsts_latas),32768,0,0))) return err;
  if((err=fsts_meminit(&lat->dsmem,sizeof(struct fsts_latds),32768,0,0))) return err;
  if((err=fsts_meminit(&lat->resmem,sizeof(struct fsts_latres),32768,0,0))) return err;
  if(!(lat->head=(struct fsts_latas *)fsts_memget(&lat->asmem))) return FSTSERR("out of memory");
  memset(lat->head,0,sizeof(struct fsts_latas));
  lat->head->os=-1;
  lat->prn=prn;
  return NULL;
}

/* Lattice frees function
 *
 * This function frees the lattice structure and all sub structures
 *
 * @param lat  Pointer to lattice storage structure
 */
void fsts_latfree(struct fsts_lat *lat){
  fsts_memfree(&lat->asmem);
  fsts_memfree(&lat->dsmem);
  fsts_memfree(&lat->resmem);
}

/* Lattice size function
 *
 * This function reports the current memory usage
 * of the lattice structure.
 *
 * @param lat  Pointer to lattice storage structure
 * @return     Memory used in MB
 */
UINT32 fsts_latsize(struct fsts_lat *lat){
  return fsts_memsize(&lat->asmem)+fsts_memsize(&lat->dsmem)+fsts_memsize(&lat->resmem);
}

/* Lattice generation function
 *
 * This function is called whenever the decoder expands
 * a transition. A new active state is generated or the
 * old one returned if possible.
 *
 * @param asref  Previous active state
 * @param os     Transition output symbol
 * @param lat    Lattice storage structure
 * @return       The new active state or <code>NULL</code> on error
 */
struct fsts_latas *fsts_latgen(struct fsts_latas *asref,INT32 os,struct fsts_lat *lat){
  if(!asref) return lat->head;
  if(os<0) return asref;
  struct fsts_latas *as=(struct fsts_latas*)fsts_memget(&lat->asmem);
  if(!as) return NULL;
  as->pa=asref;
  as->paw=NULL;
  as->os=os;
  as->w=0.;
  as->s=-1;
  return as;
}

/* Lattice join function
 *
 * This function is called whenever the decoder
 * recombines two hypothesis's. A new active state
 * is generated which refers to both previous ones.
 *
 * @param as     The recombined previous active state
 * @param asref  The surviving previous active state
 * @param wd     Weight difference of previous active states
 * @param lat    Lattice storage structure
 * @return       The new active state or <code>NULL</code> on error
 */
struct fsts_latas *fsts_latjoin(struct fsts_latas *as,struct fsts_latas *asref,FLOAT64 wd,struct fsts_lat *lat){
  if(as==asref) return asref;
  if(lat->prn && wd>lat->prn) return asref;
  struct fsts_latas *asnew=(struct fsts_latas*)fsts_memget(&lat->asmem);
  if(!asnew) return NULL;
  asnew->pa=asref;
  asnew->paw=as;
  asnew->os=-1;
  asnew->w=wd;
  asnew->s=-1;
  return asnew;
}

/* Lattice determined state creation function
 *
 * This function creates a new determined state.
 *
 * @param dsmem  Memory for determined states
 * @return       The new determined state
 */
struct fsts_latds *fsts_latdsnew(struct fsts_mem *dsmem){
  struct fsts_latds *ds=(struct fsts_latds*)fsts_memget(dsmem);
  ds->hnxt=NULL; ds->qnxt=NULL; ds->res=NULL;
  ds->nres=0; ds->hash=0;
  ds->s=-1;
  ds->os=-1;
  ds->w=T_DOUBLE_MAX;
  return ds;
}

/* Lattice determined state free function
 *
 * This function frees a determined state
 * and it's residuals.
 *
 * @param ds   Determined state
 * @param lat  Lattice storage structure
 */
void fsts_latdsfree(struct fsts_latds *ds,struct fsts_lat *lat){
  struct fsts_latres *res,*resnxt;
  for(res=ds->res;res;res=resnxt){ resnxt=res->nxt; fsts_memput(&lat->resmem,res); }
  fsts_memput(&lat->dsmem,ds);
}

/* Lattice add resdiual function
 *
 * This function adds a new residual to a determined state.
 *
 * @param ds      Determined state
 * @param as      Active state of residual
 * @param w       Residual weight
 * @param wa      Total path weight
 * @param resmem  Memory for residuals
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_latdsaddres(struct fsts_latds *ds,struct fsts_latas *as,FLOAT64 w,FLOAT64 wa,struct fsts_mem *resmem){
  struct fsts_latres **resin=&ds->res;
  /* TODO: avl tree */
  while(*resin && resin[0]->as<as) resin=&resin[0]->nxt;
  if(!*resin || resin[0]->as!=as){
    struct fsts_latres *res=(struct fsts_latres*)fsts_memget(resmem);
    if(!res) return FSTSERR("out of memory");
    res->as=as;
    res->w=w;
    res->wa=wa;
    ds->nres++;
    res->nxt=*resin;
    *resin=res;
  }else if(w<resin[0]->w){
    resin[0]->w=w;
    resin[0]->wa=wa;
  }
  ds->w=MIN(ds->w,w);
  return NULL;
}

/* Lattice residuals hash function
 *
 * This function computes a hash key for
 * the residuals of a determined state.
 *
 * @param res   Pointer to residuals
 * @param woff  Weight offset to substract from residuals
 * @return      Hash key
 */
UINT64 fsts_lathash(struct fsts_latres *res,FLOAT64 woff){
  UINT64 hash=0;
  for(;res;res=res->nxt){
    res->w-=woff;
    hash=((hash<<5)|(hash>>(8*sizeof(UINT64)-5)))^(((UINT64)res->as)>>5)^((UINT64)res->w);
  }
  return hash;
}

/* Lattice determined state equal function
 *
 * This function tests if two determined states are equal
 * in residuals.
 *
 * @param dsa  Determined state #1
 * @param dsb  Determined state #2
 * @return     1 if they are equal, 0 otherwise
 */
char fsts_latdseql(struct fsts_latds *dsa,struct fsts_latds *dsb){
  struct fsts_latres *resa=dsa->res, *resb=dsb->res;
  if(dsa->hash!=dsb->hash || dsa->nres!=dsb->nres) return 0;
  while(resa && resb && resa->as==resb->as && resa->w==resb->w){ resa=resa->nxt; resb=resb->nxt; }
  return !resa && !resb;
}

/* Lattice determined state hash insertion function
 *
 * This function inserts a new determined state into
 * the hash of determined states or updates an exiting one.
 *
 * @param ds   Determined state
 * @param lat  Lattice storage structure
 */
struct fsts_latds *fsts_latdshins(struct fsts_latds *ds,struct fsts_lat *lat){
  struct fsts_latds  *dsh;
  INT32 ch;
  ds->hash=fsts_lathash(ds->res,ds->w);
  ch=ds->hash&HMASK;
  for(dsh=lat->hds[ch];dsh && !fsts_latdseql(dsh,ds);) dsh=dsh->hnxt;
  if(dsh){
    fsts_latdsfree(ds,lat);
    return dsh;
  }else{
    ds->hnxt=lat->hds[ch];
    lat->hds[ch]=ds;
    return ds;
  }
}

/* Lattice determined propagation function of an active state
 *
 * This function propagates an active state
 * until next output symbols are reached.
 *
 * @param as      Active state to propagate
 * @param w       Residual weight
 * @param wa      Total path weight
 * @param ds2     Output-Symbol-Hash for new determined states
 * @param lat     Lattice storage structure
 * @param ash     Propagate hash (holds all visited active states)
 * @param ashmem  Memory for propagate hash
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_latasprop(struct fsts_latas *as,FLOAT64 w,FLOAT64 wa,struct fsts_latds **ds2,struct fsts_lat *lat,struct fsts_latash **ash,struct fsts_mem *ashmem){
  struct fsts_latash *ash1;
  INT32 ashch;
  const char *err;
  if(!as) return NULL;
  ashch=(((UINT64)as)>>5)&ASHMASK;
  for(ash1=ash[ashch];ash1 && ash1->as!=as;) ash1=ash1->nxt;
  if(!ash1){
    if(!(ash1=(struct fsts_latash *)fsts_memget(ashmem))) return FSTSERR("out of memory");
    ash1->as=as;
    ash1->nxt=ash[ashch];
    ash[ashch]=ash1;
  }else if(w>=ash1->w) return NULL;
  ash1->w=w;
  if(as->pa && as->os<0){
    if((err=fsts_latasprop(as->pa,w,wa,ds2,lat,ash,ashmem))) return err;
    if(!lat->prn || wa+as->w<=lat->prn)
      if((err=fsts_latasprop(as->paw,w+as->w,wa+as->w,ds2,lat,ash,ashmem))) return err;
  }else{
    INT32 ch=as->pa?(as->os&OSMASK):OSMASK;
    struct fsts_latds *ds=ds2[ch];
    while(ds && ds->os!=as->os) ds=ds->qnxt;
    if(!ds){
      if(!(ds=fsts_latdsnew(&lat->dsmem))) return FSTSERR("out of memory");
      ds->os=as->os;
      ds->qnxt=ds2[ch];
      ds2[ch]=ds;
    }
    if((err=fsts_latdsaddres(ds,as->pa?as->pa:as,w,wa,&lat->resmem))) return err;
  }
  return NULL;
}

/* Lattice determined propagation function
 *
 * This function propagates one determined state
 * until the next output symbols.
 *
 * @param ds1  Determined state to propagate
 * @param ds2  Output-Symbol-Hash for new determined states
 * @param lat  Lattice storage structure
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_latdsprop(struct fsts_latds *ds1,struct fsts_latds **ds2,struct fsts_lat *lat){
  struct fsts_latres *res;
  struct fsts_mem ashmem;
  struct fsts_latash *ash[ASHMASK+1];
  INT32 i;
  const char *err;
  for(i=0;i<=OSMASK;i++) ds2[i]=NULL;
  for(i=0;i<=ASHMASK;i++) ash[i]=NULL;
  if((err=fsts_meminit(&ashmem,sizeof(struct fsts_latash),32768,0,0))) return err;
  for(res=ds1->res;res;res=res->nxt) if((err=fsts_latasprop(res->as,res->w,res->wa,ds2,lat,ash,&ashmem))) return err;
  fsts_memfree(&ashmem);
  return NULL;
}

/* Lattice determined state print function
 *
 * This function prints one determined state and it's residuals.
 * Enable with macro OPT_PRT.
 *
 * @param pre  Prefix string
 * @param ds   Determined state
 * @param lat  Lattice storage structure
 */
/*#define OPT_PRT*/
void fsts_latdsprt(const char *pre,struct fsts_latds *ds,struct fsts_lat *lat){
  #ifdef OPT_PRT
  struct fsts_latres *res=ds->res;
  printf("%s o%i s%2i w%02.0f n%i",pre,ds->os,ds->s,ds->w,ds->nres);
  for(;res;res=res->nxt) printf(" a%04x w%2.0f",(int)(((UINT64)res->as-(UINT64)lat->asmem.buf)&0xffff),res->w);
  printf("\n");
#endif
}

/* Lattice backtrack function without determinization
 *
 * This function exports a subtree of the non-determined lattice to itDst.
 *
 * @param as     Final active state of subtree
 * @param lat    Lattice storage structure
 * @param itDst  Destination transducer
 * @param ui     Destination unit index
 * @param st     Terminal state index of subtree
 * @param os     Output symbol of transitions to st
 * @param w      Residual path weight
 */
void fsts_latbt1(struct fsts_latas *as,struct fsts_lat *lat,CFst *itDst,INT32 ui,INT32 st,INT32 os,FLOAT64 w){
  if(!as) return;
  if(as->s>=0) CFst_AddtransEx(itDst,ui,as->s,st,-1,os,w);
  else{
    as->s=UD_XS(itDst,ui);
    CFst_Addstates(itDst,ui,1,FALSE);
    CFst_AddtransEx(itDst,ui,as->s,st,-1,os,w);
    fsts_latbt1(as->pa,lat,itDst,ui,as->s,as->os,0.);
    fsts_latbt1(as->paw,lat,itDst,ui,as->s,-1,as->w);
  }
}

/* Lattice backtrack function
 *
 * This function exports the lattice to itDst.
 * Macro DET enables determinization.
 *
 * @param as     Final active state
 * @param lat    Lattice storage structure
 * @param itDst  Destination transducer
 * @param ui     Destination unit index
 * @param w      Total best path weight
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_latbt(struct fsts_latas *as,struct fsts_lat *lat,CFst *itDst,INT32 ui,FLOAT64 w){
  const char *err=NULL;
  CFst_Addstates(itDst,ui,1,FALSE);
  CFst_Addstates(itDst,ui,1,TRUE);
#define DET
#ifndef DET
  fsts_latbt1(as->pa,lat,itDst,ui,1,as->os,w);
  fsts_latbt1(as->paw,lat,itDst,ui,1,-1,w+as->w);
#else
  if(!(lat->hds=(struct fsts_latds**)calloc(HMASK+1,sizeof(struct fsts_latds*)))) return FSTSERR("out of memory");
  lat->qds=fsts_latdsnew(&lat->dsmem);
  lat->qds->s=1;
  fsts_latdsaddres(lat->qds,as,w,0.,&lat->resmem);
  struct fsts_latds *ds=fsts_latdsnew(&lat->dsmem);
  ds->s=0;
  fsts_latdsaddres(ds,lat->head,0.,0.,&lat->resmem);
  fsts_latdshins(ds,lat);
  while(lat->qds){
    struct fsts_latds *ds1=lat->qds,*ds2[OSMASK+1],*ds,*dsnxt;
    INT32 osc;
    lat->qds=ds1->qnxt;
    fsts_latdsprt("ds: ",ds1,lat);
    if((err=fsts_latdsprop(ds1,ds2,lat))) goto end;
    for(osc=0;osc<=OSMASK;osc++) for(ds=ds2[osc];ds;ds=dsnxt){
      INT32 os=ds->os;
      FLOAT64 w=ds->w;
      dsnxt=ds->qnxt;
      fsts_latdsprt("  =>",ds,lat);
      ds=fsts_latdshins(ds,lat);
      if(ds->s<0){
        ds->s=UD_XS(itDst,ui);
        #ifdef OPT_PRT
        printf("  -> s%i (new)\n",ds->s);
        #endif
        CFst_Addstates(itDst,ui,1,FALSE);
        ds->qnxt=lat->qds;
        lat->qds=ds;
      }
      #ifdef OPT_PRT
      else printf("  -> s%i\n",ds->s);
      #endif
      CFst_AddtransEx(itDst,ui,ds->s,ds1->s,-1,os,w);
    }
  }
end:
  free(lat->hds);
#endif
  return err;
}
