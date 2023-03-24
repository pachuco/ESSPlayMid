#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <conio.h>
#include "util.h"
#include "iodriver.h"
#include "esfm.h"

typedef struct {
    HMIDIIN hmi;
    MIDIINCAPSA caps;
    UINT index;
} MidiInDevice;

typedef struct {
    char fileName[32];
    char description[64];
    BYTE* pData;
} InstrBank;

static USHORT fmBase = 0;
static int curBank = 0;

#define DYNFILE_POLL_TIME 500
#define DYNFILE_FNAME "dynload.bin"
static HANDLE dynFile_handle = INVALID_HANDLE_VALUE;
static ULARGE_INTEGER dynFile_lastModifyTime = {0};
static DWORD dynFile_lastCheckTime = 0;
static BYTE* dynFile_bankData = NULL;

static InstrBank bankArr[] = {
        {"bnk_common.bin", "Most commonly distributed with OS drivers and games.", NULL},
        {"bnk_NT4.bin", "NT4 driver.", NULL},
        
        //{"?malloc", "Malloc unsanitized memory special dish.", NULL},  //will crash, instrument data not sanitized
        //{"?dynfile", "Shared dynload.bin file that you can hex edit.", NULL}
};





//not very accurate
void QPCuWait(DWORD uSecTime) { //KeStallExecutionProcessor
    static LONGLONG freq=0;
    LONGLONG start=0, cur=0, wait=0;
    
    if (freq == 0) QueryPerformanceFrequency((PLARGE_INTEGER)&freq);
    if (freq != 0) {
        wait = ((LONGLONG)uSecTime * freq)/(LONGLONG)1000000;
        QueryPerformanceCounter((PLARGE_INTEGER)&start);
        while (cur < (start + wait)) {
            QueryPerformanceCounter((PLARGE_INTEGER)&cur);
        }
    } else {
        //TODO: alternate timing mechanism
    }
}

USHORT getPortConfig(const char* configName) {
    char configPath[MAX_PATH];
    char configPortText[16];
    
    GetModuleFileNameA(NULL, configPath, MAX_PATH-1);
    util_getParentPathA(configPath);
    strncat(configPath, configName, MAX_PATH-1);
    
    /**/GetPrivateProfileString("essconfig", "port", "0x0000", configPortText, sizeof(configPortText), configPath);
    
    return (USHORT)strtol(configPortText, NULL, 0);
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
    
    (void)hMidiIn; (void)dwParam2;
    
    if(wMsg == MIM_DATA){
        status = (BYTE) ((dwParam1 & 0x000000FF)>>0);
        data1  = (BYTE) ((dwParam1 & 0x0000FF00)>>8);
        data2  = (BYTE) ((dwParam1 & 0x00FF0000)>>16);
        
        (void)(status); (void)(data1); (void)(data2);
        //printf("%02x %02x %02x\n", status, data1, data2);
        
        esfm_midiShort(dwParam1);
    } else if(wMsg == MM_MIM_OPEN){
        printf("Opening device: %i: %s\n", m->index+1, m->caps.szPname);
    } else if(wMsg == MM_MIM_CLOSE){
        printf("Closing device.\n");
    } else {
        printf("unknown message: %08x\n", wMsg);
    }
}

void printFmBankDescription(InstrBank* pBank) {
    printf("FM bank: \"%s\", %s\n", pBank->fileName, pBank->description);
}

BOOL updateDynFile() {
    DWORD bytesRead = 0;
    
    if (dynFile_handle == INVALID_HANDLE_VALUE) return FALSE;
    if (SetFilePointer(dynFile_handle, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) return FALSE;
    if (!ReadFile(dynFile_handle, dynFile_bankData, BANKLEN, &bytesRead, NULL)) return FALSE;
    if (bytesRead != BANKLEN) return FALSE;
    
    return TRUE;
}

void fmWriteCallback(BYTE baseOffset, BYTE data) {
    IODriver_writeU8((fmBase+baseOffset), data);
}

void fmDelayCallback(void) {
    QPCuWait(10);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printUsage();
        return 1;
    }
    
    if        (!strcmp(argv[1], "-h")) {
        printUsage();
        return 0;
    } else if (!strcmp(argv[1], "-l")) {
        UINT i, numMidiInDevs = midiInGetNumDevs();
        
        printf("Midi-in devices(%i total):\n", numMidiInDevs);
        for (i=0; i<numMidiInDevs; i++) {
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
        UINT i, errMidi = 0;
		BOOL isRunning;
        
        if (!devIndex) {
            printUsage();
            return 1;
        }
        midev.index = devIndex - 1;
        
        fmBase = getPortConfig("config.ini");
        if (fmBase == 0) {
            printf("Config read failure!\n");
            return 1;
        }
        printf("FM port %X\n", fmBase);
        
        assert(COUNTOF(bankArr) >= 1);
        for (i=0; i < COUNTOF(bankArr); i++) {
            int size;
            
            if(bankArr[i].fileName[0] == '?') {
                char* fName = bankArr[i].fileName;
                if        (!strcmp("?malloc", fName)) {
                    bankArr[i].pData = malloc(BANKLEN);
                    assert(bankArr[i].pData != NULL);
                } else if (!strcmp("?dynfile", fName)) {
                    dynFile_handle = CreateFileA(DYNFILE_FNAME, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
                    assert(dynFile_handle != INVALID_HANDLE_VALUE);
                    
                    dynFile_bankData = malloc(BANKLEN);
                    bankArr[i].pData = dynFile_bankData;
                    
                    assert(dynFile_bankData);
                    assert(updateDynFile());
                }
            } else if (loadFile(bankArr[i].fileName, &bankArr[i].pData, &size)) {
                assert(size == BANKLEN);
                assert(bankArr[i].pData != NULL);
            }
        }
        curBank = 0;
            
        if (!IODriver_Init(fmBase, fmBase+0xF)) {
            printf("IO driver init error!\n");
            return 1;
        }
        
        errMidi |= midiInGetDevCapsA(midev.index, &midev.caps, sizeof(MIDIINCAPSA));
        errMidi |= midiInOpen(&midev.hmi, midev.index, (DWORD_PTR)&midiCB, (DWORD_PTR)&midev, CALLBACK_FUNCTION);
        errMidi |= midiInStart(midev.hmi);
        if (errMidi) {
            printf("Midi-in init error!\n");
            IODriver_Exit();
            return 1;
        };
        
        esfm_init(bankArr[0].pData, &fmWriteCallback, &fmDelayCallback);
        printFmBankDescription(&bankArr[0]);
        
        isRunning = TRUE;
        while(isRunning) {
            if (kbhit()) {
                unsigned char c = _getch();
                
                switch (c) {
                    case VK_SPACE: {
                        InstrBank* pBank;
                        
                        if (COUNTOF(bankArr) == 1) break;
                        curBank = (curBank + 1) % COUNTOF(bankArr);
                        pBank = &bankArr[curBank];
                        printFmBankDescription(pBank);
                        
                        esfm_setBank(pBank->pData);
                        }break;
                    case VK_ESCAPE: {
                        esfm_shutdownDevice();
                        isRunning = FALSE;
                        }break;
                    default:
                    break;
                }
            }
            
            if (dynFile_handle != INVALID_HANDLE_VALUE) {
                DWORD curTick = GetTickCount();
                
                if (curTick - dynFile_lastCheckTime >= DYNFILE_POLL_TIME) { //TODO: 49 day rollover check
                    FILETIME checkedTime = {0};
                    
                    dynFile_lastCheckTime = curTick;
                    if (GetFileTime(dynFile_handle, NULL, NULL, &checkedTime)) {
                        ULARGE_INTEGER uliCheckedTime = {checkedTime.dwLowDateTime, checkedTime.dwHighDateTime};
                        
                        if (uliCheckedTime.QuadPart != dynFile_lastModifyTime.QuadPart) {
                            dynFile_lastModifyTime.QuadPart = uliCheckedTime.QuadPart;
                            updateDynFile();
                        }
                    }
                }
            }
            
            
            
            SleepEx(50, TRUE);
        }
        
        midiInStop(midev.hmi);
        midiInClose(midev.hmi);
        
        //SleepEx(2000, 1);
        IODriver_Exit();
    }
    
    return 0;
}