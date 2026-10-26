rem @echo off
rem make_jobqueue_module <self file name> <obj file name> <sources...>
setlocal
echo SPU module: %1


rem Configure paths.
SET CELL_SDK=c:\usr\local\cell
SET TL=.\source

rem Override SPU_IMAGE_DIR with your project's file server root directory to enable runtime module reloading.
if "%SPU_IMAGE_DIR%" == "" set SPU_IMAGE_DIR=%TL%\testbed

rem Choose option based on module type.
if "%~n1" == "jobqueue_kernel_spu" (
set ENTRY=%TL%\scheduler\entry_pm.c 
set MODULE_OPTS=-T%TL%\scheduler\entry_pm.ld -nostartfiles
set OPTIMIZATION_OPTS=-Os -mbranch-hints 
echo KERNEL
) else (
set ENTRY=%TL%\scheduler\entry_job.c
set MODULE_OPTS=-T%TL%\scheduler\entry_job.ld -nostartfiles
set OPTIMIZATION_OPTS=-O3 -ffast-math -mvector-simplify -mbranch-hints -fif-conversion --param inline-unit-growth=10000 --param large-function-growth=10000 --param max-inline-insns-single=10000
)

rem Set compiler options.
set GENERAL_OPTS=-g -fno-exceptions -x c++ -fno-rtti -DSPURS_JOB -DTL_PLAT_PS3_SPU -DPLATFORM_SPU -DTL_DISABLE_CONFIG_TESTS -fno-zero-initialized-in-bss
set DEAD_STRIP_OPTS=-fdata-sections -ffunction-sections -Wl,--gc-sections
set LIB_OPTS=-lspurs -ldma -latomic -lsync -lsheap
set INCLUDE_OPTS=-I%TL%\base\include -I%TL%\base\source -I%TL%\math2\include -I%TL%\jobqueue\include -I%TL%\ngl\tools\p3x\libp3x "-I%SN_PS3_PATH%\spu\include" -I%TL%\physics\include -I%TL%\physics\include\collision -I%TL%\physics\include\rbc_defs -I%TL%\physics\include\constraint_solver -I%TL%\physics\source  -I%TL%\nal\include -I%TL%\apk\include -I%TL%\..\include -I%TL% -I%TL%\..\..\core\include
set DEPENDENCY_OPTS=-Wp,-MP,-MT,%2,-MMD,%2.d

set PROJECT_OPTS=-DPLATFORM_SPU -DNGLGCM_DEVICE_VALIDATION=0 -DTARGET_PS3 -DDEBUG -D_DEBUG -DBUILD_DEBUG -DNGL_PS3 -DNO_SERIAL_IN -DNO_SERIAL_OUT -DNO_CHUNKFILE -I%TL%\obj\gen_src -I%TL% -I%TL%\ngl\include -DBUILD_CONFIG_RELEASE

set MAP_OPTS=-Wl,-Map,%~dpn1.map 
rem  -save-temps 

rem Set a stack size of 8k.
set STACK_OPTS=-fstack-limit-symbol=__stack_limit -Wl,--defsym,__stack_limit=0x3e000

rem To add additional command line options (like include directories), create a file MyModule.spu.opts and add arguments to it.
set CUSTOM_OPTS=
if exist %~dpn1.opts set CUSTOM_OPTS=@%~dpn1.opts

set ASSERT=%TL%\..\source\scheduler\tl_ps3_spu_assert.cpp
set PROFILER=%TL%\debug\profilerSPU.cpp
set JOBQUEUE=%TL%\scheduler\jobqueue_mem.cpp

set ALL_OPTS=%MODULE_OPTS% %DEAD_STRIP_OPTS% %STACK_OPTS% %CUSTOM_OPTS% %MAP_OPTS%

set CPP_OPTS=%GENERAL_OPTS% %OPTIMIZATION_OPTS% %CUSTOM_OPTS% %DEPENDENCY_OPTS% %INCLUDE_OPTS% %PROJECT_OPTS%

%CELL_SDK%\host-win32\spu\bin\spu-lv2-gcc %CPP_OPTS% -c %ASSERT% -o assert.o 
%CELL_SDK%\host-win32\spu\bin\spu-lv2-gcc %CPP_OPTS% -c %PROFILER% -o profiler.o
%CELL_SDK%\host-win32\spu\bin\spu-lv2-gcc %CPP_OPTS% -c %JOBQUEUE% -o jobqueue.o
%CELL_SDK%\host-win32\spu\bin\spu-lv2-gcc %CPP_OPTS% -c %ENTRY% -o entry.o
%CELL_SDK%\host-win32\spu\bin\spu-lv2-gcc %CPP_OPTS% -c %3% -o user1.o

%CELL_SDK%\host-win32\spu\bin\spu-lv2-gcc %ALL_OPTS% -o %~dpn1.elf entry.o profiler.o assert.o jobqueue.o user1.o %4 %5 %6 %7 %8 %9 %LIB_OPTS% 

if %ERRORLEVEL% neq 0 goto error

rem Run SN module processing utility to insert debugging GUID.
"%SN_PS3_PATH%\bin\spumodgen" -s %~dpn1.elf

rem Sign the elf file.
%CELL_SDK%\host-win32\bin\make_fself %~dpn1.elf %1

rem Extract elf to binary file.
%CELL_SDK%\host-win32\spu\bin\spu-lv2-objcopy -O binary %~dpn1.elf %~n1.elf.bin

rem Convert binary file to PPU object file.
set OBJCOPY_OPTS=-I binary -O elf64-powerpc-celloslv2 -B powerpc --set-section-align .data=7 --set-section-pad .data=128 --rename-section .data=.bin_objs.spu/%~n1.elf.bin
%CELL_SDK%\host-win32\ppu\bin\ppu-lv2-objcopy %OBJCOPY_OPTS% %~n1.elf.bin %2

rem Cleanup temp files.
move %~n1.elf.bin %SPU_IMAGE_DIR%
goto exit

:error
echo Error compiling SPU module.  Deleting temporary files.
del %~dpn1.elf
del %1
del %2

:exit
endlocal
