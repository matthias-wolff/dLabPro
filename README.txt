##### dLabPro external program example ######

This is an example for an external program using dLabPro's classes.

[Setup & Compile]
  To create a new external program you need to do:

  1) Checkout the branch externalprogram in https://github.com/matthias-wolff/dLabPro
     and create a new branch from it or export the files to another Repository.

  2) Edit Makefile:
  2.1) Set DLABPRO_HOME to the relative or absolute path of your dLabPro source tree.
  2.2) Set PROJNAME to your desired program name.
  2.3) Set SOURCES to all cpp-Files you want to add to the program. (Without the .cpp extension!)
       The files should exist in the program's directory.
  2.4) Set LIBS to all dLabPro classes you want to use and all which depend on them.
       Lines may be copied from dLabPro/programs/dlabpro/Makefile.
       The classes should be orderd according to thier dependencies
       Use the same order as in dLabPro/programs/dlabpro/Makefile.
  
  3) Edit main.cpp
  3.1) Add include and REGISTER_CLASS lines for all classes you want to use.
       Lines may be copied from dLabPro/programs/dlabpro/dlabpro.cpp.
  3.2) Edit run() function to do what you want.

  4) Build the program by running "make DEBUG" or "make RELEASE" in the program's directory.
     Or configure your Eclipse to do the same.
     The executable will be placed in bin.debug or bin.release in the program's directory.

  5) If there are "undefined references" consider adding dependend classes to the Makefile.

