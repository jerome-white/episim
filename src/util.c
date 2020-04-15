#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "model.h"

struct population population_setup(const char *path,
				   struct state *s,
				   uint64_t nsize) {
  uint8_t
    i,
    index;
  uint16_t
    lineno,
    wordno;
  uint64_t id;
  double
    people_total,
    people_n;
  size_t len = 0;
  ssize_t read;  
  char
    *line = NULL,
    *sep = ",",
    *word;
  FILE *fp;
  struct population people;
  
  fp = fopen(path, "r");
  if (fp == NULL) {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  memset((struct population *)&people, 0, sizeof(struct population));
  
  for (lineno = 0; ; lineno++) {
    read = getline(&line, &len, fp);
    if (read < 0) {
      break;
    }
    if (!lineno) { // assume there's a header
      continue;
    }

    index = -1;

    word = strtok(line, sep);
    for (wordno = 0; word; wordno++) {
      switch (wordno) {
      case 0: // source
	id = atoll(word);
	if (id > nsize) {
	  fprintf(stderr, "Proposed ID exceeds limit\n");
	  exit(EXIT_FAILURE);
	}
	if (id != s->id) {
	  word = NULL;
	  continue;
	}
	break;
      case 1: // target
	index = atoi(word);
	break;
      case 2: // movement mean
	s->movement[index].mean = atof(word);
	break;
      case 3: // movement mean
	s->movement[index].deviation = atof(word);
	break;
      case 4: // population
	people_total += atof(word);
	people_n += 1;
	break;
      default:
	break;
      }
      
      word = strtok(NULL, sep);
    }
  }
  
  fclose(fp);

  people.susceptible = people_total / people_n;

  return people;
}

void population_increase(struct population *target,
			 const struct population *source) {
  target->susceptible += source->susceptible;
  target->infected    += source->infected;
  target->recovered   += source->recovered;
}

void population_decrease(struct population *target,
			 const struct population *source) {
  target->susceptible -= source->susceptible;
  target->infected    -= source->infected;
  target->recovered   -= source->recovered;
}
