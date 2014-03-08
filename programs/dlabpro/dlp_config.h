/* dLabPro program dLabPro (dlabpro)
 * - Build configuration
 *
 * AUTHOR : Matthias Wolff
 * PACKAGE: dLabPro/programs
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

#ifndef __DLP_CONFIG_H
#define __DLP_CONFIG_H

/*#define __NOITP                   / * Do not register methods with object 1)       */
/*#define __NORTTI                  / * Switch runtime type info off 1)              */
/*#define __NOXMLSTREAM             / * Do not use XML serialization (CXmlStream)    */
/*#define __NODN3STREAM             / * Do not use DNorm3 serialization (CDn3Stream) */
/*#define __NOZLIB                  / * Do not use file compression (zlib)           */
/*#define __UNENTANGLE_FST          / * Stand alone class CFst                       */
/*#define __NOREADLINE              / * Do not use readline library                  */
/*#define __DEFAULT_FILEFORMAT_DN3  / * Set default file format to dn3               */
#define __DEFAULT_FILEFORMAT_XML  /* Set default file format to xml                  */
/*#define __NOLIBSNDFILE            / * Do not use Libsndfile library                */
/*#define __MAX_TYPE_32BIT          / * Use maximum 32bit for numeric data types     */
/*#define __DLP_DEPRECATED          / * Activate deprecated API (if any)             */

/* REMARKS */
/* 1) Some object functions will not work correctly */

/* -- Optimizations -- */
/*#define __OPTIMIZATIONS      / * Switch ALL optimizations OFF */
#ifdef __OPTIMIZATIONS
  #define __OPTIMIZE_TRIG      /* Approximation of trigonometric functions by table lookup */
  #define __OPTIMIZE_ALLOC      /* Do zero-init on zero-init allocates */
  #define __OPTIMIZE_LSADD     /* Approximation of log semiring addition (OP_LSADD) */
#endif

#endif /* #ifndef __DLP_CONFIG_H */

/* EOF */
