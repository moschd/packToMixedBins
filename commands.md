# Compile instructions

## Compile to .so
Switch COMPILE_TO_SO to 1 and run the command below.
Statically link libstdc++ because make_shared requires GLIBCXX_3.4.26...

    g++ source/myjsoncpp/jsoncpp.cpp source/main.cpp -shared -ffast-math -fno-exceptions -fno-rtti -fPIC -O3 -o libPackToMixedBinsAlgo.so.1 -static-libstdc++

## Compile for local
Switch COMPILE_TO_SO to 0 and run the command below.

    g++ source/myjsoncpp/jsoncpp.cpp source/main.cpp -ffast-math -fno-exceptions -fno-rtti -O3 -o main

## Compile for code profling
    g++ -o main -pg source/myjsoncpp/jsoncpp.cpp source/main.cpp
    ./main
    gprof main > profile.txt


## Compile flags
-ffast-math  
some calculation optimization.

-fno-exceptions  
dont include exception handling.

-fno-rtti  
Disable generation of information about every class with virtual functions for use by the C++ runtime type identification features.
