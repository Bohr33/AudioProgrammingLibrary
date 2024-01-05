#include "gtable.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>


//Generic function that will normalize an incoming gtable
//The static keyword will not allow this function to be used outside of the functions within this file. It hides it from the linking process on compiling so it is like a "hidden" function
static void norm_gtable(GTABLE* gtable){

    unsigned long i;
    double maxamp = 0.0, amp;

    for(i = 0; i < gtable->length; i++){
        amp = fabs(gtable->table[i]);
        if(maxamp < amp){
            maxamp = amp;
        }
    }
    maxamp = 1.0 / maxamp;
    for(i = 0; i < gtable->length; i++)
        gtable->table[i] *= maxamp;

    //guard point
    gtable->table[i] = gtable->table[0];
}


GTABLE* new_sine(unsigned long length){
    double step;
    int i;
    GTABLE* gtable = NULL;

    if(length <= 0)
        return NULL;

    //initialize gtable
    gtable = (GTABLE*) malloc(sizeof(GTABLE));
    if(gtable == NULL)
        return NULL;

    gtable->table = (double*) malloc(sizeof(double) * (length + 1));
    if(gtable->table == NULL){
        free(gtable);
        return NULL;
    }
    gtable->length = length;

    //fill in the table
    step = TWOPI/length;
    for(i = 0; i < length; i++){
        gtable->table[i] = sin(step * i);
    }
    gtable->table[i] = gtable->table[0]; //guard point

    return gtable;
}

GTABLE* new_triangle(unsigned long length, unsigned long nharms){
    unsigned long i, j;
    double step, amp;
    unsigned long harmonic = 1;
    GTABLE* gtable;

    if(length <= 0 || nharms == 0 || nharms >= length/2)
        return NULL;

    //initialize gtable
    gtable = new_gtable(length);
    if(gtable == NULL)
        return NULL;
    
    //Generate tri wave
    step = TWOPI/length;
    for(i = 0; i < nharms; i++){
        amp = 1.0 / (harmonic * harmonic);
        for(j = 0; j < length; j++)
            gtable->table[j] += amp * cos(j * harmonic * step);
        harmonic+=2;
    }

    norm_gtable(gtable);
    return gtable;
}

GTABLE* new_square(unsigned long length, unsigned long nharms){
    unsigned long i, j;
    double step, amp = 1.0;
    unsigned long harmonic = 1;
    GTABLE* gtable;

    if(length <= 0 || nharms == 0 || nharms >= length/2)
        return NULL;

    //initialize gtable
    gtable = new_gtable(length);
    if(gtable == NULL)
        return NULL;
    
    //Generate tri wave
    step = TWOPI/length;
    for(i = 0; i < nharms; i++){
        amp = 1.0 / (harmonic);
        for(j = 0; j < length; j++)
            gtable->table[j] += amp * sin(j * harmonic * step);
        harmonic+=2;
    }

    norm_gtable(gtable);
    return gtable;
}

GTABLE* new_saw(unsigned long length, unsigned long nharms, int up){
    unsigned long i, j;
    double step, val, amp;
    unsigned long harmonic = 1;
    GTABLE* gtable;

    if(length <= 0 || nharms == 0 || nharms >= length/2)
        return NULL;

    //initialize gtable
    gtable = new_gtable(length);
    if(gtable == NULL)
        return NULL;
    
    if(up)
        amp = -1;

    //Generate tri wave
    step = TWOPI/length;
    for(i = 0; i < nharms; i++){
        val = amp / (harmonic);
        for(j = 0; j < length; j++)
            gtable->table[j] += val * sin(j * harmonic * step);
        harmonic++;
    }

    norm_gtable(gtable);
    return gtable;
}

GTABLE* new_pulse(unsigned long length, unsigned long nharms){
    unsigned long i, j;
    double step;
    unsigned long harmonic = 1;
    GTABLE* gtable;

    if(length <= 0 || nharms == 0 || nharms >= length/2)
        return NULL;

    //initialize gtable
    gtable = new_gtable(length);
    if(gtable == NULL)
        return NULL;
    
    //Generate pulse wave
    step = TWOPI/length;
    for(i = 0; i < nharms; i++){
        for(j = 0; j < length; j++)
            gtable->table[j] += cos(j * harmonic * step);
        harmonic++;
    }

    norm_gtable(gtable);
    return gtable;
}

//generic function that creates a new gtable and returns its address
GTABLE* new_gtable(unsigned long length){

    unsigned long i;
    GTABLE* gtable = NULL;

    if(length <= 0)
        return NULL;

    //initialize gtable
    gtable = (GTABLE*) malloc(sizeof(GTABLE));
    if(gtable == NULL)
        return NULL;

    gtable->table = (double*) malloc(sizeof(double) * (length + 1));
    if(gtable->table == NULL){
        free(gtable);
        return NULL;
    }

    for(i = 0; i < length; i++)
        gtable->table[i] = 0.0;

    gtable->length = length;
    return gtable;
}


//Deconstruction function
void gtable_free(GTABLE** gtable){


    if(gtable && *gtable && (*gtable)->table){

        free((*gtable)->table);
        free(*gtable);
        *gtable = NULL;
    }
}

OSCILT* new_oscilp(unsigned long srate, const GTABLE* gtable, double phase){

    OSCILT* p_osc;

    if(gtable == NULL || gtable->table == NULL || gtable->length == 0)
        return NULL;

    p_osc = (OSCILT*) malloc(sizeof(OSCILT));
    if(p_osc == NULL)
        return NULL;

    //initalize p_osc
    p_osc->osc.curfreq = 0.0;
    p_osc->osc.curphase = gtable->length*phase;
    p_osc->osc.incr = 0.0;
    //then G-table specifics
    p_osc->gtable = gtable;
    p_osc->dtablen = (double) gtable->length;
    p_osc->sizeovrsr = p_osc->dtablen / srate;
    return p_osc;
}

OSCILT* new_oscilt(unsigned long srate, const GTABLE* gtable){

    OSCILT* p_osc;

    if(gtable == NULL || gtable->table == NULL || gtable->length == 0)
        return NULL;

    p_osc = (OSCILT*) malloc(sizeof(OSCILT));
    if(p_osc == NULL)
        return NULL;

    //initalize p_osc
    p_osc->osc.curfreq = 0.0;
    p_osc->osc.curphase = 0.0;
    p_osc->osc.incr = 0.0;
    //then G-table specifics
    p_osc->gtable = gtable;
    p_osc->dtablen = (double) gtable->length;
    p_osc->sizeovrsr = p_osc->dtablen / srate;
    return p_osc;
}


//Truncating and interpolating tick functions
double tabtick(OSCILT* p_osc, double freq){

    int index = p_osc->osc.curphase;

    double dtablength = p_osc->dtablen, curphase = p_osc->osc.curphase;
    double *table = p_osc->gtable->table;
    if(p_osc->osc.curfreq != freq){
        p_osc->osc.curfreq = freq;
        p_osc->osc.incr = p_osc->osc.curfreq * p_osc->sizeovrsr;
    }
    curphase += p_osc->osc.incr;

    while(curphase > dtablength)
        curphase -= dtablength;
    while(curphase < 0)
        curphase += dtablength;
    p_osc->osc.curphase = curphase;
    
    return table[index];
}

//Truncating and interpolating tick functions
double tabtickfmod(OSCILT* p_osc, double freq){

    int index = p_osc->osc.curphase;

    double dtablength = p_osc->dtablen, curphase = p_osc->osc.curphase;
    double *table = p_osc->gtable->table;
    if(p_osc->osc.curfreq != freq){
        p_osc->osc.curfreq = freq;
        p_osc->osc.incr = p_osc->osc.curfreq * p_osc->sizeovrsr;
    }
    curphase += p_osc->osc.incr;

    curphase = fmod(curphase, dtablength);
    p_osc->osc.curphase = curphase;
    
    return table[index];
}

double tabitick(OSCILT* p_osc, double freq){

    int base_index = p_osc->osc.curphase;
    int inext = base_index + 1;
    double frac, val, slope;

    double dtablen = p_osc->dtablen, curphase = p_osc->osc.curphase;
    double *table = p_osc->gtable->table;
    if(p_osc->osc.curfreq != freq){
        p_osc->osc.curfreq = freq;
        p_osc->osc.incr = p_osc->osc.curfreq * p_osc->sizeovrsr;
    }

    frac = curphase - base_index;
    val = table[base_index];
    slope = table[inext] - val;
    val += (slope * frac);

    curphase += p_osc->osc.incr;
    while(curphase > dtablen)
        curphase -= dtablen;
    while(curphase < 0)
        curphase += dtablen;
    p_osc->osc.curphase = curphase;

    return val;
}

double tabitickfmod(OSCILT* p_osc, double freq){

    int base_index = p_osc->osc.curphase;
    int inext = base_index + 1;
    double frac, val, slope;

    double dtablen = p_osc->dtablen, curphase = p_osc->osc.curphase;
    double *table = p_osc->gtable->table;
    if(p_osc->osc.curfreq != freq){
        p_osc->osc.curfreq = freq;
        p_osc->osc.incr = p_osc->osc.curfreq * p_osc->sizeovrsr;
    }

    frac = curphase - base_index;
    val = table[base_index];
    slope = table[inext] - val;
    val += (slope * frac);

    curphase += p_osc->osc.incr;

    curphase = fmod(curphase, dtablen);
    p_osc->osc.curphase = curphase;

    return val;
}

