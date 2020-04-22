#ifndef MODEL_H
#define MODEL_H

#include <stdint.h>
#include <stdbool.h>
#include <ross.h>

#define FNAME_LENGTH 64
#define PATH_SEPARATOR "/"

/*
 * Standard concepts of time. We assume a clock-tick is one minute
 */
#define MINUTE 1.0
#define HOUR   (MINUTE * 60.0)
#define DAY    (HOUR * 24.0)

#define MOVEMENT_TRAVEL_SPEED 20
#define MOVEMENT_DWELL_TIME   (HOUR * 8)

#define MORTALITY_RATE 0.034

tw_stime __duration;
uint64_t __tiles;
char __config[FNAME_LENGTH];

enum event_t {
  MOVEMENT_ARRIVAL_EVENT,
  MOVEMENT_DEPARTURE_EVENT,
  MOVEMENT_INTERACTION_EVENT,
  HUMAN_INTERACTION_EVENT,
  HUMAN_INFECTION_EVENT,
  HUMAN_RECOVERY_EVENT,
  HUMAN_SUSCEPTIBLE_EVENT,
};

enum health_t {
  SUSCEPTIBLE,
  INFECTED,
  RECOVERED,
  __HEALTH_COMPARTMENTS,
};

struct population {
  double health[__HEALTH_COMPARTMENTS];
};

struct transition {
  double mean;
  double deviation;
  double distance;
};

struct state {
  tw_lpid id;
  struct population people;
  struct transition *movement;
};

struct message {
  long rng_calls;
  enum event_t event;
  struct population people;
};

void init(struct state *, tw_lp *);
void pre_run(struct state *, tw_lp *);
void forward_event_handler(struct state *, tw_bf *, struct message *, tw_lp *);
void reverse_event_handler(struct state *, tw_bf *, struct message *, tw_lp *);
void uninit(struct state *, tw_lp *);
tw_peid mapping(tw_lpid);

void ev_trace(struct message *, tw_lp *, char *, int *);

bool p_empty(const struct population *);
unsigned int p_total(const struct population *);
struct population p_setup(const char *, struct state *, uint64_t);
struct population p_increase(const struct population *,
			     const struct population *);
struct population p_decrease(const struct population *,
			     const struct population *);
struct population p_normalize(const struct population *,
			      const struct population *);
struct population p_sample(tw_lp *, const struct population *, unsigned int);
struct population p_exposed(tw_lp *, const struct population *, long int *);
struct population p_person(enum health_t);
struct population p_right_shift(const struct population *);
struct population p_left_shift(const struct population *);

tw_lpid transition_select(tw_lp *,
			  const struct transition *,
			  tw_lpid,
			  long int *);
int human_departure_events(struct state *, tw_lp *);

#endif // MODEL_H
