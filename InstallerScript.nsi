Name "枕头涂胶机"
OutFile "Setup.exe"
InstallDir "$ProgramFiles\WG Automation"
ShowInstDetails show
ShowUninstDetails show

!include "MUI2.nsh"
!include "FileFunc.nsh"
!insertmacro FindWideChar
!define !define MUI_ICON "./Resources/logo.ico"
!define MUI_UNICON "./Resources/uninstall.ico"

!insertmacro MUI_PAGE_WELCOME
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE DirCheckLeave
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "SimpChinese"


Function DirCheckLeave
  ; 获取用户选择的安装目录
  StrCpy $0 $INSTDIR
  ; 检测路径中是否包含宽字符（中文）
  ${FindWideChar} $0 $1
  ; $1=1 表示检测到宽字符，$1=0 表示无
  StrCmp $1 1 DirHasChinese DirNoChinese
  
  DirHasChinese:
    ; 弹出禁止消息框（标题+内容）
    MessageBox MB_OK|MB_ICONERROR "错误：安装目录包含中文字符可能导致软件运行错误！$\r$\n请选择无中文的安装路径。"
    ; 跳回路径选择页，阻止继续安装
    Abort
  DirNoChinese:
    ; 无中文，继续安装流程
FunctionEnd

Section "MainSection" SEC01
    SetShellVarContext all
    CreateDirectory "$INSTDIR"
    File /r /y ""