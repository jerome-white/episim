#include <stdio.h>
#include <ross.h>

#include "model.h"

void ev_trace(struct message *m, tw_lp *lp, char *buffer, int *collect_flag) {
  p_printf(buffer, &m->people);
  sprintf(buffer, "%i,%s", m->event, buffer);

  return;
}

void model_stat(struct state *s, tw_lp *lp, char *buffer) {
  p_printf(buffer, &s->people);
  sprintf(buffer, "%s", buffer);

  return;
}
