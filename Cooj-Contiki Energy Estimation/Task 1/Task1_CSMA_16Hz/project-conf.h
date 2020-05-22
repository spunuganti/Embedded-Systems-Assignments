
#define NETSTACK_CONF_MAC_CHANNEL_CHECK_RATE 16

#define NETSTACK_CONF_MAC csma_driver

#ifndef PROJECT_CONF_H
#define PROJECT_CONF_H

#include "sys/clock.h"

void powertrace_start(clock_time_t perioc);
void powertrace_stop(void);

void powertrace_print(char *str);

#endif /* PROJECT_CONF_H */

