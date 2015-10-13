#!/bin/sh

# compile:
# clang++ -ferror-limit=2 -pthread src/pecarcli.cpp src/Channel.cpp src/Config.cpp src/Facade.cpp -o /tmp/pecarcli -lboost_system -lboost_filesystem -lboost_program_options -lcrypto++ -lssl

# /etc/sysctl.conf :
# net.ipv4.ip_forward=1
# net.ipv6.conf.all.forwarding=1

if [ "x$(cat /proc/sys/net/ipv4/ip_forward)" != "x1" ]; then
	echo 1 > /proc/sys/net/ipv4/ip_forward
fi

TUNNAME="tun1"

#OUTIF=`/sbin/ip route show to exact 0/0 | sed -r 's/.*dev\s+(\S+).*/\1/'`
#iptables -t nat -A POSTROUTING -s 10.0.0.0/8 -o ${OUTIF} -j MASQUERADE
#iptables --table nat --append POSTROUTING --out-interface $OUTIF --jump MASQUERADE

# = tunctl -n -t tun1
ip tuntap add dev ${TUNNAME} mode tun use wumch group wumch

ifconfig ${TUNNAME} 10.0.0.3 dstaddr 10.0.0.4 mtu 1400 up

sudo route add 222.161.220.33/32 dev ${TUNNAME}
sudo route add 112.90.51.173/32 dev ${TUNNAME}

