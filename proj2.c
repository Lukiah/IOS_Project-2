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


sem_t *baseMutex;
sem_t *printSem; // write to file semaphore
FILE *fOut; // proj2.out pointer
int *lineNum;

void semaphoresInit(){
    lineNum = mmap(NULL, sizeof(*lineNum), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0); // sh mem creation
    if (lineNum == MAP_FAILED){
        fprintf(stderr, "Attempt to memory map has failed.");
        exit(1);
    }

    baseMutex = sem_open("/xzedek03_baseMutex", O_CREAT | O_EXCL, 0666, 0);
    if (baseMutex == SEM_FAILED){ // failed to initialize
        fprintf(stderr, "Attempt to initialize a semaphore has failed.");
        exit(1);
    }
}

void semaphoresDstr(){ //destroy semaphore
    sem_close(baseMutex);
    sem_unlink("/xzedek03_sem_mutex");
    //sem_close(another_semaphore);
    //sem_unlink(name_of_another_semaphore);
    munmap(lineNum, sizeof(*lineNum));
}

void flushPrint (const char * format, ...)
{
    sem_wait(printSem); // wait until noone else is writing into file
    va_list args;
    va_start (args, format);
    fprintf(fOut, "%d: ",(*lineNum)); //'lineNum: '
    vfprintf (fOut, format, args);
    (*lineNum)++;
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
    
    flushPrint("O %d: ",oNum); // ' O oNum: ' 
}

void hFc(int hNum){
    //TODO rest of hydrogen business + randomizer
    flushPrint("H %d: ",hNum); // ' H hNum: '
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
                exit(1);    
            }
        }
        else
        {
            fprintf(stderr, "negative arguments entered\n"); //errcode for nO/nH <= 0
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

    semaphoresInit();

    for (int i = 1; i <= nH; i++){ // Hydrogen processes creator
        pid_t id = fork();
        if(id == 0){ //== successor/child process
            hFc(i);
            exit(0);
        }
    }

    for (int i = 1; i <= nO; i++){ // Oxygen processes creator
        pid_t id = fork();
        if(id == 0){ //== successor/child process
            oFc(i);
            exit(0);
        }
    }

    while(wait(NULL) > 0);
    //trash(); //after all child processes are done
    fclose(fOut);

    semaphoresDstr(lineNum);

}