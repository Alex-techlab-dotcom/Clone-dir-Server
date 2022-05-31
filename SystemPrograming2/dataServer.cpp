#include "server.h"

int main(int argc, char *argv[]) {

    pthread_mutex_init(&QueueLock, 0);
    struct sigaction exit_action;
    exit_action.sa_handler = exitHnadler;
    exit_action.sa_flags = 0;
    sigaction(SIGINT, &exit_action, NULL);

    int port, thread_pool_size, queue_size, block_size, sock;
    pthread_t *workingThreadsArray;
    int opt = 0;
    int iOptLen = sizeof(int);
    /* ./dataServer -p <port> -s <thread_pool_size> -q <queue_size> -b <block_size> */
    port = atoi(argv[2]);
    thread_pool_size = atoi(argv[4]);
    queue_size = atoi(argv[6]);
    block_size = atoi(argv[8]);
    queueForFiles.SetMaxSize(queue_size);

    /* print the arguments */
    cout << "port: " << port << endl;
    cout << "thread_pool_size: " << thread_pool_size << endl;
    cout << "queue_size: " << queue_size << endl;
    cout << "block_size: " << block_size << endl;

    if ((workingThreadsArray = (pthread_t *) malloc(thread_pool_size * sizeof(pthread_t))) == NULL) {
        perror("malloc for working threads failed");
        exit(1);
    }
    for (int i = 0; i < thread_pool_size; ++i) {
        int err;
        if ((err = pthread_create(&workingThreadsArray[i], NULL, sendFileToClient, &block_size))) {
            /* Create a communication thread */
            perror2("pthread_create", err);
            exit(1);
        }
    }

    socklen_t clientlen;
    struct sockaddr_in server, client;
    struct sockaddr *serverptr = (struct sockaddr *) &server;
    struct hostent *rem;


    /* Reap dead children asynchronously */
    signal(SIGCHLD, sigchld_handler);
    /* Create socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit("socket creation failed");
    server.sin_family = AF_INET; /* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port); /* The given port */

    int iResult;
    //iResult = getsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &iOptVal, &iOptLen);
    iResult = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* Bind socket to address */
    if (bind(sock, serverptr, sizeof(server)) < 0)
        perror_exit("binding failed");

    /* Listen for connections */
    if (listen(sock, 5) < 0)
        perror_exit("listen");
    printf("Listening for connections to port %d\n", port);
    int err;
    while (exitFlag) {
        struct sockaddr *clientptr = (struct sockaddr *) &client;
        int *newsock = (int *) malloc(sizeof(int));
        clientlen = sizeof(client);
        //cout << "accepting connection..." << endl;
        /* accept connection */
        if ((*newsock = accept(sock, clientptr, &clientlen)) < 0)
            perror_exit("accept");
        /* The server assigns a mutex to each client*/
        pthread_mutex_t* mtx=(pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(mtx, NULL); /* Initialize mutex */
        socketToMutex[*newsock] = mtx;


        if (exitFlag == false)
            break;
        /* Find client's name */
        if ((rem = gethostbyaddr((char *) &client.sin_addr.s_addr, sizeof(client.sin_addr.s_addr),
                                 client.sin_family)) == NULL) {
            herror("gethostbyaddr");
            exit(1);
        }
        //printf("Accepted connection from %s\n", rem->h_name);
        //cout<< "SERVER->NEWSOCK: "<<*newsock<<endl;
        pthread_t communicationThread;
        if ((err = pthread_create(&communicationThread, NULL, readDirName, newsock))) {
            /* Create a communication thread */
            perror2("pthread_create", err);
            exit(1);
        }
    }

}

/**
 * Lists all files and sub-directories recursively
 * considering path as base path.
 */
void listFilesRecursively(char *basePath, vector<string> &myList) {
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
            string _path = path;
            myList.push_back(_path);
            listFilesRecursively(path, myList);
        }
    }

    closedir(dir);
}

void *readDirName(void *_newsock) {
    int newsock = *((int *) _newsock);
    char inputDir[512];
    if (read(newsock, inputDir, sizeof(inputDir)) < 0)
        perror_exit("read failed!");
    //cout<<pthread_self()<<"->NEWSOCK: "<<newsock<<endl;
    //cout << "Received from client: " << inputDir << endl;

    /* we scan recursively the directory! */
    vector<string> contents;
    vector<string> folderFiles;
    listFilesRecursively(inputDir, contents);

    /* we remove the sub-dir names */
    for (int i = 0; i < contents.size(); ++i) {
        string probablyAFolder = contents[i];
        bool isFolder = false;
        for (int j = i + 1; j < contents.size(); ++j) {
            size_t found = contents[j].find(probablyAFolder);
            if (found != string::npos) {
                isFolder = true;
                break;
            }
        }
        if (isFolder == false) {
            folderFiles.push_back(contents[i]);
        }
    }
    /* the client must know how many files , he will receive! */
    int numberOfFiles = folderFiles.size();
    numberOfFiles= htons(numberOfFiles);
    //char _numberOfFiles[100];
    /* cout << " SERVER->FILES ARE: " << numberOfFiles << endl;*/
    //sprintf(_numberOfFiles, "%d", numberOfFiles);

    /* write the files number to client socket*/
    write(newsock, &numberOfFiles, sizeof(int));

    /* place the files inside the Queue! */
    for (int i = 0; i < folderFiles.size(); ++i) {
        pthread_mutex_lock(&QueueLock);
        /* if Queue is full, we wait by using nonfull condition variable! */
        while (queueForFiles.isFull())
            pthread_cond_wait(&cond_nonfull, &QueueLock);

        /* Queue is not full, so we add the current file name!*/
        cout << "<" << folderFiles[i] << " , " << newsock << ">" << " inserted inside the Queue" << endl;
        pair<string, int> filenameWithFD;
        filenameWithFD.first = folderFiles[i];
        filenameWithFD.second = newsock;
        queueForFiles.Insert(filenameWithFD);
        /* release mutex after insertion */
        pthread_mutex_unlock(&QueueLock);
        /* signal the workers, so they can start removing files from Queue */
        pthread_cond_signal(&cond_nonempty);
        /* i xrisi tis usleep gia 100 microseconds epitagxanei tin pio Orati epiteuxi sixronismou pou exei i ilipoiisi tis ergasias m*/
        /* diaforetika polles gia mikra arxeia , i se periptosi pou epomenos pelatis den milisei me ton server amesws to com_pthread prolavainei
         kai eisagei ola ta arxeia stin oura xwris na prolabei kapoios allos NEOS pelatis na dimiourgi8ei , opote i xrisi tis usleep den sterei
         apo to programma apodotikotika apla kanei pio emfanes oti ilopoiisa ton sixronismo pou epi8imite*/
        //usleep(100);
    }

    pthread_exit(NULL);
}

void *sendFileToClient(void *ptr) {
    int numOfBytes = *((int *) ptr);

    while (exitFlag) {
        pthread_mutex_lock(&QueueLock);
        /* if Queue is empty, we wait by using nonempty condition variable! */
        while (queueForFiles.GetSize() == 0) {
            pthread_cond_wait(&cond_nonempty, &QueueLock);
        }
        pair<string, int> filenameWithFD = queueForFiles.PopOut();
        cout << "Worker_thread " << pthread_self() << ": <" << filenameWithFD.first << " , " << filenameWithFD.second
             << ">" << " removed from the Queue"
             << endl;
        cout << "Worker_thread " << pthread_self() << ": is about to read " << filenameWithFD.first << endl;
        pthread_mutex_unlock(&QueueLock);
        pthread_cond_signal(&cond_nonfull);


        /* start copying the data from file to client*/
        /* first get the fileName */
        char *fileName = (char *) malloc(strlen((filenameWithFD.first).c_str()) + 2);
        strcpy(fileName, (filenameWithFD.first).c_str());

        /* acquire the mutex to block other working_threads from communicating to the same client */
        pthread_mutex_lock(socketToMutex[filenameWithFD.second]);
        //cout <<"Worker_thread: "<<pthread_self()<<" locked mutex: "<<filenameWithFD.second<<endl;
        /* send file name to client*/
        int clientSocketD = filenameWithFD.second;
        strcat(fileName, "\n");
        write(clientSocketD, fileName, strlen(fileName));

        int fd; /* open the file for copying */
        if ((fd = open(filenameWithFD.first.c_str(), O_RDONLY)) < 0) {
            perror(" file open problem from server\n");
            exit(3);
        }
        /* find the size of the file! */
        int fileSize;
        ioctl(fd, FIONREAD, &fileSize);
        int fz= htons(fileSize);
        //int fz=fileSize;

        /* convert the size to char[] with '\n' in the end*/
      /*  char number[100];
        sprintf(number, "%d", fileSize);
        strcat(number, "\n");*/

         /*write the the size inside the socket */
        cout<<"SERVER->FILESIZE: "<<fz<<endl;
        //fz = htons(fileSize);

        write(clientSocketD, &fz, sizeof(int));
        // write(clientSocketD, "\n", 1);

        char readingBuffer[numOfBytes + 1];
        memset(readingBuffer, 0, sizeof(readingBuffer));
        /* read from file! */
        while ((read(fd, readingBuffer, numOfBytes)) > 0) {
            //cout<<"buffer is "<<readingBuffer<<endl;
            /* write the contents of the file to client-socket */
            write(clientSocketD, readingBuffer, strlen(readingBuffer));
            /* clear the buffer */
            memset(readingBuffer, 0, sizeof(readingBuffer));
        }
        close(fd);
        //cout<<"I AM DONE "<<pthread_self()<<endl;
         /*char clientFinised[100];
         read(clientSocketD,clientFinised,sizeof(clientFinised));*/
        //cout <<"Worker_thread: "<<pthread_self()<<"  RELEASED MUTEX: "<<filenameWithFD.second<<endl;
        pthread_mutex_unlock(socketToMutex[filenameWithFD.second]);
        //sleep(1);
    }
    cout << "END FROM WORKER_THREAD" << endl;
    pthread_exit(NULL);
}

void *child_server(void *_newsock) {
    int newsock = *((int *) _newsock);
    char buf[1];
    while (read(newsock, buf, 1) > 0) { /* Receive 1 char */
        putchar(buf[0]); /* Print received char */
        /* Capitalize character */
        buf[0] = toupper(buf[0]);
        /* Reply */
        if (write(newsock, buf, 1) < 0)
            perror_exit("write");
    }
    printf("Closing connection.\n");
    //close(newsock); /* Close socket */
    pthread_exit(NULL);
}

/* Wait for all dead child processes */
void sigchld_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void perror_exit(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}