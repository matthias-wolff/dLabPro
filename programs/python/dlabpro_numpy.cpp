#include "Python.h"
#include "arrayobject.h"
#include "dlp_base.h"
#include "dlp_object.h"
#include "dlp_data.h"
#include "dlabpro_numpy.hpp"

#undef data

void numpy2data(PyObject *np,CData *dat){
  int d,i,c=0,r=0,b=0;
  INT16 t=0;
  PyArrayObject *na=(PyArrayObject*)np;
  CData_Reset(dat,TRUE);
  if(na->nd==1){
    c=1;
    r=na->dimensions[0];
    b=0;
  }else if(na->nd==2){
    c=na->dimensions[1];
    r=na->dimensions[0];
    b=0;
  }else if(na->nd==3){
    c=na->dimensions[2];
    r=na->dimensions[0]*na->dimensions[1];
    b=na->dimensions[0];
  }else{ printf("ERROR: only up to dim 3\n"); return; }
  /* https://docs.scipy.org/doc/numpy-1.17.0/reference/c-api.types-and-structures.html */
  /* [kind] b:bool, i:signed int, u:unsigned int, f:float, c:complex, S:bytes, U:string, V:arbitrary */
  /* [byteorder] >:big-endian <:little-endian =:native |:irrelevant */
  if(na->descr->byteorder=='|' && na->descr->kind=='S' && na->descr->type=='S' && na->strides[na->nd-1]<=255) t=na->strides[na->nd-1]; /* bytes type */
  else if(na->descr->byteorder!='='){ printf("ERROR: unknown dtype #1 (byteorder: %c kind: %c type: %c strides[-1]: %i\n",na->descr->byteorder,na->descr->kind,na->descr->type,na->strides[na->nd-1]); return; }
  else if(na->descr->kind=='f'){
    if(na->descr->type=='f' && na->strides[na->nd-1]==4) t=T_FLOAT;
    else if(na->descr->type=='d' && na->strides[na->nd-1]==8) t=T_DOUBLE;
    else{ printf("ERROR: unknown dtype #2 (byteorder: %c kind: %c type: %c strides[-1]: %i\n",na->descr->byteorder,na->descr->kind,na->descr->type,na->strides[na->nd-1]); return; }
  }else if(na->descr->kind=='i'){
    if(na->descr->type=='l' && na->strides[na->nd-1]==8) t=T_LONG;
    else if(na->descr->type=='q' && na->strides[na->nd-1]==8) t=T_LONG;
    else if(na->descr->type=='l' && na->strides[na->nd-1]==4) t=T_INT;
    else if(na->descr->type=='i' && na->strides[na->nd-1]==4) t=T_INT;
    else if(na->descr->type=='h' && na->strides[na->nd-1]==2) t=T_SHORT;
    else{ printf("ERROR: unknown dtype #3 (byteorder: %c kind: %c type: %c strides[-1]: %i\n",na->descr->byteorder,na->descr->kind,na->descr->type,na->strides[na->nd-1]); return; }
  }else{ printf("ERROR: unknown dtype #4 (byteorder: %c kind: %c type: %c strides[-1]: %i\n",na->descr->byteorder,na->descr->kind,na->descr->type,na->strides[na->nd-1]); return; }
  d=na->strides[na->nd-1];
  for(i=na->nd-1;i;i--){
    d=d*na->dimensions[i];
    if(na->strides[i-1]!=d){ printf("ERROR: no compact array (strides[%i]: %i!=%i)\n",i-1,na->strides[i-1],d); return; }
  }
  CData_Array(dat,t,c,r);
  CData_SetNBlocks(dat,b);
  memcpy(CData_XAddr(dat,0,0),na->data,na->dimensions[0]*na->strides[0]);
}

PyObject* data2numpy(CData *dat){
  int t=CData_IsHomogen(dat);
  int nd,nt;
  npy_intp dims[3];
  int b,c,r;
  PyObject *np;
  if(!t) return NULL; /* TODO: PyNone */
  switch(t){
  case T_DOUBLE: nt=NPY_DOUBLE; break;
  case T_FLOAT:  nt=NPY_FLOAT; break;
  case T_LONG:   nt=NPY_LONG; break;
  case T_INT:    nt=NPY_INT; break;
  case T_SHORT:  nt=NPY_SHORT; break;
  case T_UCHAR:  nt=NPY_UBYTE; break;
  default: 
    if(t<=255) nt=NPY_STRING; else return NULL;
  }
  b=CData_GetNBlocks(dat);
  r=CData_GetNRecs(dat);
  c=CData_GetNComps(dat);
  if(b>1){
    nd=3;
    dims[0]=b;
    dims[1]=r/b;
    dims[2]=c;
  }else if(c>1){
    nd=2;
    dims[0]=r;
    dims[1]=c;
    dims[2]=0;
  }else{
    nd=1;
    dims[0]=r;
    dims[1]=0;
    dims[2]=0;
  }
  import_array();
  if(nt==NPY_STRING) np=PyArray_New(&PyArray_Type,nd,dims,nt,NULL,NULL,t,0,NULL);
  else np=PyArray_SimpleNew(nd,dims,nt);
  PyArrayObject *na=(PyArrayObject*)np;
  memcpy(na->data,CData_XAddr(dat,0,0),na->dimensions[0]*na->strides[0]);
  return np;
}
