#include <stdio.h>
#include <stdlib.h>
#include <breakpoints.h>



int inrange(const BREAKPOINT* points, double minval, double maxval, unsigned long npoints){

    unsigned long i;
    int range_OK = 1;

    for(i = 0; i < npoints; i++){
        if(points[i].value < minval || points[i].value > maxval){
            range_OK = 0;
            break;
        }
    }
    return range_OK;
}

double val_at_brktime(const BREAKPOINT* points, unsigned long npoints, double time){
    unsigned long i;
    BREAKPOINT left, right;
    double frac, val, width;

    //scan until we find a span containing our time
    for(i = 1; i < npoints; i++){
        if(time <= points[i].time)
        break;
    }
    //maintain final value if time beyong edn of data
    if(i == npoints){
        return points[i-1].value;
    }
    left = points[i-1];
    right = points[i];

    //check for instant jump - two points with the same time
    width = right.time - left.time;
    if(width==0.0)
        return right.value;
    //get value from this span using linear interpolatiopn
    frac = (time - left.time) / width;
    val = left.value + (( right.value - left.value) * frac);

    return val;
}

BREAKPOINT maxpoint(const BREAKPOINT* points,unsigned long npoints)
{
    int i;
    BREAKPOINT point;

    point.time = points[0].time;
    point.value = points[0].value;

    for(i=0; i < npoints; i++)
    {
        if(point.value < points[i].value){
            point.value = points[i].value;
            point.time = points[i].time;
        }
    }
    return point;
}

    
BREAKPOINT* get_breakpoints(FILE* fp, unsigned long* psize){
    int got;
    long npoints = 0, size = 64;
    double lasttime = 0.0;
    BREAKPOINT* points = NULL;
    char line[80];

    if(fp==NULL)
        return NULL;
    points = (BREAKPOINT*) malloc(sizeof(BREAKPOINT) * size);
    if(points==NULL)
        return NULL;

    while(fgets(line, 80, fp)){
        got = sscanf(line, "%lf%lf",
                &points[npoints].time, &points[npoints].value);

        if(got < 0)
            continue;//empty line

        if(got==0){
            printf("Line %ld has non-numeric data\n", npoints+1);
            break;
        }

        if(got==1){
            printf("Incomplete breakpoint found at point %ld\n", npoints+1);
            break;
        }
        if(points[npoints].time < lasttime){
            printf("data error at point %ld: time not increasing\n", npoints+1);
            break;
        }
    
        lasttime = points[npoints].time;
        if(++npoints == size){
            BREAKPOINT* tmp;
            size += npoints;
            tmp=(BREAKPOINT*)realloc(points,sizeof(BREAKPOINT) *size);
            if(tmp == NULL){
                npoints = 0;
                free(points);
                points = NULL;
                break;
            }
            points = tmp;
        }
    }
    if(npoints)
        *psize = npoints;
    return points;
}


