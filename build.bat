@echo off

call getcomp.bat gcc12
set path=%path%;G:\p_files\rtdk\WDDK\7600\bin\x86

ml /coff /c src\esfm.asm
move /y esfm.obj bin\

::must not contain spaces!
set buttiolocation=C:\p_files\prog\_proj\CodeCocks\buttio

set opts=-std=c11 -mconsole -Os -s -Wall -Wextra -DNONMMAP_FALLBACK
set linkinc=-I%buttiolocation%\src\ -L%buttiolocation%\bin\
set linkinc=%linkinc% -lwinmm -lbuttio
set compiles=bin\esfm.obj src\main.c src\esfm.c src\util.c
set errlog=.\essmidi_err.log

rem xcopy "%buttiolocation%\bin\buttio.sys" .\bin\ /c /Y
del .\bin\essmidi.exe
gcc -o .\bin\essmidi.exe %compiles% %opts% %linkinc% 2> %errlog%

IF %ERRORLEVEL% NEQ 0 (
    echo oops!
    notepad %errlog%
    goto :end
)
for %%R in (%errlog%) do if %%~zR lss 1 del %errlog%
:end