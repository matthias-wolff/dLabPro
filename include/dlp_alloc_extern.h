/* dLabPro base library
 * - Header file - Redirect memory allocation in 3rd party sources
 *
 * AUTHOR : Matthias Eichner
 * PACKAGE: dLabPro/sdk
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

#ifndef __DLPALLOC_EXTERN_H
#define __DLPALLOC_EXTERN_H

#include "dlp_kernel.h"

#define calloc(A,B)  __dlp_calloc(A,B,__FILE__,__LINE__,"extern",NULL)
#define malloc(A)    __dlp_malloc(A,__FILE__,__LINE__,"extern",NULL)
#define realloc(A,B) __dlp_realloc(A,1,B,__FILE__,__LINE__,"extern",NULL)
#define free(A)      __dlp_free(A)

void* __dlp_calloc(size_t nNum, size_t nSize, const char* lpsFilename, INT32 nLine, const char* lpsClassname, const char* lpsInstancename);
void* __dlp_malloc(size_t nSize, const char* lpsFilename, INT32 nLine, const char* lpsClassname, const char* lpsInstancename);
void* __dlp_realloc(void* lpMemblock, size_t nNum, size_t nSize, const char* lpsFilename, INT32 nLine, const char* lpsClassname, const char* lpsInstancename);
void  __dlp_free(void* lpMemblock);

#endif /* if !defined __DLPALLOC_EXTERN_H */

/* EOF */
