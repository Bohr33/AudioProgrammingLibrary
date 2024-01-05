

Table interp: Generates an audio file of a selected waveform using the wavetable lookup method. There are optional arguments for phase, truncated table lookup, and for the size of the table generated. 

Usage: ./tabgen [-pN] [-wN] [-t] outfile.ext duration sampleRate #ofChannels amplitude frequency
Cont... waveform #ofHarmonics

[-pN] - an optional argument to set the initial phase of the waveform. Range is 0 to 1. Default: 0

[-wN] - an optional argument to set the size of the created lookup table. Default: 1024

[-t] - optional flag to set the table lookup mode as truncated. Default is Interpolated

Outfile.ext - name of output file with extension of ".wav" or ".aif"

Duration - sets duration of file to create. Can be any positive value

Sample Rate - Sets the sample rate of the output file. Either 48,000 or 44,100

#ofChannels - Sets the output file to mono or stereo. Cannot be greater than 2.

Amplitude - sets the amplitude of the resulting synthesized wave. Optional Breakpoint file

Frequency - sets the frequency of the resulting wave. Optional Breakpoint file

Waveform - sets the type of waveform to create.
	0: Sine Wave
	1: Triangle Wave
	2: Square Wave
	3: Saw Up Wave
	4: Saw Down Wave
	5: Pulse Wave

# of harmonics: Sets how many harmonics to create for waveforms that have more than 1 harmonic (not sine). This can be any positive integer, however excessive values will lead to aliasing.


Implementation: This program synthesizes waveforms based on the table lookup method. Where the program sigGen synthesizes waveforms based on mathematical calculations, this program instead creates a single table, calculated by its corresponding harmonic series, and then reads through it to synthesize the output. This is the more efficient method and is considered the standard for oscillator implementation.

NOtE: This program can compare truncated table look up versus interpolated lookup methods. The interpolated method creates a less distorted output, however if the size of the created table is large enough, the difference will be minimal.

Exercise: I recommend trying smaller and smaller wavetable sizes, with the other parameters being the same, to see how this leads to distortion. Surprisingly it will still work with a table size of 3, as long as you don't mind terrible audio quality.