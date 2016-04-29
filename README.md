# coms4112

Project 2

Felipe Rocha - flt2107
Kenneth Hungria - kah2204

This program runs builds and probes a cache-resident B-tree index. If the tree has only fanouts of 5, 9 and/or 17, then the program also probes the index using SIMD instructions.
In addition, if the built tree is 9-5-9, there is an improved hardcoded probing implementation, that loads 4 keys at a time.

## Usage
Running the program is rather simple. There is a MakeFile that will automatically build and will produce a build executable. Simply type "make" into the terminal. Make sure that you are inside the directory where all the files are. Once the "make" command is done, the code should compile and output the build executable. This can be ran in by "./build 10 10 2 3 4".

## Output
The tree is sorted and the format for showing the probes value and identifier is "Probe (PROBE) -> rangeid (IDENTIFIER)". This should show the user where the value will be on the table. 

If additional probing implementations are enabled (as described on the beginning of this document), then each additional algorithm will also output its results.

## Additional Info
In some clusters, as CLIC, time measurements might output zero or the program freezes. If the program freezes, the user might use "make clic" instead: this will compile the program without any time measurements, so it can be tested on such machines.