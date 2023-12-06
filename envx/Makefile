INCLUDES = -I./include
LIBS  	 = -L./lib -lportsf -lm
PROGS	 = envx.c
PORTSF 	 = ./lib/libportsf.a
CC 	 = clang


sfpan:
	$(CC) envx.c -o envx $(INCLUDES) $(LIBS)

clean:
	rm -f $(PROGS)
	rm -f *.o
