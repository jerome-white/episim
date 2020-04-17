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

#define HUMAN_TRAVEL_SPEED 20
#define HUMAN_STAY_TIME    (HOUR * 8)

tw_stime __duration;
uint64_t __tiles;
char __config[FNAME_LENGTH];
char __log_dir[FNAME_LENGTH];

enum event_t {
  HUMAN_ARRIVAL_EVENT,
  HUMAN_DEPARTURE_EVENT,
  HUMAN_INTERACTION_EVENT,
};

struct transition {
  double mean;
  double deviation;
  double distance;
};

struct population {
  double susceptible;
  double infected;
  double recovered;
};

struct state {
  tw_lpid id;
  FILE *log;
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

struct population population_setup(const char *, struct state *, uint64_t);
struct population population_increase(const struct population *,
				      const struct population *);
struct population population_decrease(const struct population *,
				      const struct population *);
struct population population_normalize(const struct population *,
				       const struct population *);
struct population population_sample(struct population *);
bool population_empty(const struct population *);

void lp_log_header(tw_lp *, const struct state *);
void lp_log(const char *,
	    tw_lp *,
	    const struct state *,
	    const struct message *);
tw_lpid transition_select(tw_lp *,
			  const struct transition *,
			  tw_lpid,
			  unsigned int *);
int human_departure_events(struct state *, tw_lp *);

#endif // MODEL_H
