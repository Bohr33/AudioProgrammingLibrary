#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <portsf.h>
#include <math.h>
#include "wave.h"
#include "breakpoints.h"


enum {ARG_PROGNAME, ARG_FREQ, ARG_DURATION, ARG_AMP, ARG_WAVEFORM, ARG_SAMPLERATE, ARG_CHANS, ARG_OUTFILE, ARG_PWM, ARG_MOD, ARG_NARGS};

enum {WAVE_SINE, WAVE_TRIANGLE, WAVE_SQUARE, WAVE_SAWUP, WAVE_SAWDOWN, WAVE_PWM, WAVE_NTYPES};
#define BUFFERSIZE 256

int main(int argc, char** argv){

    PSF_PROPS outprops;
    psf_format outformat = PSF_FMT_UNKNOWN;
    long framesread, totalread;

    OSCIL* oscil;
    int srate;
    double freq, duration, amp, ampOut;
    int chans;
    float val;

    OSCIL* amp_oscil;
    double mod_freq = 0;
    double mod_val;
    double mod_depth = .4;


    //PWM
    float pwm;
    //Define PWM Breakpoint variables
    BRKSTREAM* pwmstream = NULL;
    FILE* brk_pwm = NULL;
    unsigned long brkpwmSize = 0;
    double minpwm, maxpwm;

    unsigned long nbufs, outframes, remainder, nframes;
    int i, j, k, out_j;
    int waveform;
    //define our pointer to a function
    tickfunc tick;

    //define variables for amplitude breakpoint
    BRKSTREAM* ampstream = NULL;
    FILE* brk_amp = NULL;
    unsigned long brkampSize = 0;
    double minval, maxval;

    //define variables for frequency breakpoint
    BRKSTREAM* freqstream = NULL;
    FILE* brk_freq = NULL;
    unsigned long brkfreqsize = 0;
    double minfreq, maxfreq;


    // init all rescource vars to default states
    int ifd = -1,ofd = -1;
    int error = 0;
    PSF_CHPEAK* peaks = NULL;
    float* frame = NULL;

    printf("sinGenV2: creates an output audio file based on a given frequency, duration, and waveshape, with optional support for breakpoint files for amplitude and frequency.\n Version 2 adds support for PWM modulation\n");


    //Check for minimum number of arguments
    if(argc < ARG_NARGS - 2) {
        printf("insufficient arguments.\n"
                "usage:\nsinGen freq duration amp waveform(0-5) samplerate chans outfile [PWM 0 - 1] [F]\n");
                printf("1: Sine\n2:Triangle\n3:Square\n4:SawUp\n5:SawDown\n6:PWM\n");
                printf("[-tF] is an optional argument for amplitude modulation control, where the value given is the frequency\n");
                return 1;
    }
   
    //Convert and check essential variables
    duration = atof(argv[ARG_DURATION]);    
    srate = atoi(argv[ARG_SAMPLERATE]);
    waveform = atoi(argv[ARG_WAVEFORM]);
    chans = atoi(argv[ARG_CHANS]);
    
    if(duration <= 0){
        printf("Error: duration argument must be positive\n");
        return 1;
    }

    if(!(srate == 44100 || srate == 48000)){
        printf("Error: sample rate of %s is not supported\nSupported sample rates are 44100 and 48000\n", argv[ARG_SAMPLERATE]);
                return 1;
    }

    if((waveform < 0) || (waveform > WAVE_NTYPES)){
        printf("Error: waveform value is out of range. Please select a value between 0 and 4");
        return 1;
    }

    if(chans < 0 || chans > 2){
        printf("Error: chans must only be 2 or 1\n");
        error++;
        goto exit;
    }


    //check for optional arguments
    if(argc > ARG_NARGS - 2){
        if(waveform == 5) {
                if(argc < ARG_NARGS - 2){
                    printf("Error: Insufficient arguments;\nPWM waveform requires additional PWM argument [0 - 1]\n");
                    error++;
                    goto exit;
                }

                brk_pwm = fopen(argv[ARG_PWM], "r");
                if(brk_pwm == NULL){
                    pwm = atof(argv[ARG_PWM]);

                    if(pwm > .99 || pwm < 0.01){
                        printf("Error: PWM value is out of range 0 - 1\n");
                        error++;
                        goto exit;
                    }
                }else{
                    pwmstream = bps_newstream(brk_pwm, srate, &brkpwmSize);

                    if(bps_getminmax(pwmstream, &minpwm, &maxpwm)){
                        printf("Error reading min and max values from PWM breakpoint file\n");
                        error++;
                        goto exit;
                    }

                    printf("PWM Data: min value = %f, max value = %f\n", minpwm, maxpwm);

                    if(minpwm < 0.01 || maxpwm > .99){
                        printf("Error: pwm breakpoint values are out of range in file %s. Must be between 0 - 1\n", argv[ARG_PWM]);
                        error++;
                        goto exit;
                    }
                }
                //check for another optional argument after pwm choice
                if(argc > ARG_NARGS - 1){
                    mod_freq = atof(argv[argc - 1]);

                    if(mod_freq <= 0){
                        printf("Error: mod frequency must be positive\n");
                        error++;
                        goto exit;
                    }
                }

            //if there are more arguments, without PWM selected, save value as mod freq
            }else{
                mod_freq = atof(argv[argc - 1]);

                if(mod_freq <= 0){
                    printf("Error: mod frequency must be positive\n");
                    error++;
                    goto exit;
                }
            }

    }
 



    if(psf_init()){
        printf("unable to start portsf\n");
        return 1;
    }

    brk_freq = fopen(argv[ARG_FREQ], "r");
    if(brk_freq == NULL){
        freq = atof(argv[ARG_FREQ]);
        if(freq > 20000 || freq < 20){
            printf("Error: Frequency value %f, is out of range\nAverage audible hearing range is 20hz - 20,000hz\n", freq);
            error++;
            goto exit;
        }
    }else{
        freqstream = bps_newstream(brk_freq, srate, &brkfreqsize);
        
        if(bps_getminmax(freqstream, &minfreq, &maxfreq)){
            printf("Error reading min and max values from breakpoint file %s\n", argv[ARG_FREQ]);
            error++;
            goto exit;
        }
        printf("Min value = %f, Max Value = %f\n", minfreq, maxfreq);
          if(minfreq > 20000 || minfreq < 20 || maxfreq > 20000 || maxfreq < 20){
            printf("Error: Breakpoint values out of range in file %s, values must be between 0 and 1\n", argv[ARG_FREQ]);
            error ++;
            goto exit;
        }
    }


    //open amp breakpoint file
    brk_amp = fopen(argv[ARG_AMP], "r");
    if(brk_amp == NULL){
        amp = atof(argv[ARG_AMP]);
        if(amp <= 0 || amp > 1){
            printf("Error: amplitude value %f must be between 0 and 1\n", amp);
            error++;
            goto exit;
        }
    }else{
        ampstream = bps_newstream(brk_amp, srate, &brkampSize);

        //get min and max breakpoint values;
        if(bps_getminmax(ampstream, &minval, &maxval)){
            printf("Error reading min and max values from breakpoint file\n");
            error++;
            goto exit;
        };
        printf("Min value = %f, Max Value = %f\n", minval, maxval);
        //make sure min and max values are in range
        if(minval > 1 || minval < 0 || maxval > 1 || maxval < 0){
            printf("Error: Breakpoint values out of range in file %s, values must be between 0 and 1\n", argv[ARG_AMP]);
            error ++;
            goto exit;
        }

    }


    //settup parameters for outfile
    outprops.chans = chans;
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

    switch(waveform){

        case(WAVE_SINE):
            tick = sinetick;
            break;
        case(WAVE_TRIANGLE):
            tick = tritick;
            break;
        case(WAVE_SQUARE):
            tick = sqrtick;
            break;
        case(WAVE_SAWDOWN):
            tick = sawdtick;
            break;
        case(WAVE_SAWUP):
            tick = sawutick;
            break;
    }


    ofd = psf_sndCreate(argv[ARG_OUTFILE], &outprops, 0, 0, PSF_CREATE_RDWR);
    if(ofd < 0){
        printf("Error: unable to create outfile %s\n", argv[ARG_OUTFILE]);
        error++;
        goto exit;
    }

    //settup OSCIL parameters
    oscil = new_osc(srate);

    //settup mod Osc parameters
    amp_oscil = new_osc(srate);

    nframes = BUFFERSIZE;

    //get the needed number of frames based on srate and duration
    outframes = (unsigned long) (duration * srate + 0.5);
    nbufs = outframes / nframes;

    //get the remaining number of samples 
    remainder = outframes - nbufs * nframes;
    if(remainder > 0){
        nbufs++;
    }

    printf("Nbufs = %ld\n", nbufs);
    printf("nframes = %ld\n", nframes);



    //allocate space for buffer size frame
    frame = (float *) malloc (chans * sizeof(float) * nframes);
    if(frame==NULL){
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

    //singleframe loop to do copy, report any errors
    
    totalread = 0; //running count of sample frames

    printf("Modfreq = %f\n", mod_freq);

    if(waveform != 5){ 

        for(i = 0; i < nbufs; i++){
            if(i == nbufs-1)
                nframes = remainder;
            for(j=0; j < nframes; j++){
                if(ampstream){
                    ampOut =  bps_tick(ampstream);
                }else{
                    ampOut = amp;
                }
                if(freqstream){
                    freq = bps_tick(freqstream);
                }
                if(mod_freq){
                    mod_val = sinetick(amp_oscil, mod_freq);
                    mod_val *= mod_depth;
                    mod_val += amp;
                    mod_val -= mod_depth/2;
                    ampOut *= mod_val;
                }

                val = ampOut * tick(oscil, freq);

                for(k = 0; k < chans; k++){
                    frame[j*chans + k] = (float) val;
                }
            }
                
                if(psf_sndWriteFloatFrames(ofd, frame, nframes) != nframes){
                    printf("Error writing to outfile\n");
                    error++;
                    break;
                }else
                    totalread += nframes;
        }
    }else{
        for(i = 0; i < nbufs; i++){
            if(i == nbufs-1)
                nframes = remainder;

            for(j=0; j < nframes; j++){
                if(ampstream){
                    ampOut =  bps_tick(ampstream);
                }else{
                    ampOut = amp;
                }
                if(freqstream){
                    freq = bps_tick(freqstream);
                }
                if(pwmstream){
                    pwm = bps_tick(pwmstream);
                }
                 if(mod_freq){
                    mod_val = sinetick(amp_oscil, mod_freq);
                    mod_val *= mod_depth;
                    mod_val += amp;
                    mod_val -= mod_depth/2;
                    ampOut *= mod_val;
                }

                val = ampOut * pwm_tick(oscil, freq, pwm);

                for(k = 0; k < chans; k++){
                    frame[j*chans + k] = (float) val;
                }
            }
            
            if(psf_sndWriteFloatFrames(ofd, frame, nframes) != nframes){
                printf("Error writing to outfile\n");
                error++;
                break;
            }else
                totalread += nframes;
        }

    }

    if(totalread <= 0){
        printf("Error reading infile. Outfile is incomplete\n");
        error++;
    }
    else 
    printf("Done. %ld sample frames copied to %s\n", totalread, argv[ARG_OUTFILE]);

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
//do all cleanup
exit:
if(ofd >= 0)
    psf_sndClose(ofd);
if(frame)
    free(frame);
if(peaks)
    free(peaks);
if(ampstream){
    bps_freepoints(ampstream);
    free(ampstream);
}
if(freqstream){
    bps_freepoints(freqstream);
    free(freqstream);
}
if(pwmstream){
    bps_freepoints(pwmstream);
    free(pwmstream);
}
if(brk_freq){
    if(fclose(brk_freq))
        printf("Error closing breakpoint file %s\n", argv[ARG_FREQ]);
}
if(brk_amp){
    if(fclose(brk_amp))
        printf("Error closing breakpoint file %s\n", argv[ARG_AMP]);
}
if(brk_pwm){
    if(fclose(brk_pwm))
        printf("Error closing breakpoint file %s\n", argv[ARG_PWM]);
}
psf_finish();
return error;
};
