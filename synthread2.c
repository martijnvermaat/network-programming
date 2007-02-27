#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>

pthread_mutex_t mutex_ab;
pthread_mutex_t mutex_cd;
pthread_cond_t var_ab;
pthread_cond_t var_cd;
int predicate_ab = 0;
int predicate_cd = 0;

void display(char *str) {
    char *tmp;
    for (tmp=str;*tmp;tmp++) {
        write(1,tmp,1);
        usleep(100);
    }
}

void *cd(void *param) {
    int i;
    for (i=0;i<10;i++) {

        pthread_mutex_lock(&mutex_ab);
        predicate_ab = 1;
        pthread_cond_signal(&var_ab);
        pthread_mutex_unlock(&mutex_ab);

        pthread_mutex_lock(&mutex_cd);
        while (predicate_cd == 0) pthread_cond_wait(&var_cd, &mutex_cd);
        predicate_cd = 0;
        pthread_mutex_unlock(&mutex_cd);

        display("cd\n");
    }
    return NULL;
}

int main() {

    int i;

    pthread_t id;
    pthread_attr_t attr;

    pthread_mutex_init(&mutex_ab, NULL);
    pthread_mutex_init(&mutex_cd, NULL);
    pthread_cond_init(&var_ab, NULL);
    pthread_cond_init(&var_cd, NULL);

    pthread_attr_init(&attr);
    pthread_create(&id, &attr, cd, (void *) NULL);

    for (i=0;i<10;i++) {

        pthread_mutex_lock(&mutex_ab);
        while (predicate_ab == 0) pthread_cond_wait(&var_ab, &mutex_ab);
        predicate_ab = 0;
        pthread_mutex_unlock(&mutex_ab);

        display("ab");

        pthread_mutex_lock(&mutex_cd);
        predicate_cd = 1;
        pthread_cond_signal(&var_cd);
        pthread_mutex_unlock(&mutex_cd);

    }

    pthread_join(id, NULL);

    return 0;

}
