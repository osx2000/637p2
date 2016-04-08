# Microsoft Developer Studio Project File - Name="sfront" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=sfront - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sfront.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sfront.mak" CFG="sfront - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sfront - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "sfront - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sfront - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX- /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "sfront - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "sfront - Win32 Release"
# Name "sfront - Win32 Debug"
# Begin Source File

SOURCE=.\ascwrite.c
# End Source File
# Begin Source File

SOURCE=.\asyslib.c
# End Source File
# Begin Source File

SOURCE=.\audio.c
# End Source File
# Begin Source File

SOURCE=.\blocktree.c
# End Source File
# Begin Source File

SOURCE=.\cmainpass.c
# End Source File
# Begin Source File

SOURCE=.\collapse.c
# End Source File
# Begin Source File

SOURCE=.\oclone.c
# End Source File
# Begin Source File

SOURCE=.\optrate.c
# End Source File
# Begin Source File

SOURCE=.\optrefer.c
# End Source File
# Begin Source File

SOURCE=.\optmain.c
# End Source File
# Begin Source File

SOURCE=.\optconst.c
# End Source File
# Begin Source File

SOURCE=.\control.c
# End Source File
# Begin Source File

SOURCE=.\corecode.c
# End Source File
# Begin Source File

SOURCE=.\coreinline.c
# End Source File
# Begin Source File

SOURCE=.\corevars.c
# End Source File
# Begin Source File

SOURCE=.\csrclib.c
# End Source File
# Begin Source File

SOURCE=.\csyslib.c
# End Source File
# Begin Source File

SOURCE=.\nsyslib.c
# End Source File
# Begin Source File

SOURCE=.\psyslib.c
# End Source File
# Begin Source File

SOURCE=.\globals.c
# End Source File
# Begin Source File

SOURCE=.\lex.c
# End Source File
# Begin Source File

SOURCE=.\mp4read.c
# End Source File
# Begin Source File

SOURCE=.\mp4write.c
# End Source File
# Begin Source File

SOURCE=.\parser.tab.c
# End Source File
# Begin Source File

SOURCE=.\postparse.c
# End Source File
# Begin Source File

SOURCE=.\parsehelp.c
# End Source File
# Begin Source File

SOURCE=.\readmidi.c
# End Source File
# Begin Source File

SOURCE=.\readscore.c
# End Source File
# Begin Source File

SOURCE=.\sfmain.c
# End Source File
# Begin Source File

SOURCE=.\special.c
# End Source File
# Begin Source File

SOURCE=.\stparse.c
# End Source File
# Begin Source File

SOURCE=.\symbols.c
# End Source File
# Begin Source File

SOURCE=.\tokens.c
# End Source File
# Begin Source File

SOURCE=.\treeupdate.c
# End Source File
# Begin Source File

SOURCE=.\writemain.c
# End Source File
# Begin Source File

SOURCE=.\writeop.c
# End Source File
# Begin Source File

SOURCE=.\writeorc.c
# End Source File
# Begin Source File

SOURCE=.\writepre.c
# End Source File
# Begin Source File

SOURCE=.\wtconst.c
# End Source File
# Begin Source File

SOURCE=.\wtparse.c
# End Source File
# End Target
# End Project
