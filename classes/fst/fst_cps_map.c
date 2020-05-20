/* dLabPro class CFst (fst)
 * - Composition - Fast internal memory structure
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

#ifdef _CPS_MAPHASH_
/* Fast internal done map structure */

#define HASHBITS  25
#define HASHXCH   (1<<20)

struct fstc_mch {
  struct fstc_mch *prv;
  uint64_t used;
  uint64_t i[HASHXCH];
};

struct fstc_m {
  uint64_t na,ne,nb;
  struct fstc_mch **chs;
};

const char *mapinit(struct fstc_m *m,uint64_t na,uint64_t ne,uint64_t nb){
  uint64_t ch;
  m->na=na;
  m->ne=ne;
  m->nb=nb;
  if(!(m->chs=(struct fstc_mch**)malloc(sizeof(struct fstc_mch*)*(1<<HASHBITS)))) return "Out of memory";
  for(ch=0;ch<(1<<HASHBITS);ch++) m->chs[ch]=NULL;
  return NULL;
}
void mapfree(struct fstc_m *m){
  uint64_t ch;
  for(ch=0;ch<(1<<HASHBITS);ch++){
    while(m->chs[ch]){
      struct fstc_mch *c=m->chs[ch];
      m->chs[ch]=c->prv;
      free(c);
    }
  }
  free(m->chs);
  m->chs=NULL;
}

#define MAPID(m,a,e,b)  (((a)*(m)->ne+(e))*(m)->nb+(b))
static uint64_t mapch(uint64_t i){
  uint64_t x=1<<HASHBITS;
  uint64_t ch=0;
  while(i){ ch^=i&(x-1); i>>=HASHBITS; }
  return ch;
}
char mapget(struct fstc_m *m,uint64_t a,uint64_t e,uint64_t b){
  uint64_t i=MAPID(m,a,e,b);
  uint64_t ch=mapch(i);
  struct fstc_mch *c=m->chs[ch];
  for(;c;c=c->prv){
    uint64_t cu=c->used, *ci=c->i;
    for(;cu;cu--,ci++) if(*ci==i) return 1;
  }
  return 0;
}

const char *mapset(struct fstc_m *m,uint64_t a,uint64_t e,uint64_t b){
  uint64_t i=MAPID(m,a,e,b);
  uint64_t ch=mapch(i);
  struct fstc_mch *c;
  for(c=m->chs[ch];c;c=c->prv){
    uint64_t cu=c->used, *ci=c->i;
    for(;cu;cu--,ci++) if(*ci==i) return NULL;
  }
  c=m->chs[ch];
  if(!c || c->used==HASHXCH){
    if(!(c=(struct fstc_mch*)malloc(sizeof(struct fstc_mch)))) return "Out of memory";
    c->used=0;
    c->prv=m->chs[ch];
    m->chs[ch]=c;
  }
  c->i[c->used++]=i;
  return NULL;
}

#define MAPINIT(m,na,ne,nb) if((err=mapinit(&(m),na,ne,nb))) return err;
#define MAPFREE(m)          mapfree(&(m));
#define MAPGET(m,a,e,b)     mapget(&(m),a,e,b)
#define MAPSET(m,a,e,b)     if((err=mapset(&(m),a,e,b))) return err;

#endif
