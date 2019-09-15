# Microsoft Developer Studio Generated NMAKE File, Based on SynthPlugIn.dsp
!IF "$(CFG)" == ""
CFG=SynthPlugIn - Win32 Debug
!MESSAGE No configuration specified. Defaulting to SynthPlugIn - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "SynthPlugIn - Win32 Release" && "$(CFG)" != "SynthPlugIn - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SynthPlugIn.mak" CFG="SynthPlugIn - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SynthPlugIn - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SynthPlugIn - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "SynthPlugIn - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\SynthPlugIn.dll" ".\custom.bld"


CLEAN :
	-@erase "$(INTDIR)\Filter.obj"
	-@erase "$(INTDIR)\Instrument.obj"
	-@erase "$(INTDIR)\MediaParams.obj"
	-@erase "$(INTDIR)\ParamEnvelope.obj"
	-@erase "$(INTDIR)\PlugInApp.obj"
	-@erase "$(INTDIR)\SoftSynth.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StringMap.obj"
	-@erase "$(INTDIR)\SynthPlugIn.obj"
	-@erase "$(INTDIR)\SynthPlugIn.pch"
	-@erase "$(INTDIR)\SynthPlugIn.res"
	-@erase "$(INTDIR)\SynthPlugInPropPage.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\SynthPlugIn.dll"
	-@erase "$(OUTDIR)\SynthPlugIn.exp"
	-@erase "$(OUTDIR)\SynthPlugIn.lib"
	-@erase "custom.bld"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "." /I "include" /I "$(MSSDK)\include" /I "$(MSSDK)\Samples\Multimedia\DirectShow\BaseClasses" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\SynthPlugIn.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\SynthPlugIn.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SynthPlugIn.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib advapi32.lib ole32.lib oleaut32.lib \build\sdks\dxmedia\lib\DMOGUIDS.LIB winmm.lib /nologo /entry:"DllEntryPoint@12" /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\SynthPlugIn.pdb" /machine:I386 /nodefaultlib:"libcmt" /nodefaultlib:"libcmtd" /def:".\SynthPlugIn.def" /out:"$(OUTDIR)\SynthPlugIn.dll" /implib:"$(OUTDIR)\SynthPlugIn.lib" 
DEF_FILE= \
	".\SynthPlugIn.def"
LINK32_OBJS= \
	"$(INTDIR)\Filter.obj" \
	"$(INTDIR)\Instrument.obj" \
	"$(INTDIR)\MediaParams.obj" \
	"$(INTDIR)\ParamEnvelope.obj" \
	"$(INTDIR)\PlugInApp.obj" \
	"$(INTDIR)\SoftSynth.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\StringMap.obj" \
	"$(INTDIR)\SynthPlugIn.obj" \
	"$(INTDIR)\SynthPlugInPropPage.obj" \
	"$(INTDIR)\SynthPlugIn.res"

"$(OUTDIR)\SynthPlugIn.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\Release
TargetName=SynthPlugIn
InputPath=.\Release\SynthPlugIn.dll
SOURCE="$(InputPath)"

".\custom.bld" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	regsvr32.exe /s $(OutDir)\$(TargetName).DLL 
	echo >custom.bld 
<< 
	

!ELSEIF  "$(CFG)" == "SynthPlugIn - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\SynthPlugIn.dll" ".\custom.bld"


CLEAN :
	-@erase "$(INTDIR)\Filter.obj"
	-@erase "$(INTDIR)\Instrument.obj"
	-@erase "$(INTDIR)\MediaParams.obj"
	-@erase "$(INTDIR)\ParamEnvelope.obj"
	-@erase "$(INTDIR)\PlugInApp.obj"
	-@erase "$(INTDIR)\SoftSynth.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StringMap.obj"
	-@erase "$(INTDIR)\SynthPlugIn.obj"
	-@erase "$(INTDIR)\SynthPlugIn.pch"
	-@erase "$(INTDIR)\SynthPlugIn.res"
	-@erase "$(INTDIR)\SynthPlugInPropPage.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\SynthPlugIn.dll"
	-@erase "$(OUTDIR)\SynthPlugIn.exp"
	-@erase "$(OUTDIR)\SynthPlugIn.ilk"
	-@erase "$(OUTDIR)\SynthPlugIn.lib"
	-@erase "$(OUTDIR)\SynthPlugIn.pdb"
	-@erase "custom.bld"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "include" /I "$(MSSDK)\include" /I "$(MSSDK)\Samples\Multimedia\DirectShow\BaseClasses" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\SynthPlugIn.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\SynthPlugIn.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SynthPlugIn.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib advapi32.lib ole32.lib oleaut32.lib \build\sdks\dxmedia\lib\DMOGUIDS.LIB winmm.lib /nologo /entry:"DllEntryPoint@12" /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\SynthPlugIn.pdb" /debug /machine:I386 /nodefaultlib:"libcmt" /nodefaultlib:"libcmtd" /def:".\SynthPlugIn.def" /out:"$(OUTDIR)\SynthPlugIn.dll" /implib:"$(OUTDIR)\SynthPlugIn.lib" /pdbtype:sept 
DEF_FILE= \
	".\SynthPlugIn.def"
LINK32_OBJS= \
	"$(INTDIR)\Filter.obj" \
	"$(INTDIR)\Instrument.obj" \
	"$(INTDIR)\MediaParams.obj" \
	"$(INTDIR)\ParamEnvelope.obj" \
	"$(INTDIR)\PlugInApp.obj" \
	"$(INTDIR)\SoftSynth.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\StringMap.obj" \
	"$(INTDIR)\SynthPlugIn.obj" \
	"$(INTDIR)\SynthPlugInPropPage.obj" \
	"$(INTDIR)\SynthPlugIn.res"

"$(OUTDIR)\SynthPlugIn.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\Debug
TargetName=SynthPlugIn
InputPath=.\Debug\SynthPlugIn.dll
SOURCE="$(InputPath)"

".\custom.bld" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	regsvr32.exe /s $(OutDir)\$(TargetName).DLL 
	echo >custom.bld 
<< 
	

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("SynthPlugIn.dep")
!INCLUDE "SynthPlugIn.dep"
!ELSE 
!MESSAGE Warning: cannot find "SynthPlugIn.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "SynthPlugIn - Win32 Release" || "$(CFG)" == "SynthPlugIn - Win32 Debug"
SOURCE=.\Filter.cpp

"$(INTDIR)\Filter.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\SynthPlugIn.pch"


SOURCE=.\Instrument.cpp

"$(INTDIR)\Instrument.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\SynthPlugIn.pch"


SOURCE=.\MediaParams.cpp

"$(INTDIR)\MediaParams.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\SynthPlugIn.pch"


SOURCE=.\ParamEnvelope.cpp

"$(INTDIR)\ParamEnvelope.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\SynthPlugIn.pch"


SOURCE=.\PlugInApp.cpp

"$(INTDIR)\PlugInApp.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\SynthPlugIn.pch"


SOURCE=.\SoftSynth.cpp

"$(INTDIR)\SoftSynth.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\SynthPlugIn.pch"


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "SynthPlugIn - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "." /I "include" /I "$(MSSDK)\include" /I "$(MSSDK)\Samples\Multimedia\DirectShow\BaseClasses" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\SynthPlugIn.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\SynthPlugIn.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "SynthPlugIn - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "include" /I "$(MSSDK)\include" /I "$(MSSDK)\Samples\Multimedia\DirectShow\BaseClasses" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\SynthPlugIn.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\SynthPlugIn.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\StringMap.cpp

"$(INTDIR)\StringMap.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\SynthPlugIn.pch"


SOURCE=.\SynthPlugIn.cpp

"$(INTDIR)\SynthPlugIn.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\SynthPlugIn.pch"


SOURCE=.\SynthPlugIn.rc

"$(INTDIR)\SynthPlugIn.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\SynthPlugInPropPage.cpp

"$(INTDIR)\SynthPlugInPropPage.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\SynthPlugIn.pch"



!ENDIF 

