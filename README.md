# canary
Tests things for minors

## Building
To build run ```make``` and binaries will be generated in ```build/bin/```. A debug build with more logging options and less optimizations can be build with ```make debug``` and the binaries will be generated in ```build/dbg/bin```.

If you make changes you can test your changes against a test suite with ```make test```. I also report times with ```make time``` and these can be compared with the performance log in ```perf.log```.

## Running
After building the canary executable is in ```build/bin/canary```. All graphs must be in the graph6 format (I do not currently support sparse6) and I have provided my own implementation of ```showg``` as well. The arguments required can be seen by running the command without any arguments. Currently the option parsing is a little bit crude, and the options must be specified in the same order listed.
