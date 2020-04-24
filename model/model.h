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
#define SECOND 1000
#define MINUTE (SECOND * 60)
#define HOUR   (MINUTE * 60)
#define DAY    (HOUR   * 24)

#define MOVEMENT_TRAVEL_SPEED 20
#define MOVEMENT_DWELL_TIME   (HOUR * 8)

#define MORTALITY_RATE 0.034
#define RECOVERY_TIME (5 * DAY)
#define INFECTION_DURATION (8 * DAY)

tw_stime __duration;
uint64_t __tiles;
char __config[FNAME_LENGTH];

enum event_t {
  //
  MOVEMENT_ARRIVAL_EVENT,
  MOVEMENT_DEPARTURE_EVENT,
  MOVEMENT_INTERACTION_EVENT,
  //
  HUMAN_INTERACTION_EVENT,
  HUMAN_INFECTION_EVENT,
  HUMAN_RECOVERY_EVENT,
  HUMAN_SUSCEPTIBLE_EVENT,
};

enum health_t {
  SUSCEPTIBLE,
  INFECTED,
  RECOVERED,
  __HEALTH_COMPARTMENTS, // should be last!
};

typedef uint64_t person_t;

struct population {
  person_t health[__HEALTH_COMPARTMENTS];
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
  uint16_t rng_calls;
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
void model_stat(struct state *, tw_lp *, char *);

void p_printf(char *, const struct population *);
bool p_empty(const struct population *);
uint64_t p_total(const struct population *);
struct population p_setup(const char *, struct state *, uint64_t);
struct population p_increase(const struct population *,
			     const struct population *);
struct population p_decrease(const struct population *,
			     const struct population *);
struct population p_normalize(const struct population *,
			      const struct population *);
struct population p_sample(tw_rng_stream *,
			   const struct population *,
			   unsigned int);
struct population p_exposed(const struct population *, uint16_t *);
struct population p_person(enum health_t);
struct population p_right_shift(const struct population *);
struct population p_left_shift(const struct population *);

tw_lpid transition_select(tw_rng_stream *,
			  const struct transition *,
			  tw_lpid,
			  uint16_t *);

#endif // MODEL_H
