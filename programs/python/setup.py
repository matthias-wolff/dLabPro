#!/usr/bin/python3

import os
from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

basedir='../..'
bld_dlabpro=os.path.join(basedir,'build/dlabpro/lib.release')

setup(
    name = 'dLabPro',
    version = '1.0',
    description = 'Python Package with dLabPro Extension',

    url='https://github.com/matthias-wolff/dLabPro',
    author='Frank Duckhorn',

    ext_modules=[Extension("dlabpro",
        sources=["dlabpro.pyx","dlabpro_numpy.cpp","dlabpro_init.cpp"],
        include_dirs=[
            '.',
            os.path.join(basedir,'include'),
            os.path.join(basedir,'include/automatic'),
            '/usr/lib/python3/dist-packages/numpy/core/include/numpy',
            '/usr/lib/python3.6/site-packages/numpy/core/include/numpy',
            '/home/duckhfra/int/conda/anaconda3/lib/python3.6/site-packages/numpy/core/include/numpy',
            '/home/duckhfra/int/conda/anaconda3/lib/python3.7/site-packages/numpy/core/include/numpy',
            '/home/duckhfra/int/conda/anaconda3/lib/python3.8/site-packages/numpy/core/include/numpy',
            '/home/duckhfra/int/conda/anaconda3/lib/python3.9/site-packages/numpy/core/include/numpy',
            '/home/duckhfra/int/conda/anaconda3/lib/python3.10/site-packages/numpy/core/include/numpy',
            '/public/software/anaconda/lib/python3.7/site-packages/numpy/core/include/numpy',
            '/public/software/anaconda/lib/python3.8/site-packages/numpy/core/include/numpy',
            '/public/software/anaconda/lib/python3.9/site-packages/numpy/core/include/numpy',
            '/public/software/anaconda/lib/python3.10/site-packages/numpy/core/include/numpy',
            '/public/software/anaconda/lib/python3.11/site-packages/numpy/core/include/numpy',
        ],
        extra_objects=[
            os.path.join(bld_dlabpro,'file.a'),
            os.path.join(bld_dlabpro,'ipkclib.a'),
            os.path.join(bld_dlabpro,'signal.a'),
            os.path.join(bld_dlabpro,'var.a'),
            os.path.join(bld_dlabpro,'dlpmath.a'),
            os.path.join(bld_dlabpro,'sptk.a'),
            os.path.join(bld_dlabpro,'libsndfile.a'),
            os.path.join(bld_dlabpro,'hmm.a'),
            os.path.join(bld_dlabpro,'fstsearch.a'),
            os.path.join(bld_dlabpro,'fst.a'),
            os.path.join(bld_dlabpro,'gmm.a'),
            os.path.join(bld_dlabpro,'vmap.a'),
            os.path.join(bld_dlabpro,'statistics.a'),
            os.path.join(bld_dlabpro,'profile.a'),
            os.path.join(bld_dlabpro,'data.a'),
            os.path.join(bld_dlabpro,'dlptable.a'),
            os.path.join(bld_dlabpro,'dlpobject.a'),
            os.path.join(bld_dlabpro,'dlpbase.a'),
            os.path.join(bld_dlabpro,'matrix.a'),
            os.path.join(bld_dlabpro,'xmlstream.a'),
            os.path.join(bld_dlabpro,'dn3stream.a'),
            os.path.join(bld_dlabpro,'dnorm.a'),
            os.path.join(bld_dlabpro,'dlpmath.a'),
            os.path.join(bld_dlabpro,'kazlib.a'),
            os.path.join(bld_dlabpro,'clapack.a'),
            os.path.join(bld_dlabpro,'zlib.a'),
            os.path.join(bld_dlabpro,'xpat.a'),
      ],
      language="c++",)],

  cmdclass = {'build_ext': build_ext}
)
