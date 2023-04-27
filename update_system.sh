#!/bin/sh
set -xeu

NETBSD_IP=192.168.122.211

./build.sh -U -O ./obj -j$(nproc) -m i386 -a i386 -u kernel=GENERIC
./build.sh -U -O ./obj -j$(nproc) -m i386 -a i386 -u kernel=LOCKDOC
scp etc/rc root@$NETBSD_IP:/etc/rc
scp etc/etc.i386/boot.cfg root@$NETBSD_IP:/boot.cfg
scp obj/sys/arch/i386/compile/GENERIC/netbsd root@$NETBSD_IP:/netbsd.orig
scp obj/sys/arch/i386/compile/LOCKDOC/netbsd root@$NETBSD_IP:/netbsd
