# Microsoft Developer Studio Project File - Name="proxy_rd" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=proxy_rd - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "proxy_rd.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "proxy_rd.mak" CFG="proxy_rd - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "proxy_rd - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "proxy_rd - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "proxy_rd - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "proxy_rd - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "proxy_rd - Win32 Release"
# Name "proxy_rd - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\proxy_rd.cpp
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\autobuild.h
# End Source File
# Begin Source File

SOURCE=".\demo-script.h"
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\main.ico
# End Source File
# End Group
# Begin Group "zinc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\zinc\base64.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\base64.h
# End Source File
# Begin Source File

SOURCE=..\zinc\pcre\config.h
# End Source File
# Begin Source File

SOURCE=..\zinc\crc32.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\crc32.h
# End Source File
# Begin Source File

SOURCE=..\zinc\pcre\pcre.h
# End Source File
# Begin Source File

SOURCE=..\zinc\pcre\pcre_chartables.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\pcre\pcre_compile.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\pcre\pcre_exec.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\pcre\pcre_fullinfo.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\pcre\pcre_globals.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\pcre\pcre_info.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\pcre\pcre_internal.h
# End Source File
# Begin Source File

SOURCE=..\zinc\pcre\pcre_newline.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\pcre\pcre_tables.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\pcre\pcre_try_flipped.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\snprintf.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\snprintf.h
# End Source File
# Begin Source File

SOURCE=..\zinc\zlc\strtod.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\zlc\strtod.h
# End Source File
# Begin Source File

SOURCE=..\zinc\pcre\ucp.h
# End Source File
# Begin Source File

SOURCE=..\zinc\utstrings.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\utstrings.h
# End Source File
# Begin Source File

SOURCE=..\zinc\zalloc.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\zalloc.h
# End Source File
# Begin Source File

SOURCE=..\zinc\zcontrols.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\zcontrols.h
# End Source File
# Begin Source File

SOURCE=..\zinc\zdbg.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\zdbg.h
# End Source File
# Begin Source File

SOURCE=..\zinc\zfiles.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\zfiles.h
# End Source File
# Begin Source File

SOURCE=..\zinc\zibuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\zibuffer.h
# End Source File
# Begin Source File

SOURCE=..\zinc\zini.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\zini.h
# End Source File
# Begin Source File

SOURCE=..\zinc\zlc\zl.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\zlc\zl.h
# End Source File
# Begin Source File

SOURCE=..\zinc\zlc\zl_compiler.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\zlc\zl_compiler.h
# End Source File
# Begin Source File

SOURCE=..\zinc\zlc\zl_lexer.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\zlc\zl_opcodes.h
# End Source File
# Begin Source File

SOURCE=..\zinc\zlc\zl_parser.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\zlc\zl_parser.h
# End Source File
# Begin Source File

SOURCE=..\zinc\zlc\zl_vars.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\zlc\zl_vars.h
# End Source File
# Begin Source File

SOURCE=..\zinc\zlist.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\zlist.h
# End Source File
# Begin Source File

SOURCE=..\zinc\zlog.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\zlog.h
# End Source File
# Begin Source File

SOURCE=..\zinc\zpcre.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\zpcre.h
# End Source File
# Begin Source File

SOURCE=..\zinc\zwindows.cpp
# End Source File
# Begin Source File

SOURCE=..\zinc\zwindows.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\_info.txt
# End Source File
# End Target
# End Project
