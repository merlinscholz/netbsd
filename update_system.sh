#!/bin/sh
set -xeu

NETBSD_IP=10.0.20.246

./build.sh -U -O ./obj -j$(nproc) -m i386 -a i386 -u kernel=LOCKDOC
scp etc/rc root@$NETBSD_IP:/etc/rc
scp obj/sys/arch/i386/compile/LOCKDOC/netbsd root@$NETBSD_IP:/netbsd
