#include <stdlib.h>
#include <ross.h>

#include "model.h"

tw_lptype handlers[] = {
  {
    (init_f) init,
    (pre_run_f) pre_run,
    (event_f) forward_event_handler,
    (revent_f) reverse_event_handler,
    (commit_f) NULL,
    (final_f) uninit,
    (map_f) mapping,
    sizeof(struct state),
  },
  {
    0,
  },
};

st_model_types tracers[] = {
  {
    (ev_trace_f) ev_trace,
    sizeof(struct message),
    (model_stat_f) NULL,
    0,
    (sample_event_f) NULL,
    (sample_revent_f) NULL,
    0,
  },
  {
    0,
  },
};


const tw_optdef options[] = {
  TWOPT_GROUP("Compartmental infectious disease modelling"),

  TWOPT_UINT("tiles", __tiles, "Number of homes"),
  TWOPT_CHAR("config", __config, "Configuration file"),
  TWOPT_DOUBLE("duration", __duration, "Duration of simlulation (days)"),

  TWOPT_END()
};

int main(int argc, char **argv, char **env) {
  int i;
  uint64_t lps_per_pe;

  tw_opt_add(options);
  tw_init(&argc, &argv);

  g_tw_ts_end = __duration * DAY;
  lps_per_pe = __tiles / tw_nnodes();
  tw_define_lps(lps_per_pe, sizeof(struct message));
  for (i = 0; i < g_tw_nlp; i++) {
    tw_lp_settype(i, &handlers[0]);
    st_model_settype(i, &tracers[0]);
  }

  tw_run();
  tw_end();

  return EXIT_SUCCESS;
}
