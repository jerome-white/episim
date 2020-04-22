#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
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
  s->people = p_setup(__config, s, __tiles);

  return;
}

void pre_run(struct state *s, tw_lp *lp) {
  int i;
  tw_stime ts;
  tw_event *event;
  struct message *msg;
  struct population travelers = {0};

  travelers.health[SUSCEPTIBLE] = 1;

  for (i = 0; i < s->people.health[SUSCEPTIBLE]; i++) {
    ts = tw_rand_exponential(lp->rng, MOVEMENT_DWELL_TIME);
    event = tw_event_new(lp->gid, ts, lp);

    msg = (struct message *)tw_event_data(event);
    msg->event = MOVEMENT_DEPARTURE_EVENT;
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
  int i, j;
  unsigned int rng_calls;
  double distance, speed, survival;
  tw_lpid lpid;
  tw_stime ts;
  tw_event *event;
  struct message *msg;
  struct population people;

  memset((tw_bf *)bf, 0, sizeof(tw_bf));

  switch (m->event) {
  case MOVEMENT_ARRIVAL_EVENT:
    msg = (struct message *)tw_event_data(event);
    s->people = p_increase(&s->people, &m->people);

    ts = tw_rand_exponential(lp->rng, MOVEMENT_DWELL_TIME);
    m->rng_calls = 1;
    event = tw_event_new(lp->gid, ts, lp);

    msg = (struct message *)tw_event_data(event);
    msg->event = MOVEMENT_DEPARTURE_EVENT;
    msg->people = m->people;

    tw_event_send(event);

    break;
  case MOVEMENT_DEPARTURE_EVENT:
    m->rng_calls = 0;
    lpid = transition_select(lp, s->movement, __tiles, &m->rng_calls);
    if (lpid < __tiles) {
      bf->c0 = 1;

      m->people = p_sample(lp, &s->people, 1); // Okay to write to m?
      assert(!p_empty(&m->people));
      m->rng_calls += 1;
      s->people = p_decrease(&s->people, &people);

      distance = s->movement[lpid].distance;
      speed = tw_rand_exponential(lp->rng, MOVEMENT_TRAVEL_SPEED);
      ts = tw_rand_exponential(lp->rng, distance / speed);
      m->rng_calls += 2;

      event = tw_event_new(lpid, ts, lp);

      msg = (struct message *)tw_event_data(event);
      msg->event = MOVEMENT_ARRIVAL_EVENT;
      msg->rng_calls = m->rng_calls;
      msg->people = people;

      tw_event_send(event);
    }
    break;
  case MOVEMENT_INTERACTION_EVENT:
    ts = tw_rand_exponential(lp->rng, MINUTE);
    event = tw_event_new(lpid, ts, lp);

    msg = (struct message *)tw_event_data(event);
    msg->event = HUMAN_INTERACTION_EVENT;
    msg->people = p_sample(lp, &s->people, 2);
    msg->rng_calls = 3; // 1 from tw_rand_exponential, 2 from p_sample

    tw_event_send(event);

    // Schedule another interaction
    speed = 1 / p_total(&s->people);
    ts = tw_rand_exponential(lp->rng, speed * HOUR);
    event = tw_event_new(lpid, ts, lp);

    msg = (struct message *)tw_event_data(event);
    msg->event = MOVEMENT_INTERACTION_EVENT;
    msg->rng_calls = 1;

    tw_event_send(event);
    break;
  case HUMAN_INTERACTION_EVENT:
    m->rng_calls = 0;
    people = p_exposed(lp, &people, &m->rng_calls);

    for (i = 0; i < __HEALTH_COMPARTMENTS; i++) {
      for (j = 0; j < people.health[i]; j++) {
	ts = tw_rand_exponential(lp->rng, HOUR);
	event = tw_event_new(lpid, ts, lp);

	msg = (struct message *)tw_event_data(event);
	msg->event = HUMAN_INFECTION_EVENT;;
	msg->rng_calls = m->rng_calls + 1; // p_exposed + rand_exponential
	msg->people = p_person(i);

	tw_event_send(event);
      }
    }
    break;
  case HUMAN_INFECTION_EVENT:
    s->people = p_decrease(&s->people, &m->people);
    people = p_right_shift(&m->people);
    s->people = p_increase(&s->people, &people);

    ts = tw_rand_exponential(lp->rng, 10 * DAY);
    event = tw_event_new(lpid, ts, lp);

    msg = (struct message *)tw_event_data(event);
    msg->event = HUMAN_RECOVERY_EVENT;
    msg->rng_calls = 1;
    msg->people = people;

    tw_event_send(event);
    break;
  case HUMAN_RECOVERY_EVENT:
    s->people = p_decrease(&s->people, &m->people);
    survival = tw_rand_binomial(lp->rng, 1, 1 - MORTALITY_RATE);
    if (survival) {
      bf->c0 = 1;

      people = p_right_shift(&m->people);
      s->people = p_increase(&s->people, &people);

      ts = tw_rand_exponential(lp->rng, 5 * DAY);
      event = tw_event_new(lpid, ts, lp);
      msg = (struct message *)tw_event_data(event);

      msg->event = HUMAN_SUSCEPTIBLE_EVENT;
      msg->rng_calls = 1;
      msg->people = people;

      tw_event_send(event);
    }
    break;
  case HUMAN_SUSCEPTIBLE_EVENT:
    s->people = p_decrease(&s->people, &m->people);
    people = p_right_shift(&m->people);
    s->people = p_increase(&s->people, &people);
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

void reverse_event_handler(struct state *s,
			   tw_bf *bf,
			   struct message *m,
			   tw_lp *lp) {
  int i;
  struct population people;

  for (i = 0; i < m->rng_calls; i++) {
    tw_rand_reverse_unif(lp->rng);
  }

  switch (m->event) {
  case MOVEMENT_ARRIVAL_EVENT:
    p_decrease(&s->people, &m->people);
    break;
  case MOVEMENT_DEPARTURE_EVENT:
    if (bf->c0) {
      p_increase(&s->people, &m->people);
    }
    break;
  case MOVEMENT_INTERACTION_EVENT:
    break;
  case HUMAN_INTERACTION_EVENT:
    break;
  case HUMAN_RECOVERY_EVENT:
    s->people = p_increase(&s->people, &m->people);
    if (bf->c0) {
      people = p_right_shift(&m->people);
      s->people = p_decrease(&s->people, &people);
    }
    break;
  case HUMAN_INFECTION_EVENT:
  case HUMAN_SUSCEPTIBLE_EVENT:
    s->people = p_increase(&s->people, &m->people);
    people = p_right_shift(&m->people);
    s->people = p_decrease(&s->people, &people);
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
