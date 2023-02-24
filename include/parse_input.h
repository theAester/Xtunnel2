#ifndef _PARSE_INPUT_H_
#define _PARSE_INPUT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <arpa/inet.h>

#include "messaging.h"

#define DIR_SERVER 0
#define DIR_CLIENT 1

extern char* progname;

int print_usage(int);

int	parse_input(int argc, char* argv[], int* listenport, struct sockaddr_in* xserveraddr, struct client_list** clientaddr_list, int* list_len, int* dir);

#endif
