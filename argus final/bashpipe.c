#include <stdio.h>
#include <unistd.h>  
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h> 

//recebe "wc -l | ls -l"
    // ['wc -l', 'ls -l']
        // [['wc', '-l', NULL], ['ls', '-l', NULL]]

char ** split_line(char * command) {
    //no pior caso cada char é um comando
    char ** commands;
    int size = (strlen(command)/2)+1;
    commands = (char **) malloc(sizeof(char*)*size);

    //criar um delimitador que é um espaço e usuar a strtok para dividir a string
    char delim[] = " ";
    char * ptr = strtok(command, delim);
    int counter = 0;

    while(ptr != NULL) {
        //cada posição do array commands vai ser o comando ou um dos argumentos;
        commands[counter] = (char *)malloc(sizeof(char)*(strlen(ptr))+1);
        strcpy(commands[counter], ptr);
        ptr = strtok(NULL, delim);
        counter++;
    }

    commands[counter] = NULL;
    return commands;
}

char ** split_line_pipe(char * command, int * pt) {
    //no pior caso cada char é um comando
    char ** commands;
    int size = (strlen(command)/2)+1;
    commands = (char **) malloc(sizeof(char*)*size);

    //criar um delimitador que é um espaço e usuar a strtok para dividir a string
    char delim[] = "|";
    char * ptr = strtok(command, delim);
    int counter = 0;

    while(ptr != NULL) {
        //cada posição do array commands vai ser o comando ou um dos argumentos;
        commands[counter] = (char *)malloc(sizeof(char)*(strlen(ptr))+1);
        strcpy(commands[counter], ptr);
        ptr = strtok(NULL, delim);
        counter++;
    }

    //commands[counter] = NULL;
    *pt = counter;
    return commands;
}

int main(int argc, char* argv[]) {
    int N; // nº comandos

	// pant[0] = where to read
	// pant[1] = where to write
	int pant[2];
    int pnext[2];
	
    pipe(pant);

    char ** args = split_line_pipe(argv[1], &N);
    char ** cmd[N];
    for(int i = 0; i < N; i++) {
        cmd[i] = split_line(args[i]);
    }

    for(int i = 0 ; i< N; i++){
            if ( i!= N-1 ){
                pipe(pnext);
            }
            if ( fork()==0 ) { 
                if ( i!= 0 ){ // se não é a primeira
                    // meu stdin é stdout da iteracao anterior 
                    dup2(pant[0],0);
                    close(pant[0]);
                }
                if ( i!=N-1){ // se não é a ultima
                    close(pnext[0]);
                    dup2(pnext[1],1);
                    close(pnext[1]);
                }
                
                execvp(cmd[i][0],cmd[i]);
                perror(cmd[i][0]);
                
                _exit(1);
            }  
            else{
                if (i != 0) { // se não é a primeira
                    close(pant[0]);
                } 
                if (i!=N-1) { // se não é a ultima
                    close(pnext[1]);
                    pant[0] = pnext[0];
                }
            }
    }
    for(int i=0; i<N; i++) {
        wait(NULL);
    }
}
