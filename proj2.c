#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>

sem_t *printSem;
sem_t *qMutex;
sem_t *oQueue; 
sem_t *hQueue;
sem_t *barrierMutex;
sem_t *turnstile1;
sem_t *turnstile2;
sem_t *killMutex;


FILE *fOut; // proj2.out pointer
int *lineNum;
int *molNum;
int *hydrogen, *oxygen;
int *cnt;
int *possibleMolecules;
int *killRemaining;

void initSequence(){
    //shared memory section
    lineNum = mmap(NULL, sizeof(*lineNum), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0); // lineNum is now a shared memory variable
    if (lineNum == MAP_FAILED){
        fprintf(stderr, "An attempt to map memory has failed.");
        exit(1);
    }
    (*lineNum) = 1;

    molNum = mmap(NULL, sizeof(*molNum), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0); // lineNum is now a shared memory variable
    if (molNum == MAP_FAILED){
        fprintf(stderr, "An attempt to map memory has failed.");
        exit(1);
    }
    (*molNum) = 1;

    hydrogen = mmap(NULL, sizeof(*hydrogen), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0); // lineNum is now a shared memory variable
    if (hydrogen == MAP_FAILED){
        fprintf(stderr, "An attempt to map memory has failed.");
        exit(1);
    }
    (*hydrogen) = 0;

    oxygen = mmap(NULL, sizeof(*oxygen), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0); // lineNum is now a shared memory variable
    if (oxygen == MAP_FAILED){
        fprintf(stderr, "An attempt to map memory has failed.");
        exit(1);
    }
    (*oxygen) = 0;

    cnt = mmap(NULL, sizeof(*cnt), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0); // lineNum is now a shared memory variable
    if (cnt == MAP_FAILED){
        fprintf(stderr, "An attempt to map memory has failed.");
        exit(1);
    }
    (*cnt)=0;

    possibleMolecules = mmap(NULL, sizeof(*possibleMolecules), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0); // lineNum is now a shared memory variable
    if (possibleMolecules == MAP_FAILED){
        fprintf(stderr, "An attempt to map memory has failed.");
        exit(1);
    }

    killRemaining = mmap(NULL, sizeof(*killRemaining), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0); // lineNum is now a shared memory variable
    if (killRemaining == MAP_FAILED){
        fprintf(stderr, "An attempt to map memory has failed.");
        exit(1);
    }
    (*killRemaining)=0;


    //semaphore section
    printSem = sem_open("/xzedek03_printSem", O_CREAT | O_EXCL, 0666, 1);
    if (printSem == SEM_FAILED){ // failed to initialize
        fprintf(stderr, "An attempt to initialize a semaphore has failed.\n");
        exit(1);
    }
    oQueue = sem_open("/xzedek03_oQueue", O_CREAT | O_EXCL, 0666, 1);
    if (oQueue == SEM_FAILED){ // failed to initialize
        fprintf(stderr, "An attempt to initialize a semaphore has failed.\n");
        exit(1);
    }
    hQueue = sem_open("/xzedek03_hQueue", O_CREAT | O_EXCL, 0666, 0);
    if (hQueue == SEM_FAILED){ // failed to initialize
        fprintf(stderr, "An attempt to initialize a semaphore has failed.\n");
        exit(1);
    }
    qMutex = sem_open("/xzedek03_qMutex", O_CREAT | O_EXCL, 0666, 1);
    if (qMutex == SEM_FAILED){ // failed to initialize
        fprintf(stderr, "An attempt to initialize a semaphore has failed.\n");
        exit(1);
    }
    barrierMutex = sem_open("/xzedek03_barrierMutex", O_CREAT | O_EXCL, 0666, 1);
    if (barrierMutex == SEM_FAILED){ // failed to initialize
        fprintf(stderr, "An attempt to initialize a semaphore has failed.\n");
        exit(1);
    }
    turnstile1 = sem_open("/xzedek03_turnstile1", O_CREAT | O_EXCL, 0666, 0);
    if (turnstile1 == SEM_FAILED){ // failed to initialize
        fprintf(stderr, "An attempt to initialize a semaphore has failed.\n");
        exit(1);
    }
    turnstile2 = sem_open("/xzedek03_turnstile2", O_CREAT | O_EXCL, 0666, 1);
    if (turnstile2 == SEM_FAILED){ // failed to initialize
        fprintf(stderr, "An attempt to initialize a semaphore has failed.\n");
        exit(1);
    }
    killMutex = sem_open("/xzedek03_killMutex", O_CREAT | O_EXCL, 0666, 1);
    if (killMutex == SEM_FAILED){ // failed to initialize
        fprintf(stderr, "An attempt to initialize a semaphore has failed.\n");
        exit(1);
    }
}

void trash(){ //cleans up semaphores and shared memory
    //semaphore destroying section
    sem_close(printSem);
    sem_unlink("/xzedek03_printSem");
    sem_close(oQueue);
    sem_unlink("/xzedek03_oQueue");
    sem_close(hQueue);
    sem_unlink("/xzedek03_hQueue");
    sem_close(qMutex);
    sem_unlink("/xzedek03_qMutex");
    sem_close(barrierMutex);
    sem_unlink("/xzedek03_barrierMutex");
    sem_close(turnstile1);
    sem_unlink("/xzedek03_turnstile1");
    sem_close(turnstile2);
    sem_unlink("/xzedek03_turnstile2");
    sem_close(killMutex);
    sem_unlink("/xzedek03_killMutex");

    //memory unmapping section
    munmap(lineNum, sizeof(*lineNum));
    munmap(molNum, sizeof(*molNum));
    munmap(hydrogen, sizeof(*hydrogen));
    munmap(oxygen, sizeof(*oxygen));
    munmap(cnt, sizeof(*cnt));
    munmap(possibleMolecules, sizeof(*possibleMolecules));
    munmap(killRemaining, sizeof(*killRemaining));
}

void flushPrint (char * text, ...) // TODO change stdout to *fOut
{
    sem_wait(printSem); // wait until noone else is writing into file
    va_list args;
    va_start (args, text);
    fprintf(stdout, "%d: ",(*lineNum)); //'lineNum: '
    (*lineNum)++;
    vfprintf(stdout, text, args);
    fflush(stdout);
    va_end (args);
    sem_post(printSem);
}

void oFc(int oNum, int TI, int TB){   
    srand(time(NULL) * getpid());
    srand(time(NULL) * getpid());
    flushPrint("O %d: started\n",oNum);
    usleep(1000 * (rand() % (TI + 1))); // TI + 1 so that max value is TI, since modulo "takes that possibility away"
    flushPrint("O %d: going to queue\n",oNum);
    sem_wait(qMutex);
    (*oxygen)++;
    if ((*hydrogen) >= 2){
        sem_post(hQueue);
        sem_post(hQueue);
        (*hydrogen) -= 2;
        sem_post(oQueue);
        (*oxygen) -= 1;
    } else {
        sem_post(qMutex);
    } 
    sem_wait(oQueue);
    //bond()

    if ((*killRemaining)){ //cleanup
        flushPrint("O: %d: not enough H\n", oNum);
        return;
    }

    flushPrint("O: %d: creating molecule %d\n", oNum, (*molNum));

    //barrier.wait() START
    sem_wait(barrierMutex);
    (*cnt) += 1;
    if ((*cnt) == 3){
        sem_wait(turnstile2);
        sem_post(turnstile1);
    }
    sem_post(barrierMutex);

    sem_wait(turnstile1);
    sem_post(turnstile1);

    // critical section
    usleep(1000 * (rand() % (TB + 1)));
    flushPrint("O: %d: molecule %d created\n", oNum, (*molNum));

    sem_wait(barrierMutex);
    (*cnt) -= 1;
    if ((*cnt) == 0){
        sem_wait(turnstile1);
        sem_post(turnstile2);
    }
    sem_post(barrierMutex);

    sem_wait(turnstile2);
    sem_post(turnstile2);
    //barrier.wait() END
    (*molNum)++;
    sem_post(qMutex);    

    sem_wait(killMutex);
    if ((*possibleMolecules)<(*molNum)){ //last molecule
        (*killRemaining)=1;
        for(int j = 0; j < (*oxygen); j++){
            sem_post(oQueue);
        }
        for(int k = 0; k < (*hydrogen); k++){
            sem_post(hQueue);
        }
    } 
    sem_post(killMutex);

}

void hFc(int hNum, int TI){
    srand(time(NULL) * getpid());
    srand(time(NULL) * getpid());
    flushPrint("H %d: started\n",hNum);
    usleep(1000 * (rand() % (TI + 1)));
    flushPrint("H %d: going to queue\n",hNum);
    sem_wait(qMutex);
    (*hydrogen)++;
    if ((*hydrogen) >= 2 && (*oxygen) >= 1) {
        sem_post(hQueue);
        sem_post(hQueue);
        (*hydrogen) -= 2;
        sem_post(oQueue);
        (*oxygen) -= 1;
    } else {
        sem_post(qMutex); 
    }
    sem_wait(hQueue); //post this x times for each excess H

    if ((*killRemaining)){ //cleanup
        flushPrint("H: %d: not enough O or H\n", hNum);
        return;
    }

    flushPrint("H: %d: creating molecule %d\n", hNum, (*molNum));

    //barrier.wait() START
    sem_wait(barrierMutex);
    (*cnt) += 1;
    if ((*cnt) == 3){
        sem_wait(turnstile2);
        sem_post(turnstile1);
    }
    sem_post(barrierMutex);

    sem_wait(turnstile1);
    sem_post(turnstile1);

    flushPrint("H: %d: molecule %d created\n", hNum, (*molNum));

    sem_wait(barrierMutex);
    (*cnt) -= 1;
    if ((*cnt) == 0){
        sem_wait(turnstile1);
        sem_post(turnstile2);
    }
    sem_post(barrierMutex);

    sem_wait(turnstile2);
    sem_post(turnstile2);
    //barrier.wait() END
}

int main(int argc, char *argv[]){
    int nH = 0, nO = 0, tI, tB;

    if (argc == 5)
    {
        nO = atoi(argv[1]);
        nH = atoi(argv[2]);
        tI = atoi(argv[3]);
        tB = atoi(argv[4]);

        if ((nO > 0) && (nH > 0) && (tI > 0) && (tB > 0)) // nO and nH input args check
        {
            if ((tI <= 1000) && (tB <= 1000)) // timer check 
            {   
                trash();
                trash();
                trash();
                initSequence();
                (*possibleMolecules)=(((nH / 2) < (nO)) ? (nH / 2) : (nO));

                if ((fOut = fopen("proj2.out", "w")) == NULL){ 
                    fprintf(stderr, "An error has occured while opening proj2.out file\n");
                    exit(1);
                }
                for (int i = 1; i <= nH; i++){ // Hydrogen processes creator
                    pid_t id = fork();
                    if(id == 0){ //== successor/child process
                        hFc(i,tI);
                        exit(0);
                    }
                }
                
                for (int i = 1; i <= nO; i++){ // Oxygen processes creator
                    pid_t id = fork();
                    if(id == 0){ //== successor/child process
                        oFc(i,tI,tB);
                        exit(0);
                    }
                }
                
                while(wait(NULL) > 0); //main process waits until all its child processes are done
                trash();
                fclose(fOut);
                flushPrint("program run successful\n");
            }
            else
            {
                fprintf(stderr, "timer parameters outside of allowed range\n"); //for n0 <= TI/TB <= 1000
                exit(1);    
            }
        }
        else
        {
            fprintf(stderr, "negative arguments or arguments equal to zero entered\n"); //for nO/nH <= 0
            exit(1); 
        }
    } 
    else
    {
        fprintf(stderr, "invalid amount of parameters\n"); //for argc != 5
        exit(1);
    }

}