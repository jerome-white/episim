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
  struct observation data;

  data.size = 0;

  fp = fopen(argv[1], "r");
  do {
    fread(&line, sizeof(struct metadata), 1, fp);
    /*
    printf("%u,%u,%f,%f,%f,%i\n",
	   line.source_lp,
    	   line.destination_lp,
    	   line.virtual_send_time,
    	   line.virtual_recv_time,
    	   line.real_time,
    	   line.model_data_size);
    */
    if (line.model_data_size > data.size) {
      data.size = line.model_data_size;
      data.value = (char *)realloc((char *)data.value, data.size);
    }
    len = fread(data.value, 1, data.size, fp);
    //data.value[len] = '\0';
    printf("%s", data.value);
  } while (!feof(fp));
  fclose(fp);

  free(data.value);

  return EXIT_SUCCESS;
}
