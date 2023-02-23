#include "esfm.h"

typedef struct {
    uint8_t    flags1;
    uint8_t    field_1;
    uint16_t  timer;
    uint8_t    channel;
    uint8_t    bNote;
    uint8_t    rel_vel;
    uint8_t    field_7;
    uint16_t  detune[4];
    uint8_t    patch_flag;
    uint8_t    reg5[4];
    uint8_t    reg1[4];
    uint8_t    field_19;
} Voice;










void __stdcall hold_controller(uint8_t channel, uint8_t bVelocity);
void __stdcall fmreset();
void __stdcall NATV_CalcNewVolume(uint8_t bChannel);
void __stdcall find_voice(bool patch1617_allowed_voice1, bool patch1617_allowed_voice2, uint8_t bChannel, uint8_t bNote);
int  __stdcall steal_voice(int patch1617_allowed);
void __stdcall setup_operator(int offset, int bNote, int bVelocity, uint16_t reg, int fixed_pitch, int rel_velocity, int channel, int operator, int voicenr);
void __stdcall setup_voice(int voicenr, int offset, int channel, int bNote, int bVelocity);
void __stdcall MidiMessage(uint32_t dwData);


// Only used if ASM source
#ifndef _ESSFM_H_

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

uint32_t v1 = 0;
uint32_t v2 = 0;
uint16_t gwTimer = 0;

Voice    voice_table[18+2]        = {0};
uint8_t  gbVelLevel[16]           = {0};
uint8_t  pan_mask[16]             = {0};
uint8_t  gbChanAtten[16]          = {0};
uint16_t giBend[16]               = {0};
uint8_t  hold_table[16]           = {0};
uint8_t  program_table[16]        = {0};
uint8_t  gbChanVolume[16]         = {0};
uint8_t  gbChanExpr[16]           = {0};
uint8_t  gbChanBendRange[16]      = {0};
uint8_t  note_offs[16+1]          = {0};
uint32_t voice1, voice2;

#endif

uint8_t* gBankMem = 0;


















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


















void __stdcall voice_on(int voiceNr)
{
    if ( voiceNr >= 16 )
    {
        if ( voiceNr == 16 )
        {
            fmwrite231(0x250, 1);
            fmwrite231(0x251, 1);
        }
        else
        {
            fmwrite231(0x252, 1);
            fmwrite231(0x253, 1);
        }
    }
    else 
    {
        fmwrite231((uint16_t)voiceNr + 0x240, 1);
    }
}

void __stdcall voice_off(int voiceNr)
{
    if ( voiceNr >= 16 )
    {
        if ( voiceNr == 16 )
        {
          fmwrite231(0x250, 0);
          fmwrite231(0x251, 0);
        }
        else
        {
          fmwrite231(0x252, 0);
          fmwrite231(0x253, 0);
        }
    }
    else
    {
        fmwrite231((uint16_t)voiceNr + 0x240, 0);
    }
    voice_table[voiceNr].flags1 = 2;
    voice_table[voiceNr].timer = (uint16_t)gwTimer;
    gwTimer++;
}

/*
void __stdcall MidiMessage(uint32_t dwData) 
{
    uint8_t    bMsgType, bChannel, bVelocity, bNote;
    int     i;

    // D1("\nMidiMessage");
    bMsgType =    (uint8_t)((dwData & 0x000000F0));
    bChannel =    (uint8_t)((dwData & 0x0000000F) >> 0);
    bNote =       (uint8_t)((dwData & 0x00007F00) >> 8);
    bVelocity =   (uint8_t)((dwData & 0x007F0000) >> 16);
    
    switch (bMsgType) 
    {
        case 0x90:      // turn key on, or key off if volume == 0
            if (bVelocity)
            {
                note_on((uint8_t)dwData, bNote, bVelocity); 
            }
            else 
            {
                note_off((uint8_t)dwData, bNote);
            }
            break;
        case 0x80:      // turn key off
            note_off((uint8_t)dwData, bNote);
            break;
        case 0xb0:      // change control
            switch (bNote)
            {
                case 6:
                    if ( (hold_table[bChannel] & 6) == 6 )
                        gbChanBendRange[bChannel] = bVelocity;
                    break;
                case 7:
                    gbChanAtten[bChannel] = gbVelocityAtten[bVelocity >> 1];
                    gbChanVolume[bChannel] = bVelocity;
                    NATV_CalcNewVolume((uint8_t)dwData);
                    break;
                case 8:
                case 10:
                    if ( bVelocity <= 80 )
                    {
                      if ( bVelocity >= 48 )
                        pan_mask[bChannel] = 48;
                      else
                        pan_mask[bChannel] = 16;
                    }
                    else
                    {
                      pan_mask[bChannel] = 32;
                    }
                    break;
                case 11:
                    gbChanExpr[bChannel] = bVelocity;
                    NATV_CalcNewVolume((uint8_t)dwData);
                    break;
                case 64:
                    hold_controller(bChannel, bVelocity);
                    break;
                case 98:
                    hold_table[bChannel] &= ~2;
                    break;
                case 99:
                    hold_table[bChannel] &= ~4;                    
                    break;
                case 100:
                    if ( bVelocity == 0 )
                    {
                        hold_table[bChannel] |= 2;
                    }
                    else
                    {
                        hold_table[bChannel] &= ~2;
                    }
                    break;
                case 101:
                    if ( bVelocity == 0 )
                    {
                        hold_table[bChannel] |= 4;
                    }
                    else
                    {
                        hold_table[bChannel] &= ~4;
                    }
                    break;
                case 120:
                case 124:
                case 125:
                    for (i = 0; i < sizeof(voice_table)/sizeof(voice_table[0]); i++)
                    {
                        if ((voice_table[i].flags1 & 1) && voice_table[i].channel == bChannel)
                            voice_off(i);
                    }
                    break;
                case 121:
                    if (hold_table[bChannel] & 1)
                    {
                        for (i = 0; i < sizeof(voice_table)/sizeof(voice_table[0]); i++)
                        {
                            if ((voice_table[i].flags1 & 1) && voice_table[i].channel == bChannel && (voice_table[i].flags1 & 4))
                                voice_off(i);
                        }
                    }
                    hold_table[bChannel] &= ~1u;
                    gbChanVolume[bChannel] = 100;
                    gbChanExpr[bChannel] = 127;
                    giBend[bChannel] = 0x2000;
                    pan_mask[bChannel] = 48;
                    gbChanBendRange[bChannel] = 2;
                    break;
                case 123:
                case 126:
                case 127:
                    for (i = 0; i < sizeof(voice_table)/sizeof(voice_table[0]); i++)
                    {
                        if ((voice_table[i].flags1 & 1) && voice_table[i].channel == bChannel && (voice_table[i].flags1 & 4) == 0)
                            voice_off(i);
                    }
                    break;
            }
            break;
        case 0xc0:
            program_table[bChannel] = bNote;
            break;
        case 0xe0:      // pitch bend
            MidiPitchBend((uint8_t)dwData, bNote | (bVelocity << 7));
            break;
    }
}
*/

int16_t __stdcall NATV_CalcBend(uint16_t detune, int16_t iBend, uint16_t iBendRange);
/*int16_t __stdcall NATV_CalcBend(uint16_t detune, int16_t iBend, uint16_t iBendRange)
{
static uint16_t NATV_table1[64] = {
    1024, 1025, 1026, 1027, 1028, 1029, 1030, 1030, 1031, 1032,
    1033, 1034, 1035, 1036, 1037, 1038, 1039, 1040, 1041, 1042,
    1043, 1044, 1045, 1045, 1046, 1047, 1048, 1049, 1050, 1051,
    1052, 1053, 1054, 1055, 1056, 1057, 1058, 1059, 1060, 1061,
    1062, 1063, 1064, 1065, 1065, 1066, 1067, 1068, 1069, 1070,
    1071, 1072, 1073, 1074, 1075, 1076, 1077, 1078, 1079, 1080,
    1081, 1082, 1083, 1084,
};
static uint16_t NATV_table2[49] = {
    256,  271,  287,  304,  323,  342,  362,  384,  406,  431,
    456,  483,  512,  542,  575,  609,  645,  683,  724,  767,
    813,  861,  912,  967,  1024, 1085, 1149, 1218, 1290, 1367,
    1448, 1534, 1625, 1722, 1825, 1933, 2048, 2170, 2299, 2435,
    2580, 2734, 2896, 3069, 3251, 3444, 3649, 3866, 4096,
};
    //!WARN iBend is int16 in OPL midi driver sample
    if ( iBend == 0x2000 ) 
        return detune & 0xFF;
    else 
	{
        int v5;
        if ( iBend >= 0x3F80 ) iBend = 0x4000;
        v5 = (iBendRange * (iBend - 0x2000) >> 5) + 0x1800;
        return (detune * (uint16_t)((NATV_table1[(v5>>2)&0x3F] * NATV_table2[v5>>8]) >> 10) + 512) >> 10;
    }
}*/

uint32_t __stdcall MidiCalcFAndB(uint32_t bend, uint8_t block) {
    while (bend >= 1024) {
        bend >>= 1;
        block++;
    }
    if ( block > 7 ) block = 7;
    return bend  | (block << 10);
}

/*
void __stdcall note_on(uint8_t channel, uint8_t bNote, uint8_t bVelocity)
{
    int patch;
    int offset;
    uint8_t flags_voice1;
    int fixed_pitch;

    if ( channel == 9 )
        patch = bNote + 128;
    else
        patch = program_table[channel];
    offset = gBankMem[2 * patch] + (gBankMem[2 * patch + 1] << 8);
    if ( offset )
    {
        flags_voice1 = gBankMem[offset];
        fixed_pitch = (flags_voice1 >> 1) & 3;
        switch (fixed_pitch)
        {
        case 0:
            find_voice(flags_voice1 & 1, 0, channel, bNote);
            if ( voice1 == 255 ) voice1 = steal_voice(gBankMem[offset] & 1);
            setup_voice(voice1, offset, channel, bNote, bVelocity);
            voice_on(voice1);
            break;
        case 1:
            find_voice(flags_voice1 & 1, gBankMem[offset + 36] & 1, channel, bNote);
            if (voice1 == 255) voice1 = steal_voice(gBankMem[offset] & 1);
            setup_voice(voice1, offset, channel, bNote, bVelocity);
            if (voice2 != 255)
            {
                setup_voice(voice2, offset + 36, channel, bNote, bVelocity);
                voice_table[voice2].flags1 |= 8u;
                voice_on(voice2);
            }
            voice_on(voice1);
            break;
        case 2:
            find_voice(flags_voice1 & 1, gBankMem[offset + 36] & 1, channel, bNote);
            if ( voice1 == 255 )
            voice1 = steal_voice(gBankMem[offset] & 1);
            if ( voice2 == 255 )
            voice2 = steal_voice(gBankMem[offset + 36] & 1);
            setup_voice(voice1, offset, channel, bNote, bVelocity);
            setup_voice(voice2, offset + 36, channel, bNote, bVelocity);
            voice_on(voice1);
            voice_on(voice2);
            break;
        }
        gbVelLevel[channel] = bVelocity;
        if (note_offs[channel] == 255) note_offs[channel]=0; else note_offs[channel]++;
    }
}
*/

void __stdcall note_off(uint8_t bChannel, uint8_t bNote) {
    int i;
    
    for (i=0; i<18; i++) {
		Voice *voice = &voice_table[i];
        if ((voice->flags1 & 1) && voice->channel == bChannel && voice->bNote == bNote) {
            if ( hold_table[bChannel] & 1 ) {
                voice->flags1 |= 4;
            } else {
                voice_off(i);
            }
        }
    }
}

uint8_t __stdcall NATV_CalcVolume(uint8_t reg1, uint8_t rel_velocity, uint8_t channel) {
    uint8_t vol;

    if ( !gbChanVolume[channel] ) return 63;

    switch ( rel_velocity ) {
        case 0:
            vol = 0;
            break;
        case 1:
            vol = ((127 - gbChanExpr[channel]) >> 4 ) + ((127 - gbChanVolume[channel]) >> 4);
            break;
        case 2:
            vol = ((127 - gbChanExpr[channel]) >> 3) + ((127 - gbChanVolume[channel]) >> 3);
            break;
        case 3:
            vol = gbChanVolume[channel];
            
            if ( vol < 64 ) {
                vol = ((63 - vol) >> 1) + 16;
            } else {
                vol = (127 - vol) >> 2;
            }
            
            if ( gbChanExpr[channel] < 64 ) {
                vol += ((63 - gbChanExpr[channel]) >> 1) + 16;
            } else {
                vol += ((127 - gbChanExpr[channel]) >> 2);
            }
            break;
    }
    vol += (reg1 & 0x3F);          // ATTENUATION
    if ( vol > 63 ) vol = 63;
    return vol | (reg1 & 0xC0);      // KSL
}


//todo: shove it in after ASM decoupling
    /*static*/ uint8_t pmask_MidiPitchBend[] = {
        16, 32, 64, 128, 0, 0, 0, 0
    };
void __stdcall MidiPitchBend(uint8_t bChannel, uint16_t iBend) {

    
    uint32_t i, j;
    
    giBend[bChannel] = iBend;
    for (i=0; i < sizeof(voice_table)/sizeof(voice_table[0]); i++) {
		Voice *voice = &voice_table[i];
        
        if (voice->channel == bChannel && voice->flags1 & 1) {
            for (j=0; j < sizeof(voice->reg5); j++) {
                if ((pmask_MidiPitchBend[j] & voice->patch_flag) == 0) {
                    int16_t bnd;
                    
                    bnd = NATV_CalcBend(voice->detune[j], iBend, gbChanBendRange[bChannel]);
                    bnd = MidiCalcFAndB(bnd, (voice->reg5[j] >> 2) & 7);
                    fmwrite231(i*32 + j*8 + 5, (voice->reg5[j] & 0xE0) | (bnd>>8));
                    fmwrite231(i*32 + j*8 + 4, bnd & 0xFF);
                }
            }
        }
    }
}





