#include <windows.h>
#include <stdio.h>
#ifdef _MSC_VER
#define vsnprintf _vsnprintf
#endif

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
    int i, len = lstrlenA(inout);
    for (i=1; i <= len; i++) {
        if (inout[len-i] == '\\') {
            inout[len-i+1] = '\0';
            break;
        }
    }
}

int getFileSize(FILE* f) {
    int origPos = ftell(f);
    int size;
    
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, origPos, SEEK_SET);
    return size;
}

BOOL loadFile(PCHAR path, BYTE** ppOut, int* pSizeOut) {
    FILE* fin = NULL;
    BYTE* pDat = 0;
    int size = -1;
    
    if (!(fin = fopen(path, "rb"))) goto lErr;
    size = getFileSize(fin);
    
    if (!(pDat = malloc(size))) goto lErr;
    fread(pDat, size, 1, fin);
    
    *ppOut = pDat;
    *pSizeOut = size;
    fclose(fin);
    return TRUE;
    
    lErr:
    if (fin)  fclose(fin);
    if (pDat) free(pDat);
    *ppOut = NULL;
    *pSizeOut = -1;
    return FALSE;
}