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

// $ ./proj2 NO NH TI TB


sem_t *baseMutex;
sem_t *printSem; // write to file semaphore
sem_t *OBarrSem; 
sem_t *HBarrSem;
sem_t *BarrierMutex;

FILE *fOut; // proj2.out pointer
int *lineNum;
int *hydroLeft, *oxyLeft;

void initSequence(){
    //shared memory section
    lineNum = mmap(NULL, sizeof(*lineNum), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0); // lineNum is now a shared memory variable
    if (lineNum == MAP_FAILED){
        fprintf(stderr, "An attempt to map memory has failed.");
        exit(1);
    }
    (*lineNum) = 1;

    hydroLeft = mmap(NULL, sizeof(*hydroLeft), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0); // lineNum is now a shared memory variable
    if (hydroLeft == MAP_FAILED){
        fprintf(stderr, "An attempt to map memory has failed.");
        exit(1);
    }

    oxyLeft = mmap(NULL, sizeof(*oxyLeft), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0); // lineNum is now a shared memory variable
    if (oxyLeft == MAP_FAILED){
        fprintf(stderr, "An attempt to map memory has failed.");
        exit(1);
    }


    //semaphore section
    baseMutex = sem_open("/xzedek03_baseMutex", O_CREAT | O_EXCL, 0666, 1);
    if (baseMutex == SEM_FAILED){ // failed to initialize
        fprintf(stderr, "An attempt to initialize a semaphore has failed.\n");
        exit(1);
    }
    printSem = sem_open("/xzedek03_printSem", O_CREAT | O_EXCL, 0666, 1);
    if (printSem == SEM_FAILED){ // failed to initialize
        fprintf(stderr, "An attempt to initialize a semaphore has failed.\n");
        exit(1);
    }
    OBarrSem = sem_open("/xzedek03_OBarrSem", O_CREAT | O_EXCL, 0666, 1);
    if (OBarrSem == SEM_FAILED){ // failed to initialize
        fprintf(stderr, "An attempt to initialize a semaphore has failed.\n");
        exit(1);
    }
    HBarrSem = sem_open("/xzedek03_HBarrSem", O_CREAT | O_EXCL, 0666, 2);
    if (HBarrSem == SEM_FAILED){ // failed to initialize
        fprintf(stderr, "An attempt to initialize a semaphore has failed.\n");
        exit(1);
    }
    BarrierMutex = sem_open("/xzedek03_BarrierMutex", O_CREAT | O_EXCL, 0666, 2);
    if (BarrierMutex == SEM_FAILED){ // failed to initialize
        fprintf(stderr, "An attempt to initialize a semaphore has failed.\n");
        exit(1);
    }


}

void trash(){ 
    //semaphore destroying section
    sem_close(baseMutex);
    sem_unlink("/xzedek03_baseMutex");
    sem_close(printSem);
    sem_unlink("/xzedek03_printSem");
    sem_close(OBarrSem);
    sem_unlink("/xzedek03_OBarrSem");
    sem_close(HBarrSem);
    sem_unlink("/xzedek03_HBarrSem");
    sem_close(BarrierMutex);
    sem_unlink("/xzedek03_BarrierMutex");
    //sem_close(another_semaphore);
    //sem_unlink("/xzedek03_another_semaphore");


    //memory unmapping section
    munmap(lineNum, sizeof(*lineNum));
    munmap(hydroLeft, sizeof(*hydroLeft));
    munmap(oxyLeft, sizeof(*oxyLeft));
}

void flushPrint (char * text, ...) //TODO přepsat stdout na fOut v teto funkci a všude jinde v kódu
{
    sem_wait(printSem); // wait until noone else is writing into file
    va_list args;
    va_start (args, text);
    fprintf(stdout, "%d: ",(*lineNum)); //'lineNum: '
    (*lineNum)++;
    vfprintf(stdout, text, args);
    fflush(stdout);
    va_end (args);
    sem_post(printSem); // free the mutex for other processes to write to file
}

void oFc(int oNum, int TI){   
    srand(time(NULL) * getpid());
    srand(time(NULL) * getpid());
    flushPrint("O %d: started\n",oNum);
    usleep(1000 * (rand() % (TI + 1))); // TI + 1 so that max value is TI, since modulo "takes that possibility away"
    flushPrint("O %d: going to queue\n",oNum);
    //semaphore_wait while going to queue
    //Oxygen queue implementation TODO (possibly as Semaphore(0))
    //semaphore_post while leaving queue - signal to hydrogen queue twice (needs 2 hydrogen atoms)

}

void hFc(int hNum, int TI){
    srand(time(NULL) * getpid());
    srand(time(NULL) * getpid());
    flushPrint("H %d: started\n",hNum);
    usleep(1000 * (rand() % (TI + 1)));
    flushPrint("H %d: going to queue\n",hNum);
    //semaphore_wait while going to queue
    //Hydrogen queue implementation TODO (possibly as Semaphore(0))
    //semaphore_post while leaving queue
    //barrier waits for 2 of Hydrogen
}

void trash(); //cleans up leftover processes TODO

int main(int argc, char *argv[]){
    int nH = 0, nO = 0, tI, tB;
    
    //semaphoresDstr(lineNum); //semaphores failing to initialize <- hotfix

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
                initSequence();
                (*hydroLeft) = nH;
                (*oxyLeft) = nO;

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
                        oFc(i,tI);
                        exit(0);
                    }
                }

                while(wait(NULL) > 0); //main process waits until all its child processes are done
                printf("Parent: 'All the bastards are done. Finally, I can rest.'\n");
            }
            else
            {
                fprintf(stdout, "timer parameters outside of allowed range\n"); //errcode for n0 <= TI/TB <= 1000
                exit(1);    
            }
        }
        else
        {
            fprintf(stderr, "negative arguments or arguments equal to zero entered\n"); //errcode for nO/nH <= 0
            exit(1); 
        }
    } 
    else
    {
        fprintf(stderr, "invalid amount of parameters\n"); //errcode for argc != 5
        exit(1);
    }

    // opens(creates) file or prints on stderr if an error occurs
    if ((fOut = fopen("proj2.out", "w")) == NULL){ 
        fprintf(stderr, "An error has occured while opening proj2.out file\n");
        exit(1);
    }
    trash();
    fclose(fOut);
}