@echo off

set src=%1
set dst=%APPDATA%\DialogueMan\sorcery3\main

echo Removing previous destination
rmdir /s /q "%dst%"

echo Creating Destination Dir
mkdir "%dst%"

echo Copying main .svn folders
mkdir "%dst%\.svn"
xcopy /s /h /e "%src%\.svn" "%dst%\.svn"

echo Copying Config files
copy "%src%\config.xtp" "%dst%\config.xtp"
copy "%src%\db.xtd" "%dst%\db.xtd" 

echo Creating Destination Schema Dir
mkdir "%dst%\schema"

echo Copying Schema files
xcopy /s /h /e "%src%\schema" "%dst%\schema" 

echo Copying Packs .svn folders
mkdir "%dst%\Packs\.svn"
xcopy /s /h /e "%src%\Packs\.svn" "%dst%\Packs\.svn"

echo Creating Destination Packs Dir
mkdir "%dst%\Packs"

shift
:copy_packs
if "%1" == "" goto end
echo Copying pack %1
copy "%src%\Packs\%1" "%dst%\Packs\%1"
shift
goto copy_packs   

:end
echo Done