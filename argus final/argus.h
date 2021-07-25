#define MAX 1024 
int ELAPSED_TIME;
int MAX_TIME;
int running_pids[MAX];
int saved ;
int ended_tasks ;
int stall_time ;

extern int readline(int fd ,char* line, int size );
extern char ** split_line(char * command) ;
extern void timeupdate();
extern void register_ended_task( char * tarefa, char * reason) ;
extern void register_runing_task(int num, char * tarefa) ;
extern void unregister_runing_task(int num, char * reason) ;
extern void display_ended(int wr) ;
extern void display_running(int wr ) ;
extern void catch_child() ;


