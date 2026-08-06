#ifndef _TK_DEFINES_H
#define _TK_DEFINES_H

#define TIMEKEEPER_PORT 10

#define STATUS_FLAG_MASK     0x01
#define START_FLAG_MASK      0x02
#define STOP_FLAG_MASK       0x04
#define ONCE_FLAG_MASK       0x08
#define WAIT_RES_FLAG_MASK   0x10

typedef enum {
  TK_MODE_STOP = 0,
  TK_MODE_ONCE = 1,
  TK_MODE_CONTINUOUS = 2,
  TK_MODE_SLAVE = 3,
} tk_sync_mode_t;

typedef enum{
	TK_NOERR,
	TK_ITER_ERR,
  TK_SEM_ERR,
} err_timekeeper_t;

#define SYNC_TIMEOUT 10000

#endif
