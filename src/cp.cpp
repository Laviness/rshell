#include <iostream>
#include <istream>
#include <ostream>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int BUFFER_SIZE = BUFSIZ;

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


    int size;


    //read until the end of file is reached
    while((size = read(sfd, buf, size)) != 0){
        
       if( write(dfd, buf, size) == -1){
       perror("Write failed");
       printf("errno:%d", errno);    
       
       }
     }
    return;    
}

int main(int argc, char **argv){

    if(argv[3] != NULL){
        // Do all three methods   
     }

    // Buffer size of one
    char *buf = new char[1];
    char * dir = get_current_dir_name();
    char path[256];
    strcpy(path,dir);
    strcat(path,"/");
    strcat(path, argv[2]); 
    cp(argv[1], path, buf, 1);
    delete buf;
    buf = 0;
    buf = new char[BUFFER_SIZE];
    cp(argv[1], path, buf, BUFFER_SIZE);
    free(dir);
    



    return 0;

}


