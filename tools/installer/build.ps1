[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$Stage,
    [Parameter(Mandatory=$true)][ValidatePattern('^[A-Za-z0-9][A-Za-z0-9._-]{0,63}$')][string]$Version,
    [string]$OutputDirectory, [string]$InnoCompiler, [string]$RedistPath, [string]$EffectsDirectory,
    [switch]$InstallerTest
)
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$Stage = (Resolve-Path -LiteralPath $Stage).Path
if (!$OutputDirectory) { $OutputDirectory = Join-Path $root '_work\installer' }
New-Item -ItemType Directory -Force $OutputDirectory | Out-Null
$OutputDirectory = (Resolve-Path -LiteralPath $OutputDirectory).Path
$work = Join-Path $root ('_work\installer\'+[Guid]::NewGuid().ToString('N'))
$payload = Join-Path $work 'payload'
New-Item -ItemType Directory -Force $payload | Out-Null
$files = @('BlurRebuilt.exe','AmaxClientHost.exe','BlurInputFusion.dll','README.md',
    'InputFusion-Licenses\NOTICE.txt','InputFusion-Licenses\LGPL-3.0.txt','InputFusion-Licenses\GPL-3.0.txt',
    'InputFusion-Licenses\SDL-LICENSE.txt','InputFusion-Licenses\Detours-LICENSE.md',
    'docs\blur1pcdx11\additions.md','docs\blur1pcdx11\contribution-verification.md')
$effects = if ($EffectsDirectory) { (Resolve-Path -LiteralPath $EffectsDirectory).Path } else { Join-Path $Stage 'fx11' }
if (!(Test-Path -LiteralPath (Join-Path $effects 'effects.index'))) { throw 'Missing packaged effects.index' }
$files += @([IO.Directory]::EnumerateFiles($effects, '*', [IO.SearchOption]::AllDirectories) | Where-Object {
    [IO.Path]::GetFileName($_) -in @('effects.index','effects.samplers') -or [IO.Path]::GetExtension($_) -eq '.fxo'
} | ForEach-Object { 'fx11\'+$_.Substring($effects.Length+1) })
$uniquePayloadPaths = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
foreach ($name in $files) {
    if (!$uniquePayloadPaths.Add($name)) { throw 'Duplicate package-relative path' }
    if ($name -match '(^[\\/]|:|\.\.)') { throw 'Invalid package-relative path' }
    if ($name -match '[^\x20-\x7e]') { throw 'Payload manifest requires ASCII relative filenames' }
    $source = if ($name.StartsWith('fx11\')) { [IO.Path]::Combine($effects, $name.Substring(5)) } else { [IO.Path]::Combine($Stage, $name) }
    if (![IO.File]::Exists($source)) { throw "Missing packaged file: $name" }
    $target = [IO.Path]::Combine($payload, $name)
    [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($target)) | Out-Null
    [IO.File]::Copy($source, $target, $false)
}
if ((Get-FileHash -LiteralPath (Join-Path $payload 'BlurInputFusion.dll')).Hash -ne '242E16A3A5904EB9068BD442C6A466FD983DBBE7851022704985DCE3FB6EC408') { throw 'Backend hash mismatch' }
$manifest = Join-Path $work 'payload-files.txt'
$files | Set-Content -LiteralPath $manifest -Encoding ascii
$dependencies = & (Join-Path $PSScriptRoot 'prepare.ps1') -InnoCompiler $InnoCompiler -RedistPath $RedistPath
$name = 'BlurRebuilt-'+$Version+'-Setup'
if ($InstallerTest) { $name += '-Fixture' }
$compilerArgs = @('/Qp',('/DPayloadDir='+$payload),('/DManifestFile='+$manifest),
    ('/DOutputDir='+$OutputDirectory),('/DSetupName='+$name),('/DReleaseVersion='+$Version),
    ('/DRedistFile='+$dependencies.Redist),('/DRuntimeVersion='+$dependencies.RuntimeVersion))
if ($InstallerTest) { $compilerArgs += '/DInstallerTest=1' }
$compilerArgs += (Join-Path $PSScriptRoot 'setup.iss')
& $dependencies.Compiler @compilerArgs
if ($LASTEXITCODE -ne 0) { throw "Installer compilation failed: $LASTEXITCODE" }
$output = Join-Path $OutputDirectory ($name+'.exe')
[ordered]@{Version=$Version; PayloadFiles=$files.Count; ExecutableSHA256=(Get-FileHash -LiteralPath (Join-Path $payload 'BlurRebuilt.exe')).Hash;
    InputFusionSHA256=(Get-FileHash -LiteralPath (Join-Path $payload 'BlurInputFusion.dll')).Hash;
    RuntimeVersion=$dependencies.RuntimeVersion; RuntimeSHA256=(Get-FileHash -LiteralPath $dependencies.Redist).Hash;
    CompilerSHA256=(Get-FileHash -LiteralPath $dependencies.Compiler).Hash; InstallerSHA256=(Get-FileHash -LiteralPath $output).Hash;
    Fixture=$InstallerTest.IsPresent} | ConvertTo-Json | Set-Content -LiteralPath ($output+'.json')
Write-Output "Built $output"
