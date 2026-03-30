#!/bin/bash

xterm -T MznHRTdcReader1 -bg lightblue -geom 100x10+1280+0 -e tail -F  /tmp/daqmw/log.MznHRTdcReader1Comp &

xterm -T Merger -bg pink  -geom 100x10+1280+200 -e tail -F /tmp/daqmw/log.MergerComp & 
xterm -T BestEffortDispatcher -bg pink    -geom 100x10+1280+370 -e tail -F  /tmp/daqmw/log.BestEffortDispatcherComp &
xterm -T DAQLogger -bg yellow -geom 100x10+1280+540 -e tail -F /tmp/daqmw/log.DAQLoggerComp &
xterm -T Monitor -bg lightgreen -geom 100x15+1280+710 -e tail -F /tmp/daqmw/log.MonitorComp &
