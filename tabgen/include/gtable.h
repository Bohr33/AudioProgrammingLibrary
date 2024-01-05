#include "wave.h"
#include <stdio.h>

enum{SAW_DOWN, SAW_UP};

typedef struct t_gtable{
    double* table;
    unsigned long length;
} GTABLE;


typedef struct t_tab_oscil{

    OSCIL osc;
    const GTABLE* gtable;
    double dtablen;
    double sizeovrsr;
} OSCILT;


GTABLE* new_sine(unsigned long length);
GTABLE* new_triangle(unsigned long length, unsigned long nharms);
GTABLE* new_square(unsigned long length, unsigned long nharms);
GTABLE* new_saw(unsigned long length, unsigned long nharms, int up);
GTABLE* new_pulse(unsigned long length, unsigned long nharms);
GTABLE* new_gtable(unsigned long length);
void gtable_free(GTABLE** gtable);
OSCILT* new_oscilt(unsigned long srate, const GTABLE* gtable);
OSCILT* new_oscilp(unsigned long srate, const GTABLE* gtable, double phase);
double tabtick(OSCILT* p_osc, double freq);
double tabitick(OSCILT* p_osc, double freq);
double tabtickfmod(OSCILT* p_osc, double freq);
double tabitickfmod(OSCILT* p_osc, double freq);