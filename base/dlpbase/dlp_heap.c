/* dLabPro base library
 * - Heap (and debug-heap) manager
 *
 * AUTHOR : Matthias Wolff
 * PACKAGE: dLabPro/base
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

#include "dlp_kernel.h"
#include "dlp_base.h"

/* Local functions */
hash_val_t __dlp_ptr2hash(const void* lpMemblock, void* lpHint)
{
#if defined _MSC_VER || defined __MINGW32__
  return (hash_val_t)(((__int64)lpMemblock)&0xFFFFFFFF);                        /* Hash code: least sign. 32 bits    */
#else
  return (hash_val_t)lpMemblock;
#endif

  /*
  hash_val_t m = (hash_val_t)lpMemblock;
  hash_val_t k = 0;
  k |= m & 0x00000001 << 31;
  k |= m & 0x00000002 << 30;
  k |= m & 0x00000004 << 29;
  k |= m & 0x00000008 << 28;
  k |= m & 0x00000010 << 27;
  k |= m & 0x00000020 << 26;
  k |= m & 0x00000040 << 25;
  k |= m & 0x00000080 << 24;
  k |= m & 0xFFFFFF00 >>  8;
  return k;
  */
}

int __dlp_compkeys(const void* lpKey1, const void* lpKey2, void* lpHint)
{
  if (lpKey1 == lpKey2) return 0;
  if (lpKey1  < lpKey2) return -1;
  return 1;
}

/* Global static variables */
static hash_t* __xalloc       = NULL;
static INT32    __xalloc_flags = 0;

/**
 * Starts the XAlloc heap manager.
 *
 * @param nFlags Settings, a combination of the XA_XXX constants.
 * @return TRUE if successfull, FALSE otherwise
 * @see dlp_xalloc_done
 */
BOOL dlp_xalloc_init(INT32 nFlags)
{
#ifdef __NOXALLOC
  return TRUE;
#else
  __xalloc_flags = nFlags;

#if ( defined __MSVC && defined _DEBUG )

  /* Report debug heap messages at stdout */
  _CrtSetReportMode(_CRT_WARN  ,_CRTDBG_MODE_FILE  );
  _CrtSetReportFile(_CRT_WARN  ,_CRTDBG_FILE_STDOUT);
  _CrtSetReportMode(_CRT_ERROR ,_CRTDBG_MODE_FILE  );
  _CrtSetReportFile(_CRT_ERROR ,_CRTDBG_FILE_STDOUT);
  _CrtSetReportMode(_CRT_ASSERT,_CRTDBG_MODE_FILE  );
  _CrtSetReportFile(_CRT_ASSERT,_CRTDBG_FILE_STDOUT);

#endif

  /* Check memory integrety */
  DLP_CHECK_MEMINTEGRITY;
  DLP_CHECK_MEMLEAKS;

  /* Start XAlloc */
  __xalloc = hash_create(HASHCOUNT_T_MAX,__dlp_compkeys,__dlp_ptr2hash,NULL);
  return (__xalloc!=NULL);
#endif
}

/**
 * Shut down the XAlloc heap manager.
 *
 * @param bCleanup Free allocated memory blocks
 * @see dlp_xalloc_init
 */
void dlp_xalloc_done(BOOL bCleanup)
{
#ifndef __NOXALLOC
  hscan_t  hs;
  hnode_t* hn;

  /* Check for memory leaks */
  if (!hash_isempty(__xalloc))
  {
    if (dlp_xalloc_flags() & XA_DLP_MEMLEAKS)
    {
      printf("\n*** xalloc: ERROR - Memory leaks detected.");
      if (bCleanup)
      {
        printf("\n            A list of memory objects follows.\n");
        dlp_xalloc_print();
        printf("*** xalloc: RECOVERING - Now freeing memory objects...");
      }
      else
      {
        printf("\n*** xalloc: NO CLEANUP, clearing object list...");
        printf("\n*** xalloc: re-run with option --trace-mem for details");
      }
    }

    while (!hash_isempty(__xalloc))
    {
      hash_scan_begin(&hs,__xalloc);
      if ((hn = hash_scan_next(&hs))!=NULL)
      {/* MWX 2003-03-27: May crash -->
        if (bCleanup) __dlp_free((void*)hnode_getkey(hn));
        else
        {*/
          alloclist_t* li = (alloclist_t*)hnode_get(hn);
          hash_scan_delfree(__xalloc,hn);
          free(li);
        /*<-- }*/
      }
    }

    if (dlp_xalloc_flags() & XA_DLP_MEMLEAKS)
    {
      if (hash_isempty(__xalloc)) printf("ok.\n\n");
      else
      {
        printf("failed.\n");
        DLPASSERT(FALSE);
      }
    }
  }

  /* Terminate XAlloc */
  hash_destroy(__xalloc);

  /* Check memory integrety */
  DLP_CHECK_MEMLEAKS;
  DLP_CHECK_MEMINTEGRITY;
#endif
}

/**
 * Returns the settings (made calling dlp_xalloc_init).
 *
 * @return A combination of XA_XXX constants.
 * @see dlp_xalloc_init
 */
INT32 dlp_xalloc_flags()
{
  return __xalloc_flags;
}

/**
 * Registers a memory object with the XAlloc memory allocation system.
 * This function is for internal use by the dLabPro kernel only.
 *
 * @param nType           Type of memory object ('C' calloc, 'M' malloc, 'I' dLabPro instance)
 * @param lpMemblock      Pointer to the memory object
 * @param nNum            Number of elements in the memory object
 * @param nSize           Size in bytes of one element in the memory object
 * @param lpsFilename     Position of allocation (source file)
 * @param nLine           Position of allocation (line number in source file)
 * @param lpsClassname    Class identifier of instance registering the memory object (dLabPro classes only!)
 * @param lpsInstancename Identifier of instance registering the memory object (dLabPro classes only!)
 */
void dlp_xalloc_register_object
(
  char        nType,
  const void* lpMemblock,
  size_t      nNum,
  size_t      nSize,
  const char* lpsFilename,
  INT32       nLine,
  const char* lpsClassname,
  const char* lpsInstancename
)
{
#ifndef __NOXALLOC
  alloclist_t* li;

  if (!__xalloc) return;                                                        /* XAlloc not started (or failed)    */
  li = (alloclist_t*)malloc(sizeof(alloclist_t));
  if (!li) return;

  dlp_splitpath(lpsFilename,NULL,li->lpsFilename);
  li->nType      = nType;
  li->lpMemblock = lpMemblock;
  li->nLine      = nLine;
  li->nNum       = nNum;
  li->nSize      = nSize;
  dlp_strncpy(li->lpsClassname   ,lpsClassname   ,L_NAMES  );
  dlp_strncpy(li->lpsInstancename,lpsInstancename,4*L_NAMES);

#ifdef _DEBUG
  if (hash_lookup(__xalloc,lpMemblock))
  {
    printf("\n*** xalloc: ERROR - Pointer 0x%p already registered (%s:%d)",   /*   Hard coded error message        */
      lpMemblock,__FILE__,(int)__LINE__);                                            /*   |                               */
    dlp_xalloc_unregister_object(lpMemblock);
  }
#endif
  hash_alloc_insert(__xalloc,lpMemblock,li);
#endif
}

/**
 * Unregisters a memory object with the XAlloc memory allocation system.
 * This function is for internal use by the dLabPro kernel only. It does not
 * destroy the memory object.
 *
 * @param lpMemblock
 *          Pointer to the memory object
 * @see dlp_xalloc_register_object
 */
void dlp_xalloc_unregister_object(const void* lpMemblock)
{
#ifndef __NOXALLOC
  hnode_t*     hn;                                                              /* Hash node                         */
  const void*  key;                                                             /* Hash key                          */
  alloclist_t* li;                                                              /* XAlloc list entry                 */

  if (!__xalloc) return;                                                        /* XAlloc not started (or failed)    */

  hn = hash_lookup(__xalloc,lpMemblock);                                        /* Find object in allocation list    */
  if (hn)                                                                       /* Object found                      */
  {                                                                             /* >>                                */
    li  = (alloclist_t*)hnode_get(hn);                                          /*   Get XAlloc list entry           */
    key = hnode_getkey(hn);                                                     /*   Get hash key                    */
    DLPASSERT(key==lpMemblock);                                                 /*   ... which is the object pointer */
    hash_scan_delfree(__xalloc,hn);                                             /*   Remove hash node                */
    free(li);                                                                   /*   Free XAlloc list entry          */
  }                                                                             /* <<                                */
  else                                                                          /* Object NOT found                  */
  {                                                                             /* >>                                */
    printf("\n*** xalloc: ERROR - Pointer 0x%p not found in xalloc (%s:%d)",    /*   Hard coded error message        */
      lpMemblock,__FILE__,(int)__LINE__);                                            /*   |                               */
    DLPASSERT(FALSE);                                                           /*   That's really really bad        */
  }                                                                             /* <<                                */
#endif
}

/**
 * Determines if an object is registered with the XAlloc heap manager.
 *
 * @param lpMemblock
 *          Pointer to the memory object
 * @return <code>TRUE</code> of the object was found, <code>FALSE</code>
 *         otherwise
 * @see dlp_xalloc_find_object_ex
 * @see dlp_xalloc_register_object
 * @see dlp_xalloc_unregister_object
 */
BOOL dlp_xalloc_find_object(const void* lpMemblock)
{
#ifdef __NOXALLOC
  fprintf(stderr,"__NOXALLOC defined => you should not use dlp_xalloc_find_object\n");
  return NULL;
#else
  if (!__xalloc) return TRUE;                                                   /* XAlloc not started (or failed)    */
  return hash_lookup(__xalloc,lpMemblock)!=NULL;                                /* Find object in allocation list    */
#endif
}

/**
 * Finds an object in the XAlloc heap manager and returns a pointer to its entry
 * in the allocation list.
 *
 * @param lpMemblock
 *          Pointer to the memory object
 * @return A pointer to a <code>alloclist_t</code> struct containing the
 *         allocation list entry associated with <code>lpMemblock</code> or
 *         <code>NULL</code> if the object is not registered.
 * @see dlp_xalloc_register_object
 * @see dlp_xalloc_unregister_object
 */
alloclist_t* dlp_xalloc_find_object_ex(const void* lpMemblock)
{
#ifdef __NOXALLOC
  fprintf(stderr,"__NOXALLOC defined => you should not use dlp_xalloc_find_object_ex\n");
  return NULL;
#else
  hnode_t*     hn;                                                              /* Hash node                         */
  alloclist_t* li;                                                              /* XAlloc list entry                 */

  if (!__xalloc) return NULL;                                                   /* XAlloc not started (or failed)    */

  if (!(hn = hash_lookup(__xalloc,lpMemblock))) return NULL;                    /* Find object in allocation list    */
  li = (alloclist_t*)hnode_get(hn);                                             /* Get XAlloc list entry             */
  DLPASSERT(hnode_getkey(hn)==lpMemblock);                                      /* ... which is the object pointer   */
  return li;                                                                    /* Return XAlloxc list entry         */
#endif
}

/**
 * Returns the size in bytes of a registered memory object.
 *
 * @param lpMemblock Pointer to the memory object
 * @return The size of the memory object in bytes or 0 in case of errors.
 */
size_t dlp_size(const void* lpMemblock)
{
#ifdef __NOXALLOC
  fprintf(stderr,"__NOXALLOC defined => you should not use dlp_size\n");
  return NULL;
#else
  alloclist_t* li;
  hnode_t*     hn;

  if (!__xalloc       ) return 0; /* XAlloc not started (or failed): ok, but do nothing */
  if (lpMemblock==NULL) return 0;

  hn = hash_lookup(__xalloc,lpMemblock);
  if (!hn) return 0;

  li = (alloclist_t*)hnode_get(hn);
  return (li->nNum*li->nSize);
#endif
}

/**
 * Checks whether a memory object is registered with the XAlloc memory
 * allocation system.
 *
 * @param lpMemblock Pointer to the memory object
 * @return TRUE if the memory object is registered, FALSE otherwise
 */
BOOL dlp_in_xalloc(const void* lpMemblock)
{
#ifdef __NOXALLOC
  fprintf(stderr,"__NOXALLOC defined => you should not use dlp_in_xalloc\n");
  return NULL;
#else
  if (!__xalloc) return FALSE; /* XAlloc not started (or failed): thus memblock is "not in list" */
  return hash_lookup(__xalloc,lpMemblock)!=NULL;
#endif
}

/**
 * Prints a list of all memory objects registered with the XAlloc memory
 * allocation system on stdout.
 */
void dlp_xalloc_print()
{
  alloclist_t*  li;
  hscan_t       hs;
  hnode_t*      hn;
  char          lpBuf[255];
  UINT64 memuse = 0;
  UINT32 i;

  printf("\n ");
  dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  printf("\n  XAlloc memory usage\n ");
  dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());

  if (!__xalloc)
  {
    printf("\n\n [ XAlloc system not running ] \n\n ");
    dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
    printf("\n\n");
  }
  else
  {
    dlp_init_printstop();
    hash_scan_begin(&hs,__xalloc);

    i=0;
    while ((hn = hash_scan_next(&hs))!=NULL)
    {
      li = (alloclist_t*)hnode_get(hn);

      if (dlp_strlen(li->lpsClassname) && dlp_strlen(li->lpsInstancename))
        sprintf(lpBuf,"%s %s",li->lpsClassname,li->lpsInstancename);
      else if (dlp_strlen(li->lpsClassname))
        sprintf(lpBuf,"%s",li->lpsClassname);
      else if (dlp_strlen(li->lpsInstancename))
        sprintf(lpBuf,"%s",li->lpsInstancename);
      else
        sprintf(lpBuf,"[unknown]");

      switch (li->nType)
      {
      case 'C':
        printf("\n  %5d: %c 0x%p %8lu x %5lu bytes for %-24s @ %s:%ld",
          (int)i,li->nType,li->lpMemblock,(unsigned long)li->nNum,
          (unsigned long)li->nSize,lpBuf,li->lpsFilename,(long)li->nLine);
        break;
      default:
        printf("\n  %5d: %c 0x%p %16lud bytes for %-24s @ %s:%ld",
          (int)i,li->nType,li->lpMemblock,(unsigned long)li->nSize,lpBuf,
          li->lpsFilename,(long)li->nLine);
      }

      if (dlp_if_printstop()) break;
      i++;
    }
    if (hash_count(__xalloc)==0) printf("\n  [no memory objects]");

    hash_scan_begin(&hs,__xalloc);
    while ((hn = hash_scan_next(&hs))!=NULL)
    {
      li = (alloclist_t*)hnode_get(hn);
      memuse+=(UINT64)(li->nNum*li->nSize);
    }

    printf("\n ");
    dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
    printf("\n  Total: %d memory objects allocated through XAlloc",(int)hash_count(__xalloc));
    printf("\n         %d kBytes used\n\n",(int)((FLOAT64)memuse/1024.+.5));
  }
}

/**
 * NEVER CALL THIS FUNCTION DIRECTLY; Use macro dlp_calloc() instead.
 *
 * Allocates an array in memory with elements initialized to 0. Keeps a
 * list of all allocated objects.
 *
 * @param nNum            Number of elements
 * @param nSize           Size in bytes of each element
 * @param lpsFilename     Position of allocation (source file)
 * @param nLine           Position of allocation (line number in source file)
 * @param lpsClassname    Class identifier of instance allocating the memory block (dLabPro classes only!)
 * @param lpsInstancename Identifier of instance allocating the memory block (dLabPro classes only!)
 * @return Pointer to allocated memory or NULL if no more memory is available
 *         or in case of errors
 *
 * @see #__dlp_free dlp_free
 * @see #__dlp_malloc dlp_malloc
 * @see #__dlp_realloc dlp_realloc
 */
void* __dlp_calloc
(
  size_t      nNum,
  size_t      nSize,
  const char* lpsFilename,
  INT32       nLine,
  const char* lpsClassname,
  const char* lpsInstancename
)
{
  /* HACK: malloc(num*size) + memset(0) does not necessarily the same */
  /*       as calloc(num,size). This implementation is for backward   */
  /*       compatibility                                              */

  size_t nLength    = 0;
  void*  lpMemblock = NULL;

  nLength = nNum*nSize;
#if (defined __MIPS || defined __ULTRIX)
  if (nLength<256) nLength=256;
#endif

  lpMemblock = malloc(nLength);
  if (lpMemblock == NULL)
  {
    printf("\n*** xalloc: ERROR - Allocation of %lu bytes failed (%s:%d)",(unsigned long)nLength,lpsFilename,(int)nLine);
    return NULL;
  }

  memset(lpMemblock,0,nLength);
  dlp_xalloc_register_object('C',lpMemblock,nNum,nSize,lpsFilename,nLine,lpsClassname,lpsInstancename);

  return lpMemblock;
}

/**
 * NEVER CALL THIS FUNCTION DIRECTLY; use macro dlp_malloc() instead.
 *
 * Allocates a memory block. Keeps a list of all allocated objects.
 *
 * @param nSize           Size in bytes
 * @param lpsFilename     Position of allocation (source file)
 * @param nLine           Position of allocation (line number in source file)
 * @param lpsClassname    Class identifier of instance allocating the memory block (dLabPro classes only!)
 * @param lpsInstancename Identifier of instance allocating the memory block (dLabPro classes only!)
 * @return Pointer to allocated memory or NULL if no more memory is available
 *         or in case of errors
 *
 * @see #__dlp_free dlp_free
 * @see #__dlp_calloc dlp_calloc
 * @see #__dlp_realloc dlp_realloc
 */
void* __dlp_malloc
(
  size_t      nSize,
  const char* lpsFilename,
  INT32       nLine,
  const char* lpsClassname,
  const char* lpsInstancename
)
{
  void* lpMemblock = NULL;

#if (defined __MIPS || defined __ULTRIX)
  if (nSize<256) nSize=256;
#endif

  lpMemblock = malloc(nSize);
  if (lpMemblock == NULL)
  {
    printf("\n*** xalloc: ERROR - Allocation of %lu bytes failed (%s:%d)",(unsigned long)nSize,lpsFilename,(int)nLine);
    return NULL;
  }

#ifndef __OPTIMIZE_ALLOC
  memset(lpMemblock, 0L, nSize);
#endif

  dlp_xalloc_register_object('M',lpMemblock,1,nSize,lpsFilename,nLine,lpsClassname,lpsInstancename);

  return lpMemblock;
}

/**
 * NEVER CALL THIS FUNCTION DIRECTLY; use macro dlp_realloc() instead.
 *
 * Reallocates a previously allocated memory block.
 *
 * @param lpMemblock      Previously allocated memory block
 * @param nNum            Number of elements
 * @param nSize           Size in bytes of each element
 * @param lpsFilename     Position of allocation (source file)
 * @param nLine           Position of allocation (line number in source file)
 * @param lpsClassname    Class identifier of instance allocating the memory block (dLabPro classes only!)
 * @param lpsInstancename Identifier of instance allocating the memory block (dLabPro classes only!)
 * @return Pointer to allocated memory or NULL if no more memory is available
 *         or in case of errors
 *
 * @see #__dlp_calloc dlp_calloc
 * @see #__dlp_free dlp_free
 * @see #__dlp_malloc dlp_malloc
 */
void* __dlp_realloc
(
  void*       lpMemblock,
  size_t      nNum,
  size_t      nSize,
  const char* lpsFilename,
  INT32       nLine,
  const char* lpsClassname,
  const char* lpsInstancename
)
{
  size_t m = 0;
  size_t l = 0;
  void*  p = NULL;

  if (nNum  < 1) { dlp_free(lpMemblock); return NULL; }
  if (nSize < 1) { dlp_free(lpMemblock); return NULL; }

  if (lpMemblock == NULL) return __dlp_calloc(nNum,nSize,lpsFilename,nLine,lpsClassname,lpsInstancename);
  #ifndef __NOXALLOC
  if (!dlp_in_xalloc(lpMemblock)) return NULL;
  #endif
  m=dlp_size(lpMemblock);

  /* Try to allocate new memory block additionally */
  p=__dlp_malloc(nNum*nSize,lpsFilename,nLine,lpsClassname,lpsInstancename);
  if (p)
  {
    /* Allocation successful -> copy memory */
    if(m<nNum*nSize) {
      memmove(p,lpMemblock,m);
      memset((char*)p+m,0L,nNum*nSize-m);
    } else {
      memmove(p,lpMemblock,nNum*nSize);
    }
    dlp_free(lpMemblock);
  }
  else
  {
    /* Allocation failed -> swap through hard disk */
    char  lpsTmpfile[L_PATH];
    FILE* f = NULL;

#ifdef __LINUX
    if(mkstemp(lpsTmpfile)==-1) return NULL;
#else
    tmpnam(lpsTmpfile);
#endif
    if ((f = fopen(lpsTmpfile,"wb"))==NULL) return NULL;
    if(fwrite(lpMemblock,m,1,f)!=1) return NULL;
    fclose(f);
    dlp_free(lpMemblock);

    p = __dlp_calloc(nNum,nSize,lpsFilename,nLine,lpsClassname,lpsInstancename);
    if (p==NULL) return NULL;

    l = m;
    if (nNum*nSize < l) l = nNum*nSize;

    if ((f = fopen(lpsTmpfile,"rb"))==NULL) return NULL;
    if(fread(p,l,1,f)!=1) return NULL;
    fclose(f);
    remove(lpsTmpfile);
  }

  return p;
}

/**
 * NEVER CALL THIS FUNCTION DIRECTLY; use macro dlp_free() instead.
 *
 * Deallocates or frees a memory block. Deletes the block from the
 * list of memory objects kept by XAlloc.
 *
 * @param lpMemblock Memory block to be freed
 *
 * @see #__dlp_calloc dlp_calloc
 * @see #__dlp_malloc dlp_malloc
 * @see #__dlp_realloc dlp_realloc
 */
void __dlp_free(void* lpMemblock)
{
  if (lpMemblock==NULL) return;
  dlp_xalloc_unregister_object(lpMemblock);
  free(lpMemblock);
}

/* EOF */
