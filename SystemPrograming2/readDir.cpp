/**
 * C program to list contents of a directory recursively.
 */

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include<list>
#include <string>
#include <vector>
#include <iostream>
using namespace std;
void listFilesRecursively(char *path, vector<string> &myList);


int main() {
    // Directory path to list files
    char path[100];

    // Input path from user
    printf("Enter path to list files: ");
    scanf("%s", path);

    vector<string> l;
    vector<string> folderFiles;
    listFilesRecursively(path,l);

    for (int i = 0; i < l.size(); ++i) {
        string probablyAFolder=l[i];
        bool isFolder= false;
        for (int j = i+1; j < l.size(); ++j) {
            size_t found = l[j].find(probablyAFolder);
            if (found != string::npos){
                isFolder= true;
                break;
            }
        }
        if (isFolder== false){
            folderFiles.push_back(l[i]);
        }
    }

    for (int i = 0; i < folderFiles.size(); ++i) {
        cout<<folderFiles[i]<<endl;
    }

    return 0;
}


/**
 * Lists all files and sub-directories recursively
 * considering path as base path.
 */
void listFilesRecursively(char *basePath,vector<string> &myList) {
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basePath);

    // Unable to open directory stream
    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {


            // Construct new path from our base path
            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, dp->d_name);
            string _path=path;
            myList.push_back(_path);
            //printf("%s\n", path);

            listFilesRecursively(path,myList);
        }
    }

    closedir(dir);
}

