#include <stdio.h>
#include <stdlib.h>

void load_grid(unsigned short grid[9][9], char *file) {

	FILE* input = fopen(file, "r");

  if (input == NULL) {
    exit(1);
  }

  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      fscanf(input, "%hu", &grid[i][j]);
    }
  }

  fclose(input);

}

int main(int argc, char *argv[]) {

	if (argc != 2) {
    printf("Usage: %s input_grid\n", argv[0]);
		exit(1);
	}

  unsigned short grid[9][9];
	load_grid(grid, argv[1]);

  printf("Quebra-cabecas fornecido:\n");
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      printf("%hu ", grid[i][j]);
    }
    printf("\n");
  }

	return EXIT_SUCCESS;

}
