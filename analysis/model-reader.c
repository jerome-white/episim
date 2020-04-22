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
  size_t len;
  struct metadata line;
  struct observation data = {0};

  fp = fopen(argv[1], "r");
  if (fp == NULL) {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  /*
  printf("source,"
	 "destination,"
	 "sent,"
	 "event,"
	 "susceptible,"
	 "exposed,"
	 "infected,"
	 "recovered\n");
  */

  while (!feof(fp)) {
    fread(&line, sizeof(struct metadata), 1, fp);
    printf("%i,%i,%f,%f,%u,%u,%u,%f,%i,%i",
	   line.sample_type,
	   line.sample_size,
	   line.virtual_time,
	   line.real_time,
	   line.pe_id,
	   line.kp_id,
	   line.lp_id,
	   line.gvt,
	   line.stats_type,
	   line.model_size);
    if (line.model_size > data.size) {
      data.size = line.model_size;
      data.value = (char *)realloc((char *)data.value, 1 * sizeof(char));
    }
    len = fread(data.value, sizeof(char), data.size, fp);
    //data.value[len] = '\0';
    printf(",%s\n", data.value);
  }
  fclose(fp);

  free(data.value);

  return EXIT_SUCCESS;
}
