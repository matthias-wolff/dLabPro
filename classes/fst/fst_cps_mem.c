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

/* Fast internal memory structure */
struct fstc_mem {
  UINT32 size;        /* Size of one element */
  UINT32 num;         /* Number of elements  */
  UINT32 addnum;      /* Number of elements to add on resize */
  UINT32 used;        /* Number of elements currently used   */
  UINT32 uninit;      /* Number of elements currently not initialized */
  void **free;        /* Array of free elements */
  void  *buf;         /* Connected list of element memories (starts with pointer to the next memory) */
#ifdef _DEBUG
  UINT32 usedmax;     /* Maximal number of used elements */
#endif
};

/* Memory free function
 *
 * This function free's the memory with all it's elements.
 *
 * @param mem   Pointer to the memory
 */
void fstc_memfree(struct fstc_mem *mem){
  void *buf;
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

/* Memory resize function
 *
 * This function increases the number of elements in the memory.
 *
 * @param mem   Pointer to the memory
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fstc_memresize(struct fstc_mem *mem){
  void *buf;
  if(!mem->addnum) return FSTCERR("out of internal memory");
  if(mem->num>=mem->addnum*4) mem->addnum*=2;
  mem->num+=mem->addnum;
  if(!(mem->free=(void**)realloc(mem->free,mem->num*sizeof(void*)))) goto fail;
  if(!(buf=malloc(sizeof(void*)+mem->addnum*mem->size))) goto fail;
  mem->uninit=mem->addnum;
  ((void**)buf)[0]=mem->buf;
  mem->buf=buf;
  return NULL;
fail:
  fstc_memfree(mem);
  return FSTCERR("out of memory");
}

/* Init memory structure
 *
 * This function initializes the memory structure.
 *
 * @param mem   Pointer to the memory
 * @param size  Size of one element
 * @param num   Number of elements for initialization
 * @return <code>NULL</code> if successfull, the error string otherwise
 */
const char *fstc_meminit(struct fstc_mem *mem,UINT32 size,UINT32 num){
  mem->size=size;
  mem->num=0;
  mem->used=0;
  mem->uninit=0;
  mem->free=NULL;
  mem->buf=NULL;
#ifdef _DEBUG
  mem->usedmax=0;
#endif
  mem->addnum=num;
  return fstc_memresize(mem);
};

/* Memory get function
 *
 * This function get's one element from the memory for usage.
 *
 * @param mem   Pointer to the memory
 * @return  Pointer to the element or <code>NULL</code> on error
 */
void *fstc_memget(struct fstc_mem *mem){
  void *ret;
  if(mem->used>=mem->num && fstc_memresize(mem)) return NULL;
#ifdef _DEBUG
  if(mem->used>=mem->num) abort();
  if(mem->used+1>mem->usedmax) mem->usedmax=mem->used+1;
#endif
  if(mem->num-mem->used>mem->uninit) ret=mem->free[mem->used++];
  else{
    mem->used++;
    ret=(void*)((BYTE*)mem->buf+sizeof(void*)+(--mem->uninit)*mem->size);
  }
  return ret;
}

/* Memory put function
 *
 * This function put's one element back to the memory after usage.
 *
 * @param mem   Pointer to the memory
 * @param buf   Pointer to the element
 */
void fstc_memput(struct fstc_mem *mem,void *buf){
#ifdef _DEBUG
  if(!mem->used) abort();
#endif
  mem->free[--mem->used]=buf;
}

/* Memory size function
 *
 * This function returns the total size of the memory.
 *
 * @param mem   Pointer to the memory
 * @return      Memory size used
 */
UINT32 fstc_memsize(struct fstc_mem *mem){
  return mem->num*(mem->size+sizeof(void*));
}
