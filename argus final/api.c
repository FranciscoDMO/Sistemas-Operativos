#include "argus.h"
#include <stdio.h>
#include <unistd.h>  
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h> 
#include <sys/stat.h>



int readline(int fd ,char* line, int size ){
    int r = 0 ;
    int n = 0;
    for (char c ; (n=read(fd, &c, 1)>0) && c!='\n' && r<size; r++) line[r]=c;
    
    if( n==-1) return -1;
    else
        return r;
}


char ** split_line(char * command) {
    //no pior caso cada char é um comando
    char ** commands;
    int size = (strlen(command)/2)+1;
    commands = (char **) malloc(sizeof(char*)*size);

    //criar um delimitador que é um espaço e usuar a strtok para dividir a string
    char delim[] = "\"";
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


void timeupdate(){
    //printf("timestamp: %d\n",ELAPSED_TIME);
    ELAPSED_TIME++;
    alarm(1);
}


void register_ended_task( char * tarefa, char * reason){
    char tmp[200];
    int fd = open("terminadas", O_CREAT | O_APPEND | O_RDWR ,0777 );
    strcpy(tmp, tarefa);
    strcat(tmp, reason);
    write(fd,tmp,strlen(tmp));
    write(fd,"\n",1);
    lseek(fd,0,SEEK_SET);
    close(fd);
}

void register_runing_task(int num, char * tarefa){
    int tam = strlen(tarefa);
    int fd = open("decorrendo", O_CREAT | O_WRONLY, 0777);
    lseek(fd,0,SEEK_END);
    char *new = malloc(tam+200);
    sprintf(new, "#%d: ", num);
    strcat(new, tarefa);
    strcat(new,"\n");
    write(fd,new,strlen(new));
    close(fd);
}

void unregister_runing_task(int num, char * reason){

    
    int fd = open("decorrendo", O_CREAT  | O_RDWR ,0777 );
    int fd2 = open("temp", O_CREAT | O_RDWR, 0777);
    char linha[200];
    int read;

    while(read = readline(fd, linha, 200)) {

       if(atoi(&linha[1]) != num) {
           
            write(fd2, linha, read);
        } else {
            //printf("MATCH\n");
            linha[read] = '\0';
            register_ended_task(linha, reason);
        }
    }
    remove("decorrendo");
    rename("temp", "decorrendo");
    running_pids[num] = 0;
    close(fd);
    close(fd2);
    //printf("index: %d  // reason: %s \n",num,reason);
}

void display_ended(int wr){
   
    int fd = open("terminadas" , O_CREAT|O_RDWR ,0777);
    size_t t = lseek(fd, 0, SEEK_END);
    char *cont = malloc(t);
    lseek(fd, 0, SEEK_SET);
    read(fd, cont, t);
    lseek(fd, 0, SEEK_SET);
    close(fd);
    
    write(wr,cont,t);
    free(cont);
    
}

void display_running(int wr ){

    int fd = open("decorrendo", O_RDONLY, 0777);
   
    size_t t = lseek(fd, 0, SEEK_END) ;
   
    char *cont = malloc(t)  ;
    lseek(fd, 0, SEEK_SET) ;
    read(fd, cont, t );
    close(fd);
    
    write(wr, cont, t);
    free(cont);
    
   
}

void catch_child(){
    pid_t pid;
    int status;
    int i=0;
    
    while ((pid = waitpid(-1, &status, WNOHANG)) != -1) {

        // tratar status para saber como é que a tarefa terminou
        while(running_pids[i] != pid) {
                i++;
        }

        // tratar o status para saber a reason

        if(WEXITSTATUS(status) == 0) {
            
            unregister_runing_task(i, "concluida");
        }
    }
        
    _exit(0);
    
}
