#include <stdio.h>
#include <ross.h>

#include "model.h"

void ev_trace(struct message *m, tw_lp *lp, char *buffer, int *collect_flag) {
  sprintf(buffer,
	  "%i,%0.2f,%0.2f,%0.2f",
	  m->event,
	  m->people.health[SUSCEPTIBLE],
	  m->people.health[INFECTED],
	  m->people.health[RECOVERED]);

  return;
}

void model_stat(struct state *s, tw_lp *lp, char *buffer) {
  sprintf(buffer,
	  "%0.2f,%0.2f,%0.2f",
	  s->people.health[SUSCEPTIBLE],
	  s->people.health[INFECTED],
	  s->people.health[RECOVERED]);

  return;
}
