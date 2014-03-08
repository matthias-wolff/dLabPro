/* dLabPro class CFstsearch (fstsearch)
 * - TP global header
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
#ifndef _FSTS_TP_ALGO_H
#define _FSTS_TP_ALGO_H

struct fsts_tp_algo;

#include "fsts_tp_s.h"
#include "fsts_tp_hist.h"
#include "fsts_tp_ls.h"

/* TP internal memory structure */
struct fsts_tp_algo {
  INT32  f;                      /* Current frame index        */
  struct fsts_tp_ls ls1, ls2;    /* Active state queues        */
  struct fsts_btm btm;           /* Backtrack memory           */
  INT64  nstates;                /* Number of expanded states  */
  struct fsts_tp_job *jobs;      /* Job specific memories      */
  MUTEXHANDLE mutex;             /* Mutex for job termination  */
  CONDHANDLE  cond;              /* Signal for job termination */
  #ifdef _DEBUG
  UINT32 nlsmax;
  UINT32 hmax;
  #endif
};

/* TP job internal memory */
struct fsts_tp_job {
  INT32             jid;      /* Job id                        */
  struct fsts_tp_ls ls1, ls2; /* Active state queues           */
  struct fsts_glob *glob;     /* Global memory pointer         */
  struct fsts_w    *w;        /* Timevariant weight array      */
  THREADHANDLE      tid;      /* Thread handle                 */
  const char       *ret;      /* Error return string           */
  UINT8             fin;      /* Job finished flag             */
  UINT8             run;      /* Job running flag              */
  struct fsts_mem   smem;     /* Active state memory           */
  MUTEXHANDLE       mutex;    /* Mutex for job running         */
  CONDHANDLE        cond;     /* Signal for job running        */
  struct fsts_tp_js *js;      /* List of pending active states */
  INT32             jshf;     /* State index offset            */
  FLOAT64           wprn;     /* Current pruning threshold     */
};

/* Pending active state */
struct fsts_tp_js {
  struct fsts_tp_js *nxt; /* Next pending active state */
  UINT8 ls;               /* Queue index               */
  struct fsts_tp_s s;     /* Active state              */
};

#endif
