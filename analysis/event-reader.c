#include <stdio.h>
#include <ross.h>

struct metadata {
  unsigned int source_lp;
  unsigned int destination_lp;
  float virtual_send_time;
  float virtual_recv_time;
  float real_time; // when this event data was collected
  unsigned int model_data_size;
};

struct observation {
  unsigned int size;
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

  printf("source,"
	 "destination,"
	 "sent,"
	 "event,"
	 "susceptible,"
	 "exposed,"
	 "infected,"
	 "recovered\n");

  while (!feof(fp)) {
    fread(&line, sizeof(struct metadata), 1, fp);
    printf("%u,%u,%f",
           line.source_lp,
           line.destination_lp,
           line.virtual_send_time);
    if (line.model_data_size > data.size) {
      data.size = line.model_data_size;
      data.value = (char *)realloc((char *)data.value, data.size);
    }
    len = fread(data.value, sizeof(char), data.size, fp);
    //data.value[len] = '\0';
    printf(",%s\n", data.value);
  }
  fclose(fp);

  free(data.value);

  return EXIT_SUCCESS;
}
