#include <iostream>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <pwd.h>

// must use valgrind and cppcheck and pass without bugs
using namespace std;


void cp(const char *srcpath,const char *dstpath,int tflag){
    DIR *dir;
    struct dirent *dstdir;
    char srcname[1000];
    //std::istream::get and std::ostream::put
    char ch;
    //read and write
    char buffer[BUFSIZ];
    char chbuff[1];
    long bytes_read;
    long bytes_write;
    
    //time.h
    clock_t start, finish;
    double   duration;
    
    
    char temp[1000];
    char temppath[1000];
    struct stat s;
    char *p = strrchr(srcpath, '/');
    if(stat(srcpath,&s)==-1)
    {
        perror("error in stat:");
        return;
    }
    p++;
    strcpy(srcname,(char*)p);
    
    
    
    if(NULL==(dir=opendir(dstpath)))
    {
        perror("error in opendir");
        return;
    }
    if(NULL==(dstdir=readdir(dir)))
    {
        perror("error in readdir");
    }
    while ((dstdir=readdir(dir))!=NULL)
    {
        if(dstdir->d_name[0]=='.')
            continue;
        if (0 == strcmp(dstdir->d_name,srcname)){
            cout<<"ERROR:Exist file"<<endl;
            exit(0);
        }
    }
    strcpy(temp,(char*)"/");
    strcat(temp,srcname);
    strcpy(temppath,dstpath);
    strcat(temppath,temp);
    
    //  c++ style open file
    ofstream outfile (temppath,ios::binary);
    ifstream infile (srcpath,ios::binary);
    
    //read and write
    int srcfp;
    if (-1==(srcfp=open(srcpath,O_RDONLY))){
        perror("error in open src file");
    }
    int dstfp;
    if (-1==(dstfp=open(temppath,O_WRONLY|O_CREAT))){
        perror("error in open dst file");
    }
    
    
    
    if (tflag == 0){
        
        
        // read and write with buff
        if(srcfp)
        {
            while(BUFSIZ-1<(bytes_read=(read(srcfp,buffer,BUFSIZ)))){
                if (bytes_read == -1){
                    perror("error in read");
                    break;
                }
                else if (bytes_read > 0){
                    while (-1!=(bytes_write=(write(dstfp,buffer,BUFSIZ)))){
                        if(bytes_write == -1){
                            perror("error in write");
                        }
                        if(bytes_read == bytes_write)
                            break;
                    }
                }
            }
            bytes_write=write(dstfp,buffer,bytes_read);
            if(bytes_write == -1){
                perror("error in write");
            }
        }
    }
    else if (tflag == 1 ){
        //time
        long seconds = time((time_t*)NULL);
        struct tm *clk;
        clk=localtime(&seconds);
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
        cout<<clk->tm_mday<<" ";
        cout<<clk->tm_hour<<":"<<clk->tm_min<<" "<<endl;

        
        
        
        //user
        struct passwd *pwd=NULL;
        if (NULL==getpwuid(getuid()))
        {
            perror("error in getpwuid");
        }
        else
            pwd = getpwuid(getuid());
        cout<<"User :"<<pwd->pw_name<<endl;
        
        
        // std::istream::get and std::ostream::put
        if (-1==(srcfp=open(srcpath,O_RDONLY))){
            perror("error in open src file");
        }
        if (-1==(dstfp=open(temppath,O_WRONLY|O_CREAT))){
            perror("error in open dst file");
        }
        start = clock();
        while(infile.get(ch)){
            outfile.put(ch);
        }
        finish = clock();
        duration = (double)(finish - start) / CLOCKS_PER_SEC;
        cout<<"Using get and put way cost "<<duration*1000<<" ms"<<endl;
        remove( temppath );

        // read and write with buff
        start = clock();
        if(srcfp)
        {
            while(BUFSIZ-1<(bytes_read=(read(srcfp,buffer,BUFSIZ)))){
                if (bytes_read == -1){
                    perror("error in read");
                    break;
                }
                else if (bytes_read > 0){
                    while (-1!=(bytes_write=(write(dstfp,buffer,BUFSIZ)))){
                        if(bytes_write == -1){
                            perror("error in write");
                        }
                        if(bytes_read == bytes_write)
                            break;
                    }
                }
            }
            bytes_write = write(dstfp,buffer,bytes_read);
            if(bytes_write == -1){
                perror("error in write");
            }
        }
        finish = clock();
        duration = (double)(finish - start) / CLOCKS_PER_SEC;
        cout<<"Using read and write with buff cost "<<duration*1000<<" ms"<<endl;
        remove( temppath );

        
        //read and write in char
        start = clock();
        if (-1==(srcfp=open(srcpath,O_RDONLY))){
            perror("error in open src file");
        }
        if (-1==(dstfp=open(temppath,O_WRONLY|O_CREAT))){
            perror("error in open dst file");
        }
        if(srcfp)
        {
            while(0!=(bytes_read=read(srcfp,chbuff,1))){
                if (bytes_read == -1){
                    perror("error in read");
                    break;
                }
                bytes_read=write(dstfp,chbuff,1);
                if(bytes_write == -1){
                    perror("error in write");
                }
            }
        }
        
        finish = clock();
        duration = (double)(finish - start) / CLOCKS_PER_SEC;
        cout<<"Using read and write with char cost "<<duration*1000<<" ms"<<endl;
 
    }
        
    
    return;
}

int main(int argc, const char *argv[]){
    //const char *argv[3]={"/Users/Elvis/Documents/UCR\ /CS\ 100/lab/lab5-cp/cp.cpp","/Users/Elvis/Desktop/cp","-t"};
    //int argc=3;
    /* lasting time*/
 
    int tflag=0;
    if (argc < 3){
        cout<<"not enough input arguments"<<endl;
        cout<<"cp: illegal option -- 2"<<endl;
        cout<<"usage: cp [filename1] [filedir2] [-r]"<<endl;
        return -1;
    }
    else if (argc == 4){
        if (argv[3][0]=='-' && strlen(argv[3])<3){
            if (strstr(argv[3],"t")!=NULL)
                tflag=1;
            else{
                cout<<"cp: illegal option -- 2"<<endl;
                cout<<"usage: cp [filename1] [filedir2] [-r]"<<endl;
                return -1;
            }
            cp(argv[1],argv[2],tflag);
        }
    }
    else if (argc==3)
        cp(argv[1],argv[2],tflag);
    return 0;
    
}
