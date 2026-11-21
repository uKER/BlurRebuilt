[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$Setup)
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$Setup = (Resolve-Path -LiteralPath $Setup).Path
$metadata = Get-Content -LiteralPath ($Setup+'.json') -Raw | ConvertFrom-Json
if (!$metadata.Fixture) { throw 'Use an InstallerTest build: fixture testing must not register product shortcuts/apps.' }
$runtimeBase = [Microsoft.Win32.RegistryKey]::OpenBaseKey('LocalMachine','Registry64')
$runtimeKey = $runtimeBase.OpenSubKey('SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64')
try {
    if (!$runtimeKey -or [version]($runtimeKey.GetValue('Version').TrimStart('v')) -lt [version]$metadata.RuntimeVersion) {
        throw 'Fixture testing requires the bundled runtime version already installed; it will not install system prerequisites.'
    }
} finally { if($runtimeKey){$runtimeKey.Dispose()};$runtimeBase.Dispose() }
$work = Join-Path $root ('_work\installer-tests\'+[Guid]::NewGuid().ToString('N'))
$work = [IO.Path]::GetFullPath($work)
if (!$work.StartsWith((Join-Path $root '_work\'),[StringComparison]::OrdinalIgnoreCase)) { throw 'Fixture outside workspace scratch' }
[IO.Directory]::CreateDirectory($work) | Out-Null
function Run-Setup([string]$Target, [string]$Label) {
    $log = Join-Path $work ($Label+'.log')
    $args = @('/VERYSILENT','/SUPPRESSMSGBOXES','/NORESTART','/SP-',('/DIR="'+$Target+'"'),('/LOG="'+$log+'"'))
    $p = Start-Process -FilePath $Setup -ArgumentList $args -WindowStyle Hidden -Wait -PassThru
    return $p.ExitCode
}
$invalid = Join-Path $work 'invalid'
[IO.Directory]::CreateDirectory($invalid) | Out-Null
$invalidCode = Run-Setup $invalid 'invalid'
if ($invalidCode -eq 0 -or (Test-Path -LiteralPath (Join-Path $invalid 'BlurInputFusion.dll'))) { throw 'Invalid game target accepted' }
$game = Join-Path $work 'game with spaces'
[IO.Directory]::CreateDirectory((Join-Path $game 'fx11')) | Out-Null
$sentinels = @{'Blur.exe'='original retail executable sentinel'; 'BlurRebuilt.exe'='previous rebuilt executable sentinel';
    'XInput1_4.dll'='unmanaged controller DLL sentinel'; 'fx11\effects.index'='previous effects index';
    'custom-settings.ini'='unmanaged user settings'}
foreach($name in $sentinels.Keys) { [IO.File]::WriteAllText((Join-Path $game $name),$sentinels[$name]) }
$locked = [IO.File]::Open((Join-Path $game 'Blur.exe'),'Open','Read','None')
try { $busyCode = Run-Setup $game 'busy' } finally { $locked.Dispose() }
if ($busyCode -eq 0 -or (Test-Path -LiteralPath (Join-Path $game 'BlurInputFusion.dll'))) { throw 'Busy game target accepted' }
# [Blur2PC] An idle read-only retail executable must remain installable.
$retail = Get-Item -LiteralPath (Join-Path $game 'Blur.exe')
$retail.IsReadOnly = $true
try { $installCode = Run-Setup $game 'install' } finally { $retail.IsReadOnly = $false }
if ($installCode -ne 0) { throw "Install failed: $installCode" }
if ((Get-FileHash -LiteralPath (Join-Path $game 'BlurRebuilt.exe')).Hash -ne $metadata.ExecutableSHA256) { throw 'Installed game hash mismatch' }
if ((Get-FileHash -LiteralPath (Join-Path $game 'BlurInputFusion.dll')).Hash -ne $metadata.InputFusionSHA256) { throw 'Installed backend hash mismatch' }
$managed = Get-Content -LiteralPath (Join-Path $game '_BlurRebuiltSetup/backup/managed-files.txt')
if ($managed.Count -ne $metadata.PayloadFiles) { throw 'Installed manifest count mismatch' }
foreach($name in $managed) { if(![IO.File]::Exists([IO.Path]::Combine($game,$name))){throw "Missing installed payload: $name"} }
# [Blur2PC] A missing/truncated mapping must fail before touching managed bytes.
$listPath = Join-Path $game '_BlurRebuiltSetup/backup/managed-files.txt'
$mappingBytes = [IO.File]::ReadAllBytes($listPath)
$beforeHash = (Get-FileHash -LiteralPath (Join-Path $game 'BlurRebuilt.exe')).Hash
try {
    [IO.File]::Move($listPath, $listPath+'.fixture')
    $missingCode = Run-Setup $game 'missing-map'
    [IO.File]::Move($listPath+'.fixture', $listPath)
    [IO.File]::WriteAllText($listPath, $managed[0]+[Environment]::NewLine)
    $truncatedCode = Run-Setup $game 'truncated-map'
} finally {
    [IO.File]::WriteAllBytes($listPath, $mappingBytes)
}
if ($missingCode -eq 0 -or $truncatedCode -eq 0) { throw 'Damaged backup accepted' }
if ((Get-FileHash -LiteralPath (Join-Path $game 'BlurRebuilt.exe')).Hash -ne $beforeHash) { throw 'Rejected upgrade changed game' }
$updateCode = Run-Setup $game 'update'
if ($updateCode -ne 0) { throw "Update failed: $updateCode" }
$uninstaller = Get-ChildItem -LiteralPath (Join-Path $game '_BlurRebuiltSetup') -Filter 'unins*.exe' | Select-Object -First 1
if (!$uninstaller) { throw 'Uninstaller missing' }
$uninstallArgs = @('/VERYSILENT','/SUPPRESSMSGBOXES','/NORESTART',('/LOG="'+(Join-Path $work 'uninstall.log')+'"'))
$p = Start-Process -FilePath $uninstaller.FullName -ArgumentList $uninstallArgs -WindowStyle Hidden -Wait -PassThru
if ($p.ExitCode -ne 0) { throw "Uninstall failed: $($p.ExitCode)" }
foreach($name in $sentinels.Keys) { if([IO.File]::ReadAllText((Join-Path $game $name)) -ne $sentinels[$name]){throw "Original file changed: $name"} }
if (Test-Path -LiteralPath (Join-Path $game 'BlurInputFusion.dll')) { throw 'New backend left after uninstall' }
$result = [ordered]@{InvalidTargetRejected=$true; BusyTargetRejected=$true; InstallExit=$installCode; UpdateExit=$updateCode;
    UninstallExit=$p.ExitCode; PayloadFiles=$managed.Count; GameAndBackendHashesMatch=$true; OriginalFilesRestored=$true;
    UnmanagedFilesPreserved=$true; ReadOnlyRetailAccepted=$true; DamagedMappingRejected=$true; SystemRuntimeExecution='not needed; installed version already satisfies bundled prerequisite';
    CleanWindowsTest='pending'; FixtureDirectory=$work}
$result | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $work 'result.json')
$result | ConvertTo-Json
