#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "model.h"

#define BEFORE_POPULATION 4

void p_printf(char *buffer, const struct population *p) {
  sprintf(buffer,
	  "%lu,%lu,%lu",
	  p->health[SUSCEPTIBLE],
	  p->health[INFECTED],
	  p->health[RECOVERED]);

  return;
}

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
    p_totals = {0},
    p_counts = {0};
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

	p_totals.health[index] += llround(atof(word));
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
  person_t x, y;
  struct population p;

  for (i = 0; i < __HEALTH_COMPARTMENTS; i++) {
    x = lhs->health[i];
    y = rhs->health[i];
    p.health[i] = (x > y) ? x - y : 0;
  }

  return p;
}

struct population p_normalize(const struct population *lhs,
			      const struct population *rhs) {
  int i;
  person_t numer, denom;
  struct population p;

  for (i = 0; i < __HEALTH_COMPARTMENTS; i++) {
    numer = (long double)lhs->health[i];
    denom = (long double)rhs->health[i];
    p.health[i] = (denom) ? llround(numer / denom) : 0;
  }

  return p;
}

person_t p_total(const struct population *p) {
  int i;
  bool overflow;
  person_t person, people;

  for (i = 0, people = 0; i < __HEALTH_COMPARTMENTS; i++) {
    person = p->health[i];
    if (people > UINT64_MAX -  person) {
      fprintf(stderr, "Potential overflow detected in population accumlate");
      exit(EXIT_FAILURE);
    }
    people += person;
  }

  return people;
}

bool p_empty(const struct population *p) {
  return p_total(p) == 0;
}

struct population p_sample(tw_rng_stream *g,
			   const struct population *p,
			   unsigned int k) {
  int
    i,
    people,
    selection,
    compartment;
  struct population sample = {0};

  people = p_total(p);

  for (; k; k--) {
    selection = tw_rand_unif(g) * people;
    for (i = 0; i < __HEALTH_COMPARTMENTS; i++) {
      selection -= p->health[i];
      if (selection <= 0) {
        sample.health[i] += 1;
        break;
      }
    }
  }

  return sample;
}

struct population p_exposed(const struct population *p, uint16_t *rng_calls) {
  struct population exposed = {0};

  if (p->health[INFECTED]) {
    exposed.health[SUSCEPTIBLE] = p->health[SUSCEPTIBLE];
  }

  return exposed;
}

struct population p_person(enum health_t compartment) {
  struct population p = {0};

  p.health[compartment] = 1;

  return p;
}

struct population p_right_shift(const struct population *p) {
  int i, j;
  struct population shift;

  for (i = 0; i < __HEALTH_COMPARTMENTS; i++) {
    j = (i + 1) % __HEALTH_COMPARTMENTS;
    shift.health[j] = p->health[i];
  }

  return shift;
}

struct population p_left_shift(const struct population *p) {
  int i, j;
  struct population shift;

  for (i = 0; i < __HEALTH_COMPARTMENTS; i++) {
    j = (i - 1) % __HEALTH_COMPARTMENTS;
    shift.health[j] = p->health[i];
  }

  return shift;
}
