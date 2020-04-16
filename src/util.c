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

int human_departure_events(struct state *s, tw_lp *lp) {
  unsigned int norm_calls, sent;
  double speed, people;
  tw_lpid i;
  tw_stime ts;
  tw_event *event;
  struct message *msg;
  struct population travelers;
  struct transition *movement;

  memset((struct population *)&travelers, 0, sizeof(struct population));

  for (i = 0, sent = 0; i < __tiles; i++) {
    norm_calls = 0;
    movement = &s->movement[i];
    people = tw_rand_normal_sd(lp->rng,
			       movement->mean,
			       movement->deviation,
			       &norm_calls);
    travelers.susceptible = ROSS_MIN(s->people.susceptible, people);
    if (travelers.susceptible > 0) {
      s->people = population_decrease(&s->people, &travelers);

      travel_time = movement->distance / HUMAN_TRAVEL_SPEED;
      ts = tw_rand_exponential(lp->rng, travel_time);
      event = tw_event_new(i, ts, lp);

      msg = (struct message *)tw_event_data(event);
      msg->etype = HUMAN_ARRIVAL_EVENT;
      msg->rng_calls = norm_calls + 1;
      msg->people = travelers;

      tw_event_send(event);
      sent += 1;
    }
  }

  return sent;
}
