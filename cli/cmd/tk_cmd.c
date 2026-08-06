#include "tk/cmd_cli_tk.h"
#include "tk/client_tk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <console/cmddef.h>
#include <stdarg.h>

#include <time.h>

#include "timekeeper/tk_defines.h"


// forward declarations of the commands that are to be registered
static int tk_set_time_req(console_ctx_t *ctx, cmd_signature_t *reg);
static int tk_get_time_req(console_ctx_t *ctx, cmd_signature_t *reg);
static int tk_time_sync_req(console_ctx_t *ctx, cmd_signature_t *reg);

// the interface for shared libraries
#if defined(SHARED_BUILD)
void register_cli_cmds(void) {
  register_tk_cmds();
}

void get_version(char* semver, char* gitrev) {
  if(semver) {
    // at most 15 characters and NULL
    strncpy(semver, VERSION, 16);
  }
  if(gitrev) {
    // at most 13 characters and NULL
    strncpy(gitrev, GITREV, 14);
  }
}
#endif

// the registration function
void register_tk_cmds(void) {
  console_group_add("cli", "Client example");

  console_cmd_register(tk_set_time_req, "tk set time");
  console_cmd_register(tk_get_time_req, "tk get time");
  console_cmd_register(tk_time_sync_req, "tk time sync");
}

// implementations of all commands follow

static int tk_set_time_req(console_ctx_t *ctx, cmd_signature_t *reg) {
  (void)ctx;

  static struct {
    struct arg_int *node;
    struct arg_end *end;
  } args;

  if (reg) {
    args.node = arg_int1(NULL, NULL, "<node>", EXPENDABLE_STRING("VCOM node to set time on."));
    args.end = arg_end(2);

    reg->argtable = &args;
    reg->help = EXPENDABLE_STRING("Set timekeeper time on set node.");
    return(0);
  }

  uint8_t node = 0;
  if(args.node->count > 0) {
    node = args.node->ival[0];
  }

  struct timespec posix_time = {0};
  if (clock_gettime(CLOCK_REALTIME, &posix_time) != 0){
    console_printf("\tSet time: Failed to get POSIX time.\n");
    return CONSOLE_ERR_IO;
  }


  time_t time = posix_time.tv_sec;
  struct tm *tm_info = gmtime(&time);

  if (tm_info == NULL) {
      console_printf("\tSet time: invalid time\n");
  } else {
      console_printf(
          "\tSetting time on node %d to: %04d-%02d-%02d %02d:%02d:%02d UTC | Raw: %ld\n",
          node,
          tm_info->tm_year + 1900,
          tm_info->tm_mon + 1,
          tm_info->tm_mday,
          tm_info->tm_hour,
          tm_info->tm_min,
          tm_info->tm_sec,
          time
      );
  }

  uint8_t rpl_code = 0;
  int16_t ret = tk_set_time(node, time, &rpl_code);

  if(ret != 0) {
    console_printf("\tFailed, code %d\n", ret);
    return CONSOLE_ERR_IO;
  }


  console_printf("\tTime successfully set.\n");
  return CONSOLE_OK;
}

static int tk_get_time_req(console_ctx_t *ctx, cmd_signature_t *reg) {
  (void)ctx;

  static struct {
    struct arg_int *node;
    struct arg_end *end;
  } args;

  if (reg) {
    args.node = arg_int1(NULL, NULL, "<node>", EXPENDABLE_STRING("VCOM node to get time from."));
    args.end = arg_end(2);

    reg->argtable = &args;
    reg->help = EXPENDABLE_STRING("Get time from timekeeper on set node.");
    return(0);
  }

  uint8_t node = 0;
  if(args.node->count > 0) {
    node = args.node->ival[0];
  }

  time_t time = 0;
  int16_t ret = tk_get_time(node, &time);

  if(ret != 0) {
    console_printf("Failed, code %d\n", ret);
    return CONSOLE_ERR_IO;
  }

  struct tm *tm_info = gmtime(&time);

  if (tm_info == NULL) {
      console_printf("\tGet time: invalid time\n");
  } else {
      console_printf(
          "\tTime on node %d is: %04d-%02d-%02d %02d:%02d:%02d UTC | Raw: %ld\n",
          node,
          tm_info->tm_year + 1900,
          tm_info->tm_mon + 1,
          tm_info->tm_mday,
          tm_info->tm_hour,
          tm_info->tm_min,
          tm_info->tm_sec,
          time
      );
  }

  return CONSOLE_OK;
}

static int tk_time_sync_req(console_ctx_t *ctx, cmd_signature_t *reg) {
  (void)ctx;

  static struct {
    struct arg_int *node;

    struct arg_lit *status;
    struct arg_lit *start;
    struct arg_lit *stop;
    struct arg_lit *once;
    struct arg_lit *wait;

    struct arg_int *frequency;

    struct arg_end *end;
  } args;

  if (reg) {
    args.node = arg_int1(NULL, NULL, "<node>", EXPENDABLE_STRING("VCOM node to sync time from."));

    args.status = arg_lit0("s", "status", EXPENDABLE_STRING("Get time status of timekeeper on node."));
    args.start = arg_lit0("c", "continuous", EXPENDABLE_STRING("Start continuous time syncing on node."));
    args.stop = arg_lit0("p", "stop", EXPENDABLE_STRING("Stop time syncing on node."));
    args.once = arg_lit0("o", "once", EXPENDABLE_STRING("Perform one time sync on node."));
    args.wait = arg_lit0("w", "wait", EXPENDABLE_STRING("Wait for the operation to complete."));

    args.frequency = arg_int0("f", "frequency", "<frequency>", EXPENDABLE_STRING("The frequency of time syncing among nodes."));
    args.end = arg_end(2);

    reg->argtable = &args;
    reg->help = EXPENDABLE_STRING("Sync times through all nodes.");
    return(0);
  }

  uint8_t node = 0;
  if (args.node->count > 0) {
    node = args.node->ival[0];
  } else{
    console_printf("Node must be specified.\n");
    return CONSOLE_ERR_IO;
  }

  uint8_t flags = 0;

  if(args.status->count > 0) {
    flags |= STATUS_FLAG_MASK;
  }

  if(args.start->count > 0) {
    flags |= START_FLAG_MASK;
  }

  if(args.stop->count > 0) {
    flags |= STOP_FLAG_MASK;
  }

  if(args.once->count > 0) {
    flags |= ONCE_FLAG_MASK;
  }

  if(args.wait->count > 0) {
    flags |= WAIT_RES_FLAG_MASK;
  }

  // Checking mutual exclusivity
  int flags_count = 0;

  flags_count += (args.start->count > 0);
  flags_count += (args.stop->count > 0);
  flags_count += (args.once->count > 0);

  if (flags == 0){
    console_printf("At least one of the following flags must be set: --status, --start, --stop, --once.\n");
    return CONSOLE_ERR_IO;
  } else if (flags_count != 1 && !(flags == STATUS_FLAG_MASK)){
    console_printf("Flags have to be mutualy exclusive, you can pick only one of: --start, --stop, --once.\n");
    return CONSOLE_ERR_IO;
  }

  int64_t frequency = -1;
  if(args.frequency->count > 0) {
    frequency = args.frequency->ival[0];
  }

  struct TK_plugin_TK_SYNC_TIME_RSP* rpl;
  int16_t ret = tk_sync_time(node, flags, frequency, &rpl);

  if(ret != 0 || rpl == NULL) {
    console_printf("Failed, code %d\n", ret);
    return CONSOLE_ERR_IO;
  }

  if (rpl->ret_val != 0){
    console_printf("\tOperation failed.\n");
    return CONSOLE_OK;
  }

  if (flags & STATUS_FLAG_MASK){
    switch (rpl->error){
        case TK_NOERR:{
            console_printf("\tError: No error\n");
        } break;
        case TK_ITER_ERR:{
            console_printf("\tError: Sync failed\n");
            console_printf("\tLast failed node: %d\n", rpl->last_failed_node);
        } break;
        case TK_SEM_ERR:{
            console_printf("\tError: Semaphore init failed\n");
        } break;
        default :{
            console_printf("\tError: Wrong error code\n");
        } break;
    }

    time_t timestamp = (time_t)rpl->last_sync;

    struct tm *tm_info = gmtime(&timestamp);

    if (tm_info == NULL) {
        console_printf("\tLast sync: invalid time\n");
    } else {
        console_printf(
            "\tLast sync: %04d-%02d-%02d %02d:%02d:%02d UTC | Raw: %ld\n",
            tm_info->tm_year + 1900,
            tm_info->tm_mon + 1,
            tm_info->tm_mday,
            tm_info->tm_hour,
            tm_info->tm_min,
            tm_info->tm_sec,
            rpl->last_sync
        );
    }

    switch (rpl->mode) {
      case TK_MODE_ONCE:
        console_printf("\tActive mode: ONCE\n");
        break;
      case TK_MODE_CONTINUOUS:
        console_printf("\tActive mode: CONTINUOUS\n");
        break;
      case TK_MODE_STOP:
        console_printf("\tActive mode: STOP\n");
        break;
      case TK_MODE_SLAVE:
        console_printf("\tActive mode: SLAVE\n");
        break;
      default:
        console_printf("\tActive mode: UNKNOWN\n");
    }

    if (rpl->master_node == 0){
      console_printf("\tMaster node: This node is master\n");
    } else{
      console_printf("\tMaster node: %d\n", rpl->master_node);
    }
  }
  
  return CONSOLE_OK;
}
