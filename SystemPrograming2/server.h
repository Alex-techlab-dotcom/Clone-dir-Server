#include <iostream>
#include <stdio.h>
#include <sys/wait.h> /* sockets */
#include <sys/types.h> /* sockets */
#include <sys/socket.h> /* sockets */
#include <netinet/in.h> /* internet sockets */
#include <netdb.h> /* gethostbyaddr */
#include <unistd.h> /* fork */
#include <stdlib.h> /* exit */
#include <ctype.h> /* toupper */
#include <signal.h> /* signal */
#include <pthread.h>
#include <dirent.h>
#include <vector>
#include <string.h>
#include "dataStructures.h"
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unordered_map>

#define _OPEN_SYS_SOCK_IPV6
#define _XOPEN_SOURCE_EXTENDED 1
#define perror2(s, e) fprintf(stderr, "%s: %s\n", s, hstrerror(e));
using namespace std;


void listFilesRecursively(char *basePath, vector<string> &myList);

bool exitFlag = true;

void *child_server(void *_newsock);

void perror_exit(char *message);

void sigchld_handler(int sig);

void exitHnadler(int signum) {
    cout << "inside the exit handler" << endl;
    exitFlag = false;
}

void *readDirName(void *_newsock);

void *sendFileToClient(void *ptr);

Queue queueForFiles;
static pthread_mutex_t QueueLock;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;
unordered_map<int, pthread_mutex_t> socketToMutex;
