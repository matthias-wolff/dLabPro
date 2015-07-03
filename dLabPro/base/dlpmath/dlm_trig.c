/* dLabPro mathematics library
 * - Trigonometric functions
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

INT16 cos_tab[257] = {
    -32768, -32767, -32766, -32762, -32758, -32753, -32746, -32738,
    -32729, -32718, -32706, -32693, -32679, -32664, -32647, -32629,
    -32610, -32590, -32568, -32546, -32522, -32496, -32470, -32442,
    -32413, -32383, -32352, -32319, -32286, -32251, -32214, -32177,
    -32138, -32099, -32058, -32015, -31972, -31927, -31881, -31834,
    -31786, -31737, -31686, -31634, -31581, -31527, -31471, -31415,
    -31357, -31298, -31238, -31177, -31114, -31050, -30986, -30920,
    -30853, -30784, -30715, -30644, -30572, -30499, -30425, -30350,
    -30274, -30196, -30118, -30038, -29957, -29875, -29792, -29707,
    -29622, -29535, -29448, -29359, -29269, -29178, -29086, -28993,
    -28899, -28803, -28707, -28610, -28511, -28411, -28311, -28209,
    -28106, -28002, -27897, -27791, -27684, -27576, -27467, -27357,
    -27246, -27133, -27020, -26906, -26791, -26674, -26557, -26439,
    -26320, -26199, -26078, -25956, -25833, -25708, -25583, -25457,
    -25330, -25202, -25073, -24943, -24812, -24680, -24548, -24414,
    -24279, -24144, -24008, -23870, -23732, -23593, -23453, -23312,
    -23170, -23028, -22884, -22740, -22595, -22449, -22302, -22154,
    -22006, -21856, -21706, -21555, -21403, -21251, -21097, -20943,
    -20788, -20632, -20475, -20318, -20160, -20001, -19841, -19681,
    -19520, -19358, -19195, -19032, -18868, -18703, -18538, -18372,
    -18205, -18037, -17869, -17700, -17531, -17361, -17190, -17018,
    -16846, -16673, -16500, -16326, -16151, -15976, -15800, -15624,
    -15447, -15269, -15091, -14912, -14733, -14553, -14373, -14192,
    -14010, -13828, -13646, -13463, -13279, -13095, -12910, -12725,
    -12540, -12354, -12167, -11980, -11793, -11605, -11417, -11228,
    -11039, -10850, -10660, -10469, -10279, -10088,  -9896,  -9704,
     -9512,  -9319,  -9127,  -8933,  -8740,  -8546,  -8351,  -8157,
     -7962,  -7767,  -7571,  -7376,  -7180,  -6983,  -6787,  -6590,
     -6393,  -6195,  -5998,  -5800,  -5602,  -5404,  -5205,  -5007,
     -4808,  -4609,  -4410,  -4211,  -4011,  -3812,  -3612,  -3412,
     -3212,  -3012,  -2811,  -2611,  -2411,  -2210,  -2009,  -1809,
     -1608,  -1407,  -1206,  -1005,   -804,   -603,   -402,   -201, 0 };

INT32 CGEN_IGNORE dlm_cosinus_int(INT32 x) {
  INT32 y = 0;

  x = x & 0x3ff;

  y = (x < 256) ? -cos_tab[x] : (x < 512) ? cos_tab[512-x] : (x < 768) ? cos_tab[x-512] : -cos_tab[1024-x];

  return y;
}

INT32 CGEN_IGNORE dlm_sinus_int(INT32 x) {
  return (dlm_cosinus_int(x - 256));
}

/**
 *  Fast sine implementation - uses table lookup.
 * This function works about 5 times faster than sse2 implementation.
 *
 * @param x Argument of sine
 * @return  Result of sine
 */
FLOAT32 dlm_sinus(FLOAT32 x) {
  return dlm_sinus_int((INT32)(x*162.974661726101+0.5)) / 32768.0;
}

/**
 *  Fast cosine implementation - uses table lookup.
 * This function works about 5 times faster than sse2 implementation.
 *
 * @param x Argument of cosine
 * @return  Result of cosine
 */
FLOAT32 dlm_cosinus(FLOAT32 x) {
  return dlm_cosinus_int((INT32)(x*162.974661726101+0.5)) / 32768.0;
}
