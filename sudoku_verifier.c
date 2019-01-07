#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define SIZE 9
#define SIZE_SQRT 3
#define length(x) (sizeof(x) / sizeof((x)[0]))

void load_grid(uint8_t /*grid*/[][SIZE], char * /*file*/),
    print_errors(uint16_t /*count*/, uint8_t /*index*/, uint16_t /*tid*/,
                 char * /*type*/),
    check_row(uint8_t /*row_index*/, uint16_t /*tid*/),
    check_col(uint8_t /*col_index*/, uint16_t /*tid*/),
    check_sqr(uint8_t /*sqr_index*/, uint16_t /*tid*/);
void *choose_task(void * /*len*/);

static uint8_t error_count = 0, grid[SIZE][SIZE];
static pthread_mutex_t buf_mutex, err_mutex;

typedef void (*func_ptr_t)(uint8_t, uint16_t);
static const func_ptr_t tasks[] = {[0 ... SIZE - 1] = check_row,
                                   [SIZE... SIZE * 2 - 1] = check_col,
                                   [SIZE * 2 ... SIZE * 3 - 1] = check_sqr};
static uint32_t available_tasks = (1U << length(tasks)) - 1;

int32_t main(int32_t argc, char **argv) {
  if (argc != 3) {
    printf("Usage: %s input_grid nthreads\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  load_grid(grid, argv[1]);
  uint16_t nthreads = strtoul(argv[2], NULL, 10);
  pthread_t threads[nthreads];

  pthread_mutex_init(&buf_mutex, NULL);
  pthread_mutex_init(&err_mutex, NULL);

  for (uint16_t i = 0; i < nthreads; ++i) {
    pthread_create(&threads[i], NULL, choose_task, (void *)(intptr_t)(i + 1));
  }

  for (uint16_t i = 0; i < nthreads; ++i) {
    pthread_join(threads[i], NULL);
  }

  pthread_mutex_destroy(&err_mutex);
  pthread_mutex_destroy(&buf_mutex);

  printf("Erros encontrados: %d.\n", error_count);
  exit(EXIT_SUCCESS);
}

void load_grid(uint8_t m[][SIZE], char *file) {
  FILE *input = fopen(file, "re");
  if (input == NULL) {
    exit(EXIT_FAILURE);
  }

  printf("Quebra-cabecas fornecido:\n");
  for (uint8_t i = 0; i < SIZE; ++i) {
    for (uint8_t j = 0; j < SIZE; ++j) {
      fscanf(input, "%hhu ", &m[i][j]); // NOLINT(cert-err34-c)
      printf("%hhu ", m[i][j]);
      m[i][j] -= 1;
    }
    printf("\n");
  }

  fclose(input);
}

void check_row(uint8_t row_index, uint16_t tid) {
  uint16_t repeated = 0;
  for (uint8_t i = 0; i < length(grid[0]); ++i) {
    repeated |= 1U << grid[row_index][i];
  }

  print_errors(repeated, row_index, tid, "linha");
}

void check_col(uint8_t col_index, uint16_t tid) {
  uint16_t repeated = 0;
  for (uint8_t i = 0; i < length(grid[0]); ++i) {
    repeated |= 1U << grid[i][col_index];
  }

  print_errors(repeated, col_index, tid, "coluna");
}

void check_sqr(uint8_t sqr_index, uint16_t tid) {
  uint8_t s = SIZE_SQRT, r = (sqr_index / s) * s, c = (sqr_index % s) * s;
  uint16_t repeated = 0;
  for (uint8_t i = r; i < r + s; ++i) {
    for (uint8_t j = c; j < c + s; ++j) {
      repeated |= 1U << grid[i][j];
    }
  }

  print_errors(repeated, sqr_index, tid, "regiao");
}

void print_errors(uint16_t count, uint8_t index, uint16_t tid, char *type) {
  // NOLINTNEXTLINE(hicpp-signed-bitwise)
  uint8_t errors = __builtin_popcount(~count & ((1 << SIZE) - 1));
  if (errors) {
    printf("Thread %d: erro na %s %d.\n", tid, type, index + 1);
    pthread_mutex_lock(&err_mutex);
    error_count += errors;
    pthread_mutex_unlock(&err_mutex);
  }
}

void *choose_task(void *tid) {
  for (uint8_t i = 0; i < length(tasks); ++i) {
    pthread_mutex_lock(&buf_mutex);
    if ((available_tasks >> i) & 1U) {
      available_tasks &= ~(1U << i);
      pthread_mutex_unlock(&buf_mutex);
      tasks[i](i % SIZE, (intptr_t)tid);
    } else {
      pthread_mutex_unlock(&buf_mutex);
    }
  }

  return NULL;
}
