#include "esfm.h"

extern void __stdcall MidiMessage(DWORD dwData);
extern void __stdcall fmreset();
extern void __stdcall MidiAllNotesOff();

// Only used if ASM source
#ifndef _ESSFM_H_
USHORT fnum[] = {
    514, 544, 577, 611, 647, 686, 727, 770, 816, 864, 916, 970
};
BYTE gbVelocityAtten[] = {
    40, 36, 32, 28, 23, 21, 19, 17, 15, 14, 13, 12, 11,
    10, 9, 8, 7, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 1, 0,
    0, 0
};
DWORD td_adjust__setup_operator[] = {
    256, 242, 228, 215, 203, 192, 181, 171, 161, 152, 144, 136
};
BYTE pmask__MidiPitchBend[] = {
    16, 32, 64, 128, 0, 0, 0, 0
};

DWORD v1 = 0;
DWORD v2 = 0;
DWORD timer[2] = {0};

BYTE  voice_table[26 * (18+2)] = {0}; //TODO: struct
BYTE  gbVelLevel[16]           = {0};
BYTE  pan_mask[16]             = {0};
BYTE  gbChanAtten[16]          = {0};
USHORT giBend[16]               = {0};
BYTE  hold_table[16]           = {0};
BYTE  program_table[16]        = {0};
BYTE  gbChanVolume[16]         = {0};
BYTE  gbChanExpr[16]           = {0};
BYTE  gbChanBendRange[16]      = {0};
BYTE  byte_6BC09170[16+1]      = {0};

#endif

BYTE* gBankMem = 0;


















FunWriteCB pWriteCB = NULL;
FunDelayCB pDelayCB = NULL;

enum {
    P_STATUS,
    P_DATA,
    P_INDEX_LO,
    P_INDEX_HI,
};
int __stdcall fmwrite231(USHORT index, USHORT data) {
    pWriteCB(0x02, index);
    pWriteCB(0x03, index>>8); pDelayCB();
    pWriteCB(0x01, data); pDelayCB();
    //2 index
    //3 index>>8
    //1 data
    //dPrintfA("esfm a1:%X a2:%X\n", a1, a2);
    return 0;
}

int __stdcall fmwrite21(USHORT index, USHORT data) {
    pWriteCB(0x02, index); pDelayCB();
    pWriteCB(0x01, data); pDelayCB();
    //2 index
    //1 data
    //dPrintfA("esfm a1:%X a2:%X\n", a1, a2);
    return 0;
}







/*void synthInitNativeESFM() {
    pWriteCB(0x00, 0x00); pDelayCB();
    pWriteCB(0x02, 0x05); pDelayCB();
    pWriteCB(0x01, 0x80); pDelayCB();
}*/



void esfm_init(BYTE* pBank, FunWriteCB pfWr, FunDelayCB pfDly) {
    esfm_setBank(pBank);
    pWriteCB = pfWr;
    pDelayCB = pfDly;
    
    esfm_startupDevice();
    esfm_resetFM();
}

void esfm_setBank(BYTE* pBank) {
    gBankMem = pBank;
}

void esfm_resetFM() {
    fmreset();
}

void esfm_startupDevice() {
    //these are probably not right
    pWriteCB(0x04, 72); pDelayCB();
    pWriteCB(0x04, 72); pDelayCB();
    pWriteCB(0x05, 0); pDelayCB();
    pWriteCB(0x04, 127); pDelayCB();
    pWriteCB(0x04, 127); pDelayCB();
    pWriteCB(0x05, 0); pDelayCB();
    pWriteCB(0x04, 54); pDelayCB();
    pWriteCB(0x05, 119); pDelayCB(); //153
    pWriteCB(0x04, 107); pDelayCB();
    pWriteCB(0x05, 0); pDelayCB();
    pWriteCB(0x07, 66); pDelayCB();
    pWriteCB(0x02, 5); pDelayCB();
    pWriteCB(0x01, 128); pDelayCB();
}

void esfm_shutdownDevice() {
    pWriteCB(0x04, 72); pDelayCB();
    pWriteCB(0x04, 72); pDelayCB();
    pWriteCB(0x05, 16); pDelayCB();
    pWriteCB(0x07, 98); pDelayCB();
}

void esfm_midiShort(DWORD dwData) {
    MidiMessage(dwData);
}