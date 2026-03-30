#!/bin/bash
./kill_all.sh
sleep 1
./show_logs.sh
echo ""
echo "USE Ctrl+C to stop"
echo ""
sleep 1
#run.py -cl config_all.xml 

xterm -T Operator  -geom 95x30+10+0  -e 'run.py -cl config_all.xml' &
