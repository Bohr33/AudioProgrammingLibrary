INCLUDES = -I./include
LIBS  	 = -L./lib -lportsf -lm
PROGS	 = tabgen.c
PORTSF 	 = ./lib/libportsf.a
CC 	 = clang


sf3float:
	$(CC) -o tabgen tabgen.c ExternalFiles/gtable.c ExternalFiles/breakpoints.c $(INCLUDES) $(LIBS)

clean:
	rm -f $(PROGS)
	rm -f *.o
