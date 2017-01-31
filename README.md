# Shell #

This is a simple shell written in C. It supports redirection of input and output, piping, and basic job control such as bringing the most recent background process to the foreground and continuing the most recently stopped background process.

## Dependencies ##
GCC and Make are used to compile the shell:

    apt-get install build-essential

## Build ##

To compile:
    
    make
    
To delete the compilation files:

    make clean

Start the shell:

    ./shell

## Examples ##

### Redirection ###

Redirect information about your cpu into the standard input of `cat` and write it to a local file `cpuinfo`

    $ cat < /proc/cpuinfo > cpuinfo
    $ cat cpuinfo
    
### Piping ###

This shell supports two stage piping. For example, to search for `shell` amongst your running processes:

    $ ps -cx | grep shell

### Job Control ###

#### Start a job in the background ####

    $ sleep 10 &

#### Bring background job to the foreground ####

    $ sleep 100 &
    Running: sleep 100 
    $ fg

#### Continue a job in the background ####

    $ sleep 10
    ^Z
    Stopped: sleep 10
    $ bg
    Running: sleep 10
