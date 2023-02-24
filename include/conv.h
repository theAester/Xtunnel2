#ifndef _CONV_H_
#define _CONV_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include "queue.h"
#include "messaging.h"
#include "signals.h"
#include "error.h"

void* conv_proc(void* vargs);

#endif
