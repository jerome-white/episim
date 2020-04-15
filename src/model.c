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
  i = snprintf(fname, FNAME_LENGTH, "%s/tile-lp%lu.log", __log_dir, lp->gid);
  if (i >= FNAME_LENGTH) {
    tw_error(TW_LOC, "Unable to create LP log file", strerror(errno));
    exit(EXIT_FAILURE);
  }
  s->log = fopen(fname, "w");
  if (s->log == NULL) {
    tw_error(TW_LOC, "fopen error (%s): logging disabled", strerror(errno));
    exit(EXIT_FAILURE);
  }
  fprintf(s->log,
	  "timestamp,"
	  "event,"
	  "source,"
	  "destination,"
	  "value\n");

  // population setup
  s->movement = (struct transition *)calloc(sizeof(struct transition),__tiles);
  if (s->movement == NULL) {
    tw_error(TW_LOC, "malloc error: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  s->people = population_setup(__config, s, __tiles);

  return;
}

void forward_event_handler(struct state *s,
			   tw_bf *bf,
			   struct message *m,
			   tw_lp *lp) {
  int i;
  unsigned int calls;
  long double people;
  tw_stime ts;
  tw_event *event;
  struct transition trans;
  struct population travelers;
  struct message *msg;

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
    break;
  case HUMAN_DEPARTURE_EVENT:
    memset((struct population *)&travelers, 0, sizeof(struct population));

    for (i = 0; i < __tiles; i++) {
      calls = 0;
      trans = s->movement[i];
      people = tw_rand_normal_sd(lp->rng, trans.mean, trans.deviation, &calls);
      travelers.susceptible = ROSS_MIN(s->people.susceptible, people);
      if (travelers.susceptible > 0) {
	s->people = population_decrease(&s->people, &travelers);

	ts = tw_rand_exponential(lp->rng, HUMAN_TRAVEL_TIME);
	event = tw_event_new(i, ts, lp);

	msg = (struct message *)tw_event_data(event);
	msg->etype = HUMAN_ARRIVAL_EVENT;
	msg->rng_calls = calls + 1;
	msg->people = travelers;

	tw_event_send(event);
      }
    }

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
