#include "client.h"

int main(int argc, char *argv[]) {
    /* ./remoteClient -i <server_ip> -p <server_port> -d <directory> */
    mode_t fdmode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int port, sock, i;
    char server_ip[512];
    char directory[512];

    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr *) &server;
    struct hostent *rem;
    struct in_addr serverAddress;

    for (int i = 1; i < argc; i+=2) {
        if (!strcmp(argv[i],"-p")){
            port = atoi(argv[i+1]);
        }else if (!strcmp(argv[i],"-i")){
            strcpy(server_ip, argv[i+1]);
        }else if (!strcmp(argv[i],"-d")){
            strcpy(directory, argv[i+1]);
        }
    }

    cout << "Input directory is: " << directory << endl;

    /* Create socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit("socket");
    /* Find server address */
    inet_aton(server_ip, &serverAddress);
    if ((rem = gethostbyaddr((const char *) &serverAddress, sizeof(serverAddress), AF_INET)) == NULL) {
        herror("gethostbyaddr");
        exit(1);
    }
     /*Convert port number to integer*/
    server.sin_family = AF_INET; /* Internet domain */
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(port); /* Server port */

    /* Initiate connection */
    if (connect(sock, serverptr, sizeof(server)) < 0)
        perror_exit("connect");
    printf("Connecting to %s port %d\n", argv[1], port);

    /* write the directory name to server socket! */
    cout << "sending dir..." << directory << endl;
    cout << "CLIENT->SOCK: " << sock << endl;
    if (write(sock, directory, strlen(directory) + 1) < 0)
        perror_exit("write");
    /* retrieve how many files the directory contains*/
    int numOfFiles;
    char _numOfFiles[100];
    if (read(sock, &numOfFiles, sizeof(numOfFiles)) < 0)
        perror_exit("read file number failed! ");
    numOfFiles = ntohs(numOfFiles);
    //numOfFiles = atoi(_numOfFiles);
    cout << " FILES ARE: " << numOfFiles << endl;
    /* for each file do:*/
    for (int file = 0; file < numOfFiles; ++file) {
        /* receive the fileName */
        char bufferLetter[1];
        string fileName;
        while (read(sock, bufferLetter, 1) > 0) {
            if (bufferLetter[0] != '\n') {
                fileName += bufferLetter[0];
                cout << "filename is: " << fileName << endl;
                continue;
            } else
                break;

        }
        /* create the needed folders first*/
        _mkdir(fileName);
        int fd;
        cout << "Received file: " << fileName.c_str() << endl;
        /* create the file */
        if ((fd = open(fileName.c_str(), O_WRONLY | O_TRUNC | O_CREAT, fdmode)) < 0) {
            perror("fileName.out: file open problem from worker\n");
            exit(3);
        }
        /* the client must know how many bytes must read so he can stop the read() while Loop */
        int fileSize;
        int fz;
        string _fileSize;
        memset(bufferLetter, 0, 1);
        read(sock, &fz, sizeof(fz));
        fileSize = ntohs(fz);

        cout << "CLIENT->FILESIZE: " << fileSize << endl;

        int sizeOfBuffer;
        read(sock, &sizeOfBuffer, sizeof(int));
        sizeOfBuffer = ntohs(sizeOfBuffer);
        cout << "sizeOfBuffer: " << sizeOfBuffer << endl;


        char buf[sizeOfBuffer];
        memset(buf, 0, sizeof(buf));
        int bytes;
        while ((bytes = read(sock, buf, sizeOfBuffer - 1)) > 0) {

            if (write(fd, buf, bytes) < 1)
                perror_exit("write");
            
            fileSize -= sizeOfBuffer - 1;
            char *msg = "OK";
            write(sock, msg, strlen(msg));
            if (fileSize <= 0 or bytes == -1)
                break;
            if (fileSize <= sizeOfBuffer - 1)
                sizeOfBuffer = fileSize + 1;
            /* clear the buffer */
            memset(buf, 0, sizeOfBuffer-1);

            /* if we are reached the end of the file, we stop reading*/

        }
        cout << " i am done with file " << endl;
        /* char *message = "DONE_READING";
         write(sock, message, strlen(message));*/
        close(fd);
    }



    /* this close both reception and transmission endPoints! */
    printf("Closing connection.\n");
    shutdown(sock, 2);
}

void perror_exit(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

bool containsFolder(string directory) {
    for (int dir = 0; dir < CreatedFolders.size(); ++dir) {
        if (CreatedFolders[dir] == directory) {
            return true;
        }
    }
    return false;
}

void _mkdir(string path) {
    string dir = path;
    string subDir;
    for (int letter = 0; letter < dir.length(); ++letter) {
        if (dir[letter] != '/') {
            subDir += dir[letter];
            continue;
        }
        if (!containsFolder(subDir)) {
            mkdir(subDir.c_str(), 0700);
            CreatedFolders.push_back(subDir);
        }
        subDir += "/";
    }
}

