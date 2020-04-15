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
  uint8_t i;
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
  struct population
    p_totals,
    p_counts;
  struct transition *trans;
  
  fp = fopen(path, "r");
  if (fp == NULL) {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  memset((struct population *)&p_totals, 0, sizeof(struct population));
  memset((struct population *)&p_counts, 0, sizeof(struct population));
  
  for (lineno = 0; ; lineno++) {
    read = getline(&line, &len, fp);
    if (read < 0) {
      break;
    }
    if (lineno == 0) { // assume there's a header
      continue;
    }

    word = strtok(line, sep);
    for (wordno = 0, trans = NULL; word != NULL; wordno++) {
      switch (wordno) {
      case 0: // source
	id = atoll(word);
	if (id > nsize) {
	  fprintf(stderr, "Proposed ID exceeds limit: %lu %lu\n", id, nsize);
	  exit(EXIT_FAILURE);
	}
	if (id != s->id) {
	  word = NULL;
	  continue;
	}
	break;
      case 1: // destination
	trans = &s->movement[atoi(word)];
	break;
      case 2: // movement mean
	trans->mean = atof(word);
	break;
      case 3: // movement deviation
	trans->deviation = atof(word);
	break;
      case 4: // population
	p_totals.susceptible += atof(word);
	p_counts.susceptible += 1;
	break;
      case 5: // distance
	trans->distance = atof(word);
	break;
      default:
	break;
      }
      word = strtok(NULL, sep);
    }
  }
  
  fclose(fp);

  return population_normalize(&p_totals, &p_counts);
}

struct population population_increase(const struct population *lhs,
				      const struct population *rhs) {
  struct population p;

  p.susceptible = lhs->susceptible + rhs->susceptible;
  p.infected    = lhs->infected + rhs->infected;
  p.recovered   = lhs->recovered + rhs->recovered;

  return p;
}

struct population population_decrease(const struct population *lhs,
				      const struct population *rhs) {
  struct population p;

  p.susceptible = lhs->susceptible - rhs->susceptible;
  p.infected    = lhs->infected - rhs->infected;
  p.recovered   = lhs->recovered - rhs->recovered;

  return p;
}

struct population population_normalize(const struct population *lhs,
				       const struct population *rhs) {
  struct population p;
  memset((struct population *)&p, 0, sizeof(struct population));

  if (rhs->susceptible != 0) {
    p.susceptible = lhs->susceptible / rhs->susceptible;
  }
  if (rhs->infected != 0) {
    p.infected = lhs->infected / rhs->infected;
  }
  if (rhs->recovered != 0) {
    p.recovered = lhs->recovered / rhs->recovered;
  }

  return p;
}
