INCLUDES = -I./include
LIBS  	 = -L./lib -lportsf -lm
PROGS	 = sfpan.c
PORTSF 	 = ./lib/libportsf.a
CC 	 = clang


sfbrkpan:	sfpan.c ExternalFiles/breakpoints.c ./lib/libportsf.a
	$(CC) -o sfpan sfpan.c ExternalFiles/breakpoints.c $(INCLUDES) $(LIBS)

clean:
	rm -f $(PROGS)
	rm -f *.o