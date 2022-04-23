#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>
#include<time.h>

//prototype section
int mainProc(int nH, int nO, int lineWaitT, int molCreateT);

int main(int argc, char *argv[]){  // will be used mainly for parsing args ---> "$ ./proj2 NO NH TI TB"
    if (argc == 5)
    {
        if ((*argv[1] > 0) && (*argv[2] > 0)) // nH and nO input args check
        {
            if ((0 <= *argv[3] <= 1000) && (0 <= *argv[4] <= 1000))
            {
                //initiate main process
                return 1;
            }
            else
            {
                fprintf(stderr, "negative/zero value timer parameters were entered\n"); //errcode for n0 <= TI/TB <= 1000
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
        fprintf(stderr, "invalid amount of parameters entered\n"); //errcode for argc != 5
        return 1;
    }

}

int mainProc(int nH, int nO, int lineWaitT, int molCreateT) {

    //creates a certain number of H processes and O processes and gets their 

}