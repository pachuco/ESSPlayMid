#include <windows.h>
#include <stdio.h>

#include "util.h"

#define DPRINTF_BUF 2048
void dPrintfA(const CHAR* fmt, ...) {
    va_list args;
    char buf[DPRINTF_BUF];
    
    va_start(args, fmt);
    vsnprintf(buf, DPRINTF_BUF, fmt, args);
    va_end(args);
    OutputDebugStringA(buf);
}

void util_getParentPathA(char* inout) {
    int len = lstrlenA(inout);
    for (int i=1; i <= len; i++) {
        if (inout[len-i] == '\\') {
            inout[len-i+1] = '\0';
            break;
        }
    }
}