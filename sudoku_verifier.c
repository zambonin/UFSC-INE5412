#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

#define SIZE  9
#define THREAD_ID       syscall(__NR_gettid)
#define hamming_comp(x)  __builtin_popcount(~x & ((1 << (SIZE + 1)) - 1))

typedef void (*func_ptr_t)(short);
typedef struct { func_ptr_t check; short param; } task;

static short error_count = 0;
static unsigned short grid[SIZE][SIZE];
static pthread_t *threads;
static pthread_mutex_t scheduler, function;
static task tasks[SIZE * 3];
static int available_tasks = (1 << 27) - 1;

void load_grid(unsigned short[][SIZE], char*);
void check_row(short);
void check_col(short);
void check_sqr(short);
void* choose_task();

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: %s input_grid nthreads\n", argv[0]);
    exit(1);
  }

  load_grid(grid, argv[1]);

  int nthreads = atoi(argv[2]);
  threads = calloc((unsigned int) nthreads, sizeof(pthread_t));

  printf("Quebra-cabecas fornecido:\n");
  for (int i = 0; i < SIZE; ++i) {
    for (int j = 0; j < SIZE; ++j) {
      printf("%hu ", grid[i][j]);
    }
    printf("\n");
  }

  for (short i = 0; i < SIZE; ++i) {
    tasks[i].check = check_row;
    tasks[i].param = i % 9;
  }

  for (short i = SIZE; i < SIZE * 2; ++i) {
    tasks[i].check = check_col;
    tasks[i].param = i % 9;
  }

  for (short i = SIZE * 2; i < SIZE * 3; ++i) {
    tasks[i].check = check_sqr;
    tasks[i].param = i % 9;
  }

  pthread_mutex_init(&scheduler, NULL);
  pthread_mutex_init(&function, NULL);
  for (int i = 0; i < nthreads; ++i) {
    pthread_create(&threads[i], NULL, choose_task, NULL);
  }

  for (int i = 0; i < nthreads; ++i) {
    pthread_join(threads[i], NULL);
  }
  pthread_mutex_destroy(&function);
  pthread_mutex_destroy(&scheduler);

  printf("Erros encontrados: %d.\n", error_count);

  free(threads);

  return EXIT_SUCCESS;
}

void load_grid(unsigned short grid[][SIZE], char* file) {
  FILE* input = fopen(file, "r");

  if (input == NULL) {
    exit(1);
  }

  for (int i = 0; i < SIZE; ++i) {
    for (int j = 0; j < SIZE; ++j) {
      fscanf(input, "%hu", &grid[i][j]);
    }
  }

  fclose(input);
}

void check_row(short row_index) {
  unsigned short repeated = 1;

  for (short i = 0; i < SIZE; ++i) {
    repeated |= 1 << grid[row_index][i];
  }

  short errors = hamming_comp(repeated);
  if (errors) {
    printf("Thread %ld: erro na linha %d.\n", THREAD_ID, row_index);
    pthread_mutex_lock(&function);
    error_count += errors;
    pthread_mutex_unlock(&function);
  }
}

void check_col(short col_index) {
  unsigned short repeated = 1;

  for (short i = 0; i < SIZE; ++i) {
    repeated |= 1 << grid[i][col_index];
  }

  short errors = hamming_comp(repeated);
  if (errors) {
    printf("Thread %ld: erro na coluna %d.\n", THREAD_ID, col_index);
    pthread_mutex_lock(&function);
    error_count += errors;
    pthread_mutex_unlock(&function);
  }
}

void check_sqr(short sqr_index) {
  short r = sqr_index / 3 * 3, c = sqr_index % 3 * 3, repeated = 1;

  for (short i = r; i < r + 3; ++i) {
    for (short j = c; j < c + 3; ++j) {
      repeated |= 1 << grid[i][j];
    }
  }

  short errors = hamming_comp(repeated);
  if (errors) {
    printf("Thread %ld: erro na regiao %d.\n", THREAD_ID, sqr_index);
    pthread_mutex_lock(&function);
    error_count += errors;
    pthread_mutex_unlock(&function);
  }
}

void* choose_task() {
  for (short i = 0; i < SIZE * 3; ++i) {
    pthread_mutex_lock(&scheduler);
    if ((available_tasks >> i) & 1) {
      available_tasks &= ~(1 << i);
      tasks[i].check(tasks[i].param);
    }
    pthread_mutex_unlock(&scheduler);
  }

  return NULL;
}
