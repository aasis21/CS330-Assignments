#!/bin/bash
gem5="/home/aasis21/gem5"
echo "+++++++Making the latest binary+++++++"
cd $gem5/gemos/binaries && make
if [[ $? == 0 ]];then
    echo "+++++++++++++++++++++++++++++++++++++++"
    export M5_PATH=$gem5/gemos
    $gem5/build/X86/gem5.opt $gem5/configs/example/fs.py
fi


