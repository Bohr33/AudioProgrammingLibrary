

sigGenV2: generates an output file based on arguments defining the waveform type, frequency, amplitude, channels, and duration selected. Additional arguments for PWM control (when square is selected) as well as an optional argument for amplitude modulation

Usage: ./sigGen freq duration Amp waveform samplerate channels outfile.ext [pwm] [ampModFreq]

Freq = value between 0 and 20000, or breakpoint file
Duration = any positive value
Amp = value between 0 - 1 or breakpoint file
Waveform:
	0: Sin	1:Triangle  2: Square  3: SawUP  4: SawDown

Sample Rate: Either 44,100 or 48,000

Channels: 1 or 2 (sorry multichannel people)

Outfile.ext: a given file name with an extension of either .aif or .wav

[PWM]: optional pwm argument. Can be a single value or a breakpoint file. All values must be in range 	0.00 - 0.99

{ampModFreq]: optional amplitude modulation control. Must be in range 0 - 20000. Modulation index is always 1.


Implementation: The waveforms are generated mathematical based on their shape, as opposed to the alias conscious version of generating each waveform by their harmonic, thus they are harsh compared to true implementations.


Recommend Exercise: Try combinations of using break point files for all arguments with break point file support. You can get some cool effects by really mangling the program. Some example breakpoint files are provided to test this.
