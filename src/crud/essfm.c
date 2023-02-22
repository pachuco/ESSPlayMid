#include <windows.h>
#include <assert.h>
#include "essfm.h"

#ifdef _MSC_VER
#  ifndef STDCALL
#    define STDCALL __stdcall 
#  endif
#  define static_assert(x,y)
#endif


////////////////////////////////////////////////////////////////////////////////////////////
//AUDDRIVE.SYS

/*
#include "iodriver.h"

// It's hardcoded base address in AUDDRIVE.SYS, so we do the same
// on reconstructed source
#define ESS_FMBASE	0x388
#define FMREGLENGTH 595

//
// Devices - these values are also used as array indices
//

typedef enum {
   WaveInDevice = 0,
   WaveOutDevice,
   MidiOutDevice,
   MidiInDevice,
   LineInDevice,
   CDInternal,
   MixerDevice,
   NumberOfDevices
} SOUND_DEVICES;

BYTE FMRegs[608] = {0}; //!WARN FMREGLENGTH is 595

SHORT Address_SynthMidiData[2] = {0};


void SynthInitNativeESFM(void) 
{
    WRITE_PORT_UCHAR(ESS_FMBASE + 0, 0x00);
    KeStallExecutionProcessor(25);
    WRITE_PORT_UCHAR(ESS_FMBASE + 2, 0x05);
    KeStallExecutionProcessor(25);
    WRITE_PORT_UCHAR(ESS_FMBASE + 1, 0x80);
    KeStallExecutionProcessor(25);
}

void SynthSaveESFM(PGLOBAL_DEVICE_INFO pGDI)
{
	UINT i;

    for (i=0; i < FMREGLENGTH; i++) 
	{
        WRITE_PORT_UCHAR(ESS_FMBASE + 2, i&0xFF);
        KeStallExecutionProcessor(3);
        WRITE_PORT_UCHAR(ESS_FMBASE + 3, i>>8);
        KeStallExecutionProcessor(3);
        FMRegs[i] = READ_PORT_UCHAR(ESS_FMBASE + 1);
        KeStallExecutionProcessor(3);
        WRITE_PORT_UCHAR(ESS_FMBASE + 1, 0x00);
        KeStallExecutionProcessor(3);
    }
}

void SynthRestoreESFM(PGLOBAL_DEVICE_INFO pGDI) 
{
	UINT i;

    for (i=0; i < FMREGLENGTH; i++) 
	{
        WRITE_PORT_UCHAR(ESS_FMBASE + 2, i&0xFF);
        KeStallExecutionProcessor(3);
        WRITE_PORT_UCHAR(ESS_FMBASE + 3, i>>8);
        KeStallExecutionProcessor(3);
        WRITE_PORT_UCHAR(ESS_FMBASE + 1, FMRegs[i]);
        KeStallExecutionProcessor(3);
    }
}

void SynthMidiSendFM(PUCHAR PortBase, ULONG Address, UCHAR data) 
{
    USHORT offHigh = (Address < 0x100) ? 0 : 2;
    
    WRITE_PORT_UCHAR(PortBase + offHigh, Address&0xFF);
    KeStallExecutionProcessor(23);
    WRITE_PORT_UCHAR(PortBase + offHigh + 1, data);
    KeStallExecutionProcessor(23);
}

BOOL SynthPresent(PUCHAR outPort, PUCHAR inbase, BOOLEAN *pIsFired) 
{
    UCHAR t1, t2;

    if (pIsFired) *pIsFired = FALSE;

    // check if the chip is present
    SynthMidiSendFM(outPort, 4, 0x60);
    SynthMidiSendFM(outPort, 4, 0x80);
    t1 = READ_PORT_UCHAR(outPort + inbase);
    SynthMidiSendFM(outPort, 2, 0xFF);
    SynthMidiSendFM(outPort, 4, 0x21);
    KeStallExecutionProcessor(200);

    if (pIsFired && *pIsFired) return TRUE;

    t2 = READ_PORT_UCHAR(inbase);
    SynthMidiSendFM(outPort, 4, 0x60);
    SynthMidiSendFM(outPort, 4, 0x80);

    if (t1 & 0xE0 || (t2 & 0xE0) != 0xC0) return FALSE;

    return TRUE;
}

BOOLEAN SynthMidiIsOpl3(PSYNTH_HARDWARE pHw, BOOLEAN *isFired)
{
	BOOLEAN isOpl3;
	PUCHAR Port = pHw->SynthBase:

	SynthMidiSendFM(pHw->SynthBase, 0x105, 0);
	KeStallExecutionProcessor(20);
	if (SynthPresent(Port + 2, Port, isFired) ) 
	{
		SynthMidiSendFM(Port, 0x105, 1);
		KeStallExecutionProcessor(20);
		if (!SynthPresent(Port + 2, Port, isFired) ) 
			isOpl3 = TRUE;
	}
  }
  SynthMidiSendFM(Port, 0x105, 0);
  KeStallExecutionProcessor(20);
  return isOpl3;
}

void SynthMidiQuiet(UCHAR deviceIndex, PSYNTH_HARDWARE pHw)
{
	UINT i;

	if (deviceIndex != MixerDevice) return;

    SynthMidiSendFM(pHw->SynthBase, 0x105, 0x01);
    SynthMidiSendFM(pHw->SynthBase, 0x004, 0x60);
    SynthMidiSendFM(pHw->SynthBase, 0x104, 0x3F);
    SynthMidiSendFM(pHw->SynthBase, 0x008, 0x00);
    SynthMidiSendFM(pHw->SynthBase, 0x0BD, 0xC0);
    
    for (i=0; i < 21; i++) 
	{
        SynthMidiSendFM(fmConf, 0x040 + i, 0x3F);
        SynthMidiSendFM(fmConf, 0x140 + i, 0x3F);
    }
    
    for (i=0; i < 8; i++) 
	{
        SynthMidiSendFM(pHw->SynthBase, 0x0B0 + i, 0);
        SynthMidiSendFM(pHw->SynthBase, 0x1B0 + i, 0x00);
    }
}

/*
void SynthMidiData(FmConfig* fmConf, USHORT address, BYTE data) 
{
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
    //KeStallExecutionProcessor(23); //OPL2
    KeStallExecutionProcessor(10); //OPL3, etc.
}
*/

////////////////////////////////////////////////////////////////////////////////////////////
//AUDDRIVE.DLL
typedef struct {
    BYTE    flags1;
    BYTE    field_1;
    USHORT  timer;
    BYTE    channel;
    BYTE    bNote;
    BYTE    rel_vel;
    BYTE    field_7;
    USHORT  detune[4];
    BYTE    patch_flag;
    BYTE    reg5[4];
    BYTE    reg1[4];
    BYTE    field_19;
} Voice;
static_assert(sizeof(Voice) == 26, "");

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
BYTE pmask_MidiPitchBend[4] = {
    16, 32, 64, 128
};
int td_adjust_setup_operator[12] = {
    256, 242, 228, 215, 203, 192,
    181, 171, 161, 152, 144, 136
};
BYTE gbVelocityAtten[32] = {
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

extern PBYTE gBankMem;
BYTE  pan_mask[16]          = {0};
BYTE  gbVelLevel[16]        = {0};
BYTE  gbChanAtten[16]       = {0};
SHORT giBend[16]            = {0};
BYTE  hold_table[16]        = {0};
BYTE  gbChanVolume[16]      = {0};
BYTE  program_table[16]     = {0};
BYTE  gbChanExpr[16]        = {0};
BYTE  gbChanBendRange[16]   = {0};
BYTE  note_offs[16]         = {0};
//!HINT size: MidiPitchBend(), 26bytes * 18
//!WARN might be bigger(20 voices; melodic + perc)
Voice voice_table[18]       = {0};
WORD  DeviceData[162]       = {0};
USHORT gwTimer;
DWORD voice1, voice2;

UINT  MidiPosition = 0;


void note_on(BYTE channel, BYTE bNote, BYTE bVelocity);
void note_off(BYTE bChannel, BYTE bNote);
void hold_controller(BYTE channel, BYTE bVelocity);
void voice_on(int voiceNr);
void voice_off(int voiceNr);
void __stdcall fmreset();
BYTE NATV_CalcVolume(BYTE reg1, BYTE rel_velocity, BYTE channel);
void NATV_CalcNewVolume(BYTE bChannel);
void MidiPitchBend(BYTE bChannel, USHORT iBend);
void find_voice(BOOL patch1617_allowed_voice1, BOOL patch1617_allowed_voice2, BYTE bChannel, BYTE bNote);
int steal_voice(int patch1617_allowed);
void setup_operator(int offset, int bNote, int bVelocity, USHORT reg, int fixed_pitch, int rel_velocity, int channel, int operator, int voicenr);
void setup_voice(int voicenr, int offset, int channel, int bNote, int bVelocity);
void STDCALL MidiMessage(DWORD dwData);




SHORT NATV_CalcBend(USHORT detune, USHORT iBend, USHORT iBendRange)
{
    //!WARN iBend is int16 in OPL midi driver sample
    if ( iBend == 0x2000 ) 
        return detune & 0xFF;
    else 
	{
        int v5;
        if ( iBend >= 0x3F80 ) iBend = 0x4000;
        v5 = (iBendRange * (iBend - 0x2000) >> 5) + 0x1800;
        return (detune * (USHORT)((NATV_table1[(v5>>2)&0x3F] * NATV_table2[v5>>8]) >> 10) + 512) >> 10;
    }
}

UINT MidiCalcFAndB(UINT bend, BYTE block) {
    while (bend >= 1024) {
        bend >>= 1;
        block++;
    }
    if ( block > 7 ) block = 7;
    return bend  | (block << 10);
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

void note_on(BYTE channel, BYTE bNote, BYTE bVelocity)
{
    int patch;
    int offset;
    BYTE flags_voice1;
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

void note_off(BYTE bChannel, BYTE bNote)
{
    int i;
    
    for (i=0; i<18; i++)
    {
		Voice *voice = &voice_table[i];
        if ((voice->flags1 & 1) && voice->channel == bChannel && voice->bNote == bNote)
        {
            if ( hold_table[bChannel] & 1 ) 
            {
                voice->flags1 |= 4;
            }
            else
            {
                voice_off(i);
            }
        }
    }
}

void hold_controller(BYTE channel, BYTE bVelocity)
{
    if ( bVelocity < 64 ) 
    {
        int i;
        
        hold_table[channel] &= ~1;
        
        for (i = 0; i<sizeof(voice_table)/sizeof(voice_table[0]); i++)
        {
            if ((voice_table[i].flags1 & 4) && voice_table[i].channel == channel)
                voice_off(i);
        }
    } else {
        hold_table[channel] |= 1;
    }
}

void voice_on(int voiceNr)
{
    if ( voiceNr >= 16 )
    {
        if ( voiceNr == 16 )
        {
            fmwrite(0x250, 1);
            fmwrite(0x251, 1);
        }
        else
        {
            fmwrite(0x252, 1);
            fmwrite(0x253, 1);
        }
    }
    else 
    {
        fmwrite((USHORT)voiceNr + 0x240, 1);
    }
}

void voice_off(int voiceNr)
{
    if ( voiceNr >= 16 )
    {
        if ( voiceNr == 16 )
        {
          fmwrite(0x250, 0);
          fmwrite(0x251, 0);
        }
        else
        {
          fmwrite(0x252, 0);
          fmwrite(0x253, 0);
        }
    }
    else
    {
        fmwrite((USHORT)voiceNr + 0x240, 0);
    }
    voice_table[voiceNr].flags1 = 2;
    voice_table[voiceNr].timer = (USHORT)gwTimer;
    gwTimer++;
}



void __stdcall fmreset() {
    //!WARN partial decompile
	UINT i;
    
    //v0 = MidiPosition;
    //v0 *= 4;
    //!WARN FM write
    //*(__int16 *)((char *)DeviceData + v0) = 0x38A;
    //*(__int16 *)((char *)&DeviceData[1] + v0) = 5;
    //*(__int16 *)((char *)&DeviceData[2] + v0) = 0x389;
    //*(__int16 *)((char *)&DeviceData[3] + v0) = 0x80;
    MidiPosition += 2;
    MidiFlush();
    for (i=0; i<16; i++) 
    {
        giBend[i]           = 0x2000;
        gbChanBendRange[i]  = 0x02;
        hold_table[i]       = 0x00;
        gbChanExpr[i]       = 0x7F;
        gbChanVolume[i]     = 0x64;
        gbChanAtten[i]      = 0x04;
        pan_mask[i]         = 0x30;
    }
    
    for (i=0; i < 18; i++) 
    {
        voice_table[i].timer = 0;
        voice_table[i].flags1 = 0;
    }
    
    gwTimer = 0;
}

BYTE NATV_CalcVolume(BYTE reg1, BYTE rel_velocity, BYTE channel)
{
  BYTE vol;

  if ( !gbChanVolume[channel] ) return 63;

  switch ( rel_velocity )
  {
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
      if ( vol < 64 )
        vol = ((63 - vol) >> 1) + 16;
      else
        vol = (127 - vol) >> 2;
      if ( gbChanExpr[channel] < 64 )
      {
         vol += ((63 - gbChanExpr[channel]) >> 1) + 16;
      }
      else
      {
         vol += ((127 - gbChanExpr[channel]) >> 2);
      }
      break;
  }
  vol += (reg1 & 0x3F);          // ATTENUATION
  if ( vol > 63 ) vol = 63;
  return vol | reg1 & 0xC0;      // KSL
}

void NATV_CalcNewVolume(BYTE bChannel)
{
    UINT i, j, offset;

    for (i=0, offset=1; i < sizeof(voice_table)/sizeof(voice_table[0]); i++) 
    {
        Voice *voice = &voice_table[i];
        if ((voice->flags1 & 1) && (voice->channel == bChannel || bChannel == 0xFF)) 
        {
            for (j=0; j < sizeof(voice->channel); j++)
            {
                fmwrite((USHORT)offset, NATV_CalcVolume(voice->reg1[j], (voice->rel_vel & 3), voice->channel));
                offset += 8;
            }
        }
    }   
}

void MidiPitchBend(BYTE bChannel, USHORT iBend) 
{
    UINT i, j;
    
    giBend[bChannel] = iBend;
    for (i=0; i < sizeof(voice_table)/sizeof(voice_table[0]); i++) 
    {
		Voice *voice = &voice_table[i];
        if (voice->channel == bChannel && voice->flags1 & 1) 
        {
            for (j=0; j < sizeof(voice->reg5); j++)
            {
                if ((pmask_MidiPitchBend[j] & voice->patch_flag) == 0) 
                {
                    SHORT bnd;
                    
                    bnd = NATV_CalcBend(voice->detune[j], iBend, gbChanBendRange[bChannel]);
                    bnd = MidiCalcFAndB(bnd, (voice->reg5[j] >> 2) & 7);
                    fmwrite(i*32 + j*8 + 5, (voice->reg5[j] & 0xE0) | (bnd>>8));
                    fmwrite(i*32 + j*8 + 4, bnd & 0xFF);
                }
            }
        }
    }
}


void find_voice(BOOL patch1617_allowed_voice1, BOOL patch1617_allowed_voice2, BYTE bChannel, BYTE bNote)
{
    int i;
    USHORT td, timediff1=0, timediff2=0;
    
    // Patch 0-15
    for (i=0; i<16; i++)
    {
        Voice *voice = &voice_table[i];
        if (voice->flags1 & 1)
        {
            if (voice->channel == bChannel && voice->bNote == bNote)
                voice_off(i);
        }
        td = gwTimer - voice->timer;
        if (td < timediff1)
        {
            if (td >= timediff2)
            {
                timediff2 = td;
                voice2 = i;
            }
        }
        else
        {
            timediff2 = timediff1;
            voice2 = voice1;
            timediff1 = td;
            voice1 = i;
        }
    }
    
    // Patch 16
    if (voice_table[16].flags1 & 1)
    {
        if (voice_table[16].channel == bChannel && voice_table[16].bNote == bNote)
            voice_off(16);
    }
    td = gwTimer - voice_table[16].timer;
    if (patch1617_allowed_voice1 || td < timediff1)
    {
        if ( !patch1617_allowed_voice2 && td >= timediff2 )
        {
            timediff2 = gwTimer - voice_table[16].timer;
            voice2 = 16;
        }
    }
    else
    {
        timediff2 = timediff1;
        voice2 = voice1;
        timediff1 = td;
        voice1 = 16;
    }

    // Patch 17
    if (voice_table[17].flags1 & 1)
    {
        if (voice_table[17].channel == bChannel && voice_table[17].bNote == bNote)
            voice_off(17);
    }
    td = gwTimer - voice_table[17].timer;
    if (patch1617_allowed_voice1 || td < timediff1)
    {
        if ( !patch1617_allowed_voice2 && td >= timediff2 )
            voice2 = 17;
    }
    else
    {
        if (voice1 != 16 || !patch1617_allowed_voice2)
            voice2 = voice1;
        voice1 = 17;
    }
    
}

int steal_voice(int patch1617_allowed)
{
    UINT i, last_voice, max_voices = (patch1617_allowed?18:16);
    BYTE chn, chncmp = 0, bit3 = 0;
	USHORT timediff = 0;
    
    for (i=0; i<max_voices; i++)
    {
        chn = voice_table[i].channel == 9?1:voice_table[i].channel+2;
        if (bit3 == (voice_table[i].flags1 & 8))
        {
            if (chn <= chncmp)
            {
                if ( chn != chncmp || (gwTimer - voice_table[i].timer) <= timediff )
                    continue;
            }
        }
        else if (!bit3) bit3 = 8;
        
        chncmp = chn;
        timediff = gwTimer - voice_table[i].timer;
        last_voice = i;
    }
    voice_off(last_voice);
    return last_voice;
}

void  setup_operator(
        int offset,
        int bNote,
        int bVelocity,
        USHORT reg,
        int fixed_pitch,
        int rel_velocity,
        int channel,
        int oper,
        int voicenr)
{
    int panmask, note, transpose, block, notemod12, reg1, detune;
    USHORT fnum_block;
    BYTE reg4, reg5;
    
    panmask = pan_mask[channel];
    fmwrite(reg + 7, 0);
    
    if (fixed_pitch)
    {
        note = bNote;
    }
    else
    {
        transpose = ((((gBankMem[offset + 5]) << 2) & 0x7F) | (gBankMem[offset + 4] & 3));
        if (gBankMem[offset + 5] & 0x10) // signed?
            transpose |= ~0x7F;
        note = transpose + bNote;
    }
    
    if ( note < 19 )
        note += 12 * ((18 - note) / 12u) + 12;
    if ( note > 114 )
        note += -12 - 12 * ((note - 115) / 12u);
    block = (note - 19) / 12;
    notemod12 = (note - 19) % 12;
    
    fmwrite(reg, gBankMem[offset]);
    
    switch ( rel_velocity )
    {
    case 0:
    default:
        reg1 = 0;
        break;
    case 1:
        reg1 = (127 - bVelocity) >> 4;
        break;
    case 2:
        reg1 = (127 - bVelocity) >> 3;
        break;
    case 3:
        if ( bVelocity < 64 )
            reg1 = ((63 - bVelocity) >> 1) + 16;
        else
            reg1 = (127 - bVelocity) >> 2;
        break;
    }
    reg1 += (gBankMem[offset + 1] & 0x3F); // Attenuation
    if (reg1 > 63) *((PBYTE)&reg1) = 63;
    reg1 += (gBankMem[offset + 1] & 0xC0); // KSL
    voice_table[voicenr].reg1[oper] = (BYTE)reg1;
    
    fmwrite(reg + 1, NATV_CalcVolume((BYTE)reg1, rel_velocity, channel));
    fmwrite(reg + 2, gBankMem[offset + 2]);
    fmwrite(reg + 3, gBankMem[offset + 3]);
    
    if ( fixed_pitch )
    {
        reg4 = gBankMem[offset + 4];
        reg5 = gBankMem[offset + 5];
    }
    else
    {
        detune = gBankMem[offset + 4] & (~3);
        if (detune)
        {
            detune = ((detune >> 2) * td_adjust_setup_operator[notemod12]) >> 8;
            if (block > 1) 
                detune >>= block - 1;
        }
        detune += notemod12;
        voice_table[voicenr].reg5[oper] = (((detune >> 8) & 3) | (gBankMem[offset + 5] & 0xE0) | (block << 2)); // detune | delay | block
        fnum_block = MidiCalcFAndB(NATV_CalcBend(detune, giBend[channel], gbChanBendRange[channel]), block);
        reg4 = (BYTE)fnum_block;
        reg5 = (BYTE)(fnum_block >> 8) | (voice_table[voicenr].reg5[oper] & 0xE0);
        voice_table[voicenr].detune[oper] = (BYTE)detune;
    }
    fmwrite(reg + 4, reg4);
    fmwrite(reg + 5, reg5);
    fmwrite(reg + 5, ((gBankMem[offset + 6] & 0x30) && panmask != 0x30)?(panmask | gBankMem[offset + 6] & 0xCF):gBankMem[offset + 6]);
    fmwrite(reg + 7, gBankMem[offset + 7]);
}

void  setup_voice(int voicenr, int offset, int channel, int bNote, int bVelocity)
{
    BYTE rel_vel, flags;
    
    flags   = gBankMem[offset];
    rel_vel = gBankMem[offset + 3];
    offset += 4;
    setup_operator(offset     , bNote, bVelocity, 32 * voicenr     , flags & 0x10, rel_vel        & 3, channel, 0, voicenr);
    setup_operator(offset + 8 , bNote, bVelocity, 32 * voicenr + 8 , flags & 0x20, (rel_vel >> 2) & 3, channel, 1, voicenr);
    setup_operator(offset + 16, bNote, bVelocity, 32 * voicenr + 16, flags & 0x40, (rel_vel >> 4) & 3, channel, 2, voicenr);
    setup_operator(offset + 24, bNote, bVelocity, 32 * voicenr + 24, flags & 0x80, (rel_vel >> 6) & 3, channel, 3, voicenr);

    voice_table[voicenr].patch_flag = flags;
    voice_table[voicenr].rel_vel = rel_vel;
    voice_table[voicenr].timer = gwTimer;
    voice_table[voicenr].bNote = bNote;
    voice_table[voicenr].flags1 = 1;
    voice_table[voicenr].channel = channel;
    
    gwTimer++;
}















#if 0
#include <mmddk.h>
modSynthMessage(UINT uDeviceID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    switch(uMsg) {
        case MODM_DATA:     // 7
        break;
        case MODM_LONGDATA: // 8
        break;
    }
}
#endif

void STDCALL MidiMessage(DWORD dwData) 
{
    BYTE    bMsgType, bChannel, bVelocity, bNote;
    int     i;

    // D1("\nMidiMessage");
    bMsgType =    (BYTE)((dwData & 0x000000F0));
    bChannel =    (BYTE)((dwData & 0x0000000F) >> 0);
    bNote =       (BYTE)((dwData & 0x00007F00) >> 8);
    bVelocity =   (BYTE)((dwData & 0x007F0000) >> 16);
    
    switch (bMsgType) 
    {
        case 0x90:      /* turn key on, or key off if volume == 0 */
            if (bVelocity)
            {
                note_on((BYTE)dwData, bNote, bVelocity); 
            }
            else 
            {
                note_off((BYTE)dwData, bNote);
            }
            break;
        case 0x80:      /* turn key off */
            note_off((BYTE)dwData, bNote);
            break;
        case 0xb0:      /* change control */
            switch (bNote)
            {
                case 6:
                    if ( (hold_table[bChannel] & 6) == 6 )
                        gbChanBendRange[bChannel] = bVelocity;
                    break;
                case 7:
                    gbChanAtten[bChannel] = gbVelocityAtten[bVelocity >> 1];
                    gbChanVolume[bChannel] = bVelocity;
                    NATV_CalcNewVolume((BYTE)dwData);
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
                    NATV_CalcNewVolume((BYTE)dwData);
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
        case 0xe0:      /* pitch bend */
            MidiPitchBend((BYTE)dwData, bNote | (bVelocity << 7));
            break;
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