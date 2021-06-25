#include <windows.h>
#include <assert.h>
#include "essfm.h"
#include "buttio.h"

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

////////////////////////////////////////////////////////////////////////////////////////////
//AUDDRIVE.SYS

#define READ_PORT_UCHAR(CONF, PORT,  PDATA) buttio_ru8(&(CONF)->ioHand, (CONF)->port + (PORT), PDATA)
#define WRITE_PORT_UCHAR(CONF, PORT, DATA)  buttio_wu8(&(CONF)->ioHand, (CONF)->port + (PORT), DATA)

char FMRegs[608] = {0}; //!WARN FMREGLENGTH is 595
SHORT Address_SynthMidiData[2] = {0};


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

/*BOOL synthPresent(FmConfig* fmConf, PUCHAR inbase, BOOL *pIsFired) {
    //!WARN check inbase usage
    UCHAR t1, t2;

    if (pIsFired) *pIsFired = FALSE;

    // check if the chip is present
    synthMidiSendFM(fmConf, 4, 0x60);
    synthMidiSendFM(fmConf, 4, 0x80);
    READ_PORT_UCHAR(fmConf, inbase, &t1);
    synthMidiSendFM(fmConf, 2, 0xFF);
    synthMidiSendFM(fmConf, 4, 0x21);
    QPCuWait(200);

    if (pIsFired && *pIsFired) return TRUE;

    READ_PORT_UCHAR(fmConf, inbase, &t);
    synthMidiSendFM(fmConf, 4, 0x60);
    synthMidiSendFM(fmConf, 4, 0x80);

    if (!((t1 & 0xE0) == 0) || !((t2 & 0xE0) == 0xC0)) return FALSE;

    return TRUE;
}*/

/*BOOL SynthMidiIsOpl3(FmConfig* fmConf, BOOLEAN *isFired) {
  BOOL isOpl3 = 0;

  SynthMidiSendFM(pHw->SynthBase, 0x105, 0);
  QPCuWait(20);
  if (SynthPresent(base + 2, base, isFired) ) {
    SynthMidiSendFM(base, 0x105, 1);
    QPCuWait(20);
    if (!SynthPresent(base + 2, base, isFired) ) isOpl3 = 1;
  }
  SynthMidiSendFM(base, 0x105, 0);
  QPCuWait(20);
  return isOpl3;
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

void SynthMidiData(FmConfig* fmConf, USHORT address, BYTE data) {
    //!WARN driver MajorFunction

    //assert(v8 & 3);
    assert(address < 4);
    
    WRITE_PORT_UCHAR(fmConf, address, data);
    if ( address == 2 ) {
        Address_SynthMidiData[0] = data;
    } else if ( address == 3 ){
        Address_SynthMidiData[0] |= ((SHORT)data)<<8; //!WARN suspicious bitwise assign
    } else {
        FMRegs[Address_SynthMidiData[0]] = data;
    }
    //QPCuWait(23); //OPL2
    QPCuWait(10); //OPL3, etc.
}

////////////////////////////////////////////////////////////////////////////////////////////
//AUDDRIVE.DLL

USHORT NATV_table1[64] = {
    1024, 1025, 1026, 1027, 1028, 1029, 1030, 1030, 1031, 1032,
    1033, 1034, 1035, 1036, 1037, 1038, 1039, 1040, 1041, 1042,
    1043, 1044, 1045, 1045, 1046, 1047, 1048, 1049, 1050, 1051,
    1052, 1053, 1054, 1055, 1056, 1057, 1058, 1059, 1060, 1061,
    1062, 1063, 1064, 1065, 1065, 1066, 1067, 1068, 1069, 1070,
    1071, 1072, 1073, 1074, 1075, 1076, 1077, 1078, 1079, 1080,
    1081, 1082, 1083, 1084,
};
USHORT NATV_table2[49] = {
    256,  271,  287,  304,  323,  342,  362,  384,  406,  431,
    456,  483,  512,  542,  575,  609,  645,  683,  724,  767,
    813,  861,  912,  967,  1024, 1085, 1149, 1218, 1290, 1367,
    1448, 1534, 1625, 1722, 1825, 1933, 2048, 2170, 2299, 2435,
    2580, 2734, 2896, 3069, 3251, 3444, 3649, 3866, 4096,
};
char  pmask_MidiPitchBend[8] = {
    16, 32, 64, 128,
    0,  0,  0,  0
};
int td_adjust_setup_operator[12] = {
    256, 242, 228, 215, 203, 192,
    181, 171, 161, 152, 144, 136
};
char gbVelocityAtten[32] = {
    40, 36, 32, 28, 23, 21, 19, 17,
    15, 14, 13, 12, 11, 10, 9,  8,
    7,  6,  5,  5,  4,  4,  3,  3,
    2,  2,  1,  1,  1,  0,  0,  0
};
SHORT fnum[12] = {
    514, 544, 577, 611,
    647, 686, 727, 770,
    816, 864, 916, 970
};

BYTE  pan_mask[16]          = {0};
BYTE  gbVelLevel[16]        = {0};
BYTE  gbChanAtten[16]       = {0};
SHORT giBend[16]            = {0};
BYTE  hold_table[16]        = {0};
BYTE  gbChanVolume[16]      = {0};
BYTE  program_table[16]     = {0};
BYTE  gbChanExpr[16]        = {0};
BYTE  gbChanBendRange[16]   = {0};
//!HINT size: MidiPitchBend(), 26bytes * 18
//!WARN might be bigger(20 voices; melodic + perc)
Voice voice_table[18]       = {0};
WORD  DeviceData[162]       = {0};

UINT  MidiPosition = 0;





SHORT NATV_CalcBend(USHORT a1, USHORT iBend, USHORT a3) {
    //!WARN iBend is int16 in OPL midi driver sample
    if ( iBend == 0x2000 ) {
        return a1 & 0xFF;
    } else {
        if ( iBend >= 0x3F80 ) iBend = 0x4000;
        int v5 = (a3 * (iBend - 0x2000) >> 5) + 0x1800;
        return (a1 * (USHORT)((NATV_table1[(v5>>2)&0x3F] * NATV_table2[v5>>8]) >> 10) + 512) >> 10;
    }
}

UINT MidiCalcFAndB(UINT a1, BYTE a2) {
    while (a1 >= 0x400) {
        a1 >>= 1;
        a2++;
    }
    if ( a2 > 7 ) a2 = 7;
    return a1 | (a2 << 10);
}

void MidiFlush() {
  int numberOfBytesWritten; // ecx@0 MAPDST

  //if ( MidiPosition )
  //  WriteFile(MidiDeviceHandle, DeviceData, 4 * MidiPosition, &numberOfBytesWritten, &WriteOverlapped_S9345);
  MidiPosition = 0;
}

void fmwrite(USHORT a1, USHORT a2) {
  int pos; // eax@3

  if (MidiPosition == 81) MidiFlush();
  
  pos = 2 * MidiPosition;
  DeviceData[pos+0] = 0x38A;
  DeviceData[pos+1] = a1 & 0xFF;
  DeviceData[pos+2] = 0x38B;
  DeviceData[pos+3] = a1 >> 8;
  DeviceData[pos+4] = 0x389;
  DeviceData[pos+5] = a2;
  
  MidiPosition += 3;
  //return pos * 2;
}

void note_off(unsigned __int8 bChannel, char a2) {
    for (UINT i=0; i<18; i++) {
        Voice *voice = &voice_table[i];
        
        if ((voice->flags & 1) && voice->channel == bChannel && voice->field_5 == a2 ) {
            if ( hold_table_S9326[bChannel] & 1 ) {
                voice->flags |= 4;
            } else {
                voice_off(i);
            }
        }
    }
}

void hold_controller(int channel, signed int a3) {
    if ( a3 < 64 ) {
        hold_table[channel] &= 0xFE;
        
        for (UINT i=0; i<18; i++) {
            Voice* voice = &voice_table[i];
            
            if ( voice->flags & 4 ) {
                if ( voice->channel == channel ) voice_off(i);
            }
        }
    } else {
        hold_table[channel] |= 1;
    }
}

void voice_on(signed int voiceNr) {
    if ( voiceNr >= 16 ) {
        if ( voiceNr == 16 ) {
            fmwrite(0x250u, 1u);
            fmwrite(0x251u, 1u);
        } else {
            fmwrite(0x252u, 1u);
            fmwrite(0x253u, 1u);
        }
    } else {
        fmwrite(voiceNr + 0x240, 1u);
    }
}

//notable function list
/*
    MidiAllNotesOff
MidiCalcFAndB
    MidiClose
MidiFlush
    MidiInit
    MidiMessage
    MidiOpen
    MidiOpenDevice
    MidiPitchBend
    //MidiReset->fmreset
NATV_CalcBend
    NATV_CalcNewVolume
    NATV_CalcVolume
        find_voice
    fmreset
fmwrite
hold_controller
    midiSynthCallback
    modSynthMessage
note_off
        note_on
        setup_operator
        setup_voice
        steal_voice
    voice_off
    voice_on
*/