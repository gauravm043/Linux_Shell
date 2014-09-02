/*                                              Code Developed By:-
 *                                                 Gaurav Mishra
 *                                                IIIT-HYDERABAD
 *                                                                                                                                        */


// This code snippet is for implementing the shell in linux using system calls and concepts of processes

#include<stdio.h>
#include<stdlib.h>
#include<malloc.h>
#include<ctype.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<pwd.h>
#include<fcntl.h>

#define size 100


// Global variables declared
int fore_pid=-1;
char home[size];
int n_p=0;


// Structure for Background Processes
typedef struct ll
{
    int pid;
    char name[50];
}
node;


node back_process[100];
char all[32769][100];


void insert(int y,char**x,char*a);

// String strtok function for comparing pwd and home
char*my_strtok(char*a,char*b)
{
    while(*a++ == *b++)
        ;
    return --a;
}

// Function for printing the username and hostname of system that is the terminal
void print_terminal(char * home,char* pwd)
{
    char display[size];
    char host[size];
    gethostname(host,size);
    int p=geteuid();
    struct passwd *x=getpwuid(p);
    char *token;
    if((strcmp(pwd,home)==0))
    {
        sprintf(display,"<%s@%s:~>",x->pw_name,host);
        write(1,display,strlen(display));
    }
    else if(strncmp(pwd,home,strlen(home))==0)
    {
        token=my_strtok(pwd,home);
        sprintf(display,"<%s@%s:~/%s>",x->pw_name,host,token+1);
        write(1,display,strlen(display));
        return;
    }
    else
    {
        sprintf(display,"<%s@%s:~%s>",x->pw_name,host,pwd);
        write(1,display,strlen(display));
    }
}

/*Parsing function (It Separates the arguments from original command) */
int set_argv(char*command,char**my_argv)
{
    int count=0;
    while(*command!='\0')
    {
        while(*command==' '||*command=='\t'||*command=='\n')
            *command++='\0';
        *my_argv++ = command;
        count++;
        while(*command!=' '&& *command!='\t'&& *command!='\n'&& *command!='\0')
            command++;
    }
    *my_argv='\0';
    return count;
}

// Function for executing the command
void execute_command(char**my_argv,int mode,char show[])
{
    pid_t pid,process;
    int status,m,x;

    char pwd[size];
    char dis[size];
    pid=fork();
    strcpy(all[pid],my_argv[0]);
    process=pid;

    if(mode==1) // Foreground process
    {
        fore_pid=process;//Global variable being equated
        waitpid(process,&status,WUNTRACED);//Handling suspended signal
        if(WIFSTOPPED(status))
        {
            insert(process,my_argv,show);
        }

        process=0;
    }
    if(mode==0)// Background Process
    {
        insert(process,my_argv,show);
    }
    if(pid==0)// Child Process execution
    {
        execvp(*my_argv,my_argv);
        write(1,"Error No such command exist\n",28);
        exit(0);
    }
    return;
}

/* Insert function for our background queue*/
void insert(int pid,char**my_argv,char show[])
{
    if(pid!=0)
    {
        back_process[n_p].pid=pid;
        strcpy(back_process[n_p].name,show);
        n_p++;
    }
    return;

}

/* Showing active jobs on system*/
void show_jobs()
{
    int i;
    char dis[100];
    for(i=0;i<n_p;i++)
    {
        sprintf(dis,"[%d] ",i+1);
        write(1,dis,strlen(dis));
        sprintf(dis," %s ",back_process[i].name);
        write(1,dis,strlen(dis));
        sprintf(dis," [%d]\n",back_process[i].pid);
        write(1,dis,strlen(dis));
    }
}

/* When background process exits or converted to foreground remove it from the queue*/
void delete_queue(int pid)
{
    int i,j;
    for(i=0;i<n_p;i++)
        if(back_process[i].pid==pid)
            break;
    for(j=i;j<n_p-1;j++)
    {
        back_process[j].pid=back_process[j+1].pid;
        strcpy(back_process[j].name,back_process[j+1].name);
    }
    n_p--;
}

/* Function written for handling signals*/
void sig_handler(int signo)
{
    char sig[100];
    if (signo == SIGTSTP)/* ctrl+z */
    {
        sprintf(sig,"Receieved SIGTSTP\n");
        if(fore_pid>0)
        kill(fore_pid,SIGTSTP);
    }
    else if (signo == SIGINT)//ctrl+c
    {
        sprintf(sig,"Receieved SIGINT\n");
        write(1,sig,strlen(sig));
    }
    else if (signo == SIGCHLD)// Child exits
    {
        if(fore_pid!=-1)
        {
    //    sprintf(sig,"Receieved SIGCHLD\n");
     //   write(1,sig,strlen(sig));
        }
    }
    else if (signo == SIGQUIT)//ctrl+d
    {
        sprintf(sig,"Receieved SIGQUIT\n");
        write(1,sig,strlen(sig));
    }
}

/* For sending kill signal to the job */
void k_job(char**my_argv)
{
    int x=(int)(atoi(my_argv[1]));
    x--;
    if(n_p<=x || x<0)
    {
        printf("No such job exists\n");
        return;
    }
    int pid=back_process[x].pid;
    int ret=kill(pid,atoi(my_argv[2]));
}

// Kill all processes at once
void over_kill()
{
    int i,pid,ret;
    for(i=0;i<n_p;i++)
    {
        pid=back_process[i].pid;
        ret=kill(pid,9);
    }
}

/* Bringing background processes to foreground*/
void implement_fg(char**my_argv)
{
    int st;
    int x=atoi(my_argv[1]);
    x--;
    if(n_p<=x|| x<0)
    {
        printf("No such process exists\n");
        return;
    }
    int pid=back_process[x].pid;
    printf("Waiting for Child process %d to exit\n",pid);
    delete_queue(pid);
    kill(pid,SIGCONT);
    waitpid(pid,&st,0);
}

/* This function will show the informations of given pid*/
void show_status(int count,char**my_argv,char*home)
{
    size_t len = 0;
    ssize_t read;
    char file[size];
    char*state=NULL;
    char*virtual=NULL;
    int bytes;
    char*token;
    int pid=getpid();
    if(count>=2)
        pid=atoi(my_argv[1]);

    sprintf(file,"/proc/%d/status",pid);

    FILE*fp;
    fp=fopen(file,"r");
    if(fp==NULL)
    {
        printf("No such pid exists\n");
        return;
    }
    getline(&state,&len,fp);
    getline(&state,&len,fp);

    printf("\nPid:%d\n",pid);
    printf("Process Status%s",state+5);

    while ((read = getline(&virtual, &len, fp)) !=-1) //Reading till EOF
    {
        if(strncmp(virtual,"VmSize",6)==0)
        {
            printf("Virtual Memory in use%s",virtual+6);
            break;
        }
    }
    fclose(fp);
    sprintf(file,"/proc/%d/exe",pid);
    char executable[size]="";
    bytes=readlink(file,executable,size);
    executable[bytes]='\0';
    if(strncmp(executable,home,strlen(home))==0)
    {
       token=my_strtok(executable,home);
       printf("Executable Path:    ~/%s\n\n",token+1);
    }
    else
       printf("Executable Path:    %s\n\n",executable);
}

void delete_spaces(char*command)
{
    while(command--)
    {
        if(*command==' ' || *command=='\t')
            *command='\0';
        else 
            break;
    }
}

int main(int argc,char*argv[])
{
    n_p=0;
    int i,count,status,pid;
    char pwd[size] ;
    char sig[size];
    char show[size];
    getcwd(home,size);
    while(1)
    {
        fore_pid=-1;
        if (signal(SIGINT, sig_handler) == SIG_ERR)
        {
            sprintf(sig,"\ncan't catch SIGINT\n");
            write(1,sig,strlen(sig));
        }
        else if (signal(SIGCHLD, sig_handler) == SIG_ERR)
        {
            sprintf(sig,"\ncan't catch SIGCHLD\n");
            write(1,sig,strlen(sig));
        }
        else if (signal(SIGQUIT, sig_handler) == SIG_ERR)
        {
            sprintf(sig,"\ncan't catch SIGQUIT\n");
            write(1,sig,strlen(sig));
        }
        else if (signal(SIGTSTP, sig_handler) == SIG_ERR)
        {
            sprintf(sig,"\ncan't catch SIGTSTP\n");
            write(1,sig,strlen(sig));
        }
        pid=waitpid(-1, &status, WNOHANG);
        while (pid > 0)
        {
            delete_queue(pid);
            if(WIFEXITED(status))
                printf("%s with pid %d exited Normally\n",all[pid], pid);
            else if(WIFSIGNALED(status))
                printf("%s with pid %d exited By signal Number %d\n",all[pid], pid,WTERMSIG(status));

            pid=waitpid(-1, &status, WNOHANG);
        }
        char get_command[size];
        char * my_argv[size];
        getcwd(pwd,size);
        print_terminal(home,pwd);
        i=0;
        while(get_command[i-1]!='\n')
        {
            read(0,&get_command[i],1);
            i++;
        }
        get_command[i-1]='\0';
        if(get_command[0]=='\0')
        {
            continue;
        }
        strcpy(show,get_command);
        delete_spaces(get_command+strlen(get_command));
        if(strlen(get_command)==0)
        {
            printf("Please enter a command\n");
            continue;
        }
        count=set_argv(get_command,my_argv);

        if(!strcmp(*my_argv,"jobs"))
        {
            show_jobs();
            continue;
        }
        else if(!strcmp(*my_argv,"pinfo"))
        {
            if(count>2)
            {
                printf("Please specify atmost 1 argument\n");
                continue;
            }
            show_status(count,my_argv,home);
            continue;
        }
        else if(!strcmp(*my_argv,"fg"))
        {
            if(count!=2)
            {
                printf("Please specify 1 argument\n");
                continue;
            }
            implement_fg(my_argv);
            continue;
        }
        else if(!strcmp(*my_argv,"overkill"))
        {
            if(count>=2)
            {
                printf("Please do not specify any arguments\n");
                continue;
            }
            over_kill();
            continue;
        }
        else if(!strcmp(*my_argv,"kjob"))
        {
            if(count!=3)
            {
                printf("Please specify exactly 2 arguments\n");
                continue;
            }
            k_job(my_argv);
            continue;
        }
        else if(!strcmp(*my_argv,"cd"))
        {

            if(count>2)
            {
                printf("Please specify atmost 1 argument\n");
                continue;
            }
            if(count==1)
                chdir(home);
            else if(strcmp(my_argv[1],"~")==0)
                chdir(home);
            else if(count>1)
                chdir(my_argv[1]);
            else
                chdir(home);
            continue;
        }
        else if(!strcmp(my_argv[count-1],"&"))
        {
            my_argv[count-1]='\0';
            execute_command(my_argv,0,show);
        }
        else if(!strcmp(*my_argv,"quit"))
        {
            return 0;
        }
        else
            execute_command(my_argv,1,show);
    }
    return 0;
}
