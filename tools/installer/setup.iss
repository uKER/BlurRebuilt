; Offline installation of the complete Rebuilt payload and controller backend.
; Compiled only through build.ps1, which supplies a sanitized payload directory.
#ifndef PayloadDir
  #error Build this script using tools/installer/build.ps1
#endif
[Setup]
#ifdef InstallerTest
AppId={{55CBAFE6-BE43-40FA-AC09-00689D83F2A8}
AppName=Blur Rebuilt Installer Test
PrivilegesRequired=lowest
CreateUninstallRegKey=no
UsePreviousAppDir=no
#else
AppId={{70BEFC9C-9D12-4E54-B8A7-09DD75CC9071}
AppName=Blur Rebuilt
PrivilegesRequired=admin
#endif
AppVersion={#ReleaseVersion}
AppPublisher=Blur Rebuilt contributors
AppPublisherURL=https://github.com/uKER/BlurRebuilt
DefaultDirName={code:FindGameFolder}
AppendDefaultDirName=no
DisableDirPage=no
DirExistsWarning=no
DisableProgramGroupPage=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0
PrivilegesRequiredOverridesAllowed=commandline
CloseApplications=no
RestartApplications=no
SetupLogging=yes
UninstallFilesDir={app}\_BlurRebuiltSetup
UninstallDisplayIcon={app}\BlurRebuilt.exe
OutputDir={#OutputDir}
OutputBaseFilename={#SetupName}
Compression=lzma2/fast
SolidCompression=yes
WizardStyle=modern
[Files]
Source: "{#PayloadDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs uninsneveruninstall
Source: "{#ManifestFile}"; DestName: "payload-files.txt"; Flags: dontcopy
Source: "{#RedistFile}"; DestName: "vc_redist.x64.exe"; Flags: dontcopy
#ifndef InstallerTest
[Tasks]
Name: desktopicon; Description: "Create a desktop shortcut"; GroupDescription: "Shortcuts:"; Flags: unchecked
[Icons]
Name: "{autoprograms}\Blur Rebuilt"; Filename: "{app}\BlurRebuilt.exe"; WorkingDir: "{app}"
Name: "{autodesktop}\Blur Rebuilt"; Filename: "{app}\BlurRebuilt.exe"; WorkingDir: "{app}"; Tasks: desktopicon
#endif
[Code]
var PayloadFiles: TArrayOfString;
function OpenFileExclusive(Name: String; Access, Share, Security, Disposition, Attributes: Cardinal; Template: THandle): THandle;
  external 'CreateFileW@kernel32.dll stdcall';
function CloseFileHandle(Handle: THandle): Integer;
  external 'CloseHandle@kernel32.dll stdcall';

function GameRoot(): String;
var Path: String;
begin
  Path := ExpandConstant('{app}\');
  if Copy(Path, 1, 4) = '\\?\' then Result := Path
  else if Copy(Path, 1, 2) = '\\' then Result := '\\?\UNC\' + Copy(Path, 3, Length(Path))
  else Result := '\\?\' + Path;
end;

function SafeRelative(const Name: String): Boolean;
begin
  Result := (Name <> '') and (Pos('..', Name) = 0) and
    (Pos(':', Name) = 0) and (Pos('/', Name) = 0) and (Name[1] <> '\') and
    (CompareText(Copy(Name, 1, Length('_BlurRebuiltSetup\')), '_BlurRebuiltSetup\') <> 0);
end;

function FindInRegistry(Root: Integer): String;
var Keys: TArrayOfString; I: Integer; Location: String;
begin
  Result := '';
  if RegGetSubkeyNames(Root, 'Software\Microsoft\Windows\CurrentVersion\Uninstall', Keys) then
    for I := 0 to GetArrayLength(Keys)-1 do
      if RegQueryStringValue(Root, 'Software\Microsoft\Windows\CurrentVersion\Uninstall\' + Keys[I], 'InstallLocation', Location) then
        if FileExists(AddBackslash(Location) + 'Blur.exe') then begin
          Result := Location; Exit;
        end;
end;

function FindGameFolder(Param: String): String;
begin
  Result := ExtractFileDir(ExpandConstant('{srcexe}'));
  if FileExists(AddBackslash(Result) + 'Blur.exe') then Exit;
  Result := FindInRegistry(HKLM32);
  if Result = '' then Result := FindInRegistry(HKLM64);
  if Result = '' then Result := FindInRegistry(HKCU32);
  if Result = '' then Result := ExpandConstant('{autopf32}\Activision\Blur(TM)');
end;

function FileBusy(const Name: String): Boolean;
var Handle: THandle;
begin
  Result := False;
  if not FileExists(Name) then Exit;
  Handle := OpenFileExclusive(Name, $C0000000, 0, 0, 3, $80, 0);
  Result := Handle = THandle(-1);
  if not Result then CloseFileHandle(Handle);
end;

function GameBusy(): Boolean;
begin
  Result := FileBusy(GameRoot() + 'Blur.exe') or
    FileBusy(GameRoot() + 'BlurRebuilt.exe') or
    FileBusy(GameRoot() + 'BlurInputFusion.dll') or
    FileBusy(GameRoot() + 'AmaxClientHost.exe');
end;

function NeedRuntime(): Boolean;
var Installed: Cardinal; VersionText: String; Existing, Required: Int64;
begin
  Result := True;
  if RegQueryDWordValue(HKLM64, 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64', 'Installed', Installed) and (Installed = 1) and
    RegQueryStringValue(HKLM64, 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64', 'Version', VersionText) then begin
    if Copy(VersionText, 1, 1) = 'v' then Delete(VersionText, 1, 1);
    if StrToVersion(VersionText, Existing) and StrToVersion('{#RuntimeVersion}', Required) then
      Result := ComparePackedVersion(Existing, Required) < 0;
  end;
end;

function EnsureRuntime(var NeedsRestart: Boolean): String;
var Code: Integer;
begin
  Result := '';
  if NeedRuntime() then begin
    ExtractTemporaryFile('vc_redist.x64.exe');
    if not Exec(ExpandConstant('{tmp}\vc_redist.x64.exe'), '/install /quiet /norestart', '', SW_HIDE, ewWaitUntilTerminated, Code) then
      Result := 'The Microsoft C++ runtime installer could not start.'
    else if (Code <> 0) and (Code <> 3010) and (Code <> 1638) then
      Result := 'The Microsoft C++ runtime installation failed (code ' + IntToStr(Code) + ').'
    else if Code = 3010 then NeedsRestart := True;
  end;
end;

function BackupPayload(): String;
var I, J, N, RecordIndex, OriginalCount: Integer; Name, Original, Added, BackupRoot, ListFile: String;
  Managed, RecordIds: TArrayOfString;
begin
  Result := '';
  BackupRoot := GameRoot() + '_BlurRebuiltSetup\backup\';
  ListFile := BackupRoot + 'managed-files.txt';
  if FileExists(ListFile) then LoadStringsFromFile(ListFile, Managed);
  OriginalCount := GetArrayLength(Managed);
  SetArrayLength(RecordIds, GetArrayLength(PayloadFiles));
  for I := 0 to GetArrayLength(PayloadFiles)-1 do begin
    Name := PayloadFiles[I];
    if not SafeRelative(Name) then begin Result := 'Invalid setup file path.'; Exit; end;
    RecordIndex := -1;
    for J := 0 to OriginalCount-1 do if Managed[J] = Name then RecordIndex := J;
    if RecordIndex < 0 then begin
      N := GetArrayLength(Managed); SetArrayLength(Managed, N+1); Managed[N] := Name; RecordIndex := N;
    end;
    RecordIds[I] := IntToStr(RecordIndex);
  end;
  { Commit the name-to-record mapping before creating any numbered snapshot. }
  if not ForceDirectories(BackupRoot) or not SaveStringsToFile(ListFile, Managed, False) then begin
    Result := 'Could not save the installation backup list.'; Exit;
  end;
  for I := 0 to GetArrayLength(PayloadFiles)-1 do begin
    Name := PayloadFiles[I];
    Original := BackupRoot + 'original\' + RecordIds[I];
    Added := BackupRoot + 'added\' + RecordIds[I];
    if not FileExists(Original) and not FileExists(Added) then begin
      if FileExists(GameRoot() + Name) then begin
        if not ForceDirectories(ExtractFileDir(Original)) or
          not FileCopy(GameRoot() + Name, Original, True) then begin
          Result := 'Could not back up ' + Name + '. No game files have been replaced.'; Exit;
        end;
      end else begin
        if not ForceDirectories(ExtractFileDir(Added)) or not SaveStringToFile(Added, 'added', False) then begin
          Result := 'Could not record the installation backup for ' + Name + '.'; Exit;
        end;
      end;
    end;
  end;
end;

function PrepareToInstall(var NeedsRestart: Boolean): String;
begin
  Result := '';
  if not FileExists(GameRoot() + 'Blur.exe') then begin
    Result := 'Choose your installed Blur folder, containing Blur.exe.'; Exit;
  end;
  if GameBusy() then begin Result := 'Close Blur and its online companion, then retry setup.'; Exit; end;
  ExtractTemporaryFile('payload-files.txt');
  if not LoadStringsFromFile(ExpandConstant('{tmp}\payload-files.txt'), PayloadFiles) then begin
    Result := 'The setup file list could not be read.'; Exit;
  end;
  Result := EnsureRuntime(NeedsRestart);
  if Result = '' then Result := BackupPayload();
end;

function InitializeUninstall(): Boolean;
begin
  Result := not GameBusy();
  if not Result then MsgBox('Close Blur and its online companion before uninstalling.', mbError, MB_OK);
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var Names: TArrayOfString; I: Integer; Root, Original, Added, Name: String;
begin
  if CurUninstallStep <> usUninstall then Exit;
  Root := GameRoot() + '_BlurRebuiltSetup\backup\';
  if not LoadStringsFromFile(Root + 'managed-files.txt', Names) then
    RaiseException('Installation backup list is missing. No game files have been removed.');
  for I := 0 to GetArrayLength(Names)-1 do begin
    Name := Names[I];
    if not SafeRelative(Name) then RaiseException('Invalid backup file path.');
    Original := Root + 'original\' + IntToStr(I); Added := Root + 'added\' + IntToStr(I);
    if FileExists(Original) then begin
      if not FileCopy(Original, GameRoot() + Name, False) then
        RaiseException('Could not restore ' + Name + '. The backup is retained.');
    end else if FileExists(Added) and FileExists(GameRoot() + Name) then
      if not DeleteFile(GameRoot() + Name) then RaiseException('Could not remove ' + Name + '.');
  end;
  { Archive the restored snapshot so a later fresh install records its own baseline. }
  if not RenameFile(RemoveBackslash(Root), RemoveBackslash(Root) + '-restored-' +
      GetDateTimeString('yyyymmddhhnnss', '-', ':')) then
    RaiseException('Files were restored, but the backup could not be archived.');
end;
