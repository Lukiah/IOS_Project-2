#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/mman.h>


sem_t *mutex;
sem_t *printSem; // write to file semaphore
FILE *fOut; // proj2.out pointer
int lineNum = 1;

void semaphoreInit(){ //initializes a singular semaphore 
    mutex = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
    sem_init(mutex,1,1);
}

void semaphoreDstr(void *shMem){ //destroy semaphore
    sem_destroy(mutex);
    munmap(shMem, sizeof(sem_t));
}

void flushPrint (const char * format, ...)
{
    sem_wait(printSem); // wait until noone else is writing into file
    va_list args;
    va_start (args, format);
    fprintf(fOut, "%d: ",lineNum); //'lineNum:-'
    vfprintf (fOut, format, args);
    lineNum++;
    fflush(fOut);
    va_end (args);
    sem_post(printSem); // free the mutex for other processes to write to file
}

/*
srand pro kazdy proces aby meli fakt random cisla:
srand(time(NULL) * getpid());
usleep po vytvoreni:
usleep(1000 * (rand() % (TI + 1)));
*/

void oFc(int oNum){
    //TODO rest of oxygen business + randomizer
    flushPrint("O %d: ",oNum); // '-O-oNum:-' 
}

void hFc(int hNum){
    //TODO rest of hydrogen business + randomizer
    flushPrint("H %d: ",hNum); // '-H-hNum:-'
}

void trash(); //cleans up leftover processes TODO

int main(int argc, char *argv[]){
    int nH = 0,nO = 0, tI, tB;
    
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
                //param checks passed
            }
            else
            {
                fprintf(stdout, "timer parameters outside of allowed range\n"); //errcode for n0 <= TI/TB <= 1000
                return 1;    
            }
        }
        else
        {
            fprintf(stderr, "negative arguments entered\n"); //errcode for nO/nH <= 0
            return 1; 
        }
    } 
    else
    {
        fprintf(stderr, "invalid amount of parameters\n"); //errcode for argc != 5
        return 1;
    }

    // opens(creates) file or prints on stderr if an error occurs
    if ((fOut = fopen("proj2.out", "w")) == NULL){ 
        fprintf(stderr, "An error has occured while opening proj2.out file\n");
        return 1;
    }

    for (int i = 1; i <= nH; i++){ // Hydrogen processes creator
        pid_t id = fork();
        if(id == 0){ //then successor/child process
            sem_wait(mutex);
            hFc(i);
            sem_post(mutex);
            exit(0);
        }
    }

    for (int i = 1; i <= nO; i++){ // Oxygen processes creator
        pid_t id = fork();
        if(id == 0){ //then successor/child process
            sem_wait(mutex);
            oFc(i);
            sem_post(mutex);
            exit(0);
        }
    }

    while(wait(NULL) > 0);
    //trash(); //after all child processes are done
    fclose(fOut);


}