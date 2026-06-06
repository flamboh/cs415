#ifndef MONITOR_H
#define MONITOR_H

#include "data_model.h"

void send_monitor_snapshot(Park *park, int fd, int next_timestep);
void send_closed_snapshot(Park *park, int fd);
void send_final_statistics(Park *park, int fd);
void monitor_process(int read_fd);

#endif
