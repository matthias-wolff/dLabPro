/* dLabPro base library
 * - Debug assertions
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

#if (defined __MSVC && !defined _DEBUG)
  #pragma warning( disable : 4100 ) /* Unreferenzierter formaler Parameter */
#endif

/**
 * Prints an assertion failure message at stdout, prompts for user input
 * - A (return value 1): abort program,
 * - D (return value 2): start debugger,
 * - I (return value 3): continue program ignoring the assertion failure
 * and returns the respective return value.
 *
 * @param lpExpression Assertion failure message
 * @param lpFilename   Location of assertion failure in source code (file name)
 * @param nLine        Location of assertion failure in source code (line)
 * @return 1: abort program, 2: start debugger, 3: ignore and continue
 *         0: when called in release mode
 */
INT16 dlp_assert(const char* lpExpression, const char* lpFilename, INT32 nLine)
{

#ifdef _DEBUG

  INT16 nCode = 0;

  char lpAnswer[L_INPUTLINE];
  printf("\n\n ******* ASSERTION FAILURE *******************************"   );
  printf("\n\n  %s - Assertion Failure:",__PROJECT_NAME                     );
  printf("\n\n  > Expression: %s",lpExpression                              );
  printf(  "\n  > Location  : %s(%ld)",lpFilename,(long int)nLine           );
/*  printf("\n\n  See the programmer's reference book for more"               );
  printf(  "\n  information on how your program can cause an",__PROJECT_NAME);
  printf(  "\n  assertion failure."                                         );*/
  printf("\n\n *********************************************************"   );
  while (nCode == 0)
  {
    printf("\n  (A)bort/(D)ebug/(I)gnore? [%s Kernel]: ",__PROJECT_NAME);

    /* Test for valid stdin */
    if (ferror(stdin) || feof(stdin))
    {
      printf("\nBroken pipe <stdin>, so exiting...",0,0);
      exit(-1);
    }

    fgets(lpAnswer,L_INPUTLINE,stdin);
    switch (lpAnswer[0])
    {
    case 'a':
    case 'A': nCode = 1; break;
    case 'd':
    case 'D': nCode = 2; break;
    case 'i':
    case 'I': nCode = 3; break;
    }
  }

  return nCode;

#else /* #ifdef _DEBUG */

  return 0;

#endif /* #ifdef _DEBUG */
}

/* EOF */
