# Microsoft Developer Studio Project File - Name="MFCSample" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=MFCSample - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MFCSample.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MFCSample.mak" CFG="MFCSample - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MFCSample - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MFCSample - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Src/MIDIPlugin", WEHAAAAA"
# PROP Scc_LocalPath "..\.."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MFCSample - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "\include" /I "." /I "include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /nologo /subsystem:windows /dll /machine:I386
# Begin Custom Build - Custom Build Steps
OutDir=.\Release
TargetName=MFCSample
InputPath=.\Release\MFCSample.dll
SOURCE="$(InputPath)"

"custom.bld" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s $(OutDir)\$(TargetName).DLL 
	echo >custom.bld 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "MFCSample - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "include" /I "." /D "_DEBUG" /D "_AFXDLL" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# Begin Custom Build - Custom Build Steps
OutDir=.\Debug
TargetName=MFCSample
InputPath=.\Debug\MFCSample.dll
SOURCE="$(InputPath)"

"custom.bld" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s $(OutDir)\$(TargetName).DLL 
	echo >custom.bld 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "MFCSample - Win32 Release"
# Name "MFCSample - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\MFCSample.cpp
# End Source File
# Begin Source File

SOURCE=.\MFCSample.def
# End Source File
# Begin Source File

SOURCE=.\hlp\MFCSample.hpj

!IF  "$(CFG)" == "MFCSample - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
OutDir=.\Release
ProjDir=.
TargetName=MFCSample
InputPath=.\hlp\MFCSample.hpj

"$(OutDir)\$(TargetName).hlp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	call "$(ProjDir)\makehelp.bat" 
	if    exist    hlp\$(TargetName).hlp    xcopy    hlp\$(TargetName).hlp    $(OutDir)\  
	if    exist    hlp\$(TargetName).cnt    xcopy    hlp\$(TargetName).cnt    $(OutDir)\  
	
# End Custom Build

!ELSEIF  "$(CFG)" == "MFCSample - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
OutDir=.\Debug
ProjDir=.
TargetName=MFCSample
InputPath=.\hlp\MFCSample.hpj

"$(OutDir)\$(TargetName).hlp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	call "$(ProjDir)\makehelp.bat" 
	if    exist    hlp\$(TargetName).hlp    xcopy    hlp\$(TargetName).hlp    $(OutDir)\  
	if    exist    hlp\$(TargetName).cnt    xcopy    hlp\$(TargetName).cnt    $(OutDir)\  
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MFCSample.rc
# End Source File
# Begin Source File

SOURCE=.\MFCSamplePropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\MFCSample.h
# End Source File
# Begin Source File

SOURCE=.\MFCSamplePropPage.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Help Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\hlp\MFCSample.rtf
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
