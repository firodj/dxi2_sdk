@echo off
REM -- First make map file from Microsoft Visual C++ generated resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by Sample.HPJ. >"hlp\Sample.hm"
echo. >>"hlp\Sample.hm"
echo // Commands (ID_* and IDM_*) >>"hlp\Sample.hm"
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>"hlp\Sample.hm"
echo. >>"hlp\Sample.hm"
echo // Prompts (IDP_*) >>"hlp\Sample.hm"
makehm IDP_,HIDP_,0x30000 resource.h >>"hlp\Sample.hm"
echo. >>"hlp\Sample.hm"
echo // Resources (IDR_*) >>"hlp\Sample.hm"
makehm IDR_,HIDR_,0x20000 resource.h >>"hlp\Sample.hm"
echo. >>"hlp\Sample.hm"
echo // Dialogs (IDD_*) >>"hlp\Sample.hm"
makehm IDD_,HIDD_,0x20000 resource.h >>"hlp\Sample.hm"
echo. >>"hlp\Sample.hm"
echo // Frame Controls (IDW_*) >>"hlp\Sample.hm"
makehm IDW_,HIDW_,0x50000 resource.h >>"hlp\Sample.hm"
REM -- Make help for Project Sample

echo Building Win32 Help files
start /wait hcw /C /E /M "hlp\Sample.hpj"
echo.
if errorlevel 1 goto :Error
goto :done

:Error
echo Sample.hpj(1) : error: Problem encountered creating help file

:done
echo.
