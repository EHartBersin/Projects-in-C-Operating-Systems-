#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>

/*GLOBAL VARIABLES*/
#define NUM_MAX_SIGNALS 100
#define NUM_PROCESSES 7

//referenced nanosleep and timespec for sleeping processes
//https://man7.org/linux/man-pages/man2/nanosleep.2.html

/*FUNCTIONS*/
void signalHandler(int signal);
void signalHandler1(int sig);
void signalHandler2(int sig);
void reporterTimer(int sig);
void reporter(int value);
void generator();
double randGenerator();
void unblock_signal(int signal);
void block_signal(int signal);
bool maxSignalsCreated();

/*SIGNAL COUNTER STRUCTURE*/
//tracks amount of each signal sent and received, holds locks for data
typedef struct Counters{
    int sentSIGUSR1;
    int sentSIGUSR2;
    int receivedSIGUSR1;
    int receivedSIGUSR2;
    pthread_mutex_t signal1Lock;
    pthread_mutex_t signal2Lock;
}Counters;

/*GLOBAL COUNTER STRUCT*/
Counters *count;

/*TIME VARIABLES*/
struct timespec timeSIGUSR1;
double timeSumSIGUSR1;
struct timespec timeSIGUSR2;
double timeSumSIGUSR2;


//initializes mutexes and signals
//forks to create several child processes
    //creates two SIGUSR1 handlers
    //creates two SIGUSR2 handlers
    //creates one reporter and one generator
//parent waits for their completion then waits until number of max signals is reached
    //kills all child processes
    //prints final stats as way to indicate termination
int main() {

    //setting up shared memory for the processes
    //used to share Counter structure that holds the signal sent/recieved counters
    int sharedMem = shmget(IPC_PRIVATE, sizeof(Counters), 0666 | IPC_CREAT);
    if(sharedMem == -1) {
        puts("Error creating sharedMem");
    }
    count = (Counters *)shmat(sharedMem, NULL, 0);
    if(count == (Counters *)-1) {
        puts("Error creating counter structure");
    }

    //creating signals
    block_signal(SIGUSR1);
    block_signal(SIGUSR2);
    
    //Set-up mutexes for the processes to share
    pthread_mutexattr_t attrOne;
    pthread_mutexattr_init(&attrOne);
    pthread_mutexattr_setpshared(&attrOne, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init((&count->signal1Lock), &attrOne);

    pthread_mutexattr_t attrTwo;
    pthread_mutexattr_init(&attrTwo);
    pthread_mutexattr_setpshared(&attrTwo, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&(count->signal2Lock), &attrTwo);


    //Creating processes.
    pid_t processType[7];
    for(int i = 0; i <= NUM_PROCESSES; i++) {
        
        //fork and error check
        processType[i] = fork();
        if(processType[i] == -1) {
            printf("ERROR FORKING\n");
        }
        
        //if in child, create each process we need
        if(processType[i] == 0) {
            //for i == 0, i == 1 creates two SIGUSR1 handlers by sending value 1 to main handler
            if(i < 2) {
                //printf("Creating SIGUSR1 handlers\n");
                signalHandler1(SIGUSR1);
            }//for i == 2, i == 3, creates two SIGUSR2 handlers
            else if(i >= 2 && i < 4) {
                //printf("Creating SIGUSR2 handlers\n");
                signalHandler2(SIGUSR2);
            }//for i == 4, creates 1 reporter
            else if(i == 4) {
                //printf("Creating reporter\n");
                reporter(1);
            }
            else if(i == 5) {
                //printf("Creating generator\n");
                generator();
            }
            if(i == 6) {
                waitpid(processType[i], NULL, 0);
            }
            
        }//in parent
        else {
            //if we created every process needed
            if(i == NUM_PROCESSES) {
                //enter while loop
                while(true) {
                    //check to see if we reached max amount of signals
                    if(maxSignalsCreated()) {
                        for(int y = 0; y < NUM_PROCESSES; y++) {
                                //If max count reached, kill all child processes.
                                kill(processType[y], SIGINT);
                            }
                            printf("SIGUSR1 sent: %d, SIGUSR1 recieved: %d\n", count->sentSIGUSR1, count->receivedSIGUSR1);
                            printf("SIGUSR2 sent: %d, SIGUSR2 recieved: %d\n", count->sentSIGUSR2, count->receivedSIGUSR2);
                            exit(0);
                        }
                    }
            }
            sleep(1);
        }
    }

    return 0;
}

//frees shared memory (was used in testing)
//didnt have time to remove from code
//orignially had a different structure but didnt get to remove this
void signalHandler(int signal) {

    if(signal == SIGINT) {
        int detatchedVal = shmdt(count);
        if(detatchedVal == -1) {
            printf("Failed to detatch shared memory\n");
        }
        exit(0);
    }if(signal == SIGUSR1) {
        //printf("Received SIGUSER1 in signal handle");
    }
    else if(signal == SIGUSR2) {
        //printf("Received SIGUSER2 in signal handle");
    }
    
}


void signalHandler1(int sig) {
    
    //used to free memory when terminating
    unblock_signal(SIGINT);
    signal(SIGINT, signalHandler);
    if(sig == SIGUSR1) {
        //printf("Unblocking signal\n");
        unblock_signal(SIGUSR1);
        signal(SIGUSR1, signalHandler);
    }
    
    //loop and wait for signal
    while(true) {
        //wait until signal is given
        pause();
        
        //if signal is SIGUSR1, then grab lock and increase counter
        if(sig == SIGUSR1) {
            printf("Received SIGUSR1 by %d\n", getpid());
            pthread_mutex_lock(&count->signal1Lock);
            count->receivedSIGUSR1++;
            pthread_mutex_unlock(&count->signal1Lock);
        }

    }

    return;
}

void signalHandler2(int sig) {

    //if terminating process, clear memory
    unblock_signal(SIGINT);
    signal(SIGINT, signalHandler);
    if(sig == SIGUSR2) {
        //printf("Unblocking signal\n");
        unblock_signal(SIGUSR2);
        signal(SIGUSR2, signalHandler);
    }
    
    while(true) {
        pause();
        
        if(sig == SIGUSR2) {
            printf("Received SIGUSR2 by %d\n", getpid());
            pthread_mutex_lock(&count->signal2Lock);
            count->receivedSIGUSR2++;
            pthread_mutex_unlock(&count->signal2Lock);
        }

    }

    return;
}

//does the time for signals
void reporterTimer(int sig){
    
    if(sig == SIGINT) {
        //free shared memory
        int detatchedVal = shmdt(count);
        if(detatchedVal == -1) {
            printf("Error");
        }
        exit(0);
    }
    
    //adds the current time to signal timer
    if(sig == SIGUSR1) {
        clock_gettime(CLOCK_REALTIME, &timeSIGUSR1);
        timeSumSIGUSR1 += timeSIGUSR1.tv_sec + timeSIGUSR1.tv_nsec;
        timeSumSIGUSR1 /= 1000000000;
    }
    else if(sig == SIGUSR2) {
        clock_gettime(CLOCK_REALTIME, &timeSIGUSR2);
        timeSumSIGUSR2 += timeSIGUSR2.tv_sec + timeSIGUSR2.tv_nsec;
        timeSumSIGUSR2 /= 1000000000;
    }
    
}

void reporter(int sig) {

    //printf("reporter\n");

    unblock_signal(SIGUSR1);
    unblock_signal(SIGUSR2);
    unblock_signal(SIGINT);
    signal(SIGINT, reporterTimer);
    signal(SIGUSR1, reporterTimer);
    signal(SIGUSR2, reporterTimer);
    
    //open log file and error check
    FILE *logF = fopen("logFile.txt", "w");
    if(logF == NULL){
        printf("Error opening log file\n");
        exit(0);
    }
    
    //loop
    while(true) {
        
        //wait until signaled
        pause();
        
        //getting total signals sent and checking if 16 signals have been sent 
        int total = (count->sentSIGUSR1 + count->sentSIGUSR2);
        if((total % 16) == 0) {
            
            //takes the total time of each signal and divides it by the total signals counted (16)
            double avgTime1 = timeSumSIGUSR1/16.0;
            double avgTime2 = timeSumSIGUSR2/16.0;
            
            //printing sent and recieved of each signal, the current time, and the average time between signals
            time_t curtime;
            time(&curtime);
            fprintf(logF, "SIGUSR1: Sent: %d, Received: %d, Current Time: %s, Avg Between Signals: %f\n", count->sentSIGUSR1, count->receivedSIGUSR1, ctime(&curtime), avgTime1);
            fprintf(logF, "SIGUSR2: Sent: %d, Received: %d, Current Time: %s, Avg Between Signals: %f\n", count->sentSIGUSR2, count->receivedSIGUSR2, ctime(&curtime), avgTime2);
            
            //reset time tracking variables
            avgTime1 = 0;
            avgTime2 = 0;
            timeSumSIGUSR1 = 0;
            timeSumSIGUSR2 = 0;
            
        }
        
    }
}

//uses random number generator to send a random signal
//then sleeps for a random amount of time before sending another
void generator() {

    //printf("Generator\n");
    
    srand(time(NULL));

    //loops to keep generating signals
    while(true) {
        double randNum = randGenerator();
        if(randNum < 0.5) {
            //printf("Sent SIGUSR1\n");
            pthread_mutex_lock(&count->signal1Lock);
            count->sentSIGUSR1++;
            pthread_mutex_unlock(&count->signal1Lock);
            kill(0, SIGUSR1);
        }
        else if(randNum >= 0.5){
            //printf("Sent SIGUSR2\n");
            pthread_mutex_lock(&count->signal2Lock);
            count->sentSIGUSR2++;
            pthread_mutex_unlock(&count->signal2Lock);
            kill(0, SIGUSR2);
            
        }
        struct timespec sleepTime;
        sleepTime.tv_sec = 0;
        sleepTime.tv_nsec = randGenerator(0.01, 0.1) * 1000000000;
        nanosleep(&sleepTime, NULL);
    }

}

//generates a random number from 0.01 to 1.00
double randGenerator() {
    int number = (rand() % 100) + 1;
    double num = (double)number/100;
    return num;
}

void block_signal(int signal) {
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, signal);
    int sigReturn = sigprocmask(SIG_BLOCK, &sigset, NULL);
    if(sigReturn != 0) {
        printf("Error in block_signal()\n");
    }
    
}

void unblock_signal(int signal) {
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, signal);
    int sigReturn = sigprocmask(SIG_UNBLOCK, &sigset, NULL);
    if(sigReturn != 0) {
        printf("Error in unblock_signal()\n");
    }
    
}

//returns true if the amount of SIGUSR1 sent + SIGUSR2 sent is >= the max amount signals
//returns false if the total sent is not equal 
bool maxSignalsCreated() {
    int totalSent = (count->sentSIGUSR1 + count->sentSIGUSR2);
    if(totalSent >= NUM_MAX_SIGNALS) {
        return true;
    }
    return false;
}
