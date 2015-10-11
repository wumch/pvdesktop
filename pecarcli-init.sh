#!/bin/sh

if [ "x$(cat /proc/sys/net/ipv4/ip_forward)" != "x1" ]; then
	echo 1 > /proc/sys/net/ipv4/ip_forward
fi

OUTIF=`/sbin/ip route show to exact 0/0 | sed -r 's/.*dev\s+(\S+).*/\1/'`
iptables -t nat -A POSTROUTING -s 10.0.0.0/8 -o ${OUTIF} -j MASQUERADE
iptables --table nat --append POSTROUTING --out-interface $OUTIF --jump MASQUERADE

# = tunctl -n -t tun1
ip tuntap add dev tun1 mode tun use wumch group wumch

ifconfig tun1 10.0.0.3 dstaddr 10.0.0.4 mtu 1400 up

