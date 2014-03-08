/* dLabPro base library
 * - Header file - defines adapter for 3rd party sources
 *
 * AUTHOR : Guntram Strecha
 * PACKAGE: dLabPro/sdk
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

#ifdef ABS
#undef ABS
#endif

#ifdef MAX
#undef MAX
#endif

#ifdef MIN
#undef MIN
#endif

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

/* Avoid compiler warnings of SPTK */
#ifdef RAND_MAX
#undef RAND_MAX
#endif

/* needed for clapack compiled with mingw */
#ifdef small
#undef small
#endif
#ifdef large
#undef large
#endif
