#!/bin/bash

fix_buff_size(){
    echo $(pwd)
    # sed -i '' 29d ./include/common.h 
    sed "28 a\ 
    #define BUFFER_SIZE ${1}
    " ./include/common.h
}

run(){
    echo "Start Build Project ${1}"
    fix_buff_size $1
    # rm -rf build/
    # mkdir build
    # cd build
    # cmake ..
    # make
    # echo "Project Build Success !"
    # echo 'Start Run SimpleDB'
    # ./SimpleDB
    # cd ..
}

for i in $(seq 6 8) 
do 
   buffer_size=$((1<<$i));
   run ${buffer_size};
done

# echo 'Start Analysis runing log'
# python analysis.py 
# echo 'All have Done !'