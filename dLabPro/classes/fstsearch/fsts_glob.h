/* dLabPro class CFstsearch (fstsearch)
 * - Global search header
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

#include "dlp_cscope.h" /* Indicate C scope */
#include "dlp_fstsearch.h"

struct fsts_glob;
struct fsts_cfg;

#include "fsts_mem.h"
#include "fsts_w.h"
#include "fsts_as.h"
#include "fsts_tp.h"
#include "fsts_sdp.h"
#include "fsts_cfg.h"
#include "fsts_fst.h"
#include "fsts_lat.h"
#include "fsts_bt.h"
#include "fsts_h.h"

#ifndef _FSTS_GLOB_H
#define _FSTS_GLOB_H

/* Maximal number of layers for on-the-fly composition */
#define MAXLAYER  4
/* Maximal size of the pushdown stack memory */
#define MAXSTK    8

/* Search states */
enum fsts_state {FS_FREE=0, FS_BEGIN, FS_SEARCHING, FS_END };

/* Redefine dlabpro's alloc function for allocation without _this instance */
#undef dlp_malloc
#undef dlp_calloc
#undef dlp_realloc
#define dlp_malloc(A)    __dlp_malloc(A,     __FILE__,__LINE__,"fstsearch",NULL)
#define dlp_calloc(A,B)  __dlp_calloc(A,B,   __FILE__,__LINE__,"fstsearch",NULL)
#define dlp_realloc(A,B) __dlp_realloc(A,1,B,__FILE__,__LINE__,"fstsearch",NULL)
#ifdef _DEBUG
  /* Redefine C alloc function to use dlabpro's xalloc in DEBUG mode */
  #undef malloc
  #undef calloc
  #undef realloc
  #undef free
  #define malloc(A)    __dlp_malloc(A,     __FILE__,__LINE__,"fstsearch",NULL)
  #define calloc(A,B)  __dlp_calloc(A,B,   __FILE__,__LINE__,"fstsearch",NULL)
  #define realloc(A,B) __dlp_realloc(A,1,B,__FILE__,__LINE__,"fstsearch",NULL)
  #define free(A)      dlp_free(A)
#endif

/* These error macro produces a string containing file name and line number */
#define STRINGIFY2(X) #X
#define STRINGIFY1(X) STRINGIFY2(X)
#define FSTSERR(TXT)  __FILE__ "(" STRINGIFY1(__LINE__) "): " TXT

#define FSTSMB(N)   (((N)+512*1024)/1024/1024)

/* Internal global memory structure */
struct fsts_glob {
  UINT8 debug;           /* Debug level = fstsearch.check */
  enum fsts_state state; /* Search state */
  struct fsts_fst src;   /* Source transducer */
  struct fsts_cfg cfg;   /* Configuration */
  FLOAT64 mem;           /* Memory used in decoding */
  void *algo;            /* Algorithm specific memory */
};

#endif
