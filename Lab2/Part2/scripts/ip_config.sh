#!/bin/bash
#
#
# Network Diagram
#
# y -> Bancada (meu caso 1)
# 
#           .254┌─────┐.235
#             ┌─┤tuxy4├─┐
#             │ └─────┘ │
#           .1│         │.1
#        ┌────┴┐       ┌┴────┐
#        │tuxy1│       │tuxy2│
#        └─────┘       └─────┘
# 172.16.y0.0/24       172.16.y1.0/24

read -e -p "Bancada? > " -i "1" Y

printf "\n\n          .254┌─────┐.235\n            ┌─┤tux%s4├─┐\n            │ └─────┘ │\n          .1│         │.1\n       ┌────┴┐       ┌┴────┐\n       │tux%s1│       │tux%s2│\n       └─────┘       └─────┘\n172.16.%s0.0/24       172.16.%s1.0/24\n\n" "$Y" "$Y" "$Y" "$Y" "$Y"

read -p "Which Device in the network are we configuring? > " tux



if [ $tux -eq 1 ]
then
    printf "Machine  is Tux %s\n    -> Configuring IP on eth0 Device to 172.16.%s0.0/24\n        -> Bringing interface Down....\n" "$tux" "$Y"
    #sudo ifconfig eth0 down
    printf "        -> Bringing device eth0 back online with IP 172.16.%s0.0/24\n" "$Y"
    #sudo ifconfig eth0 172.16.10.1/24
    printf "    ....Done\n    -> Configuring Routes\n        -> to VLAN%s1 through tux%s4\n" "$Y" "$Y"
    #sudo route add -net 172.16.11.0/24 gw 172.16.10.254
    printf "        -> to internet through RC (172.16.1.19)\n"
    #sudo route add default gw 172.16.1.19
    printf "->DNS\n"
    #sudo echo "search netlab.fe.up.pt\n nameserver  172.16.1.2" > /etc/resolv.conf



elif [ $tux -eq 4 ]
then
    printf "Machine  is Tux %s\n-> Configuring this machine as Tunnel for two VLAN's \n    -> eth0 for VLAN%s0\n        -> Bringing interface eth0 Down....\n" "$tux" "$Y"
    #sudo ifconfig eth0 down
    printf "        -> Bringing device eth0 back online with IP 172.16.%s0.254/24\n"  "$Y"
    #sudo ifconfig eth0 172.16.10.254/24
    printf "    -> eth1 for VLAN%s1\n        -> Bringing interface eth1 Down....\n"
    #sudo ifconfig eth1 down
    printf "        -> Bringing device eth1 back online with IP 172.16.%s1.253/24\n"  "$Y"
    #sudo ifconfig eth1 172.16.11.253/24
    printf "....Done\n-> Configuring IP Forwarding\n "
    #echo 1 > /proc/sys/net/ipv4/ip_forward
    #echo 0 >  /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts
    printf "....Done"
    printf "->DNS\n"
    #sudo echo "search netlab.fe.up.pt\n nameserver  172.16.1.2" > /etc/resolv.conf

elif [ $tux -eq 2 ]
then
    printf "Machine  is Tux %s\n    -> Configuring IP on eth0 Device to 172.16.%s1.0/24\n       -> Bringing interface Down....\n" "$tux" "$Y"
    #sudo ifconfig eth0 down
    printf "    -> Bringing device eth0 back online with IP 172.16.%s1.0/24....\n" "$Y"
    #sudo ifconfig eth0 172.16.11.1/24
    printf "    ....Done\n    -> Configuring Routes\n        -> to VLAN%s0 through tux%s4\n" "$Y" "$Y"
    #sudo route add -net 172.16.10.0/24 gw 172.16.11.253
    printf "        -> to internet through RC (172.16.1.19)"
    #sudo route add default gw 172.16.1.19
    printf "->DNS\n"
    #sudo echo "search netlab.fe.up.pt\n nameserver  172.16.1.2" > /etc/resolv.conf
else
    printf "Not a valid Machine number"
fi

