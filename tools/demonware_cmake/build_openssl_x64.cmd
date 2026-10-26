@echo off
rem [Blur2PC][x64] Build OpenSSL 0.9.8i as x64 static libraries for DemonWare's libcurl.
rem The tree carries only x86 libeay32/ssleay32 (code/vendor/demonware/2.0/external/
rem openssl-0.9.8i). DemonWare configured that build with "perl Configure VC-WIN32"
rem and no other options (readme.demonware.txt.txt); this is the same configuration
rem for VC-WIN64A. no-asm: 0.9.8i's x86_64 perlasm targets an old MASM dialect; the
rem C implementations compute the same results. /MD matches the game's DLL CRT.
rem
rem Usage: build_openssl_x64.cmd <extracted openssl-0.9.8i dir> <install dir>
rem Source: https://www.openssl.org/source/old/0.9.x/openssl-0.9.8i.tar.gz
rem         SHA1 b2e029cfb68bf32eae997d60317a40945db5a65f
setlocal
if "%~2"=="" (echo usage: %~nx0 ^<openssl-0.9.8i dir^> ^<install dir^> & exit /b 2)
set "SRC=%~f1"
set "OUT=%~f2"
rem Any installed Visual Studio edition (the release workflow runs on GitHub's images).
set "VCVARS="
for /f "usebackq delims=" %%V in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -find VC\Auxiliary\Build\vcvars64.bat`) do set "VCVARS=%%V"
if not defined VCVARS (echo vcvars64.bat not found & exit /b 1)
call "%VCVARS%" >nul || exit /b 1
rem OpenSSL's Configure and mk1mf need perl; Git for Windows ships one.
where perl >nul 2>nul || set "PATH=%PATH%;C:\Program Files\Git\usr\bin"
cd /d "%SRC%" || exit /b 1
rem Configure is perl and drops backslashes from the prefix (OPENSSLDIR).
set "OUTFWD=%OUT:\=/%"
perl Configure VC-WIN64A no-asm --prefix="%OUTFWD%" || exit /b 1
call ms\do_win64a.bat || exit /b 1
perl -pi -e "s#/MT #/MD #g; s#/MTd #/MDd #g; s#/WX ##g" ms\nt.mak || exit /b 1
nmake -f ms\nt.mak || exit /b 1
if not exist "%OUT%\lib" mkdir "%OUT%\lib"
if not exist "%OUT%\include\openssl" mkdir "%OUT%\include\openssl"
copy /y out32\libeay32.lib "%OUT%\lib\" >nul || exit /b 1
copy /y out32\ssleay32.lib "%OUT%\lib\" >nul || exit /b 1
copy /y inc32\openssl\*.h "%OUT%\include\openssl\" >nul || exit /b 1
echo OpenSSL 0.9.8i x64 installed to %OUT%
