#include <stdio.h>
#include <sys/types.h> /* sockets */
#include <sys/socket.h> /* sockets */
#include <netinet/in.h> /* internet sockets */
#include <unistd.h> /* read, write, close */
#include <netdb.h> /* gethostbyaddr */
#include <stdlib.h> /* exit */
#include <string.h> /* strlen */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <iostream>
#include <sys/stat.h>
#include <vector>
#include <fcntl.h>
using namespace std;
#define rmdir(dir) _rmdir(dir)

void perror_exit(char *message);

vector<string> CreatedFolders;

bool containsFolder(string directory) {
    for (int dir = 0; dir < CreatedFolders.size(); ++dir) {
        if (CreatedFolders[dir] == directory) {
            return true;
        }
    }
    return false;
}
void _mkdir(string path){
    string dir = path;
    string subDir;
    for (int letter = 0; letter < dir.length(); ++letter) {
        if (dir[letter] != '/') {
            subDir += dir[letter];
            continue;
        }
        if (!containsFolder(subDir)){
            mkdir(subDir.c_str(),0700);
            CreatedFolders.push_back(subDir);
        }
        subDir+="/";
    }
}

int main(int argc, char *argv[]) {
    /* ./remoteClient -i <server_ip> -p <server_port> -d <directory> */
    int port, sock, i;
    char directory[512];
    strcpy(directory,argv[6]);
    cout<<"Input directory is: "<<directory<<endl;
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr *) &server;
    struct hostent *rem;
    struct in_addr serverAddress;

    /* create the folder*/
    mkdir(directory,0700);

    /* Create socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit("socket");
    /* Find server address */
    inet_aton( argv [2] , &serverAddress ) ;
    if ((rem =gethostbyaddr (( const char *) &serverAddress , sizeof ( serverAddress ) , AF_INET ))  == NULL) {
        herror("gethostbyaddr");
        exit(1);
    }
    port = atoi(argv[4]); /*Convert port number to integer*/
    server.sin_family = AF_INET; /* Internet domain */
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(port); /* Server port */

    /* Initiate connection */
    if (connect(sock, serverptr, sizeof(server)) < 0)
        perror_exit("connect");
    printf("Connecting to %s port %d\n", argv[1], port);

    /* write the directory name to server socket! */
    if (write(sock, directory, strlen(directory)+1) < 0)
        perror_exit("write");
    /* retrieve how many files the directory contains*/
    int numOfFiles;
    char _numOfFiles[100];
    if (read(sock, _numOfFiles, sizeof(_numOfFiles) ) < 0)
        perror_exit("read file number failed! ");
    numOfFiles=atoi(_numOfFiles);

    /* for each file do:*/
    char buf[256];
    for (int file = 0; file < numOfFiles; ++file) {
        /* receive the fileName */
        char bufferLetter[1];
        string fileName;
        while(read(sock,bufferLetter,1)>0){
            if (bufferLetter[0] != '\n') {
                fileName+=bufferLetter[0];
                continue;
            }
        }
        /* create the needed folders first*/
        _mkdir(fileName);
        int fd;
        /* create the file */
        if ((fd = open(fileName.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0644)) < 0) {
            perror("fileName.out: file open problem from worker\n");
            exit(3);
        }

        /* the client must know how many bytes must read so he can stop the read() while Loop */
        int fileSize;
        string _fileSize;
        while(read(sock,bufferLetter,1)>0){
            if (bufferLetter[0] != '\n') {
                _fileSize+=bufferLetter[0];
                continue;
            }
        }
        fileSize=atoi(_fileSize.c_str());

        while ((read(sock, buf, sizeof(buf))) > 0) {
            /* write the contents of the file to fd */
            write(fd,buf, strlen(buf));
            /* subtract the bytes we just read from the total bytes! */
            fileSize-= strlen(buf);
            /* clear the buffer */
            memset(buf,0, strlen(buf));
            /* if we are reached the end of the file, we stop reading*/
            if (fileSize<=0)
                break;
        }
        close(fd);
    }

    /* this close both reception and transmission endPoints! */
    printf("Closing connection.\n");
    shutdown(sock,2);
}

void perror_exit(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}


/*do {
      printf("Give input string: ");
      //read(0,buf,sizeof(buf));
      fgets(buf, sizeof(buf), stdin); *//* Read from stdin*//*
        for (i = 0; buf[i] != '\0'; i++) { *//* For every char *//*
            *//* Send i-th character *//*
            if (write(sock, buf + i, 1) < 0)
                perror_exit("write");
             *//* receive i-th character transformed *//*
            if (read(sock, buf + i, 1) < 0)
                perror_exit("read");
        }
        printf("Received string: %s", buf);
    } while (strcmp(buf, "END\n") != 0); *//* Finish on "end" */
//close(sock); /* Close socket and exit */