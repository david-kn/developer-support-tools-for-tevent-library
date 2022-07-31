CC=gcc

all:
	$(CC) libev_bench.c -o libev_bench -lev
	$(CC) libevent_bench.c -o libevent_bench -levent
	$(CC) tevent_bench.c -o tevent_bench -ltalloc -ltevent -levent
	