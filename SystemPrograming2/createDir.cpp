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
#include <fcntl.h>
#include <string.h>

using namespace std;

#include <vector>

vector<string> CreatedFolders;

bool containsFolder(string directory) {
    for (int dir = 0; dir < CreatedFolders.size(); ++dir) {
        if (CreatedFolders[dir] == directory) {
            return true;
        }
    }
    return false;
}

int main(int argc, char *argv[]) {
    int port, thread_pool_size, queue_size, block_size, sock;
    pthread_t *workingThreadsArray;
    int opt = 0;
    int iOptLen = sizeof(int);
    /* ./dataServer -p <port> -s <thread_pool_size> -q <queue_size> -b <block_size> */
    for (int i = 1; i < argc; i+=2) {
        if (!strcmp(argv[i],"-p")){
            port = atoi(argv[i+1]);
        }else if (!strcmp(argv[i],"-s")){
            thread_pool_size = atoi(argv[i+1]);
        }else if (!strcmp(argv[i],"-q")){
            queue_size = atoi(argv[i+1]);
        }else if (!strcmp(argv[i],"-b")){
            block_size = atoi(argv[i+1]);
        }
    }
    /* print the arguments */
    cout << "port: " << port << endl;
    cout << "thread_pool_size: " << thread_pool_size << endl;
    cout << "queue_size: " << queue_size << endl;
    cout << "block_size: " << block_size << endl;

    return 1;
    string dir = "folder1/folder2/f3/f4/f5/f2.txt";
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
    int fd;
    if ((fd = open(dir.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0644)) < 0) {
        perror("fileName.out: file open problem from worker\n");
        exit(3);
    }
}