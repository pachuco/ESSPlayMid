@echo off

call getcomp.bat rosbe

set opts=-std=c99 -mconsole -Os -s -Wall -Wextra -DNONMMAP_FALLBACK
set linkinc=
set compiles=src\main.c src\essfm.c src\util.c src\buttio_usr.c src\buttio_common.c
set errlog=.\essmidi_err.log

del .\bin\essmidi.exe
gcc -o .\bin\essmidi.exe %compiles% %opts% %linkinc% 2> %errlog%

IF %ERRORLEVEL% NEQ 0 (
    echo oops!
    notepad %errlog%
    goto :end
)
for %%R in (%errlog%) do if %%~zR lss 1 del %errlog%
:end