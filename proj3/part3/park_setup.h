#ifndef PARK_SETUP_H
#define PARK_SETUP_H

#include "data_model.h"

int park_init(Park *park);
void park_destroy(Park *park);
ThreadArg *make_arg(Park *park, int id);

#endif
