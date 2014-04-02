/* dLabPro SDK class CHelloworld (Helloworld)
 * - Additional C/C++ methods
 *
 * AUTHOR : Matthias Wolff
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

#include "dlp_cscope.h" /* Indicate C scope */
#include "dlp_helloworld.h"


short CGEN_PUBLIC CHelloworld_Dm
(
  CHelloworld* _this,
  CData*       idSrc,
  long         nOversample,
  CData*       idDst
)
{
	short  b = 1;
	long   k;
	long   i;
	double a = 1.;
	double x;
	double c = 0.;

	CREATEVIRTUAL(CData,idSrc,idDst);
	CData_Scopy(idDst,idSrc);
	CData_Allocate(idDst,CData_GetNRecs(idSrc));

	for (k=0; k<CData_GetNRecs(idSrc); k++)
	{
		x = CData_Dfetch(idSrc,k,0);
		for (i=0; i<nOversample; i++)
		{
			if (c>=x) /* must go down */
			{
				if (_this->m_bAdm)
				{
					if      (b> 0) { a/=2; b=0; }
					else if (b<-1) { a*=2;      }
					b--;
					if (a<1) a=1;
					/*if (k==0 && i<100) printf("\nD i=%ld, x=%lg, c=%lg, a=%lg, b=%d",i,x,c,a,b);*/
				}
				c-=a;
			}
      else /* must go up */
      {
				if (_this->m_bAdm)
				{
					if      (b<0) { a/=2; b=0; }
					else if (b>1) { a*=2;      }
					b++;
					if (a<1) a=1;
					/*if (k==0 && i<100) printf("\nU i=%ld, x=%lg, c=%lg, a=%lg, b=%d",i,x,c,a,b);*/
				}
      	c+=a;
      }
			if (fabs(c-x)<1. && a==1.) break;
		}
		CData_Dstore(idDst,c,k,0);
	}

	DESTROYVIRTUAL(idSrc,idDst);
	return 0;
}

/* EOF */
