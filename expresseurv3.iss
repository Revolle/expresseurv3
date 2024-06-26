[Setup]
AppName=ExpresseurV3
AppVersion=3.52
DefaultDirName={pf}\ExpresseurV3
DefaultGroupName=ExpresseurV3
UninstallDisplayIcon={app}\expresseur.ico
OutputBaseFilename=ExpresseurV3_52_setup
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

[Files]
Source: "..\..\expresseurV3_VC\basslua\x64\Release\expresseur.exe"; DestDir: "{app}";
Source: "..\..\expresseurV3_VC\basslua\x64\Release\expresscmd.exe"; DestDir: "{app}";
Source: "..\..\expresseurV3_VC\basslua\x64\Release\bass.dll"; DestDir: "{app}";
Source: "..\..\expresseurV3_VC\basslua\x64\Release\bassmix.dll"; DestDir: "{app}";
Source: "..\..\expresseurV3_VC\basslua\x64\Release\bassmidi.dll"; DestDir: "{app}";
Source: "..\..\expresseurV3_VC\basslua\x64\Release\bassasio.dll"; DestDir: "{app}";
Source: "..\..\expresseurV3_VC\basslua\x64\Release\lua53.dll"; DestDir: "{app}";
Source: "..\..\expresseurV3_VC\basslua\x64\Release\luabass.dll"; DestDir: "{app}";
Source: "..\..\expresseurV3_VC\basslua\x64\Release\basslua.dll"; DestDir: "{app}";
Source: "..\..\expresseurV3_VC\basslua\x64\Release\*.jpg"; DestDir: "{app}";
Source: "..\..\expresseurV3_VC\basslua\x64\Release\*.png"; DestDir: "{app}";
Source: "..\..\expresseurV3_VC\basslua\x64\Release\*.lua"; DestDir: "{app}";
Source: "..\..\expresseurV3_VC\basslua\x64\Release\*.qml"; DestDir: "{app}";
Source: "..\..\expresseurV3_VC\basslua\x64\Release\*.wav"; DestDir: "{app}";
Source: "..\..\expresseur\expresseur\expresseur.ico"; DestDir: "{app}";
Source: "..\..\expresseur\example\*.mxl"; DestDir: "{app}\example\";
Source: "..\..\expresseur\example\*.txt"; DestDir: "{app}\example\";
Source: "..\..\expresseur\example\*.png"; DestDir: "{app}\example\";
Source: "..\..\expresseur\example\*.txb"; DestDir: "{app}\example\";
Source: "..\..\expresseur\ressources\*.sf2"; DestDir: "{app}\ressources\";
Source: "..\..\expresseur\ressources\*.txt"; DestDir: "{app}\ressources\";
Source: "Readme.txt"; DestDir: "{app}"; Flags: isreadme
Source: "vc_redist.x64.exe"; DestDir: "{app}";

[Icons]
Name: "{group}\ExpresseurV3"; Filename: "{app}\expresseur.exe"

[Run]
Filename: "{app}\vc_redist.x64.exe"; 