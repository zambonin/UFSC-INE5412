#include <stdio.h>

#define SIZE  9
#define hamming_comp(x)  __builtin_popcount(~x & 0x3ff)

short error_count = 0;

void load_grid(unsigned short grid[][SIZE], char* file) {

	FILE* input = fopen(file, "r");

  if (input == NULL) {
    return;
  }

  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      fscanf(input, "%hu", &grid[i][j]);
    }
  }

  fclose(input);

}

void check_row(unsigned short grid[][SIZE], short row_index) {

  unsigned short repeated = 1;

  for (short i = 0; i < SIZE; ++i) {
    repeated |= 1 << grid[row_index][i];
  }

  error_count += hamming_comp(repeated);

}

void check_col(unsigned short grid[][SIZE], short col_index) {

  unsigned short repeated = 1;

  for (short i = 0; i < SIZE; ++i) {
    repeated |= 1 << grid[i][col_index];
  }

  error_count += hamming_comp(repeated);

}

void check_sqr(unsigned short grid[][SIZE], short sqr_index) {

  short r = sqr_index / 3 * 3, c = sqr_index % 3 * 3, repeated = 1;

  for (short i = r; i < r + 3; ++i) {
    for (short j = c; j < c + 3; ++j) {
      repeated |= 1 << grid[i][j];
    }
  }

  error_count += hamming_comp(repeated);

}

int main(int argc, char *argv[]) {

	if (argc != 2) {
    printf("Usage: %s input_grid\n", argv[0]);
		return 1;
	}

  unsigned short grid[SIZE][SIZE];
	load_grid(grid, argv[1]);

  printf("Quebra-cabecas fornecido:\n");
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      printf("%hu ", grid[i][j]);
    }
    printf("\n");
    check_row(grid, i);
    check_col(grid, i);
    check_sqr(grid, i);
  }

  printf("Erros encontrados: %d.\n", error_count);

	return 0;

}
