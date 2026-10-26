call mk_common

set COMP="fxc"

set DEFINES=%DEFINES% /D PLATFORM_PC

set ODIR=%BIZARRE_ROOT%\MercuryGame\Working\PC\Data\Image\Shaders
set CDIR=%BIZARRE_ROOT%\MercuryGame\image\PC\shaders

mkdir %ODIR%
mkdir %CDIR%
