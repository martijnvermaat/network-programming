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
  int event_ab;
  int event_cd;

  event_ab = semget(IPC_PRIVATE, 1, 0600);  // Start printing "ab"
  event_cd = semget(IPC_PRIVATE, 1, 0600);  // Start printing "cd\n"

  pid = fork();

  if (pid < 0) { perror("Fork error"); exit(1); }

  if (pid == 0) {

    for (i=0;i<10;i++) {
      semop(event_ab, &up, 1);
      semop(event_cd, &down, 1);
      display("cd\n");
    }

  } else {

    for (i=0;i<10;i++) {
      semop(event_ab, &down, 1);
      display("ab");
      semop(event_cd, &up, 1);
    }

    wait(NULL);
    semctl(event_ab, 0, IPC_RMID);
    semctl(event_cd, 0, IPC_RMID);

  }

  return 0;

}
