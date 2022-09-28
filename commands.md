# Compile instructions

## Compile to .so
Switch COMPILE_TO_SO to 1 and run the command below.

    g++ source/main.cpp -shared -ffast-math -fno-exceptions -fno-rtti -fPIC -O3 -o libPackToBinAlgo.so.0 -l jsoncpp

## Compile for local
Switch COMPILE_TO_SO to 0 and run the command below.

    g++ source/main.cpp -ffast-math -fno-exceptions -fno-rtti -O3 -o main -l jsoncpp

## Compile for code profling
    g++ -o first -pg first.cpp -pg -l jsoncpp
    ./first
    gprof first > profile.txt


## Compile flags
-ffast-math  
some calculation optimization.

-fno-exceptions  
dont include exception handling.

-fno-rtti  
Disable generation of information about every class with virtual functions for use by the C++ runtime type identification features.
