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
/* main.c :  generic soundfile processing main function*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <portsf.h>
#include <breakpoints.h>

/* set size of multi-channel frame-buffer */
#define NFRAMES (1024)


typedef struct panpos{
	double left;
	double right;
} PANPOS;

/* TODO define program argument list, excluding flags */
enum {ARG_PROGNAME,ARG_INFILE,ARG_OUTFILE, ARG_PANPOS, ARG_NARGS};

PANPOS simplepan(double position);
PANPOS cp_pan(double position);

PANPOS simplepan(double position){

	PANPOS pos;

	position *= 0.5;
	pos.left = position - 0.5;
	pos.right = position + 0.5;

	return pos;
}

PANPOS cp_pan(double position){

	PANPOS pos;
	
	const double piovr4 = atan(1.0);
	const double root2ovr2 =  (sqrt(2))/2;
	float thispos = piovr4 * position;	
	const double cosout = cos(thispos);
	const double sinout = sin(thispos);


	pos.left = (root2ovr2) * (cosout - sinout);
	pos.right = (root2ovr2) * (cosout + sinout);

	return pos;
}

int main(int argc, char* argv[])
{
/* STAGE 1 */	
	PSF_PROPS inprops,outprops;									
	
	/* init all dynamic resources to default states */
	int infile = -1,outfile = -1;
	int error = 0;
	PSF_CHPEAK* peaks = NULL;	
	psf_format outformat =  PSF_FMT_UNKNOWN;

	//Buffer loop related variables
	long framesread;	
	unsigned long nframes = NFRAMES;
	float* inframe = NULL;
	float* outframe = NULL;
	//Pan position object
	PANPOS pos;
	double stereopos = 0;

	int sflag = 0;

	//Declare variables for breakpoint file reading
	FILE* panFile = NULL;
	unsigned long panSize = 0;
	BRKSTREAM* panPoints = NULL;
	double maxPan, minPan;

	unsigned long srate;


	
	
/* STAGE 2 */	
	printf("SFPAN: Pan an audio file based on a given pan position break point file \n");						

	/* process any optional flags: remove this block if none used! */
	if(argc > 1){
		char flag;
		while(argv[1][0] == '-'){
			flag = argv[1][1];
			switch(flag){
			/*TODO: handle any  flag arguments here */
			case('s'):
				sflag = 1;
				break;

			case('\0'):
				printf("Error: missing flag name\n");
				return 1;
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
			"usage: sfpan [-s] infile outfile posfile.brk\n"
			"posfile.brk is a breakpoint file"
			"with values in the range of -1.00 to 1.00\n"
			"-1.00 = Full Left; 0.00 = Center; 1.00 = Full Right\n"
			);
		return 1;
	}

	if(sflag){
		printf("Simple Pan mode selected\n");
	}else{
		printf("Constant Power Pan mode selected\n");
	}

	/*  always startup portsf */
	if(psf_init()){
		printf("unable to start portsf\n");
		return 1;
	}

	//Open sound file first to determine samplerate;
	infile = psf_sndOpen(argv[ARG_INFILE], &inprops, 0);
	if(infile < 0){
		
	}
	//Check input file is mono
	if(inprops.chans != 1){
		printf("Error: infile must be MONO");
		error++;
		goto exit;
	}
	srate = inprops.srate;

	//Read and verify breakpoint file
	panFile = fopen(argv[ARG_PANPOS], "r");
	if(panFile == NULL){
		stereopos = atof(argv[ARG_PANPOS]);
		if(stereopos < -1.0 || stereopos > 1.0){
			printf("Error: pan value of %f is out of range\n"
					"Range: -1.0 to 1.0\n", stereopos);
			error++;
			goto exit;
		}
	}else{

		panPoints = bps_newstream(panFile, srate, &panSize);

		if(panPoints == NULL){
			printf("No breakpoints read.\n");
			error++;
			goto exit;
		}
		//check values are inrange with function from breakpoints.c
		if(!inrange(panPoints->points, -1, 1.0, panSize)){
			printf("Error: at least one value in breakpoint file is out of range (-1, 1)\n");
			error++;
			goto exit;
		}

		bps_getminmax(panPoints, &maxPan, &minPan);
	}






/* STAGE 3 */																							

	/* allocate space for  sample frame buffer ...*/
	inframe = (float*) malloc(nframes * inprops.chans * sizeof(float));
	if(inframe==NULL){
		puts("No memory!\n");
		error++;
		goto exit;
	}
	/* check file extension of outfile name, so we use correct output file format*/
	outformat = psf_getFormatExt(argv[ARG_OUTFILE]);
	if(outformat == PSF_FMT_UNKNOWN){
		printf("outfile name %s has unknown format.\n"
			"Use any of .wav, .aiff, .aif, .afc, .aifc\n",argv[ARG_OUTFILE]);
		error++;
		goto exit;
	}
	inprops.format = outformat;
	outprops = inprops;

	//Adjust output props to have proper channels for stereo
	outprops.chans = 2;


/* STAGE 4 */												
	/* TODO: any other argument processing and setup of variables, 		     
	   output buffer, etc., before creating outfile
	*/

	/* handle outfile */
	/* TODO:  make any changes to outprops here  */

	peaks  =  (PSF_CHPEAK*) malloc(outprops.chans * sizeof(PSF_CHPEAK));
	if(peaks == NULL){
		puts("No memory!\n");
		error++;
		goto exit;
	}

	/* TODO: if outchans != inchans, allocate outframe, and modify main loop accordingly */
	outframe = (float *) malloc(nframes * outprops.chans * sizeof(float));
	if(outframe == NULL){
		puts("No Memory!\n");
		error++;
		goto exit;
	}

	outfile = psf_sndCreate(argv[ARG_OUTFILE],&outprops,0,0,PSF_CREATE_RDWR);
	if(outfile < 0){
		printf("Error: unable to create outfile %s\n",argv[ARG_OUTFILE]);
		error++;
		goto exit;
	}


/* STAGE 5 */
	//set calculate pan of stereopos in case of no breakpoint file	
	if(sflag){
		pos = simplepan(stereopos);
	}else{
		pos = cp_pan(stereopos);
	}

	printf("processing....\n");		

	while ((framesread = psf_sndReadFloatFrames(infile,inframe,nframes)) > 0){
	
		/* <--------  add buffer processing here ------>  */
		int i, out_i;
		
		for(i = 0, out_i = 0; i < framesread; i++){

			if(panPoints){
				stereopos = bps_tick(panPoints);
					if(sflag){
						pos = simplepan(stereopos);
					}else{
						pos = cp_pan(stereopos);
					}
			}
			outframe[out_i++] = (float) (inframe[i]*pos.left);
			outframe[out_i++] = (float) (inframe[i]*pos.right);
		}

		if(psf_sndWriteFloatFrames(outfile,outframe,framesread) != framesread){
			printf("Error writing to outfile\n");
			error++;
			break;
		}			
	}

	if(framesread < 0)	{
		printf("Error reading infile. Outfile is incomplete.\n");
		error++;
	}
	else
		printf("Done: %d errors\n",error);
/* STAGE 6 */		
	/* report PEAK values to user */							
	if(psf_sndReadPeaks(outfile,peaks,NULL) > 0){
		long i;
		double peaktime;
		printf("PEAK information:\n");	
			for(i=0;i < inprops.chans;i++){
				peaktime = (double) peaks[i].pos / (double) inprops.srate;
				printf("CH %ld:\t%.4f at %.4f secs\n", i+1, peaks[i].val, peaktime);
			}
	}

/* STAGE 7 */	
	/* do all cleanup  */    									
exit:	 	
	if(infile >= 0)
		if(psf_sndClose(infile))
			printf("%s: Warning: error closing infile %s\n",argv[ARG_PROGNAME],argv[ARG_INFILE]);
	if(outfile >= 0)
		if(psf_sndClose(outfile))
			printf("%s: Warning: error closing outfile %s\n",argv[ARG_PROGNAME],argv[ARG_OUTFILE]);
	if(inframe)
		free(inframe);
	if(outframe)
		free(outframe);
	if(panFile){
		fclose(panFile);
	}
	if(peaks)
		free(peaks);
	if(panPoints){
		bps_freepoints(panPoints);
		free(panPoints);
	}
	/*TODO: cleanup any other resources */

	psf_finish();
	return error;
}
