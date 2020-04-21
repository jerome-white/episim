#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "model.h"

#define BEFORE_POPULATION 4

struct population p_setup(const char *path, struct state *s, uint64_t nsize) {
  uint8_t i;
  uint16_t
    lineno,
    wordno,
    index;
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

  /*
   *
   */
  fp = fopen(path, "r");
  if (fp == NULL) {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  /*
   *
   */
  memset((struct population *)&p_totals, 0, sizeof(struct population));
  memset((struct population *)&p_counts, 0, sizeof(struct population));
  
  /*
   *
   */
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
	  fprintf(stderr, "Proposed ID exceeds limit: %llu %llu\n", id, nsize);
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
      case 2: // distance
	trans->distance = atof(word);
	break;
      case 3: // movement mean
	trans->mean = atof(word);
	break;
      case BEFORE_POPULATION: // movement deviation
	trans->deviation = atof(word);
	break;
      default:
	index = wordno - (BEFORE_POPULATION + 1);
	assert(0 <= index);
	assert(index < __HEALTH_COMPARTMENTS);

	p_totals.health[index] += atof(word);
	p_counts.health[index] += 1;
	break;
      }
      word = strtok(NULL, sep);
    }
  }
  
  /*
   *
   */
  fclose(fp);

  return p_normalize(&p_totals, &p_counts);
}

struct population p_increase(const struct population *lhs,
			     const struct population *rhs) {
  int i;
  struct population p;

  for (i = 0; i < __HEALTH_COMPARTMENTS; i++) {
    p.health[i] = lhs->health[i] + rhs->health[i];
  }

  return p;
}

struct population p_decrease(const struct population *lhs,
			     const struct population *rhs) {
  int i;
  struct population p;

  for (i = 0; i < __HEALTH_COMPARTMENTS; i++) {
    p.health[i] = lhs->health[i] - rhs->health[i];
  }

  return p;
}

struct population p_normalize(const struct population *lhs,
			      const struct population *rhs) {
  int i;
  double denom;
  struct population p;

  for (i = 0; i < __HEALTH_COMPARTMENTS; i++) {
    denom = rhs->health[i];
    p.health[i] = (denom == 0) ? 0 : lhs->health[i] / denom;
  }

  return p;
}

struct population p_sample(tw_lp *lp,
			   const struct population *p,
			   long int *rng_calls) {
  int i, elegible;
  int remaining;
  struct population sample;

  for (i = 0, elegible = 0; i < __HEALTH_COMPARTMENTS; i++) {
    if (i != INFECTED) {
      elegible += p->health[i];
    }
  }

  remaining = tw_rand_unif(lp->rng) * elegible;
  *rng_calls += 1;

  memset((struct population *)&sample, 0, sizeof(struct population));
  for (i = 0; i < __HEALTH_COMPARTMENTS; i++) {
    remaining -= elegible;
    if (remaining < 0) {
      sample.health[i] = 1;
      break;
    }
  }

  return sample;
}

bool p_empty(const struct population *p) {
  int i;

  for (i = 0; i < __HEALTH_COMPARTMENTS; i++) {
    if (p->health[i] > 0) {
      return false;
    }
  }

  return true;
}

bool p_infectious(const struct population *p) {
  return p->health[SUSCEPTIBLE] && (p->health[EXPOSED] || p->health[INFECTED]);
}
