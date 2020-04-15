#include <stdlib.h>
#include <stdio.h>
#include <ross.h>

#include "model.h"

void init(struct state *s, tw_lp *lp) {
  int i;
  char fname[FNAME_LENGTH];
  int64_t people;

  memset((struct state *)s, 0, sizeof(struct state));
  s->id = lp->gid;

  // output log
  i = snprintf(fname, FNAME_LENGTH, "%s/tile-%05lu.log", __log_dir, lp->gid);
  if (i >= FNAME_LENGTH) {
    tw_error(TW_LOC, "Unable to create LP log file", strerror(errno));
    exit(EXIT_FAILURE);
  }
  s->log = fopen(fname, "w");
  if (s->log == NULL) {
    tw_error(TW_LOC, "fopen error (%s): logging disabled", strerror(errno));
    exit(EXIT_FAILURE);
  }

  // population setup
  s->movement = (struct transition *)calloc(sizeof(struct transition),__tiles);
  if (s->movement == NULL) {
    tw_error(TW_LOC, "malloc error: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  s->people = population_setup(__config, s, __tiles);

  lp_log("INIT", lp, s, NULL);

  human_departure_events(s, lp);
  return;
}

void forward_event_handler(struct state *s,
			   tw_bf *bf,
			   struct message *m,
			   tw_lp *lp) {
  int i;
  unsigned int norm_calls;
  long double people;
  tw_stime ts;
  tw_event *event;
  struct message *msg;
  struct population travelers;
  struct transition *trans;

  memset((tw_bf *)bf, 0, sizeof(tw_bf));

  switch (m->etype) {
  case HUMAN_ARRIVAL_EVENT:
    msg = (struct message *)tw_event_data(event);
    s->people = population_increase(&s->people, &m->people);

    ts = tw_rand_exponential(lp->rng, HUMAN_STAY_TIME);
    event = tw_event_new(lp->gid, ts, lp);

    msg = (struct message *)tw_event_data(event);
    msg->etype = HUMAN_DEPARTURE_EVENT;
    msg->rng_calls = 1;
    msg->people = m->people;

    tw_event_send(event);

    lp_log("HUMAN_ARRIVAL_EVENT", lp, s, m);
    break;
  case HUMAN_DEPARTURE_EVENT:
    human_departure_events(s, lp);
    break;
  default:
    tw_error(TW_LOC,
	     "[%d APP_ERROR]: Invalid method name: (%d)",
	     lp->id,
	     m->etype);
    exit(EXIT_FAILURE);
    break;
  }

  return;
}

void reverse_event_handler(struct state *s,
			   tw_bf *bf,
			   struct message *m,
			   tw_lp *lp) {
  int i;

  for (i = 0; i < m->rng_calls; i++) {
    tw_rand_reverse_unif(lp->rng);
  }

  switch (m->etype) {
  case HUMAN_ARRIVAL_EVENT:
    population_decrease(&s->people, &m->people);
    break;
  case HUMAN_DEPARTURE_EVENT:
    population_increase(&s->people, &m->people);
    break;
  default:
    tw_error(TW_LOC,
	     "[%d APP_ERROR]: Invalid method name: (%d)",
	     lp->id,
	     m->etype);
    exit(EXIT_FAILURE);
  }

  return;
}

void uninit(struct state *s, tw_lp *lp) {
  if (s->movement != NULL) {
    free(s->movement);
  }
  if (s->log != NULL) {
    fclose(s->log);
  }

  return;
}

tw_peid mapping(tw_lpid gid) {
  return (tw_peid)(gid / g_tw_nlp);
}
