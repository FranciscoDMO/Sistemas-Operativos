#include <stdio.h>
#include <unistd.h>  
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h> 
#include <sys/stat.h>
#include "argus.h"

int main( int argc , char * argv[]) {
    mkfifo("client_to_server",0777);
    mkfifo("server_to_client",0777);
    char help[] = {"Modo de uso:\nOpção <-i> <n> para definir um tempo máximo de inatividade de comunicação num pipe anónimo.\nOpção <-m> <n> para definir o tempo máximo de execução de uma tarefa.\nOpção <-e> <\"p1 | p2 ... | pn\"> para executar uma tarefa.\nOpção <-l> para listar as tarefas em execução.\nOpção <-t> <n> para terminar uma tarefa em execução.\nOpção <-r> para listar o registo histórico de tarefas terminadas.\nOpção <-h> para abrir este menu de ajuda.\n"};
    // tempo maximo de inactividade
    stall_time = 5 ;
    // var para saber o tempo de execucao
    ELAPSED_TIME=0;
    // numero de bytes que o read vai ler de cada vez.
    int input_max_size = 100;
    // buffer para o read.
    char* linha = malloc(input_max_size);
    // este processo so serve para contar o tempo
    int finished = 1;
    // este processo vai executar as coisas chamadas com a tag '-e'
    pid_t runing;
    // argumentos  
    char ** args;
    // serve para valor de retorno do read para sabermos quantos bytes foram lidos.
    int seen;

    saved=0;
    MAX_TIME = 5;

    int rd = open("client_to_server",O_RDONLY);
    int wr = open("server_to_client",O_WRONLY);
    while((seen=read(rd,linha,input_max_size)) > 0) {
        close(rd);
        char * copia = malloc(seen);
        strcpy(copia,linha);
        int size_input = strlen(linha);
        
        args = split_line(linha);
        //tive que apagar um espaco a mais que tinha no primeiro argumento disto.
        
        if(strlen(args[0])>2)
            args[0][strlen(args[0])-1] = '\0';
        
        if (strcmp(args[0], "-l") == 0) {
            display_running(wr);
            write(wr, "\n", 1);
        } else if (strcmp(args[0], "-i") == 0){
            stall_time=atoi(args[1]);
        } else if (strcmp(args[0], "-r") == 0){
            display_ended(wr);
            write(wr, "\n", 1);
        } else if ( strcmp(args[0], "-m" ) == 0) {
            MAX_TIME = atoi(args[1]);
            write(wr, "\n", 1);
        
        } else if (strcmp(args[0], "-h") == 0) {
            write(wr, help, strlen(help));
        } else if (strcmp(args[0], "-t") == 0) {
            kill(running_pids[atoi(args[1])], SIGTERM);
            unregister_runing_task(atoi(args[1]), " killed by user ");
            write(wr, "\n", 1);
        } else if (strcmp(args[0], "-e")==0 ){
            int pipe_fd[2];
            pipe(pipe_fd);
            if((finished = fork()) == 0){
                if ((runing = fork()) == 0){
                    close(pipe_fd[0]);
                    close(pipe_fd[1]);
                    char * argsfinal[3] = 
                        {
                            "./bashpipe", 
                            args[1],
                            NULL
                        };
                    
                    
                    if(execv(argsfinal[0], argsfinal) < 0) {
                        perror("Comando errado");
                    }

                    _exit(1);

                } else { 
                    
                    close(pipe_fd[0]);
                    write(pipe_fd[1], &runing, sizeof(runing));
                    close(pipe_fd[1]);
                    running_pids[saved] = runing;
                    signal(SIGALRM,timeupdate);
                    signal(SIGCHLD, catch_child);                    
                    alarm(1);
                    while(ELAPSED_TIME < MAX_TIME && (running_pids[saved] != 0) ){    
                        pause();                        
                    }
                    kill(runing,SIGTERM);
                    unregister_runing_task(saved, "timed out");

                    _exit(0);
                }
            } else {
                close(pipe_fd[1]);
                int pid;
                read(pipe_fd[0], &pid, sizeof(runing));
                close(pipe_fd[0]);
                running_pids[saved] = pid;
                register_runing_task(saved, args[1]);
                saved++;
                write(wr, "\n", 1);
            }
        } else {
            perror("Comando não reconhecido. Corra -h para ajuda.\n");  
        }
        rd = open("client_to_server",O_RDONLY);
    }
    close(wr);
    close(rd);
    return 0;
}
