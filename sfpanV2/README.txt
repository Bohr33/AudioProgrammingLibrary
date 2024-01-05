


SfpanV2: This is a simple panning program that takes a mono file input, a file output name, and a 	panning position or breakpoint file to create a panned stereo file. It has an additional 	optional argument to selected 1 of 2 panning modes. The default is Constant power panning, 	which is recommended, the other is a linear panning method. A default sine wav file is 		included for basic testing.

Note: pan value and pan value breakpoints must be in range -1.0 to 1.0


Usage: ./sfpan [-s] infile outfile panValue/breakpointfile.txt


Recommend Exercise: Take an input file and test the program with the same breakpoint file, but use 	the optional flag to test both panning methods. If you open the results up with an audio 	editor (I recommend Audacity) you can clearly see how the methods differ.