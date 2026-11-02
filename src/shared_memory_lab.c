#include "shared_memory_lab.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

int create_segment(void) {
    return shmget(IPC_PRIVATE, sizeof(struct shared_block), 0600 | IPC_CREAT);
}

void cleanup_segment(int segment_id, struct shared_block *block) {
    if (shmdt(block) == -1) {
        perror("shmdt");
    }
    if (shmctl(segment_id, IPC_RMID, NULL) == -1) {
        perror("shmctl");
    }
}

static void wait_for_ready(const struct shared_block *block) {
    while (!block->ready) {
        usleep(1000);
    }
}

int task_fast_exchange(void) {
    int segment_id;
    pid_t child_pid;
    int status;
    struct shared_block *block;

    segment_id = create_segment();
    if (segment_id == -1) {
        perror("shmget");
        return 1;
    }

    block = shmat(segment_id, NULL, 0);
    if (block == (void *)-1) {
        perror("shmat");
        return 1;
    }
    memset(block, 0, sizeof(*block));

    child_pid = fork();
    if (child_pid < 0) {
        perror("fork");
        cleanup_segment(segment_id, block);
        return 1;
    }

    if (child_pid == 0) {
        /* TODO: wait until parent marks block->ready. */
        wait_for_ready(block);

        /* TODO: print: child fast read: shared memory is fast */
        printf("child fast read: %s\n", block->message);

        /* TODO: detach shared memory and return 0 on success. */
        if(shmdt(block) == -1) {
            perror("shmdt");
            _exit(1);
        }
        fflush(stdout);
        _exit(0);

        //fprintf(stderr, "TODO: task_fast_exchange child path not implemented\n");
        //return 1;
    }

    /* TODO: copy "shared memory is fast" into block->message. */
    strcpy(block->message, "shared memory is fast");

    /* TODO: set block->ready = 1 so child can read it. */
    block->ready = 1;

    if (waitpid(child_pid, &status, 0) == -1) {
        perror("waitpid");
        cleanup_segment(segment_id, block);
        return 1;
    }

    printf("parent fast write: done\n");
    fflush(stdout);

    cleanup_segment(segment_id, block);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? 0 : 1;
}

int task_latest_value_limit(void) {
    int segment_id;
    pid_t child_pid;
    int status;
    struct shared_block *block;

    segment_id = create_segment();
    if (segment_id == -1) {
        perror("shmget");
        return 1;
    }

    block = shmat(segment_id, NULL, 0);
    if (block == (void *)-1) {
        perror("shmat");
        return 1;
    }
    memset(block, 0, sizeof(*block));

    child_pid = fork();
    if (child_pid < 0) {
        perror("fork");
        cleanup_segment(segment_id, block);
        return 1;
    }

    if (child_pid == 0) {
        /* TODO: wait until parent sets ready. */
        wait_for_ready(block);

        /* TODO: print: child latest view: latest-only */
        printf("child latest view: %s\n", block->message);

        /* TODO: detach and return 0 on success. */
        if(shmdt(block) == -1) {
            perror("shmdt");
            _exit(1);
        }
        fflush(stdout);
        _exit(0);

        //fprintf(stderr, "TODO: task_latest_value_limit child path not implemented\n");
        //return 1;
    }

    /* TODO: parent writes "first update" then "latest-only" to block->message. */
    strcpy(block->message, "first update");
    strcpy(block->message, "latest-only");

    /* TODO: set ready and let child read only the latest value. */
    block->ready = 1;

    if (waitpid(child_pid, &status, 0) == -1) {
        perror("waitpid");
        cleanup_segment(segment_id, block);
        return 1;
    }
    printf("limit observed: no history in shared memory\n");
    fflush(stdout);

    cleanup_segment(segment_id, block);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? 0 : 1;
}

int task_capacity_limit(void) {
    int segment_id;
    pid_t child_pid;
    int status;
    struct shared_block *block;
    const char *source = "abcdefghijklmnopqrstuvwxyz";

    segment_id = create_segment();
    if (segment_id == -1) {
        perror("shmget");
        return 1;
    }

    block = shmat(segment_id, NULL, 0);
    if (block == (void *)-1) {
        perror("shmat");
        return 1;
    }
    memset(block, 0, sizeof(*block));

    child_pid = fork();
    if (child_pid < 0) {
        perror("fork");
        cleanup_segment(segment_id, block);
        return 1;
    }

    if (child_pid == 0) {
        /* TODO: wait for ready flag. */
        wait_for_ready(block);

        /* TODO: print: child capacity bytes: 15 */
        printf("child capacity bytes: %zu\n", strlen(block->message));

        /* TODO: detach and return 0 on success. */
        if(shmdt(block) == -1) {
            perror("shmdt");
            _exit(1);
        }
        fflush(stdout);
       _exit(0);

        //fprintf(stderr, "TODO: task_capacity_limit child path not implemented\n");
        //return 1;
    }

    (void)source;
    /* TODO: copy source into block->message safely with snprintf. */
    snprintf(block->message, sizeof(block->message), "%.15s", source);

    /* TODO: set ready, wait for child, then print: parent source bytes: 26 */
    block->ready = 1; 
    if (waitpid(child_pid, &status, 0) == -1) {
        perror("waitpid");
        cleanup_segment(segment_id, block);
        return 1;
    }
    printf("parent source bytes: %zu\n", strlen(source));   
    fflush(stdout);

    cleanup_segment(segment_id, block);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? 0 : 1;
}

int main(void) {
    if (task_fast_exchange() != 0) {
        return 1;
    }

    if (task_latest_value_limit() != 0) {
        return 1;
    }

    if (task_capacity_limit() != 0) {
        return 1;
    }

    printf("all tasks done\n");
    return 0;
}