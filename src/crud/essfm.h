#ifndef _ESSFM_H_
#define _ESSFM_H_
#include <windows.h>
#include <assert.h>
#include "buttio.h"

#define FMREGLENGTH 595

typedef struct {
    IOHandler ioHand;
    UCHAR* patches;
    UCHAR  registers[FMREGLENGTH];
    USHORT port;
} FmConfig;

void QPCuWait(DWORD uSecTime);


////////////////////////////////////////////////////////////////////////////////////////////
//AUDDRIVE.SYS
void synthInitNativeESFM(FmConfig* fmConf);
void synthSaveESFM(FmConfig* fmConf);
void synthRestoreESFM(FmConfig* fmConf);
void synthMidiSendFM(FmConfig* fmConf, USHORT address, UCHAR data);
//BOOL synthPresent(PUCHAR base, PUCHAR inbase, BOOL *pIsFired)
void synthMidiQuiet(FmConfig* fmConf);

////////////////////////////////////////////////////////////////////////////////////////////
//AUDDRIVE.DLL

#endif
