# Copyright © 2012 Bart Massey
# [This program is licensed under the "MIT License"]
# Please see the file COPYING in the source
# distribution of this software for license terms.


CC = gcc
CFLAGS = -g -Wall -gstabs
ALL = echoserver

all: $(ALL)

echoserver: echoserver.c
	$(CC) $(CFLAGS) -o echoserver echoserver.c

clean:
	-rm -f $(ALL)
