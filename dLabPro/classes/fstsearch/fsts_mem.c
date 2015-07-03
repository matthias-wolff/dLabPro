/* dLabPro class CFstsearch (fstsearch)
 * - Memory management
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

/* Get the next delayed element */
#define MEMNXT(mem,buf) (*(void**)(((BYTE*)(buf))+(mem)->size-sizeof(void*)))

/* Init memory structure
 *
 * This function initializes the memory structure.
 *
 * @param mem   Pointer to the memory
 * @param size  Size of one element
 * @param num   Number of elements for initialization
 * @param delay If TRUE the deallocation of elements is delayed and performed by fsts_memputdelay
 * @param lock  If TRUE a mutex is locked before all critical actions
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_meminit(struct fsts_mem *mem,UINT32 size,UINT32 num,UINT8 delay,UINT8 lock){
  mem->size=size;
  mem->num=0;
  mem->used=0;
  mem->usedmax=0;
  mem->uninit=0;
  mem->free=NULL;
  mem->buf=NULL;
  if((mem->delay=delay)) mem->size+=sizeof(void*);
  mem->delayed=NULL;
  if((mem->lock=lock) && dlp_create_mutex(&mem->mutex)!=O_K) return FSTSERR("create mutex failed");
  mem->addnum=num;
  return fsts_memresize(mem);
};

/* Memory resize function
 *
 * This function increases the number of elements in the memory.
 *
 * @param mem   Pointer to the memory
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fsts_memresize(struct fsts_mem *mem){
  void *buf;
  if(!mem->addnum) return FSTSERR("out of internal memory");
  if(mem->num>=mem->addnum*4) mem->addnum*=2;
  mem->num+=mem->addnum;
  if(!(mem->free=(void**)realloc(mem->free,mem->num*sizeof(void*)))) goto fail;
  if(!(buf=malloc(sizeof(void*)+mem->addnum*mem->size))) goto fail;
  mem->uninit=mem->addnum;
  ((void**)buf)[0]=mem->buf;
  mem->buf=buf;
  return NULL;
fail:
  fsts_memfree(mem);
  return FSTSERR("out of memory");
}

/* Memory free function
 *
 * This function free's the memory with all it's elements.
 *
 * @param mem   Pointer to the memory
 */
void fsts_memfree(struct fsts_mem *mem){
  void *buf;
  if(mem->lock) dlp_destroy_mutex(&mem->mutex);
  buf=mem->buf;
  while(buf){
    void *buf2=((void**)buf)[0];
    free(buf);
    buf=buf2;
  }
  mem->buf=NULL;
  if(mem->free) free(mem->free);
  mem->num=0;
  mem->used=0;
}

/* Memory get function
 *
 * This function get's one element from the memory for usage.
 *
 * @param mem   Pointer to the memory
 * @return  Pointer to the element or <code>NULL</code> on error
 */
void *fsts_memget(struct fsts_mem *mem){
  void *ret;
  if(mem->lock && dlp_lock_mutex(&mem->mutex)!=O_K) return NULL;
  if(mem->used>=mem->num && fsts_memresize(mem)) return NULL;
#ifdef _DEBUG
  if(mem->used>=mem->num) abort();
#endif
  if(mem->used+1>mem->usedmax) mem->usedmax=mem->used+1;
  if(mem->num-mem->used>mem->uninit) ret=mem->free[mem->used++];
  else{
    mem->used++;
    ret=(void*)((BYTE*)mem->buf+sizeof(void*)+(--mem->uninit)*mem->size);
  }
  if(mem->lock) dlp_unlock_mutex(&mem->mutex);
  return ret;
}

/* Memory put function
 *
 * This function put's one element back to the memory after usage.
 *
 * @param mem   Pointer to the memory
 * @param buf   Pointer to the element
 */
void fsts_memput(struct fsts_mem *mem,void *buf){
  if(mem->lock && dlp_lock_mutex(&mem->mutex)!=O_K) return;
  if(mem->delay){
    MEMNXT(mem,buf)=mem->delayed;
    mem->delayed=buf;
  }else{
#ifdef _DEBUG
    if(!mem->used) abort();
#endif
    mem->free[--mem->used]=buf;
  }
  if(mem->lock) dlp_unlock_mutex(&mem->mutex);
}

/* Memory put delayed function
 *
 * This function put's all delayed elements back to the memory.
 *
 * @param mem   Pointer to the memory
 */
void fsts_memputdelay(struct fsts_mem *mem){
  void *buf;
  if(!mem->delayed) return;
  for(buf=mem->delayed;buf;buf=MEMNXT(mem,buf)){
#ifdef _DEBUG
    if(!mem->used) abort();
#endif
    mem->free[--mem->used]=buf;
  }
  mem->delayed=NULL;
}

/* Memory size function
 *
 * This function returns the total size of the memory.
 *
 * @param mem   Pointer to the memory
 * @return      Memory size used
 */
UINT32 fsts_memsize(struct fsts_mem *mem){
  return mem->num*(mem->size+sizeof(void*));
}
