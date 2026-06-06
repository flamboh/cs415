#ifndef MONITOR_H
#define MONITOR_H

#include "data_model.h"

void send_monitor_snapshot(Park *park, int fd);
void monitor_process(int read_fd);

#endif
