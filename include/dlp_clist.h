// dLabPro base library
// - Implementation file
//
// AUTHOR : Matthias Wolff
// PACKAGE: dLabPro/sdk
// 
// Copyright 2013 dLabPro contributors and others (see COPYRIGHT file) 
// - Chair of System Theory and Speech Technology, TU Dresden
// - Chair of Communications Engineering, BTU Cottbus
// 
// This file is part of dLabPro.
// 
// dLabPro is free software: you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.
// 
// dLabPro is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
// details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with dLabPro. If not, see <http://www.gnu.org/licenses/>.

#ifndef __CLIST_H
#define __CLIST_H

// String list items
typedef struct tagSStr 
{
  char     lpName[512];
  INT32      nId;
  tagSStr* next;
} SStr;

// Template Class CList
template <class S> class CList 
{

public:    S* m_items;
protected: S* m_last;

public:
CList()
{
  m_items = NULL;
  m_last  = NULL;
}

~CList()
{
  Delete();
}

void Delete()
{
  S *page = m_items, *page2;
  while (page !=NULL) 
  {
    page2 = page->next; 
    delete(page); 
    page = page2;
  }
  m_items = NULL;
  m_last  = NULL;
}

S* AddItem(const char* lpText)
{
  INT32 newId = 0;
  S*  page  = NULL;

  if (m_items == NULL)
  {
    m_items = new(S);
    page = m_items;
  }
  else 
  {
    if (m_last)
    {
      m_last->next = new(S);
      page = m_last->next;
      newId = m_last->nId+1;
    }
    else
    {
      page = m_items;
      newId++;
      while (page->next !=NULL) 
      {
        page = page->next; 
        newId++;
      }
      page->next = new(S);
      page = page->next;
    }
  }

  if (page == NULL) return NULL;

  dlp_memset(page,0,sizeof(S));
  page->next = NULL;
  page->nId  = newId;
  dlp_strcpy(page->lpName,lpText);

  m_last = page;
  return page;
}

S* InsertItem(S* lpPrec, const char* lpText)
{
  INT32 newId = 0;

  S *page = m_items, *page2;
  if (page ==NULL)
  {
    m_items = new(S);
    page = m_items;
    dlp_memset(page,0,sizeof(S));
    page->next = NULL;            // just in case NULL is not 0x00000000
    m_last = m_items;
  }
  else 
  {
    if (!lpPrec)                  // insert at first postition
    {
      page2 = m_items;
      m_items = new(S);
      dlp_memset(m_items,0,sizeof(S));
      m_items->next = page2;
      page = m_items;
      if (!page->next) m_last=page;
    }
    else
    {
      while (page->next !=NULL) 
      {
        if (page == lpPrec) break;
        page = page->next; newId++;
      }
      page2 = page->next;
      page->next = new(S);
      dlp_memset(page->next,0,sizeof(S));
      page->next->next = page2;
      page = page->next;
      if (!page->next) m_last=page;
    }
  }
  if (page == NULL) return NULL;

  page->nId = newId;
  dlp_strcpy(page->lpName,lpText);

  page2 = page;
  while (page != NULL)
  {
    page->nId++;
    page = page->next;
  }

  return page2;
}

S* DeleteItem(S* lpItem)
{
  if (!m_items) return FALSE;
  if (!lpItem) return FALSE;

  S* pageTmp;

  if (lpItem == m_items)
  {
    m_items = m_items->next;
    delete(lpItem);
    lpItem=m_items;
    while (lpItem != NULL )
    {
      lpItem->nId--;
      lpItem = lpItem->next;
    }
    if (!m_items || !m_items->next) m_last=m_items;
    return m_items;
  }

  S* page = m_items;
  while (page->next) 
  {
    if (page->next == lpItem) break;
    page = page->next;
  }
  if (page->next != lpItem) return NULL;

  page->next = lpItem->next;
  delete(lpItem);
  if (!page->next) m_last=page;

  pageTmp=page=page->next;
  while (page != NULL )
  {
    page->nId--;
    page = page->next;
  }

  return pageTmp;
}

S* DeleteItem(INT64 lItem)
{
  S* page;

  if (!m_items) return FALSE;

  if (lItem == m_items->nId)
  {
    page=m_items;
    m_items = m_items->next;
    delete(page);
    if (!m_items || !m_items->next) m_last=m_items;
    page=m_items;
    while (page != NULL )
    {
      page->nId--;
      page = page->next;
    }
    return m_items;
  }

  S* pageTmp;
  page = m_items;
  while (page->next) 
  {
    if (page->next->nId == lItem) break;
    page = page->next;
  }
  if (page->next->nId != lItem) return NULL;

  pageTmp=page->next;
  page->next = page->next->next;
  delete(pageTmp);
  if (!page->next) m_last=page;

  pageTmp=page=page->next;
  while (page != NULL )
  {
    page->nId--;
    page = page->next;
  }

  return pageTmp;
}

S* PopItem()
{
  if (!m_items) return NULL;

  S* lpReturn = m_items;
  m_items = m_items->next;
  lpReturn->next = NULL;
  S* page = m_items;
  while (page)
  {
    page->nId--;
    page = page->next;
  }
  return lpReturn;
}

S* PushItem(char* lpText)
{
  S* page = m_items;
  m_items = new(S);
  m_items->next = page;
  if (!m_items->next) m_last=m_items;
  m_items->nId = 0;
  dlp_strcpy(m_items->lpName,lpText);

  while (page != NULL) {page->nId++; page = page->next;}

  return m_items;
}

S* FindItem(const char *lpText)
{
  S* page = m_items;
  while (page !=NULL) 
  {
    if (dlp_strncmp(page->lpName,lpText,L_NAMES) ==0) return page;
    page = page->next;
  }
  return NULL;
}

S* FindItem(int lpItem)
{
  S* page = m_items;
  while (page !=NULL) 
  {
    if (page->nId==lpItem) return page;
    page = page->next;
  }
  return NULL;
}

S* FindLastItem()
{
  if (m_last) return m_last;

  S* page = m_items;
  if (!page) return NULL;
  while (page->next !=NULL) page = page->next;
  return page;
}

void PrintList()
{
  S* page = m_items;
  while (page) 
  {
    printf("  (%3d) %s\n",page->nId,page->lpName);
    page = page->next;
  }
}

INT16 Copy(CList<S>* lDestination)
{
  lDestination->Delete();

  S* pageS = m_items;
  while (pageS)
  {
    S* pageD = lDestination->AddItem("");
    *pageD=*pageS;
    pageD->next = NULL;

    pageS = pageS->next;
  }

  return TRUE;
}

INT16 IsListEmpty()
{
  if (!m_items) return TRUE;

  S* page = m_items;
  while (page)
  {
    if (page->lpName && strlen(page->lpName) >0) return FALSE;
    page = page->next;
  }

  return TRUE;
}


INT64 Count()
{
  INT64 nCnt = 0;
  S*   page = NULL;
  for (page=m_items,nCnt=0; page; page=page->next) nCnt++;
  return nCnt;
}

INT16 SortUp()
{
  if (!m_items) return TRUE;

  INT64 nCnt = Count();
  INT64 nA   = 0;
  INT64 nB   = 0;
  S**  lpSi = (S**)__dlp_calloc(nCnt+1,sizeof(S*),"clist.h",351,"CList<S>","");
  S*   page = NULL;
  
  for (page=m_items,nA=0; page; page=page->next,nA++) lpSi[nA]=page;

  for (nA=0; nA<nCnt; nA++)
    for (nB=nA+1; nB<nCnt; nB++)
      if (dlp_stricmp(lpSi[nA]->lpName,lpSi[nB]->lpName)>0)
      {
        S* ghost = lpSi[nA];
        lpSi[nA] = lpSi[nB];
        lpSi[nB] = ghost;
      }

  m_items = lpSi[0];
  for (nA=0; nA<nCnt; nA++) lpSi[nA]->next = lpSi[nA+1];

  dlp_free(lpSi);

  m_last=NULL;
  FindLastItem();
      
  return TRUE;
}

};

#endif //#ifndef __CLIST_H

// EOF
