; -- FileTagManager.iss --

[Setup]
AppName=File Tag Manager
AppVerName=File Tag Manager v1.01
DefaultDirName={pf}\Bizarre Creations\File Tag Manager
DefaultGroupName=Bizarre Creations\File Tag Manager
UninstallDisplayIcon={app}\File Tag Manager
ChangesAssociations=no

[Files]
Source: "../bin/Release/FileTagManager.exe"; DestDir: "{app}"
Source: "../bin/Release/FileTagManagerConfig.xml"; DestDir: "{app}"
Source: "../bin/Release/CustomFolderBrowser.dll"; DestDir: "{app}"
Source: "../bin/Release/FolderBrowser.dll"; DestDir: "{app}"
Source: "../bin/Release/FreeImageWrapper.dll"; DestDir: "{app}"
Source: "../bin/Release/ActorSystem.dll"; DestDir: "{app}"
Source: "../bin/Release/FreeImage.dll"; DestDir: "{app}"
Source: "../bin/Release/RegexpHelper.dll"; DestDir: "{app}"

[Icons]
Name: "{group}\File Tag Manager"; Filename: "{app}\FileTagManager.exe"; WorkingDir: "{app}"
Name: "{group}\Uninstall File Tag Manager"; Filename: "{uninstallexe}"

; NOTE: Most apps do not need registry entries to be pre-created. If you
; don't know what the registry is or if you need to use it, then chances are
; you don't need a [Registry] section.


