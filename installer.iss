#define MyAppName "FCM"
#define MyAppVersion "1.1.0"
#define MyAppPublisher "My Company"
#define MyAppExeName "FCM.exe"
#define MyAppDataDir "{userappdata}\FCM"

[Setup]
AppId={{A1B2C3D4-1234-5678-9999-ABCDEF123456}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
OutputDir=.
OutputBaseFilename=FCM-Setup
Compression=lzma
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"

[Tasks]
Name: "desktopicon"; Description: "Создать ярлык на рабочем столе"; GroupDescription: "Дополнительно:"

[Dirs]
Name: "{#MyAppDataDir}"

[Files]
Source: "FCM-build-1.1.0\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "FCM-build-1.1.0\app.ini"; DestDir: "{#MyAppDataDir}"; Flags: ignoreversion onlyifdoesntexist
Source: "FCM-build-1.1.0\models.db"; DestDir: "{#MyAppDataDir}"; Flags: ignoreversion onlyifdoesntexist

[Icons]
Name: "{group}\FCM"; Filename: "{app}\FCM.exe"; WorkingDir: "{#MyAppDataDir}"
Name: "{autodesktop}\FCM"; Filename: "{app}\FCM.exe"; WorkingDir: "{#MyAppDataDir}"; Tasks: desktopicon

[Run]
Filename: "{app}\FCM.exe"; Description: "Запустить {#MyAppName}"; WorkingDir: "{#MyAppDataDir}"; Flags: nowait postinstall skipifsilent
