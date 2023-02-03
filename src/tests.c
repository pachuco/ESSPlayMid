#include <stdio.h>
#include <assert.h>
#include "util.h"
#include "esfm.h"

#define ASS(X) assert(X)

BOOL test_bankGeometryIdentical(MidiBank* pBnk1, MidiBank* pBnk2) {
    return memcmp(pBnk1, pBnk2, 128*2);
}

void info_patchSizesDumb(char* name, MidiBank* pBnk) {
    printf("Bank %s: patch lengths\n", name);
    
    // melodic
    for (unsigned int i=0; i<128; i++) {
        USHORT s0 = pBnk->melodicOffsets[i+0];
        USHORT s1 = pBnk->melodicOffsets[i+1];
        
        printf("Midi melo patch %u: start at %u", i+1, s0);
        if (i != 127) {
            printf(", supposed size %d", s1 - s0);
        }
        printf("\n");
    }
    
    // drums
    for (unsigned int i=0; i<128; i++) {
        USHORT s0 = pBnk->drumOffsets[i+0];
        USHORT s1 = pBnk->drumOffsets[i+1];
        
        printf("Midi drum patch %u: start at %u", i+1, s0);
        if (i != 127) {
            printf(", supposed size %d", s1 - s0);
        }
        printf("\n");
    }
    
    printf("\n");
}

void info_72v36MeloPatchesBits(char* name, MidiBank* pBnk) {
    BYTE and36[36] = {0xFF};
    BYTE and72[36] = {0xFF}; // first half
    printf("Bank %s: 72v36 AND\n", name);
    
    // melodic
    for (unsigned int i=0; i<128; i++) {
        USHORT s0 = pBnk->melodicOffsets[i+0];
        USHORT s1 = pBnk->melodicOffsets[i+1];
        
        switch (s1 - s0) {
            case 36: for (unsigned int j=0; j<36; j++) { and36[j] &= pBnk->patchData[s0 + j];} break;
            case 72: for (unsigned int j=0; j<36; j++) { and72[j] &= pBnk->patchData[s0 + j];} break;
        }
        if (i == 127) break;
    }
    
    printf("36 patches AND\n");
    for (unsigned int i=0; i<36; i++) {
        printf("%02X", and36[i]);
        if ((i % 4) == 3) printf(" ");
    }
    printf("\n");
    
    printf("72 patches AND(first half)\n");
    for (unsigned int i=0; i<36; i++) {
        printf("%02X", and72[i]);
        if ((i % 4) == 3) printf(" ");
    }
    printf("\n");
    
    printf("36v72 patch XOR(different bits)\n");
    for (unsigned int i=0; i<36; i++) {
        printf("%02X", and36[i] ^ and72[i]);
        if ((i % 4) == 3) printf(" ");
    }
    printf("\n");
    
    printf("\n");
}

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    
    char* name[2] = { "bnk_common.bin", "bnk_NT4.bin" };
    MidiBank* pBnk[2];
    int size[2];
    
    ASS(loadFile(name[0], (BYTE**)&pBnk[0], &size[0]));
    ASS(loadFile(name[1], (BYTE**)&pBnk[1], &size[1]));
    ASS(size[0] == BANKLEN);
    ASS(size[1] == BANKLEN);
    
    
    ASS(test_bankGeometryIdentical(pBnk[0], pBnk[1]));
    
    info_patchSizesDumb(name[0], pBnk[0]);
    info_patchSizesDumb(name[1], pBnk[1]);
    
    info_72v36MeloPatchesBits(name[0], pBnk[0]);
    info_72v36MeloPatchesBits(name[1], pBnk[1]);
    // All zeroes. Which means dumb sizes are unreliable for differentiating 36 and 72 patches.
    // What now?
}