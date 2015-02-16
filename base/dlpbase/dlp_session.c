/* dLabPro base library
 * - Session management
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
#include "dlp_rev.h"
#ifdef __MSVC
  #pragma warning( disable : 4115 )                                             /* Named type def's in braces        */
#endif
#ifdef __WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>                                                          /* MFC core and standard components  */
  #include <signal.h>
#endif


/* Static variables */
static INT16 __nKernelDebugLevel    = 0;
static char  __lpBinaryName[L_PATH] = "";
static char  __lpBinaryPath[L_PATH] = "";
static INT32 __nRetVal              = 0;
static BOOL  __bRequestInterrupt    = FALSE;

/**
 * Sets the dLabPro kernel debugging level (verbose level).
 *
 * @param nLevel 0 for silence, greater level produces more debug messages
 * @see dlp_get_kernel_debug
 */
void dlp_set_kernel_debug(INT16 nLevel)
{
  __nKernelDebugLevel = nLevel;
}

/**
 * Returns the current dLabPro kernel debugging level (verbose level).
 *
 * @see dlp_set_kernel_debug
 */
INT16 dlp_get_kernel_debug()
{
  return __nKernelDebugLevel;
}

/**
 * Sets the dLabPro session name.
 *
 * @param lpName The session name * @see dlp_get_binary_name
 */
void dlp_set_binary_name(const char* lpName)
{
  dlp_strncpy(__lpBinaryName,lpName,L_PATH);
}

/**
 * Returns the current dLabPro session name.
 *
 * @see dlp_set_binary_name
 */
const char* dlp_get_binary_name()
{
  if (dlp_strlen(__lpBinaryName)==0)
    return __PROJECT_NAME;
  return __lpBinaryName;
}

/**
 * Sets the path name of the dLabpro binary.
 *
 * @param lpName The path name
 * @see dlp_get_binary_path
 * @see dlp_get_binary_name
 */
void dlp_set_binary_path(const char* lpName)
{
  dlp_strncpy(__lpBinaryPath,lpName,L_PATH);
}

/**
 * Returns the path name of the dLabPro binary.
 *
 * @see dlp_set_binary_path
 * @see dlp_set_binary_name
 */
const char* dlp_get_binary_path()
{
  return __lpBinaryPath;
}

/**
 * Scan the command line (argv, argc) for pairs of &lt;option&gt;&lt;del&gt;&lt;value&gt;.
 *
 * @param argc        Number of command line arguments
 * @param argv        List of command line arguments
 * @param lpOption    Key to search
 * @param lpDelimiter Key/value delimiter (may be more than one character)
 * @param lpValue     Buffer for value
 * @param bRemove     If true, option value pair is removed from command line
 * @return TRUE if option was found and a value was stored, FALSE otherwise
 */
INT16 dlp_scancmdlineoption(INT32* argc, char** argv, const char* lpOption, const char* lpDelimiter, char* lpValue, BOOL bRemove)
{
  INT16 i=0;
  INT16 j=0;

  if (lpValue) lpValue[0]=0;
  if (!dlp_strlen(lpOption)) return FALSE;

  for (i=0; i<*argc; i++)
  {
    if (dlp_strncmp(lpOption,argv[i],dlp_strlen(lpOption))==0)
    {
      char* tx = &argv[i][dlp_strlen(lpOption)];
      if (dlp_strlen(lpDelimiter))
      {
        if (dlp_strncmp(" ",lpDelimiter,dlp_strlen(lpDelimiter))==0)
        {
          if (dlp_strncmp(argv[i],lpOption,dlp_strlen(argv[i]))!=0) continue;
          if (i+1<*argc && argv[i+1] && lpValue) strcpy(lpValue,argv[i+1]);
          if (bRemove)
          {
            for (j=i; j<*argc-2; j++) argv[j]=argv[j+2];
            (*argc)-=2;
          }
          return TRUE;
        }
        if (dlp_strncmp(tx,lpDelimiter,dlp_strlen(lpDelimiter))!=0) continue;
        tx = &tx[dlp_strlen(lpDelimiter)];
      }
      if (lpValue) strcpy(lpValue,tx);
      if (bRemove)
      {
        for (j=i; j<*argc-1; j++) argv[j]=argv[j+1];
        (*argc)--;
      }
      return TRUE;
    }
  }

  return FALSE;
}

#ifndef __TMS
/**
 * OS-independent implementation of the system function.
 *
 * @param lpsCommand
 *          Pointer to a null-terminated string that specifies the command line
 *          to execute.
 * @return The process termination status (-1 indicates an error creating the
 *         process)
 */
INT32 dlp_system(char* lpsCommand)
{
#ifdef __WIN32

  STARTUPINFO         si      = { sizeof(STARTUPINFO) };                        /* Process startup info struct       */
  PROCESS_INFORMATION pi      = { 0 };                                          /* Process info struct               */
  DWORD               nRetVal = 0;                                              /* Process return value              */

  if (!CreateProcess(NULL,lpsCommand,NULL,NULL,FALSE,                           /* Create Windows process            */
    GetPriorityClass(GetCurrentThread()),NULL,NULL,&si,&pi))                    /* |                                 */
  {                                                                             /* >> (Not successfull)              */
    return -1;                                                                  /*   Forget it...                    */
  }                                                                             /* <<                                */
  WaitForSingleObject(pi.hProcess,INFINITE);                                    /* Wait for process (to end?)        */
  if (!GetExitCodeProcess(pi.hProcess,&nRetVal)) nRetVal=0;                     /* Get return code                   */
  CloseHandle(pi.hProcess);                                                     /* Close process handle              */
  CloseHandle(pi.hThread);                                                      /* Close thread handle               */
  return (INT32)nRetVal;                                                          /* Return the process' exit code     */

#else                                                                           /* -- #ifdef __WIN32 --               */

  return (INT32)WEXITSTATUS(system(lpsCommand));                                  /* Call system function              */

#endif                                                                          /* -- #ifdef __WIN32 --               */
}
#endif

/**
 * Returns the current program's version info string in a static buffer.
 */
const char* dlp_get_version_info()
{
  static char __lpsVersionInfo[255];
  char        lpsOS[255];
  char        lpsMachine[255];
  char        lpsDr[32];
  char        lpsPid[32];
  char        lpsBuild[64] = "";

  strcpy(lpsOS     ,"");
  strcpy(lpsMachine,"");
  strcpy(lpsDr     ,"");
  sprintf(lpsPid," PID:%ld",(long)dlp_getpid());

#ifdef __DLP_BUILD
  dlp_strupr(dlp_strcpy(lpsBuild,__DLP_BUILD));
  lpsBuild[10]='\0';
#endif

  /* OS */
#if (defined __WIN32 || defined __MINGW32__ || defined __CYGWIN__)

  strcpy(lpsOS,"win32");

#elif defined __SPARC

  strcpy(lpsOS,"SUN-OS/Solaris");

#elif defined __OSF

  strcpy(lpsOS,"OSF");

#elif defined __ULTRIX

  strcpy(lpsOS,"Ultrix");

#elif defined __LINUX

  strcpy(lpsOS,"Linux");

#endif

  /* Machine */
#ifdef __I386

  strcpy(lpsMachine,"(ix86,");

#elif defined __SPARC

  strcpy(lpsMachine,"(sparc,");

#elif defined __ALPHA

  strcpy(lpsMachine,"(alpha,");

#elif defined __MIPS

  strcpy(lpsMachine,"(mips,");

#endif

  /* Long int bits */
#ifdef __LONG64
  strcat(lpsMachine,"64bit)");
#else
  strcat(lpsMachine,"32bit)");
#endif

  /* Debug or release */
#ifdef _DEBUG

  strcpy(lpsDr,"debug");

#else

  strcpy(lpsDr,"release");

#endif

  sprintf(__lpsVersionInfo,"%s version %s.%s",__lpBinaryName,__DLP_VERMAJ,__DLP_VERMIN);
  if (dlp_strlen(lpsBuild))
  {
    dlp_strcat(__lpsVersionInfo," build ");
    dlp_strcat(__lpsVersionInfo,lpsBuild);
  }
  dlp_strcat(__lpsVersionInfo,lpsOS     );
  dlp_strcat(__lpsVersionInfo,lpsMachine);
  dlp_strcat(__lpsVersionInfo,lpsDr     );
  dlp_strcat(__lpsVersionInfo,lpsPid    );
  return __lpsVersionInfo;
}

/**
 * Sets the main function's return value.
 *
 * @param nVal
 *          The return value
 */
void dlp_set_retval(INT32 nVal)
{
   __nRetVal = nVal;
}

/**
 * Returns the main function's intended return value.
 *
 * @return The return value
 */
INT32 dlp_get_retval()
{
  return __nRetVal;
}

/**
 * Generates and returns a random number between 0 and RAND_MAX
 *
 * @return The random number
 */
/*
INT64 dlp_rand()
{
  static BOOL bRandInit = FALSE;
  if(!bRandInit){
    srand((unsigned int)time(NULL));
    bRandInit=TRUE;
  }
  return rand();
}*/

/* dlp_rand_init
 * 0: initialize with new random seed
 * 1: initialize from last random seed
 * 2: initialize with fixed seed
 * 3: is initialized
 */
static INT8 dlp_rand_init = 0;

static UINT64 dlp_rand_seed[16];

static INT32 dlp_rand_p;

/**
 * If this function is called with bFix=TRUE,
 * the dlp_rand() will be switched to reproducable
 * values using a fixed seed.
 *
 * !!After usage you have to call this function
 * with bFix=FALSE to enable real random for other
 * users!!
 */
void dlp_rand_fix(BOOL bFix){
  if(bFix) dlp_rand_init=2;
  else     dlp_rand_init=1;
}

/**
 * Generates an equally distributed 64 bit random unsigned integer using the
 * XORshift1024* algorithm [1].
 *
 * <p>
 *   [1] http://en.wikipedia.org/wiki/Xorshift
 * </p>
 */
UINT64 dlp_rand()
{
  if (dlp_rand_init<3)
  {
    int i;
    switch(dlp_rand_init){
    case 1: srand((int)dlp_rand_seed[0]); break;
    case 2: srand(0); break;
    default: srand((int)time(NULL)^(int)dlp_getpid()); break; /* TODO: better seed? */
    }
    for (i=0; i<16; i++)
      dlp_rand_seed[i]
        = ((UINT64)rand())<<48 | ((UINT64)rand())<<32 | ((UINT64)rand())<<16
        | ((UINT64)rand());
    dlp_rand_init = 3;
  }

  UINT64 s0 = dlp_rand_seed[dlp_rand_p];
  UINT64 s1 = dlp_rand_seed[dlp_rand_p=(dlp_rand_p+1)&15];
  s1 ^= s1<<31;
  s1 ^= s1>>11;
  s0 ^= s0>>30;
  return (dlp_rand_seed[dlp_rand_p]=s0^s1)*1181783497276652981LL;
}

/**
 * Generates a random float numbers equally distributed between [0..1] using
 * the XOR-shift algorithm.
 */
FLOAT64 dlp_frand()
{
  return (FLOAT64)dlp_rand()/(FLOAT64)RAND_MAX;
}

/**
 * Sleep for a number of milli seconds
 */
void dlp_sleep(INT32 nMilliSec)
{
#ifdef __WIN32
  Sleep(nMilliSec);
#else
  usleep((useconds_t)nMilliSec*1000);
#endif
}

void dlp_register_signals() {
  (void)signal(SIGINT,dlp_interrupt);
}

void dlp_interrupt(int nSig) {
  if(nSig == SIGINT) {
    if(__bRequestInterrupt) { (void)signal(SIGINT,SIG_DFL);raise(SIGINT);
    } else {
      dlp_register_signals();
      printf("\n---Interrupt requested. Press again to quit.---\n");
      __bRequestInterrupt = TRUE;
    }
  }
}

void dlp_set_interrupt(BOOL bRequestInterrupt) {
  __bRequestInterrupt = bRequestInterrupt;
}

BOOL dlp_get_interrupt() {
  return __bRequestInterrupt;
}

/* EOF */
