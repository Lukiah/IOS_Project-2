#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>
#include<time.h>
#include<unistd.h>

//prototype section
int mainProc(int nH, int nO, int lineWaitT, int molCreateT);
void trash(); //cleans up leftover processes TODO

int main(int argc, char *argv[]){
    int nH = 0,nO = 0, tI, tB;
    FILE *fOut; // proj2.out pointer

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

    if ((fOut = fopen("proj2.out", "w")) == NULL){ // invalid pointer to proj2.out
        fprintf(stderr, "An error has occured while opening proj2.out file\n");
        return 1;
    }

    for (int i = 1; i <= nO; i++){ // Oxygen processes creator
        pid_t id = fork();
        if(id == 0){ //then successor/child process
            //oxygen(i); //TODO
            exit(0);
        }
    }

    for (int i = 1; i <= nH; i++){ // Hydrogen processes creator
        pid_t id = fork();
        if(id == 0){ //then successor/child process
            //hydrogen(i); //TODO
            exit(0);
        }
    }

    //while(wait(NULL) > 0);
    //trash();



}