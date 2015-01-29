#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <pwd.h>
#include <uuid/uuid.h>
#include <string.h>
#include <stdio.h>
#include "read_command.h"
using namespace std;

const int arg_max=100;
const int para_max=100;
const int exec_max=100;
const int command_max_length=1000;
char command[command_max_length];
char *sep = (char*)"\\; " ;// @/ =.~`!#$%^*()_+?><[]{}:
char *word, *brkt;
char *argv[arg_max];
char *parameter[para_max];
char *exec[exec_max];
int arg_number;
int n;
int status;
bool jumpflag=false;
bool operateflag=true;
void parse(int arg_number);
pid_t waitpid(pid_t pid, int *stat_loc, int options);

int read_command()
{
	for (word = strtok_r(command, sep, &brkt);word;word = strtok_r(NULL, sep, &brkt))
	{
        argv[n]=word;
        n=n+1;
	}
    arg_number=n;
    return arg_number;
}

void parse(int arg_number)
{
    int i=0;
    int j=0;
    int l=0;
    for (i=0;i<arg_number;i++)
    {
        if (NULL!=strstr(argv[i],"&&"))
        {
            if (operateflag==false)
            {
                jumpflag=true;
            }
            else if (operateflag==true)
            {
                jumpflag=false;
            }
        }
 
        else if (NULL!=strstr(argv[i],"||"))
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

        else
        {
            l=i;
            parameter[0]=argv[l];
            int k=1;
            while (argv[i+1]!=NULL && *argv[i+1]-'-'==0)
            {
                parameter[k]=argv[i+1];
                //cout<<parameter[k]<<endl;
                k=k+1;
                i=i+1;
            }
            if (jumpflag==true)
            {
                jumpflag=false;
            }
            else
            {
                /*if(signal(SIGCHLD,SIG_IGN) == SIG_ERR)  // stop the child process being zombie
                {
                    perror("signal error");
                    exit(EXIT_FAILURE);
                }
                */
                int pid =fork();
                if (pid==-1)
                {
                    perror("fork fail");
                }
                else if(pid>0)
                {
                    //cout << "I am parent" <<pid<<endl;
                    operateflag=true;
                    if(waitpid(pid,NULL,0)==-1 && (errno != EINTR))
                    {
                        perror("error in waitpid");
                    }
                    if(WIFSIGNALED(status))
                    {
                        operateflag=false;
                        cout<<"wrong"<<endl;
                    }
                }
                else if(pid == 0)
                {
                    //cout << "I am child" <<pid<<endl;
                    if (execvp(argv[l],parameter)!=0) //whenever vp runs , it take over forever
                    {
                        perror("error in execvp");
                        operateflag=false;
                    }
                    
                }
                
            }
            for (j=1;j<k;j++)
            {
                parameter[j]=NULL;
            }
        }
    }
    return;
}

int main()
{
	while(strncmp(command,"exit",command_max_length)!=0)
	{
        type_prompt();
        cin.getline(command, command_max_length, '\n');
        if(strncmp(command,"exit",command_max_length)==0)
        {
            cout<<strncmp(command,"exit",command_max_length)<<endl;
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
