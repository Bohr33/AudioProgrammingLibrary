#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sndfile.h> 


//This program is designed to turn a Stereo file into a mono file. There will be three options to choose from 1. Summing L and R channels, 2. L only, 3. R only
//

int main(int argc, char **argv) {

    SNDFILE *inFile = NULL, *outFile = NULL; //pointers to a sound files
    SF_INFO sfInfo; //hold an information about a sound file
    SF_INFO sfMonoInfo;
    float *stereoBuffer, *monoBuffer; //Buffer for holding samples
    int mode;


    if(argc != 4){
        printf("Error: invalid number of arguments, expect 3\nUsage: ./a.out \"infile\" \"outfile\" Mode\n");
        return 1;
    }

    mode = atoi(argv[3]);
    printf("%d\n", mode);

    if(!(mode == 1 || mode == 2 || mode == 3)){
        printf("Error: Mode must equal 1, 2, 3\n");
        printf("Mode 1: Sum left and right channels\n");
        printf("Mode 2: Return only left channel\n");
        printf("Mode 3: Return only right channel\n");
        return 1;
    }

    //Initialize SF_INFO with 0s (memset is in string.h library)
    memset(&sfInfo, 0, sizeof(SF_INFO));
    memset(&sfMonoInfo, 0, sizeof(SF_INFO));

    //Open the original sound file as read mode
    inFile = sf_open(argv[1], SFM_READ, &sfInfo);
    if(!inFile){
    printf ("Error : could not open file : %s\n", argv[1]);
    puts (sf_strerror (NULL));
    return 1;
    }

    //Copy soundfile data from read file, but change the number of channels to mono
    sfMonoInfo = sfInfo;
    sfMonoInfo.channels = 1;

    //Check if the file format is in good shape
    if(!sf_format_check(&sfInfo)){	
    sf_close(inFile);
    printf("Invalid encoding\n");
    return 1;
    }

    //Allocate enough memory space to hold all samples from the input file
    stereoBuffer = (float *) malloc((sfInfo.frames * sfInfo.channels) * sizeof(float));
    monoBuffer = (float *) malloc((sfInfo.frames) * sizeof(float));

    //Copy samples from the original file to the StereoBuffer
    sf_count_t readcount = sf_read_float(inFile, stereoBuffer, sfInfo.frames * sfInfo.channels);

    printf("Frame count: %lld\nSamples read: %lld\n", sfInfo.frames, readcount);

  
    for(int i = 0; i < readcount/2; i++){
        if(mode == 1){
          monoBuffer[i] = (stereoBuffer[2 * i] + stereoBuffer[(2 * i) + 1])/2;
        }else if(mode == 2){
          monoBuffer[i] = stereoBuffer[2 * i];
        }else if(mode == 3){
          monoBuffer[i] = stereoBuffer[2 * i + 1];
        }
    }

    //Open another sound file in write mode
    outFile = sf_open(argv[2], SFM_WRITE, &sfMonoInfo);

    //Check if the file was succefully opened
    if(!outFile){	
    printf ("Error : could not open file : %s", argv[2]);
    puts (sf_strerror(NULL));
    return 1;
    }

    //Write samples to a new file
    sf_write_float(outFile, monoBuffer, readcount/2); 

    //Close the open files
    sf_close(inFile);
    sf_close(outFile);

    return 0;
}
