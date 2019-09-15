# Microsoft Developer Studio Project File - Name="DXiClasses" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=DXiClasses - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DXiClasses.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DXiClasses.mak" CFG="DXiClasses - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DXiClasses - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "DXiClasses - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DXiClasses - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /Gz /MD /W3 /GX /O2 /I "..\include" /I "$(MSSDK)\include" /I "$(MSSDK)\Samples\Multimedia\DirectShow\BaseClasses" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "DXiClasses - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /Gz /MDd /W3 /Gm /GX /ZI /Od /I "..\include" /I "$(MSSDK)\include" /I "$(MSSDK)\Samples\Multimedia\DirectShow\BaseClasses" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "DXiClasses - Win32 Release"
# Name "DXiClasses - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\DxAllocator.cpp
# End Source File
# Begin Source File

SOURCE=.\DxBufRenderer.cpp
# End Source File
# Begin Source File

SOURCE=.\DxFilterGraph.cpp
# End Source File
# Begin Source File

SOURCE=.\DxNullSource.cpp
# End Source File
# Begin Source File

SOURCE=.\MfxBufferFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\MfxDataQueue.cpp
# End Source File
# Begin Source File

SOURCE=.\MfxEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\MfxEventQueue.cpp
# End Source File
# Begin Source File

SOURCE=.\MfxHostSite.cpp
# End Source File
# Begin Source File

SOURCE=.\MfxMeterKeySigMap.cpp
# End Source File
# Begin Source File

SOURCE=.\MfxSeq.cpp
# End Source File
# Begin Source File

SOURCE=.\MfxTempoMap.cpp
# End Source File
# Begin Source File

SOURCE=.\MfxTrack.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\xRPNContext.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\DxAllocator.h
# End Source File
# Begin Source File

SOURCE=.\DxBufRenderer.h
# End Source File
# Begin Source File

SOURCE=.\DxFilterGraph.h
# End Source File
# Begin Source File

SOURCE=.\DxNullSource.h
# End Source File
# Begin Source File

SOURCE=.\MfxBufferFactory.h
# End Source File
# Begin Source File

SOURCE=.\MfxDataQueue.h
# End Source File
# Begin Source File

SOURCE=.\MfxEvent.h
# End Source File
# Begin Source File

SOURCE=.\MfxEventQueue.h
# End Source File
# Begin Source File

SOURCE=.\MfxHostSite.h
# End Source File
# Begin Source File

SOURCE=.\MfxMarker.h
# End Source File
# Begin Source File

SOURCE=.\MfxMeterKeySigMap.h
# End Source File
# Begin Source File

SOURCE=.\MfxSeq.h
# End Source File
# Begin Source File

SOURCE=.\MfxTempoMap.h
# End Source File
# Begin Source File

SOURCE=.\MfxTrack.h
# End Source File
# Begin Source File

SOURCE=.\MidiDefs.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Templates.h
# End Source File
# Begin Source File

SOURCE=.\xRPNContext.h
# End Source File
# End Group
# End Target
# End Project
