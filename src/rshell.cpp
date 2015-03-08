#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <pwd.h>
#include <iomanip>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <dirent.h>
#include "rshell.h"
using namespace std;

const int arg_max=100;
const int para_max=1000;
const int command_max_length=1000;
char command[command_max_length];               //used for storing the command input from user
char *sep = (char*)"\\ " ;                     // @/ =.~`!#$%^*()_+?><[]{}:
char *word, *brkt;                              //used for store the output from strtok_r
char *argv[arg_max];                            //store command one by one
char *parameter[para_max];                      //parameter that will be transmit to execv
char *parameter2[para_max];                     //parameter for the second command
int arg_number;                                 //count the number of args to be proceed
int n;
int status;                                     //return status from child process
bool jumpflag=false;                            //if the flag is set to be true, then skip the
                                                //next arg, for the logic operation
bool change_fdflag=false;
bool change_fd2flag=false;
bool builtin_flag=false;
bool operateflag=true;                          //store the status of an execution
bool pipeflag=false;
bool INTflag=false;                             //used for ^c interrupt
bool TSTPflag=false;
int parse(int arg_number);
bool Triredirectionflag=false;
pid_t waitpid(pid_t pid, int *stat_loc, int options);

char *ENVPATH;
char ENVDIR[100][1000];
int envpath_num=0;

void handler(int i,siginfo_t *info, void *ptr){//handler for the interrupt
    if(i==SIGINT) {
        INTflag=true;
    }
    if(i==SIGTSTP) {
        TSTPflag=true;
    }
}

int builtin_command(const char* cmd,const char* path){
    char pathname[1000];
    if((getcwd(pathname,1000))==NULL)
    {
        perror("error in getcwd");
    }
    if (strcmp(cmd,"cd")==0){
        char cdpath[1000];
        memset(cdpath,'\0',sizeof(cdpath));
        char connect[20]="/";
        if (arg_number>=2){
            if ((path[0]-'/')==0){
                strncat(cdpath,path,strlen(path)+1);
                //cout<<"path="<<cdpath<<endl;
                if (chdir(cdpath)!=0){
                    perror("error in change directory");
                    return 1;
                }
            }
            
            else{
                if ((path[0]-'/')!=0){
                    strcpy(cdpath,pathname);
                    strncat(cdpath,connect,strlen(connect)+1);
                }
                strncat(cdpath,path,strlen(path)+1);
                if (chdir(cdpath)!=0){
                    perror("error in change directory");
                    return 1;
                }
            }
        }
        else {
            cout<<"error in cd"<<endl;
            cout<<"cd usage:"<<endl;
            cout<<"cd [pathname]"<<endl;
            return 1;
        }
    }
    return 0;
}


int getpath(){                                  //read path from env
    char dirpath[1000];
    //cout<<"getpath"<<endl;
    memset(dirpath,'\0',sizeof(dirpath));
    memset(ENVDIR,'\0',sizeof(ENVDIR));
    if (NULL==(ENVPATH=getenv("PATH"))){
        perror("no env named PATH");
    }
    //cout<<"ENVPATH="<<ENVPATH<<endl;
    if (NULL==(getcwd(dirpath,1000))){
        perror("error in get current path");
    }
    int i=0;
    int j=0;
    while(ENVPATH[0]-' '!=0){
        if(ENVPATH[0]-':'==0 && strlen(ENVPATH)>1){
            ENVPATH++;
            //cout<<"ENVDIR["<<i<<"]="<<ENVDIR[i]<<endl;
            i++;
            j=0;
        }
        ENVDIR[i][j]=ENVPATH[0];
        //cout<<"PATHc"<<ENVPATH[0]<<endl;
        //cout<<"ENVDIR[i]"<<ENVDIR[i][j]<<endl;
        j++;
        ENVPATH++;
    }
    //cout<<"ENVDIR["<<i<<"]="<<ENVDIR[i]<<endl;
    envpath_num=i+1;
    strcpy(ENVDIR[i+1],"");
    strcpy(ENVDIR[i],dirpath);
    //cout<<"path number"<<envpath_num<<endl;
    return 0;
}

int findpath(const char *cmd){
    int find_flag=1;
    int i=0;
    char connector[2]="/";
    char execpath[1000];
    memset(execpath,'\0',1000);
    while (i<=envpath_num){
        strncpy(execpath,ENVDIR[i],strlen(ENVDIR[i])+1);
        if (i!=envpath_num){
            strncat(execpath,connector,strlen(connector)+1);
        }
        strncat(execpath,cmd,strlen(cmd)+1);
        //cout<<"path="<<execpath<<endl;
        find_flag=execv(execpath,parameter);
        i++;
        if (find_flag==0)
            break;
    }
    return find_flag;
}

int read_command()                              //divide the command one by one
{
    int i=0;
    int j=0;
    for (i=0;command[i]!='\0';i++)
    {
        if (command[i]==';')                    //if there is a ';' add space before and after
                                                //it to make sure ';' can be divided to one arg
        {
            
            for (j=command_max_length-3;j>i;j--)
            {
                command[j+2]=command[j];
            }
            command[i]=' ';
            command[i+1]=';';
            command[i+2]=' ';
            i=i+2;
        }
        else if (command[i]=='#')
        {
            for(j=i;j<command_max_length;j++)
            {
                command[j]='\0';
            }
        }
        
    }

    for (word = strtok_r(command, sep, &brkt);word;word = strtok_r(NULL, sep, &brkt))
	{
        argv[n]=word;
        n=n+1;                //count the number of args
	}
    arg_number=n;
    return arg_number;

}

int change_fd (const char *redir, const char *file)
{
    if (*redir-'>'==0 && strcmp(redir,">>")!=0){
        int fdout;
        //cout<<"73"<<endl;
        if (-1==(fdout=open(file,O_WRONLY|O_CREAT,0666))){
            perror("ERROR in open");
        }
        if (-1==dup(1)){
            perror ("ERROR in dup");
        }
        if (-1==dup2(fdout,1)){
            perror ("ERROR in dup2");
        }
        //cout<<"78"<<endl;
        if (-1==close(fdout)){
            perror("ERROR in close");
        }
    }
    if (strcmp(redir,"2>")==0){
        int fdout;
        //cout<<"90"<<endl;
        if (-1==(fdout=open(file,O_WRONLY|O_CREAT,0666))){
            perror("ERROR in open");
        }
        if (-1==dup(2)){
            perror ("ERROR in dup");
        }
        if (-1==dup2(fdout,2)){
            perror ("ERROR in dup2");
        }
        //cout<<"100"<<endl;
        if (-1==close(fdout)){
            perror("ERROR in close");
        }
    }
    if (strcmp(redir,">>")==0){
        int fdout;
        //cout<<"107"<<endl;
        if (-1==(fdout=open(file,O_WRONLY|O_APPEND|O_CREAT,0666)))
        {
            perror("ERROR in open");
        }
        //cout<<"fdout"<<fdout<<endl;
        if (-1==dup(1)){
            perror ("ERROR in dup");
        }
        if (-1==dup2(fdout,1)){
            perror ("ERROR in dup2");
        }
        //cout<<"fdout"<<fdout<<endl;
        //cout<<"120"<<endl;
        if (-1==close(fdout)){
            perror("ERROR in close");
        }
    }
    if (strcmp(redir,"2>>")==0){
        int fdout;
        //cout<<"127"<<endl;
        if (-1==(fdout=open(file,O_WRONLY|O_APPEND|O_CREAT,0666)))
        {
            perror("ERROR in open");
        }
        //cout<<"fdout"<<fdout<<endl;
        if (-1==dup(2)){
            perror ("ERROR in dup");
        }
        if (-1==dup2(fdout,2)){
            perror ("ERROR in dup2");
        }
        //cout<<"fdout"<<fdout<<endl;
        //cout<<"140"<<endl;
        if (-1==close(fdout)){
            perror("ERROR in close");
        }
    }

    if (*redir-'<'==0 && strcmp(redir,"<<<")!=0){
        int fdin;
        //cout<<"148"<<endl;
        if (-1==(fdin=open(file,O_RDONLY|O_CREAT,0666)))
        {
            perror("ERROR in open");
        }
        if (-1==dup(0)){
            perror ("ERROR in dup");
        }
        if (-1==dup2(fdin,0)){
            perror ("ERROR in dup2");
        }
        
        //cout<<"159"<<endl;
        if (-1==close(fdin)){
            perror("ERROR in close");
        }
    }
    if(strcmp(redir,"<<<")==0){
        //cout<<"165"<<endl;
        int fd;
        file++;
        char chbuff[BUFSIZ];
        memset(chbuff,'\0',BUFSIZ);
        //const char *enter="\n";
        char pathname[1000];
        //cout<<"chbuff="<<chbuff<<endl;
        strncat(chbuff,file,strlen(file)-1);
        
        //cout<<"inside buffer:"<<chbuff<<endl;
        if (-1==(fd=open("temp",O_RDWR|O_CREAT,0666))){
            perror("error in open temp");
        }
        if (-1==write(fd,chbuff,strlen(chbuff))){
            perror("error in write");
        }
        //strncat(chbuff,enter,1);
        if((getcwd(pathname,1000))==NULL)
        {
            perror("error in getcwd");
        }
        strncat(pathname,"/temp",5);
        parameter[1]=pathname;
        //cout<<"pathname="<<parameter[1]<<endl;
        if (-1==close(fd)){
            perror("error in close");
        }
        //cout<<"chbuff="<<chbuff<<endl;
        //cout<<endl<<"177"<<endl;
    }
    return 0;
}

int parse(int arg_number)
{
    int fd[2];
    if  (pipe(fd)==-1){
        perror("error in pipe");
    }
    int i=0;
    int j=0;
    int l=0;
    char *command[2];
    memset(command,'\0',sizeof(command));
    for (i=0;i<arg_number;i++)
    {
 
        //cout<<"line75 "<< argv[i]<<endl;
        if (NULL!=strstr(argv[i],"&&"))         //when the arg is "&&", if all of former
                                                //operations turns out true, not skip next
                                                //arg
        {
            if (operateflag==false)
            {
                jumpflag=true;
                //cout<<"line 84 jump"<<endl;
            }
            else if (operateflag==true)
            {
                jumpflag=false;
            }
        }
 
        else if (NULL!=strstr(argv[i],"||"))    //when the arg is "||", if all of former 
                                                //operation turns  out true, skip next arg
        {
            if (operateflag==true)
            {
                jumpflag=true;
            }
            else if(operateflag==false)
            {
                jumpflag=false;
            }
        }
        else if ((*argv[i]-';')==0)             //when the arg is ';', execute next arg
        {
            jumpflag=false;        }
        else
        {
            l=i;
            parameter[0]=argv[l];
            int k=1;
            int m=1;
            while (argv[i+1]!=NULL && NULL==strstr(argv[i+1],"||") && NULL== strstr(argv[i+1],"&&") && (*argv[i+1]-';')!=0 && i+1<arg_number )//i+1 exist and not || && ; ,
             {
                if (i+2<arg_number && ((*argv[i+1]-'>')==0 || (*argv[i+1]-'<')==0 || strcmp(argv[i+1],">>")==0 || strcmp(argv[i+1],"2>")==0 || strcmp(argv[i+1],"2>>")==0 || strcmp(argv[i+1],"<<<")==0) && argv[i+1]!=NULL && argv[i+2]!=NULL){
                    change_fdflag=true;//command not from > >> <
                    if(strcmp(argv[i+1],"<<<")==0)
                        Triredirectionflag=true;
                    //cout<<"242"<<endl;
                    i=i+2;
                }
                else if ((*argv[i+1]-'|')==0 && i+2<arg_number){
                    m=1;
                    command[0]=argv[l];
                    command[1]=argv[i+2];
                    parameter2[0]=argv[i+2];
                    
                    i=i+2;
                    //cout<<"command2="<<argv[i]<<endl;
                    //cout<<"arg_number"<<arg_number<<endl;
                    //cout<<"i+1="<<i+1<<endl;
                    while (argv[i+1]!=NULL && i+1<arg_number && NULL==strstr(argv[i+1],"||") && NULL== strstr(argv[i+1],"&&") && (*argv[i+1]-';')!=0 && (*argv[i+1]-'|')!=0){
                        
                        if (i+2<arg_number && ((*argv[i+1]-'>')==0 || (*argv[i+1]-'<')==0 || strcmp(argv[i+1],">>")==0 || strcmp(argv[i+1],"2>")==0 || strcmp(argv[i+1],"2>>")==0 || strcmp(argv[i+1],"<<<")==0) && argv[i+1]!=NULL && argv[i+2]!=NULL){
                            change_fd2flag=true;//command not from > >> <
                            //cout<<"259"<<endl;
                            i=i+2;
                        }
                        else {
                            parameter2[m]=argv[i+1];
                            //cout<<"parameter2["<<m<<"]="<<parameter2[m]<<endl;
                            m=m+1;
                            i=i+1;
                        }
                    }
                    pipeflag=true;
                }
                else {
                    parameter[k]=argv[i+1];
                    //cout<<"parameter"<<k<<"="<<parameter[k]<<endl;
                    k=k+1;
                    i=i+1;
                }
            }
            if (strcmp(argv[l],"cd")==0){
                builtin_flag=true;
            }
            
            
            if (jumpflag==true)
            {
                jumpflag=false;
            }
            else
            {
                int pid =fork();
                if (pid==-1)
                {
                    perror("fork fail");
                }
                else if(pid>0)
                {
                    //cout << "I am parent" <<pid<<endl;
            
                    operateflag=true;
                    if(waitpid(pid,&status,0)==-1 && (errno != EINTR))
                                                        //parent will wait until child exit
                    {
                        perror("error in waitpid");
                    }
                    if(WEXITSTATUS(status))             //read status from the child
                                                            //process
                    {
                        operateflag=false;
                        //cout<<"exit value: "<<WEXITSTATUS(status)<<endl;
                        //cout<<"child process abnormally exited"<<endl;
                    }
                    if(INTflag==true){
                        INTflag=false;
                    }
                    if(TSTPflag==true){
                        TSTPflag=false;
                    }
                    //cout<<"jumpflag="<<jumpflag<<endl;
                    if (jumpflag==false){
                        if (builtin_flag==true){
                            if(1==builtin_command(argv[l],parameter[1])){
                                operateflag=false;
                            }
                            builtin_flag=false;
                        }
                    }
                    //scout<<"status: "<<status<<endl;
                    
                    
                    
                    if (pipeflag==true){
                        int chdpid =fork();
                        if (chdpid==-1)
                        {
                            perror("fork fail");
                        }
                        if (chdpid > 0){
                            if(waitpid(chdpid,&status,0)==-1 && (errno != EINTR))
                                //parent will wait until child exit
                            {
                                perror("error in waitpid chdpid");
                            }
                            if (change_fd2flag==true){
                                change_fd2flag=false;
                            }
                        }
                        else if (chdpid == 0){
                            if (change_fd2flag==true){
                            //    cout<<"323"<<endl;
                            //    cout<<"argv[i-1]"<<argv[i-1]<<"argv[i]"<<argv[i]<<endl;
                                change_fd(argv[i-1],argv[i]);
                            }

                            if (-1==close(0)){
                                perror ("ERROR in close stdout");
                            }
                            if (-1==dup2(fd[0],0)){
                                perror ("ERROR in dup");
                            }
                            //cout<<"fd[0]="<<fd[0]<<endl;
                            //cout<<"stdin="<<fileno(stdin)<<endl;
                            if (-1==close(fd[0])){
                                perror ("ERROR in close stdout");
                            }
                           /*
                            char buf[1024];
                            if(-1==	read(fd[0],buf,sizeof(buf))){
                                perror("read error");
                            }
                            cout<<"buf content = "<<buf<<endl<<endl;
                            */
                            if (builtin_flag==true){
                                exit(0);
                            }
                            else{
                                int exec_flag=1;
                                exec_flag=findpath(argv[l]);
                                if (exec_flag!=0)   //whenever vp runs , it take over
                                    //forever
                                {
                                    perror("error in execv");
                                    operateflag=false;
                                    exit(7);                        //if error happens, stop it from                        being
                                    //zombie
                                }
                            }
                            /*else
                                exit(0);
                            if (-1==close(fd[0])){
                                perror ("ERROR in close");
                            }
                            if (-1==close(fd[1])){
                                perror ("ERROR in close");
                            }*/
                        }//detect pipe
                        
                    }//command2
                    
                    
                    if (change_fdflag==true)
                        change_fdflag=false;
                    if (pipeflag==true){
                        pipeflag=false;
                        for (j=1;j<m;j++)
                        {
                            parameter[j]=NULL;
                        }
                        
                    }
                }//rshell
                
                
                else if(pid == 0)
                {
                    //cout << "I am child" <<pid<<endl;
                    if (INTflag==true){
                        exit(0);
                        INTflag=false;
                    }
                   
                    if(TSTPflag==true){
                        int abspid;
                        if(-1==(abspid=getpid())){
                            perror("error in getpid");
                        }
                        cout<<"pid to be kill="<<abspid<<endl;
                        if(-1==kill(abspid,SIGSTOP)){
                            perror("error in kill");
                        }
                        
                        TSTPflag=false;
                    }

                    
                    if (change_fdflag==true){
                        //cout<<"384"<<endl;
                        //cout<<"argv[i-1]"<<argv[i-1]<<"argv[i]"<<argv[i]<<endl;
                        change_fd(argv[i-1],argv[i]);
                    }
                    
                    if (pipeflag==true){
                        if (-1==close(1)){
                            perror ("ERROR in close stdout");
                        }
                        if (-1==dup2(fd[1],1)){
                            perror ("ERROR in dup");
                        }
                        if (-1==close(fd[1])){
                            perror ("ERROR in close stdout");
                        }
                        //cout<<"fd[1]="<<fd[1]<<endl;
                        //cout<<"stdout="<<fileno(stdout)<<endl;
                        if (builtin_flag==true){
                            exit(0);
                        }
                        else{
                            int exec_flag=1;
                            exec_flag=findpath(argv[l]);
                            if (exec_flag!=0)   //whenever vp runs , it take over
                                //forever
                            {
                                perror("error in execv");
                                operateflag=false;
                                exit(7);                        //if error happens, stop it from                        being
                                //zombie
                            }
                        }
                    }//command1
                    
                    else {
                        /*cout<<"413"<<endl;
                        cout<<"argv[l]:"<<argv[l]<<endl;
                        cout<<"parameter="<<parameter[1]<<endl;*/
                        if (builtin_flag==true){
                            exit(0);
                        }
                        else{
                            int exec_flag=1;
                            exec_flag=findpath(argv[l]);
                            if (exec_flag!=0)   //whenever vp runs , it take over
                                                                //forever
                            {
                                perror("error in execv");
                                operateflag=false;
                                exit(7);                        //if error happens, stop it from                        being
                                                            //zombie
                            }
                        }
                     
                    }//exec
                
                
                }//command1 or exec
                
                for (j=1;j<k;j++)
                {
                    parameter[j]=NULL;
                }
            }
        }
    }
    if (Triredirectionflag==true){
        Triredirectionflag=false;
        cout<<endl;
        if(-1==remove("temp")){
            perror("error in remove");
        }
    }
    return 0;
}

int main()
{
    getpath();
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = handler;
    act.sa_flags = SA_SIGINFO;
    if(-1==sigaction(SIGINT,&act,NULL)){
        perror("error in sigaction");
    }
    if(-1==sigaction(SIGTSTP,&act,NULL)){
        perror("error in sigaction");
    }
    //cout<<"before cmd"<<endl;
	while(strncmp(command,"exit",command_max_length)!=0)
	{
        if (INTflag==true){
            exit(0);
            INTflag=false;
        }
        if(TSTPflag==true){
            int pid;
            if(-1==(pid=getpid())){
                perror("error in getpid");
            }
            if(-1==kill(pid,SIGSTOP)){
                perror("error in kill");
            }
            TSTPflag=false;
        }
        
        type_prompt();
        cin.getline(command, command_max_length, '\n');
        if(strncmp(command,"exit",command_max_length)==0)
        {
            //cout<<strncmp(command,"exit",command_max_length)<<endl;
            break;
        }
        n=0;
        arg_number=read_command();
        parse(arg_number);
        for (int i=0;i<command_max_length;i++)
        {
            command[i]='\0';
        }
    }
	return 0;
}
