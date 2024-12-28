#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int ready = 0;
int val_to_send = 0;

void *postav(void *arg) {
    int i = 0;
    for (i; i < 10; i++) {
        pthread_mutex_lock(&mutex);
        if (ready) {
            printf("невозможно отправить\n");
        } else {
            pthread_mutex_unlock(&mutex);
            val_to_send = rand()%100 + 1;
            printf("отправлено\t%d\n", val_to_send);
            ready = 1;
            pthread_cond_signal(&cond);
        }
        pthread_mutex_unlock(&mutex);
        sleep(rand() % 3 + 1); 
    }
    return NULL;
}

void *potreb(void *arg) {
    int i = 0;
    for (i; i < 10; i++) {
        pthread_mutex_lock(&mutex);
        while (!ready) {
            pthread_cond_wait(&cond, &mutex);
        }
        printf("получено\t%d\n", val_to_send);
        ready = 0;
        pthread_mutex_unlock(&mutex);
        sleep(rand() % 3 + 1);
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    pthread_t postav_thread, potreb_thread;
    pthread_create(&postav_thread, NULL, postav, NULL);
    pthread_create(&potreb_thread, NULL, potreb, NULL);

    pthread_join(postav_thread, NULL);
    pthread_join(potreb_thread, NULL);
    
}