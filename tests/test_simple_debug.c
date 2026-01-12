#include "src/melvin.h"
#include <stdio.h>
#include <string.h>

int main() {
    printf("Creating .m file...\n");
    MelvinMFile *mfile = melvin_m_create("test_debug.m");
    if (!mfile) {
        printf("Failed to create file\n");
        return 1;
    }
    
    printf("Writing input...\n");
    const char *input = "hello";
    melvin_m_universal_input_write(mfile, (const uint8_t*)input, strlen(input));
    
    printf("Processing input...\n");
    int result = melvin_m_process_input(mfile);
    printf("Process result: %d\n", result);
    
    printf("Closing file...\n");
    melvin_m_close(mfile);
    
    printf("Done!\n");
    return 0;
}
