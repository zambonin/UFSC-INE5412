#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define SIZE        9
#define length(x)   (sizeof(x) / sizeof((x)[0]))

void load_grid(uint8_t /*grid*/[][SIZE], char* /*file*/),
     print_errors(uint16_t /*count*/, uint8_t /*index*/, char* /*type*/),
     check_row(uint8_t /*row_index*/), check_col(uint8_t /*col_index*/),
     check_sqr(uint8_t /*sqr_index*/);
void* choose_task(void* /*len*/);

static uint8_t error_count = 0, grid[SIZE][SIZE];
static pthread_t *threads;
static pthread_mutex_t buf_mutex, err_mutex;

typedef void (*func_ptr_t)(uint8_t);
static const func_ptr_t tasks[] = { [0 ... SIZE - 1] = check_row,
  [SIZE ... SIZE * 2 - 1] = check_col,
  [SIZE * 2 ... SIZE * 3 - 1] = check_sqr };
static uint32_t available_tasks = (1 << length(tasks)) - 1;

int32_t main(int32_t argc, char **argv) {
  if (argc != 3) {
    printf("Usage: %s input_grid nthreads\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  load_grid(grid, argv[1]);
  uint64_t nthreads = strtoul(argv[2], NULL, 10);
  threads = calloc(nthreads, sizeof(pthread_t));

  pthread_mutex_init(&buf_mutex, NULL);
  pthread_mutex_init(&err_mutex, NULL);

  for (uint8_t i = 0; i < nthreads; ++i) {
    pthread_create(&threads[i], NULL, choose_task, (void*) length(tasks));
  }

  for (uint8_t i = 0; i < nthreads; ++i) {
    pthread_join(threads[i], NULL);
  }

  pthread_mutex_destroy(&err_mutex);
  pthread_mutex_destroy(&buf_mutex);

  printf("Erros encontrados: %d.\n", error_count);
  free(threads);

  exit(EXIT_SUCCESS);
}

void load_grid(uint8_t m[][SIZE], char* file) {
  FILE* input = fopen(file, "r");
  if (input == NULL) {
    exit(EXIT_FAILURE);
  }

  printf("Quebra-cabecas fornecido:\n");
  for (int i = 0; i < SIZE; ++i) {
    for (int j = 0; j < SIZE; ++j) {
      fscanf(input, "%hhu ", &m[i][j]);
      printf("%hhu ", m[i][j]);
    }
    printf("\n");
  }

  fclose(input);
}

void check_row(uint8_t row_index) {
  uint16_t repeated = 0;
  for (uint8_t i = 0; i < length(grid[0]); ++i) {
    repeated |= 1 << (grid[row_index][i] - 1);
  }

  print_errors(repeated, row_index, "linha");
}

void check_col(uint8_t col_index) {
  uint16_t repeated = 0;
  for (uint8_t i = 0; i < length(grid[0]); ++i) {
    repeated |= 1 << (grid[i][col_index] - 1);
  }

  print_errors(repeated, col_index, "coluna");
}

void check_sqr(uint8_t sqr_index) {
  uint8_t r = sqr_index / 3 * 3, c = sqr_index % 3 * 3;
  uint16_t repeated = 0;
  for (uint8_t i = r; i < r + 3; ++i) {
    for (uint8_t j = c; j < c + 3; ++j) {
      repeated |= 1 << (grid[i][j] - 1);
    }
  }

  print_errors(repeated, sqr_index, "regiao");
}

void print_errors(uint16_t count, uint8_t index, char* type) {
  int32_t errors = __builtin_popcount(~count & ((1 << SIZE) - 1));
  if (errors) {
    printf("Thread %lu: erro na %s %d.\n", pthread_self(), type, index + 1);
    pthread_mutex_lock(&err_mutex);
    error_count += errors;
    pthread_mutex_unlock(&err_mutex);
  }
}

void* choose_task(void* len) {
  for (uint8_t i = 0; i < (intptr_t) len; ++i) {
    pthread_mutex_lock(&buf_mutex);
    if ((available_tasks >> i) & 1) {
      available_tasks &= ~(1 << i);
      pthread_mutex_unlock(&buf_mutex);
      tasks[i](i % SIZE);
    } else {
      pthread_mutex_unlock(&buf_mutex);
    }
  }

  return NULL;
}
