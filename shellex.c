/* $begin shellmain */
//Derrick Kyereh
#include "csapp.h"
#define MAXARGS   128
/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);
char *name="sh257";
struct timeval current_time;
struct tm *i;
time_t  f;
char time_output[64];
int help() {

    printf("**********************************************************************\n\n");
    printf("A Custom Shell for CMSC 257\n\t");
    printf("- Derrick Kyereh\n");
    printf("Usage:\n\t");
    printf("Use the 'help' command to get the usage\n");
    printf("**********************************************************************\n\n");
    printf("BUILTIN COMMANDS:\n\t");
    printf("-  exit, pid, ppid, cd, help\n" );
    printf("SYSTEM COMMANDS:\n\t");
    printf("- ps -w, ls- l,cd .., date\n");
    return 1;
}
void exitt(){

   
    raise(SIGTERM);
}
int date(){

    gettimeofday(&current_time,NULL);
    f= current_time.tv_sec;
    i = localtime(&f);
    char time_o [100];
    char s[100];
    strcpy(time_o,asctime(i));
    char s1[100];
    char r[20] = "EST ";
    char addY[20];
    int k,o;
    for(o=15,k=0;o<20;o++,k++){
        s[k] = time_o[o];
    }
    strncpy(s1,time_o,15);
    strcat(s1,s);
    for(o=20,k=0;o<24;o++,k++){
        addY[k] = time_o[o];
    }
    strcat(s1,r);
    strcat(s1,addY);
    printf("%s\n",s1);
    printf("Process exited with status code 0\n");
    return 1;
}
int pid(){

    printf("%d\n", getpid());
    return 1;
}
int pspid() {

    printf("%d\n",getppid());
    return 1 ;
}
int cd_null(){
    printf("\n");
    char CurrWorkingDIr[100];
    if(getcwd(CurrWorkingDIr,sizeof(CurrWorkingDIr)) == NULL){
        perror("Error happened\n\n");
    }
    else{
        printf("Current Working directory is: %s\n",CurrWorkingDIr);
    }
    return 1;
}
int ls_l(){
    int p;
    p=fork();
    if(p ==0 ){
        execlp ( "/bin/ls", "ls", "-l", NULL );
        printf("\n");
    }
    else
    {
        wait(NULL);
        printf("\n");
        printf("Process exited with status code 0\n");
    }
    return 1;
}
int ps_w(){

    system("ps w");
    printf("Process exited with status code 0\n");
    return 1;
}
void SH(int sig){
    signal(SIGINT, SH);
    fflush(stdout);
}
int main(int argc, char *argv[])
{
    signal(SIGINT,SH);
    char cmdline[MAXLINE]; /* Command line */
    if(argc == 3){
        if(strcmp(argv[1],"-p") == 0){
            name = argv[2];
        }
    }
    while (1) {
        /* Read */
        printf("%s> ",name);
        Fgets(cmdline, MAXLINE, stdin);
        if (feof(stdin))
            exit(0);

        /* Evaluate */
        eval(cmdline);
    }
}
/* $end shellmain */

/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline)
{
    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */
    strcpy(buf, cmdline);
    bg = parseline(buf, argv);
    if (argv[0] == NULL)
        return;   /* Ignore empty lines */
    if (!builtin_command(argv)) {
        if ((pid = Fork()) == 0) {   /* Child runs user job */
            if (execve(argv[0], argv, environ) < 0) {
                printf("Execution failed (in fork)\n");
                printf("%s: Command not found.\n", argv[0]);
                printf("Process exited with status code 1\n");
                exit(0);
            }
        }
        /* Parent waits for foreground job to terminate */
        if (!bg) {
            int status;
            if (waitpid(pid, &status, 0) < 0)
                unix_error("waitfg: waitpid error");
        }
        else
            printf("%d %s", pid, cmdline);
    }
    return;
}
/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv)
{

    if(!strcmp(argv[0],"echo")){

        int i =1;
        while(!(argv[i]== NULL)){
            printf("%s ",argv[i]);
            i++;
        }
        printf("\n");
        printf("Process exited with status code 0 \n");


        return 1;
    }

    if (!strcmp(argv[0], "date")){
        return date();

    }
    if(!strcmp(argv[0],"ls") && !strcmp(argv[1],"-l")){
        return ls_l();

    }
    if(!strcmp(argv[0],"cd") && argv[1]==NULL){
        return  cd_null();

    }
    if(!strcmp(argv[0],"ps") && !strcmp(argv[1],"w")) {
        return  ps_w();

    }
    if(!strcmp(argv[0],"cd") && argv[1]!=NULL) {
        chdir(argv[1]);
        return 1;
    }
    if (!strcmp(argv[0],"pid")){
        return  pid();

    }
    if(!strcmp(argv[0],"ppid")){
        return  pspid();
    }
    if(!strcmp(argv[0],"help")){
        return  help();

    }
    if (!strcmp(argv[0], "exit")){
        exitt();
    }

    if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
        return 1;
    return 0;                     /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv)
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
        buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[argc] = NULL;

    if (argc == 0)  /* Ignore blank line */
        return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
        argv[--argc] = NULL;

    return bg;
}
/* $end parseline */