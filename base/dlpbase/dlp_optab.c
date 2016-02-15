/* dLabPro base library
 * - Basic operation tables and table functions
 *
 * AUTHOR : Guntram Strecha
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
#include "dlp_math.h"

/**
 * List of all constants
 */
static const opcode_table __ctab[] =
{
  { OP_TRUE   ,1,0,"d:o", "Logical true"              ,"TRUE"    },
  { OP_FALSE  ,1,0,"d:o", "Logical false"             ,"FALSE"   },
  { OP_NULL   ,1,0,"d:o", "Null pointer"              ,"NULL"    },
  { OP_PI     ,1,0,"d:o", "Pi"                        ,"PI"      },
  { OP_E      ,1,0,"d:o", "Euler constant"            ,"E"       },
  /* do not change the following line!               */
  /* The values are used for end of table detection. */
  { -1        ,0,0,"",    "?"                         ,""        }
};

/**
 * List of all scalar operations.
 */
static const opcode_table __otab[] =
{
  { OP_NOOP     ,0,0,""      , "No operation"              ,"noop"     },
  { OP_REAL     ,1,1,"r:od"  , "Real part"                 ,"real"     },
  { OP_IMAG     ,1,1,"r:od"  , "Imaginary part"            ,"imag"     },
  { OP_CONJ     ,1,1,"d:od"  , "Complex conjugate"         ,"conj"     },
  { OP_NEG      ,1,1,"d:od"  , "Negation"                  ,"neg"      },
  { OP_SQR      ,1,1,"d:od"  , "Square"                    ,"sqr"      },
  { OP_ABS      ,1,1,"r:od"  , "Absolute value"            ,"abs"      },
  { OP_ANGLE    ,1,1,"r:od"  , "Angle of value"            ,"angle"    },
  { OP_SQRT     ,1,1,"d:od"  , "Square root"               ,"sqrt"     },
  { OP_SIGN     ,1,1,"d:od"  , "Sign"                      ,"sign"     },
  { OP_ENT      ,1,1,"d:od"  , "Entire value"              ,"ent"      },
  { OP_FLOOR    ,1,1,"d:od"  , "Floor value"               ,"floor"    },
  { OP_CEIL     ,1,1,"d:od"  , "Ceil value"                ,"ceil"     },
  { OP_INC      ,1,1,"d:od"  , "Increment (+1)"            ,"inc"      },
  { OP_INC      ,1,1,"d:od"  , "Increment (+1)"            ,"++"       },
  { OP_DEC      ,1,1,"d:od"  , "Decrement (-1)"            ,"dec"      },
  { OP_DEC      ,1,1,"d:od"  , "Decrement (-1)"            ,"--"       },
  { OP_INVT     ,1,1,"d:od"  , "Inversion"                 ,"inv"      },
  { OP_LOG      ,1,1,"d:od"  , "Decadic logarithm"         ,"log"      },
  { OP_LOG2     ,1,1,"d:od"  , "Binary logarithm"          ,"log2"     },
  { OP_LN       ,1,1,"d:od"  , "Natural logarithm"         ,"ln"       },
  { OP_EXP      ,1,1,"d:od"  , "Exponential function"      ,"exp"      },
  { OP_SIN      ,1,1,"d:od"  , "Sine"                      ,"sin"      },
  { OP_ASIN     ,1,1,"d:od"  , "Arc sine"                  ,"asin"     },
  { OP_SINH     ,1,1,"d:od"  , "Hyperbolic sine"           ,"sinh"     },
  { OP_ASINH    ,1,1,"d:od"  , "Arc hyperbolic sine"       ,"asinh"    },
  { OP_COS      ,1,1,"d:od"  , "Cosine"                    ,"cos"      },
  { OP_ACOS     ,1,1,"d:od"  , "Arc cosine"                ,"acos"     },
  { OP_COSH     ,1,1,"d:od"  , "Hyperbolic cosine"         ,"cosh"     },
  { OP_ACOSH    ,1,1,"d:od"  , "Arc hyperbolic cosine"     ,"acosh"    },
  { OP_TAN      ,1,1,"d:od"  , "Tangent"                   ,"tan"      },
  { OP_ATAN     ,1,1,"d:od"  , "Arc tangent"               ,"atan"     },
  { OP_TANH     ,1,1,"d:od"  , "Hyperbolic tangent"        ,"tanh"     },
  { OP_ATANH    ,1,1,"d:od"  , "Arc hyperbolic tangent"    ,"atanh"    },
  { OP_SINC     ,1,1,"d:od"  , "Sinc function"             ,"sinc"     },
  { OP_ADD      ,1,2,"d:dod" , "Addition"                  ,"add"      },
  { OP_ADD      ,1,2,"d:dod" , "Addition"                  ,"+"        },
  { OP_LSADD    ,1,2,"d:dod" , "Log semiring addition"     ,"lsadd"    },
  { OP_EXPADD   ,1,2,"d:dod" , "Exponential addition"      ,"expadd"   },
  { OP_DIFF     ,1,2,"d:dod" , "Subtraction"               ,"diff"     },
  { OP_DIFF     ,1,2,"d:dod" , "Subtraction"               ,"-"        },
  { OP_ABSDIFF  ,1,2,"r:dod" , "Absolute difference"       ,"absdiff"  },
  { OP_QDIFF    ,1,2,"d:dod" , "Quadratic difference"      ,"qdiff"    },
  { OP_QABSDIFF ,1,2,"r:dod" , "Absolute difference"       ,"qabsdiff" },
  { OP_MULT     ,1,2,"d:dod" , "Multiplication"            ,"mult"     },
  { OP_MULT     ,1,2,"d:dod" , "Multiplication"            ,"*"        },
  { OP_DIV      ,1,2,"d:dod" , "Division"                  ,"div"      },
  { OP_DIV      ,1,2,"d:dod" , "Division"                  ,"/"        },
  { OP_MOD      ,1,2,"d:dod" , "Modulus"                   ,"mod"      },
  { OP_MOD      ,1,2,"d:dod" , "Modulus"                   ,"%"        },
  { OP_SET      ,1,2,"d:dod" , "Assignment"                ,"set"      },
  { OP_SET      ,1,2,"d:dod" , "Assignment"                ,"="        },
  { OP_FCTRL    ,1,1,"d:od"  , "Factorial"                 ,"factorial"},
  { OP_FCTRL    ,1,1,"d:od"  , "Factorial"                 ,"!"        },
  { OP_GAMMA    ,1,1,"d:od"  , "Gamma function"            ,"gamma"    },
  { OP_LGAMMA   ,1,1,"d:od"  , "Log. Gamma function"       ,"lgamma"   },
  { OP_STUDT    ,1,2,"d:odn" , "Student's t-dens. (k-dim.)","studt"    },
  { OP_BETA     ,1,2,"d:odd" , "Euler's Beta function"     ,"beta"     },
  { OP_BETADENS ,1,3,"d:oddd", "Beta density"              ,"betadens" },
  { OP_BETAQUANT,1,3,"d:oddd", "P-quantile of Beta CDF"    ,"betaquant"},
  { OP_LNL      ,1,2,"d:odd" , "Natural logarithm, limited","lnl"      },
  { OP_POW      ,1,2,"d:odd" , "Power"                     ,"pow"      },
  { OP_POW      ,1,2,"d:odd" , "Power"                     ,"^"        },
  { OP_NOVERK   ,1,2,"n:onn" , "n over k"                  ,"over"     },
  { OP_GAUSS    ,1,2,"d:odd" , "Gaussian function"         ,"gauss"    },
  { OP_ERF      ,1,1,"d:od"  , "Error function"            ,"erf"      },
  { OP_ERFC     ,1,1,"d:od"  , "Complementary error fnc."  ,"erfc"     },
  { OP_SIGMOID  ,1,2,"d:odd" , "Sigmoid function"          ,"sigmoid"  },
  { OP_OR       ,1,2,"d:dod" , "Logical or"                ,"or"       },
  { OP_OR       ,1,2,"d:dod" , "Logical or"                ,"||"       },
  { OP_BITOR    ,1,2,"d:dod" , "Bitwise or"                ,"bitor"    },
  { OP_BITOR    ,1,2,"d:dod" , "Bitwise or"                ,"|"        },
  { OP_AND      ,1,2,"d:dod" , "Logical and"               ,"and"      },
  { OP_AND      ,1,2,"d:dod" , "Logical and"               ,"&&"       },
  { OP_BITAND   ,1,2,"d:dod" , "Bitwise and"               ,"bitand"   },
  { OP_BITAND   ,1,2,"d:dod" , "Bitwise and"               ,"&"        },
  { OP_NOT      ,1,1,"d:od"  , "Logical not"               ,"not"      },
  { OP_EQUAL    ,1,2,"d:dod" , "Equal"                     ,"eq"       },
  { OP_EQUAL    ,1,2,"d:dod" , "Equal"                     ,"=="       },
  { OP_NEQUAL   ,1,2,"d:dod" , "Not equal"                 ,"neq"      },
  { OP_NEQUAL   ,1,2,"d:dod" , "Not equal"                 ,"!="       },
  { OP_LESS     ,1,2,"d:dod" , "Less than"                 ,"less"     },
  { OP_LESS     ,1,2,"d:dod" , "Less than"                 ,"<"        },
  { OP_GREATER  ,1,2,"d:dod" , "Greater than"              ,"greater"  },
  { OP_GREATER  ,1,2,"d:dod" , "Greater than"              ,">"        },
  { OP_LEQ      ,1,2,"d:dod" , "Less or equal"             ,"leq"      },
  { OP_LEQ      ,1,2,"d:dod" , "Less or equal"             ,"<="       },
  { OP_GEQ      ,1,2,"d:dod" , "Greater or equal"          ,"geq"      },
  { OP_GEQ      ,1,2,"d:dod" , "Greater or equal"          ,">="       },
  { OP_ISNAN    ,1,1,"d:od"  , "Is NaN"                    ,"isnan"    },
  { OP_MAX      ,1,2,"d:odd" , "Maximum"                   ,"max"      },
  { OP_AMAX     ,1,2,"d:odd" , "Absolute maximum"          ,"amax"     },
  { OP_SMAX     ,1,2,"d:odd" , "Signed absolute maximum"   ,"smax"     },
  { OP_MIN      ,1,2,"d:odd" , "Minimum"                   ,"min"      },
  { OP_AMIN     ,1,2,"d:odd" , "Absolute minimum"          ,"amin"     },
  { OP_SMIN     ,1,2,"d:odd" , "Signed absolute minimum"   ,"smin"     },
  { OP_ROUND    ,1,1,"d:od"  , "Round"                     ,"round"    },
  /* do not change the following line!               */
  /* The values are used for end of table detection. */
  { -1          ,0,0,""      , "?"                         ,""         }
};

/**
 * List of all scalar vector operations.
 */
static const opcode_table __atab[] =
{
  { OP_DIFF   ,1,1,"D:oD",  "Difference first - last value"   ,"diff"    },
  { OP_SUM    ,1,1,"D:oD",  "Sum"                             ,"sum"     },
  { OP_LSSUM  ,1,1,"D:oD",  "Log semiring sum"                ,"lssum"   },
  { OP_LSMEAN ,1,1,"D:oD",  "Log semiring mean"               ,"lsmean"  },
  { OP_PROD   ,1,1,"D:oD",  "Product"                         ,"prod"    },
  { OP_MAX    ,1,1,"D:oD",  "Maximum"                         ,"max"     },
  { OP_IMAX   ,1,1,"D:oD",  "Index of maximum"                ,"imax"    },
  { OP_MIN    ,1,1,"D:oD",  "Minimum"                         ,"min"     },
  { OP_IMIN   ,1,1,"D:oD",  "Index of minimum"                ,"imin"    },
  { OP_SPAN   ,1,1,"D:oD",  "Spanwidth"                       ,"span"    },
  { OP_MEAN   ,1,1,"D:oD",  "Arithmetic mean"                 ,"mean"    },
  { OP_AMEAN  ,1,1,"D:oD",  "Absolute mean"                   ,"amean"   },
  { OP_QMEAN  ,1,1,"D:oD",  "Quadratic mean"                  ,"qmean"   },
  { OP_MOMENT ,1,2,"D:oDn", "k-th moment"                     ,"mom"     },
  { OP_CMOMENT,1,2,"D:oDn", "k-th central moment"             ,"cmom"    },
  { OP_GMEAN  ,1,1,"D:oD",  "Geometric mean"                  ,"gmean"   },
  { OP_HMEAN  ,1,1,"D:oD",  "Harmonic mean"                   ,"hmean"   },
  { OP_RANK   ,1,2,"D:oDn", "Element of rank k"               ,"rank"    },
  { OP_MED    ,1,1,"D:oD",  "Median"                          ,"med"     },
  { OP_QUANTIL,1,2,"D:oDn", "k quantil"                       ,"quantil" },
  { OP_QUARTIL,1,1,"D:oD",  "Quartil (k=0.25)"                ,"quartil" },
/*  { OP_IQDIST ,1,1,"Interquartil distance"           ,"iqdist"  },*/
  { OP_VAR    ,1,1,"D:oD",  "Variance"                        ,"var"     },
  { OP_STDEV  ,1,1,"D:oD",  "Standard deviation"              ,"sdev"    },
  { OP_SKEW   ,1,1,"D:oD",  "Skewness"                        ,"skew"    },
  { OP_EXC    ,1,1,"D:oD",  "Excess (kurtosis)"               ,"exc"     },
  { OP_MINK   ,1,2,"D:oDn", "Minkowski sum (power k)"         ,"mink"    },
  { OP_MINKPOW,1,2,"D:oDn", "Minkowski sum (power k) w/o root","minkpow" },
  /* do not change the following line!               */
  /* The values are used for end of table detection. */
  { -1        ,0,0,"",      "?"                               ,""        }
};

/**
 * List of all string operations.
 */
static const opcode_table __stab[] =
{
  { SOP_CCAT    ,1,2,"s:sos", "concatenate components"  ,"ccat"    },
  { SOP_CHASH   ,1,1,"n:os",  "hash (cell-by-cell)"     ,"chash"   },
  { SOP_CMP     ,1,2,"b:oss", "compare"                 ,"cmp"     },
  { SOP_HASH    ,1,1,"n:os",  "hash (global)"           ,"hash"    },
  { SOP_LEFT    ,1,2,"s:osn", "left n characters"       ,"left"    },
  { SOP_LEN     ,1,1,"n:os",  "length"                  ,"len"     },
  { SOP_LWR     ,1,1,"s:os",  "lower case"              ,"lwr"     },
  { SOP_RCAT    ,1,2,"s:oss", "concatenate records"     ,"rcat"    },
  { SOP_REPLACE ,1,2,"s:oss", "replace substring"       ,"replace" },
  { SOP_RIGHT   ,1,2,"s:osn", "right n characters"      ,"right"   },
  { SOP_SEARCH  ,1,2,"s:oss", "search substring"        ,"search"  },
  { SOP_SPLIT   ,1,2,"s:oss", "split at delimiters"     ,"split"   },
  { SOP_SPLITALL,1,2,"s:oss", "split at every delimiter","splitall"},
  { SOP_SPLITD  ,1,2,"s:oss", "split keeping delimiters","splitd"  },
  { SOP_SPLITP  ,1,1,"s:oss", "split path names"        ,"splitp"  },
  { SOP_TRIM    ,1,2,"s:oss", "trim characters"         ,"trim"    },
  { SOP_UPR     ,1,1,"s:os",  "upper case"              ,"upr"     },
  /* do not change the following line!               */
  /* The values are used for end of table detection. */
  { -1         ,0,0,"",       "?"                     ,""        }
};

/**
 * List of all signal operations.
 */
static const opcode_table __ftab[] =
{
  { FOP_APS         ,1,1,"D:oD"      ,"Auto-power spectrum"                                                     ,"aps"         },
  { FOP_CCF         ,1,1,"D:oD"      ,"Cross correlation function"                                              ,"ccf"         },
  { FOP_CEP2LPC     ,2,2,"DD:oDn"    ,"Cepstrum to Linear Predictive Coding transform"                          ,"cep2lpc"     },
  { FOP_CEP2MCEP    ,1,3,"D:oDrn"    ,"Cepstrum to Mel-Cepstrum transform"                                      ,"cep2mcep"    },
  { FOP_CEP         ,1,2,"D:oDn"     ,"Cepstrum"                                                                ,"cep"         },
  { FOP_DEFRAME     ,1,2,"D:oDn"     ,"Signal de-framing"                                                       ,"deframe"     },
  { FOP_DENOISE     ,1,4,"D:oDnrs"   ,"Denoising"                                                               ,"denoise"     },
  { FOP_DESCALE     ,1,1,"D:oD"      ,"Signal descaling"                                                        ,"descale"     },
  { FOP_DISTRIBUTION,1,2,"C:oDD"     ,"Value distribution"                                                      ,"distribution"},
  { FOP_DTW         ,1,2,"D:oDD"     ,"Dynamic Time Warping"                                                    ,"dtw"         },
  { FOP_F02EXC      ,1,4,"D:oDnns"   ,"Get excitation from F0 contour"                                          ,"f02exc"      },
  { FOP_FFT         ,1,1,"C:oD"      ,"Fourier transform"                                                       ,"fft"         },
  { FOP_FILTER      ,1,4,"D:oDDDD"   ,"Infinite Imp. Response filter"                                           ,"filter"      },
  { FOP_FIR         ,1,3,"D:oDDD"    ,"Finite Imp. Response filter"                                             ,"fir"         },
  { FOP_FRAME       ,1,3,"D:oDnn"    ,"Signal framing"                                                          ,"frame"       },
  { FOP_SFRAME      ,1,5,"D:oDnnDn"  ,"Pitch synchron signal framing"                                           ,"sframe"      },
  { FOP_GCEP        ,1,3,"D:oDrn"    ,"Generalized Cepstrum"                                                    ,"gcep"        },
  { FOP_GCEP2GCEP   ,1,3,"D:oDrn"    ,"Generalized Cepstrum transform"                                          ,"gcep2gcep"   },
  { FOP_GCEP2LPC    ,2,2,"DD:oDn"    ,"Generalized Cepstrum to Linear Predictive Coding transform"              ,"gcep2lpc"    },
  { FOP_GCEP2MLPC   ,2,3,"DD:oDrn"   ,"Generalized Cepstrum to Mel-Linear Predictive Coding transform"          ,"gcep2mlpc"   },
  { FOP_GCEPNORM    ,2,1,"DD:oD"     ,"Gain normalization"                                                      ,"gcep_norm"   },
  { FOP_GETF0       ,1,4,"N:oDnns"   ,"F0 Estimation"                                                           ,"getf0"       },
  { FOP_GMULT       ,1,1,"D:oD"      ,"Multiply by gamma"                                                       ,"gmult"       },
  { FOP_IFFT        ,1,1,"C:oD"      ,"Inverse Fourier Transform"                                               ,"ifft"        },
  { FOP_IGCEPNORM   ,1,2,"D:oDD"     ,"Inverse gain normalization"                                              ,"igcep_norm"  },
  { FOP_IGMULT      ,1,1,"D:oD"      ,"Divide by gamma"                                                         ,"igmult"      },
  { FOP_IIR         ,1,3,"D:oDDD"    ,"Purely Infinite Imp. Response filter"                                    ,"iir"         },
  { FOP_IMCEP       ,1,3,"D:oDRr"    ,"Inverse Mel-Cepstrum"                                                    ,"imcep"       },
  { FOP_IMLT        ,1,1,"D:oD"      ,"Inverse Modulated Lapped Transform"                                      ,"imlt"        },
  { FOP_ISVQ        ,1,2,"D:oDN"     ,"Inverse Scalar Vector Quantization"                                      ,"isvq"        },
  { FOP_IVQ         ,1,2,"D:oDN"     ,"Inverse Vector Quantization"                                             ,"ivq"         },
  { FOP_LPC         ,2,3,"DD:oDns"   ,"Linear Predictive Coding"                                                ,"lpc"         },
  { FOP_LPC2CEP     ,1,3,"D:oDDn"    ,"Linear Predictive Coding to Cepstrum transform"                          ,"lpc2cep"     },
  { FOP_LPC2GCEP    ,1,4,"D:oDDrn"   ,"Linear Predictive Coding to Generalized Cepstrum transform"              ,"lpc2gcep"    },
  { FOP_LPC2MGCEP   ,1,5,"D:oDDrrn"  ,"Linear Predictive Coding to Mel-Generalized Cepstrum transform"          ,"lpc2mgcep"   },
  { FOP_LPC2MLPC    ,2,4,"DD:oDDrn"  ,"Linear Predictive Coding to Mel-Linear Predictive Coding transform"      ,"lpc2mlpc"    },
  { FOP_LSF2POLY    ,1,1,"R:oR"      ,"LSF to Polynomial transform"                                             ,"lsf2poly"    },
  { FOP_MCEP2CEP    ,1,2,"D:oDn"     ,"Mel-Cepstrum to Cepstrum transform"                                      ,"mcep2cep"    },
  { FOP_MCEP2MCEP   ,1,3,"D:oDrn"    ,"Mel-Cepstrum to Mel-Cepstrum transform"                                  ,"mcep2mcep"   },
  { FOP_MCEP2MLPC   ,2,3,"DD:oDrn"   ,"Mel-Cepstrum to Mel-Linear Predictive Coding transform"                  ,"mcep2mlpc"   },
  { FOP_MCEP        ,1,3,"D:oDrn"    ,"Mel-Cepstrum"                                                            ,"mcep"        },
  { FOP_MCEPENHANCE ,1,1,"D:oD"      ,"Mel-Cepstrum enhancement"                                                ,"mcep_enhance"},
  { FOP_MFB         ,1,4,"D:oDrns"   ,"MelFilter"                                                               ,"mfb"         },
  { FOP_MFBS        ,1,4,"D:oDrns"   ,"MelFilter in spectral domain"                                            ,"mfbs"        },
  { FOP_MFFT        ,1,2,"C:oDr"     ,"Mel-Fourier transform"                                                   ,"mfft"        },
  { FOP_MFILTER     ,1,5,"D:oDDDrD"  ,"Mel-Infinite Imp. Response filter"                                       ,"mfilter"     },
  { FOP_MFIR        ,1,4,"D:oDDrD"   ,"Mel-Finite Imp. Response filter"                                         ,"mfir"        },
  { FOP_MGCEP       ,1,4,"D:oDrrn"   ,"Mel-Generalized Cepstrum"                                                ,"mgcep"       },
  { FOP_MGCEP2LPC   ,2,2,"DD:oDn"    ,"Mel-Generalized Cepstrum to Linear Predictive Coding transform"          ,"mgcep2lpc"   },
  { FOP_MGCEP2MGCEP ,1,4,"D:oDrrn"   ,"Mel-Generalized Cepstrum transform"                                      ,"mgcep2mgcep" },
  { FOP_MGCEP2MLPC  ,2,3,"DD:oDrn"   ,"Mel-Generalized Cepstrum to Mel-Linear Predictive Coding transform"      ,"mgcep2mlpc"  },
  { FOP_MIIR        ,1,4,"D:oDDrD"   ,"Purely Mel-Infinite Imp. Response filter"                                ,"miir"        },
  { FOP_MLPC        ,2,4,"DD:oDrns"  ,"Mel-Linear Predictive Coding"                                            ,"mlpc"        },
  { FOP_MLPC2GCEP   ,1,4,"D:oDDrn"   ,"Mel-Linear Predictive Coding to Generalized Cepstrum transform"          ,"mlpc2gcep"   },
  { FOP_MLPC2LPC    ,2,3,"DD:oDDn"   ,"Mel-Linear Predictive Coding to Linear Predictive Coding transform"      ,"mlpc2lpc"    },
  { FOP_MLPC2MCEP   ,1,4,"D:oDDrn"   ,"Mel-Linear Predictive Coding to Mel-Cepstrum transform"                  ,"mlpc2mcep"   },
  { FOP_MLPC2MGCEP  ,1,5,"D:oDDrrn"  ,"Mel-Linear Predictive Coding to Mel-Generalized Cepstrum transform"      ,"mlpc2mgcep"  },
  { FOP_MLPC2MLPC   ,2,4,"DD:oDDrn"  ,"Mel-Linear Predictive Coding to Mel-Linear Predictive Coding transform"  ,"mlpc2mlpc"   },
  { FOP_MLSF2MLSF   ,1,2,"D:oDr"     ,"Mel-Line Spectral Frequencies to Mel-Line Spectral Frequencies transform","mlsf2mlsf"   },
  { FOP_MLT         ,1,1,"D:oD"      ,"Modulated Lapped Transform"                                              ,"mlt"         },
  { FOP_NOISIFY     ,1,1,"D:oD"      ,"Signal noisifying"                                                       ,"noisify"     },
  { FOP_PITCHMARK   ,1,5,"D:oDsnnn"  ,"Pitch marking"                                                           ,"pitchmark"   },
  { FOP_POLY2LSF    ,1,1,"R:oR"      ,"Polynomial to LSF transform"                                             ,"poly2lsf"    },
  { FOP_RMDC        ,1,1,"D:oD"      ,"Remove DC"                                                               ,"rmdc"        },
  { FOP_ROOTS       ,1,1,"C:oD"      ,"Roots of polynomial"                                                     ,"roots"       },
  { FOP_SCALE       ,1,2,"D:oDd"     ,"Signal scaling"                                                          ,"scale"       },
  { FOP_SVQ         ,2,2,"DN:oDN"    ,"Scalar Vector Quantization"                                              ,"svq"         },
  { FOP_WINDOW      ,1,5,"D:oDnnsb"  ,"Windowing"                                                               ,"window"      },
  { FOP_WVL         ,1,3,"D:oDnn"    ,"Wavelet analysis"                                                        ,"wvl"         },
  { FOP_VQ          ,2,3,"DN:oDns"   ,"Vector Quantization"                                                     ,"vq"          },
  { FOP_UNWRAP      ,1,1,"C:oC"      ,"Phase unwrapping"                                                        ,"unwrap"      },
  { FOP_ZCR         ,1,2,"N:oDR"     ,"Zero crossing"                                                           ,"zcr"         },
  /* do not change the following line!               */
  /* The values are used for end of table detection. */
  { -1              ,1,0,""         ,"?"                                       ,""          }
};

/**
 * Get operation code of an operation table from symbol.
 *
 * @param  lpOptab   The operation table.
 * @param  lpsOpname The constant name of the requested opcode.
 * @return Returns operation code according to symbol
 *         or -1 if code is not valid.
 */
INT16 dlp_op_code(const opcode_table* lpOptab, const char* lpsOpname)
{
  INT16 i=-1;

  if (lpsOpname         == NULL) return -1;
  if (strlen(lpsOpname) == 0   ) return -1;

  /* Scan table */
  do
  {
    if (strcmp(lpsOpname,lpOptab[++i].sym) == 0) break;
  }
  while(lpOptab[i].opc != -1);

  return lpOptab[i].opc;
}

/**
 * Get index of operation code table entry by operator code
 *
 * @param  lpOptab   The operation table.
 * @param  nOpcode The operation code of the requested operation.
 * @return Returns the index of the entry or -1 if code is not valid.
 */
INT16 dlp_op_index(const opcode_table* lpOptab, INT16 nOpcode) {
  INT16 i,j;

  i=-1;
  do
  {
    if ((j=lpOptab[++i].opc) == nOpcode) break;
  }
  while (j != -1);
  return (j==-1) ? -1 : i;
}
/**
 * Get operation name from operation code.
 *
 * @param  lpOptab   The operation table.
 * @param  nOpcode The operation code of the requested operation.
 * @return Returns pointer to operation name for valid operation
 *         code and NULL for invalid operation code.
 * @see    #dlp_op_code dlp_op_code
 */
const char* dlp_op_name(const opcode_table* lpOptable, INT16 nOpcode)
{
  INT16 i = dlp_op_index(lpOptable, nOpcode);
  return (i==-1) ? NULL : lpOptable[i].nam;
}

/**
 * Get operation sym name from operation code.
 *
 * @param  lpOptab The operation table.
 * @param  nOpcode The operation code of the requested operation.
 * @return Returns pointer to operation sym name for valid operation
 *         code and NULL for invalid operation code.
 * @see    #dlp_op_code dlp_op_code
 */
const char* dlp_op_sym(const opcode_table* lpOptab, INT16 nOpcode)
{
  INT16 i = dlp_op_index(lpOptab, nOpcode);
  return (i==-1) ? NULL : lpOptab[i].sym;
}

/**
 * Get number of operands from operation code.
 *
 * @param  lpOptab The operation table.
 * @param  nOpcode The operation code of the requested operation.
 * @return Returns the number of operands according to operation code
 *         or -1 if code is not valid.
 * @see    #dlp_scalop_name dlp_scalop_name
 */
INT16 dlp_op_ops(const opcode_table* lpOptab, INT16 nOpcode) {
  INT16 i = dlp_op_index(lpOptab, nOpcode);
  return (i==-1) ? NULL : lpOptab[i].ops;
}

/**
 * Get number of results from operation code.
 *
 * @param  lpOptab The operation table.
 * @param  nOpcode The operation code of the requested operation.
 * @return Returns the number of results according to operation code
 *         or -1 if code is not valid.
 * @see    #dlp_scalop_name dlp_scalop_name
 */
INT16 dlp_op_res(const opcode_table* lpOptab, INT16 nOpcode) {
  INT16 i = dlp_op_index(lpOptab, nOpcode);
  return (i==-1) ? NULL : lpOptab[i].res;
}

/**
 * Get type of operand.
 *
 * @param lpSignature The signature, i.e field 'sym' of ocode_table.
 * @param nOp         The index of the operand which type is requested.
 * @return            The type of the requested operand
 */
INT16 dlp_op_opstype(const char* lpSignature, INT16 nOp) {
  INT16 j=0,i=0;

  while(lpSignature[i] && (lpSignature[i]!=':')) i++;
  while(lpSignature[i] && (j<nOp)) {
    i++;
    while(lpSignature[i] == 'o') i++;
    j++;
  }

  switch(lpSignature[i]) {
    case 'd' : return T_OP_UNKNOWN;             break;
    case 'D' : return T_OP_UNKNOWN | T_OP_DATA; break;
    case 'r' : return T_OP_REAL;                break;
    case 'R' : return T_OP_REAL    | T_OP_DATA; break;
    case 'n' : return T_OP_INTEGER;             break;
    case 'N' : return T_OP_INTEGER | T_OP_DATA; break;
    case 's' : return T_OP_STRING;              break;
    case 'S' : return T_OP_STRING  | T_OP_DATA; break;
    case 'b' : return T_OP_BOOL;                break;
    case 'B' : return T_OP_BOOL    | T_OP_DATA; break;
    default  : return -1;
  }
}

/**
 * Get constant operation code from symbol.
 *
 * @param  lpsOpname The constant name of the requested opcode.
 * @return Returns operation code according to symbol
 *         or -1 if code is not valid.
 */
INT16 dlp_constant_code(const char* lpsOpname)
{
  return dlp_op_code(__ctab, lpsOpname);
}

/**
 * Returns a mathematical constant.
 *
 * @param  nOpcode Operation code
 * @return The constant's value or 0. if nOpcode is not valid.
 */
COMPLEX64 dlp_constant(INT16 nOpcode)
{
  switch (nOpcode)
  {
  case OP_TRUE : { return CMPLX(1.);                                                     }
  case OP_FALSE: { return CMPLX(0.);                                                     }
  case OP_NULL : { return CMPLX(0.);                                                     }
  case OP_PI   : { return CMPLX(F_PI);                                                   }
  case OP_E    : { return CMPLX(F_E);                                                    }
  default      : { DLPASSERT(FMSG("Unknown constant operation code")); return CMPLX(0.); }
  }
}

/**
 * Get an entry from the scalar operation table
 *
 * @param  nEntry The index of the entry in the table.
 * @return Returns a pointer to the entry
 * @see    #dlp_aggrop_code dlp_aggrop_code
 */
const opcode_table* dlp_scalop_entry(INT16 nEntry) {
  return &__otab[nEntry];
}

/**
 * Get operation name from operation code.
 *
 * @param  nOpcode The operation code of the requested operation.
 * @return Returns pointer to operation name for valid operation
 *         code and a pointer to empty string for invalid operation code.
 * @see    #dlp_aggrop_code dlp_aggrop_code
 */
const char* dlp_scalop_name(INT16 nOpcode)
{
  return dlp_op_name(__otab, nOpcode);
}

/**
 * Get operation sym name from operation code.
 *
 * @param  nOpcode The operation code of the requested operation.
 * @return Returns pointer to operation sym name for valid operation
 *         code and a pointer to empty string for unvalid operation code.
 * @see    #dlp_aggrop_code dlp_aggrop_code
 */
const char* dlp_scalop_sym(INT16 nOpcode)
{
  return dlp_op_sym(__otab, nOpcode);
}

/**
 * Get operation code from symbol.
 *
 * @param  lpsOpname The operation name of the requested operation.
 * @return Returns operation code according to symbol
 *         or -1 if code is not valid.
 * @see    #dlp_scalop_name dlp_scalop_name
 */
INT16 dlp_scalop_code(const char* lpsOpname)
{
  return dlp_op_code(__otab, lpsOpname);
}

/**
 * Determines of an operation code denotes a poitner operation.
 *
 * @param nOpCode
 *           The operation code
 * @return <code>TRUE</code> if the operation is a pointer operation,
 *         <code>FALSE</code> otherwise
 */
INT16 dlp_is_pointer_op_code(INT16 nOpCode)
{
  if (nOpCode==OP_NULL) return TRUE;
  return FALSE;
}

/**
 * Determines of an operation code denotes a logic (Boolean) operation.
 *
 * @param nOpCode
 *           The operation code
 * @return <code>TRUE</code> if the operation is Boolean, <code>FALSE</code>
 *         otherwise
 */
INT16 dlp_is_logic_op_code(INT16 nOpCode)
{
  if (nOpCode==OP_OR     ) return TRUE;
  if (nOpCode==OP_AND    ) return TRUE;
  if (nOpCode==OP_NOT    ) return TRUE;
  if (nOpCode==OP_EQUAL  ) return TRUE;
  if (nOpCode==OP_NEQUAL ) return TRUE;
  if (nOpCode==OP_LESS   ) return TRUE;
  if (nOpCode==OP_GREATER) return TRUE;
  if (nOpCode==OP_LEQ    ) return TRUE;
  if (nOpCode==OP_GEQ    ) return TRUE;
  if (nOpCode==OP_TRUE   ) return TRUE;
  if (nOpCode==OP_FALSE  ) return TRUE;
  return FALSE;
}

/**
 * Get operation name from operation code.
 *
 * @param  nOpcode The operation code of the requested operation.
 * @return A pointer to operation name for valid operation
 *         code and a pointer to empty string for unvalid operation code.
 * @see    #dlp_aggrop_code dlp_aggrop_code
 */
const char* dlp_aggrop_name(INT16 nOpcode)
{
  return dlp_op_name(__atab, nOpcode);
}

/**
 * Get operation code from operation symbol.
 *
 * @param  lpsOpSymbol The symbol of the requested operation.
 * @return If the lpOpSymbol is a valid operation symbol the function
 *         returns the operation code. If the symbol is not valid -1
 *         is returned.
 * @see    #dlp_aggrop_name dlp_aggrop_name
 */
INT16 dlp_aggrop_code(const char* lpsOpSymbol)
{
  return dlp_op_code(__atab, lpsOpSymbol);
}

/**
 * Get operation name from string operation code.
 *
 * @param  nOpcode The operation code of the requested operation.
 * @return A pointer to operation name for valid operation
 *         code and a pointer to empty string for unvalid operation code.
 * @see    #dlp_aggrop_code dlp_aggrop_code
 */
const char* dlp_strop_name(INT16 nOpcode)
{
  return dlp_op_name(__stab, nOpcode);
}

/**
 * Get operation code from string operation symbol.
 *
 * @param  lpsOpSymbol The symbol of the requested operation.
 * @return If the lpOpSymbol is a valid operation symbol the function
 *         returns the operation code. If the symbol is not valid -1
 *         is returned.
 * @see    #dlp_aggrop_name dlp_aggrop_name
 */
INT16 dlp_strop_code(const char* lpsOpSymbol)
{
  return dlp_op_code(__stab, lpsOpSymbol);
}

/**
 * Get an entry from the signal operation table
 *
 * @param  nEntry The index of the entry in the table.
 * @return Returns a pointer to the entry
 * @see    #dlp_aggrop_code dlp_aggrop_code
 */
const opcode_table* dlp_sigop_entry(INT16 nEntry) {
  return &__ftab[nEntry];
}

/**
 * Get signal operation name from operation code.
 *
 * @param  nOpcode The operation code of the requested operation.
 * @return Returns pointer to operation name for valid operation
 *         code and a pointer to empty string for invalid operation code.
 * @see    #dlp_aggrop_code dlp_aggrop_code
 */
const char* dlp_sigop_name(INT16 nOpcode)
{
  return dlp_op_name(__ftab, nOpcode);
}

/**
 * Get signal operation sym name from operation code.
 *
 * @param  nOpcode The operation code of the requested operation.
 * @return Returns pointer to operation sym name for valid operation
 *         code and a pointer to empty string for unvalid operation code.
 * @see    #dlp_aggrop_code dlp_aggrop_code
 */
const char* dlp_sigop_sym(INT16 nOpcode)
{
  return dlp_op_sym(__ftab, nOpcode);
}

/**
 * Get signal operation code from symbol.
 *
 * @param  lpsOpname The operation name of the requested operation.
 * @return Returns operation code according to symbol
 *         or -1 if code is not valid.
 * @see    #dlp_scalop_name dlp_scalop_name
 */
INT16 dlp_sigop_code(const char* lpsOpname)
{
  return dlp_op_code(__ftab, lpsOpname);
}

/**
 * Get number of results from signal operation code.
 *
 * @param  nOpcode The operation code of the requested operation.
 * @return Returns the number of results according to operation code
 *         or -1 if code is not valid.
 * @see    #dlp_scalop_name dlp_scalop_name
 */
INT16 dlp_sigop_res(INT16 nOpcode)
{
  return dlp_op_res(__ftab, nOpcode);
}

/**
 * Get number of operands from signal operation code.
 *
 * @param  nOpcode The operation code of the requested operation.
 * @return Returns the number of operands according to operation code
 *         or -1 if code is not valid.
 * @see    #dlp_scalop_name dlp_scalop_name
 */
INT16 dlp_sigop_ops(INT16 nOpcode)
{
  return dlp_op_ops(__ftab, nOpcode);
}

/**
 * Prints a table of constant operation codes at stdout.
 */
void dlp_constant_printtab()
{
  INT16 i=-1;
  INT16 j=0;

  printf ("\n   Table of constants");
  do {
    i++;
    j=__ctab[i].opc;
    if (j < 0) break;
    printf("\n   %2hd: %5hd  ",(short)i,(short)j);
    if (strlen(__ctab[i].sym) > 0)
      printf("%-13s",__ctab[i].sym);
    else printf("             ");
    printf("  %s",__ctab[i].nam);
  } while (j != -1);
}

/**
 * Prints a table of scalar operations at stdout.
 */
void dlp_scalop_printtab()
{
  INT16 i=-1;
  INT16 j=0;
  char  buffer[255];

  printf ("\n   Table of scalar operators");
  do {
    i++;
    j=__otab[i].opc;
    if (j < 0) break;
    printf("\n   %2hd: %5hd  ",(short)i,(short)j);
    if (strlen(__otab[i].sym) > 0)
    {
      if (__otab[i].ops == 0) sprintf(buffer,"%s()"   ,__otab[i].sym);
      if (__otab[i].ops == 1) sprintf(buffer,"%s(x)"  ,__otab[i].sym);
      if (__otab[i].ops == 2) sprintf(buffer,"%s(x,y)",__otab[i].sym);
      printf("%-13s", buffer);
    }
    else printf("             ");
    printf("  %s",__otab[i].nam);
  } while (j != -1);
}

/**
 * Prints a table of scalar vector operation codes at stdout.
 */
void dlp_aggrop_printtab()
{
  INT16 i=-1;
  INT16 j=0;

  printf("\n   Table of aggregation operators");
  do
  {
    j=__atab[++i].opc;
    if(j < 0) break;
    printf("\n   %2d: %5d  ", (int)i, (int)j);
    if (strlen(__atab[i].sym) > 0) printf("%-10s",__atab[i].sym);
    else                           printf("          ");
    printf ("  %s",__atab[i].nam);
  }
  while (j != -1);
}

/**
 * Prints a table of string operation codes at stdout.
 */
void dlp_strop_printtab()
{
  INT16 i=-1;
  INT16 j=0;

  printf("\n   Table of string operations");
  do
  {
    j=__stab[++i].opc;
    if(j < 0) break;
    printf("\n   %2d: %5d  ", (int)i, (int)j);
    if (strlen(__stab[i].sym) > 0) printf("%-10s",__stab[i].sym);
    else                           printf("          ");
    printf ("  %s",__stab[i].nam);
  }
  while (j != -1);
}

