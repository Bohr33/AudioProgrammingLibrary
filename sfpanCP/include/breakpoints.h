#include <stdio.h>


typedef struct breakpoint {
    double time;
    double value;
} BREAKPOINT;  

typedef struct maxminpoint {
    BREAKPOINT max;
    BREAKPOINT min;
} MAXMINPOINT;

int inrange(const BREAKPOINT* points, double minval, double maxval, unsigned long npoints);
double val_at_brktime(const BREAKPOINT* points, unsigned long npoints, double time);
BREAKPOINT* get_breakpoints(FILE* fp, unsigned long* psize);
MAXMINPOINT minMax(const BREAKPOINT* points, long npoints);
BREAKPOINT minpoint(const BREAKPOINT* points, long npoints);
BREAKPOINT maxpoint(const BREAKPOINT* points, long npoints);