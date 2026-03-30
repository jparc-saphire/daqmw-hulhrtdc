#!/bin/bash
echo "This PC (USB-LAN) 192.168.10.2  "
#echo "This PC (PCI)     192.168.10.16 "
echo "NIMEASIROC1       192.168.10.16 "
echo "NIMEASIROC2       192.168.10.10"
echo "HUL-Scaler        192.168.10.21 "
echo "HUL-MHTDC         192.168.10.22 "
echo "MZN-HRTDC         192.168.10.23 "
echo "Drs4Qdc1          192.168.10.50 "
echo "Drs4Qdc2          192.168.10.51 "
echo "prescaler1        192.168.10.59 "
echo "prescaler2        192.168.10.53 "

echo "  "
echo "  "

echo "connected IP list within 192.168.10.{1..254}: "
echo "  "
echo 192.168.10.{1..254} | xargs -P256 -n1 ping -s1 -c1 -W1 | grep --color=auto 'ttl'
#echo 192.168.11.{1..254} | xargs -P256 -n1 ping -s1 -c1 -W1 | grep --color=auto 'ttl'
echo "  "
echo "connected IP list within 192.168.11.{1..254}: "
echo "  "
echo 192.168.11.{1..254} | xargs -P256 -n1 ping -s1 -c1 -W1 | grep --color=auto 'ttl'
