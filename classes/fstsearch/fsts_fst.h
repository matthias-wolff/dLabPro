/* dLabPro class CFstsearch (fstsearch)
 * - Internal transducers header
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
#ifndef _FSTS_FST_H
#define _FSTS_FST_H

#include "dlp_fst.h"

/* Internal transition structure */
struct fsts_t {
  UINT32 ini,ter;        /* Initial and terminal state */
  INT32 is,os;           /* Input and output symbol    */
  INT32 stk;             /* Pushdown symbol            */
  INT32 id;              /* Transition index (in orig. source) */
  FLOAT64 w;             /* Transition weight          */
  struct fsts_t *nxt;    /* Next transition with the same inital state */
  struct fsts_t *prv;    /* Next transition with the same terminal state */
};

/* Internal unit structure */
struct fsts_unit {
  UINT8 sub;              /* Lowest layer indicator for on-the-fly composition */
  UINT32 ns;              /* Number of states */
  FLOAT64 pot0;
  struct fsts_t **tfroms; /* Transition array by initial state  (see fsts_t.nxt) */
  struct fsts_t **ttos;   /* Transition array by terminal state (see fsts_t.prv) */
  UINT8 *sfin;            /* Array indicating the final states  */
};

/* Internal transducer structure */
struct fsts_fst {
  INT32 nunits;            /* Number of units */
  INT32 maxstk;            /* Highest stk symbol index */
  struct fsts_unit *units; /* Unit array      */
  CFst *itSrc;             /* A copy of the original source transducer or NULL */
};

const char *fsts_load(struct fsts_fst *src,CFst *itSrc,INT32 uid,UINT8 fast);
const char *fsts_unload(struct fsts_fst *src);
const char *fsts_fstpw(struct fsts_fst *src);

#endif
