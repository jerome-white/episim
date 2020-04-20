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

  // population setup
  s->movement = (struct transition *)calloc(sizeof(struct transition),__tiles);
  if (s->movement == NULL) {
    tw_error(TW_LOC, "malloc error: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  s->people = population_setup(__config, s, __tiles);

  return;
}

void pre_run(struct state *s, tw_lp *lp) {
  int i;
  tw_stime ts;
  tw_event *event;
  struct message *msg;
  struct population travelers;

  memset((struct population *)&travelers, 0, sizeof(struct population));
  travelers.susceptible = 1;

  for (i = 0; i < s->people.susceptible; i++) {
    ts = tw_rand_exponential(lp->rng, HUMAN_STAY_TIME);
    event = tw_event_new(lp->gid, ts, lp);

    msg = (struct message *)tw_event_data(event);
    msg->event = HUMAN_DEPARTURE_EVENT;
    msg->rng_calls = 1;
    msg->people = travelers;

    tw_event_send(event);
  }

  return;
}

void forward_event_handler(struct state *s,
			   tw_bf *bf,
			   struct message *m,
			   tw_lp *lp) {
  unsigned int rng_calls;
  double distance, speed;
  tw_lpid lpid;
  tw_stime ts;
  tw_event *event;
  struct message *msg;

  memset((tw_bf *)bf, 0, sizeof(tw_bf));

  switch (m->event) {
  case HUMAN_ARRIVAL_EVENT:
    msg = (struct message *)tw_event_data(event);
    s->people = population_increase(&s->people, &m->people);

    ts = tw_rand_exponential(lp->rng, HUMAN_STAY_TIME);
    m->rng_calls = 1;
    event = tw_event_new(lp->gid, ts, lp);

    msg = (struct message *)tw_event_data(event);
    msg->event = HUMAN_DEPARTURE_EVENT;
    msg->people = m->people;

    tw_event_send(event);

    break;
  /* case HUMAN_INTERACTION_EVENT: */

  /*   break; */
  case HUMAN_DEPARTURE_EVENT:
    m->rng_calls = 0;
    lpid = transition_select(lp, s->movement, __tiles, &m->rng_calls);
    if (lpid < __tiles) {
      s->people = population_decrease(&s->people, &m->people);

      speed = tw_rand_exponential(lp->rng, HUMAN_TRAVEL_SPEED);
      distance = s->movement[lpid].distance;
      ts = tw_rand_exponential(lp->rng, distance / speed);
      m->rng_calls += 1;

      event = tw_event_new(lpid, ts, lp);

      msg = (struct message *)tw_event_data(event);
      msg->event = HUMAN_ARRIVAL_EVENT;
      msg->rng_calls = rng_calls;
      msg->people = m->people;

      tw_event_send(event);
    }
    break;
  default:
    tw_error(TW_LOC,
	     "[%d APP_ERROR]: Invalid method name: (%d)",
	     lp->id,
	     m->event);
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

  switch (m->event) {
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
	     m->event);
    exit(EXIT_FAILURE);
  }

  return;
}

void uninit(struct state *s, tw_lp *lp) {
  if (s->movement != NULL) {
    free(s->movement);
  }

  return;
}

tw_peid mapping(tw_lpid gid) {
  return (tw_peid)(gid / g_tw_nlp);
}

void ev_trace(struct message *m, tw_lp *lp, char *buffer, int *collect_flag) {
  sprintf(buffer,
	  "%i,%0.2f,%0.2f,%0.2f",
	  m->event,
	  m->people.susceptible,
	  m->people.infected,
	  m->people.recovered);

  return;
}
