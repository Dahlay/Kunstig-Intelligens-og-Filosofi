; packaging/windows/installer.nsi
; NSIS installer script for Modular Audio Patcher
; Requires NSIS >= 3.x with Modern UI 2
;
; Build:
;   makensis packaging/windows/installer.nsi
;
; Prerequisites on the build machine:
;   - NSIS installed (https://nsis.sourceforge.io/)
;   - Build artifact at:
;       build\ModularAudioPatcher_artefacts\Release\ModularAudioPatcher.exe
;   - installer.ico present in this directory

Unicode True

!define APPNAME     "Modular Audio Patcher"
!define APPVERSION  "1.0.0"
!define PUBLISHER   "Even"
!define WEBSITE     "https://example.com"
!define HELPURL     "https://example.com/support"
!define UPDATEURL   "https://example.com/update"
!define ABOUTURL    "https://example.com/about"
!define INSTALLSIZE 65536   ; Estimated installed size in KB

; Output installer filename
OutFile "ModularAudioPatcher-${APPVERSION}-Setup.exe"
Name    "${APPNAME} ${APPVERSION}"
InstallDir "$PROGRAMFILES64\${APPNAME}"
InstallDirRegKey HKLM "Software\${APPNAME}" "Install_Dir"
RequestExecutionLevel admin

;---------------------------------------------------------------------
; Modern UI
;---------------------------------------------------------------------
!include "MUI2.nsh"

!define MUI_ABORTWARNING
!define MUI_ICON   "installer.ico"
!define MUI_UNICON "installer.ico"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\..\LICENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

;---------------------------------------------------------------------
; Installer sections
;---------------------------------------------------------------------
Section "Main Application" SecMain
  SectionIn RO

  SetOutPath "$INSTDIR"
  File "..\..\build\ModularAudioPatcher_artefacts\Release\ModularAudioPatcher.exe"

  ; Write uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ; Start Menu shortcut
  CreateDirectory "$SMPROGRAMS\${APPNAME}"
  CreateShortcut  "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk" "$INSTDIR\ModularAudioPatcher.exe"
  CreateShortcut  "$SMPROGRAMS\${APPNAME}\Uninstall.lnk"  "$INSTDIR\Uninstall.exe"

  ; Desktop shortcut
  CreateShortcut "$DESKTOP\${APPNAME}.lnk" "$INSTDIR\ModularAudioPatcher.exe"

  ; Registry entries (Add/Remove Programs)
  WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayName"     "${APPNAME}"
  WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "InstallLocation" "$INSTDIR"
  WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayIcon"     "$INSTDIR\ModularAudioPatcher.exe"
  WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "Publisher"       "${PUBLISHER}"
  WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "HelpLink"        "${HELPURL}"
  WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "URLUpdateInfo"   "${UPDATEURL}"
  WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "URLInfoAbout"    "${ABOUTURL}"
  WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayVersion"  "${APPVERSION}"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "NoModify"        1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "NoRepair"        1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "EstimatedSize"   ${INSTALLSIZE}
SectionEnd

;---------------------------------------------------------------------
; Uninstaller
;---------------------------------------------------------------------
Section "Uninstall"
  Delete "$INSTDIR\ModularAudioPatcher.exe"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir  "$INSTDIR"

  Delete "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk"
  Delete "$SMPROGRAMS\${APPNAME}\Uninstall.lnk"
  RMDir  "$SMPROGRAMS\${APPNAME}"
  Delete "$DESKTOP\${APPNAME}.lnk"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
  DeleteRegKey HKLM "Software\${APPNAME}"
SectionEnd
