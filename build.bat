@echo off

call getcomp.bat gcc12
set path=%path%;G:\p_files\rtdk\WDDK\7600\bin\x86

ml /coff /c src\esfmmidi.asm
move /y esfmmidi.obj bin\

::must not contain spaces!
set buttiolocation=C:\p_files\prog\_proj\CodeCocks\buttio

set opts=-std=c11 -mconsole -g -Wall -Wextra -DNONMMAP_FALLBACK -DASM_SRC
set linkinc=-I%buttiolocation%\src\ -L%buttiolocation%\bin\
set linkinc=%linkinc% -lwinmm -lbuttio
set compiles=bin\esfmmidi.obj src\main.c src\iodriver.c src\esfmmidi.c src\util.c
set errlog=.\essmidi_err.log
set out=.\bin\essmidi.exe

call :compile
call :checkerr




set opts=-std=c11 -mconsole -g -Wall -Wextra
set linkinc=
set compiles=src\tests.c src\util.c
set errlog=.\tests_err.log
set out=.\bin\tests.exe

call :compile
call :checkerr

exit /B 0


:compile
del %out%
gcc -o %out% %compiles% %opts% %linkinc% 2> %errlog%
exit /B 0

:checkerr
IF %ERRORLEVEL% NEQ 0 (
    echo oops!
    notepad %errlog%
    goto :end
)
for %%R in (%errlog%) do if %%~zR lss 1 del %errlog%
:end
exit /B 0
