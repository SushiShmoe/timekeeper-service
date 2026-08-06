/*
 * app_timekeeper.h
 *
 *  Created on: May 18, 2026
 *      Author: maty
 */

#ifndef INC_APP_TIMEKEEPER_H_
#define INC_APP_TIMEKEEPER_H_

#include <csp/arch/csp_clock.h>
#include "pld_tk_plugin.h"

int tk_app_get_time(csp_timestamp_t * time);

int tk_app_set_time(const csp_timestamp_t * time);

void tk_app_sync_time(long frequency, uint8_t flags, struct TK_plugin_TK_SYNC_TIME_RSP* rsp);

void tk_app_make_slave(uint8_t master_node);

void app_init_timekeeper(void);

#endif /* INC_APP_TIMEKEEPER_H_ */
