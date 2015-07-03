/* dLabPro class CFstsearch (fstsearch)
 * - Backtracking header
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
#ifndef _FSTS_BT_H
#define _FSTS_BT_H

/* Enable free of unused backtrack elements */
#define OPT_BTFREE

/* Backtrack memory */
struct fsts_btm {
  struct fsts_mem ti;  /* Transition index memory    */
  struct fsts_mem os;  /* Output symbols memory      */
  struct fsts_lat lat; /* Lattice storage structure  */
};

/* Backtrack state node */
struct fsts_bts {
  struct fsts_bt    *ti;  /* Transition index object    */
  struct fsts_bt    *os;  /* Output symbol object       */
  struct fsts_latas *lat; /* Lattice information object */
};

/* Backtrack node */
struct fsts_bt {
  INT32 tid;          /* Transition id or output symbol         */
  UINT32 ref;         /* Reference counter                      */
  FLOAT64 w;          /* Current path weight                    */
  struct fsts_bt *pa; /* Pointer to the previous backtrack info */
  /* optional: INT32 hash;        (only for osmem and numpaths>1)  */ /* Hash value of the history for fast comparision    */
  /* optional: MUTEXHANDLE mutex; (only for jobs>1 and mem->lock)  */ /* Mutex for thread critical actions                 */
  /* optional: void* delaynxt;    (only for osmem and numpaths>1)  */ /* For delayed free: pointer to next pending element */
};

/* Backtrack working information object */
struct fsts_btinfo {
  struct fsts_cfg *cfg;     /* Configuration                */
  struct fsts_btm *btm;     /* Backtrack memory             */
  UINT32 ui,ti;             /* Unit+transition index        */
  INT32 gwi;                /* Global weight component      */
  CFst *itDst;              /* Destination transducer       */
  struct fsts_fst *src;     /* Source transducer            */
  INT32 rlt,ctis,ctos,clsr; /* Transition component indices */
};

/* Marko reading the hash value of the history */
#define BTHASH(bt)      (*(UINT32*)((bt)+1))
/* Marko reading the mutex */
#define BTMUTEX(bt,mem) ((MUTEXHANDLE*)(((BYTE*)(bt))+(mem)->size-((mem)->delay?sizeof(void*):0)-sizeof(MUTEXHANDLE)))

UINT8 fsts_bteql(struct fsts_bt *a,struct fsts_bt *b);

void fsts_btsfree(struct fsts_bts *bt,struct fsts_bts *btref,FLOAT64 wd,struct fsts_btm *btm);
const char *fsts_btsgen(struct fsts_bts *bt,struct fsts_bts *btref,struct fsts_t *t,UINT8 ui0,FLOAT64 w,struct fsts_btm *btm);

/* Return maximal amount of memory used for decoding */
#define fsts_btmmem1(btm)   fsts_memmem(&(btm)->ti) + fsts_memmem(&(btm)->os) + fsts_latmem1(&(btm)->lat)
/* Return maximal amount of memory used for backtracking */
#define fsts_btmmem2(btm)   fsts_latmem2(&(btm)->lat)

const char *fsts_btm_init(struct fsts_btm *btm,struct fsts_cfg *cfg);
void fsts_btm_free(struct fsts_btm *btm);
const char *fsts_btmdbg(struct fsts_btm *btm,UINT8 t);

const char *fsts_btstart(struct fsts_btinfo *bti,struct fsts_glob *glob,struct fsts_btm *btm,CFst *itDst);
const char *fsts_btpath(struct fsts_btinfo *bti,FLOAT64 w,struct fsts_bts *bts);

#endif
