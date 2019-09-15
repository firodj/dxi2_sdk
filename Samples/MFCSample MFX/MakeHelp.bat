@echo off
REM -- First make map file from Microsoft Visual C++ generated resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by MFCSample.HPJ. >"hlp\MFCSample.hm"
echo. >>"hlp\MFCSample.hm"
echo // Commands (ID_* and IDM_*) >>"hlp\MFCSample.hm"
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>"hlp\MFCSample.hm"
echo. >>"hlp\MFCSample.hm"
echo // Prompts (IDP_*) >>"hlp\MFCSample.hm"
makehm IDP_,HIDP_,0x30000 resource.h >>"hlp\MFCSample.hm"
echo. >>"hlp\MFCSample.hm"
echo // Resources (IDR_*) >>"hlp\MFCSample.hm"
makehm IDR_,HIDR_,0x20000 resource.h >>"hlp\MFCSample.hm"
echo. >>"hlp\MFCSample.hm"
echo // Dialogs (IDD_*) >>"hlp\MFCSample.hm"
makehm IDD_,HIDD_,0x20000 resource.h >>"hlp\MFCSample.hm"
echo. >>"hlp\MFCSample.hm"
echo // Frame Controls (IDW_*) >>"hlp\MFCSample.hm"
makehm IDW_,HIDW_,0x50000 resource.h >>"hlp\MFCSample.hm"

REM -- Make help for Project MFCSample
echo Building Win32 Help files
start /wait hcw /C /E /M "hlp\MFCSample.hpj"
echo.
if errorlevel 1 goto :Error
goto :done

:Error
echo MFCSample.hpj(1) : error: Problem encountered creating help file

:done
echo.
