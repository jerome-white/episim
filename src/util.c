#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <ross.h>

#include "model.h"

void lp_log_header(tw_lp *lp, const struct state *s) {
  tw_output(lp,
	    "timestamp,"
	    "event,"
	    "location,"
	    "susceptible,"
	    "infected,",
	    "recovered\n");

  return;
}

void lp_log(const char *etype,
	    tw_lp *lp,
	    const struct state *s,
	    const struct message *m) {
  tw_output(lp,
	    "%0.2f,%s,%llu,%0.2f,%0.2f,%0.2f\n",
	    tw_now(lp),
	    etype,
	    lp->gid,
	    s->people.susceptible,
	    s->people.infected,
	    s->people.recovered);

  return;
}
