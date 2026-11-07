[CmdletBinding()]
param([string]$InnoCompiler, [string]$RedistPath)
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$vendor = Join-Path $root '_work\vendor\installer'
New-Item -ItemType Directory -Force $vendor | Out-Null
function Assert-Signer([string]$Path, [string]$Publisher) {
    $sig = Get-AuthenticodeSignature -LiteralPath $Path
    if ($sig.Status -ne 'Valid' -or $sig.SignerCertificate.Subject -notmatch $Publisher) {
        throw "Untrusted installer dependency: $Path"
    }
}
if (!$InnoCompiler) {
    $compilerDir = Join-Path $vendor 'inno-7.1.1'
    $InnoCompiler = Join-Path $compilerDir 'ISCC.exe'
    if (!(Test-Path -LiteralPath $InnoCompiler)) {
        $download = Join-Path $vendor 'innosetup-7.1.1-x64.exe'
        Invoke-WebRequest 'https://github.com/jrsoftware/issrc/releases/download/is-7_1_1/innosetup-7.1.1-x64.exe' -OutFile $download -UseBasicParsing
        if ((Get-FileHash -LiteralPath $download).Hash -ne '8A5A6711C3766A6E5FB74F280BA0E3551E719CE94538D901FDBD1F8F9920B3F0') { throw 'Inno Setup hash mismatch' }
        Assert-Signer $download 'Pyrsys B\.V\.'
        $compilerSetupArgs = @('/PORTABLE=1','/VERYSILENT','/SUPPRESSMSGBOXES','/NORESTART','/SP-','/CURRENTUSER','/NOICONS',('/DIR="'+$compilerDir+'"'))
        $process = Start-Process -FilePath $download -ArgumentList $compilerSetupArgs -WindowStyle Hidden -Wait -PassThru
        if ($process.ExitCode -ne 0) { throw "Portable compiler preparation failed: $($process.ExitCode)" }
    }
}
$InnoCompiler = (Resolve-Path -LiteralPath $InnoCompiler).Path
Assert-Signer $InnoCompiler 'Pyrsys B\.V\.'
if (!$RedistPath) {
    $RedistPath = Join-Path $vendor 'vc_redist.x64.exe'
    if (!(Test-Path -LiteralPath $RedistPath)) {
        Invoke-WebRequest 'https://aka.ms/vc14/vc_redist.x64.exe' -OutFile $RedistPath -UseBasicParsing
    }
}
$RedistPath = (Resolve-Path -LiteralPath $RedistPath).Path
Assert-Signer $RedistPath 'Microsoft Corporation'
if ((Get-Item -LiteralPath $RedistPath).VersionInfo.ProductName -notmatch 'Visual C\+\+.*\(x64\)') { throw 'Expected the Microsoft x64 C++ redistributable' }
$runtimeVersion = (Get-Item -LiteralPath $RedistPath).VersionInfo.FileVersion
if ($runtimeVersion -notmatch '^\d+\.\d+\.\d+\.\d+$') { throw 'Cannot determine C++ runtime version' }
[pscustomobject]@{Compiler=$InnoCompiler; Redist=$RedistPath; RuntimeVersion=$runtimeVersion}
