CC=gcc
FLAGS=-ggdb -std=c99 -Wno-unused-parameter -Wno-implicit -pedantic 
LIBS=-ltalloc -ltevent

all:
	$(CC) signal_event.c -o signal_event $(FLAGS) $(LIBS)
	$(CC) time_event.c -o time_event $(FLAGS) $(LIBS)
	$(CC) subrequest.c -o subrequest $(FLAGS) $(LIBS)
	$(CC) send.c -o send $(FLAGS) $(LIBS)	
	$(CC) receive.c -o receive $(FLAGS) $(LIBS)
	$(CC) queue.c -o queue $(FLAGS) $(LIBS)
	$(CC) initizalization.c -o initizalization $(FLAGS) $(LIBS)
	$(CC) immediate_event.c -o immediate_event $(FLAGS) $(LIBS)
	$(CC) file_descriptor_event.c -o file_descriptor_event $(FLAGS) $(LIBS)
	$(CC) event_loop_wait.c -o event_loop_wait $(FLAGS) $(LIBS)
	$(CC) event_loop_once.c -o event_loop_once $(FLAGS) $(LIBS)
	$(CC) accesing_data.c -o accesing_data $(FLAGS) $(LIBS)	