#include <stdio.h>


#ifndef M_PI
#define M_PI (3.1415926535897932)
#endif
#define TWOPI (2.0 * M_PI)

typedef struct t_oscil
{
    double twopiovrsr;
    double curfreq;
    double curphase;
    double incr;
    double pwm;
} OSCIL;

//a pointer to a function
typedef double (*tickfunc) (OSCIL* osc, double);

void oscil_init(OSCIL* osc, unsigned long srate)
{
    osc->twopiovrsr = TWOPI/srate;
    osc->curfreq = 0.0;
    osc->curphase = 0.0;
    osc->incr = 0.0;
    osc->pwm = 0.5;
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

double sqrtick(OSCIL* p_osc,  double freq)
{
    double val;

    if(p_osc->curfreq != freq)
    {
        p_osc->curfreq = freq;
        p_osc->incr = p_osc->twopiovrsr * freq;
    }

    if(p_osc->curphase <= M_PI)
        val = 1;
    else
        val = -1;

    p_osc->curphase += p_osc->incr;
    if(p_osc->curphase >= TWOPI)
        p_osc->curphase -= TWOPI;
    if(p_osc->curphase < 0)
        p_osc->curphase += TWOPI;
    return val;
}

double sawdtick(OSCIL* p_osc,  double freq)
{
    double val;

    if(p_osc->curfreq != freq)
    {
        p_osc->curfreq = freq;
        p_osc->incr = p_osc->twopiovrsr * freq;
    }
    val = 1.0 - 2.0 * (p_osc->curphase * (1.0 /TWOPI));

    p_osc->curphase += p_osc->incr;
    if(p_osc->curphase >= TWOPI)
        p_osc->curphase -= TWOPI;
    if(p_osc->curphase < 0)
        p_osc->curphase += TWOPI;
    return val;
}

double sawutick(OSCIL* p_osc,  double freq)
{
    double val;

    if(p_osc->curfreq != freq)
    {
        p_osc->curfreq = freq;
        p_osc->incr = p_osc->twopiovrsr * freq;
    }
    val = 2.0 * (p_osc->curphase * (1.0 /TWOPI)) - 1;
    
    p_osc->curphase += p_osc->incr;
    if(p_osc->curphase >= TWOPI)
        p_osc->curphase -= TWOPI;
    if(p_osc->curphase < 0)
        p_osc->curphase += TWOPI;
    return val;
}

double tritick(OSCIL* p_osc,  double freq)
{
    double val;

    if(p_osc->curfreq != freq)
    {
        p_osc->curfreq = freq;
        p_osc->incr = p_osc->twopiovrsr * freq;
    }
    //rectified sawtooth
    val = (2.0 * (p_osc->curphase * (1.0 / TWOPI))) - 1.0;
    if(val < 0.0)
        val = -val;
    val = 2.0 * (val - 0.5);
    
    p_osc->curphase += p_osc->incr;
    if(p_osc->curphase >= TWOPI)
        p_osc->curphase -= TWOPI;
    if(p_osc->curphase < 0)
        p_osc->curphase += TWOPI;
    return val;
}

double pwm_tick(OSCIL* p_osc, double freq, double pwmod)
{
    double val;

    if(p_osc->curfreq != freq)
    {
        p_osc->curfreq = freq;
        p_osc->incr = p_osc->twopiovrsr * freq;
    }

    if(p_osc->pwm != pwmod){
        p_osc->pwm = pwmod;   
    }


    //rectified sawtooth
    if(p_osc->curphase < TWOPI * pwmod){
        val = 1;
    }else{
        val = -1;
    }
    
    p_osc->curphase += p_osc->incr;
    if(p_osc->curphase >= TWOPI)
        p_osc->curphase -= TWOPI;
    if(p_osc->curphase < 0)
        p_osc->curphase += TWOPI;
    return val;
}