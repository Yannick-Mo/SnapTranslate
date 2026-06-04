!define PRODUCT_NAME "SnapTranslate"
!define PRODUCT_VERSION "1.0.0"
!define PRODUCT_PUBLISHER "Yaro"
!define PRODUCT_WEB_SITE "https://github.com/Yaro-Moss/SnapTranslate"

!include "MUI2.nsh"
!include "FileFunc.nsh"

;--------------------------------
; General
;--------------------------------
Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "build\SnapTranslate-${PRODUCT_VERSION}-Setup.exe"
Unicode True
RequestExecutionLevel admin
BrandingText "SnapTranslate Installer"

;--------------------------------
; Variables
;--------------------------------
Var StartMenuFolder

;--------------------------------
; Interface Settings
;--------------------------------
!define MUI_ABORTWARNING
; Using default NSIS icons
!define MUI_COMPONENTSPAGE_SMALLDESC

;--------------------------------
; Pages
;--------------------------------
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
; Languages
;--------------------------------
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "English"

;--------------------------------
; Install Section
;--------------------------------
Section "SnapTranslate" SecMain
    SetOutPath "$INSTDIR"

    ; Main executable
    File "build\Release\screenshot-translator.exe"

    ; Qt DLLs
    File "build\Release\Qt6Core.dll"
    File "build\Release\Qt6Gui.dll"
    File "build\Release\Qt6Network.dll"
    File "build\Release\Qt6Svg.dll"
    File "build\Release\Qt6Widgets.dll"
    File "build\Release\icuuc.dll"

    ; Qt plugins
    SetOutPath "$INSTDIR\generic"
    File "build\Release\generic\qtuiotouchplugin.dll"

    SetOutPath "$INSTDIR\iconengines"
    File "build\Release\iconengines\qsvgicon.dll"

    SetOutPath "$INSTDIR\imageformats"
    File "build\Release\imageformats\qgif.dll"
    File "build\Release\imageformats\qico.dll"
    File "build\Release\imageformats\qjpeg.dll"
    File "build\Release\imageformats\qsvg.dll"

    SetOutPath "$INSTDIR\networkinformation"
    File "build\Release\networkinformation\qnetworklistmanager.dll"

    SetOutPath "$INSTDIR\platforms"
    File "build\Release\platforms\qwindows.dll"

    SetOutPath "$INSTDIR\styles"
    File "build\Release\styles\qmodernwindowsstyle.dll"

    SetOutPath "$INSTDIR\tls"
    File "build\Release\tls\qcertonlybackend.dll"
    File "build\Release\tls\qschannelbackend.dll"

    ; Config template (user copies to config.json)
    SetOutPath "$INSTDIR"
    File "config.json.template"
    File "resources\icon.png"

    ; Write uninstaller
    WriteUninstaller "$INSTDIR\uninstall.exe"

    ; Start menu shortcuts
    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
        CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
        CreateShortcut "$SMPROGRAMS\$StartMenuFolder\SnapTranslate.lnk" "$INSTDIR\screenshot-translator.exe"
        CreateShortcut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\uninstall.exe"
    !insertmacro MUI_STARTMENU_WRITE_END

    ; Add/Remove Programs entry
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" \
        "DisplayName" "${PRODUCT_NAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" \
        "DisplayVersion" "${PRODUCT_VERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" \
        "Publisher" "${PRODUCT_PUBLISHER}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" \
        "UninstallString" "$INSTDIR\uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" \
        "DisplayIcon" "$INSTDIR\screenshot-translator.exe"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" \
        "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" \
        "NoRepair" 1
SectionEnd

;--------------------------------
; Uninstall Section
;--------------------------------
Section "Uninstall"
    ; Remove files
    Delete "$INSTDIR\screenshot-translator.exe"
    Delete "$INSTDIR\Qt6Core.dll"
    Delete "$INSTDIR\Qt6Gui.dll"
    Delete "$INSTDIR\Qt6Network.dll"
    Delete "$INSTDIR\Qt6Svg.dll"
    Delete "$INSTDIR\Qt6Widgets.dll"
    Delete "$INSTDIR\icuuc.dll"
    Delete "$INSTDIR\config.json.template"
    Delete "$INSTDIR\icon.png"
    Delete "$INSTDIR\uninstall.exe"

    ; Remove plugins
    Delete "$INSTDIR\generic\qtuiotouchplugin.dll"
    Delete "$INSTDIR\iconengines\qsvgicon.dll"
    Delete "$INSTDIR\imageformats\qgif.dll"
    Delete "$INSTDIR\imageformats\qico.dll"
    Delete "$INSTDIR\imageformats\qjpeg.dll"
    Delete "$INSTDIR\imageformats\qsvg.dll"
    Delete "$INSTDIR\networkinformation\qnetworklistmanager.dll"
    Delete "$INSTDIR\platforms\qwindows.dll"
    Delete "$INSTDIR\styles\qmodernwindowsstyle.dll"
    Delete "$INSTDIR\tls\qcertonlybackend.dll"
    Delete "$INSTDIR\tls\qschannelbackend.dll"

    ; Remove directories
    RMDir "$INSTDIR\generic"
    RMDir "$INSTDIR\iconengines"
    RMDir "$INSTDIR\imageformats"
    RMDir "$INSTDIR\networkinformation"
    RMDir "$INSTDIR\platforms"
    RMDir "$INSTDIR\styles"
    RMDir "$INSTDIR\tls"
    RMDir "$INSTDIR"

    ; Remove start menu
    !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
    Delete "$SMPROGRAMS\$StartMenuFolder\SnapTranslate.lnk"
    Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
    RMDir "$SMPROGRAMS\$StartMenuFolder"

    ; Remove registry keys
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
SectionEnd
