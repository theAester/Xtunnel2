#!/bin/bash

if [ ! -x ../build ]; then
	mkdir ../build
fi

gcc testsend.c -o ../build/send
gcc testrecv.c -o ../build/recv
