#ifndef _FRONT_H_
#define _FRONT_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include "queue.h"
#include "messaging.h"
#include "signals.h"
#include "error.h"

void* front_proc(void* vargs);

#endif
