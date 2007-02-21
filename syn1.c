#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>

void display(char *str) {
  char *tmp;
  for (tmp=str;*tmp;tmp++) {
    write(1,tmp,1);
    usleep(100);
  }
}

int main() {

  int i;
  pid_t pid;

  struct sembuf up = {0, 1, 0};
  struct sembuf down = {0, -1, 0};
  int mutex;

  mutex = semget(IPC_PRIVATE, 1, 0600);
  semop(mutex, &up, 1);  // Initialize for mutual exclusion

  pid = fork();

  if (pid < 0) { perror("Fork error"); exit(1); }

  if (pid == 0) {

    for (i=0;i<10;i++) {
      semop(mutex, &down, 1);
      display("Bonjour monde\n");
      semop(mutex, &up, 1);
    }

  } else {

    for (i=0;i<10;i++) {
      semop(mutex, &down, 1);
      display("Hello world\n");
      semop(mutex, &up, 1);
    }

    wait(NULL);
    semctl(mutex, 0, IPC_RMID);

  }

  return 0;

}
