const char *cfgdef = "\
## dLabPro program recognizer (dLabPro recognizer)\n\
## - Main program\n\
##\n\
## AUTHOR : Frank Duckhorn\n\
## PACKAGE: dLabPro/programs\n\
## \n\
## Copyright 2013 dLabPro contributors and others (see COPYRIGHT file) \n\
## - Chair of System Theory and Speech Technology, TU Dresden\n\
## - Chair of Communications Engineering, BTU Cottbus\n\
## \n\
## This file is part of dLabPro.\n\
## \n\
## dLabPro is free software: you can redistribute it and/or modify it under the\n\
## terms of the GNU Lesser General Public License as published by the Free\n\
## Software Foundation, either version 3 of the License, or (at your option)\n\
## any later version.\n\
## \n\
## dLabPro is distributed in the hope that it will be useful, but WITHOUT ANY\n\
## WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS\n\
## FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more\n\
## details.\n\
## \n\
## You should have received a copy of the GNU Lesser General Public License\n\
## along with dLabPro. If not, see <http://www.gnu.org/licenses/>.\n\
# Recognizer config file\n\
#\n\
# This file (recognizer.cfg) contains default values\n\
# for all options and is compiled in the program.\n\
# If you want to change some options you may write\n\
# an own configuration file with the options to change\n\
# and execute \"recognizer -cfg FILE.cfg\". Or you may\n\
# change options by command line arguments.\n\
# For example run: \"recognizer -output dbg\".\n\
# \"recognizer -h\" will give a short help.\n\
\n\
# Print short help.\n\
# Shortcut: -h \n\
\n\
# List all options and exit.\n\
# Shortcut: -opts \n\
\n\
# Output configuration\n\
#\n\
# This option defines what the recognizer outputs.\n\
# Possible values are:\n\
#  res   output only one result per line\n\
#  cmd   recognized commands and summary\n\
#  sta   status information of recognition progress\n\
#  dbg   more detailed information about the progress\n\
#  gui   client mode: output information usable for GUI's (energy + vad state for every frame)\n\
#  vad   information about vad progress for every frame\n\
output = sta\n\
# Shortcut: -out X\n\
# [online-able]\n\
\n\
# Input configuration\n\
#\n\
# Configures whether something is read from stdin.\n\
# Possible values are:\n\
#  none   Nothing is read.\n\
#  cmd    Read commands from stdin. (only in online mode!)\n\
#         Every line is one command.\n\
#         Possible commands are:\n\
#           \"set OPTION VALUE\"\n\
#             Online set the option OPTION to VALUE.\n\
#             Only possible for onlineable options\n\
#             (see tag [online-able] here in config).\n\
#           \"exit\"\n\
#             Quit recognizer\n\
#  fea    Read features from stdin.\n\
#         Every new line marks a new segment for recognition.\n\
#         A line starting with 'l' is interpreted as label of the segment.\n\
#         The features are only modificated by PCA!\n\
input = cmd\n\
# Shortcut: -in X\n\
\n\
# Cache enabled\n\
#\n\
# This option enables the cache for grammar compiling.\n\
cache = yes\n\
# [online-able]\n\
\n\
# Use a system command to analyze the recognized fst\n\
#\n\
# It is executed with the following arguments:\n\
#  - Temporary filename of the recognized fst\n\
#  - Temporary filename of the reference recognized fst\n\
#  - Temporary filename of the nld-matrix\n\
#  - Temporary filename of the audio signal\n\
#  - Temporary filename of the fvr-fst or \"NULL\"\n\
#  - Filename of the session info object\n\
postproc.cmd = \n\
\n\
# Search configuration\n\
#\n\
# Search algorithm\n\
#  tp    Token passing\n\
#  as    A* search \n\
search.typ = tp\n\
\n\
# Debug level for search algorithm\n\
search.debug = 0\n\
\n\
# Should we search while speaking\n\
search.iterative = yes\n\
\n\
# Should we prune the search?\n\
search.prn = no\n\
\n\
# Pruning configuration\n\
#\n\
# Only usefull if search.prn = yes !\n\
# See dokumentation of fstsearch -as2_param for details.\n\
#  tpprnw   sets weight pruning for token passing (see fstsearch.tp_prnw)\n\
#  tpprnh   sets hypothesis pruning for token passing (see fstsearch.tp_prnh)\n\
#  asprn1   sets FramePruneThr for first search\n\
#  asprn2   sets FramePruneThr for reference search\n\
#  as2prn   sets FramePruneThr for two path search\n\
search.tpprnw = 100\n\
search.tpprnh = 0\n\
search.asprn1 = 40\n\
search.asprn2 = 40\n\
search.as2prn = 110\n\
\n\
# Number of threads to use\n\
#\n\
# only used in token passing search (see fstsearch.tp_threads)\n\
search.threads = 1\n\
\n\
# Permanent decoding (without VAD)\n\
#\n\
# Only valid for iterative search.\n\
# Disables VAD!\n\
search.permanent = no\n\
\n\
# Rejection method\n\
#  off   Rejection disabled\n\
#  phn   Free phoneme reference recognition\n\
#  two   Rejection based on two best paths\n\
rej.typ = phn\n\
\n\
# Rejection thresholds\n\
#\n\
#  tad        Default threshold for acoustic distance (NAD)\n\
#  ted        Default threshold for edit distance (NED)\n\
#  tad.as     Threshold for NAD With A* search\n\
#  tad.two    Threshold for NAD With two path A* search rejection\n\
#  ted.two    Threshold for NED With two path A* search rejection\n\
#  ted.fvr    Threshold for NED in FVR confidence\n\
#  ted.lambda Balance bias between NED and NAD for FVR confidence\n\
rej.tad     = 0.05\n\
rej.ted     = 0.75\n\
rej.as.tad  = 0.5\n\
rej.two.tad = 15\n\
rej.two.ted = 0\n\
rej.fvr.ted = 0.5\n\
rej.fvr.lambda = 0.5\n\
\n\
# Force vad decission or selected fst\n\
#\n\
# If you use one of these two options, you have to\n\
# generate for every file \"XX.wav\" a file \"XX.wav.vadforce\".\n\
# The binary file should contain a unsigned Byte for every\n\
# frame. The value of the Byte is interpreted so:\n\
#  0   vad off = no speech signal\n\
#  >0  vad on for vad.force and\n\
#      the number of the fst to use for fst.force\n\
# For fst.force you should have multiple units in\n\
# itRN in the session info object.\n\
fst.force = no\n\
vad.force = no\n\
\n\
# Set selected fst\n\
#\n\
# Sets the current fst unit within itRN to use for decoding.\n\
# May be used to set initial unit when using data.dialog option\n\
# or for implementing an external dialog manager.\n\
fst.sel = 0\n\
# [online-able]\n\
\n\
# Set sleep timeout\n\
#\n\
# Sets the sleep timeout in seconds for fallback\n\
# to sleep vocabulary in dialog. To use this you\n\
# should have at least one transition labeled with\n\
# __SLEEP__ in data.dialog.\n\
fst.sleep = 0\n\
\n\
# VAD configuration\n\
#\n\
# vad.minsp  defines the minimal number of frames to use\n\
#            for recognition.\n\
# vad.maxsp  defines the maximal number of frames to use\n\
#            for recognition.\n\
# vad.sigmin defines the minimal signal peak with a speech\n\
#            segment to use it for recognition.\n\
# If vad.nolimit is set to yes, then the options will be\n\
# changed in the following way:\n\
#   vad.minsp  = 0\n\
#   vad.maxsp  = 6000 # this is one minute\n\
#   vad.sigmin = 0\n\
vad.minsp = 40\n\
vad.maxsp = 600\n\
vad.sigmin = 3267\n\
vad.nolimit = no\n\
\n\
# Noise reduction (simple algorithm!)\n\
noise_reduce = no\n\
\n\
# Noise reduction buffer size\n\
noise_reduce.len = 300\n\
\n\
# Noise reduction buffer use percentage\n\
noise_reduce.prc = 0.3\n\
\n\
# Meassure time of recognition algorithms (analyse, density, search)\n\
measure_time = yes\n\
\n\
# For online recogition define the audio device for\n\
# portaudio. -1 is the default device.\n\
# See \"recognizer -l\" for other device numbers.\n\
audio.dev = -1\n\
# Shortcut: -d X\n\
\n\
# List all audio devices of portaudio and exit.\n\
audio.dev_list = no\n\
# Shortcut: -l \n\
\n\
# Use binary data file\n\
#\n\
# Currently not implemented!\n\
data.bin = \n\
\n\
# Skip NLD calculation and use precalculated ones\n\
#\n\
# Only for binary data file!\n\
skipnld = no\n\
\n\
# Forced recognition\n\
#\n\
# The FST in reco_force_rn.fst in the current directory\n\
# is used for search.\n\
force = no\n\
\n\
# Sample rate\n\
sig.sample_rate = 16000\n\
\n\
# Select signal channel\n\
#\n\
# For multichannel files select that channel for signal extraction.\n\
# (zero-based index)\n\
sig.sel_channel = 0\n\
\n\
# Feature info object\n\
#\n\
# File name of the feature info object generated by\n\
# \"HMM.xtp trn\" in the model directory.\n\
# The object should contain the following fields:\n\
#  idDlt   data    delta table\n\
#  idDltW  data    delta weights\n\
#  idX     data    normalization vector\n\
#  idW     data    PCA matrix\n\
# [online-able]\n\
data.feainfo = $UASR_HOME/data/vm/VM_one2_evo4/model/feainfo.object\n\
\n\
# Session info object\n\
#\n\
# File name of the session info object.\n\
# It can be created with the script uasr/scripts/dlabpro/tools/REC_PACKDATA.xtp\n\
# The object should contain the following fields:\n\
#  itRN    fst     recognition network\n\
#  itRNr   fst     reference recognition network\n\
#  itGP    fst     graphem to phonem transducer\n\
#  idLMtos data    output symbol table of itRN\n\
# If you want to use fst.force you have to modify the\n\
# object so that itRN has multiple units with different\n\
# recognition networks.\n\
# [online-able]\n\
data.sesinfo = $UASR_HOME/data/vm/VM_one2_evo4/log/samurai-4BCD8B62-DB006BB4-3_20_mix_subvoc.si\n\
\n\
# Gaussian mixture model\n\
#\n\
# File name of the Gaussian mixture model in single precision floating point.\n\
# It can be created with the skript uasr/scripts/dlabpro/tools/REC_PACKDATA.xtp\n\
# [online-able]\n\
data.gmm = $UASR_HOME/data/vm/VM_one2_evo4/model/3_20_mix_samurai_0_force_7_0.floatgm\n\
\n\
# VAD info object\n\
#\n\
# File name of the VAD info object.\n\
# The object should contain the following fields:\n\
#  idX     data    normalization vector for VAD (from feainfo.object)\n\
#  idW     data    PCA matrix for VAD (from feainfo.object)\n\
#  itGm    gmm     Gaussian mixture model for VAD (generated from HMM)\n\
#\n\
#      dlabpro vad_create.xtp feainfo.object XXX.floatgm XXX.vad\n\
#\n\
######## vad_create.xtp ###########\n\
## object iFea;\n\
## gmm    iGm;\n\
## object iVAD;\n\
## data   iVAD.idX;\n\
## data   iVAD.idW;\n\
## gmm    iVAD.itGm;\n\
##\n\
## \"$1\" iFea -restore;\n\
## \"$2\"    iGm  -restore;\n\
##\n\
## iFea.idX iVAD.idX  =;\n\
## iFea.idW iVAD.idW  =;\n\
## iGm      iVAD.itGm =;\n\
##\n\
## \"$3\" iVAD /zip -save;\n\
###################################\n\
data.vadinfo = $UASR_HOME/data/vm/VM_vad_10/model/3_10_mod.vad\n\
\n\
# Dialog FSA\n\
#\n\
# File name of the dialog finite state acceptor.\n\
# Each state corresponds to the unit in the recognition\n\
# network itRN. The initial dialog state can be defined\n\
# by fst.sel. If an input symbol in the dialog matches\n\
# an accepted recognition result the dialog switches the\n\
# used vocabulary (= unit in itRN).\n\
data.dialog = \n\
";
