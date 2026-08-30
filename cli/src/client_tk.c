#include "tk/client_tk.h"

// SALLY libraries
#include <service/service_err_codes.h>
#include <csp/csp.h>

// lpldgen-generated packet descriptions
#include <pld_tk_plugin.h>

#include "timekeeper/tk_defines.h"

#include <time.h>

#include <commands/cmd_lp_generic_txn.h>

// some timeout value for the transactions
#define TIMEOUT_MS    3000

// usually, these would be implemented with lp_generic_txn structures,
// and cmd_lp_generic_txn_addr() function calls,
// but since those are in the commands library (which is not strictly needed here),
// we can do a bit of reimplemetnation here

int16_t tk_set_time(uint8_t node, time_t time, uint8_t* rpl_code) {
  struct TK_plugin_TK_SET_TIME_REQ_pack {
      struct TK_plugin_TK_SET_TIME_REQ cmd;
  } pack = {
      .cmd = {
          .pld_id = TK_PLUGIN_TK_SET_TIME_REQ_ID,
          .time = (uint64_t)time
      }
  };

  struct TK_plugin_TK_SET_TIME_RSP* rpl = NULL;

  const struct lp_generic_txn txn = {
      .req_spec = PldSpec_TK_plugin_TK_SET_TIME_REQ,
      .req = &pack.cmd,
      .resp_spec = PldSpec_TK_plugin_TK_SET_TIME_RSP,
      .resp_out = (void *)&rpl,
      .timeout = TIMEOUT_MS,
  };

  int ret = cmd_lp_generic_txn_addr(&txn, node);

  if (ret != 0)
  {
      return (ret);
  }

  (*rpl_code) = rpl->ret_val;

  return(SERVICE_ERR_NONE);
}


int16_t tk_get_time(uint8_t node, time_t* time) {
  struct TK_plugin_TK_GET_TIME_REQ_pack {
      struct TK_plugin_TK_GET_TIME_REQ cmd;
  } pack = {
      .cmd = {
          .pld_id = TK_PLUGIN_TK_GET_TIME_REQ_ID
      }
  };

  struct TK_plugin_TK_GET_TIME_RSP* rpl = NULL;

  const struct lp_generic_txn txn = {
      .req_spec = PldSpec_TK_plugin_TK_GET_TIME_REQ,
      .req = &pack.cmd,
      .resp_spec = PldSpec_TK_plugin_TK_GET_TIME_RSP,
      .resp_out = (void *)&rpl,
      .timeout = TIMEOUT_MS,
  };

  int ret = cmd_lp_generic_txn_addr(&txn, node);

  if (ret != 0)
  {
      return (ret);
  }

  (*time) = rpl->time;

  return(SERVICE_ERR_NONE);
}

int16_t tk_sync_time(uint8_t node, uint8_t flags, int64_t frequency, struct TK_plugin_TK_SYNC_TIME_RSP** rpl) {
  struct TK_plugin_TK_SYNC_TIME_REQ_pack {
      struct TK_plugin_TK_SYNC_TIME_REQ cmd;
  } pack = {
      .cmd = {
          .pld_id = TK_PLUGIN_TK_SYNC_TIME_REQ_ID,
          .frequency = frequency,
          .flags = flags
      }
  };

  const struct lp_generic_txn txn = {
      .req_spec = PldSpec_TK_plugin_TK_SYNC_TIME_REQ,
      .req = &pack.cmd,
      .resp_spec = PldSpec_TK_plugin_TK_SYNC_TIME_RSP,
      .resp_out = (void *)rpl,
      .timeout = SYNC_TIMEOUT,
  };

  int ret = cmd_lp_generic_txn_addr(&txn, node);

  if (ret != 0)
  {
      return (ret);
  }

  return(SERVICE_ERR_NONE);
}