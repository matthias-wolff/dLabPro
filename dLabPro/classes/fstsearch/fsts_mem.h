/* dLabPro class CFstsearch (fstsearch)
 * - Memory management header
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
#ifndef _FSTS_MEM_H
#define _FSTS_MEM_H

/* Fast internal memory structure */
struct fsts_mem {
  UINT32 size;        /* Size of one element */
  UINT32 num;         /* Number of elements  */
  UINT32 addnum;      /* Number of elements to add on resize */
  UINT32 used;        /* Number of elements currently used   */
  UINT32 usedmax;     /* Maximal number of used elements */
  UINT32 uninit;      /* Number of elements currently not initialized */
  UINT8  delay;       /* Switch if deallocation should be delay and performend by fsts_memputdelay */
  UINT8  lock;        /* Switch if a mutex should be locked before all critical actions */
  MUTEXHANDLE mutex;  /* The mutex */
  void  *delayed;     /* Connected list of delayed elements (see MEMNXT) */
  void **free;        /* Array of free elements */
  void  *buf;         /* Connected list of element memories (starts with pointer to the next memory) */
};

/* Return maximal amount of memory used */
#define fsts_memmem(mem)  ((mem)->usedmax*((mem)->size+sizeof(void*)))

const char *fsts_meminit(struct fsts_mem *mem,UINT32 size,UINT32 num,UINT8 delay,UINT8 lock);
const char *fsts_memresize(struct fsts_mem *mem);
void fsts_memfree(struct fsts_mem *mem);
void *fsts_memget(struct fsts_mem *mem);
void fsts_memput(struct fsts_mem *mem,void *buf);
void fsts_memputdelay(struct fsts_mem *mem);
UINT32 fsts_memsize(struct fsts_mem *mem);

#endif
