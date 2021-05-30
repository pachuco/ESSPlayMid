@echo off
set COMPILER_PATH=
call getcomp rosbe

set opts=-std=c99 -mconsole -Wall -Wextra -Os -s ^
  -nostartfiles -nodefaultlibs -nostdlib -Wl,-shared ^
  -Wl,--enable-stdcall-fixup ^
  -Wl,--entry,_DriverEntry ^
  -Wl,--subsystem,native ^
  -Wl,--image-base,0x10000 ^
  -Wl,--strip-all ^
  -DCODEISDRIVER

::uncomment the other for mingw fix.
set linkinc=-I=/i686-w64-mingw32/include/ddk
::set linkinc=-I%COMPILER_PATH%/../i686-w64-mingw32/include/ddk

set linkinc=%linkinc% -lntoskrnl -lhal
set compiles=.\src\buttio_krn.c .\src\buttio_common.c
set errlog=.\buttio_err.log

del .\bin\buttio.sys
gcc -o .\bin\buttio.sys %compiles% %opts% %linkinc% 2> %errlog%

IF %ERRORLEVEL% NEQ 0 (
    echo oops!
    notepad %errlog%
    goto :end
)
for %%R in (%errlog%) do if %%~zR lss 1 del %errlog%
:end