/* dLabPro class CFstsearch (fstsearch)
 * - Search state hash header
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
#ifndef _FSTS_H_H
#define _FSTS_H_H

/* This macro gets the array of active states form the hash element */
#define fsts_hs2s(hs)      ((void *)((hs)+1))
/* This macro gets the i'th active state form the hash element */
#define fsts_hs2si(h,hs,i) ((void *)(((BYTE *)((hs)+1))+(h)->ssize*(i)))

/* Hash element */
struct fsts_hs {
  uint64_t id;         /* State id                                 */
  UINT16   use;        /* Number of visited states in this element */
  UINT16   done;       /* Number of deactivaed states              */
  struct fsts_hs *nxt; /* Next hash element in this chain          */
  struct fsts_hs *prv; /* Previous hash element in this chain      */
  /* dynamic: struct fsts_*_s s[glob->cfg.numpaths] */ /* Active states array */
};

/* Visitied state hash memory */
struct fsts_h {
  UINT32 ssize;        /* Size of one state                   */
  UINT16 snum;         /* Number of states per element        */
  UINT32 mask;         /* Hash mask / Number of chains - 1    */
  UINT8 (*cmp)(void *a,void *b); /* State compare function    */
  struct fsts_hs **hs; /* Hash chain array                    */
  struct fsts_mem mem; /* Hash element memory                 */
  void **schg[2];      /* State pointers to update in pruning */
};

/* Return maximal amount of memory used */
#define fsts_hmem(h)  fsts_memmem(&(h)->mem) /* TODO: chain memory used */

const char *fsts_hinit(struct fsts_h *h,UINT32 ssize,UINT16 snum,UINT8 (*cmp)(void *a,void *b));
void fsts_hfree(struct fsts_h *h);
const char *fsts_hdbg(struct fsts_h *h,UINT8 t);
struct fsts_hs *fsts_hfind(struct fsts_h *h,void *s);
void *fsts_hins(struct fsts_h *h,void *s);
struct fsts_hs *fsts_hfins(struct fsts_h *h,void *s,UINT8 prn);
void *fsts_hdels(struct fsts_h *h,void *s);

#endif
