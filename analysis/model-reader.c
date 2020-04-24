#include <stdio.h>
#include <stdlib.h>
#include <ross.h>

struct metadata {
  int sample_type;
  int sample_size;
  double virtual_time;
  double real_time;
  unsigned int pe_id;
  unsigned int kp_id;
  unsigned int lp_id;
  float gvt;
  int stats_type;
  int model_size;
};

struct observation {
  int size;
  char *value;
};

int main(int argc, char **argv, char **env) {
  FILE *fp;
  struct metadata line;
  struct observation data = {0};

  fp = fopen(argv[1], "r");
  if (fp == NULL) {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  printf("sample_type,"
         "sample_size,"
         "virtual_time,"
         "real_time,"
         "pe,"
         "kp,"
         "lp,"
         "gvt,"
         "stats_type,"
         "susceptible,"
         "infected,"
         "recovered\n");

  while (!feof(fp)) {
    fread(&line, sizeof(struct metadata), 1, fp);
    printf("%i,%i,%f,%f,%u,%u,%u,%f,%i",
           line.sample_type,
           line.sample_size,
           line.virtual_time,
           line.real_time,
           line.pe_id,
           line.kp_id,
           line.lp_id,
           line.gvt,
           line.stats_type);

    if (line.model_size > data.size) {
      data.size = line.model_size;
      data.value = (char *)realloc((char *)data.value, data.size);
      if (data.value == NULL) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
    }
    fread(data.value, sizeof(char), data.size, fp);
    printf(",%s\n", data.value);
  }
  fclose(fp);

  free(data.value);

  return EXIT_SUCCESS;
}
