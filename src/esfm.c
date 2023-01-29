#include <stdint.h>
#include "esfm.h"

extern __stdcall void MidiMessage(uint32_t dwData);
extern __stdcall void fmreset();
extern __stdcall void MidiAllNotesOff();

uint16_t fnum[] = {
    514, 544, 577, 611, 647, 686, 727, 770, 816, 864, 916, 970
};
uint8_t gbVelocityAtten[] = {
    40, 36, 32, 28, 23, 21, 19, 17, 15, 14, 13, 12, 11,
    10, 9, 8, 7, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 1, 0,
    0, 0
};
uint32_t td_adjust__setup_operator[] = {
    256, 242, 228, 215, 203, 192, 181, 171, 161, 152, 144, 136
};
uint8_t pmask__MidiPitchBend[] = {
    16, 32, 64, 128, 0, 0, 0, 0
};

uint8_t* gBankMem = 0;
uint32_t v1 = 0;
uint32_t v2 = 0;
uint32_t timer[2] = {0};

uint8_t  voice_table[26 * (18+2)] = {0}; //TODO: struct
uint8_t  gbVelLevel[16]           = {0};
uint8_t  pan_mask[16]             = {0};
uint8_t  gbChanAtten[16]          = {0};
uint16_t giBend[16]               = {0};
uint8_t  hold_table[16]           = {0};
uint8_t  program_table[16]        = {0};
uint8_t  gbChanVolume[16]         = {0};
uint8_t  gbChanExpr[16]           = {0};
uint8_t  gbChanBendRange[16]      = {0};
uint8_t  byte_6BC09170[16+1]      = {0};






















FunWriteCB pWriteCB = NULL;
FunDelayCB pDelayCB = NULL;

enum {
    P_STATUS,
    P_DATA,
    P_INDEX_LO,
    P_INDEX_HI,
};
int __stdcall fmwrite231(uint16_t index, uint16_t data) {
    pWriteCB(0x02, index);
    pWriteCB(0x03, index>>8); pDelayCB();
    pWriteCB(0x01, data); pDelayCB();
    //2 index
    //3 index>>8
    //1 data
    //dPrintfA("esfm a1:%X a2:%X\n", a1, a2);
    return 0;
}

int __stdcall fmwrite21(uint16_t index, uint16_t data) {
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



void esfm_init(uint8_t* pBank, FunWriteCB pfWr, FunDelayCB pfDly) {
    esfm_setBank(pBank);
    pWriteCB = pfWr;
    pDelayCB = pfDly;
    
    esfm_startupDevice();
    esfm_resetFM();
}

void esfm_setBank(uint8_t* pBank) {
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

void esfm_midiShort(uint32_t dwData) {
    MidiMessage(dwData);
}