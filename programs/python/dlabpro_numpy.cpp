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
  }else{ printf("ERROR: only up to dim 3"); return; }
  /*printf("\n%i %i %i %i\n",na->descr->byteorder,na->descr->kind,na->descr->type,na->strides[na->nd-1]);*/
  if(na->descr->byteorder==124 && na->descr->kind==83 && na->descr->type==83 && na->strides[na->nd-1]<=255) t=na->strides[na->nd-1]; /* bytes type */
  else if(na->descr->byteorder!=61){ printf("ERROR: unknown dtype"); return; }
  else if(na->descr->kind==102){
    if(na->descr->type==102 && na->strides[na->nd-1]==4) t=T_FLOAT;
    else if(na->descr->type==100 && na->strides[na->nd-1]==8) t=T_DOUBLE;
    else{ printf("ERROR: unknown dtype"); return; }
  }else if(na->descr->kind==105){
    if(na->descr->type==108 && na->strides[na->nd-1]==8) t=T_LONG;
	else if(na->descr->type==113 && na->strides[na->nd-1]==8) t=T_LONG;
    else if(na->descr->type==105 && na->strides[na->nd-1]==4) t=T_INT;
    else if(na->descr->type==104 && na->strides[na->nd-1]==2) t=T_SHORT;
    else{ printf("ERROR: unknown dtype"); return; }
  }else{ printf("ERROR: unknown dtype"); return; }
  d=na->strides[na->nd-1];
  for(i=na->nd-1;i;i--){
    d=d*na->dimensions[i];
    if(na->strides[i-1]!=d){ printf("ERROR: no compact array"); return; }
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
  if(nt==NPY_STRING) np=PyArray_New(&PyArray_Type,nd,dims,nt,NULL,CData_XAddr(dat,0,0),t,0,NULL);
  else np=PyArray_SimpleNewFromData(nd,dims,nt,CData_XAddr(dat,0,0));
  return np;
}
