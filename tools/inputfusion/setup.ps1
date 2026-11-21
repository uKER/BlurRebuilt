# [Blur2PC] T272: fetch the exact unmodified backend verified with this integration.
[CmdletBinding()]
param([string]$ArchivePath)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version 2
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$version = '0.4.1'
$url = "https://github.com/xan105/InputFusion/releases/download/$version/InputFusion.7z"
$archiveHash = '3ea9ce7f9e74cbb71647dcc07d64fec5269aafb0fc5c40354ca568ab22884c9f'
$dllHash = '242e16a3a5904eb9068bd442c6a466fd983dbbe7851022704985dce3fb6ec408'
$downloads = Join-Path $root '_work\vendor_dl'
$extract = Join-Path $downloads "inputfusion-$version"
$output = Join-Path $root "_work\vendor\inputfusion-$version"
New-Item -ItemType Directory -Path $downloads,$extract,$output -Force | Out-Null
if (!$ArchivePath) {
    $ArchivePath = Join-Path $downloads "InputFusion-$version.7z"
    if (!(Test-Path -LiteralPath $ArchivePath)) {
        Invoke-WebRequest -Uri $url -OutFile $ArchivePath -UseBasicParsing
    }
}
$ArchivePath = (Resolve-Path -LiteralPath $ArchivePath).Path
if ((Get-FileHash -LiteralPath $ArchivePath -Algorithm SHA256).Hash -ne $archiveHash) {
    throw 'InputFusion archive hash differs from the pinned upstream release.'
}
& tar.exe -xf $ArchivePath -C $extract 'XInput/x64/xinput1_4.dll'
if ($LASTEXITCODE -ne 0) { throw 'InputFusion extraction failed; tar.exe with 7z support is required.' }
$dll = Join-Path $extract 'XInput\x64\xinput1_4.dll'
if ((Get-FileHash -LiteralPath $dll -Algorithm SHA256).Hash -ne $dllHash) {
    throw 'Extracted InputFusion DLL hash differs from the verified x64 release.'
}
Copy-Item -LiteralPath $dll -Destination (Join-Path $output 'BlurInputFusion.dll') -Force
[ordered]@{
    Version=$version
    SourceCommit='e3e6e7f97f8e7b3927a09a4649c2ec3bef9ef956'
    ReleaseUrl=$url
    ArchiveSHA256=$archiveHash
    DllSHA256=$dllHash
} | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $output 'provenance.json') -Encoding UTF8
Write-Output "Verified InputFusion $version x64: $output"
