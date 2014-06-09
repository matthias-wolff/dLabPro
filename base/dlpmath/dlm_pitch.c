/* dLabPro mathematics library
 * - pitch related methods
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
 * <p>Expand/reduce number of pitch markers to fit new target sum of period length</p>
 *
 * @param pm_old
 *          Pointer to old pitch markers.
 * @param n_pm_old
 *          Number of pitch markers, equals to length of array <code>pm_old</code>.
 * @param pm_new
 *          Pointer to new pitch markers.
 * @param n_pm_new
 *          Pointer to new number of pitch markers, equals to length of array
 *          <code>pm_new</code>.
 * @param length
 *          Target length of corresponding excitation signal, equals to sum of
 *          period length of the new pitch markers.
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 dlm_pm_expand(INT16* pm_old, INT32 n_pm_old, INT16** pm_new, INT32* n_pm_new, INT32 length) {
  return dlm_pm_expand_impl(pm_old, n_pm_old, pm_new, n_pm_new, 2 * sizeof(INT16), length);
}

/**
 * <p>Expand/reduce number of pitch markers to fit new target sum of period length</p>
 *
 * @param pm_old
 *          Pointer to old pitch markers.
 * @param n_pm_old
 *          Number of pitch markers, equals to length of array <code>pm_old</code>.
 * @param pm_new
 *          Pointer to new pitch markers.
 * @param n_pm_new
 *          Pointer to new number of pitch markers, equals to length of array
 *          <code>pm_new</code>.
 * @param size
 *          Size of one pitch mark of <code>pm_old</code> and <code>pm_new</code>.
 *          Every pitch marker must consist of at least one entry, that is the period
 *          length of size <code>sizeof(short)</code>. Usually the stimulation is
 *          additionally stored, but more entries of different sizes are supported.
 *          <code>size</code> is then the sum of their sizes in byte.
 * @param length
 *          Target length of corresponding excitation signal, equals to sum of
 *          period length of the new pitch markers.
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 dlm_pm_expand_impl(INT16* pm_old, INT32 n_pm_old, INT16** pm_new, INT32* n_pm_new, INT16 size, INT32 length) {
  INT32 i_pm;
  INT32 sum;
  INT32 diff;
  if (!pm_old || !pm_new || *pm_new || !n_pm_new) {
    return NOT_EXEC;
  }

  sum = 0;
  *n_pm_new = 0;
  while (sum < length) {
    *pm_new = dlp_realloc(*pm_new, *n_pm_new + 1, size);

    i_pm = sum * n_pm_old / length;

    dlp_memmove(*pm_new + size * *n_pm_new, (BYTE*) pm_old + size * i_pm, size);

    sum += *((INT16*) (*pm_new + size * *n_pm_new));
    (*n_pm_new)++;
  }
  diff = length - sum;
  sum = 0;
  if (diff < 0) {
    while (sum > diff) {
      i_pm = (INT32) ((FLOAT64) (*n_pm_new) * (FLOAT64) (sum - 1) / (FLOAT64) (diff - 1));
      (*((INT16*) (*pm_new + size * i_pm)))--;
      sum--;
    }
  } else {
    while (sum < diff) {
      i_pm = (INT32) ((FLOAT64) (*n_pm_new) * (FLOAT64) (sum + 1) / (FLOAT64) (diff + 1));
      (*((INT16*) (*pm_new + size * i_pm)))++;
      sum++;
    }
  }
  return O_K;
}

/**
 * <p>Expand/reduce number of pitch markers to new number</p>
 *
 * @param pm_old
 *          Pointer to old pitch markers.
 * @param n_pm_old
 *          Number of pitch markers, equals to length of array <code>pm_old</code>.
 * @param pm_new
 *          Pointer to new pitch markers.
 * @param n_pm_new
 *          Number of pitch marks, equals to length of array <code>pm_new</code>.
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 dlm_pm_compress(INT16* pm_old, INT32 n_pm_old, INT16* pm_new, INT32 n_pm_new) {
  INT32 i_pm_old;
  INT32 i_pm_new;
  INT32 sum_old;
  INT32 sum_new;
  INT32 sum;
  INT32 mean_v = 0;
  INT32 mean_i = 0;
  INT16 mean_s = 0;

  if (!pm_old || !pm_new) {
    return NOT_EXEC;
  }

  for (sum_old = 0, i_pm_old = 0; i_pm_old < n_pm_old; i_pm_old++)
    sum_old += pm_old[2 * i_pm_old];

  for (sum = 0, i_pm_old = 0, i_pm_new = 0; i_pm_new < n_pm_new; i_pm_new++) {
    sum_new = (i_pm_new + 1) * sum_old / n_pm_new;

    if (sum < sum_new) {
      mean_v = 0;
      mean_s = 0;
      mean_i = 0;
      while (sum + mean_v < sum_new) {
        mean_v += pm_old[2 * i_pm_old];
        mean_s += pm_old[2 * i_pm_old + 1] && 1;
        mean_i++;
        i_pm_old++;
      }
      sum += mean_v;
      mean_v = (INT32) ((FLOAT64) mean_v / (FLOAT64) mean_i + 0.5);
      mean_s = (INT32) ((FLOAT64) mean_s / (FLOAT64) mean_i + 0.5);
      pm_new[2 * i_pm_new] = (INT16) mean_v;
      pm_new[2 * i_pm_new + 1] = (INT16) mean_s;
    } else {
      pm_new[2 * i_pm_new] = (INT16) mean_v;
      pm_new[2 * i_pm_new + 1] = (INT16) mean_s;
    }
  }
  return O_K;
}

/**
 * <p>Convert (unequal spaced) pitch markers to f0-contour with equal spaced
 * sampling points.</p>
 *
 * @param pm
 *          Pointer to pitch markers.
 * @param n_pm
 *          Number of pitch markers, equals to length of array <code>pm</code>.
 * @param f0
 *          Pointer to target f0-contour
 * @param n_f0
 *          Number of f0 sampling points, equals to length of array <code>f0</code>.
 * @param sampling_rate
 *          sampling rate.
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 dlm_pm2f0(INT16* pm, INT32 n_pm, FLOAT64* f0, INT32 n_f0, INT32 sampling_rate) {
  INT32 i_pm = 0;
  INT32 i_f0 = 0;
  INT32 i_sum_pm = 0;
  INT32 n_sum_pm = 0;

  for (n_sum_pm = 0, i_pm = 0; i_pm < n_pm; i_pm++)
    n_sum_pm += *(pm + 2 * i_pm);

  for (i_f0 = 0, i_pm = 0; i_f0 < n_f0; i_f0++) {
    while (((FLOAT64) i_sum_pm * (FLOAT64) n_f0 < ((FLOAT64) i_f0 + 0.5) * (FLOAT64) n_sum_pm) && (i_pm < n_pm - 1)) {
      i_sum_pm += *(pm + 2 * i_pm);
      i_pm++;
    }
    *(f0 + i_f0) = ((*(pm + 2 * i_pm + 1) == 0) ? -1.0 : 1.0) * (FLOAT64) sampling_rate / (FLOAT64) *(pm + 2 * i_pm);
  }

  return O_K;
}

INT16 CGEN_PUBLIC dlm_getExcPeriod(INT16 nLen, BOOL bVoiced, INT8 type, FLOAT64 nScale, INT32 nSRate, FLOAT64* exc) {
  INT32 i_samples = 0;
  FLOAT64 glott_t = 0;
  FLOAT64 glott_alpha = 5.0;
  FLOAT64 glott_beta = 100.0;
  FLOAT64 glott_ee = 1.0;
  FLOAT64 glott_e0 = glott_ee / (exp(glott_alpha * 5.0 / 8.0) * sin(2.0 * F_PI * 5.0 / 8.0));
  FLOAT64 glott_e1 = glott_ee / (1.0 - exp(-glott_beta * (1.0 - 5.0 / 8.0)));
  FLOAT64 glott_tc = 5.0 / 8.0;
  COMPLEX64* sf = NULL;

  if (bVoiced) {
    switch (type) {
    case DLM_PITCH_VOICED:
    case DLM_PITCH_PULSE:
      exc[0] = 1.0 / nScale;
      dlp_memset(exc + 1, 0, (nLen - 1) * sizeof(FLOAT64));
      break;
    case DLM_PITCH_GLOTT:
      for (i_samples = 0; i_samples < nLen; i_samples++) {
        glott_t = (FLOAT64) (i_samples) / (FLOAT64) nLen;
        if (glott_t < glott_tc) {
          exc[i_samples] = -glott_e0 * exp(glott_alpha * glott_t) * sin(2.0 * F_PI * glott_t) * 15.0 / nLen / nScale;
        } else {
          exc[i_samples] = glott_e1 * (exp(-glott_beta * (glott_t - glott_tc)) - exp(-glott_beta * (1.0 - glott_tc))) * 15.0 / nLen / nScale;
        }
      }
      break;
    case DLM_PITCH_RANDPHASE:
      sf = (COMPLEX64*) dlp_calloc(nLen, sizeof(COMPLEX64));
      for (i_samples = 1; i_samples < nLen / 2; i_samples++) {
        FLOAT64 phase = -15.0 * nSRate * ((FLOAT64) (2 * i_samples) / (FLOAT64) nLen - 8000.0 / (FLOAT64) nSRate) / 8000.0;
        phase = dlp_rand() / (FLOAT64) RAND_MAX / (1 + exp(phase)) * 2.0 * F_PI;
        sf[i_samples] = CMPLXY(cos(phase) / nScale, sin(phase) / nScale);
        sf[nLen - i_samples] = CMPLXY(sf[i_samples].x, -sf[i_samples].y);
      }
      dlm_dftC(sf, nLen, TRUE, sf);
      for (i_samples = 0; i_samples < nLen; i_samples++) {
        exc[i_samples] = sf[i_samples].x;
      }
      dlp_free(sf)
      ;
      break;
    case DLM_PITCH_UNVOICED:
      for (i_samples = 0; i_samples < nLen; i_samples++) {
        exc[i_samples] = (((FLOAT64) dlp_rand() / (FLOAT64) RAND_MAX) - 0.5f) / sqrt(12.0) / nScale;
      }
      break;
    default:
      return NOT_EXEC;
    }
  } else {
    if (type == DLM_PITCH_VOICED) {
      exc[0] = 1.0 / nScale;
      dlp_memset(exc + 1, 0, (nLen - 1) * sizeof(FLOAT64));
    } else {
      for (i_samples = 0; i_samples < nLen; i_samples++) {
        exc[i_samples] = (((FLOAT64) dlp_rand() / (FLOAT64) RAND_MAX) - 0.5f) / sqrt(12.0) / nScale;
      }
    }
  }
  return O_K;
}

/**
 * <p>Convert pitch markers to excitation signal</p>
 *
 * @param pm
 *          Pointer to pitch markers.
 * @param n_pm
 *          Number of pitch markers, equals to length of array <code>pm_old</code>.
 * @param gains
 *          Pointer to target gains of pitch pulses of excitation signal.
 *          Array length must be equal to <code>n_pm</code>.
 * @param exc
 *          Pointer to excitation.
 * @param n_exc
 *          Length of excitation, equals to length of array
 *          <code>exc</code>.
 * @param sync
 *          If <code>FALSE</code> the pitch markers are assumed to be at
 *          equidistant sampling points. If <code>TRUE</code> it is assumed that
 *          the position of the pitch markers corresponds to the lengths of periods
 *          given by the pitch markers. In this case <code>n_exc</code> is used as
 *          the target excitation length.
 * @param type
 *          Type of excitation. <code>type</code> must be one of
 *          DLM_PITCH_PULSE: Pulse train for voiced, white noise for voiceless,
 *          DLM_PITCH_GLOTT: Glottis function for voiced, white noise for voiceless.
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 dlm_pm2exc(INT16* pm, INT32 n_pm, FLOAT64** exc, INT32* n_exc, INT32 nSrate, BOOL sync, INT8 type) {
  INT32 i_pm;
  INT32 n_pm_new;
  INT32 i_exc;
  INT16* pm_new = NULL;
  INT16* pm_tmp = NULL;

  if (sync == FALSE) {
    pm_tmp = (INT16*) dlp_calloc(n_pm, 2 * sizeof(INT16));
    if (!pm_tmp)
      return NOT_EXEC;
    dlp_memmove(pm_tmp, pm, n_pm * 2 * sizeof(INT16));

    if (dlm_pm_expand_impl(pm_tmp, n_pm, &pm_new, &n_pm_new, 2 * sizeof(INT16), *n_exc) == NOT_EXEC) {
      dlp_free(pm_tmp);
      return NOT_EXEC;
    }
    dlp_free(pm_tmp);
    return dlm_pm2exc(pm_new, n_pm, exc, n_exc, TRUE, type, nSrate);
  } else {
    pm_new = pm;
    n_pm_new = n_pm;
  }

  if (!pm_new || !exc || !n_exc) {
    return NOT_EXEC;
  }

  *n_exc = 0;
  for (i_pm = 0; i_pm < n_pm_new; i_pm++) {
    *n_exc += pm_new[2 * i_pm];
  }

  if (*exc == NULL)
    *exc = (FLOAT64*) dlp_calloc(*n_exc, sizeof(FLOAT64));

  for (i_exc = 0, i_pm = 0; i_exc < *n_exc; i_exc += pm_new[2 * i_pm], i_pm++) {
    dlm_getExcPeriod(pm_new[2 * i_pm], pm_new[2 * i_pm + 1], type, 1.0, nSrate, *exc + i_exc);
  }

  return O_K;
}

/**
 * <p>Convert f0-contour with equal spaced sampling points to pitch markers.</p>
 *
 * @param f0
 *          Pointer to f0-contour.
 * @param n_f0
 *          Number f0-sampling points, equals to length of array <code>f0</code>.
 * @param pm
 *          Pointer to target f0-contour
 * @param n_pm
 *          Number of pitch markers, equals to the resulting length of array <code>pm</code>.
 * @param n
 *          target sum of period lengths
 * @param sampling_rate
 *          sampling rate.
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 dlm_f02pm(FLOAT64* f0, INT32 n_f0, INT16** pm, INT32* n_pm, INT32 n, INT32 sampling_rate) {
  INT32 i = 0;
  INT32 i_f0 = 0;
  INT32 i_pm = 0;
  INT32 n_per = 0;
  *pm = NULL;

  while (i < n) {
    i_f0 = i * n_f0 / n;
    *pm = (INT16*) dlp_realloc(*pm, i_pm + 1, 2 * sizeof(INT16));
    n_per = (INT16) ((FLOAT64) sampling_rate / *(f0 + i_f0) + 0.5);
    if (n_per < 0) {
      *(*pm + 2 * i_pm) = -n_per;
      *(*pm + 2 * i_pm + 1) = 0;
      i -= n_per;
    } else {
      *(*pm + 2 * i_pm) = n_per;
      *(*pm + 2 * i_pm + 1) = 1;
      i += n_per;
    }
    i_pm++;
  }

  *n_pm = i_pm;

  return O_K;
}

/**
 * <p>F0 Estimation via cepstrum.</p>
 *
 * @param X Input signal
 * @param n_len Input signal length
 * @param n_min Minimum F<sub>0</sub>
 * @param n_max Maximum F<sub>0</sub>
 * @param Y Output F<sub>0</sub> value;
 */
INT16 dlm_getF0Cepstrum(FLOAT64* X, INT32 n_len, INT32 n_min, INT32 n_max, INT32* Y) {
  INT32 i = 0;
  INT32 n_fft = 1;
  INT32 n_peak_i = 0;
  FLOAT64 a0 = 1.0;
  FLOAT64 sum = 0.0;
  FLOAT64 n_peak_v = 0.0;
  FLOAT64* R = NULL;
  FLOAT64* I = NULL;
  FLOAT64* A = NULL;

  while (n_fft < n_len)
    n_fft <<= 1;

  R = (FLOAT64*) dlp_calloc(n_fft, sizeof(FLOAT64));
  I = (FLOAT64*) dlp_calloc(n_fft, sizeof(FLOAT64));
  A = (FLOAT64*) dlp_calloc(n_min, sizeof(FLOAT64));
  if (!R || !I || !A)
    return NOT_EXEC;

  dlm_lpc_burg(X, n_len, A, n_min, 1.0);
  dlm_gmult(A, A, n_min, -1.0);
  a0 = A[0];
  A[0] = 1.0;
  dlm_filter(A, n_min, &a0, 1, X, R, n_len, NULL, 0);
  dlm_fba_window(R, n_len, "hamming", TRUE);

  /*  dlp_memmove(R, X, n_len * sizeof(FLOAT64));*/
  dlm_fft(R, I, n_fft, FALSE);
  R[0] = log(CMPLX_ABS(CMPLXY(R[0],I[0])));
  I[0] = 0.0;
  for (i = 1; i <= n_fft / 2; i++) {
    R[i] = log(CMPLX_ABS(CMPLXY(R[i],I[i])));
    R[n_fft - i] = R[i];
    I[i] = I[n_fft - i] = 0.0;
  }
  dlm_fft(R, I, n_fft, TRUE);

  n_max = MIN(n_fft, n_max);

  for (sum = 0.0, i = n_min; i < n_max; i++) {
    sum += R[i] * R[i];
  }
  sum = sqrt(sum / (FLOAT64) (n_max - n_min));

  n_peak_v = R[n_min] / sum;
  n_peak_i = n_min;
  for (i = n_min + 1; i < n_max; i++) {
    if (n_peak_v < R[i] / sum) {
      n_peak_v = R[i] / sum;
      n_peak_i = i;
    }
  }

  if (n_peak_v < 3.0) {
    *Y = -n_peak_i;
  } else {
    *Y = n_peak_i;
  }

  dlp_free(A);
  dlp_free(R);
  dlp_free(I);
  return O_K;
}

#define MINMAX(x,Min,Max) {if((x)<(Min)){(x)=(Min);}if((x)>(Max)){(x)=(Max);}}
#define VOICEDF0SEGMENT_MAXLEN 1000
#define STROHCOUNT 3
#define HIGH_PASS 0
#define LOW_PASS  1
#define P_LEN   20
#define SUBSTARTLEN 10
#define STUFE3

typedef struct {
  FLOAT64 rms;
  FLOAT64 ndg;
} GCIDA_NRS;

typedef struct {
  INT32 left;
  INT32 right;
} GCIDA_SEG;

typedef struct {
  INT32 uvmW;
  INT32 uvm;
  char swap_in, swap_out;
} GCIDA_PARA;

/*------------------------------------------------------------
 --------------------------------------------------------------
 HELPER FUNCTIONS
 --------------------------------------------------------------
 ------------------------------------------------------------*/

/*------------------------------------------------------------
 function:       LinearFilter

 Hoch- oder Tiefpass-Filterung eines Signals mit FFT-Filter.

 Parameter
 x       Eingabesignal = Ausgabesignal
 Lx      Laenge des Signals
 Wn      cutoff-Frequenz
 type    Filtertyp (HIGH_PASS oder LOW_PASS)
 Lw      Fensterlaenge (hanning-Fenster)
 ------------------------------------------------------------*/
void CGEN_PRIVATE dlm_gcida_LinearFilter(FLOAT64 *x, INT32 Lx, FLOAT64 Wn, INT32 type, INT32 Lw) {
  INT32 l, l2;
  FLOAT64 *win = NULL;
  FLOAT64 *re = NULL;
  FLOAT64 *im = NULL;
  FLOAT64 *y = NULL;
  FLOAT64 *y1 = NULL;
  INT32 incarg = 0;
  INT32 LwP2 = (INT32) floor(log((FLOAT64) Lw) / log(2.0f)); /* MAX(LwP2,3); */
  Lw = (INT32) pow(2, LwP2);

  INT32 N_l = 0;
  INT32 N_h = 0;
  if (type == HIGH_PASS) {
    N_l = 0;
    N_h = (INT32) floor((FLOAT64) Lw * (FLOAT64) Wn / 2.0f);
  }
  if (type == LOW_PASS) {
    N_l = (INT32) ceil((FLOAT64) Lw * (FLOAT64) Wn / 2.0f);
    N_h = Lw / 2;
  }

  win = (FLOAT64*) dlp_calloc(Lw, sizeof(FLOAT64));
  dlm_fba_makewindow(win, Lw, "hanning", FALSE);
  re = (FLOAT64*) dlp_calloc(Lw, sizeof(FLOAT64));
  im = (FLOAT64*) dlp_calloc(Lw, sizeof(FLOAT64));

  y = (FLOAT64*) dlp_calloc((INT32 )(Lx + 4 * Lw), sizeof(FLOAT64));
  y1 = (FLOAT64*) dlp_calloc((INT32 )(Lx + 4 * Lw), sizeof(FLOAT64));
  dlp_memmove(&y[2 * Lw], x, Lx * sizeof(FLOAT64));

  /* Fensterfortsetzrate = 0.25*Fensterlaenge */
  incarg = (INT32) (Lw * 0.25);

  /* copy and windowing */
  for (l2 = 0; l2 < (Lx + 3 * Lw); l2 += incarg) {
    for (l = 0; l < Lw; l++) {
      re[l] = y[l + l2] * win[l];
      im[l] = 0;
    }

    dlm_fft(re, im, Lw, FALSE);
    /* entsprechend Vorgabe Spektrallinien = 0 setzen */
    for (l = N_l; l <= N_h; l++) {
      re[l] = 0;
      im[l] = 0;
    }
    for (l = Lw - N_l - 1; l >= Lw - N_h - 1; l--) {
      re[l] = 0;
      im[l] = 0;
    }
    dlm_fft(re, im, Lw, TRUE);

    /* Werte zurueckschreiben */
    for (l = 0; l < Lw; l++) {
      y1[l + l2] = y1[l + l2] + 2.0 * re[l] * win[l] / 3.0;
    }
  }
  dlp_memmove(x, &y1[2 * Lw], Lx * sizeof(FLOAT64));
  if (win != NULL)
    dlp_free(win);
  if (re != NULL)
    dlp_free(re);
  if (im != NULL)
    dlp_free(im);
  if (y != NULL)
    dlp_free(y);
  if (y1 != NULL)
    dlp_free(y1);
}

/*------------------------------------------------------------
 function:        Decimation
 ------------------------------------------------------------*/
INT32 CGEN_PRIVATE dlm_gcida_Decimation(FLOAT64 *x, INT32 Lx, FLOAT64 *xr, INT32 Lxr, INT32 fs) {
  FLOAT64 M = (FLOAT64) fs / 16000.0f;
  INT32 l = 0;

  if (fs > 16000) {
    dlm_gcida_LinearFilter(x, Lx, 8000.0f / fs * 2.0f, LOW_PASS, (INT32) ceil(M * 1024.0f));
  }

  for (l = 0; l < Lxr; l++) {
    xr[l] = x[(INT32) floor(M * (FLOAT64) l)];
  }

  if (fs < 16000) {
    dlm_gcida_LinearFilter(xr, Lx, 8000.0f / fs * 2.0f, LOW_PASS, 1024);
  }
  return (16000);
}

/*------------------------------------------------------------
 function:        SmallConv

 BOOL SmallConv(FLOAT64 *x, INT32 Lx, FLOAT64 *h, INT32 Lh, FLOAT64 *y, INT32 Ly,INT32 HI, INT32 HS, INT32 HB)
 Berechnet die Faltung an den Stellen, an denen die Filterkoefizient ungleich Null sind.
 *x Eingang
 Lx Laenge Eingang
 h  Koeffizienten des Tiefpass
 Lh unbenutzt
 *y Ausgang
 Ly Laenge Ausgang
 HI Anzahl der Nullen zwischen den Koeffizienten
 HS,HB Anzahl der Koeffizienten ungleich Null
 ------------------------------------------------------------*/
BOOL CGEN_PRIVATE dlm_gcida_SmallConv(FLOAT64 *x, INT32 Lx, FLOAT64 *h, INT32 Lh, FLOAT64 *y, INT32 Ly, INT32 HI, INT32 HS, INT32 HB) {
  FLOAT64 sum;
  INT32 hh, H1, H2, nx;
  Ly = Lx + Lh;
  INT32 ny = 0;
  for (ny = 0; ny < HS; ny++) {
    sum = 0;

    H1 = MIN(ny, Lh - 1);
    nx = ny - H1;
    if (ny - Lx <= 0) {
      for (hh = H1; hh >= 0; hh--) {
        sum += x[nx++] * h[hh];
      }
    } else {
      H2 = ny - Lx;
      for (hh = H1; hh >= H2; hh--) {
        sum += x[nx++] * h[hh];
      }
      nx--;
      for (; hh >= 0; hh--) {
        sum += x[nx--] * h[hh];
      }
    }
    y[ny] = sum;
  }
  for (ny = HS; ny < Ly; ny++) {
    sum = 0;

    H1 = MIN(ny, HS);
    nx = ny - H1;
    if (ny - Lx <= 0) {
      for (hh = H1; hh >= HB;) {
        sum += x[nx] * h[hh];
        nx += HI;
        hh -= HI;
      }
    } else {
      H2 = ny - Lx;
      for (hh = H1; hh >= H2;) {
        sum += x[nx++] * h[hh];
        nx += HI;
        hh -= HI;
      }
      nx--;
      for (; hh >= 0;) {
        sum += x[nx] * h[hh];
        nx -= HI;
        hh -= HI;
      }
    }
    y[ny] = sum;
  }
  return (0);
}

/*------------------------------------------------------------
 function:        MyMallat

 Calculate Dyadic Wavelet Transformation with with special Mallat-Algorithm
 (algorithm a trous B-Spline Wavelet

 Parameter:
 x       Eingabesignal
 P       Feld mit Vektoren (Zeilen) von Wavelet-Koeffizienten
 Lx      Laenge Eingabesignal
 x       input data
 P       output data (array with a vector (row) for each scale)
 scales  vector with 2 elements: scales[0] = min scale; scale[1] = max scale
 Lx      length of input data

 ------------------------------------------------------------*/
BOOL CGEN_PRIVATE dlm_gcida_MyMallat(FLOAT64 *x, FLOAT64 **P, INT16 *scales, INT32 Lx) {
  INT32 z, mempos, l;
  INT32 filter_length = 6;

  /* max length of spreaded filter is 2^max_scale * filter length */
  INT32 Lh = (INT32) pow(2, scales[1]) * filter_length;
  INT32 Lh_2 = Lh / 2;

  FLOAT64 *H = NULL;
  H = (FLOAT64*) dlp_calloc(Lh, sizeof(FLOAT64));

  FLOAT64 *G = NULL;
  G = (FLOAT64*) dlp_calloc(Lh, sizeof(FLOAT64));

  FLOAT64 *S1 = NULL;
  S1 = (FLOAT64*) dlp_calloc(Lx + Lh, sizeof(FLOAT64));

  FLOAT64 *S = NULL;
  S = (FLOAT64*) dlp_calloc(Lx + Lh, sizeof(FLOAT64));

  INT32 HB = 0, HS = 0, HI = 0;

  /* Define filters: HM=scaling function lowpass filter; GM=wavelet highpass filter */
  FLOAT64 HM[] = { 0.03125, 0.15625, 0.3125, 0.3125, 0.15625, 0.03125 };
  FLOAT64 GM[] = { 0.0, 0.0, 2.0, -2.0, 0.0, 0.0 };
  /* copy input data in temporary variable */
  dlp_memmove(S, x, Lx * sizeof(FLOAT64));

  /* temporary variable with filtered data */
  INT32 Ly = Lx + Lh;
  FLOAT64 *y = NULL;
  y = (FLOAT64*) dlp_calloc(Ly, sizeof(FLOAT64));

  /* from beginning to max scale */
  for (l = 0; l <= scales[1]; l++) {
    /* set filter output to zero */
    dlp_memset(G, 0, Lh * sizeof(FLOAT64));
    dlp_memset(H, 0, Lh * sizeof(FLOAT64));

    /* for all filter coefficients */
    for (z = 1; z <= filter_length; z++) {
      /* Berechne genau die Positionen im gespreizten Filter, an denen Koeffizienten != Null stehen */
      mempos = (INT32) ceil((pow(2, l) * (z - 3.5) + filter_length * pow(2, (scales[1] - 1))));

      /* trage dort die entsprechenden Filterkoeffizienten ein */
      G[mempos] = GM[z - 1];
      H[mempos] = HM[z - 1];

      /* HI Anzahl der Nullen zwischen den Koeffizienten
       HS, HB Anzahl der Koeffizienten ungleich Null
       HB = H Beginn = Position des ersten Koeff != Null
       HS = H Stop = Position des letzten Koeff != Null */
      if (z == 1)
        HB = mempos;
      if (z == filter_length) {
        HS = mempos;
        HI = (INT32) pow(2, l);
      }
    }
    if (l >= scales[0]) {
      dlp_memset(y, 0, Ly * sizeof(FLOAT64));
      dlm_gcida_SmallConv(S, Lx, G, Lh, y, Ly, HI, HS, HB);
      dlp_memmove(P[l], &y[Lh_2], Lx * sizeof(FLOAT64));
    }
    dlm_gcida_SmallConv(S, Lx, H, Lh, S1, Lx + Lh, HI, HS, HB);
    dlp_memmove(S, &S1[Lh_2], Lx * sizeof(FLOAT64));
  }

  if (y != NULL)
    dlp_free(y);
  if (S != NULL)
    dlp_free(S);
  if (S1 != NULL)
    dlp_free(S1);
  if (G != NULL)
    dlp_free(G);
  if (H != NULL)
    dlp_free(H);

  return (0);
}

/*------------------------------------------------------------
 function:        MLNeighbor

 Sucht optimalen Nachbarn unter der Bedingung von T_0.

 FLOAT64 *P          Eingangsvektor
 INT32   LP          Laenge Eingangsvektor
 INT32   P_io        Startposition (gezaehlt von P[0])
 FLOAT64 T_0         Abstand von Startposition
 FLOAT64 sigma       Standardabweichung
 char               DirFlag Richtung von P_io aus
 FLOAT64 *V          Ausgabe des optimalen Wertes

 Rueckgabewert:
 Position des Optimums gez�hlt von P[0]
 ------------------------------------------------------------*/
INT32 CGEN_PRIVATE dlm_gcida_MLNeighbor(FLOAT64 *P, INT32 LP, INT32 P_io, FLOAT64 T_0, FLOAT64 sigma, char DirFlag, FLOAT64 *V) {
  INT32 l;
  INT32 P_i = 0;
  FLOAT64 val;

  *V = 0;
  if (DirFlag == 0) {
    for (l = 0; l < LP; l++) {
      val = P[l] * exp(-(pow(T_0 - (FLOAT64) (l - P_io), 2) / (2 * sigma * sigma)));
      if ((val > *V) && P[l] != 0) {
        *V = val;
        P_i = l;
      }
    }
  }
  if (DirFlag == 1) {
    for (l = 0; l < LP; l++) {
      val = P[l] * exp(-(pow(T_0 - (FLOAT64) abs(l - P_io), 2) / (2 * sigma * sigma)));
      if ((val > *V) && P[l] != 0) {
        *V = val;
        P_i = l;
      }
    }
  }

  if (DirFlag == 2) {
    for (l = 0; l < LP; l++) {
      val = exp(-(pow(T_0 - (FLOAT64) (l - P_io), 2) / (2 * sigma * sigma)));
      if ((val > *V) && P[l] != 0) {
        *V = val;
        P_i = l;
      }
    }
  }

  return (P_i);
}

/*------------------------------------------------------------
 function:        dlm_gcida_NDG_RMS

 NRS dlm_gcida_NDG_RMS(FLOAT64 *x, INT32 Lw)
 Berechnet Energie und die Haeufigkeit der positiven Nulldurchgaenge von x

 *x Eingangsvektor der Laenge Lw

 Rueckgabe:
 NRS.rms Leistung
 NRS.ndg Haufigkeit der Nulldurchgaenge

 ------------------------------------------------------------*/
GCIDA_NRS CGEN_PRIVATE dlm_gcida_NDG_RMS(FLOAT64 *x, INT32 Lw) {
  INT32 l, ndg = 0;
  FLOAT64 my_rms = 0;

  for (l = 0; l < Lw - 1; l++) {
    my_rms += (x[l] * x[l]);
    if (x[l] < 0 && x[l + 1] >= 0)
      ndg++; /* Nur Nulldurchgaenge von neg. nach pos. werden gezaehlt */
  }
  my_rms += x[l] * x[l]; /* "l" ist schon vom letzten Schleifendurchlauf incrementiert */

  GCIDA_NRS r;
  r.rms = sqrt(1.0f / (FLOAT64) Lw * my_rms);
  /* r.ndg=1.0f/(FLOAT64)Lw*(FLOAT64)ndg; */r.ndg = (FLOAT64) ndg / (FLOAT64) Lw;

  return (r);
}

/**
 * <p>Glottal Closure Instance Detection Algorithm (GCIDA).</p>
 *
 * Reference: Engel, T., Robuste Markierung von Grundfrequenzperioden. Dresden University, Diploma thesis, 2003 (in German).
 *
 * @param X Input signal
 * @param n_len Input signal length
 * @param n_min Minimum F<sub>0</sub>
 * @param n_max Maximum F<sub>0</sub>
 * @param Y Output F<sub>0</sub> value;
 */
INT16 dlm_gcida(FLOAT64* samples, INT32 nSamples, PERIOD** periods, INT32* nPeriods, INT32 m_nSrate, INT32 m_nMin, INT32 m_nMean, INT32 m_nMax) {
  /*-------------------
   Globale Variablen */
  INT32 fsr = 0; /* fsr .. samplrate DECIMATION */
  INT32 Band[3];
  INT32 T0_min = 0, T0_max = 0;
  INT32 T0_minr = 0, T0_maxr = 0;
  INT32 Lx = 0; /* Laenge in Abtastwerten des Eingangs-Sprachsignals x */
  INT32 pLw = 0, Lw = 0; /* Fensterlaenge orig. Sprachsignal */
  INT32 Lxr = 0; /* Laenge decimiertes/downsampled Sprachsignals xr */
  INT32 pLwr = 0, Lwr = 0; /* Fensterlaenge dezimiertes Sprachsignal */
  INT32 ovlp = 0, F0Anzahl = 0;
  INT32 l1 = 0;
  INT32 l = 0;

  FLOAT64 M = 0; /* DECIMATION ratio */

  GCIDA_PARA Para;

  FLOAT64 *x = NULL;
  FLOAT64 *xr = NULL;
  FLOAT64 *x_out = NULL;
  FLOAT64 *vuv = NULL;
  FLOAT64 *vu2 = NULL;
  FLOAT64 *F0 = NULL;
  FLOAT64 *F = NULL;
  FLOAT64 *F_ptr = NULL;
  FLOAT64 *Fw = NULL;
  FLOAT64 *Fw_ptr = NULL;
  FLOAT64 *win = NULL;
  FLOAT64 *win_ptr = NULL;
  FLOAT64 *r = NULL;
  FLOAT64 *r_ptr = NULL;
  FLOAT64 *k = NULL;
  FLOAT64 *k_ptr = NULL;
  FLOAT64 *a = NULL;
  FLOAT64 *a_ptr = NULL;
  FLOAT64 *tabs = NULL;
  FLOAT64 *P[P_LEN];
  INT16 scales[2];
  FLOAT64 *MD = NULL;
  FLOAT64 *MD_ptr = NULL;
  FLOAT64 *re = NULL;
  FLOAT64 *re_ptr = NULL;
  FLOAT64 *im = NULL;
  FLOAT64 *im_ptr = NULL;
  FLOAT64 acc_filt[2] = { 1, 1 };
  FLOAT64 b1 = 0, ABA = 0;
  FLOAT64 Fd = 0;

  INT32 GMP = 0; /* vorhergehende Schaetzunbg der Grundfrequenz */
  GCIDA_NRS ndgrms; /* Struktur fuer Nulldurchgangsdichte und Effektivwerte anlegen */
  INT32 c2, c = 0;
  FLOAT64 incarg = 0;
  FLOAT64 ws = 0;
  FLOAT64 mean = 0;
  FLOAT64 *x_a = NULL;

  INT32 vuv_idx;
  INT32 LschaetzF0 = 0;
  FLOAT64 vuv_o = 0;

  /* -- EXTRAKTIONs-Variablen -- */
  FLOAT64 MW = 0;
  INT32 MP = 0;
  INT32 MPoo = 0, MPo = 0;
  FLOAT64 *x_sum = NULL;
  FLOAT64 *WTmaxima = NULL;

  /* -- Nachverarbeitungs-Variablen -- */
  FLOAT64 *PMSuchbereich = NULL;
  FLOAT64 *PM = NULL;
  FLOAT64 *ZeroArray = NULL;

  /* Pr�Initialisierung */
  INT32 fs = m_nSrate;
  Band[1] = 50;
  Band[2] = 500;

  Para.uvm = 0;
  Para.uvmW = 0;
  Para.swap_out = 0;
  Para.swap_in = 0;

  /* Parameter einlesen */

  Band[1] = m_nMin;
  Band[2] = m_nMax;

  Para.uvm = 2;
  Para.uvmW = m_nMean;

  /* bei Binaer-Daten nur Laenge der Datei auslesen */
  Lx = nSamples;
  x = samples;

  /*-------------------------------
   Signal auf 1.0 normalisieren */
  FLOAT64 max_x = 0;
  for (l = 0; l < Lx; l++) {
    if ((x[l] > max_x) || (-x[l] > max_x)) {
      max_x = ABS(x[l]);
    }
  }
  for (l = 0; l < Lx; l++) {
    x[l] /= max_x;
  }

  /*------------------------------------
   Abtastrate auf 16000Hz konvertieren */
  fsr = fs; /* fsr .. samplrate DECIMATION */
  M = (FLOAT64) fs / 16000.0f;
  Lxr = (INT32) ceil((FLOAT64) Lx / M);
  xr = (FLOAT64*) dlp_calloc(Lxr, sizeof(FLOAT64));

  if (fs != 16000) {
    fsr = dlm_gcida_Decimation(x, Lx, xr, Lxr, fs); /* Kein echtes Resampling! */
  } else {
    dlp_memmove(xr, x, Lx * sizeof(FLOAT64));
  }

  /*------------------------
   PostInitialisierungen */
  x_out = (FLOAT64*) dlp_calloc(Lx, sizeof(FLOAT64));

  pLw = (INT32) ceil(log(fs * 0.04) / log(2)); /* Fensterl�nge: 40 ms */
  pLwr = (INT32) ceil(log(fsr * 0.04) / log(2)); /* Fensterl�nge: 40 ms */

  Lw = (INT32) pow(2, pLw);
  Lwr = (INT32) pow(2, pLwr);
  ovlp = 3;

  /* INT32 FrameStepSize = (INT32)(round(Lw / pow(2,ovlp))); */
  /* INT32 FrameStepSizer= (INT32)(round(Lwr / pow(2,ovlp))); */
  FLOAT64 vuvTH = 0.5;

  T0_min = (INT32) floor(fs / Band[2]);
  T0_max = (INT32) ceil(fs / Band[1]);
  T0_minr = (INT32) floor(fsr / Band[2]);
  T0_maxr = (INT32) ceil(fsr / Band[1]);

  if (Para.uvmW == 0) {
    Para.uvmW = T0_min;
  }

  /*----------------------------------------------------------------
   ----------------------------------------------------------------
   1.STUFE: Kurzzeitanalyse
   ----------------------------------------------------------------
   ----------------------------------------------------------------*/
  /* Prinzip:
   Fensterweise
   -- Vorverarbeitung --
   Fensterung von Signalsegment durchfuehren
   Nulldurchgangsdichte und Energie ermitteln
   Inverse Filterung:
   Berrechnung der Autokeorrelationskoeffizienten
   levinson-durbin rekursion
   Allpol invertieren
   (ungefenstertes) Signalsegment mit LPC-Koeff. filtern = Inverses
   bzw. Residualsignal

   Entrauschen durch DyWT
   Akkumulation des Inversen bzw. Residualsignals
   Quadratic Spline DyWT des akkumulierten Signals mit scalen 3,4,5
   fuer jede Scale Schwelle = 0.5 * Koeff-Mittelwert bilden und
   Werte > Schwelle fuer alle Scales zeitsynchron aufsummieren

   AKF des entrauschten Inversen ueber doppelte Spektraltransfomation
   berechnen, d.h. FFT, |X^2|, IFFT

   -- EXTRAKTION --
   Lokale Maxima in AKF-Signal finden

   Kontinuitaet zwischen F0-Werten aufeinanderfolgender Segmente ermitteln
   Kontinuitaet = Sicherheit der F0-Schaetzung

   Klassifikation von Anregungszustand und Sicherheit der F0-Schaetzung
   Listenkorrektur der Anregung
   ist F0 < F0min oder F0 > F0max, dann unvoiced und unsichere Schaetzung
   Sicherstellen, dass F0min <= F0 <= F0max

   Segmente aus sicher geschaetzten F0-Werten bilden
   ----------------------------------------------------------------
   ----------------------------------------------------------------*/

  GCIDA_SEG voicedF0segment[VOICEDF0SEGMENT_MAXLEN];

  LschaetzF0 = (INT32) (Lxr / Lwr * pow(2, ovlp));
  /*x_out = (FLOAT64*)dlp_calloc(Lx, sizeof(FLOAT64));*/
  vuv = (FLOAT64*) dlp_calloc(LschaetzF0, sizeof(FLOAT64)); /* Schaetzung der Robustheit Feld */
  vu2 = (FLOAT64*) dlp_calloc(LschaetzF0, sizeof(FLOAT64)); /* Voiced/Unvoiced -Hypothesen Feld */
  F0 = (FLOAT64*) dlp_calloc(LschaetzF0, sizeof(FLOAT64)); /* Grundfrequenz Schaetzung */

  F = (FLOAT64*) dlp_calloc(Lwr, sizeof(FLOAT64));
  F_ptr = F; /* Signalabschitt */
  Fw = (FLOAT64*) dlp_calloc(Lwr, sizeof(FLOAT64));
  Fw_ptr = Fw; /* gefensterter Signalabschnitt */
  win = (FLOAT64*) dlp_calloc(Lwr, sizeof(FLOAT64));
  win_ptr = win; /* (Hamming) Fenster */

  INT32 ORD = 24;

  r = (FLOAT64*) dlp_calloc(Lwr, sizeof(FLOAT64));
  r_ptr = r; /* Autokorellationskoeffizienten */
  k = (FLOAT64*) dlp_calloc(Lwr, sizeof(FLOAT64));
  k_ptr = k; /* Reflexionskoeffizienten */
  a = (FLOAT64*) dlp_calloc(Lwr, sizeof(FLOAT64));
  a_ptr = a; /* Praediktor - Koeffizienten */
  tabs = (FLOAT64*) dlp_calloc(Lwr, sizeof(FLOAT64));

  /* - Scale_min u. Scale_max der WT auf Werte 3 u. 5 setzen - */
  /* scale0 auf ld(dezimierte fs in kHz)-1 festlegen */
  scales[0] = (INT32) ceil(log((FLOAT64) fsr / 1000.0f) / log(2.0f)) - 1;
  /* kleineren Skalierungsfaktor festlegen */

  /* scale1 auf ld(4*fs_dezimiert in kHz)-1 */
  scales[1] = (INT32) ceil(log((FLOAT64) fsr / 250.0f) / log(2.0f)) - 1;
  /* hoeeren Skalierungsfaktor festlegen */

  /* sicherstellen, dass 0 <= scale0/scale1 <= 19 */
  MINMAX(scales[0], 0, 19);
  MINMAX(scales[1], 0, 19);

  for (l = 0; l < P_LEN; l++) {
    P[l] = NULL;
  } /* P[]-Zeiger fuer Fkt dlp_calloc auf NULL setzen */

  /* Speicher fuer WT-Koeff allocieren */
  for (l = scales[0]; l <= scales[1]; l++) {
    P[l] = (FLOAT64*) dlp_calloc(Lwr, sizeof(FLOAT64));
  }

  MD = (FLOAT64*) dlp_calloc(Lwr, sizeof(FLOAT64));
  MD_ptr = MD; /* entrauschtes Segment */

  re = (FLOAT64*) dlp_calloc(Lwr, sizeof(FLOAT64));
  re_ptr = re; /* reller Vektor der FFT */
  im = (FLOAT64*) dlp_calloc(Lwr, sizeof(FLOAT64));
  im_ptr = im; /* imaginaerer Vektor der FFT */

  dlm_fba_makewindow(win, Lwr, "hamming", FALSE);

  c = 0;
  incarg = pow(0.5, ovlp); /* �berlappung der Fenster */
  for (ws = 0; ws < (Lxr / Lwr - 1); ws += incarg) {

    /* -- Vorverarbeitung -- */

    /* Fensterung durchfuehren */
    c++;
    dlp_memmove(F, &xr[(INT32) (ws * Lwr)], Lwr * sizeof(FLOAT64));
    dlp_memmove(Fw, F, Lwr * sizeof(FLOAT64));
    for (l = 0; l < Lwr; l++) {
      Fw[l] *= win[l];
    }

    /* Nulldurchgangsdichte und Energie ermitteln */
    ndgrms = dlm_gcida_NDG_RMS(Fw, Lwr);

    /* Inverse Filterung --> */
    dlm_lpc_burg(F, Lwr, a, ORD+1, 1.0);
    a[0] = 1; /* Erster Koeffizient ist immer 1 */
    dlm_filter_fir(a, ORD+1, F, F, Lwr, NULL, 0);
    /* <--  */

    /* Entrauschen durch DyWT --> */

    /* Akkumulation des Inversen */
    dlm_filter_iir(acc_filt, 2, F, F, Lwr, NULL, 0);

    dlp_memset(MD, 0, Lwr * sizeof(FLOAT64));

    /* Quadratic Spline DyWT */
    dlm_gcida_MyMallat(F, P, scales, Lwr);

    /* fuer jede Scale Schwelle = 0.5 * Koeff-Mittelwert bilden und
     Werte > Schwelle fuer alle Scales zeitsynchron aufsummieren */
    for (l = scales[0]; l <= scales[1]; l++) { /* fuer alle 3 Scales */
      mean = 0;
      /* fuer alle WT-Koeff der Scale */
      for (l1 = (INT32) ceil(Lwr * 0.2); l1 < Lwr * 0.8; l1++) {
        mean += fabs(P[l][l1]); /* WT-Koeff. aufsummieren */
      }

      /* Mittelwert bilden und Schwelle mit 0.5 * Mittelwert festsetzen */
      mean = mean / (Lwr * 0.6) * 0.5;

      /* nur WT-Koeff, die groesser Schwelle sind, uebernehmen */
      for (l1 = 0; l1 < Lwr; l1++) {
        if (P[l][l1] > mean) {
          /* uebernommene Werte zu gleichem Zeitpunkt werden fuer alle
           Scales aufsummiert */
          MD[l1] += P[l][l1];
        }
      }
    }
    /* Entrauschen durch DyWT <-- */

    /* Doppelte Spektraltransfomation (AKF) */
    for (l = 0; l < Lwr; l++) {
      re[l] = MD[l];
      im[l] = 0;
    }
    dlm_fft(re, im, Lwr, FALSE); /* FFT */
    for (l = 0; l < Lwr; l++) {
      re[l] = re[l] * re[l] + im[l] * im[l];
      im[l] = 0;
    } /* abs(X)^2 = sqrt(re^2+im^2)^2 = re^2+im^2 */

    dlm_fft(re, im, Lwr, TRUE); /* AKF */

    /* -- EXTRAKTION -- */
    /* Lokale Maxima finden */
    MW = 0;
    MP = 0;
    MPoo = 0;
    MPo = 0; /* Zur Vermeidung von Grobfehler die groessten drei Maxima zwischenspeichern */
    for (l = T0_maxr; l > T0_minr; l--) {
      if (re[l] > MW && re[l] > re[l - 1]) {
        MW = re[l];
        MPoo = MPo; /* Indizes aktuellem, vorher aktuellem u. vorvorher aktuellem Max merken */
        MPo = MP;
        MP = l;
      }
    }

    /* Interpolation zur genaueren Schaetzung */
    if (MP > 0 && MP < Lwr) {
      b1 = 0.5 * (re[MP - 1] - re[MP + 1]) / (re[MP - 1] - 2 * re[MP] + re[MP + 1]) + MP;
      F0[c] = fsr / b1;
    }

    /* -- NACHVERARBEITUNG -- */

    /* Kontinuitaet ermitteln */
    if (c > 0) {
      Fd = fabs((F0[c - 1] - F0[c])) / F0[c]; /* relative Frequenz-Abweichung */
      if ((Fd > 0.4) && (MPo > 0) && (GMP > 0)) {
        /* Bei grosser Abweichung die weiteren Maxima auf Kontinuitaet untersuchen */
        if (MPoo == 0)
          MPoo = MPo;
        if (vuv[c - 1] > 0) {
          if (MPo > 0) {
            if (abs(MPo - GMP) / GMP < abs(MPoo - GMP) / GMP)
              MP = MPo;
            else
              MP = MPoo;
          }
          /* korrigierten Wert, wie oben, interpolieren */
          F0[c] = (FLOAT64) fsr / (0.5 * (re[MP - 1] - re[MP + 1]) / (re[MP - 1] - 2 * re[MP] + re[MP + 1]) + MP);
        }
      }

      if ((Fd < 0.09) && (vuv[c - 1] > 0)) {
        /* bei geringer Abweichung den Kontinuitaetsparameter hoeher bewerten */
        Fd = -1.0f / (MAX(Fd, 0.001));
      } else if (Fd > 0.3) {
        /* bei grosser Abweichung (Grobfehler) Parameter manipulieren  */
        Fd = 10000;
      } /* -> fuehrt dazu, dass die Schaetzung als nicht sicher eingestuft wird */
    }
    GMP = MP;

    /* Klassifikation von Anregungszustand und Sicherheit der F0-Schaetzung */
    ABA = -1.0 * ndgrms.rms + 0.35 * ndgrms.ndg + 2.0 * Fd; /* Wichtung der Merkmale */
    /* if (ndgrms.ndg>0.2) ABA=-1; */

    /* Klassifikation des Anregungszustandes (Hard Limiter) */
    if (ABA < 0) {
      vuv[c] = 1;
    } /* Voiced */
    else {
      vuv[c] = 0;
    } /* Unvoiced */

    /* Alle Abschnitte ber�cksichtigen */
    /* ABA = -1.0 * ndgrms.rms + 0.35 * ndgrms.ndg + 2.0 * Fd; */
    /* if (ndgrms.ndg>0.2) ABA=-1; */

    /* Sicherheit der Schaetzung */
    if ((ABA < 0.0) && (ndgrms.rms > 0.001)) {
      vu2[c] = 1;
    } else {
      vu2[c] = 0;
    }

    /* Listenkorrektur der Anregung */
    if (c > 1) {
      vuv_o = vuv[c - 1];
      vuv[c - 1] = (vuv[c] && (vuv[c - 1] || vuv[c - 2])) || (vuv[c - 1] && vuv[c - 2]);
      vu2[c - 1] = (vu2[c] && (vu2[c - 1] || vu2[c - 2])) || (vu2[c - 1] && vu2[c - 2]);

      if ((vuv_o == 0) && (vuv[c - 1] == 1)) /* Bereich wurde auf stimmhaft korrigiert */
        if ((ABS((F0[c-1]-F0[c])) / F0[c]) > 0.1) {
          F0[c - 1] = F0[c];
        } /* Dieser Fall ist sehr, sehr selten */

      /* pruefen, ob F0-Schaetzung ausserhalb F0min <= F0 <= F0max */
      if ((F0[c - 1] < Band[1]) || (F0[c - 1] > Band[2])) {
        vuv[c - 1] = 0; /* unvoiced */
        vu2[c - 1] = 0; /* unsichere Schaetzung */
      }

      /* Sicherstellen, dass F0min <= F0 <= F0max */
      MINMAX(F0[c - 1], Band[1], Band[2]);
    }

  } /* Ende Schleife Fensterweise F0 schaetzen */

  FLOAT64 LwM = Lwr * M; /* Veraenderte Abtastrate zurueck rechnen */

  F0Anzahl = c; /* Anzahl der F0-Schaetzungen merken */

  /* Segmente aus sicher geschaetzten F0-Werten bilden */

  /* erstes Segment wird nicht beruecksichtigt (Platzhalter) */
  voicedF0segment[0].left = 1;
  voicedF0segment[0].right = 1;
  c2 = 1;
  for (l = 1; l < F0Anzahl; l++) { /* fuer alle F0-Werte */

    /* Vorgaenger ist unsicher, aber dieser Wert ist sicher */
    if ((vu2[l - 1] == 0) && (vu2[l] == 1)) {
      voicedF0segment[c2].left = (INT32) floor(incarg * (l) * LwM) + 1; /* Segmentbegin */
    }

    /* Vorgaenger ist sicher, aber dieser Wert ist unsicher */
    if ((vu2[l - 1] == 1) && (vu2[l] == 0)) {
      voicedF0segment[c2].right = (INT32) ceil(incarg * (l) * LwM); /* Segmentende */
      c2++;
      if (c2 >= VOICEDF0SEGMENT_MAXLEN - 1) {
        return ERR_MDIM;
      }
    }
  }

  INT32 Num_voicedF0segment = c2; /* Segmentanzahl merken */

  /* letztes Segment wird nicht beruecksichtigt (Platzhalter) */
  voicedF0segment[Num_voicedF0segment].left = Lx;
  voicedF0segment[Num_voicedF0segment].right = Lx;

  /* Speicher freigeben */
  if (F_ptr != NULL) {
    dlp_free(F_ptr);
    F_ptr = NULL;
  }
  if (Fw_ptr != NULL) {
    dlp_free(Fw_ptr);
    Fw_ptr = NULL;
  }
  if (win_ptr != NULL) {
    dlp_free(win_ptr);
    win_ptr = NULL;
  }
  if (r_ptr != NULL) {
    dlp_free(r_ptr);
    r_ptr = NULL;
  }
  if (k_ptr != NULL) {
    dlp_free(k_ptr);
    k_ptr = NULL;
  }
  if (a_ptr != NULL) {
    dlp_free(a_ptr);
    a_ptr = NULL;
  }
  if (tabs != NULL) {
    dlp_free(tabs);
    tabs = NULL;
  }
  if (MD_ptr != NULL) {
    dlp_free(MD_ptr);
    MD_ptr = NULL;
  }
  if (re_ptr != NULL) {
    dlp_free(re_ptr);
    re_ptr = NULL;
  }
  if (im_ptr != NULL) {
    dlp_free(im_ptr);
    im_ptr = NULL;
  }
  if (xr != NULL) {
    dlp_free(xr);
    xr = NULL;
  }
  for (l = scales[0]; l <= scales[1]; l++) {
    if (P[l] != NULL) {
      dlp_free(P[l]);
      P[l] = NULL;
    }
  }

  /*----------------------------------------------------------------
   ----------------------------------------------------------------
   2.STUFE: Pitch-Tracker
   ----------------------------------------------------------------
   ----------------------------------------------------------------*/
  /*----------------------------------------------------------------
   Prinzip:

   -- Vorverarbeitung --
   Werte von F0min und F0max an Grundfrequ.-Schaetzungen der 1. Stufe anpassen
   Scales dimensionieren: Nmin = ld(fs/F0max); Nmax = ld(fs / F0min)
   bei Fs=16kHZ: z.B.: Nmin=5, Nmax=8; 32kHz: Nmin=6; Nmax=9

   Eingangssignal akkumulieren
   akkumuliertes Signal Hochpass-filtern: f_g = 40Hz
   DyWT des akkumulierten Signals

   -- Extraktion --
   DyWT-Koeffizienten summieren und invertieren
   lokale Maxima bestimmen

   -- Nachverarbeitung --
   fuer jedes voiced F0-Segment aus Stufe1
   Grenzen des F0-Segments nochmal pruefen
   groesstes lokales Maxima im (Sub)Segment suchen = Th
   Start bei groesstem Maximum der WT-Koeff gegen die Zeit,
   anschliessend von hier mit der Zeit
   Perioden markieren bis eine Abbruchbedingung gueltig wird
   Position des max. WT-Koeff in Fensteridx aus 1. Stufe umrechnen
   Laenge der zu erwartenden Grundperiode aus F0-Schaetzung ermitteln
   erwartete Periodenlaenge in richtigen Abtastwerte-Index umrechnen
   Pruefen, ob es im erwarteten PM-Suchbereich relevante WT-Maxima gibt

   wenn ja:
   Aufgrund der erwartete Periodenlaenge naechstes WT-Maxima ausw�hlen
   dlm_gcida_NDG_RMS im Bereich zwischen neuer und vorheriger PM berechnen
   Anregungsart korrigieren
   ----------------------------------------------------------------*/

  /* -- Vorverarbeitung -- */

  FLOAT64 f_0_min = Band[2]; /* kleinster u. groesster F0-Wert */
  FLOAT64 f_0_max = Band[1];

  /* F0 enthaelt Grundfrequ.-Schaetzungen der 1. Stufe */
  /* Werte von F0min und F0max an Grundfrequ.-Schaetzungen der 1. Stufe anpassen */
  for (l = 0; l < F0Anzahl; l++) {
    if ((f_0_min > F0[l]) && vuv[l]) {
      f_0_min = F0[l];
    }
    /* kleinste zuverlaessig bestimmte Frequenz im Segment */

    if ((f_0_max < F0[l]) && vuv[l]) {
      f_0_max = F0[l];
    }
    /* groesste zuverlaessig bestimmte Frequenz im Segment */
  }

  /* f_0_min u. f_0_max wieder in den Bereich Band[1] <= f_0_min/f_0_max <= Band[2] bringen */
  MINMAX(f_0_min, Band[1], Band[2]);
  MINMAX(f_0_max, Band[1], Band[2]);

  /* Scales dimensionieren: */
  /* Nmin = ld(fs/F0max);
   Nmax = ld(fs / F0min) */
  INT16 N_min = (INT16) floor(log((FLOAT64) fs / f_0_max) / log(2.0f)) - 1;
  INT16 N_max = (INT16) floor(log((FLOAT64) fs / f_0_min) / log(2.0f)) - 1;

  /* sicherstellen, dass Nmax > Nmin */
  N_max = MAX(N_max, N_min);

  /* sicherstellen, dass Nmin >= 1 */
  N_min = MAX(N_min, 1); /* 0-ten scale ausschliessen */
  scales[0] = N_min;
  scales[1] = N_max;

  /* Speicher fuer WT-Koeffizienten reserieren */
  for (l = scales[0]; l <= scales[1]; l++) {
    P[l] = (FLOAT64*) dlp_calloc(Lx, sizeof(FLOAT64));
  }

  /* akkumulieren */
  x_a = (FLOAT64*) dlp_calloc(Lx, sizeof(FLOAT64)); /* Speicher fuer Akkumulation reservieren */
  dlp_memmove(x_a, x, Lx * sizeof(FLOAT64)); /* Signal in reservierten Speicher kopieren */
  dlm_filter_iir(acc_filt, 2, x_a, x_a, Lx, NULL, 0);

  /* akkumuliertes Signal Hochpass-filtern: f_g = 40Hz */
  dlm_gcida_LinearFilter(x_a, Lx, 40.0f / (FLOAT64) fs * 2.0f, HIGH_PASS, Lw);
  dlm_gcida_MyMallat(x_a, P, scales, Lx); /* DyWT des Signals */

  /* -- Extraktion -- */

  x_sum = (FLOAT64*) dlp_calloc(Lx, sizeof(FLOAT64)); /* Speicher fuer Summe der WT-Koeffizienten reservieren */
  WTmaxima = (FLOAT64*) dlp_calloc(Lx, sizeof(FLOAT64)); /* Speicher fuer lok. Maxima der Koeffizienten reservieren */

  /* Koeffizienten summieren und invertieren */
  for (l = scales[0]; l <= scales[1]; l++) {
    for (l1 = 0; l1 < Lx; l1++) {
      x_sum[l1] -= P[l][l1];
    }
  }

  /* lokale Maxima bestimmen */
  for (l = 1; l < Lx - 1; l++) {
    if ((x_sum[l - 1] < x_sum[l]) && (x_sum[l + 1] <= x_sum[l])) {
      WTmaxima[l] = x_sum[l];
    }
  }

  /* Speicher der WT-Koeffizienten wieder freigeben */
  for (l = scales[0]; l <= scales[1]; l++) {
    if (P[l]) {
      dlp_free(P[l]);
      P[l] = NULL;
    }
  }

  /* -- Nachverarbeitung -- */

  PMSuchbereich = (FLOAT64*) dlp_calloc((INT32 )(2 * T0_max), sizeof(FLOAT64)); /* Speicher fuer Hypothesen-Marker reservieren */
  PM = (FLOAT64*) dlp_calloc(Lx, sizeof(FLOAT64));
  PM[0] = -1; /* Speicher fuer PM-Marker reservieren */
  ZeroArray = (FLOAT64*) dlp_calloc(2 * T0_min, sizeof(FLOAT64)); /* Feld mit Nullen generieren */

  /* Initialisierungen */
  INT32 b = 0, P_i = 0, P_i_o = 0, P_i_oo = 0, P_i_est = 0, FirstMarker = 0, Pkmin = 0, Pkmax = 0;
  INT32 LoopVon = 0, LoopBis = 0;
  INT32 SubStart = 0;
  INT32 SubStart_o[SUBSTARTLEN];
  INT32 Sub_o = 0, Sub = 0;

  FLOAT64 Th = 0, Th_o = 0, V_i = 0;
  FLOAT64 T0 = 0, T0_o = 0, T0_oo = 0, sigma = 0/* , x_E=0 */;
  BOOL EndReached, SubSegment = 0;
  FLOAT64 EstTh = 0, EstTh_o = 0;
  INT32 StrohCounter = STROHCOUNT;

  l1 = 0;
  SubSegment = 0;
  while (l1 < Num_voicedF0segment) { /* jedes sth F0-Segment aus Stufe1 bearbeiten */
    if (!SubSegment) {
      /* Grenzen des F0-Segments nochmal pruefen */
      /* ?? falls die Segmentgrenze noch nicht erreicht wurde, auf weiter
       stimmhafte Anteile pr�fen ?? */
      l1++;
      for (Sub = 0; Sub < SUBSTARTLEN; Sub++) {
        SubStart_o[Sub] = 0;
      }
      LoopVon = (INT32) voicedF0segment[l1].left;
      LoopBis = MAX((INT32 )voicedF0segment[l1].right, (LoopVon + 2 * T0_max));
      MINMAX(LoopBis, 0, (Lx - 1));
    } else {
      SubSegment = 0;
      LoopVon = SubStart;
      LoopBis = MAX((INT32 )voicedF0segment[l1].right, (LoopVon + 2 * T0_max));
      MINMAX(LoopBis, 0, (Lx - 1));
    }

    /* das groesste lokale Maxima im (Sub)Segment suchen */
    Th = 0;
    EstTh = 0;
    EstTh_o = 0;
    for (l = LoopVon; l <= LoopBis; l++) {
      EstTh = (BOOL) (vu2[(INT32) round((FLOAT64) l / ((FLOAT64) LwM * incarg))] > vuvTH);
      if (((Th < WTmaxima[l]) || ((EstTh_o < WTmaxima[l]) && EstTh)) && !(EstTh == 0 && EstTh_o != 0)) {
        Th = WTmaxima[l]; /* Max. Wert */
        b = l; /* Pos. von Max. Wert */
        if (EstTh) {
          EstTh_o = Th;
        }
      }
    }
    if (Th == 0) { /* kein Marker im Segment gefunden */
      continue;
    }

    /* Start bei groesstem Maximum der WT-Koeff gegen die Zeit,
     anschliessend von dort mit der Zeit */
    INT16 Direction = 0; /* Direction: 0 = gegen die Zeit, 1 = mit der Zeit */
    while (Direction <= 1) {
      if (Direction > 0) {
        if (FirstMarker > 0) {
          P_i = FirstMarker;
        } else {
          P_i = b;
        }
        P_i_o = P_i;
        Th_o = 0;
        T0 = 0;
        T0_o = 0;
        T0_oo = 0;
      } else { /* ACHTUNG: Dieser Teil wird zuerst durchlaufen! */
        P_i = b; /* Start bei groesstem Maximum der WT-Koeff */
        FirstMarker = 0;
        P_i_o = P_i;
        Th_o = 0;
        T0 = 0;
        T0_o = 0;
        T0_oo = 0;
      }

      /* Perioden markieren bis eine Abbruchbedingung gueltig wird */
      EndReached = 0;
      while (!EndReached) {
        /* Position des max. WT-Koeff in Fensteridx aus 1. Stufe umrechnen */
        /* Pi_est = (Pi +/-1 * T0) / (LwM * incarg) */
        P_i_est = (INT32) floor(((FLOAT64) P_i + ((FLOAT64)Direction-0.5) * (FLOAT64) T0) / ((FLOAT64) LwM * incarg) + 0.5);
        MINMAX(P_i_est, 0, F0Anzahl);

        /* Laenge der zu erwartenden Grundperiode aus F0-Schaetzung ermitteln */
        if ((vuv[P_i_est] > vuvTH) || (T0 == 0)) { /* Wenn die Grundfrequenz sicher geschaetzt wurde, */
          T0_o = T0; /* wird die zu erwartende zu Grundperiode durch sie definiert */
          T0 = (FLOAT64) fs / F0[P_i_est];
          if ((T0_o) && (T0_o > 1.4 * T0) && (--StrohCounter > 0)) { /* Sonderfall Strohbass oder aehnl. */
            sigma = 0.4; /* greossere Standardabweichung */
            T0 = T0_o * 1.2;
          } else {
            StrohCounter = STROHCOUNT; /* Counter zuruecksetzen */
            sigma = 0.2; /* normale Standardabweichung */
          }
        } else {
          T0_o = T0;
          sigma = 0.4;
        }
        MINMAX(T0, T0_min, T0_max);

        /* erwartete Periodenlaenge in richtigen Abtastwerte-Index umrechnen */
        if (Direction) {
          Pkmin = (INT32) floor(P_i + 1);
          Pkmax = (INT32) ceil(P_i + 1.5 * T0);
        } else {
          Pkmin = (INT32) floor(P_i - 1.5 * T0);
          Pkmax = (INT32) ceil(P_i - 1);
        }
        /* Einschraenkung auf gueltigen Bereich */
        MINMAX(Pkmin, 1, Lx);
        MINMAX(Pkmax, 1, Lx);

        /* Pruefen, ob es im PM-Suchbereich relevante WT-Maxima gibt */
        INT32 L_PMSuchbereich = Pkmax - Pkmin;
        BOOL SetFlag = 0;
        dlp_memmove(PMSuchbereich, &WTmaxima[Pkmin], L_PMSuchbereich * sizeof(FLOAT64));
        for (l = 0; l < L_PMSuchbereich; l++) {
          if ((PMSuchbereich[l] < 0.2 * Th) || (PMSuchbereich[l] < 0.005)) {
            PMSuchbereich[l] = 0;
          } else {
            SetFlag = 1;
          }
        }
        if (!SetFlag) { /* Es wurde keine relevantes Maxima gefunden */
          if (((P_i < voicedF0segment[l1].left) && !Direction) || ((P_i > voicedF0segment[l1].right) && Direction)) {
            EndReached = 1;
          } else {
            if (Direction) {
              Sub_o = 0;
              if (SubStart > 0) {
                for (Sub = 0; Sub < SUBSTARTLEN; Sub++) {
                  Sub_o += (SubStart == SubStart_o[Sub]);
                }
              }
              if (Sub_o == 0) {
                for (Sub = SUBSTARTLEN - 2; Sub >= 0; Sub--) {
                  SubStart_o[Sub + 1] = SubStart_o[Sub];
                }
                SubStart_o[0] = SubStart;
                SubSegment = TRUE;
                SubStart = P_i + (INT32) round(T0);
              }
              EndReached = 1;
            } else {
              /* if (!Direction) { */
              P_i = (INT32) round(P_i - T0);
              MINMAX(P_i, 0, Lx - 1);
            }
            continue;
          }
        }

        /* es gibt relevante WT-Maxima im erwarteten PM-Suchbereich: */
        Th_o = Th;
        if (!EndReached) {

          /* Aufgrund der erwartete Periodenlaenge naechstes WT-Maxima ausw�hlen */
          /* dlm_gcida_NDG_RMS im Bereich zwischen neuer und vorheriger PM berechnen */
          P_i_oo = P_i_o;
          P_i_o = P_i;
          if (Direction) {
            P_i = dlm_gcida_MLNeighbor(PMSuchbereich, L_PMSuchbereich, 0, T0, T0 * sigma, 0, &V_i);
            Th = PMSuchbereich[P_i]; /* Achtung ohne bedingte Wahrscheinlichkeit! */
            P_i = P_i + Pkmin;
            ndgrms = dlm_gcida_NDG_RMS(&x[P_i_o], P_i - P_i_o);
          } else {
            P_i = dlm_gcida_MLNeighbor(PMSuchbereich, L_PMSuchbereich, (INT32) round(L_PMSuchbereich - T0), 0, T0 * sigma, 0, &V_i);
            Th = PMSuchbereich[P_i]; /* Achtung ohne bedingte Wahrscheinlichkeit! */
            P_i = P_i + Pkmin;
            ndgrms = dlm_gcida_NDG_RMS(&x[P_i], P_i_o - P_i);
          }
          MINMAX(P_i, 1, Lx - 1);
          T0 = abs(P_i_o - P_i); /* Periodenl�nge aktualisieren */

          /* Anregungsart korrigieren: */
          /* Kontinuit�tskriterium an KNN anpassen */
          if (T0_o > 0) {
            Fd = fabs(T0 - T0_o) / T0_o;
            if (Fd < 0.09) {
              Fd = -1 / (1.0 + Fd + 0.0001);
            } else {
              if (T0_oo > 0) {
                FLOAT64 To_tmp = fabs(T0_o - T0_oo) / T0_oo;
                if ((fabs(T0 - T0_o) / T0_o > To_tmp) && (To_tmp < 0.4)) {
                  Fd = 0.0;
                }
              }
            }
          } else {
            Fd = 0.0;
          }

          /* "KNN" */
          ABA = -1.0 * ndgrms.rms + 0.35 * ndgrms.ndg + 1.0 * Fd;
          if (ABA < 0) {
            ABA = 1;
          } /* voiced */
          else {
            ABA = 0;
          } /* unvoiced */
          if (Fd > 0.5)
            ABA = 0;
          if (Fd > 1.0)
            ABA = 0;

          vuv_idx = (INT32) round((FLOAT64) P_i / ((FLOAT64) LwM * incarg));
          if (vuv_idx >= LschaetzF0 || (((T0 < T0_min) || (T0 > T0_max) || (Th < 0.03 * Th_o) || (Th < 0.005) /* -45dB */
          || !ABA || (P_i > voicedF0segment[l1 + 1].right) || (P_i < voicedF0segment[l1 - 1].left)) && (vuv[vuv_idx] < vuvTH))) {
            Sub_o = 0;
            if (SubStart > 0) {
              for (Sub = 0; Sub < SUBSTARTLEN; Sub++) {
                Sub_o += (SubStart == SubStart_o[Sub]);
              }
            }
            if ((Direction) && (Sub_o == 0) && (P_i < voicedF0segment[l1].right)) { /* Untersegment einfuegen */
              SubSegment = TRUE;
              for (Sub = SUBSTARTLEN - 2; Sub >= 0; Sub--) {
                SubStart_o[Sub + 1] = SubStart_o[Sub];
              }
              SubStart_o[0] = SubStart;
              SubStart = P_i + (INT32) round(T0);
            }
            EndReached = 1;
            P_i = P_i_o;
            T0 = T0_o;
          } else {
            /* P_i<T0_min ? 0 : P_i-T0_min; */
            INT32 nDiff = 0;
            if (P_i > T0_min)
              nDiff = P_i - T0_min;
            dlp_memmove(&PM[nDiff], ZeroArray, 2 * T0_min * sizeof(FLOAT64));
            /*dlp_memmove(&PM[P_i-T0_min], ZeroArray, 2*T0_min*sizeof(FLOAT64));*/

            FLOAT64 P_i_e = 0.5 * (x_sum[P_i - 1] - x_sum[P_i + 1]) / (x_sum[P_i - 1] - 2 * x_sum[P_i] + x_sum[P_i + 1]) + P_i;
            P_i = (INT32) round(P_i_e);
            PM[P_i] = 1.0f + P_i_e - (FLOAT64) P_i;

            T0_oo = T0_o;
            T0_o = T0;
          }
          if ((!FirstMarker) && (!Direction)) {
            FirstMarker = P_i;
          }
        }
        if (EndReached) {
          INT32 PMtmp = 0;
          if (Direction) {
            PMtmp = P_i + 2;
          } else {
            PMtmp = P_i - 2;
          }
          MINMAX(PMtmp, 0, Lx - 1);
          PM[PMtmp] = -1;
        }
      } /* while ! Endreached */
      Direction++; /* Richtung wechseln */
    }
  } /* Ende Schleife f�r alls voiced F0-Segments */

  if (WTmaxima != NULL) {
    dlp_free(WTmaxima);
    WTmaxima = NULL;
  }
  if (PMSuchbereich != NULL) {
    dlp_free(PMSuchbereich);
    PMSuchbereich = NULL;
  }
  if (ZeroArray)
    dlp_free(ZeroArray);

  /*---------------------------
   PM-PostProcessor
   ---------------------------*/
  FLOAT64 *K = NULL;
  K = (FLOAT64*) dlp_calloc(10, sizeof(FLOAT64));
  FLOAT64 *TT = NULL;
  TT = (FLOAT64*) dlp_calloc(10, sizeof(FLOAT64));
  INT32 I[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  P_i_oo = 0;
  P_i_o = 0;
  /* INT32 counter=0; */

  /* Marker-Korrektur stimmlos */
  dlp_memset(K, 0, 10 * sizeof(FLOAT64));
  dlp_memset(TT, 0, 10 * sizeof(FLOAT64));
  dlp_memset(I, 0, 10 * sizeof(INT32));
  for (l = 0; l < Lx; l++) {
    if (PM[l] != 0) {
      K[0] = K[1];
      I[0] = I[1];
      K[1] = K[2];
      I[1] = I[2];
      K[2] = PM[l];
      I[2] = l;
      if (K[2] > 0 && K[1] < 0 && K[0] > 0) {
        PM[(I[1])] = 0;
      }
      if (K[2] < 0 && K[1] > 0 && K[0] < 0) {
        PM[(I[1])] = -ABS((PM[(I[1])]));
      }
      /* einzelne stimmhaft-Perioden immer l�schen */
    }
  }

  /* Anf�nge und Enden von Segmenten auf Rauschen untersuchen */
  dlp_memset(K, 0, 10 * sizeof(FLOAT64));
  dlp_memset(I, 0, 10 * sizeof(INT32));
  INT32 bis = 0;
  for (l = 0; l < Lx; l++) {
    if (PM[l] != 0) {
      if (PM[l] > 0) {
        /* suche Wechsel stimmlos-stimmhaft */
        /* Marken vorw�rts pr�fen */
        EndReached = 0;
        do {
          I[0] = I[1];
          I[1] = l;
          if (PM[(I[0])] < 0) {
            INT32 bis = T0_max;
            if ((l + bis) >= Lx - 1) {
              bis = Lx - (l + bis) - 1;
            }
            for (l1 = 1; l1 < bis; l1++) {
              if (PM[(l1 + l)] > 0)
                break;
            }
            if (I[1] - l1 < 0) {
              l1 = I[1];
            }
            ndgrms = dlm_gcida_NDG_RMS(&x[(I[1] - l1)], l1);
          } else {
            ndgrms = dlm_gcida_NDG_RMS(&x[(I[0])], (I[1] - I[0]));
          }

          if (ndgrms.ndg > 0.1) {
            PM[I[0]] = -ABS(PM[I[0]]);
          } else {
            EndReached = 1;
          }
          while ((l < Lx-1) && (PM[l] == 0)) {
            l++;
          }
          if ((l >= Lx-1) || (PM[l] < 0)) {
            EndReached = 1;
          }
        } while (EndReached == 0);
        PM[MAX(I[0] - 1, 0)] = -1;
        while ((l < Lx - 1) && (PM[l] >= 0)) {
          l++;
        } /* Gehe zu Ende */
        EndReached = 0;
        I[1] = l; /* Erste stimmlos Marke */
        I[9] = l;
        while ((l > 0) && (PM[l] <= 0)) {
          l--;
        } /* Erste stimmhaft Marke */
        /* Marken r�ckw�rts pr�fen */
        do {
          I[0] = I[1];
          I[1] = l;
          if (PM[(I[0])] <= 0 || ((I[0] - I[1]) < T0_min)) {
            bis = T0_max;
            if ((l - bis) < 0) {
              bis = l;
            }
            for (l1 = 1; l1 < bis; l1++) {
              if (PM[(l - l1)] > 0) {
                l--;
                break;
              }
            }

            ndgrms = dlm_gcida_NDG_RMS(&x[(I[1])], l1);
          } else {
            ndgrms = dlm_gcida_NDG_RMS(&x[(I[1])], (I[0] - I[1]));
          }
          if (ndgrms.ndg > 0.1) {
            PM[(I[0])] = -ABS(PM[(I[0])]);
          } else {
            EndReached = 1;
          }
          while ((PM[--l] == 0) && (l > 1)) {
            ;
          }
          if ((PM[l] < 0) || (l < 1)) {
            EndReached = 1;
          }
        } while (EndReached == 0);

        if ((I[0] + 10) < Lx) {
          PM[I[0] + 1] = -1;
        } else {
          PM[Lx - 10] = -1;
        }
        l = I[9];
      } else {
        I[1] = l;
      }
    }
  }

  /* Marker-Korrektur Stimmlos einf�gen */
  dlp_memset(K, 0, 10 * sizeof(FLOAT64));
  dlp_memset(I, 0, 10 * sizeof(INT32));
  for (l = 0; l < Lx; l++) {
    if (PM[l] != 0) {
      K[0] = K[1];
      I[0] = I[1];
      K[1] = K[2];
      I[1] = I[2];
      K[2] = PM[l];
      I[2] = l;
      if (K[2] > 0 && K[1] > 0 && ((I[2] - I[1]) > T0_max)) {
        PM[(I[1] + 2)] = -4;
        PM[(I[2] - 2)] = -4;
      }
    }
  }

  /* kurze stimmlose Perioden l�schen, wenn umliegende Perioden kontinuierlich sind */
  dlp_memset(K, 0, 10 * sizeof(FLOAT64));
  dlp_memset(I, 0, 10 * sizeof(INT32));
  for (l = 0; l < Lx; l++) {
    if (PM[l] != 0) {
      for (l1 = 0; l1 < 9; l1++) {
        K[l1] = K[l1 + 1];
        I[l1] = I[l1 + 1];
      }
      K[5] = PM[l];
      I[5] = l;
      if (K[2] < 0 && K[3] < 0 && ((I[4] - I[1]) < T0_max)) {
        if (K[0] > 0 && K[1] > 0) {
          TT[1] = I[1] - I[0];
        } else {
          continue;
        }
        if (K[1] > 0 && K[4] > 0) {
          TT[2] = I[4] - I[1];
        } else {
          continue;
        }
        if (K[5] > 0 && K[4] > 0) {
          TT[5] = I[5] - I[4];
        } else {
          continue;
        }
        if ((((FLOAT64) ABS(TT[1] - TT[5]) / (FLOAT64) TT[5]) < 0.3) && (((FLOAT64) ABS(TT[2] - TT[5]) / (FLOAT64) TT[5]) < 0.4)) {
          PM[(I[2])] = 0;
          PM[(I[3])] = 0;
        }
      } else if (K[2] < 0 && K[0] > 0 && K[1] > 0 && K[3] > 0 && K[4] > 0) {
        TT[1] = I[1] - I[0];
        TT[2] = I[2] - I[1];
        TT[3] = I[3] - I[2];
        TT[4] = I[4] - I[3];
        TT[5] = I[5] - I[4];
      }
    }
  }

  /* R�nder auf Kontinuit�t pr�fen */
  dlp_memset(K, 0, 10 * sizeof(FLOAT64));
  dlp_memset(I, 0, 10 * sizeof(INT32));
  P_i_oo = 0;
  P_i_o = 0;
  for (l = 0; l < Lx; l++) {
    if (PM[l] > 0 && P_i_o > 0) {
      I[P_i_oo++] = l;
      if (P_i_oo > 3) {
        T0_oo = I[3] - I[2];
        T0_o = I[2] - I[1];
        T0 = I[1] - I[0];
        if ((ABS((T0-T0_o)) / T0_o) > 0.2) {
          PM[(I[0])] = 0;
        } /* Erster Marker weicht zu stark ab */
        if ((ABS((T0_o-T0_oo)) / T0_oo) > 0.2) {
          PM[(I[1])] = 0;
          PM[(I[0])] = 0;
        }
        P_i_o = 0;
      }
    } else if (PM[l] < 0) {
      P_i_oo = 0;
      P_i_o = l;
    }
  }

#if defined(STUFE3)
  /*------------------------------------------------------------
   --------------------------------------------------------------
   3.STUFE: DP
   --------------------------------------------------------------
   ------------------------------------------------------------*/

  GCIDA_SEG voicedF0segment2[VOICEDF0SEGMENT_MAXLEN];

  FLOAT64 *PP = NULL;
  PP = (FLOAT64*) dlp_calloc(Lx, sizeof(FLOAT64));
  FLOAT64 *PM1 = NULL;
  PM1 = (FLOAT64*) dlp_calloc(Lx, sizeof(FLOAT64));
  FLOAT64 P_e = 0;

  l1 = -1;
  l = 0;
  P_i_o = 0;
  while (l < Lx - 1) {
    l++;
    if (PM[l] > 0) { /* finde aktuelle Segmentgrenzen */
      ++l1;
      voicedF0segment2[l1].left = l;
      while ((l < Lx - 1) && (PM[l] >= 0)) {
        if (PM[l] > 0) {
          P_i_o = l;
        }
        l++;
      }
      voicedF0segment2[l1].right = P_i_o;
    }
  }
  INT32 Num_voicedF0segment2 = l1;

  INT32 sc0 = (INT32) floor(log((FLOAT64) fs / (FLOAT64) 2000.0f) / log(2.0f)); /* scales[0]; */

  scales[0] = (INT16) sc0;
  scales[1] = (INT16) sc0;

  for (l = scales[0]; l <= scales[1]; l++) {
    P[l] = (FLOAT64*) dlp_calloc(Lx, sizeof(FLOAT64));
  }
  for (l = 0; l < Lx; l++) {
    x_a[l] = -x_a[l]; /* waechsle Vorzeichen von x_a */
  }
  dlm_gcida_MyMallat(x_a, P, scales, Lx); /* Berechne DyWT */

  dlp_memmove(PP, P[sc0], Lx * sizeof(FLOAT64));

  for (l1 = scales[0]; l1 <= scales[1]; l1++) { /* loesche alle nicht Maxima */
    for (l = 1; l < Lx - 1; l++) {
      if (!((P[l1][l - 1] < P[l1][l]) && (P[l1][l] >= P[l1][l + 1]))) {
        P[l1][l] = 0;
      }
    }
  }

  l1 = 0;

  while (l1 <= Num_voicedF0segment2) {
    b = voicedF0segment2[l1].left;
    Th = P[sc0][b];

    FLOAT64 Drift = 0;
    FLOAT64 Drift_o = 0;
    FLOAT64 Drift_m = 0;
    FLOAT64 Th1 = 0;
    FLOAT64 P_i_e = 0;
    FLOAT64 V_i = 0;
    FLOAT64 P_pos = 0;
    INT32 von = 0, bis = 0;
    INT32 nbh = (INT32) floor((FLOAT64) fs / (FLOAT64) Band[2] * 0.2);

    for (l = voicedF0segment2[l1].left; l <= voicedF0segment2[l1].right; l++) {
      if (Th1 < P[sc0][l]) {
        Th1 = P[sc0][l];
        b = l;
      }
    }
    /* Drift Initialisieren */
    von = b - T0_max;
    MINMAX(von, 0, Lx - 1);
    bis = b + T0_max;
    MINMAX(bis, 0, Lx - 1);
    P_i = dlm_gcida_MLNeighbor(&PM[von], bis - von, b - von, 0, 1, 2, &V_i) + von;

    if (V_i > 0)
      Drift = (INT32) (b - P_i);
    else
      Drift = 0;

    Drift_o = Drift;

    INT16 D = 0;
    while (D <= 1) {
      l = b;
      EndReached = 0;
      if (D == 1) {
        Drift = Drift_o;
      }
      while (!EndReached) {

        if (D == 0)
          while ((l > voicedF0segment2[l1].left) && (PM[l] <= 0)) {
            l--;
          }
        if (D == 1)
          while ((l < voicedF0segment2[l1].right) && (PM[l] <= 0)) {
            l++;
          }

        PM1[l] = 0;
        /* ----------------------------------- */
        P_pos = (FLOAT64) l + (PM[l] - 1.0f);
        P_i_o = (INT32) round(P_pos + Drift);

        von = P_i_o - nbh;
        MINMAX(von, 0, Lx - 1); /* Untersuche nur naechste Umgebung */
        bis = P_i_o + nbh;
        MINMAX(bis, 0, Lx - 1);

        P_i = dlm_gcida_MLNeighbor(&P[sc0][von], bis - von, P_i_o - von, 0, 1, 0, &V_i) + von; /* finde wahrscheinlichstes Maximum */

        P_i_e = 0.5 * (PP[P_i - 1] - PP[P_i + 1]) / (PP[P_i - 1] - 2 * PP[P_i] + PP[P_i + 1]) + P_i; /* Interpolation */
        Th = P[sc0][P_i];

        Drift_m = (P_i_e - P_pos);
        if (ABS((Drift_m-Drift)) > T0_min)
          Drift_m = Drift;

        Drift = 0.80 * Drift + 0.2 * Drift_m; /* Glaettungsfilter */

        P_e = P_pos + Drift;

        P_i = (INT32) round(P_e);
        PM1[MAX(P_i, 0)] = 1.0f + P_e - (FLOAT64) P_i; /* schreibe Marker-Array */
        /* ----------------------------------- */
        if (l >= voicedF0segment2[l1].right) {
          EndReached = 1;
          PM1[P_i + 2] = -1;
        }

        if (l <= voicedF0segment2[l1].left) {
          EndReached = 1;
          PM1[MAX(P_i - 2, 0)] = -1;
        }
        if (D > 0) {
          l++;
        } else {
          l--;
        }
      }
      D++;
    }
    l1++;
  }
  for (l = scales[0]; l <= scales[1]; l++) {
    if (P[l]) {
      dlp_free(P[l]);
    }
  }
  dlp_memmove(PM, PM1, Lx * sizeof(FLOAT64));

  if (PM1 != NULL) {
    dlp_free(PM1);
    PM1 = NULL;
  }
  if (PP != NULL) {
    dlp_free(PP);
    PP = NULL;
  }

#endif
  /*------------------------------------------------------
   Ausgabe Routinen:
   ------------------------------------------------------
   Nullen auff�llen
   */
  dlp_memset(K, 0, 10 * sizeof(FLOAT64));
  dlp_memset(I, 0, 10 * sizeof(INT32));
  l = -1;
  PM[0] = -1;
  P_i_o = 0;
  INT32 ZD = 0;

  if (Para.uvm == 2) { /* mit Konstante */
    ZD = Para.uvmW;
  }

  while (l < Lx - 1) {
    if (PM[++l] < 0) {
      P_i_o = l;
      while ((PM[++l] <= 0) && (l < (Lx - 1))) {
        ;
      }

      T0 = I[1] - I[0];
      MINMAX(T0, T0_min, T0_max);
      if ((Para.uvm == 1) || (Para.uvm == 0)) { /* mit T0 */
        ZD = (INT32) T0;
      }

      if ((l - P_i_o) > ZD) {
        for (l1 = I[1] + 1; l1 < l - 1; l1++) {
          PM[l1] = 0;
        }

        l1 = I[1] + ZD;
        for (; l1 < l;) {
          PM[l1] = -1;
          if (Para.uvm == 0) { /* Zufaellig */
            ZD = (INT32) floor((0.8f + 0.4f * ((FLOAT64) dlp_rand() / (FLOAT64) RAND_MAX)) * T0);
          }
          l1 += ZD;
        }
      }
    } else {
      if (PM[l] > 0) {
        I[0] = I[1];
        I[1] = l;
      }
    }
  }

  l1 = -1;
  l = 0;
  P_i_o = 0;
  FLOAT64 OldVUV = 0;
  while (l < Lx) {

    if (PM[l] != 0) {
      if ((PM[l] > 0) && (OldVUV < 0)) {
        /* erste stimmhafte Periode eines stimmhaften Abschnittes finden */
        OldVUV = PM[l];
        PM[l] = -1;
      } else if ((PM[l] < 0) && (OldVUV > 0)) {
        /* letzte stimmhafte Periode eines stimmhaften Abschnittes finden */
        OldVUV = PM[l];
        PM[P_i_o] = -1;
      } else {
        OldVUV = PM[l];
      }
      P_i_o = l;
    }
    l++;
  }

  /*----------------------------
   free:
   */
  if (vuv != NULL) {
    dlp_free(vuv);
    vuv = NULL;
  }
  if (vu2 != NULL) {
    dlp_free(vu2);
    vu2 = NULL;
  }
  if (F0 != NULL) {
    dlp_free(F0);
    F0 = NULL;
  }

  for (l = 0; l < P_LEN; l++) {
    if (P[l] != NULL) {
      dlp_free(P[l]);
      P[l] = NULL;
    }
  }

  if (K != NULL) {
    dlp_free(K);
    K = NULL;
  }
  if (TT != NULL) {
    dlp_free(TT);
    TT = NULL;
  }
  if (x_sum != NULL) {
    dlp_free(x_sum);
    x_sum = NULL;
  }
  /*---------------------------
   Write: PM-File
   */
  (*nPeriods) = 0;
  INT32 T0_sum = 0;
  /* HEADER */
  UINT16 tmp1 = 2;
  unsigned char tmp2 = 0;

  *periods = NULL;
  *nPeriods = 0;

  P_i_o = 0;
  char D = 0;
  /* DATA */
  for (l = 1; l < Lx; l++) {
    if (PM[l] != 0) {
      if (*periods == NULL) {
        (*periods) = (PERIOD*) dlp_calloc((*nPeriods) + 1, sizeof(PERIOD));
      } else {
        (*periods) = (PERIOD*) dlp_realloc((*periods), (*nPeriods) + 1, sizeof(PERIOD));
      }
      T0 = (FLOAT64) (l - P_i_o);
      P_i_o = l;
      tmp1 = (UINT16) T0;
      T0_sum += tmp1;
      if (Para.swap_out)
        tmp1 = ((tmp1 << 8) & 0xff00) | ((tmp1 >> 8) & 0x00ff);
      ((*periods) + (*nPeriods))->nPer = tmp1;
      if (PM[P_i_o] > 0) {
        tmp2 = 1;
        D = (char) round((PM[P_i_o] - 1.0f) * 256);
        ((*periods) + (*nPeriods))->stimulation = tmp2;
        ((*periods) + (*nPeriods))->dummy = D;
      } else {
        tmp2 = 0;
        D = 0;
        ((*periods) + (*nPeriods))->stimulation = tmp2;
        ((*periods) + (*nPeriods))->dummy = D;
      }
      (*nPeriods)++;
    }
  }
  /* letzte Periode genau bis zum Ende: */
  tmp1 = (UINT16) (Lx - T0_sum) + tmp1;
  if (Para.swap_out)
    tmp1 = ((tmp1 << 8) & 0xff00) | ((tmp1 >> 8) & 0x00ff);
  (*periods) = (PERIOD*) dlp_realloc((*periods), (*nPeriods) + 1, sizeof(PERIOD));
  ((*periods) + (*nPeriods))->nPer = tmp1;
  ((*periods) + (*nPeriods))->stimulation = tmp2;
  ((*periods) + (*nPeriods))->dummy = D;
  (*nPeriods)++;

  if (x_out != NULL) {
    dlp_free(x_out);
    x_out = NULL;
  }
  if (x_a != NULL) {
    dlp_free(x_a);
    x_a = NULL;
  }

  dlp_free(PM);

  return O_K;
}
