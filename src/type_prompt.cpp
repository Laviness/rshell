#include "rshell.h"

const int path_length=1000;
const int host_length=100;

void type_prompt(void)
{
	char pathname[path_length];
	char hostname[host_length];
	struct passwd *pwd=NULL;
	if (NULL==getpwuid(getuid()))
    {
        perror("error in getpwuid");
    }
    else
        pwd = getpwuid(getuid());
	if((getcwd(pathname,path_length))==NULL)
    {
        perror("error in getcwd");
    }
	if(gethostname(hostname,host_length)==-1)
	{
		perror("gethostname fail");
	}
	unsigned int dirlength=strlen(pwd->pw_dir);
	if (dirlength >= strlen(pathname))
	{
		cout<<"[rshell]"<<pwd->pw_name<<"@"<<hostname<<":"<<pwd->pw_dir<<"$ ";
	}
	else 
	{
		
		cout<<"[rshell]"<<pwd->pw_name<<"@"<<hostname<<":~"<<pathname+dirlength<<"$ ";
	}
	return;		
}

//int main()
//{
//	type_prompt();
//	return 0;
//}
