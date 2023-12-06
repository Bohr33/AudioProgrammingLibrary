INCLUDES = -I./include
LIBS  	 = -L./lib -lportsf -lm
PROGS	 = sfenv.c
PORTSF 	 = ./lib/libportsf.a
CC 	 = clang


sfbrkpan:	sfenv.c breakpoints.c ./lib/libportsf.a
	$(CC) -o sfenv sfenv.c breakpoints.c $(INCLUDES) $(LIBS)

clean:
	rm -f $(PROGS)
	rm -f *.o
