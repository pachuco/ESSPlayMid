#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include "util.h"
#include "buttio.h"
#include "essfm.h"

#define CONFNAME "config.ini"

typedef struct {
    HMIDIIN hmi;
    MIDIINCAPSA caps;
    UINT index;
} MidiInDevice;

static FmConfig fmConfig = {0};

void getPortConfig(const char* configName, FmConfig* config) {
    char configPath[MAX_PATH];
    char configPortText[16];
    
    GetModuleFileNameA(NULL, configPath, MAX_PATH-1);
    util_getParentPathA(configPath);
    strncat(configPath, configName, MAX_PATH-1);
    
    /**/GetPrivateProfileString("essconfig", "port", "0x0000", configPortText, sizeof(configPortText), configPath);
    
    config->port = (USHORT)strtol(configPortText, NULL, 0);
}

void printUsage() {
    printf(
        "Usage:\n"
        "essmidi.exe PARAM\n"
        "Parameters:\n"
        "   -h: Show this help.\n"
        "   -l: List midi-in devices.\n"
        "   NUMBER: 1-999; Connect to midi-in device. Press ESC at any time to exit.\n"
    );
}

//lazilly copypasted from https://github.com/wjlandryiii/midilog
void CALLBACK midiCB(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    MidiInDevice* m = (MidiInDevice*)dwInstance;
    BYTE status;
    BYTE data1;
    BYTE data2;
    
    (void)hMidiIn;
    (void)dwParam2;
    
    if(wMsg == MIM_DATA){
        status = (BYTE) ((dwParam1 & 0x000000FF)>>0);
        data1  = (BYTE) ((dwParam1 & 0x0000FF00)>>8);
        data2  = (BYTE) ((dwParam1 & 0x00FF0000)>>16);
        printf("%02x %02x %02x\n", status, data1, data2);
    } else if(wMsg == MM_MIM_OPEN){
        printf("Opening device: %i: %s\n", m->index+1, m->caps.szPname);
    } else if(wMsg == MM_MIM_CLOSE){
        printf("Closing device.\n");
    } else {
        printf("unknown message: %08x\n", wMsg);
}
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printUsage();
        return 1;
    }
    
    if        (!strcmp(argv[1], "-h")) {
        printUsage();
        return 0;
    } else if (!strcmp(argv[1], "-l")) {
        UINT numMidiInDevs = midiInGetNumDevs();
        
        printf("Midi-in devices(%i total):\n", numMidiInDevs);
        for (UINT i=0; i<numMidiInDevs; i++) {
            MIDIINCAPSA midiCaps = {0};
            
            if (midiInGetDevCapsA(i, &midiCaps, sizeof(MIDIINCAPSA)) == MMSYSERR_NOERROR) {
                printf("%i: %s\n", i+1, midiCaps.szPname);
            } else {
                printf("%i: ___________________\n", i+1);
            }
        }
        return 0;
    } else {
        UINT devIndex = strtol(argv[1], NULL, 10);
        MidiInDevice midev = {0};
        UINT errMidi = 0;
        
        if (!devIndex) {
            printUsage();
            return 1;
        }
        midev.index = devIndex - 1;
        
        getPortConfig(CONFNAME, &fmConfig);
        printf("FM port %X\n", fmConfig.port);
        
        if (!buttio_init(&fmConfig.ioHand, NULL, BUTTIO_MET_IOPM)) {
            printf("BUTTIO init failure!\n");
            return 1;
        }
        
        errMidi |= midiInGetDevCapsA(midev.index, &midev.caps, sizeof(MIDIINCAPSA));
        errMidi |= midiInOpen(&midev.hmi, midev.index, (DWORD_PTR)&midiCB, (DWORD_PTR)&midev, CALLBACK_FUNCTION);
        errMidi |= midiInStart(midev.hmi);
        if (errMidi) {
            printf("Midi-in init error!\n");
            buttio_shutdown(&fmConfig.ioHand);
            return 1;
        };
        
        while(1) {
            if (kbhit()) {
                unsigned char c = _getch();
                
                if (c == 0x1B) break; //ESCAPE
            }
            SleepEx(50, TRUE);
        }
        
        midiInStop(midev.hmi);
        midiInClose(midev.hmi);
    }
    
    SleepEx(2000, 1);
    buttio_shutdown(&fmConfig.ioHand);
    
    return 0;
}