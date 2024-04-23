#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>

// Инициализируем 5 пчел и горшочек вместительностью 10
#define N 5
#define H 10
#define BEE_WORKING_TIME 5

sem_t* full_sems[H];
sem_t* mutex;
char shared_memory_name[] = "shared_memory";
char* full_sems_names[H] = {"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9"};
char mutex_name[] = "m";

typedef struct{
	int count;
	int is_full;
} cup;

void *bee(void *arg) {
    int bee_id = *(int *)arg;
    int buf_id = shm_open(shared_memory_name, O_CREAT|O_RDWR, 0666);
    ftruncate(buf_id, sizeof(cup));
    cup* buffer = mmap(0, sizeof (cup), PROT_WRITE|PROT_READ, MAP_SHARED, buf_id, 0);
    while (1) {
        sem_wait(mutex);
        buffer->count++;
        printf("Bee %d putting honey in the pot\n", bee_id);
        sem_post(mutex);
        sem_post(full_sems[buffer->count % H]);
        sleep(rand() % BEE_WORKING_TIME);
    }
    
    shm_unlink(shared_memory_name);
}

void *bear(void *arg) {
    int buf_id = shm_open(shared_memory_name, O_CREAT|O_RDWR, 0666);
    ftruncate(buf_id, sizeof(cup));
    cup* buffer = mmap(0, sizeof (cup), PROT_WRITE|PROT_READ, MAP_SHARED, buf_id, 0);
    while (1) {
        sem_wait(full_sems[H - 1]);
        sem_wait(mutex);
        printf("Bear eating honey\n");
        buffer->count = 0;
        sem_post(mutex);
    }
    
    shm_unlink(shared_memory_name);
}

int main() {
    pthread_t bees[N], bear_t;
    int bee_ids[N];

    for (int i = 0; i < H; i++) {
    	full_sems[i] = sem_open(full_sems_names[i], O_CREAT, 0666, 0);
    }
    mutex = sem_open(mutex_name, O_CREAT, 0666, 1);


    for (int i = 0; i < N; i++) {
        bee_ids[i] = i;
        pthread_create(&bees[i], NULL, bee, &bee_ids[i]);
    }
    pthread_create(&bear_t, NULL, bear, NULL);

    for (int i = 0; i < N; i++) {
        pthread_join(bees[i], NULL);
    }
    pthread_join(bear_t, NULL);

    for (int i = 0; i < H; i++) {
        sem_destroy(full_sems[i]);
    }
    sem_destroy(mutex);

    return 0;
}

