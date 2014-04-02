/* dLabPro base library
 * - Thread methods
 *
 * AUTHOR : Matthias Eichner
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
#include <time.h>

/**
 * Returns the current process ID.
 */
INT32 dlp_getpid()
{
#ifdef __TMS
  return 0;
#else
  return getpid();
#endif
}

/*
#if (defined __GNUC__ && !defined __MINGW32__)
  #include <sys/times.h>
#else
  #include <windows.h>
  #include <winuser.h>
#endif
*/
/**
 * Creates a thread.
 *
 * @param ThreadFunc  The thread function
 * @param lpThreadArg Argument to thread function
 * @return Handle to created thread or 0 if an error occured
 * @see #dlp_terminate_thread dlp_terminate_thread
 * @see #dlp_join_thread dlp_join_thread
 */
THREADHANDLE dlp_create_thread(THREADFUNC, void* lpThreadArg)
{

#ifdef HAVE_MSTHREAD
 UINT64 nThreadID;

 return CreateThread(NULL,0,ThreadFunc,lpThreadArg,0,&nThreadID);

#elif defined HAVE_PTHREAD

  THREADHANDLE    lpThread;
  pthread_attr_t  pthread_custom_attr;

  pthread_attr_init(&pthread_custom_attr);
  pthread_create(&lpThread, &pthread_custom_attr, ThreadFunc, lpThreadArg);

  return lpThread;

#else
  DLPASSERT(FALSE);
  return NULL;
#endif
}

/**
 * Join a thread. This method suspends the execution of the
 * calling thread until the thread identified by hThread terminates.
 *
 * @param hTread    Handle of thread to join
 * @return nonzero if succeeded, zero if an error occured. Call
 *         GetLastError to get extended error information (MS
 *         OSes only)
 * @see #dlp_create_thread dlp_create_thread
 * @see #dlp_terminate_thread dlp_terminate_thread
 */
INT16 dlp_join_thread(THREADHANDLE hThread)
{

#ifdef HAVE_MSTHREAD

 WaitForSingleObject(hThread,INFINITE);

#elif defined HAVE_PTHREAD

  pthread_join(hThread, NULL);

#else
  DLPASSERT(FALSE);
#endif

  return O_K;
}

/**
 * Terminates a thread.
 *
 * @param hTread    Handle of thread to terminate
 * @param nExitCode Exit code to send to thread (MS OSes only)
 * @return nonzero if succeeded, zero if an error occured. Call
 *         GetLastError to get extended error information (MS
 *         OSes only)
 * @see #dlp_create_thread dlp_create_thread
 * @see #dlp_join_thread dlp_join_thread
 */
INT16 dlp_terminate_thread(THREADHANDLE hThread, INT32 nExitCode)
{
#ifdef HAVE_MSTHREAD

 INT16 bResult = TerminateThread(hThread,nExitCode);
 CloseHandle(hThread); /* This handle is not longer valid! */
 return bResult;

#elif defined HAVE_PTHREAD

 pthread_kill(hThread,9);

#else
  DLPASSERT(FALSE);
#endif

 return O_K;
}

/**
 * Returns the current system time in milliseconds. The value is only useful
 * to measure the elapsed time between subsequent calls of the function. The
 * absolute time value depends on the system.
 *
 * @return The system time in milliseconds.
 */
UINT64 dlp_time()
{
  time_t nTime;
  time(&nTime);
  nTime*=1000;

  {
    #if defined __GNUC__ || defined __TMS
      nTime += (time_t)(clock()/(time_t)((FLOAT64)CLOCKS_PER_SEC/1000.));
    #else
      MMTIME rNow; timeGetSystemTime(&rNow,sizeof(MMTIME));
      nTime += (time_t)rNow.u.ms%1000;
    #endif
  }

  return (UINT64)nTime;
}

/* EOF */
