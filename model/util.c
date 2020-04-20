#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <ross.h>

#include "model.h"

tw_lpid transition_select(tw_lp *lp,
			  const struct transition *tr,
			  tw_lpid limit,
			  unsigned int *rng_calls) {
  tw_lpid i;
  int remaining;
  unsigned int weight;

  weight = 0;
  for (i = 0; i < limit; i++) {
    weight += tr[i].mean;
  }

  remaining = tw_rand_unif(lp->rng) * weight;
  *rng_calls += 1;

  for (i = 0; i < limit; i++) {
    remaining -= weight;
    if (remaining < 0) {
	return i;
      }
  }

  return limit;
}
