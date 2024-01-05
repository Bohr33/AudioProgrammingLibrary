#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "wave.h"


#ifndef M_PI
#define M_PI (3.1415926535897932)
#endif
#define TWOPI (2.0 * M_PI)


void oscil_init(OSCIL* osc, unsigned long srate)
{
    osc->twopiovrsr = TWOPI/srate;
    osc->curfreq = 0.0;
    osc->curphase = 0.0;
    osc->incr = 0.0;
}

OSCIL* new_oscp(unsigned long srate, double phase)
{

    OSCIL* p_osc = malloc(sizeof(OSCIL));
    if(p_osc == NULL)
        return NULL;
    p_osc->twopiovrsr = TWOPI/ (double) srate;
    p_osc->curfreq = 0.0;
    p_osc->curphase = TWOPI * phase;
    p_osc->incr = 0.0;

    return p_osc;
}

OSCIL* new_osc(unsigned long srate)
{

    OSCIL* p_osc = malloc(sizeof(OSCIL));
    if(p_osc == NULL)
        return NULL;
    p_osc->twopiovrsr = TWOPI/ (double) srate;
    p_osc->curfreq = 0.0;
    p_osc->curphase = 0.0;
    p_osc->incr = 0.0;

    return p_osc;
}

double sinetick(OSCIL* p_osc,  double freq)
{
    double val;

    val = (sin(p_osc->curphase));
    if(p_osc->curfreq != freq)
    {
        p_osc->curfreq = freq;
        p_osc->incr = p_osc->twopiovrsr * freq;
    }
    p_osc->curphase += p_osc->incr;
    if(p_osc->curphase >= TWOPI)
        p_osc->curphase -= TWOPI;
    if(p_osc->curphase < 0)
        p_osc->curphase += TWOPI;
    return val;
}