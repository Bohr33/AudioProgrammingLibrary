


This is a simple program that will pan a mono file across the stereo field, creating a stereo file, based on a given breakpoint file with values in range -1 to 1.

The algorithm for panning in this program is the standard constant power algorithm.

Usage: ./sfpan {infile} {outfile} {posfile.brk}

Notes: For the breakpoints, -1 represents far left, and 1 represents far right.
	- Three breakpoint files have been provided to test and examine