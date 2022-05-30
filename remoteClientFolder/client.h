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

vector <string> CreatedFolders;
bool containsFolder(string directory);
void _mkdir(string path);