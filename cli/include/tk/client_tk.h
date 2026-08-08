#ifndef DRIVER_TEMPLATE_CLIENT_H
#define DRIVER_TEMPLATE_CLIENT_H

/*!
  \file client.h 
  \brief This template header contains template client commands
  This version performs transactions via CSP.
*/

#include <stdint.h>
#include <time.h>
#include "timekeeper/tk_defines.h"
#include "../../common/lpldgen/include/pld_tk_plugin.h"

// the public client C API
// this should expose all the client functionality
// and therefore, MUST be properly documented by Doxygen!

/*!
  \brief Send a ping to the server, it will reply with "Hello World!\r\n".
  \param node Address of the server to send the request to.
  \param time Target time to set mcu to (default is POSIX computer time).
  \param rpl_code Reply code of the csp transaction.
  \returns 0 for success, negative value for error code.
*/
int16_t tk_set_time(uint8_t node, time_t time, uint8_t* rpl_code);

/*!
  \brief Send a string to be echoed by the server.
  \param node Address of the server to send the request to.
  \param time Temp buffer to store the requested time.
  \returns 0 for success, negative value for error code.
*/
int16_t tk_get_time(uint8_t node, time_t* time);

/*!
  \brief Stop the server.
  \param node Address of the server to send the request to.
  \param frequency A frequency of how often should target node sync time with other nodes.
  \returns 0 for success, negative value for error code.
*/
int16_t tk_sync_time(uint8_t node, uint8_t flags, int64_t frequency, struct TK_plugin_TK_SYNC_TIME_RSP** rpl_out);

#endif
