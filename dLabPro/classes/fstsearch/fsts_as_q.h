/* dLabPro class CFstsearch (fstsearch)
 * - A* queue header
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
#ifndef _FSTS_AS_Q_H
#define _FSTS_AS_Q_H

/* A* active state queu */
struct fsts_as_q {
  UINT32  len;          /* Queue size                      */
  UINT32  used;         /* Number of states in the queue   */
  UINT32  usedmax;      /* Maximal used elements           */
  UINT8   prn;          /* 0: Queue resize, 1: Queue prune */
  FLOAT64 wmax;         /* Current maximal weight          */
  #ifdef OPT_AVL
  struct fsts_mem    qsmem;
  struct fsts_as_qs *head;
  struct fsts_as_qs *first;
  struct fsts_as_qs *last;
  #else
  struct fsts_as_s **s; /* Active state memory             */
  #endif
};

#ifdef OPT_AVL
struct fsts_as_qs {
  struct fsts_as_s  *s;
  struct fsts_as_qs *pa;
  struct fsts_as_qs *ch[2];
  struct fsts_as_qs *nxt,*prv;
  INT32              h;
};
#endif

/* Return maximal amount of memory used */
#ifndef OPT_AVL
  #define fsts_as_qmem(q) ((q)->usedmax*sizeof(struct fsts_as_s*))
#else
  #define fsts_as_qmem(q) fsts_memmem(&(q)->qsmem)
#endif

const char *fsts_as_qinit(struct fsts_as_q *q,UINT32 len);
void fsts_as_qfree(struct fsts_as_q *q);
const char *fsts_as_qdbg(struct fsts_as_q *q,UINT8 t);
#ifdef OPT_AVL
const char *fsts_as_qrpl(struct fsts_as_q *q,struct fsts_as_s *s,struct fsts_as_qs *qs);
struct fsts_as_s *fsts_as_qpop(struct fsts_as_q *q,struct fsts_as_qs *qs);
#else
const char *fsts_as_qrpl(struct fsts_as_q *q,struct fsts_as_s *s,UINT32 p);
struct fsts_as_s *fsts_as_qpop(struct fsts_as_q *q,UINT32 p);
#endif

#endif
