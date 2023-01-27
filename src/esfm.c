#include <stdio.h>
#include <windows.h>
#include <assert.h>
#include "buttio.h"
#include "esfm.h"
#include "util.h"

extern BYTE* gBankMem;
extern __stdcall void MidiMessage(DWORD dwData);
extern __stdcall void fmreset();
extern __stdcall void MidiAllNotesOff();

static USHORT fmBase = 0;
static IOHandler ioHand = {0};

static InstrBank bankArr[] = {
        {"bnk_common.bin", "Most commonly distributed with OS drivers and games.", NULL},
        {"bnk_NT4.bin", "NT4 driver.", NULL},
        
        //{"?malloc", "Malloc unsanitized memory special dish.", NULL},  //will crash, instrument data not sanitized
};
static int curBank = 0;




//not very accurate
void QPCuWait(DWORD uSecTime) { //KeStallExecutionProcessor
    static LONGLONG freq=0;
    LONGLONG start=0, cur=0, wait=0;
    
    if (freq == 0) QueryPerformanceFrequency((PLARGE_INTEGER)&freq);
    if (freq != 0) {
        wait = ((LONGLONG)uSecTime * freq)/(LONGLONG)1000000;
        QueryPerformanceCounter((PLARGE_INTEGER)&start);
        while (cur < (start + wait)) {
            //__asm__("pause");
            QueryPerformanceCounter((PLARGE_INTEGER)&cur);
        }
    } else {
        //TODO: alternate timing mechanism
    }
}





enum {
    P_STATUS,
    P_DATA,
    P_INDEX_LO,
    P_INDEX_HI,
};
int __stdcall fmwrite231(USHORT index, USHORT data) {
    buttio_wu8(&ioHand, fmBase+2, index);
    //QPCuWait(25);
    buttio_wu8(&ioHand, fmBase+3, index>>8);
    QPCuWait(10);
    buttio_wu8(&ioHand, fmBase+1, data);
    QPCuWait(10);
    //2 index
    //3 index>>8
    //1 data
    //dPrintfA("esfm a1:%X a2:%X\n", a1, a2);
    return 0;
}

int __stdcall fmwrite21(USHORT index, USHORT data) {
    buttio_wu8(&ioHand, fmBase+2, index);
    QPCuWait(10);
    buttio_wu8(&ioHand, fmBase+1, data);
    QPCuWait(10);
    //2 index
    //1 data
    //dPrintfA("esfm a1:%X a2:%X\n", a1, a2);
    return 0;
}





#define IO_write8(PORT, DATA) buttio_wu8(&ioHand, fmBase+PORT, DATA); QPCuWait(10)
void FM_startSynth() {
    //these are probably not right
    IO_write8(0x04, 72);
    IO_write8(0x04, 72);
    IO_write8(0x05, 0);
    IO_write8(0x04, 127);
    IO_write8(0x04, 127);
    IO_write8(0x05, 0);
    IO_write8(0x04, 54);
    IO_write8(0x05, 119);//153
    IO_write8(0x04, 107);
    IO_write8(0x05, 0);
    IO_write8(0x07, 66);
    IO_write8(0x02, 5);
    IO_write8(0x01, 128);
}

void FM_stopSynth() {
    IO_write8(0x04, 72);
    IO_write8(0x04, 72);
    IO_write8(0x05, 16);
    IO_write8(0x07, 98);
}

/*void synthInitNativeESFM() {
    buttio_wu8(&ioHand, fmBase+0, 0x00);
    QPCuWait(25);
    buttio_wu8(&ioHand, fmBase+2, 0x05);
    QPCuWait(25);
    buttio_wu8(&ioHand, fmBase+1, 0x80);
    QPCuWait(25);
}*/



BOOL esfm_init(USHORT port) {
    assert(COUNTOF(bankArr) >= 1);
    for (int i=0; i < COUNTOF(bankArr); i++) {
        int size;
        
        if(bankArr[i].fileName[0] == '?') {
            if (!strcmp("?malloc", bankArr[i].fileName)) {
                bankArr[i].pData = malloc(BANKLEN);
                bankArr[i].pData != NULL;
            }
        } else if (loadFile(bankArr[i].fileName, &bankArr[i].pData, &size)) {
            assert(size == BANKLEN);
            assert(bankArr[i].pData != NULL);
        }
    }
    gBankMem = bankArr[0].pData;
    curBank = 0;
    
    if (!buttio_init(&ioHand, NULL, BUTTIO_MET_DRIVERCALL)) return FALSE;
    iopm_fillRange(&ioHand.iopm, port, port+0xF, TRUE);
    buttio_flushIOPMChanges(&ioHand);
    fmBase = port;
    
    FM_startSynth();
    fmreset();
    
    return TRUE;
}

InstrBank* esfm_switchBank() {
    if (COUNTOF(bankArr) == 1) return &bankArr[0];
    curBank = (curBank + 1) % COUNTOF(bankArr);
    gBankMem = bankArr[curBank].pData;
    
    return &bankArr[curBank];
}

void esfm_shutdown() {
    FM_stopSynth();
    buttio_shutdown(&ioHand);
}

void esfm_midiShort(DWORD dwData) {
    MidiMessage(dwData);
}

















