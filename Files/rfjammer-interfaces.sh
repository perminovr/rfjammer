#!/bin/sh

ROOTDIR="/home/root"
CONFDIR=${ROOTDIR}/"Conf"
NETCFG=${CONFDIR}/"static-net.cfg"

touch /run/interfaces
[ ! -f /run/interfaces ] && exit 1

if [ ! -L /etc/network/interfaces ]; then
	rm /etc/network/interfaces
	ln -sf /run/interfaces /etc/network/interfaces
fi

sip=`head -1 ${NETCFG} | tail -1`
mask=`head -2 ${NETCFG} | tail -1`

cat << EOF > /run/interfaces
auto lo
iface lo inet loopback

auto eth0
iface eth0 inet static
	address $sip
	netmask $mask
EOF

exit 0