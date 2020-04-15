#ifndef MODEL_H
#define MODEL_H

#include <stdint.h>

#include <ross.h>

#define FNAME_LENGTH 32
#define PATH_SEPARATOR "/"

/*
 * Standard concepts of time. We assume a clock-tick is one minute
 */
#define MINUTE 1.0
#define HOUR   (MINUTE * 60.0)
#define DAY    (HOUR * 24.0)

#define HUMAN_TRAVEL_TIME (MINUTE * 40)
#define HUMAN_STAY_TIME   (HOUR * 8)

tw_stime __duration;
uint64_t __tiles;
char __config[FNAME_LENGTH];
char __log_dir[FNAME_LENGTH];

enum event {
  HUMAN_ARRIVAL_EVENT,
  HUMAN_DEPARTURE_EVENT,
  /* HUMAN_INTERACTION_EVENT, */
};

struct transition {
  double mean;
  double deviation;
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
  enum event etype;
  struct population people;
};

void init(struct state *, tw_lp *);
void forward_event_handler(struct state *, tw_bf *, struct message *, tw_lp *);
void reverse_event_handler(struct state *, tw_bf *, struct message *, tw_lp *);
void uninit(struct state *, tw_lp *);
tw_peid mapping(tw_lpid);

struct population population_setup(const char *, struct state *, uint64_t);
void population_increase(struct population *, const struct population *);
void population_decrease(struct population *, const struct population *);

#endif // MODEL_H
