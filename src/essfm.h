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
typedef struct {
    BYTE    flags;
    BYTE    field_1;
    USHORT  timer;
    BYTE    channel;
    BYTE    field_5;
    BYTE    field_6;
    BYTE    field_7;
    USHORT  field_8;
    BYTE    field_A;
    BYTE    field_B;
    BYTE    field_C;
    BYTE    field_D;
    BYTE    field_E;
    BYTE    field_F;
    BYTE    field_10;
    BYTE    field_11[4];
    BYTE    field_15;
    BYTE    field_16;
    BYTE    field_17;
    BYTE    field_18;
    BYTE    field_19;
} Voice;
static_assert(sizeof(Voice) == 26, "");