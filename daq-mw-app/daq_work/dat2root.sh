#!/bin/bash
directory="/home/mrpcdaq/work/201908_HRTDC/Analyzer/data/$1"
	
if [ -e ${directory} ]	
	then
	echo "$directory Found"
else
	echo "$directory NOT Found"
fi

cd /home/mrpcdaq/work/201908_HRTDC/Analyzer

datfile=`find data/$1/*.dat`

for i in $datfile
do
	echo $i

	./bin/analysMonitor conf/analyzer.conf.default $i ${i:0:-4}.root
done

cd /home/mrpcdaq/work/201908_HRTDC/daq_work
