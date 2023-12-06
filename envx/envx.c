/* Copyright (c) 2009 Richard Dobson

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/
/* enx.c: extract amplitude audio from a soundfile*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <portsf.h>

/* set size of multi-channel frame-buffer */
#define NFRAMES (1024)
#define max(x,y) ((x) > (y) ? (x) : (y))
#define DEFAULT_WINDOW_MSECS (15);

/* TODO define program argument list, excluding flags */
enum {ARG_PROGNAME,ARG_INFILE,ARG_OUTFILE,ARG_NARGS};

double maxsamp(float* buf, unsigned long blocksize);

double maxsamp(float* buf, unsigned long blocksize){

    double peak = 0;
    double absval;

    for(unsigned long i=0; i < blocksize; i++){

        absval = fabs(buf[i]);
        peak = max(absval, peak);
    }
    return peak;
}

int main(int argc, char* argv[])
{
/* STAGE 1 */	
	PSF_PROPS inprops;
	long framesread;	
	/* init all dynamic resources to default states */
	int infile = -1,outfile = -1;
	int error = 0;
	unsigned long nframes = NFRAMES;
	float* inframe = NULL;

	FILE *fp = NULL; //This will be the output file we create
	double windur = DEFAULT_WINDOW_MSECS; //duration of the window size that our amplitude seeking function uses to get sampls in msecs
	unsigned long winsize; //this will be windur divided by the sample rate of our input file to give us a value related to time
	double brktime; //holds the time for the current breakpooint time
	unsigned long npoints; //counts the number of points created in the breakpoint file
	
/* STAGE 2 */	
	printf("ENVX: Create breakpoint text file of input file amplitude envelope\n");						

	/* process any optional flags: remove this block if none used! */
	if(argc > 1){
		char flag;
		while(argv[1][0] == '-'){
			flag = argv[1][1];
			switch(flag){
			/*TODO: handle any  flag arguments here */
			case('\0'):
				printf("Error: missing flag name\n");
				return 1;
			case('w'):
				windur = atof(&argv[1][2]);
				if(windur <= 0.0){
					printf("bad value for Window Duration. Must be positive\n");
					return 1;
				}
			default:
				break;
			}
			argc--;
			argv++;
		}
	}

	/* check rest of commandline */
	if(argc < ARG_NARGS){
		printf("insufficient arguments.\n"
			/* TODO: add required usage message */
			"usage: envx [-wN] insndfile outfile.brk\n"
			" -wN: set extraction window size to N msecs\n"
			"(default is 15)\n"
			);
		return 1;
	}
	/*  always startup portsf */
	if(psf_init()){
		printf("unable to start portsf\n");
		return 1;
	}

/* STAGE 3 */																							
	infile = psf_sndOpen(argv[ARG_INFILE],&inprops,0);															  
	if(infile < 0){
		printf("Error: unable to open infile %s\n",argv[ARG_INFILE]);
		error++;
		goto exit;
	}
	/* TODO: verify infile format for this application */
	if(inprops.chans != 1){
		printf("Error: input file must be mono\n");
		goto exit;
	}

	printf("Infile sample rate = %d", inprops.srate);
	//convert windur from milliseconds to seconds
	windur /= 1000.0;
	//create relative sample based value for the window that reads the file
	winsize = (unsigned long) (windur * inprops.srate);
	printf("winsize = %lu\n", winsize);
	printf("windur = %f\n", windur);

	//allocate memory for the input frame
	inframe = (float*) malloc(winsize * sizeof(float));
	if(inframe == NULL){
		puts("No Memory!\n");
		error++;
		goto exit;
	}

/* STAGE 4 */												
	/* TODO: any other argument processing and setup of variables, 		     
	   output buffer, etc., before creating outfile
	*/

	/* handle outfile */
	fp = fopen(argv[ARG_OUTFILE], "w");
	if(fp == NULL){
		printf("Error creating output breakpoint file %s\n",argv[ARG_OUTFILE]);
		error++;
		goto exit;
	}

/* STAGE 5 */	
	printf("processing....\n");								
	/* TODO: init any loop-related variables */
	brktime = 0.0;
	npoints = 0.0;


	while ((framesread = psf_sndReadFloatFrames(infile,inframe,winsize)) > 0){

		double amp;
		amp = maxsamp(inframe, framesread);
		
		if(fprintf(fp, "%f\t%f\n", brktime, amp) < 2){
			printf("failed to write to breakpoint file %s\n", argv[ARG_OUTFILE]);
			error++;
			break;
		}
		npoints++;
		brktime += windur;		
	}

	if(framesread < 0)	{
		printf("Error reading infile. Outfile is incomplete.\n");
		error++;
	}
	else{
		printf("Done: %d errors\n",error);
		printf("The number of breakpoints written to %s was %lu\n", argv[ARG_OUTFILE], npoints);
	}


/* STAGE 7 */	
	/* do all cleanup  */    									
exit:	 	
	if(infile >= 0)
		if(psf_sndClose(infile))
			printf("%s: Warning: error closing infile %s\n",argv[ARG_PROGNAME],argv[ARG_INFILE]);
	if(inframe)
		free(inframe);
	if(fp){
		if(fclose(fp))
			printf("Error closing sound file %s\n", argv[ARG_OUTFILE]);
	}


	psf_finish();
	return error;
}
