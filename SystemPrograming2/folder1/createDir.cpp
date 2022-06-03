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