/* dLabPro class CFstsearch (fstsearch)
 * - A* closed map header
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
#ifndef _FSTS_AS_MAP_H
#define _FSTS_AS_MAP_H

/* A* state done map */
struct fsts_as_map {
  UINT32 n;  /* Size of m in bytes */
  UINT8 *m;  /* Map pointer */
};

/* OPT_BYTE uses one byte per state,
 * otherwise only one bit is used */
/* #define OPT_BYTE */
#ifdef OPT_BYTE

#define fsts_as_mapinit(map,num) { \
  (map).n=(num); \
  if(!((map).m=(UINT8*)calloc((map).n,sizeof(UINT8)))) return FSTSERR("out of memory"); \
}

#define fsts_as_mapget(map,id) ((map).m[(id)])
#define fsts_as_mapon(map,id)  ((map).m[(id)]=1)

#else

#define fsts_as_mapinit(map,num) { \
  (map).n=((num)>>3)+1; \
  if(!((map).m=(UINT8*)calloc((map).n,sizeof(UINT8)))) return FSTSERR("out of memory"); \
}

#define fsts_as_mapget(map,id) ((map).m[(id)>>3]&(0x1<<((id)&0x7)))
#define fsts_as_mapon(map,id)  ((map).m[(id)>>3]|=0x1<<((id)&0x7))

#endif

#define fsts_as_mapfree(map)  free((map).m)
#define fsts_as_mapsize(map)  ((map).n)

/* Return maximal amount of memory used */
#define fsts_as_mapmem(map)   ((map).n)

#endif
