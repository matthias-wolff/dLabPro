#ifndef _DLABPRO_NUMPY
#define _DLABPRO_NUMPY

void numpy2data(PyObject *np,CData *dat);
PyObject* data2numpy(CData *dat);

#endif
