#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
//#include <uuid/uuid.h>
#include <time.h>
#include <dirent.h>
#include <iomanip>
#include <string.h>
// must use valgrind and cppcheck and pass without bugs
using namespace std;

int print_file(int lflag,char *pathname,char *filename){
    struct stat s;
    struct passwd *pwd;
    struct group *grp;
    struct tm *clk;
    time_t time;
    int dflag=0;
    //char *blue=(char*)"\x1b[31m";
    //char *green=(char*)"\x1b[0;32m";
    //char *grey=(char*)"\x1b[30;1m";
    if(stat(pathname,&s)==-1)
    {
        perror("error in stat");
    }
    else{
        if (NULL==getpwuid(s.st_uid))
        {
            perror("error in getpwuid");
        }
        else
            pwd = getpwuid(s.st_uid);
        if (NULL==getgrgid(s.st_gid))
        {
            perror("error in getgrgid");
        }
        else
            grp = getgrgid(s.st_gid);
        //	cout << oct;
        //	cout<<s.st_mode<<endl;
        
        cout.fill(' ');
        switch(s.st_mode & S_IFMT)
        {
            case S_IFSOCK:if (lflag==1)
                cout<<"s";break;
            case S_IFLNK :if (lflag==1)
                cout<<"l";break;
            case S_IFDIR :if (lflag==1)
                cout<<"d";dflag=1;break;
            case S_IFCHR :if (lflag==1)
                cout<<"c";break;
            case S_IFBLK :if (lflag==1)
                cout<<"b";break;
            case S_IFREG :if (lflag==1)
                cout<<"-";break;
            case S_IFIFO :if (lflag==1)
                cout<<"p";break;
            default      :break;
        }
        if (lflag==1){
            
            cout<<((s.st_mode&S_IRUSR)?"r":"-");
            cout<<((s.st_mode&S_IWUSR)?"w":"-");
            cout<<((s.st_mode&S_IXUSR)?"x":"-");
            
            cout<<((s.st_mode&S_IRGRP)?"r":"-");
            cout<<((s.st_mode&S_IWGRP)?"w":"-");
            cout<<((s.st_mode&S_IXGRP)?"x":"-");
            
            cout<<((s.st_mode&S_IROTH)?"r":"-");
            cout<<((s.st_mode&S_IWOTH)?"w":"-");
            cout<<((s.st_mode&S_IXOTH)?"x":"-");
            
            cout<<setw(3)<<right<<s.st_nlink<<" ";
            cout<<setw(7)<<left<<pwd->pw_name;
            cout<<setw(5)<<left<<grp->gr_name;
            cout<<setw(6)<<right<<s.st_size<<" ";
            
            time=s.st_mtime;
            clk=localtime(&time);
            switch (clk->tm_mon+1)
            {
                case 1 :cout<<"Jan";break;
                case 2 :cout<<"Feb";break;
                case 3 :cout<<"Mar";break;
                case 4 :cout<<"Apr";break;
                case 5 :cout<<"May";break;
                case 6 :cout<<"Jun";break;
                case 7 :cout<<"Jul";break;
                case 8 :cout<<"Aug";break;
                case 9 :cout<<"Sep";break;
                case 10:cout<<"Oct";break;
                case 11:cout<<"Nov";break;
                case 12:cout<<"Dec";break;
            }
            cout<<setw(3)<<right<<clk->tm_mday<<" ";
            cout.fill('0');
            cout<<setw(2)<<clk->tm_hour<<":"<<setw(2)<<clk->tm_min<<" ";
        }
        cout.fill(' ');
        if (filename[0]=='.'){
                cout<<"\x1b[37m"<<setw(12)<<left<<filename;
                cout<<"\x1b[0m";
            }
        else if ((s.st_mode & S_IFMT) == S_IFREG && (s.st_mode&S_IXOTH) == 1){
            cout<<"\x1b[0;32m"<<setw(12)<<left<<filename;
            cout<<"\x1b[0m";
        }
        else if((s.st_mode & S_IFMT) == S_IFDIR){
            cout<<"\x1b[1;29m"<<setw(12)<<left<<filename;
            cout<<"\x1b[0m";
        }
       else
            cout<<setw(12)<<left<<filename;
    }
    if (lflag == 1)
        cout<<endl;
    return dflag;
}

void print_al_info(int lflag, int aflag, int Rflag, char *pathname){
    DIR *dir;
    //int path_length=1000;
    //char pathname[path_length];
    //cout<<"I am in folder"<<pathname<<endl;
    struct dirent *pdir;
    int i=0,j=0;
    int dflag=0;
    char *chpathname;
    char temp[1000];
    char connector[2]="/";
    char list_of_cpath[20][1000];
    char list_of_cname[20][1000];
    int dirno[20];
    //cout<<pathname<<endl;
    if(NULL==(dir=opendir(pathname)))
    {
        perror("error in opendir");
    }
    while ((pdir=readdir(dir))!=NULL)
    {
        strcpy(temp,pathname);
        chpathname=strcat(temp,connector);
        strcpy(list_of_cname[i],pdir->d_name);
        //cout<<pdir->d_name<<endl;
        strcpy(list_of_cpath[i],strcat(chpathname,pdir->d_name));
        for (j=0; j<1000; j++)
        {
            temp[j]='\0';
        }
        if (aflag==0)
        {
            if(pdir->d_name[0]=='.'){
                i++;
                continue;
            }
        }
        //if (lflag==1)
        //{
        dflag=print_file(lflag,list_of_cpath[i],list_of_cname[i]);
        //}
        /*else
        {
            dflag=print_file(lflag,list_of_cpath[i],list_of_cname[i]);
            cout.fill(' ');
            cout<<setw(22)<<left<<pdir->d_name;
            if (i%3==0)
                cout<<endl;
        }*/
        if (Rflag==1 && dflag==1)
        {
            if(pdir->d_name[0]=='.'){
                i++;
                continue;
            }
            /*strcpy(temp,pathname);
             chpathname=strcat(temp,connector);
             strcpy(list_of_cname[j],pdir->d_name);
             //cout<<list_of_cname[j]<<endl;
             strcpy(list_of_cpath[j],strcat(chpathname,pdir->d_name));
             */
            //cout<<list_of_cpath[j]<<endl;
            dirno[i]=1;
        }
        i++;
    }
    if (lflag==0)
        cout<<endl;
    if (closedir(dir)==-1)
    {
        perror("error in closedir");
    }
    if (list_of_cpath!=NULL)
    {
        for (i=0;i<10;i++ )
        {
            if(dirno[i]==1){
                dirno[i]=0;
                cout<<endl<<"./"<<list_of_cname[i]<<":"<<endl;
                print_al_info(lflag,aflag,Rflag,list_of_cpath[i]);
            }
        }
    }
    
    return;
}

int main(int argc, char *argv[]){
    int Rflag=0;
    int aflag=0;
    int lflag=0;
    char *sep = (char*)"-alR" ;                     // @/ =.~`!#$%^*()_+?><[]{}:
    char *word, *brkt;
    char *temppathname;
    char connector[2]="/";
    char temp[1000];
    //DIR *dir;
    //int path_length=1000;
    
    char pathname[1000];
    if (NULL==getcwd(pathname,1000))
    {
        perror("error in getcwd");
    }
    /*if(NULL==(dir=opendir(pathname)))
     {
     perror("error in opendir");
     }*/
    
    //int argc=2;
    //char *argv[2]={(char*)"ls",(char*)"-alR"};
    
    if (argc < 1){
        perror("not enough input arguments");
        return -1;
    }
    else if (argc >1){
        for (int i=1;i<argc;i++){
            if (argv[i][0]=='-' && strlen(argv[i])<5){
                if (strstr(argv[i],"a")!=NULL)
                    aflag=1;
                if (strstr(argv[i],"l")!=NULL)
                    lflag=1;
                if (strstr(argv[i],"R")!=NULL)
                    Rflag=1;
                if (NULL!=(word = strtok_r(argv[i], sep, &brkt))){
                    cout<<"ls: illegal option -- 2"<<endl;
                    cout<<"usage: ls [-alR] [file ...]"<<endl;
                }
            }
            else if (argv[i][0]!='-'){
                strcpy(temp,pathname);
                temppathname=strcat(temp,connector);
                strcpy(temppathname,argv[i]);
                print_file(lflag,temppathname,argv[i]);
                if (lflag==0)
                    cout<<endl;
                if (i<argc-1){
                    if (argv[i+1][0]=='\0')
                        exit(0);
                    if (argv[i+1][0]=='-'){
                        cout<<"usage: error format"<<endl;
                        exit(0);
                    }
                }
                else if (i==argc-1)
                    exit(0);
            }
            else{
                cout<<"ls: illegal option -- 2"<<endl;
                cout<<"usage: ls [-alR] [file ...] rshell"<<endl;
                return -1;
            }
        }
    }
    
    print_al_info(lflag,aflag,Rflag,pathname);
    
    
    return 0;
    
}
