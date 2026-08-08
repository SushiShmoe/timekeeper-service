// app headers
#include "timekeeper/server_tk.h"

// standard libraries
#include <string.h>

// SALLY libraries
#include <service/service_err_codes.h>
#include <service-csp/csp_pld_txn.h>

// lpldgen-generated packet descriptions
#include "../../common/lpldgen/include/pld_tk_plugin.h"

#include "../../../../src/services/svc_tk.h" // FIXME fix import on integration

// forward declarations of functions that we will use to do the actual stuff
static int8_t tk_set_time(void* vreq, void* vrpl);
static int8_t tk_get_time(void* vreq, void* vrpl);
static int8_t tk_sync_time(void* vreq, void* vrpl);
static int8_t tk_sync_slaves(void* vreq, void* vrpl);


void server_process_timekeeper_packet(csp_packet_t *packet, csp_conn_t* conn) {
  // based on how the service was set up, it will receive either CSP packets or CSP connections (or both)
  // in this case we ignore conections and just process packets
  (void)conn;
  if(!packet) {
    return;
  }

  // it is usually a good idea to do some basic validation here
  if(packet->length < 1) {
    csp_buffer_free(packet);
    return;
  }

  // the rest of this example assumes lpldgen is being used to generate the packet structures
  // if you are not using lpldgen, then feel free to ignore this completely,
  // and just use the CSP packet/connection structs as you see fit
  // just remember to dispose of the packet with csp_buffer_free(packet) when done!

  // SALLY contians a macro that lets yout to statically
  // determine required buffer sizes based on max req/rsp struct size
  uint8_t pld_req_buff[SIZEOF_MAX(struct TK_plugin_TK_SET_TIME_REQ a; \
                                  struct TK_plugin_TK_GET_TIME_REQ b; \
                                  struct TK_plugin_TK_SYNC_TIME_REQ c;
                                  struct TK_plugin_TK_SYNC_SLAVES_REQ d;)];
  uint8_t pld_rsp_buff[SIZEOF_MAX(struct TK_plugin_TK_SET_TIME_RSP a; \
                                  struct TK_plugin_TK_GET_TIME_RSP b;
  	  	  	  	  	  	  	  	  struct TK_plugin_TK_SYNC_TIME_RSP c;
  	  	  	  	  	  	  	  	  struct TK_plugin_TK_SYNC_SLAVES_RSP d;)];
  
  // this structure will hold information about the transaction
  struct csp_pld_txn_t txn = {
    .pack_req = packet,
    .pld_req = pld_req_buff,
    .pld_rsp = pld_rsp_buff,
  };

  // set transaction properties based on the command ID
  // this is just an example - see the SALLY documentation for details
  switch(packet->data[0]) {
    case TK_PLUGIN_TK_SET_TIME_REQ_CMD_ID: {
      txn.bin_size_rsp = TK_PLUGIN_TK_SET_TIME_RSP_BIN_SIZE;
      txn.req_parse_s = (fn_parse_static_t)TK_plugin_TK_SET_TIME_REQ_parse_s;
      txn.rsp_build_s = (fn_build_static_t)TK_plugin_TK_SET_TIME_RSP_build_s;
      txn.fce = tk_set_time;
    } break;

    case TK_PLUGIN_TK_GET_TIME_REQ_CMD_ID: {
      txn.bin_size_rsp = TK_PLUGIN_TK_GET_TIME_RSP_BIN_SIZE;
      txn.req_parse_s = (fn_parse_static_t)TK_plugin_TK_GET_TIME_REQ_parse_s;
      txn.rsp_build_s = (fn_build_static_t)TK_plugin_TK_GET_TIME_RSP_build_s;
      txn.fce = tk_get_time;
    } break;

    case TK_PLUGIN_TK_SYNC_TIME_REQ_CMD_ID: {
      txn.bin_size_rsp = TK_PLUGIN_TK_SYNC_TIME_RSP_BIN_SIZE;
      txn.req_parse_s = (fn_parse_static_t)TK_plugin_TK_SYNC_TIME_REQ_parse_s;
      txn.rsp_build_s = (fn_build_static_t)TK_plugin_TK_SYNC_TIME_RSP_build_s;
      txn.fce = tk_sync_time;
    } break;

    case TK_PLUGIN_TK_SYNC_SLAVES_REQ_CMD_ID: {
      txn.bin_size_rsp = TK_PLUGIN_TK_SYNC_SLAVES_RSP_BIN_SIZE;
      txn.req_parse_s = (fn_parse_static_t)TK_plugin_TK_SYNC_SLAVES_REQ_parse_s;
      txn.rsp_build_s = (fn_build_static_t)TK_plugin_TK_SYNC_SLAVES_RSP_build_s;
      txn.fce = tk_sync_slaves;
    } break;

    default: {
      fprintf(stderr, "Unknown command 0x%02x!", packet->data[0]);
      csp_buffer_free(packet);
      return;
    }
  }

  csp_pld_txn(&txn);
}

// normal vcom set time
static int8_t tk_set_time(void* vreq, void* vrpl) {
  struct TK_plugin_TK_SET_TIME_REQ* req = (struct TK_plugin_TK_SET_TIME_REQ*)vreq;
  struct TK_plugin_TK_SET_TIME_RSP* rpl = (struct TK_plugin_TK_SET_TIME_RSP*)vrpl;
  rpl->pld_id = TK_PLUGIN_TK_SET_TIME_RSP_ID;

  csp_timestamp_t ts;
  ts.tv_sec = (uint32_t)(req->time);
  ts.tv_nsec = 0;
  int status = tk_app_set_time(&ts);

  rpl->ret_val = status;

  return(SERVICE_ERR_NONE);
}

// request from master node to sync
static int8_t tk_sync_slaves(void* vreq, void* vrpl) {
	struct TK_plugin_TK_SYNC_SLAVES_REQ* req = (struct TK_plugin_TK_SYNC_SLAVES_REQ*)vreq;
	struct TK_plugin_TK_SYNC_SLAVES_RSP* rpl = (struct TK_plugin_TK_SYNC_SLAVES_RSP*)vrpl;
	rpl->pld_id = TK_PLUGIN_TK_SYNC_SLAVES_RSP_ID;

	csp_timestamp_t ts;
	ts.tv_sec = (uint32_t)(req->time);
	ts.tv_nsec = 0;
	int status = tk_app_set_time(&ts);

	tk_app_make_slave(req->node);

	rpl->ret_val = status;

	return(SERVICE_ERR_NONE);
}

static int8_t tk_get_time(void* vreq, void* vrpl) {
  (void)vreq;
  struct TK_plugin_TK_GET_TIME_RSP* rpl = (struct TK_plugin_TK_GET_TIME_RSP*)vrpl;
  rpl->pld_id = TK_PLUGIN_TK_GET_TIME_RSP_ID;

  csp_timestamp_t time = {0};

  (void)tk_app_get_time(&time);

  rpl->time = time.tv_sec;

  return(SERVICE_ERR_NONE);
}

static int8_t tk_sync_time(void* vreq, void* vrpl) {
  struct TK_plugin_TK_SYNC_TIME_REQ* req = (struct TK_plugin_TK_SYNC_TIME_REQ*)vreq;
  struct TK_plugin_TK_SYNC_TIME_RSP* rpl = (struct TK_plugin_TK_SYNC_TIME_RSP*)vrpl;
  rpl->pld_id = TK_PLUGIN_TK_SYNC_TIME_RSP_ID;

  tk_app_sync_time(req->frequency, req->flags, rpl);

  return(SERVICE_ERR_NONE);
}
