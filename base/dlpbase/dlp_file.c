/* dLabPro base library
 * - File Operations
 *
 * AUTHOR : Frank Duckhorn
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

/* Get mktemp() on MSVC */
#ifdef __MSVC
#undef _POSIX_
#undef __STDC__
#include <io.h>
#endif

#ifndef __NOZLIB
#  include "zlib.h"
#  include "zutil.h"
#endif

#ifdef USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#endif

/*#define USE_MMAP        / * Use mmap routines to map whole file to memory before compression, needs type caddr_t (not ANSI nor POSIX) */

#ifndef DLP_PRINTF_BUFSIZE
#  define DLP_PRINTF_BUFSIZE 4096
#endif

#if !defined ZLIB_VERSION && !defined __NOZLIB
#  warning "zlib not found => compiling without support for zlib"
#  define __NOZLIB
#endif

#define BUFLEN      16384
#define MAX_NAME_LEN 1024

#ifndef __NOZLIB
BOOL gz_compress      OF((FILE   *lpInfile, gzFile lpOutfile));
#ifdef USE_MMAP
BOOL gz_compress_mmap OF((FILE   *lpInfile, gzFile lpOutfile));
#endif
BOOL gz_uncompress    OF((gzFile lpInfile, FILE   *lpOutfile));
INT32    gz_rename(char* lpsSource, const char* lpsDest);
#endif /* __NOZLIB */


/*
 * Open a file <path>. The file is opened through zlib if <mode> starts
 * with the character 'z'. Otherwise if file exists compression is selected
 * if file is zipped, or <fopen> is used. If succsesfull a pointer to
 * a <DLP_FILE> structure is returned.
 */
DLP_FILE *dlp_fopen(const char *path,const char *mode)
{
  DLP_FILE *lpZF;
  lpZF = malloc(sizeof(DLP_FILE));
#ifndef __NOZLIB
  lpZF->m_nCompressed = mode[0]=='z' || mode[0]=='r';
  if(mode[0]=='z') mode++;
  lpZF->m_lpFile = lpZF->m_nCompressed ? (void *)gzopen(path,mode) : (void *)fopen(path,mode);
#else /* __NOZLIB */
  lpZF->m_nCompressed = 0;
  lpZF->m_lpFile = (void *)fopen(path,mode + (mode[0]=='z'?1:0));
#endif /* __NOZLIB */
  if(!lpZF->m_lpFile)
  {
    free(lpZF);
    return NULL;
  }
  return lpZF;
}

/*
 * Closes the zipped or normal file and frees the memory of the <DLP_FILE> structure
 */
INT32 dlp_fclose(DLP_FILE *lpZF)
{
  INT32 ret;
  if(!lpZF) return -1;
  ret =
#ifndef __NOZLIB
    lpZF->m_nCompressed ? gzclose((gzFile)lpZF->m_lpFile) :
#endif /* __NOZLIB */
    fclose((FILE*)lpZF->m_lpFile);
  free(lpZF);
  return ret;
}

/*
 * Read a buffer from a zipped or normal file.
 */
size_t dlp_fread(void *ptr,size_t size,size_t nmemb,DLP_FILE *lpZF)
{
  if(!lpZF) return 0;
  return
#ifndef __NOZLIB
    lpZF->m_nCompressed ? (size_t)gzread((gzFile)lpZF->m_lpFile,ptr,size*nmemb) :
#endif /* __NOZLIB */
    fread(ptr,size,nmemb,(FILE*)lpZF->m_lpFile);
}

/*
 * Write a buffer to a zipped or normal file.
 */
size_t dlp_fwrite(const void *ptr, size_t size, size_t nmemb, DLP_FILE *lpZF)
{
  if(!lpZF) return 0;
  return
#ifndef __NOZLIB
    lpZF->m_nCompressed ? (size_t)gzwrite((gzFile)lpZF->m_lpFile,(void *)ptr,size*nmemb) :
#endif /* __NOZLIB */
    fwrite(ptr,size,nmemb,(FILE*)lpZF->m_lpFile);
}

int dlp_ferror(DLP_FILE *lpZF)
{
  if(!lpZF) return -1;
#ifndef __NOZLIB
  if(lpZF->m_nCompressed){
    int nErr;
    gzerror((gzFile)lpZF->m_lpFile,&nErr);
    return nErr<0 ? nErr : 0;
  }
#endif /* __NOZLIB */
  return ferror((FILE*)lpZF->m_lpFile);
}

/*
 * Check end of file of a zipped or normal file.
 */
INT32 dlp_feof(DLP_FILE *lpZF)
{
  if(!lpZF) return -1;
  return
#ifndef __NOZLIB
    lpZF->m_nCompressed ? gzeof((gzFile)lpZF->m_lpFile) :
#endif /* __NOZLIB */
    feof((FILE*)(lpZF)->m_lpFile);
}
/*
 * Write a formated string to a zipped or normal file.
 */
INT32 dlp_fprintf(DLP_FILE *lpZF,const char *format, ...)
{
  va_list va;
  size_t len;
  char buf[DLP_PRINTF_BUFSIZE];
  if(!lpZF) return -1;
  va_start(va,format);
#ifdef HAS_vsnprintf
   (void)vsnprintf(buf, sizeof(buf), format, va);
#else
   (void)vsprintf(buf, format, va);
#endif
   va_end(va);
   len=strlen(buf);
   if(len<=0) return 0;
  return dlp_fwrite(buf,len,1,lpZF);
}


/**
 * Compress the given file: create a temporary output file. Remove original
 * file and rename temporary file after successfull completion of gz_compress().
 *
 * @param lpsInfile
 *       Pointer to filename of input file
 * @param lpsMode
 *       Mode of file compression. ("wb[0-9]")
 * @return <code>TRUE</code> if compression succeeded, <code>FALSE</code> otherwise.
 */
BOOL dlp_fzip(const char *lpsInfile, const char *lpsMode)
{
#ifndef __NOZLIB
  char   lpsFile[L_PATH];
  char   lpsDir[L_PATH];
  char  *lpsOutfile = NULL;
  BOOL   bSuccess = TRUE;
    FILE  *lpInfile;
    gzFile lpOutfile;

  /* Open input file */
    lpInfile = fopen(lpsInfile, "rb");
    if (lpInfile == NULL) {
        perror(lpsInfile);
        return FALSE;
    }

  /* Create and open temporary output file */
  dlp_splitpath(lpsInfile,lpsDir,lpsFile);
  lpsOutfile = dlp_tempnam(NULL,lpsFile);
    lpOutfile = gzopen(lpsOutfile, lpsMode);
    if (lpOutfile == NULL) {
        fprintf(stderr, "zlib@zlib_fcompress(): can't gzopen %s\n", lpsOutfile);
        fclose(lpInfile);
        return FALSE;
    }


    bSuccess = gz_compress(lpInfile, lpOutfile);

  if(bSuccess)
  {
      strncpy(lpsFile,lpsInfile,L_PATH-2);
      strcat(lpsFile,"~");
      rename(lpsInfile,lpsFile);
      if(gz_rename(lpsOutfile,lpsInfile)==0)
      {
    if(unlink(lpsFile))  perror("unlink");
      }
      else
      {
    fprintf(stderr, "zlib@zlib_fcompress(): failed to compress file. Save %s uncompressed\n", lpsInfile);
    unlink(lpsInfile);
    rename(lpsFile,lpsInfile);
      }
  }
  else
  {
      fprintf(stderr, "zlib@zlib_fcompress(): failed to compress file. Save %s uncompressed\n", lpsInfile);
      if(unlink(lpsOutfile)) perror("unlink");
  }

  return TRUE;
#else /* __NOZLIB */
  return FALSE;
#endif
}


/**
 * Try to uncompress a file. Return immidiately if it is not compressed. Otherwise
 * uncompress it to a temporary file and return the (temporary) filename in lpsOutfile.
 * The filename is returned in an internal static buffer. Thus any subsequent
 * calls destroy the value.<code>free</code> does not need to be called to
 * deallocate this pointer
 *
 * @param lpsInfile
 *       Pointer to filename of input file
 * @param lpsInfile
 *       Pointer to filename of output file
 * @return <code>TRUE</code> if file has been uncompressed, <code>FALSE</code> otherwise.
 */
BOOL dlp_funzip(const char *lpsInfile,char const **lpsOutfile)
{
#ifndef __NOZLIB
  char   lpsFile[L_PATH];
  char   lpsDir[L_PATH];
  BOOL   bSuccess       = TRUE;
    FILE  *lpOutfile      = NULL;
    gzFile lpInfile       = NULL;

  /* Do nothing if file is not compressed */
    lpInfile = gzopen(lpsInfile, "rb");
    if (!lpInfile) return FALSE;
    if (gzdirect(lpInfile))
    {
      gzclose(lpInfile);
      return FALSE;
    }

  /* Create and open temporary output file */
  dlp_splitpath(lpsInfile,lpsDir,lpsFile);
  *lpsOutfile = dlp_tempnam(NULL,lpsFile);
    lpOutfile = fopen(*lpsOutfile, "wb");
    if (lpOutfile == NULL)
    {
        perror(*lpsOutfile);
        gzclose(lpInfile);
        unlink(*lpsOutfile);
        return FALSE;
    }

    bSuccess = gz_uncompress(lpInfile, lpOutfile);

  if(!bSuccess)
  {
        unlink(*lpsOutfile);
    *lpsOutfile=lpsInfile;
    return FALSE;
  }

    return TRUE;
#else /* __NOZLIB */
  return FALSE;
#endif
}


#ifndef __NOZLIB

/* ===========================================================================
 * Compress input to output then close both files.
 */
BOOL gz_compress(FILE *lpInfile, gzFile lpOutfile)
{
  BOOL nSuccess = TRUE;
    local char buf[BUFLEN];
    INT32 len;
    int     err;

#ifdef USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    if (gz_compress_mmap(lpInfile, lpOutfile) == Z_OK) return;
#endif
    for (;;) {
        len = fread(buf, 1, sizeof(buf), lpInfile);
        if (ferror(lpInfile)) {
            perror("fread");
            nSuccess = FALSE;
        }
        if (len == 0) break;

        if (gzwrite(lpOutfile, buf, (unsigned)len) != len)
        {
          perror(gzerror(lpOutfile, &err));
          nSuccess = FALSE;
        }
    }
    fclose(lpInfile);
    if (gzclose(lpOutfile) != Z_OK)
    {
      perror("failed gzclose");
      return nSuccess;
    }

    return TRUE;
}

#ifdef USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * if success, Z_ERRNO otherwise.
 */
BOOL gz_compress_mmap(FILE *lpInfile, gzFile lpOutfile)
{
    INT32 len;
    INT32 err;
    INT32 ifd = fileno(lpInfile);
    caddr_t buf;    /* mmap'ed buffer for the entire input file */
    off_t buf_len;  /* length of the input file */
    struct stat sb;

    /* Determine the size of the file, needed for mmap: */
    if (fstat(ifd, &sb) < 0) return Z_ERRNO;
    buf_len = sb.st_size;
    if (buf_len <= 0) return Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    if (buf == (caddr_t)(-1)) return Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(lpOutfile, (char *)buf, (unsigned)buf_len);

    if (len != (INT32)buf_len) perror(gzerror(lpOutfile, &err));

    munmap(buf, buf_len);
    fclose(lpInfile);
    if (gzclose(lpOutfile) != Z_OK) perror("failed gzclose");
    return Z_OK;
}
#endif /* USE_MMAP */

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
BOOL gz_uncompress(gzFile lpInfile, FILE *lpOutfile)
{
  BOOL nSuccess =TRUE;
    local char buf[BUFLEN];
    INT32 len;
    int     err;

    for (;;) {
        len = gzread(lpInfile, buf, sizeof(buf));
        if (len < 0)
        {
          perror (gzerror(lpInfile, &err));
          nSuccess =FALSE;
        }
        if (len == 0) break;

        if ((INT32)fwrite(buf, 1, (unsigned)len, lpOutfile) != len)
        {
        perror("failed fwrite");
        nSuccess =FALSE;
    }
    }
    if (fclose(lpOutfile))
    {
      perror("failed fclose");
      nSuccess =FALSE;
    }

    if (gzclose(lpInfile) != Z_OK)
    {
      perror("failed gzclose");
      nSuccess =FALSE;
    }

    return nSuccess;
}

/* Rename file even if source and target live on different file systems */
/* Return errno                                                                             */
INT32 gz_rename(char* lpsSource, const char* lpsDest)
{
  /*printf("\n %s -> %s\n",lpsSource, lpsDest);*/
  errno = 0;
  if(rename(lpsSource,lpsDest)!=0)  /*  Files live on different file systems  */
  {
      local char buf[BUFLEN];
      INT32 len;
      FILE  *in;
      FILE  *out;
      errno = 0;

      in = fopen (lpsSource,"rb");
      if (in == NULL) {
        perror(lpsSource);
        return errno;
      }

      out = fopen (lpsDest,"wb");
      if (out == NULL) {
        perror(lpsDest);
        fclose(in);
        return errno;
      }

      for (;;) {
      len = fread(buf, 1, sizeof(buf), in);
      if (ferror(in)) {
        perror("fread");
        break;
      }
      if (len == 0) break;

      if ((INT32)fwrite(buf, 1, (unsigned)len, out) != len) {
          perror("failed fwrite");
      }
    }
    if (fclose(in))   perror("fclose");
    if (fclose(out)) perror("fclose");
    unlink(lpsSource);
  }

  /*printf("\n errno=%d\n",(int)errno);*/

  return errno;
}

#endif /* __NOZLIB */

#ifndef __TMS
/**
 * Changes the current working directory. If <code>bCreate</code> is
 * <code>TRUE</code> and the directory does not exists, <code>dlp_chdir</code>
 * will create the directory. <code>dlp_chdir</code> is capable of creating a
 * tree of non-existing directories at once.
 *
 * @param lpsDirname Destination directory
 * @param bCreate    Create directory if not exists
 * @return 0 if successful, an error code (or -1) if not sucessful
 */
INT16 dlp_chdir(const char* lpsDirname, BOOL bCreate)
{
  char  lpsDir[L_PATH];
  char* tx;
  INT16 nError;

  if (dlp_strlen(lpsDirname)==0) return 0;
  nError = (INT16)chdir(lpsDirname);
  if (nError == 0 || !bCreate) return nError;

  /* Try to create path */
  dlp_memset(lpsDir,0,L_PATH);
  dlp_strcpy(lpsDir,lpsDirname);

  /* Change into the furthest existing directory */
  tx = &lpsDir[dlp_strlen(lpsDir)-1];
  while (tx != lpsDir)
  {
    while (tx!=lpsDir && *tx != '\\' && *tx != '/') tx--;
    if (tx!=lpsDir) *tx=0;
    if (chdir(lpsDir) == 0) break;
  }
  if (tx!=lpsDir) tx++;

  /* Create remaining sub directories */
  while (tx[1])
  {
#if (defined __MSOS || defined __WIN32)
    nError = (INT16)mkdir(tx);
#else
    nError = (INT16)mkdir(tx,0xFFFF);
#endif
    if (nError) return nError;
    nError = (INT16)chdir(tx);
    if (nError) return nError;
    while (*tx++);
  }

  /* Should have worked... */
  return 0;
}
#endif

/**
 * Create a name for a temporary file. The function calls <code>mkstemp</code>.
 * mktemp is used if no mkstemp is available (see platform dependent define
 * <code>HAVE_NO_MKSTEMP</code> in dlp_base.h).
 * The filename is returned in an internal static buffer. Thus any subsequent
 * calls destroy the value.<code>free</code> does not need to be called to
 * deallocate this pointer.
 *
 * @param lpsDir
 *          Target directory to be used. If lpsDir is NULL, the function tries
 *       to determine the temporary directory by evaluation of the environment
 *       variables TEMP and TMP in this order. If none of these is set then
 *       the current directory is used.
 * @param lpsPfx
 *          Filename prefix
 * @return A pointer to the generated name or <code>NULL</code> if no temporary
 *         name could be created.
 */
char* dlp_tempnam(const char* lpsDir, const char* lpsPfx)
{
  static const char *lpsTempDir = NULL;
  char         lpsBuf[L_PATH];
  static char  lpsTemname[L_PATH];

  sprintf(lpsBuf, "%sXXXXXX",lpsPfx);
  if(lpsDir==NULL)
  {
    if(!lpsTempDir) lpsTempDir = getenv("TEMP");
    if(!lpsTempDir) lpsTempDir = getenv("TMP");
#ifdef __LINUX
#ifndef __TMS
    if(!lpsTempDir)
    {
      if(getcwd(lpsTemname,L_PATH) == NULL) return NULL;
      if(chdir("/var/tmp")==0) lpsTempDir = "/var/tmp";
      else if(chdir("/tmp")==0) lpsTempDir = "/tmp";
      if(chdir(lpsTemname)!=0) return NULL;
    }
#endif
#endif
    if(!lpsTempDir) lpsTempDir = ".";
    sprintf(lpsTemname,"%s%c%s",lpsTempDir,C_DIR,lpsBuf);
  }
  else sprintf(lpsTemname,"%s%c%s",lpsDir,C_DIR,lpsBuf);

#ifndef __TMS
#ifdef HAVE_NO_MKSTEMP
  dlp_strncpy(lpsTemname,_mktemp(lpsTemname),L_PATH-1);
#else
  close(mkstemp(lpsTemname));  /* mkstemp opens the file and returns the file descriptor. */
                                             /* Lets close it immediately, since we just need the name. */
#endif
#endif
  return lpsTemname;
}

#ifndef __TMS
/**
 * Create an absolute or full path name for the specified relative path name.
 *
 * @param lpsAbsPath
 *          Pointer to a buffer to be filled with the absolute path.
 * @param lpsRelPath
 *          Pointer to a null-terminated string containing the relative path.
 * @param nMaxLen
 *          Maximum length of the absolute path name buffer
 *          (<code>lpsAbsPath</code>).
 * @return <code>lpsAbsPath</code> if successful, <code>NULL</code> in case of
 *         errors.
 */
char* dlp_fullpath(char* lpsAbsPath, const char* lpsRelPath, INT32 nMaxLen)
{
  char        lpsCwd [L_PATH+1];                                                /* Saved current working directory   */
  char        lpsDir [L_PATH+1];                                                /* Directory part of lpsRelPath      */
  char        lpsFile[L_PATH+1];                                                /* Filename part of lpsRelPath       */
  struct stat filestat;                                                         /* File status struct                */
  if (!lpsAbsPath || nMaxLen<=0) return NULL;                                   /* No buffer, no service!            */
  *lpsAbsPath='\0';                                                             /* Initialize result to empty string */
  dlp_splitpath(lpsRelPath,lpsDir,lpsFile);                                     /* Split relative path               */
  if (!dlp_strlen(lpsDir)) dlp_strcpy(lpsDir,".");                              /* No directory -> use current one   */
  if(getcwd(lpsCwd,L_PATH)==NULL) return NULL;                                  /* Save current working directory    */
  if (dlp_chdir(lpsDir,FALSE)!=0) { dlp_chdir(lpsCwd,FALSE); return NULL;  }    /* Change to relative directory ...  */
  if(getcwd(lpsDir,L_PATH)==NULL) return NULL;                                  /* ... and determine its full path   */
  dlp_chdir(lpsCwd,FALSE);                                                      /* Change back to saved working dir. */
  if (nMaxLen<(INT32)(dlp_strlen(lpsDir)+dlp_strlen(lpsFile)+2)) return NULL;   /* Check return buffer size          */
  sprintf(lpsAbsPath,"%s%c%s",lpsDir,C_DIR,lpsFile);                            /* Create absolute path              */
#ifdef _WINDOWS                                                                 /* -- WINDOZE -->                    */
  dlp_strreplace(lpsAbsPath,"\\","/");                                          /* Replace backslashes by slashes    */
/*  dlp_strlwr(lpsAbsPath); */                                                    /* Convert to lower case             */
#endif                                                                          /* <--                               */
  if (stat(lpsAbsPath,&filestat)!=0) return NULL;                               /* Check file status (must exist)    */
  return lpsAbsPath;                                                            /* Return pointer to buffer          */
}
#endif

/* EOF */
