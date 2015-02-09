#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Timer.h"

using namespace std;

static int BUFFER_SIZE = BUFSIZ;

void cp(char *source, char *destination){
    
    ifstream ifile;
    ifile.open(source, ifstream::in | ifstream::binary); 
    if(!ifile.is_open()){
        printf("Could not of destination");
        return;

        }

    ofstream ofile;
    ofile.open(destination, ofstream::out | ifstream::binary);
    if(!ofile.is_open()){
        printf("Could not of destination");
        return;
    }

    char *c = new char[1];
    while(ifile.good()){
        *c = ifile.get();
        ofile.write(c, 1);
    }
    delete c;
    c = 0;

    ifile.close();
    ofile.close();
    
}

void cp(char *source, char *destination, char * buf, int size){
    
   int sfd = open(source, O_RDONLY); 
   if(sfd == -1){
      perror("Open Failed" );
      printf("%s:errno:%d\n",source, errno ) ;
       // Failed to open, bail out
        return;
      } 

    // open succesfull

    int dfd = open(destination, O_WRONLY | O_CREAT);
    if(dfd == -1){
        perror("Open failed");
        printf("%s:errno:%d\n", destination, errno);
        return;
    }


    int reads;


    //read until the end of file is reached
    while((reads = read(sfd, buf, size)) != 0){
        
       if( write(dfd, buf, size) == -1){
       perror("Write failed");
       printf("errno:%d", errno);    
       
       }
     }
     if(close(sfd) == -1){
         perror("Close failed");
     }

     if(close(dfd) == -1){
         perror("Close failed");
      }
    return;    
}

int main(int argc, char **argv){

    bool all = false;
    if(argv[3] != NULL){
        // Do all three methods   
        all = true;
     }

    
    // Buffer size of one
    char *buf = new char[1];
    char * dir = get_current_dir_name();
    char path[256];
    strcpy(path,dir);
    free(dir);
    strcat(path,"/");
    strcat(path, argv[2]); 
    Timer time;
    double euTime;
    double ewTime;
    double esTime;

    if(all){
    
        time.start();
        cp(argv[1], argv[2]);    
        time.elapsedSystemTime(esTime);
        time.elapsedUserTime(euTime);
        time.elapsedWallclockTime(ewTime);
        printf("%f:%f:%f\n", euTime, esTime, ewTime);
    }
    
    euTime = 0;
    ewTime = 0;
    esTime = 0;

    if(all){
        time.start();
        cp(argv[1], path, buf, 1);
        time.elapsedUserTime(euTime);
        time.elapsedSystemTime(esTime);
        time.elapsedWallclockTime(ewTime);
        printf("%f:%f:%f\n", euTime, esTime, ewTime);
    }
    delete buf;
    buf = 0;
    buf = new char[BUFFER_SIZE];
   
   
    ewTime = 0;
    euTime = 0;
    esTime = 0;
    
    // Start for third option
    time.start();
    cp(argv[1], path, buf, BUFFER_SIZE);
    time.elapsedUserTime(euTime);
    printf("%f:%f:%f\n", euTime, esTime, ewTime);


    return 0;

}


