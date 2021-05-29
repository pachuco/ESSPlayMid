#include <windows.h>
#include <stdio.h>
#include "util.h"
#include "buttio.h"
#include "essfm.h"

static const char CONFNAME[] = "config.ini";
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

int main(int argc, char* argv[]) {
    getPortConfig(CONFNAME, &fmConfig);
    printf("FM port %X\n", fmConfig.port);
    
    fmConfig.pHandler = buttio_init(NULL, BUTTIO_MET_IOPM);
    if (!fmConfig.pHandler) return 1;
    
    buttio_shutdown();
    
    return 0;
}