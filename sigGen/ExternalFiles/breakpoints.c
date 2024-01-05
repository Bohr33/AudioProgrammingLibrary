#include <stdio.h>
#include <stdlib.h>
#include <breakpoints.h>

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif



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

BRKSTREAM* bps_newstream(FILE *fp, unsigned long srate, unsigned long *size){
    BRKSTREAM* stream;
    BREAKPOINT *points;
    unsigned long npoints;

    if(srate == 0){
        printf("Error creating stream - srate cannot be zero\n");
        return NULL;
    }

        stream = (BRKSTREAM*) malloc(sizeof(BRKSTREAM));
        if(stream == NULL)
            return NULL;
        //load breakpoint file and settup info
        points = get_breakpoints(fp, &npoints);
        if(points == NULL){
            free(stream);
            return NULL;
        }
        if(npoints < 2){
            printf("breakpoint file is too small - at least 2 points required\n");
            free(stream);
            return NULL;
        }
        //init the stream object
        stream->points = points;
        stream->npoints = npoints;
        //counters
        stream->curpos = 0.0;
        stream->ileft = 0;
        stream->iright = 1;
        stream->incr = 1.0 / srate;
        //first span
        stream->leftpoint = stream->points[stream->ileft];
        stream->rightpoint = stream->points[stream->iright];
        stream->width = stream->rightpoint.time - stream->leftpoint.time;
        stream->height = stream->rightpoint.value - stream->leftpoint.value;
        stream->more_points = 1;
        if(size)
            *size = stream->npoints;
        return stream;

    
}

double bps_tick(BRKSTREAM* stream){
    double thisval, frac;

    //beyond end of brk data?
    if(stream->more_points == 0)
        return stream->rightpoint.value;
    if(stream->width == 0.0){
        thisval = stream->rightpoint.value;
    }else{
        frac = (stream->curpos-stream->leftpoint.time)/(stream->width);
        thisval = (stream->height * frac) + stream->leftpoint.value;
    }
    //move up ready for next sample
    stream->curpos += stream->incr;
    if(stream->curpos > stream->rightpoint.time){
        stream->ileft ++;
        stream->iright ++;
        if(stream->iright < stream->npoints){
            stream->leftpoint = stream->points[stream->ileft];
            stream->rightpoint = stream->points[stream->iright];
            stream->width = stream->rightpoint.time - stream->leftpoint.time;
            stream->height = stream->rightpoint.value - stream->leftpoint.value;
        }else{
            stream->more_points = 0;
        }
    }
    return thisval;
}

void bps_freepoints(BRKSTREAM* stream){

    if(stream && stream->points){
        free(stream->points);
        stream->points = NULL;
    }
}

int bps_getminmax(BRKSTREAM* stream,double *outmin,double *outmax)
{
	double val,minval,maxval;
	unsigned long i;
	
	if(stream==NULL || stream->npoints < 2)
		return -1;
	
	minval = maxval = stream->points[0].value;

	for(i=1;i < stream->npoints;i++){
		val = stream->points[i].value;
		minval = MIN(minval,val);
		maxval = MAX(maxval,val);
	}
	*outmin = minval;
	*outmax = maxval;
	return 0;
}

