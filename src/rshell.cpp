#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <pwd.h>
#include <uuid/uuid.h>
#include <string.h>
#include <stdio.h>
#include "rshell.h"
using namespace std;

const int arg_max=100;
const int para_max=100;
const int command_max_length=1000;
char command[command_max_length];               //used for storing the command input from user
char *sep = (char*)"\\; " ;                     // @/ =.~`!#$%^*()_+?><[]{}:
char *word, *brkt;                              //used for store the output from strtok_r
char *argv[arg_max];                            //store command one by one
char *parameter[para_max];                      //parameter that will be transmit to execvp
int arg_number;                                 //count the number of args to be proceed
int n;
int status;                                     //return status from child process
bool jumpflag=false;                            //if the flag is set to be true, then skip the
                                                //next arg, for the logic operation
bool operateflag=true;                          //store the status of an execution
void parse(int arg_number);
pid_t waitpid(pid_t pid, int *stat_loc, int options);

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
            for(j=i;j<strlen(command);j++)
            {
                command[j]=' ';
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

void parse(int arg_number)
{
    int i=0;
    int j=0;
    int l=0;
    for (i=0;i<arg_number;i++)
    {
        if (NULL!=strstr(argv[i],"&&"))         //when the arg is "&&", if all of former \
                                                //operations turns out true, not skip next
                                                //arg
        {
            i=i+1;
            if (operateflag==false)
            {
                jumpflag=true;
            }
            else if (operateflag==true)
            {
                jumpflag=false;
            }
        }
 
        else if (NULL!=strstr(argv[i],"||"))    //when the arg is "||", if all of former \
                                                //operation turns  out true, skip next arg
        {
            i=i+1;
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
            jumpflag=false;
            i=i+1;
        }
        else
        {
            l=i;
            parameter[0]=argv[l];
            int k=1;
             while (argv[i+1]!=NULL && NULL==strstr(argv[i+1],"||") && NULL== strstr(argv[i+1],"&&") && (*argv[i+1]-';')!=0)
            {
                parameter[k]=argv[i+1];
                //cout<<parameter[k]<<endl;
                k=k+1;
                i=i+1;
            }
            if (argv[i+1]!=NULL)
            {
                i=i+1;
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
                    if(waitpid(pid,NULL,0)==-1 && (errno != EINTR))
                                                        //parent will wait until child exit
                    {
                        perror("error in waitpid");
                    }
                    if(WIFSIGNALED(status))             //read status from the child process
                    {
                        operateflag=false;
                        cout<<"child process abnormally exited"<<endl;
                    }
                }
                else if(pid == 0)
                {
                    //cout << "I am child" <<pid<<endl;
                    if (execvp(argv[l],parameter)!=0)   //whenever vp runs , it take over
                                                        //forever
                    {
                        perror("error in execvp");
                        operateflag=false;
                        exit(0);                        //if error happens, stop it from being
                                                        //zombie
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
