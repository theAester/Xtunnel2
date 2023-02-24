#ifndef _ERROR_H_
#define _ERROR_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define eprint(x) \
	fprintf(stderr, "Error at %s:%s:%d [%d] %s\n" x, __FILE__, __func__, __LINE__, errno, strerror(errno));


#define eprintf(x, ...) \
	fprintf(stderr, "Error at %s:%s:%d [%d] %s\n" x, __FILE__, __func__, __LINE__, errno, strerror(errno), __VA_ARGS__);

#endif
