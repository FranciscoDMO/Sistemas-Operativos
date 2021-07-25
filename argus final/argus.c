#include <stdio.h>
#include <unistd.h>  
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h> 
#include "argus.h"


int main(int argc, char* argv[]) {

    int input_max_size = 100;

    char* linha = malloc(input_max_size);
    int seen;
    
    if (fork() == 0){
        if(argc == 1) {
            
            while ((seen = readline(0,linha,input_max_size)) > 0) {     
                int to_sv  = open("client_to_server", O_WRONLY); 

                strcat(linha,"\n");

                write(to_sv, linha, seen+2);


                close(to_sv);
            }
        } else {
            char tmp[500];
            strcpy(tmp, argv[1]);
            for(int i = 2; i < argc; i++) {
                strcat(tmp, " ");
                strcat(tmp, "\"");
                strcat(tmp, argv[2]);
                strcat(tmp, "\"");
            }
            strcat(tmp, "\n");
            int to_sv  = open("client_to_server", O_WRONLY);
            write(to_sv, tmp, strlen(tmp));
            close(to_sv);
        }
        _exit(0);
    }
    else {
        if(argc == 1) {
            int from_sv = open("server_to_client", O_RDONLY);
            while ((seen = read(from_sv,linha,input_max_size)) > 0){
                write(1,linha,seen);
            }
            close(from_sv);
        } else {
            int from_sv = open("server_to_client", O_RDONLY);
            seen = read(from_sv,linha,input_max_size);
            write(1, linha, seen);
            close(from_sv);
        }
    }
    return 0;
}
