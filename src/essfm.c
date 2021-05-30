#include <windows.h>
#include "essfm.h"
#include "buttio.h"

#define WRITE_PORT_UCHAR(CONF, PORT, DATA)  (CONF)->pHandler.wu8(&(CONF)->pHandler, (CONF)->port + (PORT), (DATA))
#define READ_PORT_UCHAR(CONF, PORT,  PDATA) (CONF)->pHandler.ru8(&(CONF)->pHandler, (CONF)->port + (PORT), (PDATA))

//not very accurate
void QPCuWait(DWORD uSecTime) { //KeStallExecutionProcessor
    static LONGLONG freq=0;
    LONGLONG start=0, cur=0, wait=0;
    
    if (freq == 0) QueryPerformanceFrequency((PLARGE_INTEGER)&freq);
    if (freq != 0) {
        wait = ((LONGLONG)uSecTime * freq)/(LONGLONG)1000000;
        QueryPerformanceCounter((PLARGE_INTEGER)&start);
        while (cur < (start + wait)) {
            __asm__("pause");
            QueryPerformanceCounter((PLARGE_INTEGER)&cur);
        }
    } else {
        //TODO: alternate timing mechanism
    }
}

void synthInitNativeESFM(FmConfig* fmConf) {
    WRITE_PORT_UCHAR(fmConf, 0, 0x00);
    QPCuWait(25);
    WRITE_PORT_UCHAR(fmConf, 2, 0x05);
    QPCuWait(25);
    WRITE_PORT_UCHAR(fmConf, 1, 0x80);
    QPCuWait(25);
}

void synthSaveESFM(FmConfig* fmConf) {
    for (UINT i=0; i < FMREGLENGTH; i++) {
        WRITE_PORT_UCHAR(fmConf, 2, i&0xFF);
        QPCuWait(3);
        WRITE_PORT_UCHAR(fmConf, 3, i>>8);
        QPCuWait(3);
        READ_PORT_UCHAR(fmConf, 1, &(fmConf->registers[i]));
        QPCuWait(3);
        WRITE_PORT_UCHAR(fmConf, 1, 0x00);
        QPCuWait(3);
    }
}

void synthRestoreESFM(FmConfig* fmConf) {
    for (UINT i=0; i < FMREGLENGTH; i++) {
        WRITE_PORT_UCHAR(fmConf, 2, i&0xFF);
        QPCuWait(3);
        WRITE_PORT_UCHAR(fmConf, 3, i>>8);
        QPCuWait(3);
        WRITE_PORT_UCHAR(fmConf, 1, fmConf->registers[i]);
        QPCuWait(3);
    }
}

void synthMidiSendFM(FmConfig* fmConf, USHORT reg, UCHAR data) {
    USHORT offHigh = (reg < 0x100) ? 0 : 2;
    
    WRITE_PORT_UCHAR(fmConf, 0 + offHigh, reg&0xFF);
    QPCuWait(23);
    WRITE_PORT_UCHAR(fmConf, 1 + offHigh, data);
    QPCuWait(23);
}

/*BOOL synthPresent(PUCHAR base, PUCHAR inbase, BOOL *pIsFired) {
    UCHAR t1, t2;

    if (pIsFired) *pIsFired = FALSE;

    // check if the chip is present
    synthMidiSendFM(base, 4, 0x60);
    synthMidiSendFM(base, 4, 0x80);
    READ_PORT_UCHAR(fmConf, inbase, &t1);
    synthMidiSendFM(base, 2, 0xFF);
    synthMidiSendFM(base, 4, 0x21);
    QPCuWait(200);

    if (pIsFired && *pIsFired) return TRUE;

    READ_PORT_UCHAR(fmConf, inbase, &t);
    synthMidiSendFM(base, 4, 0x60);
    synthMidiSendFM(base, 4, 0x80);

    if (!((t1 & 0xE0) == 0) || !((t2 & 0xE0) == 0xC0)) return FALSE;

    return TRUE;
}*/

void synthMidiQuiet(FmConfig* fmConf) {
    synthMidiSendFM(fmConf, 0x105, 0x01);
    synthMidiSendFM(fmConf, 0x004, 0x60);
    synthMidiSendFM(fmConf, 0x104, 0x3F);
    synthMidiSendFM(fmConf, 0x008, 0x00);
    synthMidiSendFM(fmConf, 0x0BD, 0xC0);
    
    for (UINT i=0; i < 21; i++) {
        synthMidiSendFM(fmConf, 0x040 + i, 0x3F);
        synthMidiSendFM(fmConf, 0x140 + i, 0x3F);
    }
    
    for (UINT i=0; i < 8; i++) {
        synthMidiSendFM(fmConf, 0x0B0 + i, 0);
        synthMidiSendFM(fmConf, 0x1B0 + i, 0x00);
    }
}