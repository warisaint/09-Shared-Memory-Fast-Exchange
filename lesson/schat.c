#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MEM_SIZE 2*(sizeof(int)+BUFSIZ)

int SHMID = 0;
void *SH_MEM = NULL;

struct shm_st
{
    int written_0;
    char data_0[BUFSIZ];
    int written_1;
    char data_1[BUFSIZ];
};

void shmrm(int signo)
{
    printf("Detaching Shared Memory...\n");
    if (shmdt(SH_MEM) == -1)
    {
        fprintf(stderr, "shmdt failed\n");
        exit(EXIT_FAILURE);
    }
    
    if (signo == SIGUSR1)
    {
        printf("Removing Shared Memory...\n");
        if (shmctl(SHMID, IPC_RMID, 0) == -1)
        {
            fprintf(stderr, "shmctl failed\n");
            exit(EXIT_FAILURE);
        }
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    int child;
    struct shm_st *sh_area;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <[1, 2]>\n", *argv);
        exit(EXIT_FAILURE);
    }

    SHMID = shmget((key_t) 12345, MEM_SIZE, 0666 | IPC_CREAT);
    if (SHMID == -1)
    {
        perror("shmget() failed\n"); exit(EXIT_FAILURE);
    }

    signal(SIGUSR1, shmrm); signal(SIGUSR2, shmrm);
    child = fork();
    
    if (strcmp(argv[1], "1") == 0)
    {
        switch (child)
        {
            case -1:
                     perror("Forking failed\n"); exit(EXIT_FAILURE);
            case  0:
                     SH_MEM = shmat(SHMID, NULL, 0);
                     if (SH_MEM == (void *) -1)
                     {
                         fprintf(stderr, "shmat failed\n");
                         exit(EXIT_FAILURE);
                     }

                     sh_area = (struct shm_st *) SH_MEM;
                
                     do {
                         if (sh_area->written_1)
                         {
                             printf("%s", sh_area->data_1);
                             sh_area->written_1 = 0;
                         }
                         usleep(100);
                     } while (strncmp(sh_area->data_1, "end chat", 8)); 
                     break;
            default:
                     SH_MEM = shmat(SHMID, NULL, 0);
                     if (SH_MEM == (void *) -1)
                     {
                         fprintf(stderr, "shmat failed\n");
                         exit(EXIT_FAILURE);
                     }

                     sh_area = (struct shm_st *) SH_MEM;
                
                     while (strncmp(sh_area->data_0, "end chat", 8))
                     {
                         while (sh_area->written_0) usleep(200);
                         fgets(sh_area->data_0, BUFSIZ, stdin);
                         sh_area->written_0 = 1;
                     }
        }
    }
    else if (strcmp(argv[1], "2") == 0)
    {
        switch (child)
        {
            case -1:
                     perror("Forking failed\n"); exit(EXIT_FAILURE);
            case  0:
                     SH_MEM = shmat(SHMID, NULL, 0);
                     if (SH_MEM == (void *) -1)
                     {
                         fprintf(stderr, "shmat failed\n");
                         exit(EXIT_FAILURE);
                     }

                     sh_area = (struct shm_st *) SH_MEM;
                
                    do {
                         if (sh_area->written_0)
                         {
                             printf("%s", sh_area->data_0);
                             sh_area->written_0 = 0;
                         }
                         usleep(100);
                     } while (strncmp(sh_area->data_0, "end chat", 8));
                     break;
            default:
                         SH_MEM = shmat(SHMID, NULL, 0);
                     if (SH_MEM == (void *) -1)
                     {
                         fprintf(stderr, "shmat failed\n");
                         exit(EXIT_FAILURE);
                     }

                     sh_area = (struct shm_st *) SH_MEM;
                
                     while (strncmp(sh_area->data_1, "end chat", 8))
                     {
                         while (sh_area->written_1) usleep(200);
                         fgets(sh_area->data_1, BUFSIZ, stdin);
                         sh_area->written_1 = 1;
                     }
        }
    }

    usleep(100);

    if (child)
        kill(child, SIGUSR1);
    else
        kill(getppid(), SIGUSR2);
    
    printf("Detaching Shared Memory...\n");
    if (shmdt(SH_MEM) == -1)
    {
        fprintf(stderr, "shmdt failed\n");
        exit(EXIT_FAILURE);
    }
    
    exit(EXIT_SUCCESS);
}