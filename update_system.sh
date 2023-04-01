#!/bin/sh
set -xeu

NETBSD_IP=192.168.122.211

./build.sh -U -O ./obj -j$(nproc) -m i386 -a i386 -u kernel=LOCKDOC
scp etc/rc root@$NETBSD_IP:/etc/rc
scp obj/sys/arch/i386/compile/LOCKDOC/netbsd root@$NETBSD_IP:/netbsd
