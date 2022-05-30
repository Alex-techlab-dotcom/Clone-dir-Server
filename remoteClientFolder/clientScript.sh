#!/bin/bash

./remoteClient -i 192.168.1.10 -p 10000 -d folder1/folder2/f3/f4/f5 > log2 &
./remoteClient -i 192.168.1.10 -p 10000 -d testFolder/subDir1 > log1 &
./remoteClient -i 192.168.1.10 -p 10000 -d testF1 > log3 &
./remoteClient -i 192.168.1.10 -p 10000 -d testF2 > log4 &
./remoteClient -i 192.168.1.10 -p 10000 -d testF3 > log5 &
./remoteClient -i 192.168.1.10 -p 10000 -d folder1/folder2/f3 > log6 &
