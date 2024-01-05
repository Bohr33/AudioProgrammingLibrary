
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

} OSCIL;

void oscil_init(OSCIL* osc, unsigned long srate);
OSCIL* new_oscp(unsigned long srate, double phase);
OSCIL* new_osc(unsigned long srate);
double sinetick(OSCIL* p_osc,  double freq);
