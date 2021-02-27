#!/bin/sh

ROOTDIR="/home/root"
LIBDIR=${ROOTDIR}/"Lib"
BINDIR=${ROOTDIR}/"Bin"
CONFDIR=${ROOTDIR}/"Conf"

MAINBIN=${BINDIR}/"qtcore"
NETCFG=${CONFDIR}/"static-net.cfg"

IFACE="eth0"
TUNL="tun0"

do_cmd() {
	echo "$1"
	$1
}

if [ ! -f ${NETCFG} ]; then
	echo "${NETCFG} not found"
	exit 1
fi

static_ip=`head -1 ${NETCFG} | tail -1`
static_mask=`head -2 ${NETCFG} | tail -1`
port=`head -3 ${NETCFG} | tail -1`

case "$1" in

start)
	if [ ! -x ${MAINBIN} ]; then
		echo "${MAINBIN} exec not found"
		exit 1
	fi

	# allow ssh from static network
	do_cmd "iptables -A INPUT -p tcp -s ${static_ip}/${static_mask} --dport 22 -j ACCEPT"
	# block ssh from other
	do_cmd "iptables -A INPUT -p tcp --dport 22 -j REJECT"
	# route tcp:80 from ethernet to static ip
	do_cmd "iptables -A PREROUTING -t nat -i ${IFACE} -j DNAT -p tcp --dport 80 --to-destination ${static_ip}:${port}"
	# block tcp:80 from tunnel
	do_cmd "iptables -A INPUT -j REJECT -p tcp --dport 80"

	# start main programm
	export LD_LIBRARY_PATH=${LIBDIR}:/usr/local/lib
	do_cmd "start-stop-daemon -S -b -x ${MAINBIN} &> /dev/null"

;;

stop)

	do_cmd "start-stop-daemon -K -x ${MAINBIN} &> /dev/null"

	do_cmd "iptables -D INPUT -j REJECT -p tcp --dport 80"
	do_cmd "iptables -D PREROUTING -t nat -i ${IFACE} -j DNAT -p tcp --dport 80 --to-destination ${static_ip}:${port}"	
	do_cmd "iptables -D INPUT -p tcp --dport 22 -j REJECT"
	do_cmd "iptables -D INPUT -p tcp -s ${static_ip}/${static_mask} --dport 22 -j ACCEPT"

;;

esac