INCLUDES = -I./include
LIBS  	 = -L./lib -lportsf -lm
PROGS	 = sfpanCP.c
PORTSF 	 = ./lib/libportsf.a
CC 	 = clang


sfbrkpan:	sfpanCP.c breakpoints.c ./lib/libportsf.a
	$(CC) -o sfpanCP sfpanCP.c breakpoints.c $(INCLUDES) $(LIBS)

clean:
	rm -f $(PROGS)
	rm -f *.o