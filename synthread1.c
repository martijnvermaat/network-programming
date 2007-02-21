#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

void display(char *str) {
  char *tmp;
  for (tmp=str;*tmp;tmp++) {
    write(1,tmp,1);
    usleep(100);
  }
}

void *bonjour(void *param) {
    for (i=0;i<10;i++) {
      semop(mutex, &down, 1);
      display("Bonjour monde\n");
      semop(mutex, &up, 1);
    }
    return NULL;
}

int main() {

  int i;
  pthread_t id;
  pthread_attr_t attr;

  int mutex;

  mutex = semget(IPC_PRIVATE, 1, 0600);
  semop(mutex, &up, 1);  // Initialize for mutual exclusion

  pthread_attr_init(&attr);
  pthread_create(&id, &attr, bonjour, (void *) NULL);

  for (i=0;i<10;i++) {
    semop(mutex, &down, 1);
    display("Hello world\n");
    semop(mutex, &up, 1);
  }

  pthread_join(id, NULL);

  return 0;

}
