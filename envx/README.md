
envx - creates an output breakpoint file by reading the amplitude envelope of an inputted file.

Usage - ./envx -wN {audio file to read} {name of break point file to create}

    - -wN is an optional flag that will set the size of the window that is used to extract amplitude points from the file. Default is 15.
    
    Notes: - only works with mono file souces
           - uses portsf audio library for handling audio files
           
    Tips: - a drum loop has been provided to test this program. Try applying this with the sfenv program to apply the drum envlope to another audio file
