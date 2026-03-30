#!/bin/bash
IP=192.168.10.16
RUN=$1
NUM=$2
./bin/daq $IP $RUN $NUM
#./bin/decoder $RUN