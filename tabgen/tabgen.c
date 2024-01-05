#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "portsf.h"
#include "gtable.h"
#include "breakpoints.h"


enum {ARG_PROGNAME, ARG_OUTFILE, ARG_DURATION, ARG_SAMPLERATE, ARG_CHANS, ARG_AMP, ARG_FREQ, ARG_WAVE, ARG_NHARMS, ARG_NARGS};
#define BUFFERSIZE 256
#define TABLELEN (1024)

int main(int argc, char** argv){

    PSF_PROPS outprops;
    psf_format outformat = PSF_FMT_UNKNOWN;

    //Variables to hold arguments
    int srate, nchans, wave, nharms;
    double duration, amp, freq;

    //flags
    unsigned long tabwidth = TABLELEN;
    double phase = 0.0;

    //Gtable and OscilT pointers
    GTABLE* gtable;
    OSCILT* oscil;

    //Variables for Buffer calculations and loop settup
    unsigned long nbufs, outframes, nframes, remainder;
    int i, j, k;
    double val;

    //pointer to tick functions
    typedef double (*tickfunc) (OSCILT*, double);
    tickfunc tick = tabitickfmod;

    //BRKpoint variables
    FILE*   ampFile = NULL;
    BRKSTREAM* ampstream = NULL;
    unsigned long ampSize = 0;
    double ampMax, ampMin;

    FILE*   freqFile = NULL;
    BRKSTREAM* freqstream = NULL;
    unsigned long freqSize = 0;
    double freqMax, freqMin;


    // init all rescource vars to default states
    int ifd = -1,ofd = -1;
    int error = 0;
    PSF_CHPEAK* peaks = NULL;
    float* outframe = NULL;

    // Clock variable
    clock_t starttime, endtime;

    printf("tabgen: generates an audio file using a lookup table based on selcted wave type with optional breakpoint files for frequency and amplitude \n");


  	/* process any optional flags: remove this block if none used! */
	if(argc > 1){
		char flag;
		while(argv[1][0] == '-'){
			flag = argv[1][1];
			switch(flag){
			/*TODO: handle any  flag arguments here */
            case('p'):
                argv[1]+=2;
                phase = atof(argv[1]);
            case('t'):
                tick = tabtickfmod;
            break;
            case('w'):
                argv[1]+=2;
                tabwidth = atoi(argv[1]);
            break;
			case('\0'):
				printf("Error: missing flag name\n"
                       "Current Val = %s\n", argv[1]);
				return 1;
			default:
				break;
			}
			argc--;
			argv++;
		}
	}


    if(argc < ARG_NARGS) {
        printf("insufficient arguments.\n"
                "usage:\ntablegen [-pN] [-wN] [-t] outfile dur srate nchans amp freq type nharms\n"
                "-pN: set the initial phase of the waveform (0 - 1)"
                "-wN: set width of lookup table N points (default: 1024)\n"
                "-t: use truncating lookup (default: interpolating lookup)\n"
                );
                return 1;
    }


    duration = atof(argv[ARG_DURATION]);
    srate = atoi(argv[ARG_SAMPLERATE]);
    nchans = atoi(argv[ARG_CHANS]);
    wave = atoi(argv[ARG_WAVE]);
    nharms = atoi(argv[ARG_NHARMS]);

    if(tabwidth < 1){
        printf("Error: Table width must be a positive integer\n"
                "Recommended values are large powers of two, default is 1024\n");
        error ++;
        goto exit;
    }
    if(tabwidth < 128){
        printf("Warning, tabwidth of %lu is small, output distortion will be noticable\n"
                "Recommended values are large powers of two; default is 1024\n", tabwidth);
    }

    if(duration <= 0){
        printf("Error: duration argument must be positive\n");
        return 1;
    }

    if(!(srate == 44100 || srate == 48000)){
        printf("Error: sample rate of %s is not supported\nSupported sample rates are 44100 and 48000\n", argv[ARG_SAMPLERATE]);
                return 1;
    }

    if(nchans < 1 || nchans > 2){
        printf("Error: nchans value %d is out of range. Only mono and stereo supported (1 or 2)\n", nchans);
        error++;
        goto exit;
    }


    if(wave < 0 || wave > 5){
        printf("Error: wave value of %d is out of range\n"
               "Wave options are...\n"
               "0: Sine Wave\n"
               "1: Triangle Wave\n"
               "2: Square wavve\n"
               "3: Sawtooth Up Wave\n"
               "4: Sawtooth Down Wave\n"
               "5: Pulse Wave\n", wave);
        error++;
        goto exit;
    }

    if(nharms < 1){
        printf("Error: nharms must be a positive integer\n");
        error++;
        goto exit;
    }

    //check for breakpoint files
    ampFile = fopen(argv[ARG_AMP], "r");
    if(ampFile == NULL){
        amp = atof(argv[ARG_AMP]);

        if(amp <= 0 || amp > 1){
        printf("Error: amplitude value %f must be between 0 and 1\n", amp);
        error++;
        goto exit;
        }
    }else{
        ampstream = bps_newstream(ampFile, srate, &ampSize);
        if(ampstream == NULL){
            printf("Error initializing amplitude breakpoint stream");
            error++;
            goto exit;
        }
        if(bps_getminmax(ampstream, &ampMin, &ampMax)){
            printf("Error getting min and max breakpoints from file %s\n", argv[ARG_AMP]);
            error++;
            goto exit;
        }
        printf("Amp Min = %f, Amp Max = %f\n", ampMin, ampMax);
        //make sure min and max values are in range
        if(ampMin > 1 || ampMin < 0 || ampMax > 1 || ampMax < 0){
            printf("Error: Breakpoint values out of range in file %s, values must be between 0 and 1\n", argv[ARG_AMP]);
            error ++;
            goto exit;
        }
    }

    freqFile = fopen(argv[ARG_FREQ], "r");
    if(freqFile == NULL){
        freq = atof(argv[ARG_FREQ]);
        if(freq <= 0){
            printf("Error: freq argument must be positive\n");
           return 1;
        }
    }else{
        freqstream = bps_newstream(freqFile, srate, &freqSize);
        if(freqstream == NULL){
            printf("Error reading frequency breakpoint file %s\n", argv[ARG_FREQ]);
            error++;
            goto exit;
        }
        if(bps_getminmax(freqstream, &freqMin, &freqMax)){
            printf("Error getting min and max breakpoints from file %s\n", argv[ARG_FREQ]);
            error++;
            goto exit;
        }
        printf("Freq Min = %f, Freq Max = %f\n", freqMin, freqMax);
        //make sure min and max values are in range
        if(freqMin <= 0 || freqMax > 20000 ){
            printf("Error: Breakpoint values out of range in file %s, values must be between 0 and 20000\n", argv[ARG_FREQ]);
            error ++;
            goto exit;
        }
    }


    
    switch(wave){
        case(0):
            gtable = new_sine(tabwidth);
            break;
        case(1):
            gtable = new_triangle(tabwidth, nharms);
            break;
        case(2):
            gtable = new_square(tabwidth, nharms);
            break;
        case(3):
            gtable = new_saw(tabwidth, nharms, SAW_UP);
            break;
        case(4):
            gtable = new_saw(tabwidth, nharms, SAW_DOWN);
            break;
        case(5):
            gtable = new_pulse(tabwidth, nharms);
            break;
    }

    if(gtable == NULL){
        printf("Error creating wavetable\n");
        error++;
        goto exit;
    }

    //create the oscilator with gtable created above
    oscil = new_oscilp(srate, gtable, phase);
    if(oscil == NULL){
        printf("Error creating oscillator\n");
        error++;
        goto exit;
    }

    if(psf_init()){
        printf("unable to start portsf\n");
        return 1;
    }


    //settup parameters for outfile
    //sets mono 16 bit format
    outprops.chans = nchans;
    outprops.srate = srate;
    outprops.samptype = (psf_stype) PSF_SAMP_16;
    outprops.chformat = STDWAVE;

    outformat = psf_getFormatExt(argv[ARG_OUTFILE]);

    if(outformat == PSF_FMT_UNKNOWN){
        printf("Error: unknown file format for %s", argv[ARG_OUTFILE]);
        error++;
        goto exit;
    }

    outprops.format = outformat;


    ofd = psf_sndCreate(argv[ARG_OUTFILE], &outprops, 0, 0, PSF_CREATE_RDWR);
    if(ofd < 0){
        printf("Error: unable to create outfile %s\n", argv[ARG_OUTFILE]);
        error++;
        goto exit;
    }
    
    //Initalize nframes and calculate the required number of buffers
    nframes = BUFFERSIZE;

    //get the needed number of frames based on srate and duration
    outframes = (unsigned long) (duration * srate + 0.5);
    nbufs = outframes / nframes;

    //get the remaining number of samples 
    remainder = outframes - nbufs * nframes;
    //if nbufs is not a perfect integer division of outframes, add another buffer to calculate remainder
    if(remainder > 0){
        nbufs++;
    }



    //allocate space for buffer size frame
    outframe = (float *) malloc (outprops.chans * sizeof(float) * BUFFERSIZE);
    if(outframe==NULL){
        puts("No memory!\n");
        error++;
        goto exit;
    }
    //and allocate space for PEAK info
    peaks = (PSF_CHPEAK*) malloc(outprops.chans * sizeof(PSF_CHPEAK));
    if(peaks == NULL){
        puts("No memory!\n");
        error ++;
        goto exit;
    }


    printf("copying.....\n");
    starttime = clock();

    //Processing Loop
    for(i = 0; i < nbufs; i++){
        if(i == nbufs-1)
            nframes = remainder;
        for(j=0; j < nframes; j++){

            if(ampstream)
                amp = bps_tick(ampstream);
            if(freqstream)
                freq = bps_tick(freqstream);
            
            val = tick(oscil, freq) * amp;
            for(k = 0; k < nchans; k++){
                outframe[(j*nchans) + k] = val;
            }
        }
            if(psf_sndWriteFloatFrames(ofd, outframe, nframes) != nframes){
            printf("Error writing to outfile\n");
            error++;
            break;
        }
        
    }
    endtime = clock();
 
    printf("Done. %ld sample frames copied to %s\n", outframes, argv[ARG_OUTFILE]);

    printf("Elapsed Time: %f\n", (endtime - starttime)/(double) CLOCKS_PER_SEC);

//REPORT PEAK VALUES TO USER
if(psf_sndReadPeaks(ofd, peaks, NULL) > 0){
    long i;
    double peaktime;
    printf("PEAK information:\n");
    for(i=0; i < outprops.chans; i++){
        peaktime = (double) peaks[i].pos / outprops.srate;
        printf("CH %ld:\t%.4f at %.4f secs\n", i+1, peaks[i].val, peaktime);
    }
}
// do all cleanup
exit:
if(ofd >= 0)
    psf_sndClose(ofd);
if(outframe)
    free(outframe);
if(peaks)
    free(peaks);
if(gtable)
    gtable_free(&gtable);
if(oscil)
    free(oscil);
if(ampstream){
    bps_freepoints(ampstream);
    free(ampstream);
}
if(ampFile){
    if(fclose(ampFile))
        printf("Error closing amplitude breakpoint file %s", argv[ARG_AMP]);
}
if(freqstream){
    bps_freepoints(freqstream);
    free(freqstream);
}
if(freqFile){
    if(fclose(freqFile))
        printf("Error closing amplitude breakpoint file %s", argv[ARG_FREQ]);
}
psf_finish();
return error;
}
