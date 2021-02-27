#!/bin/sh

BCKPROUTE=/tmp/onvpnconfig.route.bckp

case $1 in

start)
	insmod /lib/modules/`uname -r`/kernel/drivers/net/tun.ko
	ip ro | grep default | tr ' ' '\n' | head -5 | xargs -n5 > $BCKPROUTE	
	ip ro del default
;;

running)
	(
		while [ -z "`ip l | grep tun0`" ] ; do sleep 1 ; done
		ip ro add default dev tun0
	) &
;;

stop)
	ip ro del default dev tun0
	if [ -s $BCKPROUTE ]; then
		ip ro add `cat $BCKPROUTE`
	fi
	rmmod /lib/modules/`uname -r`/kernel/drivers/net/tun.ko
;;

esac