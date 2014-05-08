## External library libsndfile
## - Makefile
##
## AUTHOR : Erik de Castro Lopo (libsndfile), Matthias Wolff (Makefile)
## PACKAGE: dLabPro/ext
##
## LICENSE: libsndfile is under the Gnu Lesser General Public License.

ifeq (${DLABPRO_HOME},)
  DLABPRO_HOME = ../..
endif

PROJNAME=libsndfile

CFLAGS_FILTEROUT += -Wall -ansi

## Target settings
SEXT    = c
SOURCES = aiff     \
          au       \
          command  \
          double64 \
          file_io  \
          float32  \
          pcm      \
          raw      \
          sndfile  \
          strings  \
          wav      \
          common   \
          wav_w64
          
DEPS    = common.h     \
					config.h     \
					float_cast.h \
					sf_unistd.h  \
					sfconfig.h   \
					sfendian.h   \
					wav_w64.h    \
					../../include/sndfile.h

include $(DLABPRO_HOME)/make/ext.mk

## EOF
