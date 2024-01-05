INCLUDES = -I./include
LIBS  	 = -L./lib -lportsf -lm
PROGS	 = sinGen.c
PORTSF 	 = ./lib/libportsf.a
CC 	 = clang


sf2float:
	$(CC) -o sigGen sinGen.c ExternalFiles/breakpoints.c $(INCLUDES) $(LIBS)

clean:
	rm -f $(PROGS)
	rm -f *.o
