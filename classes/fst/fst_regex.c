/* dLabPro class CFst (fst)
 * - optimization methods
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
#include "dlp_fst.h"

#define RGX_ANY    0x100
#define RGX_NO     0x101
#define RGX_START  0x102
#define RGX_END    0x103
#define RGX_NONE   -1

/* Test skript
#!/usr/bin/env dlabpro

fst f;
data chk;

{
	{ "a*b"      "aaaabx"       0 5 }
	{ "a*b"      "bx"           0 1 }
	{ "a+b"      "aaaabx"       0 5 }
	{ "a?b"      "abx"          0 2 }
	{ "a?b"      "bx"           0 1 }
	{ "(ab)+c"   "ababcd"       0 5 }
	{ "(ab)+c"   "xyzababc"     3 5 }
	{ "a\\*c"    "za*c"         1 3 }
	{ "f..k"     "zfolky"       1 4 }
	{ "z.*b"     "xyzababc"     2 5 }
	{ "a?b"      "aaab"         2 2 }
	{ "a?a"      "aa"           0 2 }
	{ "a?"       "a"            0 1 }
	{ ".*b"      "bc"           0 1 }
	{ "a*"       "zb"           0 0 }

	{ "[abc]"    "zab"          1 1 }
	{ "[^xyz]"   "zab"          1 1 }
	{ "[0-9]"    "z8t"          1 1 }
	{ "[0-9]+"   "abc337xyz"    3 3 }

	{ "[^a-z]"   "123xyz"       0 1 }
	{ "a\\d+b"   "za789b"       1 5 }
	{ "c\\A*"    "abc12.."      2 5 }

	{ "^ab"      "abc12.."      0  2 }
	{ "^ab"      "axc12.."     -1 -2 }
	{ "3a$"      "abc123a"      5  2 }
	{ "4a$"      "axc123a"     -1 -2 }
} chk =;

var s;
var l;
0 var n; label LR; n chk.nrec < if;
	"\n${chk[n,0]}  ${chk[n,1]}  ${chk[n,2]} ${chk[n,3]}" -echo;
	:chk[n,0]: f -regex_compile;
	:chk[n,1]: f -regex_match s = l =;
	s :chk[n,2]: == l :chk[n,3]: == && if;
		" => ok" -echo;
	else;
		" => err: $[s] $[l]" -echo;
		f -print;
		quit;
	endif;
n ++=; goto LR; endif;

"\n" -echo;
quit;
*/


/*
 * Generate one non-final state in unit 0 in itDst.
 *
 * @return   number of the state
 */
INT32 CGEN_IGNORE rgx_addstate(CFst *itDst)
{
  CFst_Addstates(itDst,0,1,FALSE);
  return UD_XS(itDst,0)-1;
}

/*
 * Add one transition to unit 0 in itDst.
 * Destination state nE of transition is generated if negative.
 *
 * @return   destination state of transition
 */
INT32 CGEN_IGNORE rgx_addtrans(CFst *itDst,INT32 nS,INT32 nE,INT32 nTis)
{
  if(nE<0) nE=rgx_addstate(itDst);
  CFst_AddtransEx(itDst,0,nS,nE,nTis,-1,0);
  return nE;
}

/*
 * Parse a selection in the regex string (i.e. [a-z]).
 *
 * @param sRgx  selection to parse
 * @param itDst regex fst to generate
 * @param nS    starting state of selection
 * @param nE    return ending state of selection
 * @return      new position in sRgx (at ']' or end of string)
 */
const char* CGEN_IGNORE rgx_select(const char* sRgx,CFst* itDst,INT32 nS,INT32* nE)
{
  *nE=rgx_addstate(itDst);
  if(sRgx[0]=='^'){
    nS=rgx_addtrans(itDst,nS,-1,RGX_NO);
    sRgx++;
  }
  while(sRgx[0] && sRgx[0]!=']'){
    if(sRgx[0]=='\\') if(!(++sRgx)[0]) break;
    rgx_addtrans(itDst,nS,*nE,sRgx[0]);
    if(sRgx[1]=='-' && sRgx[2] && sRgx[2]!=']'){
      unsigned char cI = sRgx[2];
      unsigned char cE = sRgx[0];
      while(cI!=cE){
        rgx_addtrans(itDst,nS,*nE,cI);
        if(cI>cE) cI--; else cI++;
      }
      sRgx+=2;
    }
    sRgx++;
  }
  return sRgx;
}

struct rgx_select_auto {
  const char cLab;
  const char *sRgx;
};
const struct rgx_select_auto rRgxSelectAuto[] = {
  { 'd', "0-9" },
  { 'D', "^0-9" },
  { 'x', "0-9A-Fa-f" },
  { 'X', "^0-9A-Fa-f" },
  { 'o', "0-7" },
  { 'O', "^0-7" },
  { 'w', "0-9A-Za-z_" },
  { 'W', "^0-9A-Za-z_" },
  { 'h', "0-9A-Za-z" },
  { 'H', "^0-9A-Za-z" },
  { 'a', "A-Za-z" },
  { 'A', "^A-Za-z" },
  { 'l', "a-z" },
  { 'L', "^a-z" },
  { 'u', "A-Z" },
  { 'U', "^A-Z" },
  { '\0', NULL },
};

/*
 * Parse one token in sRgx.
 *
 * @param sRgx  regex string to parse
 * @param itDst regex fst to generate
 * @param nS    current state in itDst
 * @return      new position in sRgx (after token)
 */
const char* CGEN_IGNORE rgx_findtok(const char* sRgx,CFst* itDst,INT32 *nS)
{
  const char *sEnd;
  INT32 nE,nT;

  /*printf("rgx_findtok %s %i\n",sRgx,*nS);*/

  switch(sRgx[0]){
    case '(':
      sEnd=sRgx+1;
      nE=*nS;
      while(sEnd && sEnd[0] && sEnd[0]!=')') sEnd=rgx_findtok(sEnd,itDst,&nE);
      if(!sEnd){ printf("\nError: no matching ')' found"); return NULL; }
      sEnd++;
    break;
    case '[':
      sEnd=rgx_select(sRgx+1,itDst,*nS,&nE);
      if(!sEnd || sEnd[0]!=']'){ printf("\nError: no matching ']' found"); return NULL; }
      sEnd++;
    break;
    case '.':
      sEnd=sRgx+1;
      nE=rgx_addtrans(itDst,*nS,-1,RGX_ANY);
    break;
    case '^':
      sEnd=sRgx+1;
      nE=rgx_addtrans(itDst,*nS,-1,RGX_START);
    break;
    case '$':
      sEnd=sRgx+1;
      nE=rgx_addtrans(itDst,*nS,-1,RGX_END);
    break;
    case '\\':
      sRgx++;
      nT=0;
      for(nT=0;rRgxSelectAuto[nT].cLab;nT++) if(rRgxSelectAuto[nT].cLab==sRgx[0]){
        rgx_select(rRgxSelectAuto[nT].sRgx,itDst,*nS,&nE);
        break;
      }
      if(rRgxSelectAuto[nT].cLab){ sEnd=sRgx+1; break; }
    default:
      sEnd=sRgx+1;
      nE=rgx_addtrans(itDst,*nS,-1,sRgx[0]);
    break;
  }
  
  switch(sEnd[0]){
    case '*':
      rgx_addtrans(itDst,nE,*nS,-1);
      nE=rgx_addtrans(itDst,*nS,-1,-1);
      sEnd++;
    break;
    case '+':
      rgx_addtrans(itDst,nE,*nS,-1);
      sEnd++;
    break;
    case '?':
      nT=rgx_addtrans(itDst,nE,-1,-1);
      rgx_addtrans(itDst,*nS,nT,-1);
      nE=nT;
      sEnd++;
    break;
  }

  *nS=nE;
  return sEnd;
}

/*
 * Manual page at fsttools.def
 */
INT16 CGEN_PUBLIC CFst_RegexCompile(CFst* _this,const char* sRgx)
{
  char lpsName[L_SSTR];
  INT32 nS=0;

  CHECK_THIS_RV(NOT_EXEC);

  snprintf(lpsName,L_SSTR,"Regex: %s",sRgx);
  CFst_Reset(BASEINST(_this),TRUE);
  ISETOPTION(_this,"/fsa");
  CFst_Addunit(_this,lpsName);
  CFst_Addstates(_this,0,1,FALSE);

  while(sRgx && sRgx[0]) sRgx=rgx_findtok(sRgx,_this,&nS);

  if(sRgx){
    SD_FLG(_this,nS)|=SD_FLG_FINAL;
    return O_K;
  }else{
    CFst_Reset(BASEINST(_this),TRUE);
    return NOT_EXEC;
  }
}

/*
 * Matches the first characters in sStr against an inverse selection list.
 *
 * @param sStr  pointer to character for match
 * @param itSrc regex fst
 * @param lpTI  iterator for itSrc
 * @param nS    starting state in itSrc
 * @return      returns the new starting state in itSrc or negativ for no match
 */
INT32 CGEN_IGNORE rgx_matchno(const char* sStr,CFst *itSrc,FST_TID_TYPE *lpTI,INT32 nS)
{
  BYTE *lpT=NULL;
  INT32 nE =-1;
  if(SD_FLG(itSrc,nS)&SD_FLG_FINAL) return -1;
  while((lpT=CFst_STI_TfromS(lpTI,nS,lpT))){
    INT32 nTis = *CFst_STI_TTis(lpTI,lpT);
    if(nTis==sStr[0]) return -1;
    nE = *CFst_STI_TTer(lpTI,lpT);
  }
  return nE;
}

/*
 * Matches the sStr against itSrc from state nS.
 *
 * @param sStr  string for match
 * @param itSrc regex fst
 * @param lpTI  iterator for itSrc
 * @param nS    starting state in itSrc
 * @return      number of characters matching or negative for no match
 */
INT32 CGEN_IGNORE rgx_match(const char* sStr,CFst *itSrc,FST_TID_TYPE *lpTI,INT32 nS,BOOL bStart)
{
  BYTE *lpT=NULL;
  INT32 nRmax=-1;
  /*printf("rgx_match: %s %i\n",sStr,nS);*/
  if(SD_FLG(itSrc,nS)&SD_FLG_FINAL) nRmax=0;
  while((lpT=CFst_STI_TfromS(lpTI,nS,lpT))){
    INT32 nTis = *CFst_STI_TTis(lpTI,lpT);
    INT32 nE   = *CFst_STI_TTer(lpTI,lpT);
    INT32 nR=-1;
    INT32 nI=-1;
    if(nTis>=0 && nTis!=RGX_END && !sStr[0]) continue;
    switch(nTis){
      case RGX_NONE: nI=0; break;
      case RGX_ANY:  nI=1; break;
      case RGX_NO:
        nE=rgx_matchno(sStr,itSrc,lpTI,nE);
        if(nE>=0) nI=1;
      break;
      case RGX_START: if(bStart)   nI=0; break;
      case RGX_END:   if(!sStr[0]) nI=0; break;
      default: if(nTis==sStr[0])   nI=1; break;
    }
    if(nI>=0) nR=rgx_match(sStr+nI,itSrc,lpTI,nE,bStart && nI==0)+nI;
    if(nR>nRmax) nRmax=nR;
    /*printf("=> %i %i\n",nR,nRmax);*/
  }
  if(nRmax>=0) return nRmax;
  return -2;
}

INT16 CGEN_PUBLIC CFst_RegexMatch_int(CFst* _this,const char* sStr,INT32 *nS,INT32 *nL)
{
  FST_TID_TYPE *lpTI;

  CHECK_THIS_RV(NOT_EXEC);
  if(UD_XXU(_this)<1)  return IERROR(_this,FST_BADID,"unit count",UD_XXU(_this),0);
  if(UD_XS(_this,0)<1) return IERROR(_this,FST_BADID,"state count",UD_XS(_this,0),0);
  if(!sStr) return IERROR(_this,FST_INVALID,"input string is NULL",0,0);

  lpTI = CFst_STI_Init(_this,0,FSTI_SORTINI);
  nL[0]=-1;

  for(nS[0]=0;sStr[nS[0]];nS[0]++)
    if((nL[0]=rgx_match(sStr+nS[0],_this,lpTI,0,TRUE))>=0) break;

  CFst_STI_Done(lpTI);
  if(nL[0]<0) nS[0]=-1;

  return O_K;
}

/*
 * Manual page at fsttools.def
 */
INT16 CGEN_PUBLIC CFst_RegexMatch(CFst* _this,const char* sStr)
{
  INT32 nS,nL;
  INT16 nErr;
  IF_NOK((nErr=CFst_RegexMatch_int(_this,sStr,&nS,&nL))) return nErr;
  MIC_PUT_N((FLOAT64)nL);
  MIC_PUT_N((FLOAT64)nS);
  return O_K;
}
