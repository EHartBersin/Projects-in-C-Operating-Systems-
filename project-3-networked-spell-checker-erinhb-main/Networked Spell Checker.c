#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <pthread.h>

/*Default parameters*/
#define DEFAULT_PORT 8888
#define DEFAULT_DICTIONARY "words.txt"
#define DICTIONARY_BUFFER 50

/*Global variables*/
FILE *dictionaryFile;
int BUFFER_MAX;

/*Mutual exclusion*/
pthread_mutex_t clientMutex;
pthread_mutex_t logMutex;
pthread_cond_t clientNotEmpty;
pthread_cond_t clientNotFull;
pthread_cond_t logNotEmpty;
pthread_cond_t logNotFull;

/*Counters and pointers*/
int clientCount;
int logCount;
int logReadptr;
int logWriteptr;

/*Functions*/
void workerThread(void *args);
void loggerThread(void *args);
void addLog(char *word, int result);
char *removeLog();
int checkSpelling(char *word);


//Circular buffer struct to hold sockets
typedef struct cbuff_{
    int * buff;
    int start;
    int end;
    int size;
    int count;
} cbuff_t;

cbuff_t * cbuff_new(){
    
    cbuff_t * cb = (cbuff_t*)malloc(sizeof(cbuff_t));
    memset(cb, 0, sizeof(cbuff_t));
    cb->size = BUFFER_MAX;
    cb->buff = (int*)malloc(sizeof(int)*BUFFER_MAX);
    return cb;

}

void cbuff_add(cbuff_t * cb, int elem){
    
    int end = cb->end;
    if(cb->count && (end % cb->size) == cb->start){
        //printf("Overflow Elem[%d] %d lost\n", cb->start, cb->buff[cb->start]);
        cb->start = (cb->start + 1 ) %cb->size;
        cb->count --;
    }
    //printf("Added Elem[%d] = %d\n",cb->end, elem);
    cb->buff[cb->end] = elem;
    cb->end = (cb->end+1) % cb->size;
    cb->count ++;
    
}

int cbuff_remove(cbuff_t * cb){
    
    int start = cb->start ;
    int ret = -1;
    if(cb->count <= 0) {
        //printf("Buffer is empty\n");
        return ret;
    }
    if(cb->count || (start % cb->size) != cb->end) {
        //printf("Removed Elem[%d] = %d\n",cb->start, cb->buff[cb->start]);
        ret = cb->buff[cb->start];
        cb->start = (cb->start + 1 ) % cb->size;
        cb->count--;
    } else {
        //printf("Buffer is empty\n");
    }
    return ret;
    
}

void cbuff_print(cbuff_t * cb){
    
    int start = cb->start ;
    int end = cb->end ;
    int i, count = 0;
    for(i = start; count < cb->count; i = (i + 1)%cb->size){
        printf("Elem[%d] = %d\n", i, cb->buff[i]);
        count++;
        if(i == (end - 1)) {
            break;
        }
    }
    
}

void cbuff_delete(cbuff_t * cb){
    free(cb->buff);
    free(cb);
}


//buffers for logs and clients
cbuff_t *clients;
char *logs[];


/*MAIN*/
//uses given parameters to create a networked spellchecker
//clients connect to the program and gives words to spell check
//main creates a number workers to spell check the given words
//then creates threads to log the results to a log file
//can take in a line of the following parameters
    //name of dictionary file
    //port number
    //number of element cells in the ‘connection buffer’
    //the number of worker threads
    //the scheduling type for spell checking
int main(int argc, char **argv[]){
    
    //initialize variables
    char *dictionary = DEFAULT_DICTIONARY;
    int port = DEFAULT_PORT;
    int bufferMax = 20;
    int numWorkers = 5;
    char *schedulingType;
    BUFFER_MAX = bufferMax;
    //printf("Arguments given: %d\n", argc - 1);

/*
    if(argc == 1){
	printf("No arguments given, using default");
	dictionary = DEFAULT_DICTIONARY;
	port = DEFAULT_PORT;
	bufferMax = 20;
	numWorkers = 5;
	BUFFER_MAX = bufferMax;
    }else if(argc == 5){
	dictionary = argv[2];
	port = argv[3];
	bufferMax = argv[4];
	numWorkers = argv[5];
	BUFFER_MAX = bufferMax;
    }else if(argc == 2){
	dictionary = DEFAULT_DICTIONARY;
	port = DEFAULT_PORT;
	bufferMax = argv[2];
	numWorkers = argv[5];
	BUFFER_MAX = bufferMax;
    }
*/

    //logs = (char*)malloc(BUFFER_MAX * sizeof(char));
    //char* logs[BUFFER_MAX];
    for(int i = 0; i < BUFFER_MAX; i++){
	logs[i] = (char *) calloc(1, sizeof(char *));
    }
    
    //open dictionary
    if(!(dictionaryFile = fopen(dictionary, "r"))){
        printf("Error opening dictionary");
        exit(0);
    }
    
    
    /*initialize network connection*/
    //variables
    int socket_desc, new_socket, c;
    struct sockaddr_in server, client;
    
    //Create a socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1){
        puts("Error: socket creation failed");
        exit(1);
    }
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    
    //bind
    int bind_result = bind(socket_desc, (struct sockaddr*)&server, sizeof(server));
    if(bind_result < 0){
        puts("Error: failed to bind");
        exit(1);
    }
    puts("Bind done");
    
    //listen
    listen(socket_desc, 3);
    puts("Waiting for oncoming clients...");
    
    //points and counters for the client and log buffers
    clientCount = 0;
    //clientReadptr = 0;
    //clientWriteptr = 0;
    
    logCount = 0;
    logReadptr = 0;
    logWriteptr = 0;

    clients = cbuff_new(); 
    
    /*Threads*/
    //create worker threads based on number of worker threads
    //array to hold worker threads
    pthread_t workers[numWorkers];
    // create worker threads
    for (size_t i = 0; i < numWorkers; ++i){
        if (pthread_create(&workers[i], NULL, workerThread, NULL) != 0){
            printf("Error: Failed to create thread\n");
            exit(1);
        }
    }
    
    pthread_t logger;
    if (pthread_create(&logger, NULL, loggerThread, NULL) != 0){
        printf("Error: Failed to create thread\n");
        exit(1);
    }
    
    
    
    /*Main loop*/
    //greeting client
    char *message = "Networked Spell Checker. Type a word to spell check it.\n";
    //start accepting clients
    while(1){
        
        //accept client and error check
        c = sizeof(struct sockaddr_in);
        new_socket = accept(socket_desc, (struct sockaddr*)&client, (socklen_t*)&c);
        if(new_socket < 0){
            puts("Error: accept failed");
            continue;
        }
        puts("Connection accepted");
        
        send(new_socket, message, strlen(message), 0);
        
        //get client queue lock
        pthread_mutex_lock(&clientMutex);
        
        //if client queue is full, wait until its not full
        while (clientCount == BUFFER_MAX) {
            pthread_cond_wait(&clientNotFull, &clientMutex);
        }
        
        //add client socket to the queue and increment count
        cbuff_add(clients, new_socket);
	clientCount = clientCount + 1;
        //cbuff_print(clients);

        //signal workers that client was added
        pthread_cond_signal(&clientNotEmpty);
        
        //unlock the client queue
        pthread_mutex_unlock(&clientMutex);
        
    }

    return 0;
    
}

/*WORKER THREAD*/
//using mutual exclusion, takes a word from the buffer
//calls checkSpelling on client word
//then writes the results to log file
void workerThread(void *args){

    char *word;
    int nBytes;
    char *errorMessage = "Problem receiving word to spell check";
    char *result;
    //char *message = "\nEnter a word:";
    
    while(1){
        
	//printf("before the worker lock");
        //get lock on job queue 
        pthread_mutex_lock(&clientMutex);
        
        //if there are no words to spell check, then wait
        while (clientCount == 0) {
            pthread_cond_wait(&clientNotEmpty, &clientMutex);
        }
        
        //get socket from client queue
        int socket = cbuff_remove(clients);
	clientCount = clientCount - 1;
	//send signal to other workers to use client buffer
	pthread_cond_signal(&clientNotFull);
	
	//unlock client mutex
	pthread_mutex_unlock(&clientMutex);
	
	//send(socket, message, strlen(message), 0);
        //printf("socket in pthread %d", socket);
        while(1){
            
            //allocating space and getting client word
            word = calloc(DICTIONARY_BUFFER, 1);
            nBytes = recv(socket, word, DICTIONARY_BUFFER, 0);
            //send(socket, word, strlen(word), 0);
            //error check
            if (nBytes < 0) {
                send(socket, errorMessage, strlen(errorMessage), 0);
                continue;
            }
            
            //check spelling of word
            int check = checkSpelling(word);
            
            //1 if correct
            if(check == 1){
                result = "OK\n";
            //0 if incorrect
            }else{
                result = "MISSPELLED\n";
            }
            
            //send result to the client
            send(socket, result, strlen(result), 0);

            //printf("before log lock");
            //sending the result to the log queue
            //getting log queue lock
            pthread_mutex_lock(&logMutex);
            //printf("in the log lock");
            
	    //check to see if log is full, if full, wait for it to not be
            while(logWriteptr == logReadptr && logCount == BUFFER_MAX){
                pthread_cond_wait(&logNotFull, &logMutex);
            }
            //printf("before adding a log");
            //got the lock, write to the log queue
            addLog(word, check);
            
            //send signal to other worker threads
            pthread_cond_signal(&logNotEmpty);
            //unlock the lock 
            pthread_mutex_unlock(&logMutex);
            
        }
        
        close(socket);

    }
    
}

/*LOGGER THREAD*/
//using mutual exclusion, takes log from queue and adds it to log file
void loggerThread(void *args){
    
    //open log file to append it
    FILE *logFile = fopen("logFile.txt", "a");
    
    //main loop
    while(1){
        
        //get lock on the log queue
        pthread_mutex_lock(&logMutex);
        
        //if there is nothing in the log queue, wait until there is
        if (logReadptr == logWriteptr && logCount == 0) {
            pthread_cond_wait(&logNotEmpty, &logMutex);
        }
        
        //remove the log
        char *nextLog = removeLog();
        
        //write results to log
        fprintf(logFile, "%s\n", nextLog);
        fflush(logFile);
        
        //signal to other threads
        pthread_cond_signal(&logNotFull);
        
        //unlock the log queue for other threads to use
        pthread_mutex_unlock(&logMutex);
        
    }
    
}

//inserts a log for worker thread
void addLog(char *word, int result){
    
    //printf("In add log");
    //holds word and result in one string
    char string[DICTIONARY_BUFFER];

    //remove the \n from the user word by replacing it with null
    size_t len = strlen(word);
    word[len-1] = '\0';
    
    //holds ok or misspelled string based on result
    char *res;
    //1 if correct
    if(result = 1){
        res = "OK\n";
    //0 if incorrect
    }else{
        res = "MISSPELLED\n";
    }
    
    //copying the word and result into a string buffer
    strcat(string, word);
    strcat(string, " is ");
    strcat(string, res);
    
    //add the log to the log buffer, then increment the index
    strcat(logs[logWriteptr], string);
    logWriteptr = (logWriteptr + 1) % BUFFER_MAX;
    
    //increment log counter
    logCount = logCount + 1;
    
}

//removes a log for logger thread
char* removeLog(){
    
    //get the log
    char *nextLog = logs[logReadptr];
    
    //clearing index of log
    logs[logReadptr] = (char *)calloc(1, sizeof(char *));

    //increment the read pointer
    logReadptr = (logReadptr + 1) % BUFFER_MAX;
    
    //decrement log count
    logCount = logCount - 1;
    
    return nextLog;
    
}

int checkSpelling(char *word){
    
    //variables for tracking a match and to hold each word in dictionary
    int match = 0;
    char nextWord[DICTIONARY_BUFFER];
    
/*
    //removing new line character
    //string for client word
    char *clientWord;
    //clean up new line for client word
    const char s[2] = "\n";
    char *token;
    //get first token
    token = strtok(word, s);
    //walk through other tokens
    while( token != NULL ) {
        clientWord = token;
        token = strtok(NULL, s);
    }
*/    
    //go through dictionary
    while(fgets(nextWord, DICTIONARY_BUFFER, dictionaryFile) != NULL) {
        
	/*
        //removing new line character
        //string for next dictionary word
        char *dictWord;
        //clean up new line for dictionary word
        const char s[2] = "\n";
        char *token;
        //get first token
        token = strtok(nextWord, s);
        //walk through other tokens
        while( token != NULL ) {
            dictWord = token;
            token = strtok(NULL, s);
        }
        */

        //if the word isn't a match, get next word
        if(strcmp(nextWord, word) != 0) {
            continue;
        //if it is, rewind the dictionary pointer and return match = 1
        }else {
            rewind(dictionaryFile);
            return ++match;
        }
    }
    
    //no match was found, rewind dictionary and return match = 0
    rewind(dictionaryFile);
    return match;
    
}
