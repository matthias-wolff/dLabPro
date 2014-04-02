/* dLabPro class CDlpObject (object)
 * - Printing
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

#include "dlp_cscope.h" /* Indicate C scope */
#include "dlp_object.h"

/**
 * Prints the field named lpsName at stdout. If bInline is FALSE, the output
 * will start at a new line and have the form "fieldname = value", otherwise
 * just the value of the field will be printed without any leading or trailing
 * white space characters.
 *
 * If lpsName is "*\/all", all fields will be printed as a list, if lpsName is
 * "*" only writable fields will be listed.
 *
 * @param _this   This instance
 * @param lpsName Identifier of field to print
 * @param bInline If TRUE print short form in line, else start new line and
 *                print long form
 * @return O_K if successfull, a negative error code otherwise.
 */
INT16 CDlpObject_PrintField(CDlpObject* _this, const char* lpsName, BOOL bInline)
{
  BOOL     bAllHidden = FALSE;
  BOOL     bAll       = FALSE;
  SWord*   lpField;
  hscan_t  hs;
  hnode_t* hn;

  /* Verification */
  if (dlp_strlen(lpsName) <1) return IERROR(_this,ERR_NOTFIELD,lpsName,0,0);
#ifdef __NORTTI
  IERROR(_this,ERR_DANGEROUS,"CDlpObject_PrintField","Field printing not available in __NORTTI mode.",0);
  DLPASSERT(FALSE);
  return NOT_EXEC;
#endif

  /* Initialize */
  bAllHidden = (!strcmp(lpsName,"*/all") && !bInline);
  bAll       = (lpsName[0] == '*' && !bInline);

  hash_scan_begin(&hs,_this->m_lpDictionary);

  if (bAll) 
  {
    hn = hash_scan_next(&hs);
    if (!hn) 
    {
      printf(" [ Class %s has no fields ]",_this->m_lpClassName);
      return NOT_EXEC;
    }
    DLPASSERT((lpField = (SWord*)hnode_get(hn))!=NULL); /* NULL entry in dictionary */
    if (!lpField)
    {
      printf(" [ Class %s has no fields ]",_this->m_lpClassName);
      return NOT_EXEC;
    }
  }
  else 
  {
    lpField = CDlpObject_FindWord(_this,lpsName,WL_TYPE_FIELD);
    if (!lpField) return IERROR(_this,ERR_NOTFIELD,lpsName,0,0);
  }

  while (lpField)
  {
    if (lpField->nWordType == WL_TYPE_FIELD && (!(lpField->nFlags & FF_HIDDEN) || bAllHidden))
    {
      if (!bInline) 
      {
        char lpFQN[255]; CDlpObject_GetFQName(_this,lpFQN,FALSE);
        printf("\n %s.%-15s = ",lpFQN,lpField->lpName);
      }

      if (lpField->lpData)
      {
        switch (lpField->ex.fld.nType)
        {
          case T_BOOL    : printf("%hd"   ,(short         )*(     BOOL*)lpField->lpData); break;
          case T_UCHAR   : printf("%hhu"  ,(unsigned char )*(    UINT8*)lpField->lpData); break;
          case T_CHAR    : printf("%hhd"  ,(char          )*(     INT8*)lpField->lpData); break;
          case T_USHORT  : printf("%hu"   ,(unsigned short)*(   UINT16*)lpField->lpData); break;
          case T_SHORT   : printf("%hd"   ,(short         )*(    INT16*)lpField->lpData); break;
          case T_UINT    : printf("%u"    ,(unsigned int  )*(   UINT32*)lpField->lpData); break;
          case T_INT     : printf("%d"    ,(int           )*(    INT32*)lpField->lpData); break;
          case T_ULONG   : printf("%lu"   ,(unsigned long )*(   UINT64*)lpField->lpData); break;
          case T_LONG    : printf("%ld"   ,(long          )*(    INT64*)lpField->lpData); break;
          case T_FLOAT   : printf("%G"    ,(double        )*(  FLOAT32*)lpField->lpData); break;
          case T_DOUBLE  : printf("%G"    ,(double        )*(  FLOAT64*)lpField->lpData); break;
          case T_COMPLEX : { char sTmp[L_SSTR];dlp_sprintc(sTmp,*((COMPLEX64*)lpField->lpData),FALSE);printf("%s",sTmp); break; }
          case T_TEXT    :
          case T_CSTRING :
          case T_STRING  :
            if (*(char**)lpField->lpData) printf("'%s'",*(char**)lpField->lpData); 
            else                          printf("''");
            break;
/* FIXME: GCC won't print 64 bits pointers correctly! */
#ifdef _MSC_VER
          case T_PTR     : printf("0x%0p",(__int64)*(void**)lpField->lpData); break;
#else
          case T_PTR     : printf("0x%0p",(long)*(void**)lpField->lpData); break;
#endif
          case T_INSTANCE:
          {
            CDlpObject* iInst = (CDlpObject*)*(void**)lpField->lpData;
            if (!iInst) printf("(none)");
            else
            {
              if (CDlpObject_GetParent(iInst)!=_this)
              {
                char lpBuf[255];
                CDlpObject_GetFQName(iInst,lpBuf,FALSE);
                printf("%s (*)",lpBuf);
              }
              else printf("%s",iInst->m_lpInstanceName);
            }
            break; 
          }
        }
        if (lpField->ex.fld.nType >0 && lpField->ex.fld.nType <=256) 
          printf("'%s'",(char*)lpField->lpData);
      }
      else printf("<Field pointer is NULL>");
    }

    if (!bAll) break;
    if ((hn = hash_scan_next(&hs))==NULL) break;
    DLPASSERT((lpField = (SWord*)hnode_get(hn))!=NULL); /* NULL entry in dictionary */
  }

  if (!bInline) printf("\n\n");
  return O_K;
}

/**
 * Prints one line of the class member listing at stdout.
 *
 * @param _this  This instance
 * @param lpWord Pointer to a SWord structure identifying the word to print
 * @param nHow   Specifies the output format, one of the PWL_XXX constants
 * @param bLast  TRUE denotes printing the last item of the member list
 * @see CDlpObject_PrintAllMembers
 */
void CDlpObject_PrintMember(CDlpObject* _this, SWord* lpWord, INT16 nHow, BOOL bLast)
{
  char lpAux[255];

  CHECK_THIS();

  if (!lpWord) return;

  printf("\n   %c   %c-- %-18s",lpWord->nWordType==WL_TYPE_INSTANCE?' ':'|',
    bLast?'\'':'|',lpWord->lpName);

  if (nHow == PWL_HELP) 
  {
    switch (lpWord->nWordType)
    {
      case WL_TYPE_INSTANCE:
        printf("Class %s",((CDlpObject*)lpWord->lpData)->m_lpClassName);
        return;
      case WL_TYPE_OPERATOR:
        if (strlen(lpWord->ex.op.lpsSig))  printf("%-15s",lpWord->ex.op.lpsSig);
        break;
    }
    if (dlp_strlen(lpWord->lpComment)) printf("%s",lpWord->lpComment);
    return;
  }

  switch (lpWord->nWordType)
  {
  case WL_TYPE_METHOD:
    if (dlp_strlen(lpWord->ex.mth.lpSyntax) >0) printf("%s ",lpWord->ex.mth.lpSyntax);
    printf("%s ",lpWord->lpName);
    if (dlp_strlen(lpWord->ex.mth.lpPostsyn) >0) printf("%s",lpWord->ex.mth.lpPostsyn);
    break;
  case WL_TYPE_OPTION:
    break;
  case WL_TYPE_FIELD: 
    if (lpWord->nFlags & FF_HIDDEN      ) strcpy(lpAux,"h"); else strcpy(lpAux,"-");
    if (lpWord->nFlags & FF_NOSET       ) strcat(lpAux,"-"); else strcat(lpAux,"w");
    if (lpWord->nFlags & FF_NOSAVE      ) strcat(lpAux,"-"); else strcat(lpAux,"s");
    if (lpWord->nFlags & FF_NONAUTOMATIC) strcat(lpAux,"-"); else strcat(lpAux,"a");
    printf("%s ",lpAux);

    memset(lpAux,0,255);
    if (lpWord->ex.fld.nType == T_INSTANCE)
    {
      sprintf(lpAux,"INSTANCE(\"%s\")",lpWord->ex.fld.lpType);
      printf("%-20s %s",lpAux,lpWord->lpName);
    }
    else if (lpWord->ex.fld.nType == T_PTR)
    {
      sprintf(lpAux,"%s*",lpWord->ex.fld.lpType);
      printf("%-20s %s",lpAux,lpWord->lpName);
    }
    else printf("%-20s %s",dlp_get_type_name(lpWord->ex.fld.nType),lpWord->lpName);
    break;
  case WL_TYPE_FACTORY: 
    break;
  case WL_TYPE_INSTANCE: 
    break;
  default: DLPASSERT(FALSE); /* Unknown word type */
  }
}

/**
 * Prints a listing of all class members of type nWordType at stdout.
 *
 * @param _this      This instance
 * @param nWordType Type of members to list, one of the WL_TYPE_XXX constants
 * @param nHow      Specifies the output format, one of the PWL_XXX constants
 * @see CDlpObject_PrintMember
 * @see CDlpObject_PrintDictionary
 */
void CDlpObject_PrintAllMembers(CDlpObject* _this, INT16 nWordType, INT16 nHow)
{
  INT16     i;
  INT16     nWords  = 0;
  hash_t*   lpDict  = NULL;
  hscan_t   hs;
  hnode_t*  hn;
  SWord**   lpWords = NULL;
  SWord*    lpWord  = NULL;
  SWord*    lpPrev;

  CHECK_THIS();
#ifdef __NORTTI
  IERROR(_this,ERR_DANGEROUS,"CDlpObject_PrintAllMembers","Printing of members not available in __NORTTI mode.",0);
  DLPASSERT(FALSE);
  return;
#endif

  switch (nWordType)
  {
  case WL_TYPE_METHOD  : printf("\n   |-- METHODS"  ); lpDict = _this->m_lpDictionary; break;
  case WL_TYPE_OPERATOR: printf("\n   |-- OPERATORS"); lpDict = _this->m_lpOpDict;     break;
  case WL_TYPE_OPTION  : printf("\n   |-- OPTIONS"  ); lpDict = _this->m_lpDictionary; break;
  case WL_TYPE_FIELD   : printf("\n   |-- FIELDS"   ); lpDict = _this->m_lpDictionary; break;
  case WL_TYPE_FACTORY : printf("\n   |-- CLASSES"  ); lpDict = _this->m_lpDictionary; break;
  case WL_TYPE_INSTANCE: printf("\n   '-- INSTANCES"); lpDict = _this->m_lpDictionary; break;
  default              : DLPASSERT(FALSE); /* Unsupported word type */
  }

  /* Iterate dictionary */
  lpPrev = NULL;

  hash_scan_begin(&hs,lpDict);
  while ((hn = hash_scan_next(&hs))!=NULL)
  {
    DLPASSERT((lpWord = (SWord*)hnode_get(hn))!=NULL); /* NULL entry in dictionary */

    if (lpWord->nWordType == nWordType                             &&
      (nWordType!=WL_TYPE_FIELD || !(lpWord->nFlags & FF_HIDDEN)) )
    {
      if (lpPrev) {
        lpWords = (SWord**)dlp_realloc(lpWords,nWords+1,sizeof(SWord*));
        lpWords[nWords++] = lpPrev;
      }
      lpPrev = lpWord;
    }
  }
  if (lpPrev) {
    lpWords = (SWord**)dlp_realloc(lpWords,nWords+1,sizeof(SWord*));
    lpWords[nWords++] = lpPrev;
  }

  qsort(lpWords,nWords,sizeof(SWord*),CDlpObject_CompareWordsByName);

  for(i = 0; i < nWords-1; i++) {
    CDlpObject_PrintMember(_this,lpWords[i],nHow,FALSE);
  }
  if(nWords) CDlpObject_PrintMember(_this,lpWords[i],nHow,TRUE);

  if (nWordType != WL_TYPE_INSTANCE) printf("\n   |");

  dlp_free(lpWords);
}

/**
 * Print the classes version info at stdout.
 *
 * @param _this      This instance
 */
void CDlpObject_PrintVersionInfo(CDlpObject* _this)
{
  CHECK_THIS();

  printf("\n---------- %s: Version %s %s ----------\n",_this->m_lpInstanceName,_this->m_version.no,_this->m_version.date);
}

/**
 * Prints a listing of all class members and all local instances.
 *
 * @param _this      This instance
 * @param nHow      Specifies the output format, one of the PWL_XXX constants
 * @see CDlpObject_PrintAllMembers
 */
void CDlpObject_PrintDictionary(CDlpObject* _this, INT16 nHow)
{
  CHECK_THIS();

  printf("\n");
  dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  printf("\n   %s for instance\n   %s %s",
    nHow==PWL_HELP?"Help":"Syntax",_this->m_lpClassName,
    _this->m_lpInstanceName);
  
  /* Print header */
  printf("\n");
  dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  printf("\n   %s",_this->m_lpInstanceName);
  printf("\n   |");

  /* Print members */
  CDlpObject_PrintAllMembers(_this,WL_TYPE_METHOD  ,nHow);
  CDlpObject_PrintAllMembers(_this,WL_TYPE_OPERATOR,nHow);
  CDlpObject_PrintAllMembers(_this,WL_TYPE_OPTION  ,nHow);
  CDlpObject_PrintAllMembers(_this,WL_TYPE_FIELD   ,nHow);
  CDlpObject_PrintAllMembers(_this,WL_TYPE_FACTORY ,nHow);
  CDlpObject_PrintAllMembers(_this,WL_TYPE_INSTANCE,nHow);

  /* Print footer */
  printf("\n\n");  
  dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  printf("\n");  
}

/* EOF */
