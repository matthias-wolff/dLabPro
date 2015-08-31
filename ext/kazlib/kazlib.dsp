# Microsoft Developer Studio Project File - Name="kazlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=kazlib - Win32 Debug_C
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "kazlib.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "kazlib.mak" CFG="kazlib - Win32 Debug_C"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "kazlib - Win32 Debug_C" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "kazlib - Win32 Release_C" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "kazlib - Win32 Debug_C"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug_C"
# PROP BASE Intermediate_Dir "Debug_C"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\lib.Debug_C"
# PROP Intermediate_Dir "..\..\obj.Debug_C"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "../../include/" /I "../../include/automatic" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_DLP_C" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../../include/" /I "../../include/automatic" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_DLP_C" /YX /FD /TC /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "kazlib - Win32 Release_C"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release_C"
# PROP BASE Intermediate_Dir "Release_C"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\lib.Release_C"
# PROP Intermediate_Dir "..\..\obj.Release_C"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_DLP_C" /D "_RELEASE" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../../include/" /I "../../include/automatic" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_DLP_C" /D "_RELEASE" /YX /FD /TC /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "kazlib - Win32 Debug_C"
# Name "kazlib - Win32 Release_C"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\dict.c
# End Source File
# Begin Source File

SOURCE=.\except.c
# End Source File
# Begin Source File

SOURCE=.\except.h
# End Source File
# Begin Source File

SOURCE=.\hash.c
# End Source File
# Begin Source File

SOURCE=.\list.c
# End Source File
# Begin Source File

SOURCE=.\sfx.c
# End Source File
# Begin Source File

SOURCE=.\sfx.h
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\kzl_dict.h
# End Source File
# Begin Source File

SOURCE=..\..\include\kzl_hash.h
# End Source File
# Begin Source File

SOURCE=..\..\include\kzl_list.h
# End Source File
# End Group
# End Target
# End Project
