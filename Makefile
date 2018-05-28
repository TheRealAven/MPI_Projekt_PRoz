CC=mpicc

HEADER = %.h
DEPS = main.c monitor.c list.c

$(HEADER):
	 

all: $(DEPS) $(HEADER)
	$(CC) $(DEPS) -o main

clean:
	rm -f main
