/*
Para compilar incluir la librería m (matemáticas)
Ejemplo:
gcc -o mercator mercator.c -lm
*/
#include <fcntl.h>
#include <math.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#define NPROCS 4
#define SERIES_MEMBER_COUNT 200000

double *sums;
double x = 1.0;
int *proc_count;
double *res;
sem_t *sem_start_all;
sem_t *sem_count;
sem_t *sem_master;

double get_member(int n, double x) {
  int i;
  double numerator = 1;
  for (i = 0; i < n; i++)
    numerator = numerator * x;
  if (n % 2 == 0)
    return (-numerator / n);
  else
    return numerator / n;
}

void proc(int proc_num) {
  int i;
  sem_wait(sem_start_all);

  sums[proc_num] = 0;
  for (i = proc_num; i < SERIES_MEMBER_COUNT; i += NPROCS)
    sums[proc_num] += get_member(i + 1, x);

  sem_wait(sem_count);
  (*proc_count)++;
  if (*proc_count == NPROCS) {
    sem_post(sem_master);
  }
  sem_post(sem_count);
  exit(0);
}

void master_proc() {
  int i;
  sleep(1);
  for (i = 0; i < NPROCS; i++) {
    sem_post(sem_start_all);
  }

  sem_wait(sem_master);

  *res = 0;
  for (i = 0; i < NPROCS; i++)
    *res += sums[i];
  exit(0);
}

int main() {
  struct timeval start_time, end_time;
  double elapsed_seconds;
  int i, p, shmid;
  void *shmstart;

  size_t page_size = sysconf(_SC_PAGE_SIZE);
  size_t min_size = NPROCS * sizeof(double) + sizeof(int) + sizeof(double);
  size_t shm_size = (min_size + page_size - 1) & ~(page_size - 1);

  shmid = shmget(IPC_PRIVATE, shm_size, IPC_CREAT | 0666);
  if (shmid == -1) {
    perror("shmget");
    exit(1);
  }

  shmstart = shmat(shmid, NULL, 0);
  if (shmstart == (void *)-1) {
    perror("shmat");
    exit(1);
  }

  sums = (double *)shmstart;
  proc_count = (int *)(shmstart + NPROCS * sizeof(double));
  res = (double *)(shmstart + NPROCS * sizeof(double) + sizeof(int));
  *proc_count = 0;

  sem_unlink("/sem_start_all");
  sem_unlink("/sem_count");
  sem_unlink("/sem_master");

  sem_start_all = sem_open("/sem_start_all", O_CREAT, 0666, 0);
  sem_count = sem_open("/sem_count", O_CREAT, 0666, 1);
  sem_master = sem_open("/sem_master", O_CREAT, 0666, 0);

  if (sem_start_all == SEM_FAILED || sem_count == SEM_FAILED ||
      sem_master == SEM_FAILED) {
    perror("sem_open");
    exit(1);
  }

  gettimeofday(&start_time, NULL);

  for (i = 0; i < NPROCS; i++) {
    p = fork();
    if (p == -1) {
      perror("fork");
      exit(1);
    }
    if (p == 0) {
      proc(i);
    }
  }

  p = fork();
  if (p == -1) {
    perror("fork");
    exit(1);
  }
  if (p == 0) {
    master_proc();
  }

  printf("El recuento de ln(1 + x) miembros de la serie de Mercator es %d\n",
         SERIES_MEMBER_COUNT);
  printf("El valor del argumento x es %f\n", x);

  for (i = 0; i < NPROCS + 1; i++) {
    wait(NULL);
  }

  gettimeofday(&end_time, NULL);

  elapsed_seconds = (end_time.tv_sec - start_time.tv_sec) +
                    (end_time.tv_usec - start_time.tv_usec) / 1000000.0;

  printf("Tiempo de ejecucion = %.6f segundos\n", elapsed_seconds);
  printf("El resultado es %10.8f\n", *res);
  printf("Llamando a la función ln(1 + %f) = %10.8f\n", x, log(1 + x));

  shmdt(shmstart);
  shmctl(shmid, IPC_RMID, NULL);
  sem_close(sem_start_all);
  sem_close(sem_count);
  sem_close(sem_master);
  sem_unlink("/sem_start_all");
  sem_unlink("/sem_count");
  sem_unlink("/sem_master");

  return 0;
}