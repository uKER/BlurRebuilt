; -- Nexus.iss --

[Setup]
AppName=Nexus
AppVerName=Nexus v1.77
DefaultDirName={pf}\Bizarre Creations\Nexus
DefaultGroupName=Bizarre Creations\Nexus
UninstallDisplayIcon={app}\Nexus.exe
ChangesAssociations=yes

[Files]
Source: "../bin/Nexus.exe"; DestDir: "{app}"
Source: "../bin/BTNexgenIPL32.dll"; DestDir: "{app}"
Source: "../bin/XSIFtk.dll"; DestDir: "{app}"
Source: "../help/Help.htm"; DestDir: "{app}"

[Icons]
Name: "{group}\Nexus"; Filename: "{app}\Nexus.exe"; WorkingDir: "{app}"
Name: "{group}\Help"; Filename: "{app}\Help.htm"
Name: "{group}\Uninstall Nexus"; Filename: "{uninstallexe}"

; NOTE: Most apps do not need registry entries to be pre-created. If you
; don't know what the registry is or if you need to use it, then chances are
; you don't need a [Registry] section.

[Registry]
; Start "Software\My Company\My Program" keys under HKEY_CURRENT_USER
; and HKEY_LOCAL_MACHINE. The flags tell it to always delete the
; "My Program" keys upon uninstall, and delete the "My Company" keys
; if there is nothing left in them.
Root: HKCR; Subkey: ".a3d"; ValueType: string; ValueName: ""; ValueData: "Abstract3D"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "Abstract3D"; ValueType: string; ValueName: ""; ValueData: "Abstract3D"; Flags: uninsdeletekey
Root: HKCR; Subkey: "Abstract3D\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\Nexus.exe,0"
Root: HKCR; Subkey: "Abstract3D\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\Nexus.exe"" ""%1"""

