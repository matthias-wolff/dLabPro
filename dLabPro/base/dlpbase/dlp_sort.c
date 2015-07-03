/* dLabPro base library
 * - Sort algorithm
 *
 * AUTHOR : Frank Duckhorn
 * PACKAGE: dLabPro/base
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

#include "dlp_kernel.h"
#include "dlp_base.h"

hash_t *dlpsort_hash=NULL;
int(*dlpsort_cmp)(const void *, const void *);
size_t dlpsort_size;
size_t dlpsort_keysize;

#define KEYSIZE 64
static const void *dlpsort_keyfnc(const void *key,unsigned char *buf)
{
  static unsigned char statbuf[KEYSIZE+1];
  const unsigned char *str = key;
  size_t i;
  if(!buf) buf=statbuf;
  memset(buf,0,dlpsort_keysize+1);
  for(i=0;i<dlpsort_size;i++){
    buf[i%dlpsort_keysize]+=str[i];
  }
  return (const void *)buf;
}

int dlpsort_cmpseq(const void *a, const void *b){
  int ret=(*dlpsort_cmp)(a,b);
  if(!ret){
    hnode_t *ha=hash_lookup(dlpsort_hash,dlpsort_keyfnc(a,NULL));
    hnode_t *hb=hash_lookup(dlpsort_hash,dlpsort_keyfnc(b,NULL));
    if(ha && hb){
      size_t idxa=(size_t)hnode_get(ha);
      size_t idxb=(size_t)hnode_get(hb);
      if(idxa<idxb) ret=-1;
      if(idxa>idxb) ret=1;
    }
  }
  return ret;
}

static int dlpsort_hashcmpfnc(const void *key1, const void *key2, void *context)
{
  return memcmp(key1,key2,dlpsort_keysize);
}

/*
 * This funktion makes a stable sorting algorithm based on qsort.
 * The order of equal elements is retained.
 * This is done by a hash map which references the old element indicies.
 * Warning: if no memory can be allocated the algorithm will fall back to unstable qsort.
 * Warning: in case of the hash collision the sorting algorithm is not stable any more.
 * Warning: the sorting algorithm is not thread secure if called it should fall back to unstable qsort.
 * TODO: someone should implement an own stable sorting algorithm.
 */
void dlpsort(void *base, size_t nmemb, size_t size, int(*compar)(const void *, const void *))
{
  size_t idx;
  unsigned char *keybuf;
  if(dlpsort_hash) goto fallback;
  if(!(dlpsort_hash=hash_create(HASHCOUNT_T_MAX,dlpsort_hashcmpfnc,NULL,NULL))) goto fallback;
  dlpsort_cmp=compar;
  dlpsort_size=size;
  dlpsort_keysize=MIN(size,KEYSIZE);
  if(!(keybuf=dlp_malloc((dlpsort_keysize+1)*nmemb))) goto fallback;
  for(idx=0;idx<nmemb;idx++){
    const void *key=dlpsort_keyfnc(base+idx*size,keybuf+(dlpsort_keysize+1)*idx);
    hnode_t *h=hash_lookup(dlpsort_hash,key);
    if(!h) hash_alloc_insert(dlpsort_hash,key,(void *)idx);
  }
  qsort(base,nmemb,size,dlpsort_cmpseq);
  hash_free_nodes(dlpsort_hash);
  hash_destroy(dlpsort_hash);
  dlpsort_hash=NULL;
  dlp_free(keybuf);
  return;
fallback:
  if(dlpsort_hash){
    hash_destroy(dlpsort_hash);
    dlpsort_hash=NULL;
  }
  qsort(base,nmemb,size,compar);
}
