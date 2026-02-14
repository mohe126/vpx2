#include "vpx2.c"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


uint32_t get_file_size(FILE* file){
    fseek(file, 0, SEEK_END);
    uint32_t size = ftell(file); //32 bit because VPX can't access more than 4 GB anyway.
    rewind(file);

    return size;
}

uint8_t execute_hostcall(uint32_t hostcall){
    //Register 61 is for the hostcall code.
    //register 60 for arguments (array ptr if multiple.)
    switch(hostcall){
        default: return 1; //error
        case 0: exit(vpx2_rreg(60)); break;
        case 1: putchar('T'); break; //Debug
        //Will add other hostcalls Later for IO and whatever.


    }
    return 0;
}


int main(int argc, char *argv[]){

    FILE* file = fopen(argv[1], "rb"); //First argument is the file.
    //[[ CHECK IF FILE OPENED SUCCESSFULLY ]]
    if(file == NULL){
        printf("failed to open file: %s \n", argv[1]);
        return 1;
    }


    uint32_t file_size = get_file_size(file);

    uint8_t *mem_ptr = malloc(file_size);

    //[[ CHECK IF ALLOCATION SUCCESSFUL ]]
    if(mem_ptr == NULL){
        printf("failed to allocate memory for file: %u Bytes\n", file_size);
        return 1; //Failed
    }

    //[[ COPY FILE CONTENTS TO ARRAY ]]
    fread(mem_ptr, 1, file_size, file);

    //[[ INITIALIZE VPX ]]
    vpx2_init(mem_ptr, file_size); //Can realloc with hostcalls if needed.

    //[[ MAIN VPX LOOP ]]
    while(1){
        uint8_t rt = vpx2_start();
        if(rt == 1){
            printf("error during vpx execution.\n");
            printf("error code: %hhu\n", vpx2_err_code);
            printf("error value: %u\n", vpx2_err_val);
            printf("RPC state: %u\n", vpx2_err_pc_state);
            return 1;
        }
        uint32_t hostcall_code = vpx2_rreg(61);
        uint8_t st = execute_hostcall(hostcall_code);
        if(st == 1){
            printf("attempt to execute invalid hostcall: %u", hostcall_code);
            return 1;
        }


    }



    return 0;

    
}